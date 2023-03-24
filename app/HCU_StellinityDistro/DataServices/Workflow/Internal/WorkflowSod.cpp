#include "Apps/AppManager.h"
#include "Common/ImrParser.h"
#include "WorkflowSod.h"
#include "DataServices/Workflow/DS_WorkflowData.h"
#include "DataServices/Workflow/DS_WorkflowAction.h"
#include "DataServices/Exam/DS_ExamAction.h"
#include "DataServices/Device/DS_DeviceAction.h"
#include "DataServices/Device/DS_DeviceData.h"
#include "DataServices/Alert/DS_AlertAction.h"
#include "DataServices/Alert/DS_AlertData.h"
#include "DataServices/Mcu/DS_McuAction.h"

WorkflowSod::WorkflowSod(QObject *parent, EnvGlobal *env_) :
    ActionBaseExt(parent, env_)
{
    envLocal = new EnvLocal("DS_Workflow-StartOfDay", "WORKFLOW_SOD", LOG_MID_SIZE_BYTES);

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });

    state = STATE_INACTIVE;
}

WorkflowSod::~WorkflowSod()
{
    delete envLocal;
}

void WorkflowSod::slotAppInitialised()
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
            state = STATE_READY;
            processState();
        }
        else if ( (workflowState == DS_WorkflowDef::STATE_INACTIVE) &&
                  (workflowStatePrev != DS_WorkflowDef::STATE_INACTIVE) )
        {
            env->actionMgr->deleteActAll(actStatusBuf.guid);
            env->actionMgr->deleteActAll(guidSudsWaiting);

            if ( (state != STATE_READY) && (state != STATE_PURGE_SUSPENDED) )
            {
                if (actStatusBuf.guid != "")
                {
                    // SOD action is still in progress, but aborted
                    env->ds.workflowData->setSodErrorState(DS_WorkflowDef::SOD_ERROR_STATE_SERVICE_ABORT);
                    actStatusBuf.state = DS_ACTION_STATE_ALLSTOP_BTN_ABORT;
                    actStatusBuf.err = ImrParser::ToImr_SodErrorState(DS_WorkflowDef::SOD_ERROR_STATE_SERVICE_ABORT);
                    actionCompleted(actStatusBuf);
                }
            }

            state = STATE_INACTIVE;

            processState();
        }
    });

    connect(env->ds.alertData, &DS_AlertData::signalDataChanged_ActiveAlerts, this, [=]() {
       if (state == STATE_FIND_PLUNGERS_PROGRESS)
       {
           if (env->ds.alertAction->isActivated("StopcockEngagementFault", "", true))
           {
                actSodAbort(DS_WorkflowDef::SOD_ERROR_STATE_FIND_PLUNGERS_FAILED);
           }
       }
    });
}

bool WorkflowSod::isSetStateReady()
{
    if (state == STATE_INACTIVE)
    {
        return false;
    }
    return true;
}

