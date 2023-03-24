#include "Apps/AppManager.h"
#include "DeviceSyringeSod.h"
#include "DataServices/Workflow/DS_WorkflowData.h"
#include "DataServices/Alert/DS_AlertAction.h"
#include "DataServices/Device/DS_DeviceAction.h"
#include "DataServices/Device/DS_DeviceData.h"
#include "DataServices/Mcu/DS_McuAction.h"
#include "DataServices/Mcu/DS_McuData.h"
#include "DataServices/Capabilities/DS_Capabilities.h"

DeviceSyringeSod::DeviceSyringeSod(QObject *parent, EnvGlobal *env_, EnvLocal *envLocal_, SyringeIdx location_) :
    ActionBaseExt(parent, env_),
    location(location_)
{
    envLocal = envLocal_;

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });

    state = STATE_READY;
}

DeviceSyringeSod::~DeviceSyringeSod()
{
}

void DeviceSyringeSod::slotAppInitialised()
{
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
            env->actionMgr->deleteActAll(guidSyringesIdleWaiting);
            env->actionMgr->deleteActAll(guidSudsWaiting);
            env->actionMgr->deleteActAll(guidSubAction);

            state = STATE_INACTIVE;
        }
    });
}

bool DeviceSyringeSod::isBusy()
{
    return state != STATE_READY;
}

DataServiceActionStatus DeviceSyringeSod::actSodStart(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "SodStart", QString().asprintf("%d", location));

    if (isBusy())
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "SOD In Progress";
        actionStarted(status);
        return status;
    }

    actStatusBuf = status;
    actStatusBuf.state = DS_ACTION_STATE_STARTED;
    actionStarted(actStatusBuf);

    setState(STATE_STARTED);

    return actStatusBuf;
}

DataServiceActionStatus DeviceSyringeSod::actSodAbort(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "SodAbort", QString().asprintf("%d", location));

    if (!isBusy())
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "SOD Not In Progress";
        actionStarted(status);
        return status;
    }

    actionStarted(status);

    env->ds.mcuAction->actStopAll();
    if ( (state == STATE_SYRINGES_IDLE_WAITING) ||
         (state == STATE_SUDS_INSERT_WAITING) )
    {
        setState(STATE_ABORT);
    }
    else
    {
        LOG_INFO("actSodAbort(): Abort requested..\n");
    }
    abortRequested = true;

    // TODO: Emit completed signal when abort completes
    actionCompleted(status);
    return status;
}

