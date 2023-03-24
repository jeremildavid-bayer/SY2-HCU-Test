#include "Apps/AppManager.h"
#include "WorkflowAutomaticQualifiedDischarge.h"
#include "DataServices/Workflow/DS_WorkflowData.h"
#include "DataServices/Workflow/DS_WorkflowAction.h"
#include "DataServices/Capabilities/DS_Capabilities.h"
#include "DataServices/Mcu/DS_McuData.h"
#include "DataServices/Alert/DS_AlertAction.h"
#include "DataServices/Alert/DS_AlertData.h"

WorkflowAutomaticQualifiedDischarge::WorkflowAutomaticQualifiedDischarge(QObject *parent, EnvGlobal *env_) :
    ActionBaseExt(parent, env_)
{
    envLocal = new EnvLocal("DS_Workflow-AutomaticQualifiedDischarge", "WORKFLOW_AUTOMATIC_QUALIFIED_DISCHARGE");

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialized();
    });

    reset();
}

WorkflowAutomaticQualifiedDischarge::~WorkflowAutomaticQualifiedDischarge()
{
    delete envLocal;
}

DS_WorkflowDef::AutomaticQualifiedDischargeStatus WorkflowAutomaticQualifiedDischarge::getAutomaticQualifiedDischargeStatus()
{
    return env->ds.workflowData->getAutomaticQualifiedDischargeStatus();
}

void WorkflowAutomaticQualifiedDischarge::slotAppInitialized()
{
    DS_WorkflowDef::AutomaticQualifiedDischargeStatus status;
    status.state = DS_WorkflowDef::AQD_STATE_IDLE;
    status.message = "";
    env->ds.workflowData->setAutomaticQualifiedDischargeStatus(status);

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_BMSDigests, this, [=](const DS_McuDef::BMSDigests &bmsDigests, const DS_McuDef::BMSDigests &prevBmsDigests) {
        if (signalBMSChanged_func != NULL)
        {
            (this->*signalBMSChanged_func)(bmsDigests, prevBmsDigests);
        }
    });

    connect(env->ds.capabilities, &DS_Capabilities::signalConfigChanged_BMS_AQD_BatteryPackA_MaxError_Trigger, this, [=](const Config::Item &cfg){
        batteryATrigger = cfg.value.toInt();
    });

    connect(env->ds.capabilities, &DS_Capabilities::signalConfigChanged_BMS_AQD_BatteryPackB_MaxError_Trigger, this, [=](const Config::Item &cfg){
        batteryBTrigger = cfg.value.toInt();
    });

    connect(env->ds.capabilities, &DS_Capabilities::signalConfigChanged_BMS_AQD_OtherBatteryChargeLimit, this, [=](const Config::Item &cfg){
        otherBatteryChargeLimit = cfg.value.toInt();
    });

    processState();
}

void WorkflowAutomaticQualifiedDischarge::logLiveBMS(const DS_McuDef::BMSDigests &bmsDigests)
{
    LOG_INFO("Live BMS Data for battery [%s]:\n", getBatteryName(batteryIndex).CSTR());
    LOG_INFO("%s\n", Util::qVarientToJsonData((ImrParser::ToImr_BMSDigest(bmsDigests[batteryIndex])), false).CSTR());
}

DataServiceActionStatus WorkflowAutomaticQualifiedDischarge::actForceQueAQD(int batteryIdx, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "ForceQueAQD");

    batteryIndex = batteryIdx;

    setState(DS_WorkflowDef::AQD_STATE_AQD_QUEUED);

    actStatusBuf = status;
    actStatusBuf.state = DS_ACTION_STATE_STARTED;
    actionStarted(actStatusBuf);
    return actStatusBuf;
}

DataServiceActionStatus WorkflowAutomaticQualifiedDischarge::actCancelAQD(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "CancelAQD");

    if (getState() == DS_WorkflowDef::AQD_STATE_AQD_QUEUED)
    {
        setState(DS_WorkflowDef::AQD_STATE_IDLE);
    }
    else if (getState() == DS_WorkflowDef::AQD_STATE_DISCHARGE_PROGRESS)
    {
        env->ds.workflowAction->actWorkflowBatteryAbort();
    }
    else
    {
        LOG_ERROR("AQD Canceled in invalid state!\n");
        actStatusBuf = status;
        actStatusBuf.state = DS_ACTION_STATE_INVALID_STATE;
        actStatusBuf.err = "AQD Canceled in invalid state";
        actionStarted(actStatusBuf);
        return actStatusBuf;
    }

    actStatusBuf = status;
    actStatusBuf.state = DS_ACTION_STATE_STARTED;
    actionStarted(actStatusBuf);
    return actStatusBuf;
}