void WorkflowSod::processState()
{
    switch (state)
    {
    case STATE_INACTIVE:
        LOG_INFO("STATE_INACTIVE\n");
        break;
    case STATE_READY:
        LOG_INFO("STATE_READY\n");
        break;
    case STATE_STARTED:
        LOG_INFO("STATE_STARTED\n");
        setState(STATE_FIND_PLUNGERS_STARTED);
        break;
    case STATE_FIND_PLUNGERS_STARTED:
        LOG_INFO("STATE_FIND_PLUNGERS_STARTED\n");
        handleSubAction(STATE_FIND_PLUNGERS_PROGRESS, STATE_FIND_PLUNGERS_DONE, STATE_FIND_PLUNGERS_FAILED);
        env->ds.deviceAction->actMudsFindPlungerAll(guidSubAction);
        break;
    case STATE_FIND_PLUNGERS_PROGRESS:
        LOG_INFO("STATE_FIND_PLUNGERS_PROGRESS\n");
        break;
    case STATE_FIND_PLUNGERS_DONE:
        LOG_INFO("STATE_FIND_PLUNGERS_DONE\n");
        setState(STATE_IDENTIFYING_MUDS_AGE);
        break;
    case STATE_FIND_PLUNGERS_FAILED:
        LOG_WARNING("STATE_FIND_PLUNGERS_FAILED\n");
        {
            DS_WorkflowDef::SodErrorState sodErrorState = DS_WorkflowDef::SOD_ERROR_STATE_FIND_PLUNGERS_FAILED;
            if (actStatusBuf.err.contains("USER_ABORT"))
            {
                sodErrorState = DS_WorkflowDef::SOD_ERROR_STATE_FIND_PLUNGERS_USER_ABORT;
            }
            env->ds.workflowData->setSodErrorState(sodErrorState);
            setState(STATE_FAILED);
        }
        break;
    case STATE_IDENTIFYING_MUDS_AGE:
        {
            LOG_INFO("STATE_IDENTIFYING_MUDS_AGE\n");
            DataServiceActionStatus status = env->ds.deviceAction->actMudsInit();
            if (status.state == DS_ACTION_STATE_COMPLETED)
            {
                LOG_INFO("STATE_IDENTIFYING_MUDS_AGE: Status=%s\n", ImrParser::ToImr_DataServiceActionStatusStr(actStatusBuf).CSTR());
                if (status.reply == _L("USED_MUDS"))
                {
                    // Legitimate used MUDS
                    setState(STATE_REUSE_MUDS_STARTED);
                }
                else
                {
                    // New MUDS
                    setState(STATE_PURGE_PREPARING);
                }
            }
            else
            {
                // Bad used MUDS
                LOG_ERROR("STATE_IDENTIFYING_MUDS_AGE: Status=%s\n", ImrParser::ToImr_DataServiceActionStatusStr(actStatusBuf).CSTR());
                env->ds.workflowData->setSodErrorState(DS_WorkflowDef::SOD_ERROR_STATE_USED_MUDS_DETECTED);
                setState(STATE_SUSPENDED);
            }
        }
        break;
    case STATE_REUSE_MUDS_STARTED:
        LOG_INFO("STATE_REUSE_MUDS_STARTED\n");

        // Restore selected contrast from last injection
        env->ds.examAction->actReloadSelectedContrast();
        if (env->ds.mcuData->getSudsInserted())
        {
            // Error State must be set prior to calling actSudsSetNeedsReplace since that condition is required to force set needs replaced
            env->ds.workflowData->setSodErrorState(DS_WorkflowDef::SOD_ERROR_STATE_REUSE_MUDS_WAITING_SUDS_REMOVAL);

            env->ds.deviceAction->actSudsSetNeedsReplace();
            setState(STATE_REUSE_MUDS_SUDS_REMOVAL_WAITING);
        }
        else
        {
            setState(STATE_ENGAGE_STARTED);
        }
        break;
    case STATE_REUSE_MUDS_SUDS_REMOVAL_WAITING:
        LOG_INFO("STATE_REUSE_MUDS_SUDS_REMOVAL_WAITING\n");
        {
            QString guid = Util::newGuid();
            QMetaObject::Connection conn = connect(env->ds.mcuData, &DS_McuData::signalDataChanged_SudsInserted, this, [=](bool sudsInserted) {
                if (state != STATE_REUSE_MUDS_SUDS_REMOVAL_WAITING)
                {
                    // unexpected callback
                    LOG_WARNING("Unexpected 'SudsInserted' data changed. state=%d\n", state);
                    env->actionMgr->deleteActCompleted(guid);
                }
                else if (!sudsInserted)
                {
                    env->actionMgr->deleteActCompleted(guid);
                    actSodRestart(actStatusBuf.guid);
                }
            });
            env->actionMgr->createActCompleted(guid, conn, QString(__PRETTY_FUNCTION__) + ": STATE_REUSE_MUDS_SUDS_REMOVAL_WAITING: SudsInserted");
        }
        break;
    case STATE_PURGE_PREPARING:
        LOG_INFO("STATE_PURGE_PREPARING\n");
        env->ds.alertAction->deactivate("InterruptedPurgingFactoryMUDS", "AllStop");
        env->ds.alertAction->deactivate("InterruptedPurgingFactoryMUDS", "SUDS");
        if (isSystemBusy())
        {
            waitForIdleSyringes(STATE_PURGE_PREPARING, STATE_PURGE_PREPARING);
        }
        else
        {
            if (env->ds.mcuData->getSudsInserted())
            {
                env->ds.alertAction->activate("InterruptedPurgingFactoryMUDS", "SUDS");
                env->ds.workflowData->setSodErrorState(DS_WorkflowDef::SOD_ERROR_STATE_PURGE_FAILED_SUDS_INSERTED);
                setState(STATE_PURGE_SUDS_REMOVAL_WAITING);
            }
            else
            {
                setState(STATE_PURGE_STOPCOCK_SETTING);
            }
        }
        break;
    case STATE_PURGE_SUDS_REMOVAL_WAITING:
        LOG_INFO("STATE_PURGE_SUDS_REMOVAL_WAITING\n");
        {
            QString guid = Util::newGuid();
            QMetaObject::Connection conn = connect(env->ds.mcuData, &DS_McuData::signalDataChanged_SudsInserted, this, [=](bool sudsInserted) {
                if (state != STATE_PURGE_SUDS_REMOVAL_WAITING)
                {
                    // unexpected callback
                    env->actionMgr->deleteActCompleted(guid);
                }
                else if (!sudsInserted)
                {
                    LOG_INFO("STATE_PURGE_SUDS_REMOVAL_WAITING: SUDS is removed. Resume purging..\n");
                    env->actionMgr->deleteActCompleted(guid);
                    env->ds.workflowData->setSodErrorState(DS_WorkflowDef::SOD_ERROR_STATE_NONE);
                    setState(STATE_PURGE_PREPARING);
                }
            });
            env->actionMgr->createActCompleted(guid, conn, QString(__PRETTY_FUNCTION__) + ": STATE_PURGE_SUDS_REMOVAL_WAITING: SudsInserted");
        }
        break;
    case STATE_PURGE_STOPCOCK_SETTING:
        LOG_INFO("STATE_PURGE_STOPCOCK_SETTING\n");
        handleSubAction(STATE_PURGE_STOPCOCK_PROGRESS, STATE_PURGE_STOPCOCK_DONE, STATE_PURGE_STOPCOCK_FAILED);
        env->ds.deviceAction->actStopcock(DS_McuDef::STOPCOCK_POS_INJECT, DS_McuDef::STOPCOCK_POS_INJECT, DS_McuDef::STOPCOCK_POS_INJECT, STOPCOCK_ACTION_TRIALS_LIMIT, guidSubAction);
        break;
    case STATE_PURGE_STOPCOCK_PROGRESS:
        LOG_INFO("STATE_PURGE_STOPCOCK_PROGRESS\n");
        break;
    case STATE_PURGE_STOPCOCK_DONE:
        LOG_INFO("STATE_PURGE_STOPCOCK_DONE\n");
        setState(STATE_PURGE_STARTED);
        break;
    case STATE_PURGE_STOPCOCK_FAILED:
        LOG_WARNING("STATE_PURGE_STOPCOCK_FAILED\n");
        env->ds.workflowData->setSodErrorState(DS_WorkflowDef::SOD_ERROR_STATE_PURGE_FAILED_STOPCOCK_FAILED);
        setState(STATE_PURGE_FAILED);
        break;
    case STATE_PURGE_STARTED:
        LOG_INFO("STATE_PURGE_STARTED\n");
        {
            if (env->ds.alertAction->isActivated("InterruptedPurgingFactoryMUDS", "AllStop"))
            {
                LOG_WARNING("InterruptedPurgingFactoryMUDS is activated, abort purge\n");
                env->ds.workflowData->setSodErrorState(DS_WorkflowDef::SOD_ERROR_STATE_PURGE_USER_ABORT);
                setState(STATE_PURGE_SUSPENDED);
            }
            else
            {
                handleSubAction(STATE_PURGE_PROGRESS, STATE_PURGE_DONE, STATE_PURGE_FAILED);
                env->ds.deviceAction->actMudsPurgeAirAll(guidSubAction);
            }
        }
        break;
    case STATE_PURGE_PROGRESS:
        LOG_INFO("STATE_PURGE_PROGRESS\n");
        break;
    case STATE_PURGE_DONE:
        LOG_INFO("STATE_PURGE_DONE\n");
        {
            // Turn LED off after purge
            DS_McuDef::ActLedParams param;
            param.setColorOff();
            env->ds.deviceAction->actLeds(LED_IDX_SUDS1, param);
            env->actionMgr->deleteActCompleted(guidSudsWaiting);
			
            // Get diagnostic data to generate the plunger friction alert
            env->ds.mcuAction->actGetPlungerFrictionData();
			
            setState(STATE_ENGAGE_STARTED);
        }
        break;
    case STATE_PURGE_FAILED:
        LOG_WARNING("STATE_PURGE_FAILED: Status=%s\n", ImrParser::ToImr_DataServiceActionStatusStr(actStatusBuf).CSTR());
        {
            DS_WorkflowDef::SodErrorState sodErrorState = DS_WorkflowDef::SOD_ERROR_STATE_PURGE_FAILED;

            if (actStatusBuf.err.contains("USER_ABORT"))
            {
                env->ds.alertAction->activate("InterruptedPurgingFactoryMUDS", "AllStop");
                sodErrorState = DS_WorkflowDef::SOD_ERROR_STATE_PURGE_USER_ABORT;
            }
            else if ( (actStatusBuf.err.contains("SUDS_INSERTED"))||
                      (actStatusBuf.err.contains("SUDSDetected")) )
            {
                sodErrorState = DS_WorkflowDef::SOD_ERROR_STATE_PURGE_FAILED_SUDS_INSERTED;
            }
            else if (actStatusBuf.err.contains("MUDS_REMOVED"))
            {
                sodErrorState = DS_WorkflowDef::SOD_ERROR_STATE_PURGE_FAILED_MUDS_REMOVED;
            }
            else if (actStatusBuf.err.contains("PLUNGER_ENGAGE_FAULT"))
            {
                sodErrorState = DS_WorkflowDef::SOD_ERROR_STATE_ENGAGE_FAILED;
            }

            env->ds.workflowData->setSodErrorState(sodErrorState);

            env->actionMgr->deleteActCompleted(guidSudsWaiting);

            if (sodErrorState == DS_WorkflowDef::SOD_ERROR_STATE_PURGE_FAILED_SUDS_INSERTED)
            {
                env->ds.alertAction->activate("InterruptedPurgingFactoryMUDS", "SUDS");
                setState(STATE_PURGE_SUDS_REMOVAL_WAITING);
            }
            else
            {
                setState(STATE_SUSPENDED);
            }
        }
        break;
    case STATE_PURGE_SUSPENDED:
        LOG_INFO("STATE_PURGE_SUSPENDED\n");
        env->actionMgr->deleteActCompleted(guidSudsWaiting);
        setState(STATE_SUSPENDED);
        break;
    case STATE_ENGAGE_STARTED:
        LOG_INFO("STATE_ENGAGE_STARTED\n");
        handleSubAction(STATE_ENGAGE_PROGRESS, STATE_ENGAGE_DONE, STATE_ENGAGE_FAILED);
        env->ds.deviceAction->actMudsEngageAll(guidSubAction);
        break;
    case STATE_ENGAGE_PROGRESS:
        LOG_INFO("STATE_ENGAGE_PROGRESS\n");
        break;
    case STATE_ENGAGE_DONE:
        LOG_INFO("STATE_ENGAGE_DONE\n");
		if (env->ds.alertAction->isActivated("CalSlackFailed", "", true))
        {
            LOG_WARNING("SOD: Cal Slack Failed from last SOD\n");
            env->ds.workflowData->setSodErrorState(DS_WorkflowDef::SOD_ERROR_STATE_CAL_SLACK_FAILED);
            setState(STATE_FAILED);
        }
        else if (env->ds.alertAction->isActivated("ReservoirAirCheckCalFailed", "", true))
        {
            LOG_WARNING("Reservoir Air Check Calibration Failed from last SOD\n");
            QVariantMap alert = env->ds.alertAction->getActiveAlert("ReservoirAirCheckCalFailed", "", true);
            QString alertData = alert[_L("Data")].toString();
            if (alertData.contains("BAD_DATA"))
            {
                env->ds.workflowData->setSodErrorState(DS_WorkflowDef::SOD_ERROR_STATE_CAL_SYRINGE_AIR_CHECK_FAILED_BAD_DATA);
            }
            else
            {
                env->ds.workflowData->setSodErrorState(DS_WorkflowDef::SOD_ERROR_STATE_CAL_SYRINGE_AIR_CHECK_FAILED_HW_FAULT);
            }
            setState(STATE_FAILED);
        }
        else if (env->ds.alertAction->isActivated("ReservoirAirDetected", "", true))
        {
            LOG_WARNING("Reservoir Air Detected from last SOD\n");
            setState(STATE_FAILED);
        }
        else if (env->ds.alertAction->isActivated("SRUClearingInletFluid", "", true))
        {
            LOG_WARNING("Fluid is not removed from last Fluid Removal\n");
            setState(STATE_FAILED);
        }
        else if (env->ds.alertAction->isActivated("ReservoirAirRecoveryFailed", "", true))
        {
            LOG_WARNING("Reservoir Air Recovery Failed from last SOD\n");
            env->ds.workflowData->setSyringeAirRecoveryState(DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_FAILED_OPERATION_FAILED);
            setState(STATE_FAILED);
        }
        else
        {
            setState(STATE_MUDS_SOD_STARTED);
        }
        break;
    case STATE_ENGAGE_FAILED:
        LOG_WARNING("STATE_ENGAGE_FAILED\n");
        {
            DS_WorkflowDef::SodErrorState sodErrorState = DS_WorkflowDef::SOD_ERROR_STATE_ENGAGE_FAILED;

            if (actStatusBuf.err.contains("USER_ABORT"))
            {
                sodErrorState = DS_WorkflowDef::SOD_ERROR_STATE_ENGAGE_USER_ABORT;
            }
            env->ds.workflowData->setSodErrorState(sodErrorState);
            setState(STATE_FAILED);
        }
        break;
    case STATE_MUDS_SOD_STARTED:
        LOG_INFO("STATE_MUDS_SOD_STARTED\n");
        handleSubAction(STATE_MUDS_SOD_PROGRESS, STATE_MUDS_SOD_DONE, STATE_MUDS_SOD_FAILED);
        env->ds.deviceAction->actMudsSodStart(guidSubAction);
        break;
    case STATE_MUDS_SOD_PROGRESS:
        LOG_INFO("STATE_MUDS_SOD_PROGRESS\n");
        break;
    case STATE_MUDS_SOD_DONE:
        LOG_INFO("STATE_MUDS_SOD_DONE\n");
        setState(STATE_DONE);
        break;
    case STATE_MUDS_SOD_FAILED:
        LOG_ERROR("STATE_MUDS_SOD_FAILED\n");
        {
            DS_WorkflowDef::SodErrorState sodErrorState = env->ds.workflowData->getSodErrorState();
            if ( (sodErrorState != DS_WorkflowDef::SOD_ERROR_STATE_CAL_SLACK_FAILED) &&
                 (sodErrorState != DS_WorkflowDef::SOD_ERROR_STATE_SYRINGE_PRIME_ABORT) &&
                 (sodErrorState != DS_WorkflowDef::SOD_ERROR_STATE_SYRINGE_PRIME_FAILED_SUDS_REMOVED) &&
                 (sodErrorState != DS_WorkflowDef::SOD_ERROR_STATE_SYRINGE_PRIME_FAILED_INSUFFICIENT_VOLUME) &&
                 (sodErrorState != DS_WorkflowDef::SOD_ERROR_STATE_SYRINGE_PRIME_FAILED_HW_FAULT) &&
                 (sodErrorState != DS_WorkflowDef::SOD_ERROR_STATE_SYRINGE_PRIME_FAILED_AIR_DETECTED) &&
                 (sodErrorState != DS_WorkflowDef::SOD_ERROR_STATE_CAL_SYRINGE_AIR_CHECK_FAILED_INSUFFICIENT_VOLUME) &&
                 (sodErrorState != DS_WorkflowDef::SOD_ERROR_STATE_CAL_SYRINGE_AIR_CHECK_FAILED_BAD_DATA) &&
                 (sodErrorState != DS_WorkflowDef::SOD_ERROR_STATE_CAL_SYRINGE_AIR_CHECK_FAILED_HW_FAULT) )
            {
                LOG_ERROR("STATE_MUDS_SOD_FAILED: Unexpected SodErrorState(%s)\n", ImrParser::ToImr_SodErrorState(sodErrorState).CSTR());
            }
            setState(STATE_FAILED);
        }
        break;
    case STATE_SUSPENDED:
        LOG_WARNING("STATE_SUSPENDED\n");
        actStatusBuf.state = DS_ACTION_STATE_ALLSTOP_BTN_ABORT;
        actStatusBuf.err = ImrParser::ToImr_SodErrorState(env->ds.workflowData->getSodErrorState());
        actionCompleted(actStatusBuf);
        break;
    case STATE_FAILED:
        LOG_WARNING("STATE_FAILED\n");
        actStatusBuf.state = DS_ACTION_STATE_INTERNAL_ERR;
        actStatusBuf.err = ImrParser::ToImr_SodErrorState(env->ds.workflowData->getSodErrorState());
        setState(STATE_READY);
        actionCompleted(actStatusBuf);
        handleFailedCases();
        break;
    case STATE_DONE:
        LOG_INFO("STATE_DONE\n");
        actStatusBuf.state = DS_ACTION_STATE_COMPLETED;
        setState(STATE_READY);
        actionCompleted(actStatusBuf);
        break;
    default:
        LOG_ERROR("Unknown State(%d)\n", state);
        break;
    }
}

