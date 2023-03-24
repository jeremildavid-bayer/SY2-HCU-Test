#include "Apps/AppManager.h"
#include "WorkflowBattery.h"
#include "DataServices/Workflow/DS_WorkflowData.h"
#include "DataServices/Alert/DS_AlertAction.h"
#include "DataServices/Mcu/DS_McuData.h"
#include "DataServices/Mcu/DS_McuAction.h"
#include "DataServices/System/DS_SystemAction.h"
#include "DataServices/CfgLocal/DS_CfgLocal.h"


WorkflowBattery::WorkflowBattery(QObject *parent, EnvGlobal *env_) :
    ActionBaseExt(parent, env_)
{
    envLocal = new EnvLocal("DS_Workflow-WorkflowBattery", "WORKFLOW_BATTERY");

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });

    // only initializing battery index here not in reset function. Please refer to comments in reset function
    batteryIndex = 0;

    reset();
}

WorkflowBattery::~WorkflowBattery()
{
    delete envLocal;
}

void WorkflowBattery::slotAppInitialised()
{
    DS_WorkflowDef::WorkflowBatteryStatus workflowBatteryStatus;
    workflowBatteryStatus.state = DS_WorkflowDef::WORKFLOW_BATTERY_STATE_IDLE;
    workflowBatteryStatus.message = "";
    env->ds.workflowData->setWorkflowBatteryStatus(workflowBatteryStatus);

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_BMSDigests, this, [=](const DS_McuDef::BMSDigests &bmsDigests, const DS_McuDef::BMSDigests &prevBmsDigests) {
        if (signalBMSChanged_func != NULL)
        {
            (this->*signalBMSChanged_func)(bmsDigests, prevBmsDigests);
        }
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_PowerStatus, this, [=](const DS_McuDef::PowerStatus &powerStatus, const DS_McuDef::PowerStatus &prevPowerStatus_) {
        DS_WorkflowDef::WorkflowBatteryState state = (DS_WorkflowDef::WorkflowBatteryState)getState();
        switch (state)
        {
        case DS_WorkflowDef::WORKFLOW_BATTERY_STATE_CHARGE_PROGRESS:
        case DS_WorkflowDef::WORKFLOW_BATTERY_STATE_CHARGE_PREPARATION:
        {
            if (!powerStatus.isAcPowered)
            {
                QString abortMsg = QString().asprintf("[%s] FAILED: Power is disconnected.", getStateStr(state).CSTR());
                setStatusMessage(abortMsg);
                setState(DS_WorkflowDef::WORKFLOW_BATTERY_STATE_CHARGE_FAIL);
            }
            break;
        }
        default:
            break;
        }
    });

    processState();
}

DataServiceActionStatus WorkflowBattery::actBatteryAction(int batteryIdx, QString action, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "WWorkflowBatteryAction");
    if (action == "Baseboard Test")
    {
        actStatusBuf = status;
        actStatusBuf.state = DS_ACTION_STATE_STARTED;

        actionStarted(actStatusBuf);

        setState(DS_WorkflowDef::WORKFLOW_BATTERY_STATE_BASEBOARD_VOUTTEST_ON);

        return actStatusBuf;
    }
    else
    {
        bool isNumber = false;
        int targetCharge = action.toInt(&isNumber);
        if (isNumber)
        {
            return actBatteryChargeToStart(batteryIdx, targetCharge, actGuid);
        }
    }

    LOG_ERROR("Action %s is not supported\n" , action.CSTR());
    status.err = "Invalid action: " + action;
    status.state = DS_ACTION_STATE_INTERNAL_ERR;
    actionStarted(status);
    return status;
}


