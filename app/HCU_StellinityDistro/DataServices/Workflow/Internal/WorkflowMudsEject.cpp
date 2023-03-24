#include "Apps/AppManager.h"
#include "WorkflowMudsEject.h"
#include "Common/ImrParser.h"
#include "DataServices/Workflow/DS_WorkflowData.h"
#include "DataServices/Alert/DS_AlertAction.h"
#include "DataServices/Exam/DS_ExamAction.h"
#include "DataServices/Exam/DS_ExamData.h"
#include "DataServices/Device/DS_DeviceAction.h"
#include "DataServices/Device/DS_DeviceData.h"
#include "DataServices/Capabilities/DS_Capabilities.h"
#include "DataServices/Mcu/DS_McuData.h"
#include "DataServices/Mcu/DS_McuAction.h"
#include "DataServices/Mcu/Internal/McuAlarm.h"

WorkflowMudsEject::WorkflowMudsEject(QObject *parent, EnvGlobal *env_) :
    ActionBaseExt(parent, env_)
{
    envLocal = new EnvLocal("DS_Workflow-MudsEject", "WORKFLOW_MUDS_EJECT");

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });
}

WorkflowMudsEject::~WorkflowMudsEject()
{
    delete envLocal;
}

void WorkflowMudsEject::slotAppInitialised()
{
    connect(env->ds.workflowData, &DS_WorkflowData::signalDataChanged_WorkflowState, this, [=](DS_WorkflowDef::WorkflowState workflowState, DS_WorkflowDef::WorkflowState workflowStatePrev) {
        if ( (workflowState != DS_WorkflowDef::STATE_INACTIVE) &&
             (workflowStatePrev == DS_WorkflowDef::STATE_INACTIVE) )
        {
            env->ds.workflowData->setMudsEjectState(DS_WorkflowDef::MUDS_EJECT_STATE_READY);
            processState();

            if (!env->ds.mcuData->getMudsInserted())
            {
                DS_DeviceDef::FluidSource fluidSourceMuds = env->ds.deviceData->getFluidSourceMuds();
                if (fluidSourceMuds.isInstalled())
                {
                    LOG_WARNING("signalDataChanged_WorkflowState(): MUDS is removed without running MUDS Eject. Start MUDS Eject..\n");
                    actMudsEjectStart();
                }
            }
        }
        else if ( (workflowState == DS_WorkflowDef::STATE_INACTIVE) &&
                  (workflowStatePrev != DS_WorkflowDef::STATE_INACTIVE) )
        {
            env->actionMgr->deleteActAll(actStatusBuf.guid);
            env->actionMgr->deleteActAll(guidSyringesIdleWaiting);
            env->actionMgr->deleteActAll(guidStatePathIdleWaiting);

            env->ds.workflowData->setMudsEjectState(DS_WorkflowDef::MUDS_EJECT_STATE_INACTIVE);
            processState();
        }
    });
}

int WorkflowMudsEject::getState()
{
    return env->ds.workflowData->getMudsEjectState();
}

QString WorkflowMudsEject::getStateStr(int state)
{
    return ImrParser::ToImr_MudsEjectState((DS_WorkflowDef::MudsEjectState)state);
}

void WorkflowMudsEject::setStateSynch(int newState)
{
    env->ds.workflowData->setMudsEjectState((DS_WorkflowDef::MudsEjectState)newState);
}

bool WorkflowMudsEject::isSetStateReady()
{
    DS_WorkflowDef::MudsEjectState state = env->ds.workflowData->getMudsEjectState();
    if (state == DS_WorkflowDef::MUDS_EJECT_STATE_INACTIVE)
    {
        return false;
    }
    return true;
}

