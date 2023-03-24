#include "Apps/AppManager.h"
#include "WorkflowManualQualifiedDischarge.h"
#include "DataServices/Workflow/DS_WorkflowData.h"
#include "DataServices/Workflow/DS_WorkflowAction.h"
#include "DataServices/Alert/DS_AlertAction.h"
#include "DataServices/Device/DS_DeviceAction.h"
#include "DataServices/Device/DS_DeviceData.h"
#include "DataServices/Mcu/DS_McuData.h"
#include "DataServices/Mcu/DS_McuAction.h"
#include "DataServices/Capabilities/DS_Capabilities.h"
#include "DataServices/System/DS_SystemAction.h"
#include "DataServices/CfgLocal/DS_CfgLocal.h"
#include "Common/ImrParser.h"


WorkflowManualQualifiedDischarge::WorkflowManualQualifiedDischarge(QObject *parent, EnvGlobal *env_) :
    ActionBaseExt(parent, env_)
{
    envLocal = new EnvLocal("DS_Workflow-ManualQualifiedDischarge", "WORKFLOW_MANUAL_QUALIFIED_DISCHARGE");

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });

    reset();
}

WorkflowManualQualifiedDischarge::~WorkflowManualQualifiedDischarge()
{
    delete envLocal;
}

void WorkflowManualQualifiedDischarge::slotAppInitialised()
{
    DS_WorkflowDef::ManualQualifiedDischargeStatus mqdStatus;
    mqdStatus.state = DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_READY;
    mqdStatus.message = "";
    env->ds.workflowData->setManualQualifiedDischargeStatus(mqdStatus);

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_BMSDigests, this, [=](const DS_McuDef::BMSDigests &bmsDigests, const DS_McuDef::BMSDigests &prevBmsDigests) {
        if (signalBMSChanged_func != NULL)
        {
            (this->*signalBMSChanged_func)(bmsDigests, prevBmsDigests);
        }
    });

    processState();
}

DS_WorkflowDef::ManualQualifiedDischargeStatus WorkflowManualQualifiedDischarge::getManualQualifiedDischargeStatus()
{
   return env->ds.workflowData->getManualQualifiedDischargeStatus();
}

int WorkflowManualQualifiedDischarge::getState()
{
    return getManualQualifiedDischargeStatus().state;
}

QString WorkflowManualQualifiedDischarge::getStateStr(int state)
{
    return ImrParser::ToImr_ManualQualifiedDischargeState((DS_WorkflowDef::ManualQualifiedDischargeState)state);
}

void WorkflowManualQualifiedDischarge::setStateSynch(int newState)
{
    DS_WorkflowDef::ManualQualifiedDischargeStatus mqdStatus = getManualQualifiedDischargeStatus();
    mqdStatus.state = (DS_WorkflowDef::ManualQualifiedDischargeState)newState;

    env->ds.workflowData->setManualQualifiedDischargeStatus(mqdStatus);
}

bool WorkflowManualQualifiedDischarge::isSetStateReady()
{
    if (getState() == DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_INACTIVE)
    {
        return false;
    }
    return true;
}

void WorkflowManualQualifiedDischarge::setStatusMessage(QString message)
{
    DS_WorkflowDef::ManualQualifiedDischargeStatus mqdStatus = getManualQualifiedDischargeStatus();
    mqdStatus.message = message;

    env->ds.workflowData->setManualQualifiedDischargeStatus(mqdStatus);
}

void WorkflowManualQualifiedDischarge::reset()
{
    batteryIndex = 0;
    dischargeMethod = DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_METHOD_SELF;
    digestCount = 0;
    signalBMSChanged_func = NULL;
}