DataServiceActionStatus WorkflowBattery::actAbort(QString actGuid)
{
    DS_WorkflowDef::WorkflowBatteryState state = (DS_WorkflowDef::WorkflowBatteryState)getState();
    DS_WorkflowDef::WorkflowBatteryState newState;
    DataServiceActionStatus status = actionInit(actGuid, "WorkflowBatteryAbort");

    switch(state)
    {
    case DS_WorkflowDef::WORKFLOW_BATTERY_STATE_CHARGE_PREPARATION:
    case DS_WorkflowDef::WORKFLOW_BATTERY_STATE_CHARGE_PROGRESS:
    case DS_WorkflowDef::WORKFLOW_BATTERY_STATE_CHARGE_DONE:
    {
        newState = DS_WorkflowDef::WORKFLOW_BATTERY_STATE_CHARGE_ABORT;
        break;
    }
    case DS_WorkflowDef::WORKFLOW_BATTERY_STATE_DISCHARGE_PREPARATION:
    case DS_WorkflowDef::WORKFLOW_BATTERY_STATE_DISCHARGE_PROGRESS:
    case DS_WorkflowDef::WORKFLOW_BATTERY_STATE_DISCHARGE_DONE:
    {
        newState = DS_WorkflowDef::WORKFLOW_BATTERY_STATE_DISCHARGE_ABORT;
        break;
    }
    // baseboard tests. reset all baseboard / bms
    case DS_WorkflowDef::WORKFLOW_BATTERY_STATE_BASEBOARD_VOUTTEST_OFF:
    case DS_WorkflowDef::WORKFLOW_BATTERY_STATE_BASEBOARD_VOUTTEST_ON:
    {
        digestCount = 0;
        signalBMSChanged_func = NULL;
        env->ds.mcuAction->actMainToUnit(true);
        env->ds.mcuAction->actChargeBattery(batteryIndex, true);
        env->ds.mcuAction->actChargeBattery(getOtherBatteryIndex(batteryIndex), true);
        newState = DS_WorkflowDef::WORKFLOW_BATTERY_STATE_ABORT;
        break;
    }
    default:
    {
        status.err = "Invalid State: " + ImrParser::ToImr_WorkflowBatteryState(state);
        status.state = DS_ACTION_STATE_INVALID_STATE;
        actionStarted(status);
        return status;
    }
    }

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);

    setState(newState);

    status.state = DS_ACTION_STATE_COMPLETED;
    actionCompleted(status);

    return status;
}

bool WorkflowBattery::basicFailCheck(DataServiceActionStatus &status, int batteryIdx)
{
    DS_WorkflowDef::WorkflowBatteryState state = (DS_WorkflowDef::WorkflowBatteryState)getState();
    if (state != DS_WorkflowDef::WORKFLOW_BATTERY_STATE_IDLE)
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "Invalid State";
        actionStarted(status);
        return true;
    }

    if (batteryIdx >= POWER_BATTERY_INDEX_MAX)
    {
        status.state = DS_ACTION_STATE_BAD_REQUEST;
        status.err = "Bad Battery Index";
        actionStarted((status));
        return true;
    }

    QString bmsI2CFaultAlarmName = (batteryIdx == 0) ? "ALARM_I2C_GENERAL_BATTERY_MANAGEMENT_SYSTEM_A" : "ALARM_I2C_GENERAL_BATTERY_MANAGEMENT_SYSTEM_B";
    if (env->ds.alertAction->isActivated("GeneralI2CFault", bmsI2CFaultAlarmName))
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "GeneralI2CFault: " + bmsI2CFaultAlarmName;
        actionStarted(status);
        return true;
    }

    return false;
}

DataServiceActionStatus WorkflowBattery::actDischargeBatteryStart(int batteryIdx, int target, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "DischargeBatteryStart");

    if (basicFailCheck(status, batteryIdx))
    {
        return status;
    }

    const DS_McuDef::BMSDigests curDigests = env->ds.mcuData->getBMSDigests();
    if (curDigests[batteryIdx].sbsStatus.relativeStateOfCharge < target)
    {
        status.state = DS_ACTION_STATE_BAD_REQUEST;
        status.err = "Battery already lower than target charge";
        actionStarted(status);
        return status;
    }

    batteryIndex = batteryIdx;
    targetCharge = target;

    actStatusBuf = status;
    actStatusBuf.state = DS_ACTION_STATE_STARTED;

    actionStarted(actStatusBuf);

    setState(DS_WorkflowDef::WORKFLOW_BATTERY_STATE_DISCHARGE_PREPARATION);

    return actStatusBuf;
}

DataServiceActionStatus WorkflowBattery::actChargeBatteryStart(int batteryIdx, int target, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "ChargeBatteryStart");

    if (basicFailCheck(status, batteryIdx))
    {
        return status;
    }

    const DS_McuDef::BMSDigests curDigests = env->ds.mcuData->getBMSDigests();
    if (curDigests[batteryIdx].sbsStatus.relativeStateOfCharge > target)
    {
        status.state = DS_ACTION_STATE_BAD_REQUEST;
        status.err = "Battery already higher than target charge";
        actionStarted(status);
        return status;
    }

    batteryIndex = batteryIdx;
    targetCharge = target;

    actStatusBuf = status;
    actStatusBuf.state = DS_ACTION_STATE_STARTED;

    actionStarted(actStatusBuf);

    setState(DS_WorkflowDef::WORKFLOW_BATTERY_STATE_CHARGE_PREPARATION);

    return actStatusBuf;
}