void WorkflowAutomaticQualifiedDischarge::BMSChanged_Idle(const DS_McuDef::BMSDigests &bmsDigests, const DS_McuDef::BMSDigests &prevBmsDigests)
{
    // Idle, don't flood logging

    batteryPackMissing = (env->ds.alertAction->isActivated("GeneralI2CFault", "ALARM_I2C_GENERAL_BATTERY_MANAGEMENT_SYSTEM_A") ||
                          env->ds.alertAction->isActivated("GeneralI2CFault", "ALARM_I2C_GENERAL_BATTERY_MANAGEMENT_SYSTEM_B"));

    batteryPermanentFailures = (bmsDigests[0].pfStatusBytes.contains(QRegularExpression("[^0 ]")) || bmsDigests[1].pfStatusBytes.contains(QRegularExpression("[^0 ]")));

    bool aqdCanRun = (!batteryPackMissing && !batteryPermanentFailures);

    if (aqdCanRun)
    {
        // only que if we are not in service mode
        DS_SystemDef::StatePath statePath = env->ds.systemData->getStatePath();
        if (statePath != DS_SystemDef::STATE_PATH_BUSY_SERVICING)
        {
            // Using different limits so both packs don't require AQD at the same time - set from capabilities
            batteryARequiresAQD = (bmsDigests[0].sbsStatus.maxError >= batteryATrigger);
            batteryBRequiresAQD = (bmsDigests[1].sbsStatus.maxError >= batteryBTrigger);

            if (batteryARequiresAQD)
            {
                batteryIndex = 0;
                LOG_INFO("Battery Pack A max error = %d, queing AQD\n", bmsDigests[batteryIndex].sbsStatus.maxError);
                setState(DS_WorkflowDef::AQD_STATE_AQD_QUEUED);
            }
            else if (batteryBRequiresAQD)
            {
                batteryIndex = 1;
                LOG_INFO("Battery Pack B max error = %d, queing AQD\n", bmsDigests[batteryIndex].sbsStatus.maxError);
                setState(DS_WorkflowDef::AQD_STATE_AQD_QUEUED);
            }
        }
    }
}

void WorkflowAutomaticQualifiedDischarge::BMSChanged_AQDQueued(const DS_McuDef::BMSDigests &bmsDigests, const DS_McuDef::BMSDigests &prevBmsDigests)
{
    LOG_INFO("AQD Queued...\n");
    for (int i = 0; i < POWER_BATTERY_INDEX_MAX; i++)
    {
        LOG_INFO("\t Battery [%s]: Current = %d, Relative State of Charge = %d%%, AllFetAction = %s\n", getBatteryName(i).CSTR(), bmsDigests[i].sbsStatus.current, bmsDigests[i].sbsStatus.relativeStateOfCharge, (bmsDigests[i].manufacturingStatus.allFetAction?"enabled":"disabled"));
    }
    batteryPackMissing = (env->ds.alertAction->isActivated("GeneralI2CFault", "ALARM_I2C_GENERAL_BATTERY_MANAGEMENT_SYSTEM_A") ||
                          env->ds.alertAction->isActivated("GeneralI2CFault", "ALARM_I2C_GENERAL_BATTERY_MANAGEMENT_SYSTEM_B"));

    batteryPermanentFailures = (bmsDigests[0].pfStatusBytes.contains(QRegularExpression("[^0 ]")) || bmsDigests[1].pfStatusBytes.contains(QRegularExpression("[^0 ]")));

    bool aqdCanRun = (!batteryPackMissing && !batteryPermanentFailures);

    if (aqdCanRun)
    {
        DS_McuDef::PowerStatus powerStatus = env->ds.mcuData->getPowerStatus();
        // VDQ is set for target battery && other battery is above certain % - set from capabilities
        // AC power is NOT plugged in
        if (!powerStatus.isAcPowered && bmsDigests[batteryIndex].gaugingStatus.dischargeQualifiedForLearning && bmsDigests[getOtherBatteryIndex(batteryIndex)].sbsStatus.relativeStateOfCharge >= otherBatteryChargeLimit)
        {
            setState(DS_WorkflowDef::AQD_STATE_START);
            return;
        }
    }
    else
    {
        // return to idle
        LOG_INFO("AQD was queued but can't be run due to breached conditions. Canceling...\n");
        setState(DS_WorkflowDef::AQD_STATE_IDLE);
        return;
    }
}

void WorkflowAutomaticQualifiedDischarge::BMSChanged_AQDProgress(const DS_McuDef::BMSDigests &bmsDigests, const DS_McuDef::BMSDigests &prevBmsDigests)
{
    LOG_INFO("AQD in progress...\n");
    logLiveBMS(bmsDigests);
    batteryPackMissing = (env->ds.alertAction->isActivated("GeneralI2CFault", "ALARM_I2C_GENERAL_BATTERY_MANAGEMENT_SYSTEM_A") ||
                          env->ds.alertAction->isActivated("GeneralI2CFault", "ALARM_I2C_GENERAL_BATTERY_MANAGEMENT_SYSTEM_B"));

    // permanent failure checks will be done in workflow battery
    bool aqdCanRun = (!batteryPackMissing);

    if (!aqdCanRun)
    {
        // Termination conditions
        // normal discharge fail conditions are checked in WorkflowBattery
        // AQD specific fail condition is aqdCanRun
        LOG_WARNING("Terminating AQD! Reasons : BatteryPackMissing : %s, BatteryPermanentFailures : %s\n", batteryPackMissing?"TRUE":"FALSE", batteryPermanentFailures?"TRUE":"FALSE");
        env->ds.workflowAction->actWorkflowBatteryAbort();
    }
}