void WorkflowSod::handleFailedCases()
{
    DS_WorkflowDef::SodErrorState sodErrorState = env->ds.workflowData->getSodErrorState();
    LOG_WARNING("SOD Failed/Suspended (sodErrorState=%s)\n", ImrParser::ToImr_SodErrorState(sodErrorState).CSTR());

    switch (sodErrorState)
    {
    case DS_WorkflowDef::SOD_ERROR_STATE_SYRINGE_PRIME_FAILED_SUDS_REMOVED:
        if (!env->ds.mcuData->getSudsInserted())
        {
            // Should retry SOD if SUDS is reinserted
            env->actionMgr->deleteActAll(guidSudsWaiting);
            guidSudsWaiting = Util::newGuid();
            QMetaObject::Connection conn = connect(env->ds.mcuData, &DS_McuData::signalDataChanged_SudsInserted, this, [=](bool sudsInserted) {
                if (env->ds.workflowData->getWorkflowState() != DS_WorkflowDef::STATE_SOD_SUSPENDED)
                {
                    // unexpected callback
                    env->actionMgr->deleteActCompleted(guidSudsWaiting);
                }
                else if (sudsInserted)
                {
                    LOG_INFO("SOD_ERROR_STATE_SYRINGE_PRIME_FAILED_SUDS_REMOVED: SUDS is re-inserted. Resuming SOD\n");
                    env->actionMgr->deleteActCompleted(guidSudsWaiting);
                    env->ds.workflowAction->actSodRestart();
                }
            });
            env->actionMgr->createActCompleted(guidSudsWaiting, conn, QString(__PRETTY_FUNCTION__) + ": SOD_ERROR_STATE_SYRINGE_PRIME_FAILED_SUDS_REMOVED: SudsInserted");
        }
        else
        {
            LOG_WARNING("SUDS is very quickly re-inserted. Resuming SOD soon..\n");
            QTimer::singleShot(100, this, [=](){
                env->ds.workflowAction->actSodRestart();
            });
        }
        break;
    default:
        break;
    }
}

