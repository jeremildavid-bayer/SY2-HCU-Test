#include "DeviceLeds.h"
#include "Apps/AppManager.h"
#include "DataServices/Workflow/DS_WorkflowData.h"
#include "DataServices/Mcu/DS_McuAction.h"
#include "DataServices/Mcu/DS_McuData.h"
#include "DataServices/CfgGlobal/DS_CfgGlobal.h"
#include "DataServices/CfgLocal/DS_CfgLocal.h"
#include "DataServices/System/DS_SystemData.h"
#include "Common/ImrParser.h"

DeviceLeds::DeviceLeds(QObject *parent, EnvGlobal *env_) :
    ActionBase(parent, env_)
{
    envLocal = new EnvLocal("DS_Device-Led", "DEVICE_LED", LOG_MID_SIZE_BYTES);

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });

    connect(&tmrFlash, &QTimer::timeout, this, &DeviceLeds::slotFlash);

    for (int ledIdx = 0; ledIdx < LED_IDX_MAX; ledIdx++)
    {
        DS_McuDef::ActLedParams params;
        params.setColorOff();
        curLedParamsList.append(params);
    }

    flashFlag = false;
}

DeviceLeds::~DeviceLeds()
{
    tmrFlash.stop();
    delete envLocal;
}

void DeviceLeds::slotAppInitialised()
{
    connect(env->ds.systemData, &DS_SystemData::signalDataChanged_StatePath, this, [=](DS_SystemDef::StatePath curStatePath, DS_SystemDef::StatePath prevStatePath) {
        if ( ( (prevStatePath == DS_SystemDef::STATE_PATH_IDLE) && (curStatePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING) ) ||
             ( (prevStatePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING) && (curStatePath == DS_SystemDef::STATE_PATH_IDLE) ) )
        {
            LOG_INFO("\n\n-----------------\nStatePath=%s\n\n", ImrParser::ToImr_StatePath(curStatePath).CSTR());
        }
    });

    connect(env->ds.workflowData, &DS_WorkflowData::signalDataChanged_WorkflowState, this, [=](DS_WorkflowDef::WorkflowState workflowState, DS_WorkflowDef::WorkflowState workflowStatePrev) {
        if ( (workflowState != DS_WorkflowDef::STATE_INACTIVE) &&
             (workflowStatePrev == DS_WorkflowDef::STATE_INACTIVE) )
        {
            tmrFlash.stop();
            tmrFlash.start(LED_FLASH_INTERVAL_MS);
        }
        else if ( (workflowState == DS_WorkflowDef::STATE_INACTIVE) &&
                  (workflowStatePrev != DS_WorkflowDef::STATE_INACTIVE) )
        {
            DS_McuDef::ActLedParams param;
            param.setColorOff();
            actLeds(LED_IDX_SALINE, param);
            actLeds(LED_IDX_CONTRAST1, param);
            actLeds(LED_IDX_CONTRAST2, param);
            actLeds(LED_IDX_SUDS1, param);
            actLeds(LED_IDX_DOOR1, param);
            actLeds(LED_IDX_AIR_DOOR, param);
            tmrFlash.stop();
        }
    });
}

DataServiceActionStatus DeviceLeds::actLeds(LedIndex index, DS_McuDef::ActLedParams params, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "Leds", QString().asprintf("%s;%s", ImrParser::ToImr_LedIndex(index).CSTR(), Util::qVarientToJsonData(ImrParser::ToImr_ActLedParams(params)).CSTR()));

    DS_McuDef::ActLedParams *curParams = &curLedParamsList[index];

    if ( (curParams->isFlashing == params.isFlashing) &&
         ( (*curParams == params) ||
           (params.type == DS_McuDef::LED_CONTROL_TYPE_NO_CHANGE) ) )
    {
        //LOG_DEBUG("LED[%s]: Duplicated LED request. Request ignored.\n", ImrParser::ToImr_LedIndex(index).CSTR());
        status.state = DS_ACTION_STATE_STARTED;
        actionStarted(status);
        return status;
    }

    *curParams = params;

    // Update global LED params
    if (index == LED_IDX_SUDS1)
    {
        curLedParamsList[LED_IDX_SUDS2] = params;
        curLedParamsList[LED_IDX_SUDS3] = params;
        curLedParamsList[LED_IDX_SUDS4] = params;
    }
    else if (index == LED_IDX_DOOR1)
    {
        curLedParamsList[LED_IDX_DOOR2] = params;
        curLedParamsList[LED_IDX_DOOR3] = params;
        curLedParamsList[LED_IDX_DOOR4] = params;
    }

    if (curParams->isFlashing)
    {
        status.state = DS_ACTION_STATE_STARTED;
        actionStarted(status);
        return status;
    }

    // Prepare params
    DS_McuDef::ActLedParamsList newParamsList;
    for (int ledIdx = 0; ledIdx < LED_IDX_MAX; ledIdx++)
    {
        DS_McuDef::ActLedParams params;
        newParamsList.append(params);
    }

    if (index == LED_IDX_SUDS1)
    {
        newParamsList[LED_IDX_SUDS1] = *curParams;
        newParamsList[LED_IDX_SUDS2] = *curParams;
        newParamsList[LED_IDX_SUDS3] = *curParams;
        newParamsList[LED_IDX_SUDS4] = *curParams;
    }
    else if (index == LED_IDX_DOOR1)
    {
        newParamsList[LED_IDX_DOOR1] = *curParams;
        newParamsList[LED_IDX_DOOR2] = *curParams;
        newParamsList[LED_IDX_DOOR3] = *curParams;
        newParamsList[LED_IDX_DOOR4] = *curParams;
    }
    else
    {
        newParamsList[index] = *curParams;
    }

    // Send request
    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);

        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            env->actionMgr->onActionCompleted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
                actionCompleted(curStatus, &status);
            });
        }
    });

    return env->ds.mcuAction->actLeds(newParamsList, guid);
}

DataServiceActionStatus DeviceLeds::actCopyLeds(LedIndex from, LedIndex to, QString actGuid)
{
    DS_McuDef::ActLedParams fromParams = curLedParamsList[from];

    return actLeds(to, fromParams);
}

void DeviceLeds::slotFlash()
{
    bool flashingOn = false;
    for (int ledIdx = 0; ledIdx < curLedParamsList.length(); ledIdx++)
    {
        if ( (curLedParamsList[ledIdx].type == DS_McuDef::LED_CONTROL_TYPE_SET) &&
             (curLedParamsList[ledIdx].isFlashing) )
        {
            flashingOn = true;
            break;
        }
    }

    if (!flashingOn)
    {
        return;
    }


    DS_McuDef::ActLedParamsList newParamsList;

    for (int ledIdx = 0; ledIdx < curLedParamsList.length(); ledIdx++)
    {
        DS_McuDef::ActLedParams params;
        if ( (curLedParamsList[ledIdx].type == DS_McuDef::LED_CONTROL_TYPE_SET) &&
             (curLedParamsList[ledIdx].isFlashing) )
        {
            if (flashFlag)
            {
                params.colorR = curLedParamsList[ledIdx].colorR;
                params.colorG = curLedParamsList[ledIdx].colorG;
                params.colorB = curLedParamsList[ledIdx].colorB;
                params.type = DS_McuDef::LED_CONTROL_TYPE_SET;
            }
            else
            {
                params.type = DS_McuDef::LED_CONTROL_TYPE_OFF;
            }
        }
        newParamsList.append(params);
    }

    flashFlag = !flashFlag;

    env->ds.mcuAction->actLeds(newParamsList);
}