DataServiceActionStatus WorkflowBattery::actBatteryChargeToStart(int batteryIdx, int target, QString actGuid)
{
    const DS_McuDef::BMSDigests curDigests = env->ds.mcuData->getBMSDigests();
    if (target < curDigests[batteryIdx].sbsStatus.relativeStateOfCharge)
    {
        return actDischargeBatteryStart(batteryIdx, target, actGuid);
    }
    return actChargeBatteryStart(batteryIdx, target, actGuid);
}

void WorkflowBattery::BMSChanged_Idle(const DS_McuDef::BMSDigests &bmsDigests, const DS_McuDef::BMSDigests &prevBmsDigests)
{
    // Idle, don't flood logging
    // restore BMS All FET

    // only force this if we are not in service mode
    DS_SystemDef::StatePath statePath = env->ds.systemData->getStatePath();
    if (statePath != DS_SystemDef::STATE_PATH_BUSY_SERVICING)
    {
        for (int i = 0; i < POWER_BATTERY_INDEX_MAX; i++)
        {
            if (!bmsDigests[i].manufacturingStatus.allFetAction)
            {
                LOG_WARNING("[%s] TargetBattery : %s - Battery has all Fet Action disabled. Enabling...\n", getStateStr(getState()).CSTR(), getBatteryName(i).CSTR());
                env->ds.mcuAction->actBatteryToUnit(i, true);
            }
        }
    }
}

void WorkflowBattery::BMSChanged_Charge_Preparation(const DS_McuDef::BMSDigests &bmsDigests, const DS_McuDef::BMSDigests &prevBmsDigests)
{
    LOG_INFO("WorkflowBattery_STATE_CHARGE_PREPARATION\n");
    digestCount++;
    QString msg = "";
    msg = QString().asprintf("[%s]\n TargetBattery : %s, Target Charge : %d", getStateStr(getState()).CSTR(), getBatteryName(batteryIndex).CSTR(), targetCharge);

    if (checkCommonBatteryErrors(bmsDigests[batteryIndex], &msg))
    {
        LOG_ERROR("%s\n", msg.CSTR());
        actStatusBuf.state = DS_ACTION_STATE_INTERNAL_ERR;
        actStatusBuf.err = msg;
        setStatusMessage(msg);
        setState(DS_WorkflowDef::WORKFLOW_BATTERY_STATE_CHARGE_FAIL);
        return;
    }

    // ignore first digest in case it doesn't have updated BMS
    if (digestCount > 1)
    {
        // battery is capable of charging - check all FETS
        if (!bmsDigests[batteryIndex].manufacturingStatus.allFetAction)
        {
            LOG_WARNING("[%s] TargetBattery : %s - Battery has all Fet Action disabled. Enabling...\n", getStateStr(getState()).CSTR(), getBatteryName(batteryIndex).CSTR());
            env->ds.mcuAction->actBatteryToUnit(batteryIndex, true);
        }
        // disable other battery's fet
        env->ds.mcuAction->actBatteryToUnit(getOtherBatteryIndex(batteryIndex), false);
        setState(DS_WorkflowDef::WORKFLOW_BATTERY_STATE_CHARGE_PROGRESS);
    }
    setStatusMessage(msg);
}