void DeviceSyringeSod::processState()
{
    switch (state)
    {
    case STATE_READY:
        LOG_INFO("SOD[%d]: STATE_READY\n", location);
        {
            DS_WorkflowDef::MudsSodStatus mudsSodStatus = env->ds.workflowData->getMudsSodStatus();
            mudsSodStatus.syringeSodStatusAll[location].actionInProgress = false;
            env->ds.workflowData->setMudsSodStatus(mudsSodStatus);
        }
        break;
    case STATE_STARTED:
        LOG_INFO("SOD[%d]: STATE_STARTED\n", location);
        abortRequested = false;
        setState(STATE_SYRINGES_IDLE_WAITING);
        break;
    case STATE_SYRINGES_IDLE_WAITING:
        LOG_INFO("SOD[%d]: STATE_SYRINGES_IDLE_WAITING\n", location);
        {
            // Set ActionInProgress=false
            DS_WorkflowDef::MudsSodStatus mudsSodStatus = env->ds.workflowData->getMudsSodStatus();
            mudsSodStatus.syringeSodStatusAll[location].actionInProgress = false;
            env->ds.workflowData->setMudsSodStatus(mudsSodStatus);

            // Fluids ready callback connection - if fluids change to filling action state, go back to fluids waiting state
            env->actionMgr->deleteActAll(guidSyringesIdleWaiting);
            guidSyringesIdleWaiting = Util::newGuid();

            QMetaObject::Connection conn = connect(env->ds.deviceData, &DS_DeviceData::signalDataChanged_FluidSourceSyringes, this, [=] {
                if ( (state != STATE_SYRINGES_IDLE_WAITING) &&
                     (state != STATE_SUDS_INSERT_WAITING) )
                {
                    // unexpected state
                    env->actionMgr->deleteActAll(guidSyringesIdleWaiting);
                    return;
                }

                if (env->ds.deviceData->getFluidSourceSyringesBusy(location))
                {
                    // Wait until syringe become ready
                    if (state == STATE_SUDS_INSERT_WAITING)
                    {
                        setState(STATE_SYRINGES_IDLE_WAITING);
                    }
                }
                else
                {
                    if (state == STATE_SYRINGES_IDLE_WAITING)
                    {
                        setState(STATE_SUDS_INSERT_WAITING);
                    }
                }
            });
            env->actionMgr->createActCompleted(guidSyringesIdleWaiting, conn, QString(__PRETTY_FUNCTION__) + ": STATE_SYRINGES_IDLE_WAITING: FluidSourceSyringes");

            // Go to next state if possible
            if (!env->ds.deviceData->getFluidSourceSyringesBusy(location))
            {
                setState(STATE_SUDS_INSERT_WAITING);
            }
        }
        break;
    case STATE_SUDS_INSERT_WAITING:
        LOG_INFO("SOD[%d]: STATE_SUDS_INSERT_WAITING\n", location);
        {
            // Set ActionInProgress=false
            DS_WorkflowDef::MudsSodStatus mudsSodStatus = env->ds.workflowData->getMudsSodStatus();
            mudsSodStatus.syringeSodStatusAll[location].actionInProgress = false;
            env->ds.workflowData->setMudsSodStatus(mudsSodStatus);

            env->actionMgr->deleteActAll(guidSudsWaiting);

            // Suds Connect Waiting callback connection
            guidSudsWaiting = Util::newGuid();

            QMetaObject::Connection conn1 = connect(env->ds.mcuData, &DS_McuData::signalDataChanged_SudsInserted, this, [=](bool sudsInserted) {
                if (state != STATE_SUDS_INSERT_WAITING)
                {
                    env->actionMgr->deleteActAll(guidSudsWaiting);
                    return;
                }

                if (sudsInserted)
                {
                    setState(STATE_CAL_SLACK_STARTED);
                }
            });
            env->actionMgr->createActCompleted(guidSudsWaiting, conn1, QString(__PRETTY_FUNCTION__) + ": STATE_SUDS_INSERT_WAITING: SudsInserted");
		
            // Go to next state if possible

            if (env->ds.mcuData->getSudsInserted())
            {
                setState(STATE_CAL_SLACK_STARTED);
            }
        }
        break;
	// ==========================================
    // Slack Calibration Sequence
    case STATE_CAL_SLACK_STARTED:
        LOG_INFO("SOD[%d]: STATE_CAL_SLACK_STARTED\n", location);
        {
            // Wait for a while then start the cal slack. This is to avoid FluidSource callback get/set conflict from previous states.
            env->actionMgr->deleteActAll(guidSyringesIdleWaiting);
            env->actionMgr->deleteActAll(guidSudsWaiting);

            // Set ActionInProgress=true
            DS_WorkflowDef::MudsSodStatus mudsSodStatus = env->ds.workflowData->getMudsSodStatus();
            mudsSodStatus.syringeSodStatusAll[location].actionInProgress = true;
            env->ds.workflowData->setMudsSodStatus(mudsSodStatus);

            // Set syringe state to busy
            DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
            fluidSourceSyringes[location].isBusy = true;
            env->ds.deviceData->setFluidSourceSyringes(fluidSourceSyringes);

            if (mudsSodStatus.syringeSodStatusAll[location].calSlackDone)
            {
                LOG_INFO("SOD[%d]: STATE_CAL_SLACK_STARTED: CalSlack is not required. Moving to next step..\n", location);
                setState(STATE_CAL_SLACK_DONE);
                return;
            }

            env->ds.alertAction->activate("SRUCalibratingMUDS", ImrParser::ToImr_FluidSourceSyringeIdx(location));

            handleSubAction(STATE_CAL_SLACK_PROGRESS, STATE_CAL_SLACK_DONE, STATE_CAL_SLACK_FAILED);

            env->ds.mcuAction->actCalSlack(location, guidSubAction);
        }
        break;
    case STATE_CAL_SLACK_PROGRESS:
        LOG_INFO("SOD[%d]: STATE_CAL_SLACK_PROGRESS\n", location);
        break;
    case STATE_CAL_SLACK_FAILED:
        LOG_ERROR("SOD[%d]: STATE_CAL_SLACK_FAILED: Status=%s\n", location, ImrParser::ToImr_DataServiceActionStatusStr(actStatusBuf).CSTR());
        env->ds.alertAction->deactivate("SRUCalibratingMUDS");
        env->ds.alertAction->activate("CalSlackFailed", ImrParser::ToImr_FluidSourceSyringeIdx(location));
        env->ds.workflowData->setSodErrorState(DS_WorkflowDef::SOD_ERROR_STATE_CAL_SLACK_FAILED);
        setState(STATE_FAILED);
        break;
    case STATE_CAL_SLACK_DONE:
        LOG_INFO("SOD[%d]: STATE_CAL_SLACK_DONE\n", location);
        {
            env->ds.alertAction->deactivate("SRUCalibratingMUDS");

            DS_WorkflowDef::MudsSodStatus mudsSodStatus = env->ds.workflowData->getMudsSodStatus();
            mudsSodStatus.syringeSodStatusAll[location].calSlackDone = true;
            env->ds.workflowData->setMudsSodStatus(mudsSodStatus);

            primeTrialIndex = 0;
            setState(STATE_PRIME_STARTED);
        }
        break;
    // ==========================================
    case STATE_PRIME_STARTED:
        LOG_INFO("SOD[%d]: STATE_PRIME_STARTED\n", location);
        {
            env->actionMgr->deleteActAll(guidSyringesIdleWaiting);
            env->actionMgr->deleteActAll(guidSudsWaiting);

            if (abortRequested)
            {
                LOG_WARNING("SOD[%d]: STATE_PRIME_STARTED: Abort Requested Before. Aborting..\n", location);
                setState(STATE_ABORT);
                return;
            }

            bool sudsBubbleDetected = env->ds.mcuData->getSudsBubbleDetected();
            DS_WorkflowDef::MudsSodStatus mudsSodStatus = env->ds.workflowData->getMudsSodStatus();

            LOG_DEBUG("SOD[%d]: STATE_PRIME_STARTED: sudsBubbleDetected=%s, primed=%s\n",
                     location,
                     sudsBubbleDetected ? "TRUE" : "FALSE",
                     mudsSodStatus.syringeSodStatusAll[location].primed ? "TRUE" : "FALSE");

            if ( (!sudsBubbleDetected) &&
                 (mudsSodStatus.syringeSodStatusAll[location].primed) )
            {
                LOG_INFO("SOD[%d]: STATE_PRIME_STARTED: Prime is not required. Moving to next step..\n", location);
                setState(STATE_PRIME_DONE);
                return;
            }

            // See if ready to prime
            if (env->ds.deviceData->getFluidSourceSyringesBusy(location))
            {
                LOG_WARNING("SOD[%d]: STATE_PRIME_STARTED: At least one syringe is busy, Resume when syringes are idle\n", location);
                setState(STATE_SYRINGES_IDLE_WAITING);
                return;
            }

            // Start prime
            handleSubAction(STATE_PRIME_PROGRESS, STATE_PRIME_DONE, STATE_PRIME_FAILED);
            env->ds.deviceAction->actSyringePrime(location, DS_DeviceDef::SUDS_PRIME_TYPE_SYRINGE_PRIME, guidSubAction);
        }
        break;
    case STATE_PRIME_PROGRESS:
        LOG_INFO("SOD[%d]: STATE_PRIME_PROGRESS\n", location);
        break;
    case STATE_PRIME_FAILED:
        LOG_ERROR("SOD[%d]: STATE_PRIME_FAILED\n", location);
        {
            if (actStatusBuf.err.contains("INSUFFICIENT_FLUID"))
            {
                // Prime is not started yet.
            }
            else
            {
                env->ds.alertAction->activate("MudsPrimeFailed", ImrParser::ToImr_FluidSourceSyringeIdx(location));
            }

            // Update SOD Error State. This shall be handled in UI to give proper status information and instruction.
            DS_WorkflowDef::SodErrorState sodErrorState;
            if (actStatusBuf.err.contains("USER_ABORT"))
            {
                sodErrorState = DS_WorkflowDef::SOD_ERROR_STATE_SYRINGE_PRIME_ABORT;
            }
            else if (actStatusBuf.err.contains("SUDS_REMOVED"))
            {
                sodErrorState = DS_WorkflowDef::SOD_ERROR_STATE_SYRINGE_PRIME_FAILED_SUDS_REMOVED;
            }
            else if (actStatusBuf.err.contains("INSUFFICIENT_FLUID"))
            {
                sodErrorState = DS_WorkflowDef::SOD_ERROR_STATE_SYRINGE_PRIME_FAILED_INSUFFICIENT_VOLUME;
            }
            else if (actStatusBuf.err.contains("AIR_DETECTED"))
            {
                sodErrorState = DS_WorkflowDef::SOD_ERROR_STATE_SYRINGE_PRIME_FAILED_AIR_DETECTED;
            }
            else
            {
                sodErrorState = DS_WorkflowDef::SOD_ERROR_STATE_SYRINGE_PRIME_FAILED_HW_FAULT;
            }
            env->ds.workflowData->setSodErrorState(sodErrorState);
            setState(STATE_FAILED);
        }
        break;
    case STATE_PRIME_DONE:
        LOG_INFO("SOD[%d]: STATE_PRIME_DONE\n", location);
        setState(STATE_OUTLET_AIR_CHECK_STARTED);
        break;
    // ==========================================
    case STATE_OUTLET_AIR_CHECK_STARTED:
        LOG_INFO("SOD[%d]: STATE_OUTLET_AIR_CHECK_STARTED\n", location);
        {
            bool sudsBubbleDetected = env->ds.mcuData->getSudsBubbleDetected();
            LOG_INFO("SOD[%d]: STATE_OUTLET_AIR_CHECK_STARTED: SUDS Bubble Detected=%s\n", location, sudsBubbleDetected ? "TRUE" : "FALSE");
            if (sudsBubbleDetected)
            {
                setState(STATE_OUTLET_AIR_CHECK_FAILED);
            }
            else
            {
                setState(STATE_OUTLET_AIR_CHECK_DONE);
            }
        }
        break;
    case STATE_OUTLET_AIR_CHECK_FAILED:
        LOG_ERROR("SOD[%d]: STATE_OUTLET_AIR_CHECK_FAILED\n", location);
        primeTrialIndex++;
        if (primeTrialIndex < SOD_SYRINGE_PRIME_TRIALS_LIMIT)
        {
            LOG_WARNING("SOD[%d]: PrimeTrial[%d]: Air detected from outlet air sensor. Retry Priming again..\n", location, primeTrialIndex);
            setState(STATE_PRIME_STARTED);
        }
        else
        {
            actStatusBuf.err = "AIR_DETECTED";
            actStatusBuf.state = DS_ACTION_STATE_INTERNAL_ERR;
            setState(STATE_PRIME_FAILED);
        }
        break;
    case STATE_OUTLET_AIR_CHECK_DONE:
        LOG_INFO("SOD[%d]: STATE_OUTLET_AIR_CHECK_DONE\n", location);
        {
            env->ds.alertAction->deactivate("MudsPrimeFailed", ImrParser::ToImr_FluidSourceSyringeIdx(location));

            DS_WorkflowDef::MudsSodStatus mudsSodStatus = env->ds.workflowData->getMudsSodStatus();
            mudsSodStatus.syringeSodStatusAll[location].primed = true;
            env->ds.workflowData->setMudsSodStatus(mudsSodStatus);
            setState(STATE_CAL_SYRINGE_AIR_CHECK_STARTED);
        }
        break;
    // ==========================================
    // Reservoir Air Check Calibration Sequence Starts
    case STATE_CAL_SYRINGE_AIR_CHECK_STARTED:
        LOG_INFO("SOD[%d]: STATE_CAL_SYRINGE_AIR_CHECK_STARTED\n", location);
        {
            if (abortRequested)
            {
                LOG_WARNING("SOD[%d]: STATE_CAL_SYRINGE_AIR_CHECK_STARTED: Abort Requested Before. Aborting..\n", location);
                setState(STATE_ABORT);
                return;
            }

            DS_WorkflowDef::MudsSodStatus mudsSodStatus = env->ds.workflowData->getMudsSodStatus();
            if (mudsSodStatus.syringeSodStatusAll[location].airCheckCalibrated)
            {
                LOG_INFO("SOD[%d]: STATE_CAL_SYRINGE_AIR_CHECK_STARTED: AirCheck Calibration is not required. Moving to next step..\n", location);
                env->ds.alertAction->deactivateFromSyringeIdx("ReservoirAirCheckCalFailed", location);
                setState(STATE_CAL_SYRINGE_AIR_CHECK_DONE);
                return;
            }

            // Set syringe state to busy
            DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
            fluidSourceSyringes[location].isBusy = true;
            env->ds.deviceData->setFluidSourceSyringes(fluidSourceSyringes);

            // Volume check
            double requiredVolume = env->ds.capabilities->get_AirCheck_SyringeAirCheckRequiredVol();
            double syringeVolume = fluidSourceSyringes[location].currentVolumesTotal();
            if (Util::isFloatVarGreaterThan(requiredVolume, syringeVolume, 1))
            {
                LOG_WARNING("SOD[%d]: STATE_CAL_SYRINGE_AIR_CHECK_STARTED: Insufficient Fluid (%.1fml < %.1fml)\n", location, fluidSourceSyringes[location].currentVolumesTotal(), requiredVolume);
                actStatusBuf.err = "INSUFFICIENT_FLUID";
                actStatusBuf.state = DS_ACTION_STATE_INVALID_STATE;
                setState(STATE_CAL_SYRINGE_AIR_CHECK_FAILED);
                return;
            }

            // See if ready to prime
            if (env->ds.deviceData->getFluidSourceSyringesBusy(location))
            {
                LOG_WARNING("SOD[%d]: STATE_CAL_SYRINGE_AIR_CHECK_STARTED: At least one syringe is busy, Resume when syringes are idle\n", location);
                setState(STATE_SYRINGES_IDLE_WAITING);
                return;
            }

            QString alertData = QString().asprintf("%s;%.1f", ImrParser::ToImr_FluidSourceSyringeIdx(location).CSTR(), syringeVolume);
            env->ds.alertAction->activate("SRUReservoirAirCheckCalStarted", alertData);
            env->ds.alertAction->activate("SRUAirCheckingMUDS", ImrParser::ToImr_FluidSourceSyringeIdx(location));
            env->ds.alertAction->deactivateFromSyringeIdx("ReservoirAirCheckCalFailed", location);
            handleSubAction(STATE_CAL_SYRINGE_AIR_CHECK_PROGRESS, STATE_CAL_SYRINGE_AIR_CHECK_DONE, STATE_CAL_SYRINGE_AIR_CHECK_FAILED);
            env->ds.mcuAction->actCalSyringeAirCheck(location, guidSubAction);
        }
        break;
    case STATE_CAL_SYRINGE_AIR_CHECK_PROGRESS:
        LOG_INFO("SOD[%d]: STATE_CAL_SYRINGE_AIR_CHECK_PROGRESS\n", location);
        break;
    case STATE_CAL_SYRINGE_AIR_CHECK_FAILED:
        LOG_ERROR("SOD[%d]: STATE_CAL_SYRINGE_AIR_CHECK_FAILED: Error=%s\n", location, actStatusBuf.err.CSTR());
        {
            QString alertData = QString().asprintf("%s;%s", ImrParser::ToImr_FluidSourceSyringeIdx(location).CSTR(), actStatusBuf.err.CSTR());
            env->ds.alertAction->activate("ReservoirAirCheckCalFailed", alertData);

            DS_WorkflowDef::SodErrorState sodErrorState;
            if (actStatusBuf.err == "INSUFFICIENT_FLUID")
            {
                sodErrorState = DS_WorkflowDef::SOD_ERROR_STATE_CAL_SYRINGE_AIR_CHECK_FAILED_INSUFFICIENT_VOLUME;
            }
            else if (actStatusBuf.err == "BAD_DATA")
            {
                sodErrorState = DS_WorkflowDef::SOD_ERROR_STATE_CAL_SYRINGE_AIR_CHECK_FAILED_BAD_DATA;
            }
            else
            {
                sodErrorState = DS_WorkflowDef::SOD_ERROR_STATE_CAL_SYRINGE_AIR_CHECK_FAILED_HW_FAULT;
            }

            env->ds.workflowData->setSodErrorState(sodErrorState);

            env->ds.alertAction->deactivate("SRUAirCheckingMUDS", ImrParser::ToImr_FluidSourceSyringeIdx(location));
            setState(STATE_FAILED);
        }
        break;
    case STATE_CAL_SYRINGE_AIR_CHECK_DONE:
        LOG_INFO("SOD[%d]: STATE_CAL_SYRINGE_AIR_CHECK_DONE\n", location);
        {
            DS_WorkflowDef::MudsSodStatus mudsSodStatus = env->ds.workflowData->getMudsSodStatus();
            mudsSodStatus.syringeSodStatusAll[location].airCheckCalibrated = true;
            env->ds.workflowData->setMudsSodStatus(mudsSodStatus);

            env->ds.alertAction->deactivate("SRUAirCheckingMUDS", ImrParser::ToImr_FluidSourceSyringeIdx(location));
            setState(STATE_CAL_SYRINGE_GET_AIR_CHECK_COEFF_STARTED);
        }
        break;
    // ==========================================
    case STATE_CAL_SYRINGE_GET_AIR_CHECK_COEFF_STARTED:
        LOG_INFO("SOD[%d]: STATE_CAL_SYRINGE_GET_AIR_CHECK_COEFF_STARTED\n", location);
        handleSubAction(STATE_CAL_SYRINGE_GET_AIR_CHECK_COEFF_PROGRESS, STATE_CAL_SYRINGE_GET_AIR_CHECK_COEFF_DONE, STATE_CAL_SYRINGE_GET_AIR_CHECK_COEFF_FAILED);
        env->ds.mcuAction->actGetSyringeAirCheckCoeff(location, guidSubAction);
        break;
    case STATE_CAL_SYRINGE_GET_AIR_CHECK_COEFF_PROGRESS:
        LOG_INFO("SOD[%d]: STATE_CAL_SYRINGE_GET_AIR_CHECK_COEFF_PROGRESS\n", location);
        break;
    case STATE_CAL_SYRINGE_GET_AIR_CHECK_COEFF_FAILED:
        LOG_ERROR("SOD[%d]: STATE_CAL_SYRINGE_GET_AIR_CHECK_COEFF_FAILED: %s\n", location, ImrParser::ToImr_DataServiceActionStatusStr(actStatusBuf).CSTR());
        // Clear the error state as this state was only to raise an alert
        actStatusBuf.err = "";
        actStatusBuf.state = DS_ACTION_STATE_COMPLETED;
        setState(STATE_DONE);
        break;
    case STATE_CAL_SYRINGE_GET_AIR_CHECK_COEFF_DONE:
        LOG_INFO("SOD[%d]: STATE_CAL_SYRINGE_GET_AIR_CHECK_COEFF_DONE\n", location);
        {
            DS_McuDef::SyringeAirCheckCoeffDigests syringeAirCheckCoeffDigests = env->ds.mcuData->getSyringeAirCheckCoeffDigests();
            double slope = syringeAirCheckCoeffDigests[location].slope;
            double intercept = syringeAirCheckCoeffDigests[location].intercept;
            QString state = syringeAirCheckCoeffDigests[location].state;
            LOG_INFO("SOD[%d]: STATE_CAL_SYRINGE_GET_AIR_CHECK_COEFF_DONE: Calculated slope = %.1f, intercept = %.1f, state = %s.\n", location, slope, intercept, state.CSTR());
            QString alertData = QString().asprintf("%s;%.1f;%.1f;%s", ImrParser::ToImr_FluidSourceSyringeIdx(location).CSTR(), slope, intercept, state.CSTR());
            env->ds.alertAction->activate("SRUReservoirAirCheckCalEnded", alertData);

            setState(STATE_DONE);
        }
        break;
    // ==========================================
    case STATE_ABORT:
        LOG_INFO("SOD[%d]: STATE_ABORT\n", location);
        actStatusBuf.err = "USER_ABORT";
        actStatusBuf.state = DS_ACTION_STATE_USER_ABORT;
        setState(STATE_DONE);
        break;
    case STATE_FAILED:
        LOG_ERROR("SOD[%d]: STATE_FAILED\n", location);
        setState(STATE_DONE);
        break;
    case STATE_DONE:
        LOG_INFO("SOD[%d]: STATE_DONE\n", location);
        {
            if (actStatusBuf.err == "")
            {
                // Set MUDS ready
                actStatusBuf.state = DS_ACTION_STATE_COMPLETED;
            }

            // Set syringe state to false
            DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
            fluidSourceSyringes[location].isBusy = false;
            env->ds.deviceData->setFluidSourceSyringes(fluidSourceSyringes);

            // Giving a moment to update fluidSource states (i.e. FluidSourceMuds.IsBusy is derived from alerts and fluidSourceSyringe state)
            actionCompleted(actStatusBuf);
            setState(STATE_READY);
        }
        break;
    default:
        LOG_ERROR("SOD[%d]: Unknown state(%d)\n", location, state);
        break;
    }
}