void WorkflowManualQualifiedDischarge::BMSChanged_ExternalLoad_Discharge_Progress(const DS_McuDef::BMSDigests &bmsDigests, const DS_McuDef::BMSDigests &prevBmsDigests)
{
    // LEAVE MAIN ON
    // LEAVE OTHER BATTERY ON
    // TARGET BATTERY FET ON
    // MONITOR VDQ and EDV
    QString msg = "";

    DS_WorkflowDef::ManualQualifiedDischargeState nextState = DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_LOAD_DISCHARGING_PROGRESS;

    LOG_INFO("Live BMS Data for battery [%s]:\n", getBatteryName(batteryIndex).CSTR());
    LOG_INFO("%s\n", Util::qVarientToJsonData((ImrParser::ToImr_BMSDigest(bmsDigests[batteryIndex])), false).CSTR());

    digestCount++;

    bool batteryPermanentFailures = bmsDigests[batteryIndex].pfStatusBytes.contains(QRegularExpression("[^0 ]"));
    if (batteryPermanentFailures)
    {
        LOG_ERROR("[%s] TargetBattery : %s - Battery has permanent failures!\n", getStateStr(getState()).CSTR(),getBatteryName(batteryIndex).CSTR());
        msg = QString().asprintf("[%s]\n TargetBattery : %s - Battery has permanent failures!", getStateStr(getState()).CSTR(), getBatteryName(batteryIndex).CSTR());
        nextState = DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_LOAD_DISCHARGING_FAIL;
    }
    else
    {
        if (digestCount > 1)
        {
            if (bmsDigests[batteryIndex].gaugingStatus.endOfDischargeVoltageLevel2)
            {
                // lets not let it discharge below EDV2
                LOG_INFO("[%s] TargetBattery : %s - EDV2 reached\n", getStateStr(getState()).CSTR(), getBatteryName(batteryIndex).CSTR());
                msg += QString().asprintf("\n Discharge DONE: EDV2 reached");
                nextState  = DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_RECONNECT_BATTERY;
            }
            else if (!bmsDigests[batteryIndex].gaugingStatus.dischargeQualifiedForLearning)
            {
                LOG_ERROR("[%s] TargetBattery : %s - VDQ unset during Qualified Discharge\n", getStateStr(getState()).CSTR(), getBatteryName(batteryIndex).CSTR());
                msg += QString().asprintf("\n FAILED: VDQ unset during Qualified Discharge");
                nextState = DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_LOAD_DISCHARGING_FAIL;
            }
        }
    }

    setStatusMessage(msg);

    if (nextState != DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_LOAD_DISCHARGING_PROGRESS)
    {
        setState(nextState);
        digestCount = 0;
        signalBMSChanged_func = NULL;
    }
}