int WorkflowAutomaticQualifiedDischarge::getState()
{
    return getAutomaticQualifiedDischargeStatus().state;
}

void WorkflowAutomaticQualifiedDischarge::setStateSynch(int newState)
{
    DS_WorkflowDef::AutomaticQualifiedDischargeStatus status = getAutomaticQualifiedDischargeStatus();
    status.state = (DS_WorkflowDef::AutomaticQualifiedDischargeState)newState;
    env->ds.workflowData->setAutomaticQualifiedDischargeStatus(status);
}

bool WorkflowAutomaticQualifiedDischarge::isSetStateReady()
{
    return (getState() != DS_WorkflowDef::AQD_STATE_INACTIVE);
}

void WorkflowAutomaticQualifiedDischarge::setStatusMessage(QString message)
{
    DS_WorkflowDef::AutomaticQualifiedDischargeStatus status = getAutomaticQualifiedDischargeStatus();
    status.message = message;
    env->ds.workflowData->setAutomaticQualifiedDischargeStatus(status);
}

void WorkflowAutomaticQualifiedDischarge::processState()
{
    DS_WorkflowDef::AutomaticQualifiedDischargeState state = (DS_WorkflowDef::AutomaticQualifiedDischargeState)getState();

    switch(state)
    {
    case DS_WorkflowDef::AQD_STATE_INACTIVE:
    {
        break;
    }

    case DS_WorkflowDef::AQD_STATE_IDLE:
    {
        LOG_INFO("[AQD_STATE_IDLE]\n");
        if (env->ds.alertAction->isActivated("AutomaticQualifiedDischargeInProgress", "", true))
        {
            env->ds.alertAction->deactivate("AutomaticQualifiedDischargeInProgress");
        }
        reset();
        signalBMSChanged_func = &WorkflowAutomaticQualifiedDischarge::BMSChanged_Idle;
        break;
    }

    case DS_WorkflowDef::AQD_STATE_AQD_QUEUED:
    {
        QString msg = "";
        msg = QString().asprintf("Battery [%s]", getBatteryName(batteryIndex).CSTR());
        setStatusMessage(msg);
        // waiting for AQD start conditions
        LOG_INFO("[AQD_STATE_AQD_QUEUED] target battery = [%s]\n", getBatteryName(batteryIndex).CSTR());
        signalBMSChanged_func = &WorkflowAutomaticQualifiedDischarge::BMSChanged_AQDQueued;
        break;
    }

    case DS_WorkflowDef::AQD_STATE_START:
    {
        LOG_INFO("[AQD_STATE_START]\n");
        env->ds.alertAction->activate("AutomaticQualifiedDischargeInProgress", getBatteryName(batteryIndex));
        handleSubAction(DS_WorkflowDef::AQD_STATE_DISCHARGE_PROGRESS, DS_WorkflowDef::AQD_STATE_DISCHARGE_DONE, DS_WorkflowDef::AQD_STATE_DISCHARGE_FAIL);
        // -1 means down to EDV2 (qualified discharge)
        env->ds.workflowAction->actWorkflowBatteryChargeToStart(batteryIndex, -1, guidSubAction);
        break;
    }

    case DS_WorkflowDef::AQD_STATE_DISCHARGE_PROGRESS:
    {
        LOG_INFO("[AQD_STATE_DISCHARGE_PROGRESS]\n");
        signalBMSChanged_func = &WorkflowAutomaticQualifiedDischarge::BMSChanged_AQDProgress;
        break;
    }

    case DS_WorkflowDef::AQD_STATE_DISCHARGE_DONE:
    {
        LOG_INFO("[AQD_STATE_DISCHARGE_DONE]\n");
        setState(DS_WorkflowDef::AQD_STATE_DONE);
        break;
    }

    case DS_WorkflowDef::AQD_STATE_DISCHARGE_FAIL:
    {
        LOG_INFO("[AQD_STATE_DISCHARGE_FAIL]\n");
        setState(DS_WorkflowDef::AQD_STATE_IDLE);
        break;
    }

    case DS_WorkflowDef::AQD_STATE_DONE:
    {
        LOG_INFO("[AQD_STATE_DISCHARGE_DONE]\n");
        setState(DS_WorkflowDef::AQD_STATE_IDLE);
        setStatusMessage("");
        break;
    }

    default:
    {
        break;
    }
    }
}

void WorkflowAutomaticQualifiedDischarge::reset()
{
    batteryIndex = -1;
    signalBMSChanged_func = NULL;
    batteryPackMissing = false;
    batteryPermanentFailures = false;
    batteryARequiresAQD = false;
    batteryBRequiresAQD = false;
}

QString WorkflowAutomaticQualifiedDischarge::getBatteryName(int batteryIdx)
{
    return (batteryIdx == 0) ? "A" : "B";
}

int WorkflowAutomaticQualifiedDischarge::getOtherBatteryIndex(int batteryIdx)
{
    return (batteryIdx == 0) ? 1 : 0;
}
