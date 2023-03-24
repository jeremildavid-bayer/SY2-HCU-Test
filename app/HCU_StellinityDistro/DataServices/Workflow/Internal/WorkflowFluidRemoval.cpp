#include "Apps/AppManager.h"
#include "Common/ImrParser.h"
#include "WorkflowFluidRemoval.h"
#include "DataServices/Workflow/DS_WorkflowData.h"
#include "DataServices/Device/DS_DeviceAction.h"
#include "DataServices/Device/DS_DeviceData.h"
#include "DataServices/Mcu/DS_McuAction.h"
#include "DataServices/System/DS_SystemAction.h"
#include "DataServices/Workflow/DS_WorkflowAction.h"
#include "DataServices/Alert/DS_AlertAction.h"

WorkflowFluidRemoval::WorkflowFluidRemoval(QObject *parent, EnvGlobal *env_) :
    ActionBaseExt(parent, env_)
{
    envLocal = new EnvLocal("DS_Workflow-FluidRemoval", "WORKFLOW_FLUID_REMOVAL");

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });

    location = SYRINGE_IDX_NONE;
}

WorkflowFluidRemoval::~WorkflowFluidRemoval()
{
    delete envLocal;
}

void WorkflowFluidRemoval::slotAppInitialised()
{
    connect(env->ds.workflowData, &DS_WorkflowData::signalDataChanged_WorkflowState, this, [=](DS_WorkflowDef::WorkflowState workflowState, DS_WorkflowDef::WorkflowState workflowStatePrev) {
        if ( (workflowState != DS_WorkflowDef::STATE_INACTIVE) &&
             (workflowStatePrev == DS_WorkflowDef::STATE_INACTIVE) )
        {
            env->ds.workflowData->setFluidRemovalState(DS_WorkflowDef::FLUID_REMOVAL_STATE_READY);
            processState();
        }
        else if ( (workflowState == DS_WorkflowDef::STATE_INACTIVE) &&
                  (workflowStatePrev != DS_WorkflowDef::STATE_INACTIVE) )
        {
            // Destroy action instances
            env->actionMgr->deleteActAll(actStatusBuf.guid);

            env->ds.workflowData->setFluidRemovalState(DS_WorkflowDef::FLUID_REMOVAL_STATE_INACTIVE);
            processState();
        }
    });
}

int WorkflowFluidRemoval::getState()
{
    return env->ds.workflowData->getFluidRemovalState();
}

QString WorkflowFluidRemoval::getStateStr(int state)
{
    return ImrParser::ToImr_FluidRemovalState((DS_WorkflowDef::FluidRemovalState)state);
}

void WorkflowFluidRemoval::setStateSynch(int newState)
{
    env->ds.workflowData->setFluidRemovalState((DS_WorkflowDef::FluidRemovalState)newState);
}

bool WorkflowFluidRemoval::isSetStateReady()
{
    if (getState() == DS_WorkflowDef::FLUID_REMOVAL_STATE_INACTIVE)
    {
        return false;
    }
    return true;
}