void WorkflowManualQualifiedDischarge::processState()
{
    DS_WorkflowDef::ManualQualifiedDischargeState state = (DS_WorkflowDef::ManualQualifiedDischargeState)getState();

    LOG_INFO("[%s]: Target Battery [%s]\n", getStateStr(state).CSTR(), getBatteryName(batteryIndex).CSTR());
    switch (state)
    {
    case DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_INACTIVE:
        break;
    case DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_READY:
        {
            // clear error
            setStatusMessage("");
            reset();
        }
        break;
    case DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_PREPARATION:
    {
        DS_McuDef::BMSDigests digests = env->ds.mcuData->getBMSDigests();
        LOG_INFO("[%s]: Target Battery [%d] Charged=%d%%, MaxError=%d%%\n", getStateStr(state).CSTR(), batteryIndex, digests[batteryIndex].sbsStatus.relativeStateOfCharge, digests[batteryIndex].sbsStatus.maxError);

        // don't let screen sleep during MDQ
        env->ds.systemAction->actSetScreenSleepTime(0);

        // Check if the target battery has any permanent failures - abort if any
        // pfStatusBytes is of a type QString. Example "00 00 00 00". Any non-zero represents a permanent failure, but due to the string also containing spaces, we ignore spaces as well
        bool batteryPermanentFailures = digests[batteryIndex].pfStatusBytes.contains(QRegularExpression("[^0 ]"));

        if (batteryPermanentFailures)
        {
            LOG_ERROR("Battery %s: MANUAL_QUALIFIED_DISCHARGE_STATE_PREPARATION: Battery has permanent failures!\n", (batteryIndex == 0 ? "A" : "B"));

            setStatusMessage("Battery Permanent Failure Found.");
            setState(DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_FAILED);
            return;
        }

        QString bmsI2CFaultAlarmName = (batteryIndex == 0) ? "ALARM_I2C_GENERAL_BATTERY_MANAGEMENT_SYSTEM_A" : "ALARM_I2C_GENERAL_BATTERY_MANAGEMENT_SYSTEM_B";
        if (env->ds.alertAction->isActivated("GeneralI2CFault", bmsI2CFaultAlarmName))
        {
            actStatusBuf.state = DS_ACTION_STATE_INVALID_STATE;
            actStatusBuf.err = "GeneralI2CFault: " + bmsI2CFaultAlarmName;
            setStatusMessage("Battery is missing!");
            setState(DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_FAILED);
            return;
        }

        DS_McuDef::PowerStatus powerStatus = env->ds.mcuData->getPowerStatus();
        if (powerStatus.isAcPowered)
        {
            // MDQ is good to start
            setState(DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_START);
        }
        else
        {
            setState(DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_USER_POWER_CONNECT_WAITING);
            return;
        }
    }
        break;
    case DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_USER_POWER_CONNECT_WAITING:
        // Wait for user interaction
        break;
    case DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_USER_POWER_CONNECT_CONFIRMED:
    {
        // ensure power is connected
        DS_McuDef::PowerStatus powerStatus = env->ds.mcuData->getPowerStatus();
        if (powerStatus.isAcPowered)
        {
            setState(DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_START);
            return;
        }
        else
        {
            // shouldn't reach here (qml doesn't allow) but safety measure
            LOG_ERROR("[%s]: Power is NOT connected.\n", getStateStr(getState()).CSTR());
            setStatusMessage("Power is NOT connected.");
            setState(DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_FAILED);
            return;
        }
        break;
    }
    case DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_START:
    {
        handleSubAction(DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_CHARGING_FULL_PROGRESS, DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_CHARGING_FULL_DONE, DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_CHARGING_FULL_FAIL);
        env->ds.workflowAction->actWorkflowBatteryChargeToStart(batteryIndex, 100, guidSubAction);
        break;
    }
    case DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_CHARGING_FULL_PROGRESS:
    {
        break;
    }
    case DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_CHARGING_FULL_FAIL:
    {
        LOG_WARNING("[%s]: Target Battery [%d], Error = %s\n", getStateStr(state).CSTR(), batteryIndex, actStatusBuf.err.CSTR());
        setStatusMessage(actStatusBuf.err);
        setState(DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_FAILED);
        break;
    }
    case DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_CHARGING_FULL_DONE:
    {
        setState(DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_DISCHARGING_START);
        break;
    }

    case DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_DISCHARGING_START:
    {
        if (dischargeMethod == DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_METHOD_EXTERNAL_LOAD)
        {
            setState(DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_CONNECT_EXTERNAL_LOAD);
        }
        else
        {
            handleSubAction(DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_DISCHARGING_PROGRESS, DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_DISCHARGING_DONE, DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_DISCHARGING_FAIL);
            env->ds.workflowAction->actWorkflowBatteryChargeToStart(batteryIndex, -1, guidSubAction);
        }
        break;
    }
    case DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_DISCHARGING_PROGRESS:
    {
        break;
    }
    case DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_DISCHARGING_FAIL:
    {
        LOG_WARNING("[%s]: Target Battery [%d], Error = %s\n", getStateStr(state).CSTR(), batteryIndex, actStatusBuf.err.CSTR());
        setStatusMessage(actStatusBuf.err);
        setState(DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_FAILED);
        break;
    }
    case DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_DISCHARGING_DONE:
    {
        setState(DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_DONE);
        break;
    }

    case DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_CONNECT_EXTERNAL_LOAD:
    {
        // wait for user interaction
        break;
    }
    case DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_LOAD_DISCHARGING_PROGRESS:
    {
        digestCount = 0;
        signalBMSChanged_func = &WorkflowManualQualifiedDischarge::BMSChanged_ExternalLoad_Discharge_Progress;
        break;
    }
    case DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_LOAD_DISCHARGING_FAIL:
    {
        DS_WorkflowDef::ManualQualifiedDischargeStatus status = getManualQualifiedDischargeStatus();
        actStatusBuf.state = DS_ACTION_STATE_INTERNAL_ERR;
        actStatusBuf.err = "Load Discharging Failed: " + status.message;

        setState(DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_DONE);
        break;
    }
    case DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_LOAD_DISCHARGING_DONE:
    {
        setState(DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_DONE);
        break;
    }

    case DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_RECONNECT_BATTERY:
    {
        // disable battery discharge until batteries are connected back to unit
        env->ds.mcuAction->actBatteryToUnit(batteryIndex, false);
        // user interaction
        break;
    }
    case DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_RECONNECT_BATTERY_CONFIRMED:
    {
        // turn BMS fet back on
        env->ds.mcuAction->actBatteryToUnit(batteryIndex, true);
        setState(DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_DONE);
        break;
    }
    case DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_FAILED:
    {
        LOG_WARNING("[%s] Target Battery [%s]: %s\n", getStateStr(getState()).CSTR(), getBatteryName(batteryIndex).CSTR(), actStatusBuf.err.CSTR());
        DS_WorkflowDef::ManualQualifiedDischargeStatus status = getManualQualifiedDischargeStatus();
        actStatusBuf.state = DS_ACTION_STATE_INTERNAL_ERR;
        actStatusBuf.err = "Failed: " + status.message;
        setState(DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_DONE);
        break;
    }
    case DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_ABORTED:
    {
        LOG_WARNING("[%s] Target Battery [%s]: %s\n", getStateStr(getState()).CSTR(), getBatteryName(batteryIndex).CSTR(), actStatusBuf.err.CSTR());
        actStatusBuf.state = DS_ACTION_STATE_USER_ABORT;
        actStatusBuf.err = "User Aborted";
        setStatusMessage(actStatusBuf.err);
        setState(DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_DONE);
        break;
    }
    case DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_DONE:
    {
        LOG_INFO("[%s] Target Battery [%s]: %s\n", getStateStr(getState()).CSTR(), getBatteryName(batteryIndex).CSTR(), actStatusBuf.err.CSTR());
        actionCompleted(actStatusBuf);
        env->ds.alertAction->deactivate("ManualQualifiedDischargeInProgress");
        // waiting for user interaction
        break;
    }
    default:
        LOG_ERROR("[%d]: MANUAL_QUALIFIED_DISCHARGE_STATE: Invalid State (%d)\n", batteryIndex, state);
        break;
    }
}

