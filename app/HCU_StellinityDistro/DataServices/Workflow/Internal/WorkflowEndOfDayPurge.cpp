#include "Apps/AppManager.h"
#include "Common/ImrParser.h"
#include "WorkflowEndOfDayPurge.h"
#include "DataServices/Workflow/DS_WorkflowData.h"
#include "DataServices/Device/DS_DeviceAction.h"
#include "DataServices/Device/DS_DeviceData.h"
#include "DataServices/Mcu/DS_McuAction.h"
#include "DataServices/CfgLocal/DS_CfgLocal.h"

WorkflowEndOfDayPurge::WorkflowEndOfDayPurge(QObject *parent, EnvGlobal *env_) :
    ActionBaseExt(parent, env_)
{
    envLocal = new EnvLocal("DS_Workflow-EndOfDayPurge", "WORKFLOW_EOD_PURGE");
    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });
}

WorkflowEndOfDayPurge::~WorkflowEndOfDayPurge()
{
    delete envLocal;
}

void WorkflowEndOfDayPurge::slotAppInitialised()
{
    purgeSyringe = QList<bool>({ true, true, true });
    emptyContrastFirstEnabled = false;

    connect(env->ds.workflowData, &DS_WorkflowData::signalDataChanged_WorkflowState, this, [=](DS_WorkflowDef::WorkflowState workflowState, DS_WorkflowDef::WorkflowState workflowStatePrev) {
        if ( (workflowState != DS_WorkflowDef::STATE_INACTIVE) &&
             (workflowStatePrev == DS_WorkflowDef::STATE_INACTIVE) )
        {
            env->ds.workflowData->setEndOfDayPurgeState(DS_WorkflowDef::END_OF_DAY_PURGE_STATE_READY);
            processState();
        }
        else if ( (workflowState == DS_WorkflowDef::STATE_INACTIVE) &&
                  (workflowStatePrev != DS_WorkflowDef::STATE_INACTIVE) )
        {
            env->ds.workflowData->setEndOfDayPurgeState(DS_WorkflowDef::END_OF_DAY_PURGE_STATE_INACTIVE);
            processState();
        }
    });
}

int WorkflowEndOfDayPurge::getState()
{
    return env->ds.workflowData->getEndOfDayPurgeState();
}

QString WorkflowEndOfDayPurge::getStateStr(int state)
{
    return ImrParser::ToImr_EndOfDayPurgeState((DS_WorkflowDef::EndOfDayPurgeState)state);
}

void WorkflowEndOfDayPurge::setStateSynch(int newState)
{
    env->ds.workflowData->setEndOfDayPurgeState((DS_WorkflowDef::EndOfDayPurgeState)newState);
}

bool WorkflowEndOfDayPurge::isSetStateReady()
{
    if (getState() == DS_WorkflowDef::END_OF_DAY_PURGE_STATE_INACTIVE)
    {
        return false;
    }
    return true;
}

