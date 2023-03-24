#include "QML_McuSim.h"
#include "Common/Util.h"
#include "Common/ImrParser.h"
#include "DataServices/Mcu/DS_McuData.h"
#include "DataServices/Mcu/Internal/McuAlarm.h"
#include "DataServices/Capabilities/DS_Capabilities.h"
#include "DataServices/Workflow/DS_WorkflowData.h"
#include "DataServices/Workflow/DS_WorkflowAction.h"
#include "DataServices/Device/DS_DeviceData.h"
#include "DataServices/Device/DS_DeviceAction.h"

QML_McuSim::QML_McuSim(QObject *parent, EnvGlobal *env_) :
    QObject(parent),
    env(env_)
{
    envLocal = new EnvLocal("QML_McuSim", "QML_MCU_SIM");
    qmlSrc = env->qml.object->findChild<QObject*>("dsMcuSim");
    env->qml.engine->rootContext()->setContextProperty("dsMcuSimCpp", this);

    if (qmlSrc == NULL)
    {
        LOG_ERROR("Failed to assign qmlSrc.\n");
    }

    // Register Data Callbacks
    connect(env->ds.capabilities, &DS_Capabilities::signalConfigChanged_Developer_McuSimulationEnabled, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("simulatorEnabled", cfg.value);
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_LedControlStatus, this, [=](const DS_McuDef::LedControlStatus &ledControlStatus) {
        qmlSrc->setProperty("ledControlStatus", ImrParser::ToImr_LedControlStatus(ledControlStatus));
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_SimDigest, this, [=](const DS_McuDef::SimDigest &simDigest) {
        qmlSrc->setProperty("syringesAirVolume", simDigest.syringesAirVolume);
    });
}

QML_McuSim::~QML_McuSim()
{
    delete envLocal;
}

void QML_McuSim::slotDoorStateChanged(QString state)
{
    DS_McuDef::SimDigest data = env->ds.mcuData->getSimDigest();
    data.doorState = state;
    env->ds.mcuData->setSimDigest(data);
}

void QML_McuSim::slotOutletDoorStateChanged(QString state)
{
    DS_McuDef::SimDigest data = env->ds.mcuData->getSimDigest();
    data.outletDoorState = state;
    env->ds.mcuData->setSimDigest(data);
}

void QML_McuSim::slotMudsPresent(bool present)
{
    DS_McuDef::SimDigest data = env->ds.mcuData->getSimDigest();
    data.mudsPresent = present;

    data.bottleAirDetectedState[SYRINGE_IDX_SALINE] = present ? DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_FLUID : DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_MISSING;
    data.bottleAirDetectedState[SYRINGE_IDX_CONTRAST1] = present ? DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_FLUID : DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_MISSING;
    data.bottleAirDetectedState[SYRINGE_IDX_CONTRAST2] = present ? DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_FLUID : DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_MISSING;

    env->ds.mcuData->setSimDigest(data);
}

void QML_McuSim::slotMudsLatched(bool latched)
{
    DS_McuDef::SimDigest data = env->ds.mcuData->getSimDigest();
    data.mudsLatched = latched;
    env->ds.mcuData->setSimDigest(data);
}

void QML_McuSim::slotSudsInserted(bool inserted)
{
    DS_McuDef::SimDigest data = env->ds.mcuData->getSimDigest();
    data.sudsInserted = inserted;
    env->ds.mcuData->setSimDigest(data);
}

void QML_McuSim::slotBubbleSaline()
{
    DS_McuDef::BottleBubbleDetectorState curState = env->ds.mcuData->getBottleBubbleStates()[SYRINGE_IDX_SALINE];
    DS_McuDef::BottleBubbleDetectorState newState;
    switch (curState)
    {
    case DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_MISSING:
        newState = DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_FLUID;
        break;
    case DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_FLUID:
        newState = DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_AIR;
        break;
    default:
        newState = DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_MISSING;
        break;
    }

    DS_McuDef::SimDigest data = env->ds.mcuData->getSimDigest();
    data.bottleAirDetectedState[SYRINGE_IDX_SALINE] = newState;
    env->ds.mcuData->setSimDigest(data);
}

void QML_McuSim::slotBubbleContrast1()
{
    DS_McuDef::BottleBubbleDetectorState curState = env->ds.mcuData->getBottleBubbleStates()[SYRINGE_IDX_CONTRAST1];
    DS_McuDef::BottleBubbleDetectorState newState;
    switch (curState)
    {
    case DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_MISSING:
        newState = DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_FLUID;
        break;
    case DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_FLUID:
        newState = DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_AIR;
        break;
    default:
        newState = DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_MISSING;
        break;
    }

    DS_McuDef::SimDigest data = env->ds.mcuData->getSimDigest();
    data.bottleAirDetectedState[SYRINGE_IDX_CONTRAST1] = newState;
    env->ds.mcuData->setSimDigest(data);
}

void QML_McuSim::slotBubbleContrast2()
{
    DS_McuDef::BottleBubbleDetectorState curState = env->ds.mcuData->getBottleBubbleStates()[SYRINGE_IDX_CONTRAST2];
    DS_McuDef::BottleBubbleDetectorState newState;
    switch (curState)
    {
    case DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_MISSING:
        newState = DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_FLUID;
        break;
    case DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_FLUID:
        newState = DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_AIR;
        break;
    default:
        newState = DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_MISSING;
        break;
    }

    DS_McuDef::SimDigest data = env->ds.mcuData->getSimDigest();
    data.bottleAirDetectedState[SYRINGE_IDX_CONTRAST2] = newState;
    env->ds.mcuData->setSimDigest(data);
}

void QML_McuSim::slotAdaptiveFlowStateChanged(QString state)
{
    DS_McuDef::InjectDigest data = env->ds.mcuData->getSimInjectDigest();
    data.adaptiveFlowState = ImrParser::ToCpp_McuAdaptiveFlowState(state);
    env->ds.mcuData->setSimInjectDigest(data);
}

void QML_McuSim::slotSyringesAir()
{
    DS_McuDef::SimDigest data = env->ds.mcuData->getSimDigest();
    data.syringesAirVolume = (data.syringesAirVolume > 0) ? 0 : 10;
    env->ds.mcuData->setSimDigest(data);

}
void QML_McuSim::slotBubblePatientLine()
{
    DS_McuDef::SimDigest data = env->ds.mcuData->getSimDigest();
    data.sudsBubbleDetected = !data.sudsBubbleDetected;
    env->ds.mcuData->setSimDigest(data);
}

void QML_McuSim::slotWasteBinStateChanged(QString state)
{
    DS_McuDef::SimDigest data = env->ds.mcuData->getSimDigest();
    data.wasteBinState = state;
    env->ds.mcuData->setSimDigest(data);
}

void QML_McuSim::slotStopButtonPressed(bool pressed)
{
    DS_McuDef::SimDigest data = env->ds.mcuData->getSimDigest();
    data.stopBtnPressed = pressed;
    env->ds.mcuData->setSimDigest(data);
}

void QML_McuSim::slotManualPrimeButtonPressed(bool pressed)
{
    DS_McuDef::SimDigest data = env->ds.mcuData->getSimDigest();
    data.primeBtnPressed = pressed;
    env->ds.mcuData->setSimDigest(data);
}

void QML_McuSim::slotBatteryStateChanged(QString state)
{
    DS_McuDef::SimDigest data = env->ds.mcuData->getSimDigest();
    data.batteryState = state;
    env->ds.mcuData->setSimDigest(data);
}

void QML_McuSim::slotAcConnectedStateChanged(QString state)
{
    DS_McuDef::SimDigest data = env->ds.mcuData->getSimDigest();
    data.isAcPowered = state == _L("AC");
    env->ds.mcuData->setSimDigest(data);
}

void QML_McuSim::slotTemperature1Changed(double temperature)
{
    DS_McuDef::SimDigest data = env->ds.mcuData->getSimDigest();
    data.temperature1 = temperature;
    env->ds.mcuData->setSimDigest(data);
}

void QML_McuSim::slotTemperature2Changed(double temperature)
{
    DS_McuDef::SimDigest data = env->ds.mcuData->getSimDigest();
    data.temperature2 = temperature;
    env->ds.mcuData->setSimDigest(data);
}

QString QML_McuSim::slotAlarmGetNext(QString alarmName)
{
    int alarmIdx = (int)McuAlarm::getAlarmId(alarmName);
    alarmIdx++;
    QString nextAlarm = McuAlarm::getAlarmName((McuAlarm::AlarmId)alarmIdx);

    if (nextAlarm.contains("ALARM_BAD_ID"))
    {
        nextAlarm = McuAlarm::getAlarmName(McuAlarm::ALARM_ID_START);
    }

    return nextAlarm;

}

void QML_McuSim::slotAlarmActivate(QString alarmName)
{
    McuAlarm::AlarmId alarmId = McuAlarm::getAlarmId(alarmName);
    DS_McuDef::SimDigest data = env->ds.mcuData->getSimDigest();
    data.alarmCode = McuAlarm::getAlarmBitsFromAlarmIdSet(alarmId);
    env->ds.mcuData->setSimDigest(data);
}