DataServiceActionStatus WorkflowManualQualifiedDischarge::actManualQualifiedDischargeStart(int batteryIdx, DS_WorkflowDef::ManualQualifiedDischargeMethod methodType, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "ManualQualifiedDischargeStart");
    DS_WorkflowDef::ManualQualifiedDischargeState state = (DS_WorkflowDef::ManualQualifiedDischargeState)getState();

    if (state != DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_READY)
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "Invalid State";
        actionStarted(status);
        return status;
    }

    if (batteryIdx >= POWER_BATTERY_INDEX_MAX)
    {
        status.state = DS_ACTION_STATE_BAD_REQUEST;
        status.err = "Bad Battery Index";
        actionStarted(status);
        return status;
    }

    QString bmsI2CFaultAlarmName = (batteryIdx == 0) ? "ALARM_I2C_GENERAL_BATTERY_MANAGEMENT_SYSTEM_A" : "ALARM_I2C_GENERAL_BATTERY_MANAGEMENT_SYSTEM_B";

    if (env->ds.alertAction->isActivated("GeneralI2CFault", bmsI2CFaultAlarmName))
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "GeneralI2CFault: " + bmsI2CFaultAlarmName;
        actionStarted(status);
        return status;
    }

    batteryIndex = batteryIdx;
    dischargeMethod = methodType;
    actStatusBuf = status;
    actStatusBuf.state = DS_ACTION_STATE_STARTED;

    actionStarted(actStatusBuf);

    setState(DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_PREPARATION);
    env->ds.alertAction->activate("ManualQualifiedDischargeInProgress", getBatteryName(batteryIndex));

    return actStatusBuf;
}

DataServiceActionStatus WorkflowManualQualifiedDischarge::actManualQualifiedDischargeResume(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "ManualQualifiedDischargeResume");

    DS_WorkflowDef::ManualQualifiedDischargeState state = (DS_WorkflowDef::ManualQualifiedDischargeState)getState();
    DS_WorkflowDef::ManualQualifiedDischargeState newState;

    switch (state)
    {
    case DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_USER_POWER_CONNECT_WAITING:
        newState = DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_USER_POWER_CONNECT_CONFIRMED;
        break;
    case DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_CONNECT_EXTERNAL_LOAD:
        newState = DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_LOAD_DISCHARGING_PROGRESS;
        break;
    case DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_RECONNECT_BATTERY:
        newState = DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_RECONNECT_BATTERY_CONFIRMED;
        break;
    case DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_DONE:
        newState = DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_READY;
        break;
    default:
        status.err = "Invalid State: " + ImrParser::ToImr_ManualQualifiedDischargeState(state);
        status.state = DS_ACTION_STATE_INVALID_STATE;
        actionStarted(status);
        return status;
    }

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);

    setState(newState);

    status.state = DS_ACTION_STATE_COMPLETED;
    actionCompleted(status);

    return status;
}

DataServiceActionStatus WorkflowManualQualifiedDischarge::actManualQualifiedDischargeAbort(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "ManualQualifiedDischargeAbort");

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);

    if ((getState() == DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_CHARGING_FULL_PROGRESS) ||
            (getState() == DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_DISCHARGING_PROGRESS))
    {
        // handle subaction will change state
        env->ds.workflowAction->actWorkflowBatteryAbort();
    }
    else if (getState() == DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_LOAD_DISCHARGING_PROGRESS)
    {
        setState(DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_RECONNECT_BATTERY);
    }
    else
    {
        setState(DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_ABORTED);
    }

    status.state = DS_ACTION_STATE_COMPLETED;
    actionCompleted(status);

    return status;
}

QString WorkflowManualQualifiedDischarge::getBatteryName(int batteryIdx)
{
    return (batteryIdx == 0) ? "A" : "B";
}
