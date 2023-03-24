#include "Apps/AppManager.h"
#include "DeviceMudsSod.h"
#include "Common/ImrParser.h"
#include "DataServices/Workflow/DS_WorkflowData.h"
#include "DataServices/Alert/DS_AlertAction.h"
#include "DataServices/Device/DS_DeviceAction.h"
#include "DataServices/Device/DS_DeviceData.h"
#include "DataServices/Mcu/DS_McuAction.h"
#include "DataServices/Capabilities/DS_Capabilities.h"

DeviceMudsSod::DeviceMudsSod(QObject *parent, EnvGlobal *env_, EnvLocal *envLocal_) :
    ActionBaseExt(parent, env_)
{
    envLocal = envLocal_;
    syringeIdxSodStart = SYRINGE_IDX_NONE;

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });
}

DeviceMudsSod::~DeviceMudsSod()
{
}

void DeviceMudsSod::slotAppInitialised()
{
    connect(env->ds.workflowData, &DS_WorkflowData::signalDataChanged_WorkflowState, this, [=](DS_WorkflowDef::WorkflowState workflowState, DS_WorkflowDef::WorkflowState workflowStatePrev) {
        if ( (workflowState != DS_WorkflowDef::STATE_INACTIVE) &&
             (workflowStatePrev == DS_WorkflowDef::STATE_INACTIVE) )
        {
            env->ds.workflowData->setMudsSodState(DS_WorkflowDef::MUDS_SOD_STATE_READY);
            processState();
        }
        else if ( (workflowState == DS_WorkflowDef::STATE_INACTIVE) &&
                  (workflowStatePrev != DS_WorkflowDef::STATE_INACTIVE) )
        {
            env->actionMgr->deleteActAll(actStatusBuf.guid);
            env->actionMgr->deleteActAll(guidSyringesIdleWaiting);
            env->actionMgr->deleteActAll(guidSalineVolWaiting);
            env->actionMgr->deleteActAll(guidSudsWaiting);
            env->actionMgr->deleteActAll(guidOutletDoorWaiting);
            env->actionMgr->deleteActAll(guidWasteContainerWaiting);

            env->ds.workflowData->setMudsSodState(DS_WorkflowDef::MUDS_SOD_STATE_INACTIVE);
            processState();
        }
    });
}

int DeviceMudsSod::getState()
{
    return env->ds.workflowData->getMudsSodState();
}

QString DeviceMudsSod::getStateStr(int state)
{
    return ImrParser::ToImr_MudsSodState((DS_WorkflowDef::MudsSodState)state);
}

void DeviceMudsSod::setStateSynch(int newState)
{
    env->ds.workflowData->setMudsSodState((DS_WorkflowDef::MudsSodState)newState);
}

DataServiceActionStatus DeviceMudsSod::actSodStart(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "MudsSodStart");

    DS_WorkflowDef::MudsSodState mudsSodState = env->ds.workflowData->getMudsSodState();

    if (mudsSodState != DS_WorkflowDef::MUDS_SOD_STATE_READY)
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = QString().asprintf("Muds SOD in progress, state=%s", ImrParser::ToImr_MudsSodState(mudsSodState).CSTR());
        actionStarted(status);
        return status;
    }

    actStatusBuf = status;
    actStatusBuf.state = DS_ACTION_STATE_STARTED;
    actionStarted(actStatusBuf);
    setState(DS_WorkflowDef::MUDS_SOD_STATE_STARTED);

    return actStatusBuf;
}

DataServiceActionStatus DeviceMudsSod::actSodAbort(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "MudsSodAbort");

    DS_WorkflowDef::MudsSodState state = env->ds.workflowData->getMudsSodState();

    if ( (state == DS_WorkflowDef::MUDS_SOD_STATE_INACTIVE) ||
         (state == DS_WorkflowDef::MUDS_SOD_STATE_READY) ||
         (state == DS_WorkflowDef::MUDS_SOD_STATE_DONE) )
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "Muds Sod not active";
        actionStarted(status);
        return status;
    }

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);

    setState(DS_WorkflowDef::MUDS_SOD_STATE_ABORT);

    // TODO: Emit completed signal when abort completes

    status.state = DS_ACTION_STATE_COMPLETED;
    actionCompleted(status);

    return status;
}

