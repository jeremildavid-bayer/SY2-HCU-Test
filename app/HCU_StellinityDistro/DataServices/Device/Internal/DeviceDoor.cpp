#include "Apps/AppManager.h"
#include "Common/ImrParser.h"
#include "DeviceDoor.h"
#include "DataServices/Workflow/DS_WorkflowData.h"
#include "DataServices/Exam/DS_ExamData.h"
#include "DataServices/Device/DS_DeviceData.h"
#include "DataServices/Device/DS_DeviceAction.h"
#include "DataServices/Mcu/DS_McuData.h"
#include "DataServices/Mcu/Internal/McuAlarm.h"
#include "DataServices/System/DS_SystemData.h"
#include "DataServices/Alert/DS_AlertAction.h"
#include "DataServices/CfgLocal/DS_CfgLocal.h"
#include "DataServices/Test/DS_TestData.h"

DeviceDoor::DeviceDoor(QObject *parent, EnvGlobal *env_) :
    ActionBaseExt(parent, env_)
{
    envLocal = new EnvLocal("DS_Device-Door", "DEVICE_DOOR", LOG_MID_SIZE_BYTES);

    connect(&tmrBackgroundMoodLightPlay, SIGNAL(timeout()), this, SLOT(slotBackgroundMoodLightPlay()));

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });

    setStateAsynch(STATE_INACTIVE);
}

DeviceDoor::~DeviceDoor()
{
    delete envLocal;
}