void WorkflowBattery::BMSChanged_Charge_Progress(const DS_McuDef::BMSDigests &bmsDigests, const DS_McuDef::BMSDigests &prevBmsDigests)
{
    LOG_INFO("WorkflowBattery_STATE_CHARGE_PROGRESS\n");
    digestCount++;
    QString msg = "";
    msg = QString().asprintf("[%s]\n TargetBattery : %s, Target Charge : %d", getStateStr(getState()).CSTR(), getBatteryName(batteryIndex).CSTR(), targetCharge);
    DS_WorkflowDef::WorkflowBatteryState nextState = DS_WorkflowDef::WORKFLOW_BATTERY_STATE_INVALID;

    if (checkCommonBatteryErrors(bmsDigests[batteryIndex], &msg))
    {
        DS_WorkflowDef::WorkflowBatteryStatus status = getWorkflowBatteryStatus();
        LOG_ERROR("%s\n", msg.CSTR());
        actStatusBuf.state = DS_ACTION_STATE_INTERNAL_ERR;
        actStatusBuf.err = msg;
        setStatusMessage(msg);
        setState(DS_WorkflowDef::WORKFLOW_BATTERY_STATE_CHARGE_FAIL);
        return;
    }

    if (digestCount > 1)
    {
        // update maximum charge current
        maxChargeCurrent = qMax(maxChargeCurrent, (int)bmsDigests[batteryIndex].sbsStatus.current);
        double chargeDropPerc = ((double)bmsDigests[batteryIndex].sbsStatus.current / (double)maxChargeCurrent) * 100.0f;

        // what's the best condition to use from below? (there are more we can use)
        //if (bmsDigests[batteryIndex].operationStatus.chargingDisabled)
        //if (bmsDigests[batteryIndex].gaugingStatus.fullyCharged)
        if (bmsDigests[batteryIndex].gaugingStatus.terminateCharge)
        {
            LOG_INFO("[%s] TargetBattery : %s - fully charged (BMS fully charged flag)\n", getStateStr(getState()).CSTR(), getBatteryName(batteryIndex).CSTR());
            msg += QString().asprintf("\n DONE: Fully charged (BMS fully charged flag)");
            nextState = DS_WorkflowDef::WORKFLOW_BATTERY_STATE_CHARGE_DONE;
        }
        else if ((bmsDigests[batteryIndex].sbsStatus.current == 0) || chargeDropPerc <= 70)
        {
            // when max error is too high, relative state of charge cannot be trusted. Check charging current and see if it dropped to 70% of max charging current
            // if current is 0 while the battery is charging, consider it fully charged
            LOG_INFO("[%s] TargetBattery : %s - fully charged (charging rate dropped... is max error high?)\n", getStateStr(getState()).CSTR(), getBatteryName(batteryIndex).CSTR());
            msg += QString().asprintf("\n DONE: Fully charged (charging rate dropped... is max error high?)");
            nextState = DS_WorkflowDef::WORKFLOW_BATTERY_STATE_CHARGE_DONE;
        }
        else if (bmsDigests[batteryIndex].sbsStatus.relativeStateOfCharge >= targetCharge)
        {
            LOG_INFO("[%s] TargetBattery : %s - target Charge reached\n", getStateStr(getState()).CSTR(), getBatteryName(batteryIndex).CSTR());
            msg += QString().asprintf("\n DONE: Target Charge reached");
            nextState = DS_WorkflowDef::WORKFLOW_BATTERY_STATE_CHARGE_DONE;
        }
    }
    setStatusMessage(msg);
    if (nextState != DS_WorkflowDef::WORKFLOW_BATTERY_STATE_INVALID)
    {
        setState(nextState);
    }
}

void WorkflowBattery::BMSChanged_Discharge_Preparation(const DS_McuDef::BMSDigests &bmsDigests, const DS_McuDef::BMSDigests &prevBmsDigests)
{
    LOG_INFO("WorkflowBattery_STATE_DISCHARGE_PREPARATION\n");
    digestCount++;
    QString msg = "";
    msg = QString().asprintf("[%s]\n TargetBattery : %s, Target Charge : %d", getStateStr(getState()).CSTR(), getBatteryName(batteryIndex).CSTR(), targetCharge);
    DS_WorkflowDef::WorkflowBatteryState nextState = DS_WorkflowDef::WORKFLOW_BATTERY_STATE_INVALID;

    if (checkCommonBatteryErrors(bmsDigests[batteryIndex], &msg))
    {
        DS_WorkflowDef::WorkflowBatteryStatus status = getWorkflowBatteryStatus();
        LOG_ERROR("%s\n", msg.CSTR());
        actStatusBuf.state = DS_ACTION_STATE_INTERNAL_ERR;
        actStatusBuf.err = msg;
        setStatusMessage(msg);
        setState(DS_WorkflowDef::WORKFLOW_BATTERY_STATE_DISCHARGE_FAIL);
        return;
    }

    // ignore first digest in case it doesn't have updated BMS
    if (digestCount == 2)
    {
        // battery is capable of charging - check all FETS
        if (!bmsDigests[batteryIndex].manufacturingStatus.allFetAction)
        {
            LOG_WARNING("[%s] TargetBattery : %s - Battery has all Fet Action disabled. Enabling...\n", getStateStr(getState()).CSTR(), getBatteryName(batteryIndex).CSTR());
            env->ds.mcuAction->actBatteryToUnit(batteryIndex, true);
        }

        // disable other battery's fet
        env->ds.mcuAction->actBatteryToUnit(getOtherBatteryIndex(batteryIndex), false);

        // faster discharging
        fasterDischarging(true);
    }
    else if (digestCount == 3)
    {
        // turn off MAINTOUNIT only AFTER the BMS FETS are configured! - otherwise may power off system
        // MAIN TO UNIT false also disables both battery charging!
        env->ds.mcuAction->actMainToUnit(false);

        nextState = DS_WorkflowDef::WORKFLOW_BATTERY_STATE_DISCHARGE_PROGRESS;
    }

    setStatusMessage(msg);
    if (nextState != DS_WorkflowDef::WORKFLOW_BATTERY_STATE_INVALID)
    {
        setState(nextState);
    }
}