bool WorkflowSod::isSystemBusy()
{
    DS_McuDef::SyringeStates syringeStates = env->ds.mcuData->getSyringeStates();
    foreach (DS_McuDef::SyringeState state,  syringeStates)
    {
        if (state == DS_McuDef::SYRINGE_STATE_PROCESSING)
        {
            return true;
        }
    }

    return false;
}

void WorkflowSod::waitForIdleSyringes(State curState, State returnState)
{
    env->actionMgr->deleteActAll(guidSyringeStatesWaiting);
    guidSyringeStatesWaiting = Util::newGuid();
    QMetaObject::Connection conn = connect(env->ds.mcuData, &DS_McuData::signalDataChanged_SyringeStates, this, [=]() {
        if (state != curState)
        {
            // unexpected callback
            env->actionMgr->deleteActCompleted(guidSyringeStatesWaiting);
        }
        else if (!isSystemBusy())
        {
            env->actionMgr->deleteActCompleted(guidSyringeStatesWaiting);
            setState(returnState);
        }
    });
    env->actionMgr->createActCompleted(guidSyringeStatesWaiting, conn, QString(__PRETTY_FUNCTION__) + ": waitForIdleSyringes: SyringeStates");
}

DataServiceActionStatus WorkflowSod::actSodStart(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "SOD:Start");

    if (state != STATE_READY)
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "Invalid state";
        actionStarted(status);
        return status;
    }

    actStatusBuf = status;
    actStatusBuf.state = DS_ACTION_STATE_STARTED;

    // Reset Start of Day Error
    env->ds.workflowData->setSodErrorState(DS_WorkflowDef::SOD_ERROR_STATE_NONE);

    actionStarted(actStatusBuf);

    setState(STATE_STARTED);

    return actStatusBuf;
}