void WorkflowFluidRemoval::processState()
{
    DS_WorkflowDef::FluidRemovalState state = env->ds.workflowData->getFluidRemovalState();

    switch (state)
    {
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_INACTIVE:
        LOG_INFO("FLUID_REMOVAL_STATE_INACTIVE\n");
        break;
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_READY:
        LOG_INFO("FLUID_REMOVAL_STATE_READY\n");
        env->actionMgr->deleteActAll(actStatusBuf.guid);
        env->actionMgr->deleteActAll(guidSubAction);
        env->actionMgr->deleteActAll(guidSudsWaiting);
        env->actionMgr->deleteActAll(guidWasteContainerMonitor);
        break;
    // -------------------------------------------------------------
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_STARTED:
        LOG_INFO("[%d]: FLUID_REMOVAL_STATE_STARTED\n", location);
        if (env->ds.deviceData->getFluidSourceSyringesBusy(location))
        {
            setState(DS_WorkflowDef::FLUID_REMOVAL_STATE_FAILED_OTHER_SYRINGE_BUSY);
        }
        else
        {
            setState(DS_WorkflowDef::FLUID_REMOVAL_STATE_WAIT_USER_START);
        }
        break;
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_WAIT_USER_START:
        LOG_INFO("[%d]: FLUID_REMOVAL_STATE_WAIT_USER_START\n", location);
        // Wait for user interaction
        break;
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_WAIT_REMOVE_CURRENT_SUPPLY:
        LOG_INFO("[%d]: FLUID_REMOVAL_STATE_WAIT_REMOVE_CURRENT_SUPPLY\n", location);
        // Wait for user interaction
        break;
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_WAIT_SUDS_TO_WASTE_CONTAINER:
        LOG_INFO("[%d]: FLUID_REMOVAL_STATE_WAIT_SUDS_TO_WASTE_CONTAINER\n", location);
        // Wait for user interaction
        break;
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_SUDS_INSERT_WAITING:
        LOG_INFO("[%d]: FLUID_REMOVAL_STATE_SUDS_INSERT_WAITING\n", location);
        {
            // Suds Connect Waiting callback connection
            env->actionMgr->deleteActAll(guidSudsWaiting);
            guidSudsWaiting = Util::newGuid();

            QMetaObject::Connection conn = connect(env->ds.mcuData, &DS_McuData::signalDataChanged_SudsInserted, this, [=](bool sudsInserted) {
                DS_WorkflowDef::FluidRemovalState state = env->ds.workflowData->getFluidRemovalState();
                if (state != DS_WorkflowDef::FLUID_REMOVAL_STATE_SUDS_INSERT_WAITING)
                {
                    env->actionMgr->deleteActAll(guidSudsWaiting);
                    return;
                }

                if (sudsInserted)
                {
                    env->actionMgr->deleteActAll(guidSudsWaiting);
                    setState(DS_WorkflowDef::FLUID_REMOVAL_STATE_PURGE_STARTED);
                }
            });
            env->actionMgr->createActCompleted(guidSudsWaiting, conn, QString(__PRETTY_FUNCTION__) + ": FLUID_REMOVAL_STATE_SUDS_INSERT_WAITING: SudsInserted");

            // Go to next state if possible
            if (env->ds.mcuData->getSudsInserted())
            {
                setState(DS_WorkflowDef::FLUID_REMOVAL_STATE_PURGE_STARTED);
            }
        }
        break;
    // -------------------------------------------------------------
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_PURGE_STARTED:
        LOG_INFO("[%d]: FLUID_REMOVAL_STATE_PURGE_STARTED\n", location);
        {
            env->actionMgr->deleteActAll(guidSudsWaiting);

            // Check WasteContainer Level
            if (!env->ds.deviceData->isFluidSourceWasteContainerReady())
            {
                setState(DS_WorkflowDef::FLUID_REMOVAL_STATE_FAILED_WASTE_CONTAINER_NOT_READY);
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
                    setState(DS_WorkflowDef::FLUID_REMOVAL_STATE_FAILED_WASTE_CONTAINER_NOT_READY);
                    return;
                }
            });
            env->actionMgr->createActCompleted(guidWasteContainerMonitor, conn, QString(__PRETTY_FUNCTION__) + ": FLUID_REMOVAL_STATE_PURGE_STARTED: WasteContainer");


            LOG_INFO("[%d]: FLUID_REMOVAL_STATE_PURGE_STARTED: Fluid purge started. Setting SUDS to NeedsPrime state\n", location);

            if (env->ds.mcuData->getSyringeVols()[location] == 0)
            {
                setState(DS_WorkflowDef::FLUID_REMOVAL_STATE_PURGE_DONE);
            }
            else
            {
                // Set busy state to false before start prime
                DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
                fluidSourceSyringes[location].isBusy = true;
                env->ds.deviceData->setFluidSourceSyringes(fluidSourceSyringes);

                env->actionMgr->deleteActAll(guidSubAction);
                handleSubAction(DS_WorkflowDef::FLUID_REMOVAL_STATE_PURGE_PROGRESS, DS_WorkflowDef::FLUID_REMOVAL_STATE_PURGE_DONE, DS_WorkflowDef::FLUID_REMOVAL_STATE_PURGE_FAILED);
                env->ds.deviceAction->actSyringePrime(location, DS_DeviceDef::SUDS_PRIME_TYPE_FLUID_PURGE, guidSubAction);
            }
        }
        break;
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_PURGE_PROGRESS:
        LOG_INFO("[%d]: FLUID_REMOVAL_STATE_PURGE_PROGRESS\n", location);
        break;
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_PURGE_DONE:
        LOG_INFO("[%d]: FLUID_REMOVAL_STATE_PURGE_DONE\n", location);
        setState(DS_WorkflowDef::FLUID_REMOVAL_STATE_FILL_STOPCOCK_FILL_SETTING);
        break;
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_PURGE_FAILED:
        LOG_WARNING("[%d]: FLUID_REMOVAL_STATE_PURGE_FAILED: %s\n", location, ImrParser::ToImr_DataServiceActionStatusStr(actStatusBuf).CSTR());
        if (actStatusBuf.err.contains("SUDS_REMOVED"))
        {
            setState(DS_WorkflowDef::FLUID_REMOVAL_STATE_FAILED_SUDS_REMOVED);
        }
        else if (actStatusBuf.err.contains("USER_ABORT"))
        {
            setState(DS_WorkflowDef::FLUID_REMOVAL_STATE_FAILED_USER_ABORT);
        }
        else
        {
            setState(DS_WorkflowDef::FLUID_REMOVAL_STATE_FAILED_HW_FAILED);
        }
        break;
    // -------------------------------------------------------------
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_FILL_STOPCOCK_FILL_SETTING:
        LOG_INFO("[%d]: FLUID_REMOVAL_STATE_FILL_STOPCOCK_FILL_SETTING\n", location);
        handleSubAction(DS_WorkflowDef::FLUID_REMOVAL_STATE_FILL_STOPCOCK_FILL_PROGRESS, DS_WorkflowDef::FLUID_REMOVAL_STATE_FILL_STOPCOCK_FILL_DONE, DS_WorkflowDef::FLUID_REMOVAL_STATE_FILL_STOPCOCK_FILL_FAILED);
        env->ds.deviceAction->actStopcock(location, DS_McuDef::STOPCOCK_POS_FILL, STOPCOCK_ACTION_TRIALS_LIMIT, guidSubAction);
        break;
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_FILL_STOPCOCK_FILL_PROGRESS:
        LOG_INFO("[%d]: FLUID_REMOVAL_STATE_FILL_STOPCOCK_FILL_PROGRESS\n", location);
        break;
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_FILL_STOPCOCK_FILL_DONE:
        LOG_INFO("[%d]: FLUID_REMOVAL_STATE_FILL_STOPCOCK_FILL_DONE\n", location);
        setState(DS_WorkflowDef::FLUID_REMOVAL_STATE_FILL_STARTED);
        break;
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_FILL_STOPCOCK_FILL_FAILED:
        LOG_ERROR("[%d]: FLUID_REMOVAL_STATE_FILL_STOPCOCK_FILL_FAILED\n", location);
        setState(DS_WorkflowDef::FLUID_REMOVAL_STATE_FAILED_HW_FAILED);
        break;
    // -------------------------------------------------------------
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_FILL_STARTED:
        LOG_INFO("[%d]: FLUID_REMOVAL_STATE_FILL_STARTED\n", location);
        {
            env->ds.alertAction->activate("SRUClearingInletFluid", ImrParser::ToImr_FluidSourceSyringeIdx(location).CSTR());

            handleSubAction(DS_WorkflowDef::FLUID_REMOVAL_STATE_FILL_PROGRESS, DS_WorkflowDef::FLUID_REMOVAL_STATE_FILL_DONE, DS_WorkflowDef::FLUID_REMOVAL_STATE_FILL_FAILED);

            // Fill without checking any sensors - To withdraw fluid from the inlet line
            DS_McuDef::ActPistonParams params;
            params.idx = location;
            params.vol = FLUID_REMOVAL_INLET_LINE_WITHDRAW_VOL;
            params.flow = FLUID_REMOVAL_INLET_LINE_WITHDRAW_FLOW;
            env->ds.mcuAction->actPiston(params, guidSubAction);
        }
        break;
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_FILL_PROGRESS:
        LOG_INFO("[%d]: FLUID_REMOVAL_STATE_FILL_PROGRESS\n", location);
        break;
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_FILL_DONE:
        LOG_INFO("[%d]: FLUID_REMOVAL_STATE_FILL_DONE\n", location);
        setState(DS_WorkflowDef::FLUID_REMOVAL_STATE_CHECK_SPIKE_STATE_STARTED);
        break;
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_FILL_FAILED:
        LOG_ERROR("[%d]: FLUID_REMOVAL_STATE_FILL_FAILED: %s\n", location, ImrParser::ToImr_DataServiceActionStatusStr(actStatusBuf).CSTR());
        setState(DS_WorkflowDef::FLUID_REMOVAL_STATE_REMOVING_TRAPPED_FLUID_ABORTED);
        break;
    // -------------------------------------------------------------
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_CHECK_SPIKE_STATE_STARTED:
        LOG_INFO("[%d]: FLUID_REMOVAL_STATE_CHECK_SPIKE_STATE_STARTED\n", location);
        {
            DS_McuDef::BottleBubbleDetectorState bottleBubbleState = env->ds.mcuData->getBottleBubbleStates()[location];
            if (bottleBubbleState == DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_AIR)
            {
                setState(DS_WorkflowDef::FLUID_REMOVAL_STATE_FINAL_PURGE_STARTED);
            }
            else
            {
                LOG_WARNING("[%d]: FLUID_REMOVAL_STATE_CHECK_SPIKE_STATE_STARTED: Unexpected spike state(%s)\n", location, ImrParser::ToImr_BottleBubbleDetectorState(bottleBubbleState).CSTR());
                actStatusBuf.err = "SUPPLY_STILL_LOADED";
                actStatusBuf.state = DS_ACTION_STATE_INVALID_STATE;
                setState(DS_WorkflowDef::FLUID_REMOVAL_STATE_REMOVING_TRAPPED_FLUID_ABORTED_BY_SUPPLY_STILL_LOADED);
            }
        }
        break;
    // -------------------------------------------------------------
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_FINAL_PURGE_STARTED:
        LOG_INFO("[%d]: FLUID_REMOVAL_STATE_FINAL_PURGE_STARTED\n", location);
        handleSubAction(DS_WorkflowDef::FLUID_REMOVAL_STATE_FINAL_PURGE_PROGRESS, DS_WorkflowDef::FLUID_REMOVAL_STATE_FINAL_PURGE_DONE, DS_WorkflowDef::FLUID_REMOVAL_STATE_FINAL_PURGE_FAILED);
        env->ds.deviceAction->actSyringePrime(location, DS_DeviceDef::SUDS_PRIME_TYPE_FLUID_PURGE, guidSubAction);
        break;
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_FINAL_PURGE_PROGRESS:
        LOG_INFO("[%d]: FLUID_REMOVAL_STATE_FINAL_PURGE_PROGRESS\n", location);
        break;
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_FINAL_PURGE_DONE:
        LOG_INFO("[%d]: FLUID_REMOVAL_STATE_FINAL_PURGE_DONE\n", location);
        {
            // We finished purging out the reservoir, so we can clear this alert        
            env->ds.alertAction->deactivateFromSyringeIdx("SRUClearingInletFluid", location);

            actStatusBuf = actStatusBeforeResumed;
            if (actStatusBuf.err == "")
            {
                setState(DS_WorkflowDef::FLUID_REMOVAL_STATE_DONE);
            }
            else
            {
                setState(DS_WorkflowDef::FLUID_REMOVAL_STATE_FAILED_DURING_REMOVING_TRAPPED_FLUID);
            }
        }
        break;
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_FINAL_PURGE_FAILED:
        LOG_ERROR("[%d]: FLUID_REMOVAL_STATE_FINAL_PURGE_FAILED: %s\n", location, ImrParser::ToImr_DataServiceActionStatusStr(actStatusBuf).CSTR());
        setState(DS_WorkflowDef::FLUID_REMOVAL_STATE_REMOVING_TRAPPED_FLUID_ABORTED);
        break;
    // -------------------------------------------------------------
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_FAILED_INVALID_STATE:
        LOG_ERROR("[%d]: FLUID_REMOVAL_STATE_FAILED_INVALID_STATE\n", location);
        setState(DS_WorkflowDef::FLUID_REMOVAL_STATE_FAILED);
        break;
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_FAILED_OTHER_SYRINGE_BUSY:
        LOG_ERROR("[%d]: FLUID_REMOVAL_STATE_FAILED_OTHER_SYRINGE_BUSY\n", location);
        actStatusBuf.err = "Syringes Busy";
        actStatusBuf.state = DS_ACTION_STATE_INVALID_STATE;
        setState(DS_WorkflowDef::FLUID_REMOVAL_STATE_FAILED);
        break;
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_FAILED_SUDS_REMOVED:
        LOG_ERROR("[%d]: FLUID_REMOVAL_STATE_FAILED_SUDS_REMOVED\n", location);
        setState(DS_WorkflowDef::FLUID_REMOVAL_STATE_FAILED);
        break;
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_FAILED_WASTE_CONTAINER_NOT_READY:
        LOG_ERROR("[%d]: FLUID_REMOVAL_STATE_FAILED_WASTE_CONTAINER_NOT_READY\n", location);
        env->ds.mcuAction->actStopAll();
        env->actionMgr->deleteActAll(guidWasteContainerMonitor);
        setState(DS_WorkflowDef::FLUID_REMOVAL_STATE_FAILED);
        break;
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_FAILED_USER_ABORT:
        LOG_ERROR("[%d]: FLUID_REMOVAL_STATE_FAILED_USER_ABORT\n", location);
        setState(DS_WorkflowDef::FLUID_REMOVAL_STATE_FAILED);
        break;
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_FAILED_HW_FAILED:
        LOG_ERROR("[%d]: FLUID_REMOVAL_STATE_FAILED_HW_FAILED\n", location);
        setState(DS_WorkflowDef::FLUID_REMOVAL_STATE_FAILED);
        break;

    // -------------------------------------------------------------
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_REMOVING_TRAPPED_FLUID_ABORTED:
        LOG_WARNING("[%d]: FLUID_REMOVAL_STATE_REMOVING_TRAPPED_FLUID_ABORTED: %s\n", location, ImrParser::ToImr_DataServiceActionStatusStr(actStatusBuf).CSTR());
        if ( (actStatusBuf.err.contains("USER_ABORT")) ||
             (actStatusBuf.err.contains("SUDS_REMOVED")) )
        {
            setState(DS_WorkflowDef::FLUID_REMOVAL_STATE_REMOVING_TRAPPED_FLUID_ABORTED_BY_USER_STOP);
        }
        else
        {
            setState(DS_WorkflowDef::FLUID_REMOVAL_STATE_REMOVING_TRAPPED_FLUID_ABORTED_BY_HW_FAILED);
        }
        break;
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_REMOVING_TRAPPED_FLUID_ABORTED_BY_USER_STOP:
        LOG_WARNING("[%d]: FLUID_REMOVAL_STATE_REMOVING_TRAPPED_FLUID_ABORTED_BY_USER_STOP\n", location);
        // Wait for user interaction
        break;
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_REMOVING_TRAPPED_FLUID_ABORTED_BY_SUPPLY_STILL_LOADED:
        LOG_WARNING("[%d]: FLUID_REMOVAL_STATE_REMOVING_TRAPPED_FLUID_ABORTED_BY_SUPPLY_STILL_LOADED\n", location);
        // Wait for user interaction
        break;
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_REMOVING_TRAPPED_FLUID_ABORTED_BY_HW_FAILED:
        LOG_WARNING("[%d]: FLUID_REMOVAL_STATE_REMOVING_TRAPPED_FLUID_ABORTED_BY_HW_FAILED\n", location);
        // Wait for user interaction
        break;
    // -------------------------------------------------------------
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_FAILED:
        LOG_ERROR("[%d]: FLUID_REMOVAL_STATE_FAILED: %s\n", location, ImrParser::ToImr_DataServiceActionStatusStr(actStatusBuf).CSTR());
        {
            DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
            fluidSourceSyringes[location].isBusy = false;
            env->ds.deviceData->setFluidSourceSyringes(fluidSourceSyringes);

            actionCompleted(actStatusBuf);
            setState(DS_WorkflowDef::FLUID_REMOVAL_STATE_READY);
        }
        break;
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_FAILED_DURING_REMOVING_TRAPPED_FLUID:
        LOG_ERROR("[%d]: FLUID_REMOVAL_STATE_FAILED_DURING_REMOVING_TRAPPED_FLUID: %s\n", location, ImrParser::ToImr_DataServiceActionStatusStr(actStatusBuf).CSTR());
        {
            if (!isAutoEmpty)
            {
                DS_WorkflowDef::MudsSodStatus mudsSodStatus = env->ds.workflowData->getMudsSodStatus();
                mudsSodStatus.syringeSodStatusAll[location].primed = false;
                mudsSodStatus.syringeSodStatusAll[location].airCheckCalibrated = false;
                env->ds.workflowData->setMudsSodStatus(mudsSodStatus);
            }
            setState(DS_WorkflowDef::FLUID_REMOVAL_STATE_FAILED);
        }
        break;
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_DONE:
        LOG_INFO("[%d]: FLUID_REMOVAL_STATE_DONE\n", location);
        {
            DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
            fluidSourceSyringes[location].isBusy = false;
            env->ds.deviceData->setFluidSourceSyringes(fluidSourceSyringes);

            // Removal completed successfully
            env->ds.deviceAction->actDeactivateReservoirAlertsOnEmptied(location);

            actStatusBuf.state = DS_ACTION_STATE_COMPLETED;

            // Allow a different concentration of compatible type to be loaded
            env->ds.deviceAction->actBottleUnload(location);

            if (!isAutoEmpty)
            {
                // Mark the reservoir to require a new prime and air check cal once loaded
                DS_WorkflowDef::MudsSodStatus mudsSodStatus = env->ds.workflowData->getMudsSodStatus();
                mudsSodStatus.syringeSodStatusAll[location].primed = false;
                mudsSodStatus.syringeSodStatusAll[location].airCheckCalibrated = false;
                env->ds.workflowData->setMudsSodStatus(mudsSodStatus);
            }

            actionCompleted(actStatusBuf);
            setState(DS_WorkflowDef::FLUID_REMOVAL_STATE_READY);
        }
        break;
    default:
        LOG_ERROR("[%d]: Invalid State (%s)\n", location, ImrParser::ToImr_FluidRemovalState(state).CSTR());
        break;
    }
}