bool DeviceMudsSod::isSetStateReady()
{
    if (getState() == DS_WorkflowDef::MUDS_SOD_STATE_INACTIVE)
    {
        return false;
    }
    return true;
}

void DeviceMudsSod::processState()
{
    DS_WorkflowDef::MudsSodState state = env->ds.workflowData->getMudsSodState();

    switch (state)
    {
    case DS_WorkflowDef::MUDS_SOD_STATE_INACTIVE:
        LOG_INFO("STATE_INACTIVE\n");
        break;
    case DS_WorkflowDef::MUDS_SOD_STATE_READY:
        LOG_INFO("STATE_READY\n");
        break;
    case DS_WorkflowDef::MUDS_SOD_STATE_STARTED:
        LOG_INFO("STATE_STARTED\n");
        sodPerformedSyringes.clear();
        syringeIdxSodStart = SYRINGE_IDX_NONE;
        setState(DS_WorkflowDef::MUDS_SOD_STATE_SYRINGES_IDLE_WAITING);
        break;
    // ==========================================
    case DS_WorkflowDef::MUDS_SOD_STATE_SYRINGES_IDLE_WAITING:
        LOG_INFO("STATE_SYRINGES_IDLE_WAITING\n");
        {
            // Fluids ready callback connection - if fluids change to filling action state, go back to syringe waiting state
            env->actionMgr->deleteActAll(guidSyringesIdleWaiting);
            guidSyringesIdleWaiting = Util::newGuid();
            QMetaObject::Connection conn = connect(env->ds.deviceData, &DS_DeviceData::signalDataChanged_FluidSourceSyringes, this, [=] {
                DS_WorkflowDef::MudsSodState state = (DS_WorkflowDef::MudsSodState)getState();
                if ( (state != DS_WorkflowDef::MUDS_SOD_STATE_SYRINGES_IDLE_WAITING) &&
                     (state != DS_WorkflowDef::MUDS_SOD_STATE_SALINE_VOLUME_WAITING) &&
                     (state != DS_WorkflowDef::MUDS_SOD_STATE_SUDS_READY_WAITING) &&
                     (state != DS_WorkflowDef::MUDS_SOD_STATE_OUTLET_AIR_DOOR_CLOSE_WAITING) &&
                     (state != DS_WorkflowDef::MUDS_SOD_STATE_WASTE_CONTAINER_WAITING) )
                {
                    // unexpected state
                    env->actionMgr->deleteActAll(guidSyringesIdleWaiting);
                    return;
                }

                if (!env->ds.deviceData->getFluidSourceSyringesBusy())
                {
                    if (state == DS_WorkflowDef::MUDS_SOD_STATE_SYRINGES_IDLE_WAITING)
                    {
                        LOG_INFO("MUDS_SOD_STATE_SYRINGES_IDLE_WAITING: Syringes Idle. Moving to next step..\n");
                        setState(DS_WorkflowDef::MUDS_SOD_STATE_SALINE_VOLUME_WAITING);
                    }
                }
                else
                {
                    if (state != DS_WorkflowDef::MUDS_SOD_STATE_SYRINGES_IDLE_WAITING)
                    {
                        setState(DS_WorkflowDef::MUDS_SOD_STATE_SYRINGES_IDLE_WAITING);
                    }
                }
            });
            env->actionMgr->createActCompleted(guidSyringesIdleWaiting, conn, QString(__PRETTY_FUNCTION__) + ": STATE_SYRINGES_IDLE_WAITING: FluidSourceSyringes");

            // Go to next state if possible
            if (!env->ds.deviceData->getFluidSourceSyringesBusy())
            {
                LOG_INFO("MUDS_SOD_STATE_SYRINGES_IDLE_WAITING: Syringes Idle. Moving to next step..\n");
                setState(DS_WorkflowDef::MUDS_SOD_STATE_SALINE_VOLUME_WAITING);
            }
        }
        break;
    // ==========================================
    case DS_WorkflowDef::MUDS_SOD_STATE_SALINE_VOLUME_WAITING:
        LOG_INFO("STATE_SALINE_VOLUME_WAITING\n");
        {
            // Saline loaded callback connection - if fluid volume is not enough, go back to syringe waiting state
            env->actionMgr->deleteActAll(guidSalineVolWaiting);
            guidSalineVolWaiting = Util::newGuid();
            QMetaObject::Connection conn = connect(env->ds.deviceData, &DS_DeviceData::signalDataChanged_FluidSourceSyringes, this, [=](const DS_DeviceDef::FluidSources &fluidSourceSyringes) {
                DS_WorkflowDef::MudsSodState state = (DS_WorkflowDef::MudsSodState)getState();

                if ( (state != DS_WorkflowDef::MUDS_SOD_STATE_SALINE_VOLUME_WAITING) &&
                     (state != DS_WorkflowDef::MUDS_SOD_STATE_SUDS_READY_WAITING) &&
                     (state != DS_WorkflowDef::MUDS_SOD_STATE_OUTLET_AIR_DOOR_CLOSE_WAITING) &&
                     (state != DS_WorkflowDef::MUDS_SOD_STATE_WASTE_CONTAINER_WAITING) )
                {
                    // unexpected state
                    env->actionMgr->deleteActAll(guidSalineVolWaiting);
                    return;
                }

                if (env->ds.deviceData->getSyringeSodStartReady(SYRINGE_IDX_SALINE))
                {
                    // Finally, Saline syringe is ready to start SOD
                    if (state == DS_WorkflowDef::MUDS_SOD_STATE_SALINE_VOLUME_WAITING)
                    {
                        LOG_INFO("STATE_SALINE_VOLUME_WAITING: Saline syringe is ready to start SOD. Moving to next step..\n");
                        setState(DS_WorkflowDef::MUDS_SOD_STATE_SUDS_READY_WAITING);
                    }
                }
                else
                {
                    if (state != DS_WorkflowDef::MUDS_SOD_STATE_SALINE_VOLUME_WAITING)
                    {
                        setState(DS_WorkflowDef::MUDS_SOD_STATE_SALINE_VOLUME_WAITING);
                    }
                }
            });
            env->actionMgr->createActCompleted(guidSalineVolWaiting, conn, QString(__PRETTY_FUNCTION__) + ": STATE_SALINE_VOLUME_WAITING: FluidSourceSyringes");

            // Go to next state if possible
            if (env->ds.workflowData->getMudsSodStatus().syringeSodStatusAll[SYRINGE_IDX_SALINE].primed)
            {
                LOG_INFO("STATE_SALINE_VOLUME_WAITING: Saline syringe completed the prime. Skip the volume check for saline. Moving to next step..\n");
                setState(DS_WorkflowDef::MUDS_SOD_STATE_SUDS_READY_WAITING);
            }
            else if (env->ds.deviceData->getSyringeSodStartReady(SYRINGE_IDX_SALINE))
            {
                LOG_INFO("STATE_SALINE_VOLUME_WAITING: Saline syringe needs to start prime and it is ready. Moving to next step..\n");
                setState(DS_WorkflowDef::MUDS_SOD_STATE_SUDS_READY_WAITING);
            }
        }
        break;
    // ==========================================
    case DS_WorkflowDef::MUDS_SOD_STATE_SUDS_READY_WAITING:
        LOG_INFO("STATE_SUDS_READY_WAITING\n");
        {
            // Suds inserted callback connection - if SUDS change to missing or needsReplace state, go back to saline volume waiting state
            env->actionMgr->deleteActAll(guidSudsWaiting);
            guidSudsWaiting = Util::newGuid();
            QMetaObject::Connection conn = connect(env->ds.deviceData, &DS_DeviceData::signalDataChanged_FluidSourceSuds, [=](const DS_DeviceDef::FluidSource &fluidSourceSuds) {
                DS_WorkflowDef::MudsSodState state = (DS_WorkflowDef::MudsSodState)getState();

                if ( (state != DS_WorkflowDef::MUDS_SOD_STATE_SUDS_READY_WAITING) &&
                     (state != DS_WorkflowDef::MUDS_SOD_STATE_OUTLET_AIR_DOOR_CLOSE_WAITING) &&
                     (state != DS_WorkflowDef::MUDS_SOD_STATE_WASTE_CONTAINER_WAITING) )
                {
                    // unexpected state
                    env->actionMgr->deleteActAll(guidSudsWaiting);
                    return;
                }

                if ( (fluidSourceSuds.isInstalled()) &&
                     (!fluidSourceSuds.needsReplaced) )
                {
                    if (state == DS_WorkflowDef::MUDS_SOD_STATE_SUDS_READY_WAITING)
                    {
                        LOG_INFO("MUDS_SOD_STATE_SUDS_READY_WAITING: Suds Ready. Moving to next step..\n");
                        setState(DS_WorkflowDef::MUDS_SOD_STATE_OUTLET_AIR_DOOR_CLOSE_WAITING);
                    }
                }
                else
                {
                    if (state != DS_WorkflowDef::MUDS_SOD_STATE_SUDS_READY_WAITING)
                    {
                        setState(DS_WorkflowDef::MUDS_SOD_STATE_SUDS_READY_WAITING);
                    }
                }
            });
            env->actionMgr->createActCompleted(guidSudsWaiting, conn, QString(__PRETTY_FUNCTION__) + ": STATE_SUDS_READY_WAITING: SudsReady");

            // Go to next state if possible
            DS_DeviceDef::FluidSource fluidSourceSuds = env->ds.deviceData->getFluidSourceSuds();
            if ( (fluidSourceSuds.isInstalled()) &&
                 (!fluidSourceSuds.needsReplaced) )
            {
                LOG_INFO("MUDS_SOD_STATE_SUDS_READY_WAITING: Suds Ready. Moving to next step..\n");
                setState(DS_WorkflowDef::MUDS_SOD_STATE_OUTLET_AIR_DOOR_CLOSE_WAITING);
            }
        }
        break;
    // ==========================================
    case DS_WorkflowDef::MUDS_SOD_STATE_OUTLET_AIR_DOOR_CLOSE_WAITING:
        LOG_INFO("STATE_OUTLET_AIR_DOOR_CLOSE_WAITING\n");
        {
            // Outlet Door close callback connection - if door change to open state, go back to SUDS waiting state
            env->actionMgr->deleteActAll(guidOutletDoorWaiting);
            guidOutletDoorWaiting = Util::newGuid();
            QMetaObject::Connection conn = connect(env->ds.mcuData, &DS_McuData::signalDataChanged_OutletDoorState, this, [=](DS_McuDef::OutletDoorState outletDoorState) {
                DS_WorkflowDef::MudsSodState state = (DS_WorkflowDef::MudsSodState)getState();

                if ( (state != DS_WorkflowDef::MUDS_SOD_STATE_OUTLET_AIR_DOOR_CLOSE_WAITING) &&
                     (state != DS_WorkflowDef::MUDS_SOD_STATE_WASTE_CONTAINER_WAITING) )
                {
                    // unexpected state
                    env->actionMgr->deleteActAll(guidOutletDoorWaiting);
                    return;
                }

                if (outletDoorState == DS_McuDef::OUTLET_DOOR_STATE_CLOSED)
                {
                    LOG_INFO("STATE_OUTLET_AIR_DOOR_CLOSE_WAITING: Outlet Door Closed. Moving to next step..\n");
                    setState(DS_WorkflowDef::MUDS_SOD_STATE_WASTE_CONTAINER_WAITING);
                }
                else
                {
                    if (state != DS_WorkflowDef::MUDS_SOD_STATE_OUTLET_AIR_DOOR_CLOSE_WAITING)
                    {
                        setState(DS_WorkflowDef::MUDS_SOD_STATE_OUTLET_AIR_DOOR_CLOSE_WAITING);
                    }
                }
            });
            env->actionMgr->createActCompleted(guidOutletDoorWaiting, conn, QString(__PRETTY_FUNCTION__) + ": STATE_OUTLET_AIR_DOOR_CLOSE_WAITING: OutletDoorClosed");

            // Go to next state if possible
            if (env->ds.mcuData->getOutletDoorState() == DS_McuDef::OUTLET_DOOR_STATE_CLOSED)
            {
                LOG_INFO("STATE_OUTLET_AIR_DOOR_CLOSE_WAITING: Outlet Door Closed. Moving to next step..\n");
                setState(DS_WorkflowDef::MUDS_SOD_STATE_WASTE_CONTAINER_WAITING);
            }
        }
        break;
    // ==========================================
    case DS_WorkflowDef::MUDS_SOD_STATE_WASTE_CONTAINER_WAITING:
        LOG_INFO("STATE_WASTE_CONTAINER_WAITING\n");
        {
            // Waste Container callback connection - if WC change to bad state, go back to oulet air door waiting state
            env->actionMgr->deleteActAll(guidWasteContainerWaiting);
            guidWasteContainerWaiting = Util::newGuid();
            QMetaObject::Connection conn = connect(env->ds.deviceData, &DS_DeviceData::signalDataChanged_FluidSourceWasteContainer, this, [=](const DS_DeviceDef::FluidSource &fluidSourceWasteContainer) {
                DS_WorkflowDef::MudsSodState state = (DS_WorkflowDef::MudsSodState)getState();

                if (state != DS_WorkflowDef::MUDS_SOD_STATE_WASTE_CONTAINER_WAITING)
                {
                    // unexpected state
                    env->actionMgr->deleteActAll(guidWasteContainerWaiting);
                    return;
                }

                if (env->ds.deviceData->isFluidSourceWasteContainerReady())
                {
                    setState(DS_WorkflowDef::MUDS_SOD_STATE_SYRINGE_SOD_PREPARING);
                }
                else
                {
                    // WC state is not ready yet
                    if (state != DS_WorkflowDef::MUDS_SOD_STATE_WASTE_CONTAINER_WAITING)
                    {
                        setState(DS_WorkflowDef::MUDS_SOD_STATE_WASTE_CONTAINER_WAITING);
                    }
                }
            });
            env->actionMgr->createActCompleted(guidWasteContainerWaiting, conn, QString(__PRETTY_FUNCTION__) + ": STATE_OUTLET_AIR_DOOR_CLOSE_WAITING: OutletDoorClosed");

            // Go to next state if possible
            if (env->ds.deviceData->isFluidSourceWasteContainerReady())
            {
                setState(DS_WorkflowDef::MUDS_SOD_STATE_SYRINGE_SOD_PREPARING);
            }
        }
        break;
    // ==========================================
    case DS_WorkflowDef::MUDS_SOD_STATE_SYRINGE_SOD_PREPARING:
        LOG_INFO("STATE_SYRINGE_SOD_PREPARING\n");
        {
            env->actionMgr->deleteActAll(guidSyringesIdleWaiting);
            env->actionMgr->deleteActAll(guidSalineVolWaiting);
            env->actionMgr->deleteActAll(guidSudsWaiting);
            env->actionMgr->deleteActAll(guidOutletDoorWaiting);
            env->actionMgr->deleteActAll(guidWasteContainerWaiting);

            syringeIdxSodStart = SYRINGE_IDX_NONE;
            DS_WorkflowDef::MudsSodStatus mudsSodStatus = env->ds.workflowData->getMudsSodStatus();

            for (int syringeIdx = SYRINGE_IDX_START; syringeIdx < SYRINGE_IDX_MAX; syringeIdx++)
            {
                if (mudsSodStatus.syringeSodStatusAll[syringeIdx].isCompleted())
                {
                    // no need for syringe SOD
                }
                else if (env->ds.deviceData->getSyringeSodStartReady((SyringeIdx)syringeIdx))
                {
                    syringeIdxSodStart = (SyringeIdx)syringeIdx;
                    break;
                }
            }

            if (syringeIdxSodStart == SYRINGE_IDX_NONE)
            {
                // MUDS prime done or not needed or waiting
                if (sodPerformedSyringes.length() == 0)
                {
                    LOG_INFO("STATE_SYRINGE_SOD_PREPARING: No more syringes to start SOD. Completing SOD action..\n");
                }
                setState(DS_WorkflowDef::MUDS_SOD_STATE_DONE);
            }
            else
            {
                LOG_INFO("STATE_SYRINGE_SOD_PREPARING: Syringe[%d] SOD started\n", syringeIdxSodStart);
                sodPerformedSyringes.append(syringeIdxSodStart);
                setState(DS_WorkflowDef::MUDS_SOD_STATE_SYRINGE_SOD_STARTED);
            }
        }
        break;
    case DS_WorkflowDef::MUDS_SOD_STATE_SYRINGE_SOD_STARTED:
        LOG_INFO("STATE_SYRINGE_SOD_STARTED\n");
        handleSubAction(DS_WorkflowDef::MUDS_SOD_STATE_SYRINGE_SOD_PROGRESS, DS_WorkflowDef::MUDS_SOD_STATE_SYRINGE_SOD_DONE, DS_WorkflowDef::MUDS_SOD_STATE_SYRINGE_SOD_FAILED);
        env->ds.deviceAction->actSyringeSodStart(syringeIdxSodStart, guidSubAction);
        break;
    case DS_WorkflowDef::MUDS_SOD_STATE_SYRINGE_SOD_PROGRESS:
        LOG_INFO("STATE_SYRINGE_SOD_PROGRESS\n");
        break;
    case DS_WorkflowDef::MUDS_SOD_STATE_SYRINGE_SOD_FAILED:
        LOG_ERROR("STATE_SYRINGE_SOD_FAILED\n");
        setState(DS_WorkflowDef::MUDS_SOD_STATE_DONE);
        break;
    case DS_WorkflowDef::MUDS_SOD_STATE_SYRINGE_SOD_DONE:
        LOG_INFO("STATE_SYRINGE_SOD_DONE\n");
        setState(DS_WorkflowDef::MUDS_SOD_STATE_SYRINGE_SOD_PREPARING);
        break;
    case DS_WorkflowDef::MUDS_SOD_STATE_ABORT:
        LOG_INFO("STATE_ABORT\n");

        actStatusBuf.err = "USER_ABORT";
        actStatusBuf.state = DS_ACTION_STATE_USER_ABORT;

        if (syringeIdxSodStart != SYRINGE_IDX_NONE)
        {
            LOG_INFO("STATE_ABORT: Syringe SOD Abort for %s...\n", ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdxSodStart).CSTR());
            env->ds.deviceAction->actSyringeSodAbort(syringeIdxSodStart);

            // TODO: Emit completed signal when abort completes
        }

        actionCompleted(actStatusBuf);
        setState(DS_WorkflowDef::MUDS_SOD_STATE_READY);
        break;
    case DS_WorkflowDef::MUDS_SOD_STATE_DONE:
        LOG_INFO("STATE_DONE\n");
        {
            if (actStatusBuf.err == "")
            {
                actStatusBuf.state = DS_ACTION_STATE_COMPLETED;
            }
            actionCompleted(actStatusBuf);
            setState(DS_WorkflowDef::MUDS_SOD_STATE_READY);
        }
        break;
    default:
        LOG_ERROR("Unknown State(%s)\n", ImrParser::ToImr_MudsSodState(state).CSTR());
        break;
    }
}