void WorkflowBattery::BMSChanged_Discharge_Progress(const DS_McuDef::BMSDigests &bmsDigests, const DS_McuDef::BMSDigests &prevBmsDigests)
{
    LOG_INFO("WorkflowBattery_STATE_DISCHARGE_PROGRESS\n");
    digestCount++;
    QString msg = "";
    msg = QString().asprintf("[%s]\n TargetBattery : %s, Target Charge : %d", getStateStr(getState()).CSTR(), getBatteryName(batteryIndex).CSTR(), targetCharge);
    DS_WorkflowDef::WorkflowBatteryState nextState = DS_WorkflowDef::WORKFLOW_BATTERY_STATE_INVALID;

    if (checkCommonBatteryErrors(bmsDigests[batteryIndex], &msg))
    {
        DS_WorkflowDef::WorkflowBatteryStatus status = getWorkflowBatteryStatus();
        LOG_ERROR("%s\n", msg.CSTR());
        actStatusBuf.state = DS_ACTION_STATE_INTERNAL_ERR;
        actStatusBuf.err = msg;
        setStatusMessage(msg);
        setState(DS_WorkflowDef::WORKFLOW_BATTERY_STATE_DISCHARGE_FAIL);
        return;
    }

    if (digestCount > 1)
    {
        // when target charge is -1, we are doing qualified discharge
        if (targetCharge == -1)
        {
            if (!bmsDigests[batteryIndex].gaugingStatus.dischargeQualifiedForLearning)
            {
                LOG_ERROR("[%s] TargetBattery : %s - VDQ unset during Qualified Discharge\n", getStateStr(getState()).CSTR(), getBatteryName(batteryIndex).CSTR());
                msg += QString().asprintf("\n FAILED: VDQ unset during Qualified Discharge");
                nextState = DS_WorkflowDef::WORKFLOW_BATTERY_STATE_DISCHARGE_FAIL;
            }
        }

        if (bmsDigests[batteryIndex].gaugingStatus.endOfDischargeVoltageLevel2)
        {
            // lets not let it discharge below EDV2
            LOG_INFO("[%s] TargetBattery : %s - EDV2 reached\n", getStateStr(getState()).CSTR(), getBatteryName(batteryIndex).CSTR());
            msg += QString().asprintf("\n Discharge DONE: EDV2 reached");
            nextState  = DS_WorkflowDef::WORKFLOW_BATTERY_STATE_DISCHARGE_DONE;
        }
        else if (bmsDigests[batteryIndex].sbsStatus.relativeStateOfCharge <= targetCharge)
        {
            LOG_INFO("[%s] TargetBattery : %s - target Charge reached\n", getStateStr(getState()).CSTR(), getBatteryName(batteryIndex).CSTR());
            msg = QString().asprintf("\n DONE: Target Charge reached");
            nextState  = DS_WorkflowDef::WORKFLOW_BATTERY_STATE_DISCHARGE_DONE;
        }
        else if (bmsDigests[batteryIndex].sbsStatus.relativeStateOfCharge <= 5)
        {
            // lets not let battery go below 5%
            LOG_INFO("[%s] TargetBattery : %s - relativeStateOfCharge is below 5%%. Stopping discharge.\n", getStateStr(getState()).CSTR(), getBatteryName(batteryIndex).CSTR());
            msg = QString().asprintf("\n DONE: Battery charge too low");
            nextState  = DS_WorkflowDef::WORKFLOW_BATTERY_STATE_DISCHARGE_DONE;
        }
        else if (bmsDigests[batteryIndex].sbsStatus.current == 0)
        {
            LOG_ERROR("[%s] TargetBattery : %s - Is Not Discharging. Not connected correctly?\n", getStateStr(getState()).CSTR(), getBatteryName(batteryIndex).CSTR());
            msg += QString().asprintf("\n FAILED: target battery is not discharging. Check battery connections");
            nextState = DS_WorkflowDef::WORKFLOW_BATTERY_STATE_DISCHARGE_FAIL;
        }
    }

    setStatusMessage(msg);
    if (nextState != DS_WorkflowDef::WORKFLOW_BATTERY_STATE_INVALID)
    {
        setState(nextState);
    }
}