DataServiceActionStatus WorkflowFluidRemoval::actFluidRemovalStart(SyringeIdx syringeIdx, bool isAutoEmpty, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "FluidRemoval:Start", QString().asprintf("%s", ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdx).CSTR()));

    DS_WorkflowDef::FluidRemovalState fluidRemovalState = env->ds.workflowData->getFluidRemovalState();

    if (fluidRemovalState != DS_WorkflowDef::FLUID_REMOVAL_STATE_READY)
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = QString().asprintf("FluidRemovalState=%s", ImrParser::ToImr_FluidRemovalState(fluidRemovalState).CSTR());
        setState(DS_WorkflowDef::FLUID_REMOVAL_STATE_FAILED_INVALID_STATE);
        actionStarted(status);
        return status;
    }

    // Ready to start
    location = syringeIdx;
    actStatusBuf = status;
    actStatusBuf.state = DS_ACTION_STATE_STARTED;
    actStatusBeforeResumed = actStatusBuf;
    actionStarted(actStatusBuf);

    this->isAutoEmpty = isAutoEmpty;

    if (isAutoEmpty)
    {
        // AutoEmpty mode. Skip user interaction
        setState(DS_WorkflowDef::FLUID_REMOVAL_STATE_PURGE_STARTED);
    }
    else
    {
        setState(DS_WorkflowDef::FLUID_REMOVAL_STATE_STARTED);
    }

    return actStatusBuf;
}