void WorkflowEndOfDayPurge::processState()
{
    DS_WorkflowDef::EndOfDayPurgeState state = (DS_WorkflowDef::EndOfDayPurgeState)getState();
    switch (state)
    {
    case DS_WorkflowDef::END_OF_DAY_PURGE_STATE_INACTIVE:
        LOG_INFO("END_OF_DAY_PURGE_STATE_INACTIVE\n");
        break;
    case DS_WorkflowDef::END_OF_DAY_PURGE_STATE_READY:
        LOG_INFO("END_OF_DAY_PURGE_STATE_READY\n");
        purgeSyringe = { true, true, true };
        env->actionMgr->deleteActAll(actStatusBuf.guid);
        env->actionMgr->deleteActAll(guidSubAction);
        env->actionMgr->deleteActAll(guidWasteContainerMonitor);
        break;
    case DS_WorkflowDef::END_OF_DAY_PURGE_STATE_STARTED:
        LOG_INFO("END_OF_DAY_PURGE_STATE_STARTED\n");
        {
            emptyContrastFirstEnabled = env->ds.cfgLocal->get_Settings_Injector_EmptyContrastFirst();
            DS_DeviceDef::FluidSource fluidSourceSyringeC1 = env->ds.deviceData->getFluidSourceSyringes().at(SYRINGE_IDX_CONTRAST1);
            DS_DeviceDef::FluidSource fluidSourceSyringeC2 = env->ds.deviceData->getFluidSourceSyringes().at(SYRINGE_IDX_CONTRAST2);

            // Empty Contrast First enabled and contrast syringes haven been emptied (in case of user abort after contrast empty)
            if ( (emptyContrastFirstEnabled) &&
                 ( (fluidSourceSyringeC1.currentVolumesTotal() > 0) || (fluidSourceSyringeC2.currentVolumesTotal() > 0) ) )
            {
                purgeSyringe[0] = false;
            }
            setState(DS_WorkflowDef::END_OF_DAY_PURGE_STATE_WAIT_USER_START);
        }
        break;
    case DS_WorkflowDef::END_OF_DAY_PURGE_STATE_WAIT_USER_START:
        LOG_INFO("END_OF_DAY_PURGE_STATE_WAIT_USER_START\n");
        // Wait for user interaction
        break;
    case DS_WorkflowDef::END_OF_DAY_PURGE_STATE_WAIT_INSERT_SUDS:
        LOG_INFO("END_OF_DAY_PURGE_STATE_WAIT_INSERT_SUDS\n");
        {
            // Monitor SUDS state
            QString guid = Util::newGuid();
            QMetaObject::Connection conn = connect(env->ds.mcuData, &DS_McuData::signalDataChanged_SudsInserted, this, [=](bool sudsInserted) {
                DS_WorkflowDef::EndOfDayPurgeState curState = env->ds.workflowData->getEndOfDayPurgeState();

                if (curState == DS_WorkflowDef::END_OF_DAY_PURGE_STATE_READY)
                {
                    // Not interested about suds state any more. Delete callback
                    env->actionMgr->deleteActCompleted(guid);
                }
                else if (curState == DS_WorkflowDef::END_OF_DAY_PURGE_STATE_WAIT_INSERT_SUDS)
                {
                    if (sudsInserted)
                    {
                        setState(DS_WorkflowDef::END_OF_DAY_PURGE_STATE_WAIT_SUDS_TO_WASTE_CONTAINER);
                    }
                }
                else if (curState == DS_WorkflowDef::END_OF_DAY_PURGE_STATE_WAIT_SUDS_TO_WASTE_CONTAINER)
                {
                    if (!sudsInserted)
                    {
                        env->actionMgr->deleteActCompleted(guid);
                        setState(DS_WorkflowDef::END_OF_DAY_PURGE_STATE_WAIT_INSERT_SUDS);
                    }
                }
            });
            env->actionMgr->createActCompleted(guid, conn, QString(__PRETTY_FUNCTION__) + ": END_OF_DAY_PURGE_STATE_WAIT_INSERT_SUDS: SudsInserted");

            if (env->ds.mcuData->getSudsInserted())
            {
                setState(DS_WorkflowDef::END_OF_DAY_PURGE_STATE_WAIT_SUDS_TO_WASTE_CONTAINER);
            }
        }
        break;
    case DS_WorkflowDef::END_OF_DAY_PURGE_STATE_WAIT_SUDS_TO_WASTE_CONTAINER:
        LOG_INFO("END_OF_DAY_PURGE_STATE_SUDS_WASTE_BIN_CHECKING\n");
        // Wait for user interaction
        break;
    case DS_WorkflowDef::END_OF_DAY_PURGE_STATE_STOPCOCK_INJECT_SETTING:
        LOG_INFO("END_OF_DAY_PURGE_STATE_STOPCOCK_INJECT_SETTING\n");
        {
            env->actionMgr->deleteActAll(guidSubAction);
            DS_McuDef::StopcockPosAll stopcockPosAll = { DS_McuDef::STOPCOCK_POS_UNKNOWN, DS_McuDef::STOPCOCK_POS_UNKNOWN, DS_McuDef::STOPCOCK_POS_UNKNOWN };
            for (int syrIdx = 0; syrIdx < SYRINGE_IDX_MAX; syrIdx++)
            {
                if (purgeSyringe[syrIdx])
                {
                    stopcockPosAll[syrIdx] = DS_McuDef::STOPCOCK_POS_INJECT;
                }
                else
                {
                    stopcockPosAll[syrIdx] = DS_McuDef::STOPCOCK_POS_CLOSED;
                }
            }

            handleSubAction(DS_WorkflowDef::END_OF_DAY_PURGE_STATE_STOPCOCK_INJECT_PROGRESS, DS_WorkflowDef::END_OF_DAY_PURGE_STATE_STOPCOCK_INJECT_DONE, DS_WorkflowDef::END_OF_DAY_PURGE_STATE_STOPCOCK_INJECT_FAILED);
            env->ds.deviceAction->actStopcock(stopcockPosAll[SYRINGE_IDX_SALINE], stopcockPosAll[SYRINGE_IDX_CONTRAST1], stopcockPosAll[SYRINGE_IDX_CONTRAST2], STOPCOCK_ACTION_TRIALS_LIMIT, guidSubAction);
        }
        break;
    case DS_WorkflowDef::END_OF_DAY_PURGE_STATE_STOPCOCK_INJECT_PROGRESS:
        LOG_INFO("END_OF_DAY_PURGE_STATE_STOPCOCK_INJECT_PROGRESS\n");
        break;
    case DS_WorkflowDef::END_OF_DAY_PURGE_STATE_STOPCOCK_INJECT_DONE:
        LOG_INFO("END_OF_DAY_PURGE_STATE_STOPCOCK_INJECT_DONE\n");
        setState(DS_WorkflowDef::END_OF_DAY_PURGE_STATE_PUSH_PLUNGERS_STARTED);
        break;
    case DS_WorkflowDef::END_OF_DAY_PURGE_STATE_STOPCOCK_INJECT_FAILED:
        LOG_ERROR("END_OF_DAY_PURGE_STATE_STOPCOCK_INJECT_FAILED\n");
        setState(DS_WorkflowDef::END_OF_DAY_PURGE_STATE_FAILED);
        break;
    case DS_WorkflowDef::END_OF_DAY_PURGE_STATE_PUSH_PLUNGERS_STARTED:
        LOG_INFO("END_OF_DAY_PURGE_STATE_PUSH_PLUNGERS_STARTED\n");
        {
            // Check WasteContainer Level
            if (!env->ds.deviceData->isFluidSourceWasteContainerReady())
            {
                setState(DS_WorkflowDef::END_OF_DAY_PURGE_STATE_FAILED_WASTE_CONTAINER_NOT_READY);
                return;
            }

            // Set Prime states
            env->ds.deviceAction->actSudsSetNeedsPrime(true);
            env->ds.deviceData->setIsFirstAutoPrimeCompleted(false);

            // Monitor WasteContainer Level
            env->actionMgr->deleteActAll(guidWasteContainerMonitor);
            guidWasteContainerMonitor =  Util::newGuid();
            QMetaObject::Connection conn = connect(env->ds.deviceData, &DS_DeviceData::signalDataChanged_FluidSourceWasteContainer, this, [=] {
                if (!env->ds.deviceData->isFluidSourceWasteContainerReady())
                {
                    setState(DS_WorkflowDef::END_OF_DAY_PURGE_STATE_FAILED_WASTE_CONTAINER_NOT_READY);
                    return;
                }
            });
            env->actionMgr->createActCompleted(guidWasteContainerMonitor, conn, QString(__PRETTY_FUNCTION__) + ": END_OF_DAY_PURGE_STATE_PUSH_PLUNGERS_STARTED: WasteContainer");

            // Start push plunger process
            env->actionMgr->deleteActAll(guidSubAction);
            handleSubAction(DS_WorkflowDef::END_OF_DAY_PURGE_STATE_PUSH_PLUNGERS_PROGRESS, DS_WorkflowDef::END_OF_DAY_PURGE_STATE_PUSH_PLUNGERS_DONE, DS_WorkflowDef::END_OF_DAY_PURGE_STATE_PUSH_PLUNGERS_FAILED);
            env->ds.deviceAction->actMudsPurgeFluid(purgeSyringe, guidSubAction);
        }
        break;
    case DS_WorkflowDef::END_OF_DAY_PURGE_STATE_PUSH_PLUNGERS_PROGRESS:
        LOG_INFO("END_OF_DAY_PURGE_STATE_PUSH_PLUNGERS_PROGRESS\n");
        break;
    case DS_WorkflowDef::END_OF_DAY_PURGE_STATE_PUSH_PLUNGERS_DONE:
        LOG_INFO("END_OF_DAY_PURGE_STATE_PUSH_PLUNGERS_DONE\n");
        setState(DS_WorkflowDef::END_OF_DAY_PURGE_STATE_CHECK_SALINE_LEVEL);
        break;
    case DS_WorkflowDef::END_OF_DAY_PURGE_STATE_PUSH_PLUNGERS_FAILED:
        LOG_WARNING("END_OF_DAY_PURGE_STATE_PUSH_PLUNGERS_FAILED\n");
        if (actStatusBuf.err.contains("SUDS_REMOVED"))
        {
            setState(DS_WorkflowDef::END_OF_DAY_PURGE_STATE_FAILED_SUDS_REMOVED);
        }
        else if (actStatusBuf.err.contains("USER_ABORT"))
        {
            setState(DS_WorkflowDef::END_OF_DAY_PURGE_STATE_FAILED_USER_ABORT);
        }
        else
        {
            setState(DS_WorkflowDef::END_OF_DAY_PURGE_STATE_FAILED);
        }
        break;
    case DS_WorkflowDef::END_OF_DAY_PURGE_STATE_CHECK_SALINE_LEVEL:
        LOG_INFO("END_OF_DAY_PURGE_STATE_CHECK_SALINE_LEVEL\n");
        {
            DS_DeviceDef::FluidSource fluidSourceSyringe = env->ds.deviceData->getFluidSourceSyringes().at(SYRINGE_IDX_SALINE);
            if ( fluidSourceSyringe.currentVolumesTotal() > 0 )
            {
                purgeSyringe = { true, false, false };
                setState(DS_WorkflowDef::END_OF_DAY_PURGE_STATE_WAIT_USER_CONFIRM_EMPTY_SALINE);
            }
            else
            {
                setState(DS_WorkflowDef::END_OF_DAY_PURGE_STATE_DONE);
            }
        }
        break;
    case DS_WorkflowDef::END_OF_DAY_PURGE_STATE_WAIT_USER_CONFIRM_EMPTY_SALINE:
        LOG_INFO("END_OF_DAY_PURGE_STATE_WAIT_USER_CONFIRM_EMPTY_SALINE\n");
        // Wait for user interaction
        break;
    case DS_WorkflowDef::END_OF_DAY_PURGE_STATE_FAILED_SUDS_REMOVED:
        LOG_WARNING("END_OF_DAY_PURGE_STATE_FAILED_SUDS_REMOVED\n");
        setState(DS_WorkflowDef::END_OF_DAY_PURGE_STATE_FAILED);
        break;
    case DS_WorkflowDef::END_OF_DAY_PURGE_STATE_FAILED_WASTE_CONTAINER_NOT_READY:
        LOG_WARNING("END_OF_DAY_PURGE_STATE_FAILED_WASTE_CONTAINER_NOT_READY\n");
        env->actionMgr->deleteActAll(guidWasteContainerMonitor);
        env->ds.mcuAction->actStopAll();
        setState(DS_WorkflowDef::END_OF_DAY_PURGE_STATE_FAILED);
        break;
    case DS_WorkflowDef::END_OF_DAY_PURGE_STATE_FAILED_USER_ABORT:
        LOG_WARNING("END_OF_DAY_PURGE_STATE_FAILED_USER_ABORT\n");
        setState(DS_WorkflowDef::END_OF_DAY_PURGE_STATE_FAILED);
        break;
    case DS_WorkflowDef::END_OF_DAY_PURGE_STATE_FAILED:
        LOG_WARNING("END_OF_DAY_PURGE_STATE_FAILED\n");
        actionCompleted(actStatusBuf);
        setState(DS_WorkflowDef::END_OF_DAY_PURGE_STATE_READY);
        break;
    case DS_WorkflowDef::END_OF_DAY_PURGE_STATE_DONE:
        LOG_INFO("END_OF_DAY_PURGE_STATE_DONE\n");

        for (int syringeIdx = 0; syringeIdx < SYRINGE_IDX_MAX; syringeIdx++)
        {
            env->ds.deviceAction->actDeactivateReservoirAlertsOnEmptied((SyringeIdx)syringeIdx);
        }

        actionCompleted(actStatusBuf);
        setState(DS_WorkflowDef::END_OF_DAY_PURGE_STATE_READY);
        break;
    default:
        LOG_ERROR("Invalid State to Process (%d)\n", state);
        setState(DS_WorkflowDef::END_OF_DAY_PURGE_STATE_READY);
        break;
    }
}