void WorkflowBattery::BMSChanged_BaseboardTest(const DS_McuDef::BMSDigests &bmsDigests, const DS_McuDef::BMSDigests &prevBmsDigests)
{
    LOG_INFO("[%s]\n", getStateStr(getState()).CSTR());
    digestCount++;
    // change every 3 digests
    if (!(digestCount%3))
    {
        DS_WorkflowDef::WorkflowBatteryState state = (DS_WorkflowDef::WorkflowBatteryState)getState();
        switch (state)
        {
        case DS_WorkflowDef::WORKFLOW_BATTERY_STATE_BASEBOARD_VOUTTEST_ON:
        {
            // vout is on. don't check much.
            setState(DS_WorkflowDef::WORKFLOW_BATTERY_STATE_BASEBOARD_VOUTTEST_OFF);
            break;
        }
        case DS_WorkflowDef::WORKFLOW_BATTERY_STATE_BASEBOARD_VOUTTEST_OFF:
        {
            // vout is off. check if batteries are discharging
            if (bmsDigests[0].sbsStatus.current != 0 || bmsDigests[1].sbsStatus.current != 0)
            {
                setState(DS_WorkflowDef::WORKFLOW_BATTERY_STATE_BASEBOARD_VOUTTEST_ON);
            }
            else
            {
                // batteries are NOT discharging.. is the fet blown?
                setStatusMessage("VOUT TEST: Batteries are NOT discharging");
                setState(DS_WorkflowDef::WORKFLOW_BATTERY_STATE_FAIL);
            }
            break;
        }
        default:
        {
            // code shouldn't reach here
            break;
        }
        }
    }
}

