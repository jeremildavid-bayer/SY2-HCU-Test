#include "Apps/AppManager.h"
#include "WorkflowShippingMode.h"
#include "Common/ImrParser.h"
#include "DataServices/Workflow/DS_WorkflowData.h"
#include "DataServices/Workflow/DS_WorkflowAction.h"
#include "DataServices/System/DS_SystemAction.h"
#include "DataServices/CfgLocal/DS_CfgLocal.h"
#include "DataServices/Alert/DS_AlertAction.h"
#include "DataServices/Mcu/DS_McuData.h"
#include "DataServices/Mcu/DS_McuAction.h"
#include "DataServices/Test/DS_TestAction.h"
#include "DataServices/Capabilities/DS_Capabilities.h"

WorkflowShippingMode::WorkflowShippingMode(QObject *parent, EnvGlobal *env_) :
    ActionBaseExt(parent, env_)
{
    envLocal = new EnvLocal("DS_Workflow-ShippingMode", "WORKFLOW_SHIPPING_MODE");

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });

    batteryIndex = 0;
}

WorkflowShippingMode::~WorkflowShippingMode()
{
    delete envLocal;
}

void WorkflowShippingMode::slotAppInitialised()
{
    DS_WorkflowDef::ShippingModeStatus status;
    status.state = DS_WorkflowDef::SHIPPING_MODE_STATE_READY;
    status.message = "";
    env->ds.workflowData->setShippingModeStatus(status);
    processState();
}

DS_WorkflowDef::ShippingModeStatus WorkflowShippingMode::getShippingModeStatus()
{
    return env->ds.workflowData->getShippingModeStatus();
}

int WorkflowShippingMode::getState()
{
    return getShippingModeStatus().state;
}

QString WorkflowShippingMode::getStateStr(int state)
{
    return ImrParser::ToImr_ShippingModeState((DS_WorkflowDef::ShippingModeState)state);
}

void WorkflowShippingMode::setStateSynch(int newState)
{
    DS_WorkflowDef::ShippingModeStatus status = getShippingModeStatus();
    status.state = (DS_WorkflowDef::ShippingModeState)newState;
    env->ds.workflowData->setShippingModeStatus(status);
}

void WorkflowShippingMode::setStatusMessage(QString message)
{
    DS_WorkflowDef::ShippingModeStatus status = getShippingModeStatus();
    status.message = message;
    env->ds.workflowData->setShippingModeStatus(status);
}

bool WorkflowShippingMode::isSetStateReady()
{
    return (getState() != DS_WorkflowDef::SHIPPING_MODE_STATE_INACTIVE);
}