DataServiceActionStatus WorkflowEndOfDayPurge::actEndOfDayPurgeStart(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "EndOfDayPurge:Start");

    DS_WorkflowDef::WorkflowState workflowState = env->ds.workflowData->getWorkflowState();
    DS_WorkflowDef::EndOfDayPurgeState endOfDayPurgeState = env->ds.workflowData->getEndOfDayPurgeState();
    if ( (workflowState != DS_WorkflowDef::STATE_END_OF_DAY_PURGE_STARTED) &&
         (endOfDayPurgeState != DS_WorkflowDef::END_OF_DAY_PURGE_STATE_READY) )
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = QString().asprintf("%s", ImrParser::ToImr_EndOfDayPurgeState(endOfDayPurgeState).CSTR());
        actionStarted(status);
        return status;
    }

    // Ready to start
    actStatusBuf = status;
    actStatusBuf.state = DS_ACTION_STATE_STARTED;
    actionStarted(actStatusBuf);

    purgeStartSyringeVols = env->ds.mcuData->getSyringeVols();

    setState(DS_WorkflowDef::END_OF_DAY_PURGE_STATE_STARTED);

    return actStatusBuf;
}

DataServiceActionStatus WorkflowEndOfDayPurge::actEndOfDayPurgeResume(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "EndOfDayPurge:Resume");

    DS_WorkflowDef::WorkflowState workflowState = env->ds.workflowData->getWorkflowState();
    if (workflowState != DS_WorkflowDef::STATE_END_OF_DAY_PURGE_PROGRESS)
    {
        status.err = "Invalid Workflow State: " + ImrParser::ToImr_WorkFlowState(workflowState);
        status.state = DS_ACTION_STATE_INVALID_STATE;
        actionStarted(status);
        return status;
    }

    DS_WorkflowDef::EndOfDayPurgeState state = env->ds.workflowData->getEndOfDayPurgeState();
    DS_WorkflowDef::EndOfDayPurgeState newState;

    switch (state)
    {
    case DS_WorkflowDef::END_OF_DAY_PURGE_STATE_WAIT_USER_START:
    case DS_WorkflowDef::END_OF_DAY_PURGE_STATE_WAIT_USER_CONFIRM_EMPTY_SALINE:
        newState = DS_WorkflowDef::END_OF_DAY_PURGE_STATE_WAIT_INSERT_SUDS;
        break;
    case DS_WorkflowDef::END_OF_DAY_PURGE_STATE_WAIT_SUDS_TO_WASTE_CONTAINER:
        newState = DS_WorkflowDef::END_OF_DAY_PURGE_STATE_STOPCOCK_INJECT_SETTING;
        break;
    default:
        status.err = "Invalid End Of Day Purge State: " + ImrParser::ToImr_EndOfDayPurgeState(state);
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

DataServiceActionStatus WorkflowEndOfDayPurge::actEndOfDayPurgeAbort(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "EndOfDayPurge:Abort");

    DS_WorkflowDef::WorkflowState workflowState = env->ds.workflowData->getWorkflowState();
    if ( (workflowState != DS_WorkflowDef::STATE_END_OF_DAY_PURGE_STARTED) &&
         (workflowState != DS_WorkflowDef::STATE_END_OF_DAY_PURGE_PROGRESS) )
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        actionStarted(status);
        return status;
    }

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);

    DS_WorkflowDef::EndOfDayPurgeState state = env->ds.workflowData->getEndOfDayPurgeState();

    if (state == DS_WorkflowDef::END_OF_DAY_PURGE_STATE_PUSH_PLUNGERS_PROGRESS)
    {
        env->ds.mcuAction->actStopAll();
    }

    // Abort End of Day Purge
    actStatusBuf.state = DS_ACTION_STATE_ALLSTOP_BTN_ABORT;
    actStatusBuf.err = "User Aborted";
    setState(DS_WorkflowDef::END_OF_DAY_PURGE_STATE_FAILED_USER_ABORT);

    // Abort Done
    status.state = DS_ACTION_STATE_COMPLETED;
    actionCompleted(status);

    return status;
}