void WorkflowMudsEject::processState()
{
    DS_WorkflowDef::MudsEjectState state = env->ds.workflowData->getMudsEjectState();
    switch (state)
    {
    case DS_WorkflowDef::MUDS_EJECT_STATE_INACTIVE:
        LOG_INFO("MUDS_EJECT_STATE_INACTIVE\n");
        break;
    case DS_WorkflowDef::MUDS_EJECT_STATE_READY:
        LOG_INFO("MUDS_EJECT_STATE_READY\n");
        break;
    case DS_WorkflowDef::MUDS_EJECT_STATE_STARTED:
        LOG_INFO("MUDS_EJECT_STATE_STARTED\n");
        {
            env->ds.deviceAction->actBottlesUnload();

            if (env->ds.examData->isExamStarted())
            {
                QString examGuid = env->ds.examData->getExamGuid();
                env->ds.examAction->actExamEnd();
                env->ds.alertAction->activate("MUDSEjectedExamEnded", examGuid);
            }

            DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
            fluidSourceSyringesBeforeEject = fluidSourceSyringes;
            env->ds.workflowData->setSodErrorState(DS_WorkflowDef::SOD_ERROR_STATE_NONE);

            // Set needsReplaced to show orange sources on UI
            for (int syringeIdx = 0; syringeIdx < fluidSourceSyringes.length(); syringeIdx++)
            {
                // Needs to be set before deactivating alerts so they are not re-raised
                fluidSourceSyringes[syringeIdx].needsReplaced = true;
            }
            env->ds.deviceData->setFluidSourceSyringes(fluidSourceSyringes);
            env->ds.deviceAction->actSudsSetNeedsReplace();

            env->ds.alertAction->deactivate("MotorPositionFault");
            env->ds.alertAction->deactivate("StopcockEngagementFault");
            env->ds.alertAction->deactivate("PlungerNotDetected");
            env->ds.alertAction->deactivate("PlungerEngagementFault");
            env->ds.alertAction->deactivate("StopcockUnintendedMotionDetected");
            env->ds.alertAction->deactivate("UsedMUDSDetected");
			env->ds.alertAction->deactivate("CalSlackFailed");
            env->ds.alertAction->deactivate("OutletAirDetected");
            env->ds.alertAction->deactivate("SalineReservoirContrastPossible");
            env->ds.alertAction->deactivate("AutoPrimeFailed");
            env->ds.alertAction->deactivate("SUDSPortExposed");

            for (int syringeIdx = 0; syringeIdx < fluidSourceSyringes.length(); syringeIdx++)
            {
                env->ds.deviceAction->actDeactivateReservoirAlertsOnEmptied((SyringeIdx)syringeIdx);
            }

            env->actionMgr->deleteActAll(guidSyringesIdleWaiting);
            env->actionMgr->deleteActAll(guidStatePathIdleWaiting);

            setState(DS_WorkflowDef::MUDS_EJECT_STATE_WAIT_STATE_PATH_IDLE_WAITING);
        }
        break;
    case DS_WorkflowDef::MUDS_EJECT_STATE_WAIT_STATE_PATH_IDLE_WAITING:
        LOG_INFO("MUDS_EJECT_STATE_WAIT_STATE_PATH_IDLE_WAITING\n");
        {
            DS_SystemDef::StatePath statePath = env->ds.systemData->getStatePath();

            if (statePath != DS_SystemDef::STATE_PATH_IDLE)
            {
                LOG_WARNING("MUDS_EJECT_STATE_WAIT_STATE_PATH_IDLE_WAITING: StatePath=%s, Waiting for StatePath=%s..\n", ImrParser::ToImr_StatePath(statePath).CSTR(), ImrParser::ToImr_StatePath(DS_SystemDef::STATE_PATH_IDLE).CSTR());
                env->actionMgr->deleteActAll(guidStatePathIdleWaiting);
                guidStatePathIdleWaiting = Util::newGuid();

                QMetaObject::Connection conn = connect(env->ds.systemData, &DS_SystemData::signalDataChanged_StatePath, this, [=](DS_SystemDef::StatePath statePath) {
                    DS_WorkflowDef::MudsEjectState state = (DS_WorkflowDef::MudsEjectState)getState();
                    if (state != DS_WorkflowDef::MUDS_EJECT_STATE_WAIT_STATE_PATH_IDLE_WAITING)
                    {
                        // unexpected state
                        env->actionMgr->deleteActAll(guidStatePathIdleWaiting);
                        return;
                    }

                    if (statePath == DS_SystemDef::STATE_PATH_IDLE)
                    {
                        LOG_INFO("MUDS_EJECT_STATE_WAIT_STATE_PATH_IDLE_WAITING: StatePath=%s. Restart the eject sequence..\n", ImrParser::ToImr_StatePath(statePath).CSTR());
                        setState(DS_WorkflowDef::MUDS_EJECT_STATE_STARTED);
                    }
                });
                env->actionMgr->createActCompleted(guidStatePathIdleWaiting, conn, QString(__PRETTY_FUNCTION__) + ": MUDS_EJECT_STATE_WAIT_STATE_PATH_IDLE_WAITING: StatePath");
            }
            else
            {
                setState(DS_WorkflowDef::MUDS_EJECT_STATE_SYRINGES_IDLE_WAITING);
            }
        }
        break;
    case DS_WorkflowDef::MUDS_EJECT_STATE_SYRINGES_IDLE_WAITING:
        LOG_INFO("MUDS_EJECT_STATE_SYRINGES_IDLE_WAITING\n");
        if (env->ds.deviceData->getFluidSourceSyringesBusy())
        {
            LOG_WARNING("MUDS_EJECT_STATE_SYRINGES_IDLE_WAITING: Waiting for syringes become idle..\n");
            env->actionMgr->deleteActAll(guidSyringesIdleWaiting);
            guidSyringesIdleWaiting = Util::newGuid();
            QMetaObject::Connection conn = connect(env->ds.deviceData, &DS_DeviceData::signalDataChanged_FluidSourceSyringes, this, [=] {
                DS_WorkflowDef::MudsEjectState state = (DS_WorkflowDef::MudsEjectState)getState();
                if (state != DS_WorkflowDef::MUDS_EJECT_STATE_SYRINGES_IDLE_WAITING)
                {
                    // unexpected state
                    env->actionMgr->deleteActAll(guidSyringesIdleWaiting);
                    return;
                }

                if (!env->ds.deviceData->getFluidSourceSyringesBusy())
                {
                    LOG_INFO("MUDS_EJECT_STATE_SYRINGES_IDLE_WAITING: Syringes are idle. Restart the eject sequence..\n");
                    setState(DS_WorkflowDef::MUDS_EJECT_STATE_STARTED);
                }
            });
            env->actionMgr->createActCompleted(guidSyringesIdleWaiting, conn, QString(__PRETTY_FUNCTION__) + ": MUDS_EJECT_STATE_SYRINGES_IDLE_WAITING: FluidSourceSyringes");
        }
        else
        {
            setState(DS_WorkflowDef::MUDS_EJECT_STATE_PULL_PISTONS_STARTED);
        }
        break;
    case DS_WorkflowDef::MUDS_EJECT_STATE_PULL_PISTONS_STARTED:
        LOG_INFO("MUDS_EJECT_STATE_PULL_PISTONS_STARTED\n");
        if (env->ds.mcuData->getPlungersEngaged())
        {
            DS_McuDef::ActPistonParams pistonParams;
            pistonParams.flow = MUDS_EJECT_WITHDRAW_FLOW;
            pistonParams.vol = MUDS_EJECT_WITHDRAW_VOL;
            handleSubAction(DS_WorkflowDef::MUDS_EJECT_STATE_PULL_PISTONS_PROGRESS, DS_WorkflowDef::MUDS_EJECT_STATE_PULL_PISTONS_DONE, DS_WorkflowDef::MUDS_EJECT_STATE_PULL_PISTONS_FAILED);
            env->ds.mcuAction->actPistonAll(pistonParams, guidSubAction);
        }
        else
        {
            setState(DS_WorkflowDef::MUDS_EJECT_STATE_PULL_PISTONS_DONE);
        }
        break;
    case DS_WorkflowDef::MUDS_EJECT_STATE_PULL_PISTONS_PROGRESS:
        LOG_INFO("MUDS_EJECT_STATE_PULL_PISTONS_PROGRESS\n");
        break;
    case DS_WorkflowDef::MUDS_EJECT_STATE_PULL_PISTONS_DONE:
        LOG_INFO("MUDS_EJECT_STATE_PULL_PISTONS_DONE\n");
        setState(DS_WorkflowDef::MUDS_EJECT_STATE_STOPCOCK_FILL_SETTING);
        break;
    case DS_WorkflowDef::MUDS_EJECT_STATE_PULL_PISTONS_FAILED:
        LOG_WARNING("MUDS_EJECT_STATE_PULL_PISTONS_FAILED\n");
        setState(DS_WorkflowDef::MUDS_EJECT_STATE_STOPCOCK_FILL_SETTING);
        break;
    case DS_WorkflowDef::MUDS_EJECT_STATE_STOPCOCK_FILL_SETTING:
        LOG_INFO("MUDS_EJECT_STATE_STOPCOCK_FILL_SETTING\n");
        handleSubAction(DS_WorkflowDef::MUDS_EJECT_STATE_STOPCOCK_FILL_PROGRESS, DS_WorkflowDef::MUDS_EJECT_STATE_STOPCOCK_FILL_DONE, DS_WorkflowDef::MUDS_EJECT_STATE_STOPCOCK_FILL_FAILED);
        env->ds.deviceAction->actStopcock(DS_McuDef::STOPCOCK_POS_FILL, DS_McuDef::STOPCOCK_POS_FILL, DS_McuDef::STOPCOCK_POS_FILL, STOPCOCK_ACTION_TRIALS_LIMIT, guidSubAction);
        break;
    case DS_WorkflowDef::MUDS_EJECT_STATE_STOPCOCK_FILL_PROGRESS:
        LOG_INFO("MUDS_EJECT_STATE_STOPCOCK_FILL_PROGRESS\n");
        break;
    case DS_WorkflowDef::MUDS_EJECT_STATE_STOPCOCK_FILL_DONE:
        LOG_INFO("MUDS_EJECT_STATE_STOPCOCK_FILL_DONE\n");
        setState(DS_WorkflowDef::MUDS_EJECT_STATE_DISENGAGE_STARTED);
        break;
    case DS_WorkflowDef::MUDS_EJECT_STATE_STOPCOCK_FILL_FAILED:
        LOG_WARNING("MUDS_EJECT_STATE_STOPCOCK_FILL_FAILED\n");
        setState(DS_WorkflowDef::MUDS_EJECT_STATE_FAILED_SYSTEM_ERROR);
        break;
    case DS_WorkflowDef::MUDS_EJECT_STATE_DISENGAGE_STARTED:
        LOG_INFO("MUDS_EJECT_STATE_DISENGAGE_STARTED\n");
        handleSubAction(DS_WorkflowDef::MUDS_EJECT_STATE_DISENGAGE_PROGRESS, DS_WorkflowDef::MUDS_EJECT_STATE_DISENGAGE_DONE, DS_WorkflowDef::MUDS_EJECT_STATE_DISENGAGE_FAILED);
        env->ds.deviceAction->actMudsDisengageAll(guidSubAction);
        break;
    case DS_WorkflowDef::MUDS_EJECT_STATE_DISENGAGE_PROGRESS:
        LOG_INFO("MUDS_EJECT_STATE_DISENGAGE_PROGRESS\n");
        break;
    case DS_WorkflowDef::MUDS_EJECT_STATE_DISENGAGE_DONE:
        LOG_INFO("MUDS_EJECT_STATE_DISENGAGE_DONE\n");
        setState(DS_WorkflowDef::MUDS_EJECT_STATE_UNLATCH_WAITING);
        break;
    case DS_WorkflowDef::MUDS_EJECT_STATE_DISENGAGE_FAILED:
        LOG_WARNING("MUDS_EJECT_STATE_DISENGAGE_FAILED\n");
        if (actStatusBuf.state == DS_ACTION_STATE_ALLSTOP_BTN_ABORT)
        {
            setState(DS_WorkflowDef::MUDS_EJECT_STATE_FAILED_USER_ABORT);
        }
        else
        {
            setState(DS_WorkflowDef::MUDS_EJECT_STATE_FAILED_SYSTEM_ERROR);
        }
        break;
    case DS_WorkflowDef::MUDS_EJECT_STATE_UNLATCH_WAITING:
        LOG_INFO("MUDS_EJECT_STATE_UNLATCH_WAITING\n");
        {
            env->ds.alertAction->deactivate("MudsPrimeFailed");
            env->ds.alertAction->deactivate("BulkUseLifeExceeded");
            env->ds.alertAction->deactivate("MUDSUseLifeExceeded");
            env->ds.alertAction->deactivate("MUDSUseLifeLimitEnforced");

            if (!env->ds.alertAction->isActivated("MUDSEjectedRemovalRequired", "", true))
            {
                // Prevent duplicated MUDSEjectedRemovalRequired alert with different data
                env->ds.alertAction->activate("MUDSEjectedRemovalRequired", Util::qVarientToJsonData(ImrParser::ToImr_FluidSourceSyringes(fluidSourceSyringesBeforeEject)));
            }

            if (!env->ds.mcuData->getMudsLatched())
            {
                setState(DS_WorkflowDef::MUDS_EJECT_STATE_UNLATCH_DONE);
            }
            else
            {
                QString guid = Util::newGuid();
                QMetaObject::Connection conn = connect(env->ds.mcuData, &DS_McuData::signalDataChanged_MudsLatched, this, [=](bool mudsLatched) {
                    env->actionMgr->deleteActCompleted(guid);

                    DS_WorkflowDef::MudsEjectState state = env->ds.workflowData->getMudsEjectState();

                    if (state != DS_WorkflowDef::MUDS_EJECT_STATE_UNLATCH_WAITING)
                    {
                        // unexpected callback
                    }
                    else if (!mudsLatched)
                    {
                        setState(DS_WorkflowDef::MUDS_EJECT_STATE_UNLATCH_DONE);
                    }
                });
                env->actionMgr->createActCompleted(guid, conn, QString(__PRETTY_FUNCTION__) + ": MUDS_EJECT_STATE_UNLATCH_WAITING: MudsLatched");
            }
        }
        break;
    case DS_WorkflowDef::MUDS_EJECT_STATE_UNLATCH_DONE:
        LOG_INFO("MUDS_EJECT_STATE_UNLATCH_DONE\n");
        setState(DS_WorkflowDef::MUDS_EJECT_STATE_REMOVAL_WAITING);
        break;
    case DS_WorkflowDef::MUDS_EJECT_STATE_REMOVAL_WAITING:
        LOG_INFO("MUDS_EJECT_STATE_REMOVAL_WAITING\n");

        if (!env->ds.mcuData->getMudsPresent())
        {
            setState(DS_WorkflowDef::MUDS_EJECT_STATE_REMOVAL_DONE);
        }
        else
        {
            QString guid = Util::newGuid();
            QMetaObject::Connection conn = connect(env->ds.mcuData, &DS_McuData::signalDataChanged_MudsPresent, this, [=](bool mudsPresent) {
                env->actionMgr->deleteActCompleted(guid);

                if (state != DS_WorkflowDef::MUDS_EJECT_STATE_REMOVAL_WAITING)
                {
                    // unexpected callback
                }
                else if (!mudsPresent)
                {
                    setState(DS_WorkflowDef::MUDS_EJECT_STATE_REMOVAL_DONE);
                }
            });
            env->actionMgr->createActCompleted(guid, conn, QString(__PRETTY_FUNCTION__) + ": MUDS_EJECT_STATE_REMOVAL_WAITING: MudsPresent");
        }
        break;
    case DS_WorkflowDef::MUDS_EJECT_STATE_REMOVAL_DONE:
        LOG_INFO("MUDS_EJECT_STATE_REMOVAL_DONE\n");
        {
            env->ds.alertAction->deactivate("MUDSEjectedRemovalRequired");
            env->ds.alertAction->deactivate("InterruptedPurgingFactoryMUDS");

            DS_DeviceDef::FluidInfo selectedContrast;
            env->ds.examData->setSelectedContrast(selectedContrast);

            // Update Fluid Sources
            DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
            for (int syringeIdx = 0; syringeIdx < fluidSourceSyringes.length(); syringeIdx++)
            {
                fluidSourceSyringes[syringeIdx].needsReplaced = false;
            }
            env->ds.deviceData->setFluidSourceSyringes(fluidSourceSyringes);

            DS_DeviceDef::FluidSource fluidSourceMuds = env->ds.deviceData->getFluidSourceMuds();
            fluidSourceMuds.currentFluidKinds.clear();
            fluidSourceMuds.currentFluidKinds.append("Invalid");
            fluidSourceMuds.sourcePackages.clear();
            fluidSourceMuds.setIsInstalled(false);
            env->ds.deviceData->setFluidSourceMuds(fluidSourceMuds);

            DS_WorkflowDef::MudsSodStatus mudsSodStatus = env->ds.workflowData->getMudsSodStatus();
            mudsSodStatus.init();
            env->ds.workflowData->setMudsSodStatus(mudsSodStatus);

            setState(DS_WorkflowDef::MUDS_EJECT_STATE_DONE);
        }
        break;
    case DS_WorkflowDef::MUDS_EJECT_STATE_FAILED_USER_ABORT:
        LOG_WARNING("MUDS_EJECT_STATE_FAILED_USER_ABORT\n");
        setState(DS_WorkflowDef::MUDS_EJECT_STATE_FAILED);
        break;
    case DS_WorkflowDef::MUDS_EJECT_STATE_FAILED_SYSTEM_ERROR:
        LOG_WARNING("MUDS_EJECT_STATE_FAILED_SYSTEM_ERROR\n");
        setState(DS_WorkflowDef::MUDS_EJECT_STATE_FAILED);
        break;
    case DS_WorkflowDef::MUDS_EJECT_STATE_FAILED:
        LOG_WARNING("MUDS_EJECT_STATE_FAILED\n");
        actionCompleted(actStatusBuf);
        setState(DS_WorkflowDef::MUDS_EJECT_STATE_READY);
        break;
    case DS_WorkflowDef::MUDS_EJECT_STATE_DONE:
        LOG_INFO("MUDS_EJECT_STATE_DONE\n");
        actStatusBuf.state = DS_ACTION_STATE_COMPLETED;
        actionCompleted(actStatusBuf);
        setState(DS_WorkflowDef::MUDS_EJECT_STATE_READY);
        break;
    default:
        LOG_WARNING("Bad Muds Eject State(%d)\n", state);
        break;
    }
}

DataServiceActionStatus WorkflowMudsEject::actMudsEjectStart(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "MudsEject:Start");

    DS_WorkflowDef::MudsEjectState state = env->ds.workflowData->getMudsEjectState();

    if ( (state != DS_WorkflowDef::MUDS_EJECT_STATE_READY) &&
         (state != DS_WorkflowDef::MUDS_EJECT_STATE_INACTIVE) )
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "Invalid state";
        actionStarted(status);
        return status;
    }

    actStatusBuf = status;
    actStatusBuf.state = DS_ACTION_STATE_STARTED;
    actionStarted(actStatusBuf);

    setState(DS_WorkflowDef::MUDS_EJECT_STATE_STARTED);

    return actStatusBuf;
}