void WorkflowBattery::processState()
{
    DS_WorkflowDef::WorkflowBatteryState state = (DS_WorkflowDef::WorkflowBatteryState)getState();

    switch (state)
    {
    case DS_WorkflowDef::WORKFLOW_BATTERY_STATE_INACTIVE:
    {
        digestCount = 0;
        signalBMSChanged_func = NULL;
        break;
    }
    case DS_WorkflowDef::WORKFLOW_BATTERY_STATE_IDLE:
    {
        reset();
        setStatusMessage("");

        // Automatic Qualified discharge trigger checkshould be here?
        signalBMSChanged_func = &WorkflowBattery::BMSChanged_Idle;
        break;
    }

    // Workflow Charge
    case DS_WorkflowDef::WORKFLOW_BATTERY_STATE_CHARGE_PREPARATION:
    {
        digestCount = 0;
        // AC must be connected prompt and wait
        DS_McuDef::PowerStatus powerStatus = env->ds.mcuData->getPowerStatus();
        if (!powerStatus.isAcPowered)
        {
            setState(DS_WorkflowDef::WORKFLOW_BATTERY_STATE_CHARGE_FAIL);
            setStatusMessage("Power is not connected");
            return;
        }

        signalBMSChanged_func = &WorkflowBattery::BMSChanged_Charge_Preparation;
        break;
    }
    case DS_WorkflowDef::WORKFLOW_BATTERY_STATE_CHARGE_PROGRESS:
    {
        digestCount = 0;
        signalBMSChanged_func = &WorkflowBattery::BMSChanged_Charge_Progress;
        break;
    }
    case DS_WorkflowDef::WORKFLOW_BATTERY_STATE_CHARGE_FAIL:
    {
        DS_WorkflowDef::WorkflowBatteryStatus status = getWorkflowBatteryStatus();
        digestCount = 0;
        signalBMSChanged_func = NULL;

        actStatusBuf.state = DS_ACTION_STATE_INTERNAL_ERR;
        actStatusBuf.err = "Charging Failed: " + status.message;

        setState(DS_WorkflowDef::WORKFLOW_BATTERY_STATE_CHARGE_DONE);
        break;
    }
    case DS_WorkflowDef::WORKFLOW_BATTERY_STATE_CHARGE_ABORT:
    {
        digestCount = 0;
        signalBMSChanged_func = NULL;

        actStatusBuf.state = DS_ACTION_STATE_USER_ABORT;
        actStatusBuf.err = "User Aborted";

        setState(DS_WorkflowDef::WORKFLOW_BATTERY_STATE_CHARGE_DONE);
        break;
    }
    case DS_WorkflowDef::WORKFLOW_BATTERY_STATE_CHARGE_DONE:
    {
        digestCount = 0;
        signalBMSChanged_func = NULL;
        env->ds.mcuAction->actBatteryToUnit(getOtherBatteryIndex(batteryIndex), true);

        actionCompleted(actStatusBuf);

        setState(DS_WorkflowDef::WORKFLOW_BATTERY_STATE_DONE);
        break;
    }


    // Workflow discharge
    case DS_WorkflowDef::WORKFLOW_BATTERY_STATE_DISCHARGE_PREPARATION:
    {
        digestCount = 0;
        signalBMSChanged_func = &WorkflowBattery::BMSChanged_Discharge_Preparation;
        break;
    }
    case DS_WorkflowDef::WORKFLOW_BATTERY_STATE_DISCHARGE_PROGRESS:
    {
        digestCount = 0;
        signalBMSChanged_func = &WorkflowBattery::BMSChanged_Discharge_Progress;
        break;
    }
    case DS_WorkflowDef::WORKFLOW_BATTERY_STATE_DISCHARGE_FAIL:
    {
        digestCount = 0;
        signalBMSChanged_func = NULL;

        actStatusBuf.state = DS_ACTION_STATE_INTERNAL_ERR;
        actStatusBuf.err = "Discharging Failed\n" + actStatusBuf.err;

        setState(DS_WorkflowDef::WORKFLOW_BATTERY_STATE_DISCHARGE_DONE);
        break;
    }
    case DS_WorkflowDef::WORKFLOW_BATTERY_STATE_DISCHARGE_ABORT:
    {
        digestCount = 0;
        signalBMSChanged_func = NULL;

        actStatusBuf.state = DS_ACTION_STATE_USER_ABORT;
        actStatusBuf.err = "User Aborted";

        setState(DS_WorkflowDef::WORKFLOW_BATTERY_STATE_DISCHARGE_DONE);
        break;
    }
    case DS_WorkflowDef::WORKFLOW_BATTERY_STATE_DISCHARGE_DONE:
    {
        digestCount = 0;
        signalBMSChanged_func = NULL;
        env->ds.mcuAction->actMainToUnit(true);
        env->ds.mcuAction->actChargeBattery(batteryIndex, true);
        env->ds.mcuAction->actChargeBattery(getOtherBatteryIndex(batteryIndex), true);
        env->ds.mcuAction->actBatteryToUnit(getOtherBatteryIndex(batteryIndex), true);

        QTimer::singleShot(2000, this, [=] {
            fasterDischarging(false);
            actionCompleted(actStatusBuf);
            setState(DS_WorkflowDef::WORKFLOW_BATTERY_STATE_DONE);
        });
        break;
    }

    case DS_WorkflowDef::WORKFLOW_BATTERY_STATE_ABORT:
    {
        signalBMSChanged_func = NULL;
        setState(DS_WorkflowDef::WORKFLOW_BATTERY_STATE_DONE);
        break;
    }
    case DS_WorkflowDef::WORKFLOW_BATTERY_STATE_FAIL:
    {
        // nothing. user needs to see fail message and press "Abort"
        break;
    }
    case DS_WorkflowDef::WORKFLOW_BATTERY_STATE_DONE:
    {
        signalBMSChanged_func = NULL;
        setState(DS_WorkflowDef::WORKFLOW_BATTERY_STATE_IDLE);
        break;
    }

    case DS_WorkflowDef::WORKFLOW_BATTERY_STATE_BASEBOARD_VOUTTEST_ON:
    {
        signalBMSChanged_func = &WorkflowBattery::BMSChanged_BaseboardTest;

        // This also turns on VOUT
        env->ds.mcuAction->actMainToUnit(true);
        env->ds.mcuAction->actChargeBattery(0, true);
        env->ds.mcuAction->actChargeBattery(1, true);
        // state switching happens every 3 digest
        QString msg = QString().asprintf("Toggle Count = %d", digestCount/3);
        setStatusMessage(msg);

        break;
    }
    case DS_WorkflowDef::WORKFLOW_BATTERY_STATE_BASEBOARD_VOUTTEST_OFF:
    {
        signalBMSChanged_func = &WorkflowBattery::BMSChanged_BaseboardTest;

        // This also turns off both batteries' chgfet
        env->ds.mcuAction->actMainToUnit(false);
        env->ds.mcuAction->actChargeBattery(0, false);
        env->ds.mcuAction->actChargeBattery(1, false);
        // state switching happens every 3 digest
        QString msg = QString().asprintf("Toggle Count = %d", digestCount/3);
        setStatusMessage(msg);
        break;
    }
    default:
    {
        LOG_ERROR("State [%s] is not handled!\n", getStateStr(state).CSTR());
        break;
    }
    }
}