void WorkflowShippingMode::processState()
{
    DS_WorkflowDef::ShippingModeState state = (DS_WorkflowDef::ShippingModeState)getState();

    switch (state)
    {
    case DS_WorkflowDef::SHIPPING_MODE_STATE_INACTIVE:
        LOG_INFO("SHIPPING_MODE_STATE_INACTIVE\n");
        break;
    case DS_WorkflowDef::SHIPPING_MODE_STATE_READY:
        LOG_INFO("SHIPPING_MODE_STATE_READY\n");
        break;
    case DS_WorkflowDef::SHIPPING_MODE_STATE_PREPARING:
    {
        LOG_INFO("[%d]: SHIPPING_MODE_STATE_PREPARING\n", batteryIndex);
        env->ds.alertAction->activate("ShippingModeInProgress", getBatteryName(batteryIndex));

        DS_McuDef::BMSDigests bmsDigests = env->ds.mcuData->getBMSDigests();
        int charged = bmsDigests[batteryIndex].sbsStatus.relativeStateOfCharge;

        // allowance of 1%
        if (qAbs(targetChargedLevel - charged) <= 1)
        {
            LOG_INFO("[%d]: Battery already within 1%% of target. Skipping charge/discharge.\n", batteryIndex);
            setState(DS_WorkflowDef::SHIPPING_MODE_STATE_SET_SLEEP_MODE);
        }
        else
        {
            // Unit expects AC to be connected for shipping mode
            DS_McuDef::PowerStatus powerStatus = env->ds.mcuData->getPowerStatus();
            if (powerStatus.isAcPowered)
            {
                handleSubAction(DS_WorkflowDef::SHIPPING_MODE_STATE_CHARGING_DISCHARGING_PROGRESS, DS_WorkflowDef::SHIPPING_MODE_STATE_CHARGING_DISCHARGING_DONE, DS_WorkflowDef::SHIPPING_MODE_STATE_CHARGING_DISCHARGING_FAIL);
                env->ds.workflowAction->actWorkflowBatteryChargeToStart(batteryIndex, targetChargedLevel, guidSubAction);
            }
            else
            {
                setState(DS_WorkflowDef::SHIPPING_MODE_STATE_USER_POWER_CONNECT_WAITING);
            }
        }
        break;
    }
    //-----------------------------------
    case DS_WorkflowDef::SHIPPING_MODE_STATE_USER_POWER_CONNECT_WAITING:
    {
        LOG_INFO("[%d]: SHIPPING_MODE_STATE_USER_POWER_CONNECT_WAITING\n", batteryIndex);
        // Wait for user interaction
        break;
    }
    case DS_WorkflowDef::SHIPPING_MODE_STATE_USER_POWER_CONNECT_CONFIRMED:
    {
        LOG_INFO("[%d]: SHIPPING_MODE_STATE_USER_POWER_CONNECT_CONFIRMED\n", batteryIndex);
        setState(DS_WorkflowDef::SHIPPING_MODE_STATE_PREPARING);
        break;
    }

    //-----------------------------------
    case DS_WorkflowDef::SHIPPING_MODE_STATE_CHARGING_DISCHARGING_PROGRESS:
    {
        LOG_INFO("SHIPPING_MODE_STATE_CHARGING_DISCHARGING_PROGRESS\n");
        QString msg = QString().asprintf("Battery : [%s], Target Charge = %d", getBatteryName(batteryIndex).CSTR(), targetChargedLevel);
        setStatusMessage(msg);
        break;
    }
    case DS_WorkflowDef::SHIPPING_MODE_STATE_CHARGING_DISCHARGING_DONE:
    {
        LOG_INFO("SHIPPING_MODE_STATE_CHARGING_DISCHARGING_DONE\n");
        setState(DS_WorkflowDef::SHIPPING_MODE_STATE_SET_SLEEP_MODE);
        break;
    }
    case DS_WorkflowDef::SHIPPING_MODE_STATE_CHARGING_DISCHARGING_FAIL:
    {
        LOG_INFO("SHIPPING_MODE_STATE_CHARGING_DISCHARGING_FAIL\n");
        setStatusMessage(actStatusBuf.err);
        setState(DS_WorkflowDef::SHIPPING_MODE_STATE_FAILED);
        break;
    }

    case DS_WorkflowDef::SHIPPING_MODE_STATE_SET_SLEEP_MODE:
    {
        LOG_INFO("[%d]: SHIPPING_MODE_STATE_SET_SLEEP_MODE\n", batteryIndex);
        QString msg = QString().asprintf("Putting Battery [%s] into sleep mode", getBatteryName(batteryIndex).CSTR());
        setStatusMessage(msg);
        env->ds.mcuAction->actChargeBattery(batteryIndex, false);
        QTimer::singleShot(60000, this, [=] {
            // Send Sleep Mode command
            env->ds.mcuAction->actBmsCommand(batteryIndex, "0011");
            QTimer::singleShot(2000, this, [=] {
                // Disable BMS Digest polling by setting HwType to OCS
                // Once the battery is in sleep mode, I2C must remain slient otherwise it will wake up the battery.
                // So we change the baseboard type to OCS so battery doesn't wake up by another bms digest polling.
                // However there is still a very small room where the polling wakes up the battery due to timing issues.
                // Batteries remaining in sleep state is a desired outcome however it is not a regulation or a game changer.
                // If it accidentally wakes up, it is still OK
                // There is no way to check if the battery is actually in sleep (due to any BMS polling waking up the battery)
                env->ds.mcuAction->actSetBaseType("OCS");
                setState(DS_WorkflowDef::SHIPPING_MODE_STATE_USER_REMOVE_BATTERY_WAITING);
            });
        });
        break;
    }
    //-----------------------------------
    case DS_WorkflowDef::SHIPPING_MODE_STATE_USER_REMOVE_BATTERY_WAITING:
    {
        LOG_INFO("[%d]: SHIPPING_MODE_STATE_USER_REMOVE_BATTERY_WAITING\n", batteryIndex);
        QString msg = QString().asprintf("Please remove battery pack [%s] and click 'Continue'", getBatteryName(batteryIndex).CSTR());
        setStatusMessage(msg);
        // Wait for user interaction
        break;
    }
    case DS_WorkflowDef::SHIPPING_MODE_STATE_USER_REMOVE_BATTERY_DONE:
    {
        LOG_INFO("[%d]: SHIPPING_MODE_STATE_USER_REMOVE_BATTERY_DONE\n", batteryIndex);

        // set base type back
        env->ds.mcuAction->actSetBaseType();

        // NOTE: if MAINTOUNIT if disabled (it shouldn't be) this command will enable MAINTOUNIT first
        env->ds.mcuAction->actChargeBattery(batteryIndex, true);

        QTimer::singleShot(2000, this, [=] {
            actionCompleted(actStatusBuf);
            setStatusMessage("");
            setState(DS_WorkflowDef::SHIPPING_MODE_STATE_DONE);
        });
        break;
    }
    //-----------------------------------
    case DS_WorkflowDef::SHIPPING_MODE_STATE_FAILED:
    {
        LOG_ERROR("[%s] Target Battery %d: %s\n", getStateStr(getState()).CSTR(), batteryIndex, actStatusBuf.err.CSTR());
        DS_WorkflowDef::ShippingModeStatus status = getShippingModeStatus();
        actStatusBuf.state = DS_ACTION_STATE_INTERNAL_ERR;
        actStatusBuf.err = "Failed: " + status.message;
        setStatusMessage(actStatusBuf.err);
        setState(DS_WorkflowDef::SHIPPING_MODE_STATE_DONE);
        break;
    }
    case DS_WorkflowDef::SHIPPING_MODE_STATE_ABORTED:
    {
        LOG_INFO("[%d]: SHIPPING_MODE_STATE_ABORTED\n", batteryIndex);
        actStatusBuf.state = DS_ACTION_STATE_USER_ABORT;
        actStatusBuf.err = "User Aborted";
        setStatusMessage(actStatusBuf.err);
        setState(DS_WorkflowDef::SHIPPING_MODE_STATE_DONE);
        break;
    }
    case DS_WorkflowDef::SHIPPING_MODE_STATE_DONE:
    {
        LOG_INFO("[%d]: SHIPPING_MODE_STATE_DONE\n", batteryIndex);
        actionCompleted(actStatusBuf);
        env->ds.alertAction->deactivate("ShippingModeInProgress");
        // waiting for user interaction
        break;
    }
    default:
        LOG_ERROR("[%d]: SHIPPING_MODE_STATE: Invalid State (%d)\n", batteryIndex, state);
        break;
    }
}