DataServiceActionStatus WorkflowFluidRemoval::actFluidRemovalResume(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "FluidRemoval:Resume");

    DS_WorkflowDef::WorkflowState workflowState = env->ds.workflowData->getWorkflowState();
    if ( (workflowState != DS_WorkflowDef::STATE_FLUID_REMOVAL_STARTED) &&
         (workflowState != DS_WorkflowDef::STATE_FLUID_REMOVAL_PROGRESS) &&
         // Fluid Removal can happen inside auto-empty
         (workflowState != DS_WorkflowDef::STATE_AUTO_EMPTY_STARTED) &&
         (workflowState != DS_WorkflowDef::STATE_AUTO_EMPTY_PROGRESS))
    {
        status.err = "Invalid Workflow State: " + ImrParser::ToImr_WorkFlowState(workflowState);
        status.state = DS_ACTION_STATE_INVALID_STATE;
        actionStarted(status);
        return status;
    }

    DS_WorkflowDef::FluidRemovalState state = env->ds.workflowData->getFluidRemovalState();
    DS_WorkflowDef::FluidRemovalState newState;

    switch (state)
    {
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_WAIT_USER_START:
        newState = DS_WorkflowDef::FLUID_REMOVAL_STATE_WAIT_REMOVE_CURRENT_SUPPLY;
        break;
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_WAIT_REMOVE_CURRENT_SUPPLY:
        newState = DS_WorkflowDef::FLUID_REMOVAL_STATE_WAIT_SUDS_TO_WASTE_CONTAINER;
        break;
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_WAIT_SUDS_TO_WASTE_CONTAINER:
        newState = DS_WorkflowDef::FLUID_REMOVAL_STATE_SUDS_INSERT_WAITING;
        break;
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_REMOVING_TRAPPED_FLUID_ABORTED_BY_USER_STOP:
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_REMOVING_TRAPPED_FLUID_ABORTED_BY_SUPPLY_STILL_LOADED:
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_REMOVING_TRAPPED_FLUID_ABORTED_BY_HW_FAILED:
        actStatusBeforeResumed = actStatusBuf;
        // Do not clear the error state as the Fluid Removal is not successful
        // The user must initiate the fluid removal process again to clear inlet
        // line and load new concentration of compatible type
        newState = DS_WorkflowDef::FLUID_REMOVAL_STATE_FINAL_PURGE_STARTED;
        break;
    default:
        status.err = "Invalid Fluid Removal State: " + ImrParser::ToImr_FluidRemovalState(state);
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

DataServiceActionStatus WorkflowFluidRemoval::actFluidRemovalAbort(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "FluidRemoval:Abort");

    DS_WorkflowDef::WorkflowState workflowState = env->ds.workflowData->getWorkflowState();
    if ( (workflowState != DS_WorkflowDef::STATE_FLUID_REMOVAL_STARTED) &&
         (workflowState != DS_WorkflowDef::STATE_FLUID_REMOVAL_PROGRESS) &&
         // Fluid Removal can happen inside auto-empty
         (workflowState != DS_WorkflowDef::STATE_AUTO_EMPTY_STARTED) &&
         (workflowState != DS_WorkflowDef::STATE_AUTO_EMPTY_PROGRESS))
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "Invalid Workflow State: " + ImrParser::ToImr_WorkFlowState(workflowState);
        actionStarted(status);
        return status;
    }

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);

    DS_WorkflowDef::FluidRemovalState state = env->ds.workflowData->getFluidRemovalState();

    LOG_INFO("actFluidRemovalAbort(): Fluid Removal aborted while state=%s, lastActionStatus=%s\n",
             ImrParser::ToImr_FluidRemovalState(state).CSTR(),
             ImrParser::ToImr_DataServiceActionStatusStr(actStatusBuf).CSTR());

    if ( (state == DS_WorkflowDef::FLUID_REMOVAL_STATE_PURGE_PROGRESS) ||
         (state == DS_WorkflowDef::FLUID_REMOVAL_STATE_FILL_PROGRESS) ||
         (state == DS_WorkflowDef::FLUID_REMOVAL_STATE_FINAL_PURGE_PROGRESS) )
    {
        env->ds.mcuAction->actStopAll();
    }

    if (state == DS_WorkflowDef::FLUID_REMOVAL_STATE_REMOVING_TRAPPED_FLUID_ABORTED_BY_HW_FAILED)
    {
        LOG_WARNING("actFluidRemovalAbort(): Fluid Removal aborted while HW Failed. Marking as SOD required..\n");
    }
    else
    {
        actStatusBuf.state = DS_ACTION_STATE_ALLSTOP_BTN_ABORT;
        actStatusBuf.err = "User Aborted";
    }

    // Abort Fluid Removal
    setState(DS_WorkflowDef::FLUID_REMOVAL_STATE_FAILED);

    // Abort Done
    status.state = DS_ACTION_STATE_COMPLETED;
    actionCompleted(status);

    return status;
}