QString WorkflowBattery::getBatteryName(int batteryIdx)
{
    return (batteryIdx == 0) ? "A" : "B";
}

int WorkflowBattery::getOtherBatteryIndex(int batteryIdx)
{
    return (batteryIdx == 0) ? 1 : 0;
}

void WorkflowBattery::reset()
{
    targetCharge = 0;
    digestCount = 0;
    signalBMSChanged_func = NULL;

    // commenting this out. This necessarily does not need to be reset to 0 as 0 also is valid battery pack.
    // this was causing issues with asynchronous behavior of the state machine and it was reset at wrong time causing battery B BMS functions to not work.
    // batteryIndex = 0;

    maxChargeCurrent = 0;
}

void WorkflowBattery::setStateSynch(int newState)
{
    DS_WorkflowDef::WorkflowBatteryStatus workflowBatteryStatus = getWorkflowBatteryStatus();
    workflowBatteryStatus.state = (DS_WorkflowDef::WorkflowBatteryState)newState;
    env->ds.workflowData->setWorkflowBatteryStatus(workflowBatteryStatus);
}

int WorkflowBattery::getState()
{
    return getWorkflowBatteryStatus().state;
}

bool WorkflowBattery::isSetStateReady()
{
    return (getState() != DS_WorkflowDef::WORKFLOW_BATTERY_STATE_INACTIVE);
}

QString WorkflowBattery::getStateStr(int state)
{
    return ImrParser::ToImr_WorkflowBatteryState((DS_WorkflowDef::WorkflowBatteryState) state);
}

DS_WorkflowDef::WorkflowBatteryStatus WorkflowBattery::getWorkflowBatteryStatus()
{
    return env->ds.workflowData->getWorkflowBatteryStatus();
}

void WorkflowBattery::setStatusMessage(QString message)
{
    DS_WorkflowDef::WorkflowBatteryStatus status = getWorkflowBatteryStatus();
    status.message = message;
    env->ds.workflowData->setWorkflowBatteryStatus(status);
}

bool WorkflowBattery::checkCommonBatteryErrors(const DS_McuDef::BMSDigest &bmsDigest, QString *err)
{
    QString bmsI2CFaultAlarmName = (batteryIndex == 0) ? "ALARM_I2C_GENERAL_BATTERY_MANAGEMENT_SYSTEM_A" : "ALARM_I2C_GENERAL_BATTERY_MANAGEMENT_SYSTEM_B";
    if (env->ds.alertAction->isActivated("GeneralI2CFault", bmsI2CFaultAlarmName))
    {
        *err = QString().asprintf("[%s]\n TargetBattery : %s - Battery I2C Fault", getStateStr(getState()).CSTR(), getBatteryName(batteryIndex).CSTR());
        return true;
    }

    // must check after I2C error.
    // there shouldn't be any permanent failures
    // Check if the target battery has any permanent failures - abort if any
    // pfStatusBytes is of a type QString. Example "00 00 00 00". Any non-zero represents a permanent failure, but due to the string also containing spaces, we ignore spaces as well
    bool batteryPermanentFailures = bmsDigest.pfStatusBytes.contains(QRegularExpression("[^0 ]"));
    if (batteryPermanentFailures)
    {
        *err = QString().asprintf("[%s]\n TargetBattery : %s - Battery Permanent Failure", getStateStr(getState()).CSTR(), getBatteryName(batteryIndex).CSTR());
        return true;
    }

    return false;
}

void WorkflowBattery::fasterDischarging(bool enable)
{
    if (enable)
    {
        env->ds.systemAction->actSetScreenSleepTime(0);
        env->ds.systemAction->actSetScreenBrightness(DS_CfgLocalDef::SCREEN_BRIGHTNESS_LEVEL_MAX);
    }
    else // restore
    {
        // Restore screen saver
        env->ds.systemAction->actSetScreenSleepTimeToDefault();

        // Restore screen brightness
        int screenBrightnessLevel = env->ds.cfgLocal->get_Settings_Display_ScreenBrightness();
        env->ds.systemAction->actSetScreenBrightness(screenBrightnessLevel);
    }
}