void DeviceDoor::slotAppInitialised()
{
    connect(env->ds.systemData, &DS_SystemData::signalDataChanged_StatePath, this, [=](DS_SystemDef::StatePath curStatePath, DS_SystemDef::StatePath prevStatePath) {
        if ( ( (prevStatePath == DS_SystemDef::STATE_PATH_IDLE) && (curStatePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING) ) ||
             ( (prevStatePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING) && (curStatePath == DS_SystemDef::STATE_PATH_IDLE) ) )
        {
            LOG_INFO("\n\n-----------------\nStatePath=%s\n\n", ImrParser::ToImr_StatePath(curStatePath).CSTR());
        }
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_DoorState, this, [=](DS_McuDef::DoorState state, DS_McuDef::DoorState prevState) {
        LOG_INFO("signalDataChanged_DoorState(): Door state changed from %s to %s\n", ImrParser::ToImr_DoorState(prevState).CSTR(), ImrParser::ToImr_DoorState(state).CSTR());

        if ( (prevState == DS_McuDef::DOOR_UNKNOWN) ||
             (state == prevState) )
        {
            // No door state changed
            return;
        }

        if (state == DS_McuDef::DOOR_OPEN)
        {
            env->ds.alertAction->activate("SRUDoorOpened");
        }
        else if (state == DS_McuDef::DOOR_CLOSED)
        {
            env->ds.alertAction->activate("SRUDoorClosed");
        }

        if (state != DS_McuDef::DOOR_UNKNOWN)
        {
            if (env->ds.alertAction->isActivated("DoorOpenFault", "", true))
            {
                env->ds.alertAction->deactivate("DoorOpenFault");
            }
        }
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_InjectorStatus, this, [=] {
        setMoodLight();
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_MudsInserted, this, [=] {
        setMoodLight();
    });

    connect(env->ds.systemData, &DS_SystemData::signalDataChanged_StatePath, this, [=] {
        setMoodLight();
    });

    connect(env->ds.cfgLocal, &DS_CfgLocal::signalConfigChanged_Settings_Injector_MoodLightColor, this, [=] {
        setMoodLight();
    });

    connect(env->ds.testData, &DS_TestData::signalDataChanged_TestStatus, this, [=] {
        setMoodLight();
    });

    connect(env->ds.deviceData, &DS_DeviceData::signalDataChanged_MudsLineFluidSyringeIndex, this, [=]() {
        setMoodLight();
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_StopcockPosAll, this, [=]() {
        // Copying suds leds when stopcocks move since StatePath changes happen before stopcock movements and it ends up copying previous SUDS LEDS
        copySudsLeds();
    });

    connect(env->ds.workflowData, &DS_WorkflowData::signalDataChanged_WorkflowState, this, [=](DS_WorkflowDef::WorkflowState workflowState, DS_WorkflowDef::WorkflowState workflowStatePrev) {
        if ( (workflowState != DS_WorkflowDef::STATE_INACTIVE) &&
             (workflowStatePrev == DS_WorkflowDef::STATE_INACTIVE) )
        {
            state = STATE_INIT;
            processState();
        }
        else if ( (workflowState == DS_WorkflowDef::STATE_INACTIVE) &&
                  (workflowStatePrev != DS_WorkflowDef::STATE_INACTIVE) )
        {
            state = STATE_INACTIVE;
            processState();
        }
    });
}

bool DeviceDoor::isSetStateReady()
{
    if (state == STATE_INACTIVE)
    {
        return false;
    }
    return true;
}

void DeviceDoor::processState()
{
    switch (state)
    {
    case STATE_INACTIVE:
        LOG_INFO("STATE_INACTIVE\n");
        break;
    case STATE_INIT:
        LOG_INFO("STATE_INIT\n");
        break;
    default:
        LOG_ERROR("BAD STATE(%d)\n", state);
        break;
    }
}

void DeviceDoor::setMoodLight()
{
    static QString lastLedChangeReason = "";

    DS_McuDef::ActLedParams params;
    params.isFlashing = false;
    params.setNoChange();

    DS_SystemDef::StatePath statePath = env->ds.systemData->getStatePath();
    bool mudsInserted = env->ds.mcuData->getMudsInserted();
    bool moodLightChanged = false;
    DS_McuDef::InjectorStatus mcuProgress = env->ds.mcuData->getInjectorStatus();
    DS_TestDef::TestStatus testStatus = env->ds.testData->getTestStatus();
    QString ledChangeReason = "";
    QString injectorStateStr;

    if (testStatus.guid != EMPTY_GUID)
    {
        ledChangeReason = "Cycle Test is in progress. Start MoodLight";
        backgroundMoodLightStart();
        goto bail;
    }

    if ( (statePath == DS_SystemDef::STATE_PATH_STARTUP_UNKNOWN) ||
         (statePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING) ||
         (statePath == DS_SystemDef::STATE_PATH_SERVICING) )
    {
        ledChangeReason = QString().asprintf("StatePath is %s", ImrParser::ToImr_StatePath(statePath).CSTR());
        params.setColorOff();
        moodLightChanged = true;
        goto bail;
    }

    if (!mudsInserted)
    {
        ledChangeReason = "MUDS not inserted. Start MoodLight";
        backgroundMoodLightStart();
        goto bail;
    }

    injectorStateStr = ImrParser::ToImr_InjectorState(mcuProgress.state);

    switch (mcuProgress.state)
    {
    case DS_McuDef::INJECTOR_STATE_IDLE:
        ledChangeReason = QString().asprintf("MCU State is %s", injectorStateStr.CSTR());
        params.setColorOff();
        moodLightChanged = true;
        break;
    case DS_McuDef::INJECTOR_STATE_READY_START:
        ledChangeReason = QString().asprintf("MCU State is %s", injectorStateStr.CSTR());
        params.setColorYellow();
        params.isFlashing = true;
        moodLightChanged = true;
        break;
    case DS_McuDef::INJECTOR_STATE_DELIVERING:
    case DS_McuDef::INJECTOR_STATE_PHASE_PAUSED:
    case DS_McuDef::INJECTOR_STATE_HOLDING:
    case DS_McuDef::INJECTOR_STATE_COMPLETING:
        // during injection, door lights are copied from suds.
        // Not copying SUDS LEDS here since StatePath changes happen before stopcock movements and it ends up copying previous SUDS LEDS
        // refer to copySudsLeds()
        moodLightChanged = false;
        ledChangeReason = QString().asprintf("MCU State is %s: Copying SUDS LEDs\n", injectorStateStr.CSTR());
        break;
    case DS_McuDef::INJECTOR_STATE_COMPLETED:
        ledChangeReason = QString().asprintf("MCU State is %s", injectorStateStr.CSTR());
        params.setColorOff();
        moodLightChanged = true;
        break;
    default:
        break;
    }

bail:
    if (ledChangeReason != lastLedChangeReason)
    {
        LOG_DEBUG("setMoodLight(): moodLightChanged=%s, Reason=%s, Params[%s]=%s\n",
                  moodLightChanged ? "TRUE" : "FASE",
                  ledChangeReason.CSTR(),
                  ImrParser::ToImr_LedIndex(LED_IDX_DOOR1).CSTR(),
                  Util::qVarientToJsonData(ImrParser::ToImr_ActLedParams(params)).CSTR());

        if (moodLightChanged)
        {
            backgroundMoodLightStop();
            env->ds.deviceAction->actLeds(LED_IDX_DOOR1, params);
        }

        lastLedChangeReason = ledChangeReason;
    }
}

void DeviceDoor::backgroundMoodLightStart()
{
    QString moodLightColorData = env->ds.cfgLocal->get_Settings_Injector_MoodLightColor();
    if (moodLightColorData == curMoodLightColorData)
    {
        // No change in mood light color data
        return;
    }

    curMoodLightColorData = moodLightColorData;

    DS_McuDef::ActLedParams params;
    bool isSequenceType = false;

    if (curMoodLightColorData == _L("off"))
    {
        params.setColorOff();
    }
    else if (curMoodLightColorData == _L("white"))
    {
        params.setColorWhite();
    }
    else if (curMoodLightColorData == _L("blue"))
    {
        params.setColorSaline();
    }
    else if (curMoodLightColorData == _L("green"))
    {
        params.setColorContrast1();
    }
    else if (curMoodLightColorData == _L("purple"))
    {
        params.setColorContrast2();
    }
    else
    {
        isSequenceType = true;

        // Mood Light is sequence type
        if (curMoodLightColorData == _L("imaxeon"))
        {
            curMoodLightColorData = LED_MOOD_LIGHT_CONFIG_IMAXEON;
        }
        else if (curMoodLightColorData == _L("kipper"))
        {
            curMoodLightColorData = LED_MOOD_LIGHT_CONFIG_KIPPER;
        }
        else if (curMoodLightColorData == _L("heartbeat"))
        {
            curMoodLightColorData = LED_MOOD_LIGHT_CONFIG_HEARTBEAT;
        }

        processBackgroundMoodLight();
    }

    if (!isSequenceType)
    {
        backgroundMoodLightStop();
        LOG_INFO("Mood light is set with %s\n", Util::qVarientToJsonData(ImrParser::ToImr_ActLedParams(params)).CSTR());
        env->ds.deviceAction->actLeds(LED_IDX_DOOR1, params);
    }
}

void DeviceDoor::processBackgroundMoodLight()
{
    // Transform curMoodLightColorData to backgroundMoodLightSequenceItems
    QJsonParseError parseErr;
    QJsonDocument document = QJsonDocument::fromJson(curMoodLightColorData.toUtf8(), &parseErr);
    QVariantList listItems = document.toVariant().toList();
    backgroundMoodLightSequenceItems.clear();

    if (parseErr.error != QJsonParseError::NoError)
    {
        LOG_ERROR("Bad Background Mood Light Sequence Data (data=%s, parseErr=%s)\n", curMoodLightColorData.CSTR(), parseErr.errorString().CSTR());
        return;
    }

    // Expected item format: { From: <RGB>, To: <RGB>, Duration: <TimeMs> }
    // E.g. { "From": "ff0000", "To": "00ff00", "Duration": 1000 }
    // Note: Duration has to be step of LED_MOOD_LIGHT_BACKGROUND_CTRL_INTERVAL_MS

    for (int itemIdx = 0; itemIdx < listItems.length(); itemIdx++)
    {
        // Convert itemMap to sequenceItem:
        // E.g. { "From": "ff0000", "To": "00ff00", "Duration": 1000 }
        //      -> t0:    { "Color": (ff0000 * 100%) + (00ff00 * 0%  ) }
        //      -> t200:  { "Color": (ff0000 * 80% ) + (00ff00 * 20% ) }
        //      -> t400:  { "Color": (ff0000 * 60% ) + (00ff00 * 40% ) }
        //      -> t600:  { "Color": (ff0000 * 40% ) + (00ff00 * 60% ) }
        //      -> t800:  { "Color": (ff0000 * 20% ) + (00ff00 * 80% ) }
        //      -> t1000: { "Color": (ff0000 * 0%  ) + (00ff00 * 100%) }

        QVariantMap mapItem = listItems[itemIdx].toMap();

        if ( (!mapItem.contains("Duration")) ||
             (!mapItem.contains("From")) ||
             (!mapItem.contains("To")) )
        {
            LOG_ERROR("Bad Background Mood Light Sequence Data[%d] (data=%s, Err=Missing field)\n", itemIdx, curMoodLightColorData.CSTR());
            continue;
        }

        int colorSteps = mapItem["Duration"].toInt() / LED_MOOD_LIGHT_BACKGROUND_CTRL_INTERVAL_MS;
        int subItemCount = colorSteps + 1;
        int colorMixPercentIncrement = 100 / colorSteps;

        for (int subItemIdx = 0; subItemIdx < subItemCount; subItemIdx++)
        {
            double colorMixPercent = colorMixPercentIncrement * subItemIdx;
            bool convertState;
            QString colorFrom = mapItem["From"].toString();
            int r0 = colorFrom.mid(0, 2).toUInt(&convertState, 16);
            int g0 = colorFrom.mid(2, 2).toUInt(&convertState, 16);
            int b0 = colorFrom.mid(4, 2).toUInt(&convertState, 16);
            QString colorTo = mapItem["To"].toString();
            int r1 = colorTo.mid(0, 2).toUInt(&convertState, 16);
            int g1 = colorTo.mid(2, 2).toUInt(&convertState, 16);
            int b1 = colorTo.mid(4, 2).toUInt(&convertState, 16);

            // Mix color
            int r2 = qMin(((r0 * (100 - colorMixPercent)) / 100) + ((r1 * colorMixPercent) / 100), 255.0);
            int g2 = qMin(((g0 * (100 - colorMixPercent)) / 100) + ((g1 * colorMixPercent) / 100), 255.0);
            int b2 = qMin(((b0 * (100 - colorMixPercent)) / 100) + ((b1 * colorMixPercent) / 100), 255.0);

            // Add sequence Item
            DS_McuDef::ActLedParams sequenceItem;
            sequenceItem.type = DS_McuDef::LED_CONTROL_TYPE_SET;
            sequenceItem.colorR = r2;
            sequenceItem.colorG = g2;
            sequenceItem.colorB = b2;

            //LOG_DEBUG("Adding SequenceItem[%d]: %s\n", backgroundMoodLightSequenceItems.length(), Util::qVarientToJsonData(ImrParser::ToImr_ActLedParams(sequenceItem)).CSTR());
            backgroundMoodLightSequenceItems.append(sequenceItem);
        }
        LOG_INFO("SequenceItems(len=%d) is created\n", (int)backgroundMoodLightSequenceItems.length());
    }

    if (backgroundMoodLightSequenceItems.length() == 0)
    {
        LOG_ERROR("Bad Background Mood Light Sequence Data (data=%s, Err=No Sequence Item)\n", curMoodLightColorData.CSTR());
        return;
    }

    LOG_INFO("Background Mood Light Play started with %d sequence items, interval=%dms\n", (int)backgroundMoodLightSequenceItems.length(), LED_MOOD_LIGHT_BACKGROUND_CTRL_INTERVAL_MS);
    backgroundMoodLightSequenceIdx = 0;
    tmrBackgroundMoodLightPlay.start(LED_MOOD_LIGHT_BACKGROUND_CTRL_INTERVAL_MS);
}

void DeviceDoor::backgroundMoodLightStop()
{
    LOG_INFO("backgroundMoodLightStop()\n");
    tmrBackgroundMoodLightPlay.stop();
    backgroundMoodLightSequenceItems.clear();
}

void DeviceDoor::slotBackgroundMoodLightPlay()
{
    if (backgroundMoodLightSequenceItems.length() == 0)
    {
        return;
    }

    if (backgroundMoodLightSequenceIdx >= backgroundMoodLightSequenceItems.length())
    {
        backgroundMoodLightSequenceIdx = 0;
    }

    DS_McuDef::ActLedParams sequenceItem = backgroundMoodLightSequenceItems[backgroundMoodLightSequenceIdx];
    env->ds.deviceAction->actLeds(LED_IDX_DOOR1, sequenceItem);
    backgroundMoodLightSequenceIdx++;
}

void DeviceDoor::copySudsLeds()
{
    DS_McuDef::InjectorStatus mcuProgress = env->ds.mcuData->getInjectorStatus();
    switch(mcuProgress.state)
    {
    case DS_McuDef::INJECTOR_STATE_DELIVERING:
    case DS_McuDef::INJECTOR_STATE_PHASE_PAUSED:
    case DS_McuDef::INJECTOR_STATE_HOLDING:
    case DS_McuDef::INJECTOR_STATE_COMPLETING:
        // stopcock movements during injection, copy SUDS LEDS
        backgroundMoodLightStop();
        env->ds.deviceAction->actCopyLeds(LED_IDX_SUDS1, LED_IDX_DOOR1);
        break;
    default:
        // do nothing
        break;
    }
}