DataServiceActionStatus WorkflowSod::actSodRestart(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "SOD:Restart");

    DS_WorkflowDef::WorkflowState workflowState = env->ds.workflowData->getWorkflowState();

    if ( (state == STATE_PURGE_SUDS_REMOVAL_WAITING) ||
         (state == STATE_REUSE_MUDS_SUDS_REMOVAL_WAITING) )
    {
        LOG_INFO("actSodRestart(): Resume while waiting for SUDS removal\n");
    }
    else if ( (workflowState != DS_WorkflowDef::STATE_SOD_RESUMED) &&
              (workflowState != DS_WorkflowDef::STATE_SOD_PROGRESS) )
    {
        LOG_ERROR("actSodRestart(): Failed to start. State not STATE_SOD_RESUMED (%s)\n", ImrParser::ToImr_WorkFlowState(workflowState).CSTR());
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = QString().asprintf("Invalid State(%s)", ImrParser::ToImr_WorkFlowState(workflowState).CSTR());
        actionStarted(status);
        return status;
    }

    DS_WorkflowDef::SodErrorState sodErrorState = env->ds.workflowData->getSodErrorState();

    LOG_INFO("actSodRestart(): SOD Restart initiating while SodErrorState=%s\n", ImrParser::ToImr_SodErrorState(sodErrorState).CSTR());

    // Reset Start of Day Error
    env->ds.workflowData->setSodErrorState(DS_WorkflowDef::SOD_ERROR_STATE_NONE);

    switch (sodErrorState)
    {
    case DS_WorkflowDef::SOD_ERROR_STATE_FIND_PLUNGERS_USER_ABORT:
    case DS_WorkflowDef::SOD_ERROR_STATE_FIND_PLUNGERS_FAILED:
        setState(STATE_FIND_PLUNGERS_STARTED);
        break;
    case DS_WorkflowDef::SOD_ERROR_STATE_PURGE_FAILED:
    case DS_WorkflowDef::SOD_ERROR_STATE_PURGE_USER_ABORT:
    case DS_WorkflowDef::SOD_ERROR_STATE_PURGE_FAILED_SUDS_INSERTED:
        setState(STATE_PURGE_PREPARING);
        break;
    case DS_WorkflowDef::SOD_ERROR_STATE_ENGAGE_USER_ABORT:
    case DS_WorkflowDef::SOD_ERROR_STATE_ENGAGE_FAILED:
        setState(STATE_ENGAGE_STARTED);
        break;
    case DS_WorkflowDef::SOD_ERROR_STATE_SYRINGE_PRIME_ABORT:
    case DS_WorkflowDef::SOD_ERROR_STATE_SYRINGE_PRIME_FAILED_SUDS_REMOVED:
    case DS_WorkflowDef::SOD_ERROR_STATE_SYRINGE_PRIME_FAILED_INSUFFICIENT_VOLUME:
    case DS_WorkflowDef::SOD_ERROR_STATE_SYRINGE_PRIME_FAILED_HW_FAULT:
    case DS_WorkflowDef::SOD_ERROR_STATE_SYRINGE_PRIME_FAILED_AIR_DETECTED:
    case DS_WorkflowDef::SOD_ERROR_STATE_CAL_SYRINGE_AIR_CHECK_FAILED_BAD_DATA:
        setState(STATE_MUDS_SOD_STARTED);
        break;
    case DS_WorkflowDef::SOD_ERROR_STATE_REUSE_MUDS_WAITING_SUDS_REMOVAL:
        setState(STATE_ENGAGE_STARTED);
        break;
    case DS_WorkflowDef::SOD_ERROR_STATE_NONE:
    case DS_WorkflowDef::SOD_ERROR_STATE_USER_ABORT:
    default:
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = ImrParser::ToImr_SodErrorState(sodErrorState);
        actionStarted(status);
        QString err = QString().asprintf("Failed to Resume Unexpected last SOD state(%s)", ImrParser::ToImr_SodErrorState(sodErrorState).CSTR());
        LOG_ERROR("SOD_RESUME: %s\n", err.CSTR());
        env->ds.alertAction->activate("HCUInternalSoftwareError", err);
        return status;
    }

    actStatusBuf = status;
    actStatusBuf.state = DS_ACTION_STATE_STARTED;

    actionStarted(actStatusBuf);

    return actStatusBuf;
}