DataServiceActionStatus WorkflowShippingMode::actShippingModeStart(int batteryIdx, int targetCharge, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "ShippingModeStart", QString().asprintf("%d; target Charge: %d", batteryIdx, targetCharge));
    DS_WorkflowDef::ShippingModeState state = (DS_WorkflowDef::ShippingModeState)getState();

    if (state != DS_WorkflowDef::SHIPPING_MODE_STATE_READY)
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
    targetChargedLevel = targetCharge;

    actStatusBuf = status;
    actStatusBuf.state = DS_ACTION_STATE_STARTED;

    actionStarted(actStatusBuf);

    setState(DS_WorkflowDef::SHIPPING_MODE_STATE_PREPARING);

    return actStatusBuf;
}

DataServiceActionStatus WorkflowShippingMode::actShippingModeResume(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "ShippingModeResume");

    DS_WorkflowDef::ShippingModeState state = (DS_WorkflowDef::ShippingModeState)getState();
    DS_WorkflowDef::ShippingModeState newState;

    switch (state)
    {
    case DS_WorkflowDef::SHIPPING_MODE_STATE_USER_POWER_CONNECT_WAITING:
        newState = DS_WorkflowDef::SHIPPING_MODE_STATE_USER_POWER_CONNECT_CONFIRMED;
        break;
    case DS_WorkflowDef::SHIPPING_MODE_STATE_USER_REMOVE_BATTERY_WAITING:
        newState = DS_WorkflowDef::SHIPPING_MODE_STATE_USER_REMOVE_BATTERY_DONE;
        break;
    case DS_WorkflowDef::SHIPPING_MODE_STATE_DONE:
        newState = DS_WorkflowDef::SHIPPING_MODE_STATE_READY;
        break;
    default:
        status.err = "Invalid State: " + ImrParser::ToImr_ShippingModeState(state);
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

DataServiceActionStatus WorkflowShippingMode::actShippingModeAbort(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "ShippingModeAbort");

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);

    if (getState() == DS_WorkflowDef::SHIPPING_MODE_STATE_FAILED)
    {
        setState(DS_WorkflowDef::SHIPPING_MODE_STATE_READY);
    }
    else
    {
        if (getState() == DS_WorkflowDef::SHIPPING_MODE_STATE_CHARGING_DISCHARGING_PROGRESS)
        {
            // handle subaction will change state
            env->ds.workflowAction->actWorkflowBatteryAbort();
        }
        else
        {
            setState(DS_WorkflowDef::SHIPPING_MODE_STATE_ABORTED);
        }
    }

    status.state = DS_ACTION_STATE_COMPLETED;
    actionCompleted(status);

    return status;
}

int WorkflowShippingMode::getOtherBatteryIndex(int batteryIdx)
{
    return (batteryIdx == 0) ? 1 : 0;
}

QString WorkflowShippingMode::getBatteryName(int batteryIdx)
{
    return (batteryIdx == 0) ? "A" : "B";
}