DataServiceActionStatus WorkflowSod::actMudsSodWithNewFluid(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "SOD:MudsSodWithNewFluid");

    if (state != STATE_READY)
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "Invalid state";
        actionStarted(status);
        return status;
    }

    actStatusBuf = status;
    actStatusBuf.state = DS_ACTION_STATE_STARTED;

    // Reset Start of Day Error
    env->ds.workflowData->setSodErrorState(DS_WorkflowDef::SOD_ERROR_STATE_NONE);

    actionStarted(actStatusBuf);

    setState(STATE_MUDS_SOD_STARTED);

    return actStatusBuf;
}

DataServiceActionStatus WorkflowSod::actSodAbort(DS_WorkflowDef::SodErrorState sodErrorState, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "SOD:Abort", QString().asprintf("%s", ImrParser::ToImr_SodErrorState(sodErrorState).CSTR()));

    DS_WorkflowDef::WorkflowState workflowState = env->ds.workflowData->getWorkflowState();

    if  ( (workflowState < DS_WorkflowDef::STATE_SOD_STARTED) ||
          (workflowState > DS_WorkflowDef::STATE_SOD_DONE) )
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = QString().asprintf("Invalid Workflow State(%s)", ImrParser::ToImr_WorkFlowState(workflowState).CSTR());
        actionStarted(status);
        return status;
    }

    if ( (state == STATE_INACTIVE) ||
         (state == STATE_READY) ||
         (state == STATE_FAILED) ||
         (state == STATE_DONE) )
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "SOD not active";
        actionStarted(status);
        return status;
    }

    env->ds.workflowData->setSodErrorState(sodErrorState);

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);

    setState(STATE_FAILED);
    env->ds.deviceAction->actMudsSodAbort();

    status.state = DS_ACTION_STATE_COMPLETED;
    actionCompleted(status);

    return status;
}

