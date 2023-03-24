#include "Apps/AppManager.h"
#include "DeviceSyringePrime.h"
#include "DataServices/Workflow/DS_WorkflowData.h"
#include "DataServices/Device/DS_DeviceData.h"
#include "DataServices/Device/DS_DeviceAction.h"
#include "DataServices/Alert/DS_AlertAction.h"
#include "DataServices/Mcu/DS_McuAction.h"
#include "DataServices/Capabilities/DS_Capabilities.h"

DeviceSyringePrime::DeviceSyringePrime(QObject *parent, EnvGlobal *env_, EnvLocal *envLocal_, SyringeIdx location_) :
    ActionBaseExt(parent, env_),
    location(location_)
{
    envLocal = envLocal_;

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });

    syringeVolBeforePrime = -1;
    extendedAutoPrimeCounter = 0;
    state = STATE_READY;
}

DeviceSyringePrime::~DeviceSyringePrime()
{
}

void DeviceSyringePrime::slotAppInitialised()
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
            env->actionMgr->deleteActAll(guidSubAction);

            state = STATE_INACTIVE;
            processState();
        }
    });
}

DataServiceActionStatus DeviceSyringePrime::actPrime(DS_DeviceDef::SudsPrimeType newPrimeType, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "Prime", QString().asprintf("%s;%s", ImrParser::ToImr_FluidSourceSyringeIdx(location).CSTR(), ImrParser::ToImr_SudsPrimeType(newPrimeType).CSTR()));
    extendedAutoPrimeCounter = 0;

    switch (newPrimeType)
    {
    case DS_DeviceDef::SUDS_PRIME_TYPE_AUTO:
        //Clear previous failed alert
        env->ds.alertAction->deactivate("AutoPrimeFailed");

        // Check auto prime condition
        checkAutoPrimeCondition(&status);
        break;
    case DS_DeviceDef::SUDS_PRIME_TYPE_MANUAL:
        checkManualPrimeCondition(&status);
        break;
    case DS_DeviceDef::SUDS_PRIME_TYPE_SYRINGE_PRIME:
        checkSyringePrimeCondition(&status);
        break;
    case DS_DeviceDef::SUDS_PRIME_TYPE_USER_DEFINED:
        checkUserDefinedPrimeCondition(&status);
        break;
    case DS_DeviceDef::SUDS_PRIME_TYPE_SYRINGE_AIR_RECOVERY:
        checkSyringeAirRecoveryPrimeCondition(&status);
        break;
    case DS_DeviceDef::SUDS_PRIME_TYPE_FLUID_PURGE:
    default:
        break;
    }

    // Check MUDS busy state
    if (status.err != "")
    {
        LOG_WARNING("actPrime(): Failed to start prime. Prime Condition check failed\n");
    }
    else if (env->ds.mcuData->getSyringesBusy())
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "PISTON_MOVING";
        LOG_WARNING("actPrime(): Failed to start prime. Syringes Busy. %s\n", Util::qVarientToJsonData(ImrParser::ToImr_SyringeStates(env->ds.mcuData->getSyringeStates())).CSTR());
    }
    else if ((newPrimeType == DS_DeviceDef::SUDS_PRIME_TYPE_AUTO) && env->ds.deviceData->getFluidSourceSyringesBusy())
    {
        // Checking SUDS_PRIME_TYPE_AUTO as this condition was specifically added to address simultaneouse AUTO_PRIME and AUTO_REFILL timing issues
        // Not checking this prime type causes other prime types to fail unexpectedly
        // This condition was added as it was discovered that env->ds.mcuData->getSyringesBusy() doesn't cover stopcock movements
        // using existing PISTON_MOVING err message as this generates generic error message : "Injector is busy and cannot prime."
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "PISTON_MOVING";
        LOG_WARNING("actPrime(): Failed to start prime. FluidSourceSyringesBusy\n");
    }
    else if (state != STATE_READY)
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "STATE_BUSY";
        LOG_WARNING("actPrime(): Failed to start prime. State Busy(%d)\n", state);
    }
    else if ( (newPrimeType == DS_DeviceDef::SUDS_PRIME_TYPE_MANUAL) &&
              (!env->ds.mcuData->getPrimeBtnPressed()) )
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "PRIME_BTN_RELEASED";
        LOG_WARNING("actPrime(): Failed to start prime. Prime button released\n");
    }

    if (status.err != "")
    {
        // Prime failed to start
        if (newPrimeType == DS_DeviceDef::SUDS_PRIME_TYPE_AUTO)
        {
            if ( (status.err == "INSUFFICIENT_FLUID") ||
                 (status.err == "PISTON_MOVING") ||
                 (status.err == "INVALID_INJECTOR_STATE") )
            {
                env->ds.alertAction->activate("AutoPrimeFailed", status.err);
            }
            env->ds.deviceAction->actSudsSetNeedsPrime(true);
        }
        actionStarted(status);
        return status;
    }

    actStatusBuf = status;
    primeType = newPrimeType;
    actStatusBuf.state = DS_ACTION_STATE_STARTED;
    actionStarted(actStatusBuf);

    // Set Muds state
    DS_DeviceDef::FluidSource fluidSourceMuds = env->ds.deviceData->getFluidSourceMuds();
    fluidSourceMuds.currentFluidKinds[0] = (location == SYRINGE_IDX_SALINE) ? "Flush" : "Contrast";
    env->ds.deviceData->setFluidSourceMuds(fluidSourceMuds);

    env->ds.deviceAction->actSudsPrimeInit(location);

    // Update alerts
    switch (primeType)
    {
    case DS_DeviceDef::SUDS_PRIME_TYPE_AUTO:
    case DS_DeviceDef::SUDS_PRIME_TYPE_MANUAL:
        env->ds.alertAction->activate("SRUPrimingSUDS");
        break;
    case DS_DeviceDef::SUDS_PRIME_TYPE_SYRINGE_PRIME:
        env->ds.alertAction->activate("SRUPrimingMUDS", ImrParser::ToImr_FluidSourceSyringeIdx(location));
        break;
    case DS_DeviceDef::SUDS_PRIME_TYPE_SYRINGE_AIR_RECOVERY:
        break;
    case DS_DeviceDef::SUDS_PRIME_TYPE_FLUID_PURGE:
        env->ds.alertAction->activate("SRUPurgingFluid", ImrParser::ToImr_FluidSourceSyringeIdx(location));
        break;
    default:
        break;
    }

    primeParams = getPrimeParams();
    setState(STATE_STARTED);

    return actStatusBuf;
}

DataServiceActionStatus DeviceSyringePrime::actPrime(const DS_McuDef::ActPrimeParams &primeParams_, QString actGuid)
{
    primeParams = primeParams_;
    return actPrime(DS_DeviceDef::SUDS_PRIME_TYPE_USER_DEFINED, actGuid);
}

bool DeviceSyringePrime::isSetStateReady()
{
    if (state == STATE_INACTIVE)
    {
        return false;
    }
    return true;
}

void DeviceSyringePrime::processState()
{
    DS_DeviceDef::FluidSource fluidSourceSyringe = env->ds.deviceData->getFluidSourceSyringes()[location];

    switch (state)
    {
    case STATE_INACTIVE:
        LOG_INFO("PRIME: STATE_INACTIVE\n");
        env->ds.alertAction->deactivate("SRUPrimingSUDS");
        env->ds.alertAction->deactivate("SRUPrimingMUDS");
        env->ds.alertAction->deactivate("SRUPurgingFluid");
        break;
    case STATE_READY:
        LOG_INFO("PRIME[%d]: STATE_READY\n", location);
        break;
    case STATE_STARTED:
        LOG_INFO("PRIME[%d]: STATE_STARTED: %s\n", location, ImrParser::ToImr_SudsPrimeType(primeType).CSTR());
        {
            // Start Priming
            DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
            fluidSourceSyringes[location].isBusy = true;
            env->ds.deviceData->setFluidSourceSyringes(fluidSourceSyringes);
        }
        setState(STATE_STOPCOCK_INJECT_SETTING);
        break;
    case STATE_STOPCOCK_INJECT_SETTING:
        LOG_INFO("PRIME[%d]: STATE_STOPCOCK_INJECT_SETTING\n", location);
        {
            handleSubAction(STATE_STOPCOCK_INJECT_PROGRESS, STATE_STOPCOCK_INJECT_DONE, STATE_STOPCOCK_INJECT_FAILED);

            // Set stopcock positions: if other syringe is in inject position, set it to closed position
            DS_McuDef::StopcockPosAll stopcockPosAll = env->ds.mcuData->getStopcockPosAll();
            DS_McuDef::ActStopcockParams scParams;
            for (int scIdx = 0; scIdx < scParams.posAll.length(); scIdx++)
            {
                if (scIdx == location)
                {
                    scParams.posAll[scIdx] = DS_McuDef::STOPCOCK_POS_INJECT;
                }
                else
                {
                    if ( (stopcockPosAll[scIdx] == DS_McuDef::STOPCOCK_POS_INJECT) ||
                         (stopcockPosAll[scIdx] == DS_McuDef::STOPCOCK_POS_UNKNOWN) )
                    {
                        scParams.posAll[scIdx] = DS_McuDef::STOPCOCK_POS_CLOSED;
                    }
                    else
                    {
                        scParams.posAll[scIdx] = DS_McuDef::STOPCOCK_POS_NONE;
                    }
                }
            }
            env->ds.deviceAction->actStopcock(scParams, STOPCOCK_ACTION_TRIALS_LIMIT, guidSubAction);
        }
        break;
    case STATE_STOPCOCK_INJECT_PROGRESS:
        LOG_INFO("PRIME[%d]: STATE_STOPCOCK_INJECT_PROGRESS\n", location);
        break;
    case STATE_STOPCOCK_INJECT_DONE:
        LOG_INFO("PRIME[%d]: STATE_STOPCOCK_INJECT_DONE\n", location);

        if ( (primeType == DS_DeviceDef::SUDS_PRIME_TYPE_MANUAL) &&
             (!env->ds.mcuData->getPrimeBtnPressed()) )
        {
            // Manual prime button released before manual prime started
            actStatusBuf.state = DS_ACTION_STATE_ALLSTOP_BTN_ABORT;
            actStatusBuf.err = "USER_ABORT";
            setState(STATE_INJECT_FAILED);
        }
        else
        {
            setState(STATE_INJECT_STARTED);
        }
        break;
    case STATE_STOPCOCK_INJECT_FAILED:
        LOG_INFO("PRIME[%d]: STATE_STOPCOCK_INJECT_FAILED\n", location);
        actStatusBuf.err = "STOPCOCK_FAILED";
        setState(STATE_FAILED);
        break;
    case STATE_INJECT_STARTED:
        LOG_INFO("PRIME[%d]: STATE_INJECT_STARTED\n", location);
        {
            if (primeParams.idx == SYRINGE_IDX_NONE)
            {
                actStatusBuf.state = DS_ACTION_STATE_BAD_REQUEST;
                actStatusBuf.err = "Bad Prime Type";

                QString err = QString().asprintf("PRIME[%d]: Bad Prime Type(%d)\n", location, primeType);
                LOG_ERROR("%s", err.CSTR());
                env->ds.alertAction->activate("HCUInternalSoftwareError", err);
                setState(STATE_INJECT_FAILED);
                return;
            }

            if ( isAutoPrimePostAutoEmpty())
            {
                if ( extendedAutoPrimeCounter > 0 )
                {
                    // reload prime params. PostAutoEmpty auto prime has phases and params should change for phases
                    primeParams = getPrimeParams();
                }
                extendedAutoPrimeCounter++;
            }

            // Save the syringe volume before it primes
            syringeVolBeforePrime = fluidSourceSyringe.currentVolumesTotal();

            handleSubAction(STATE_INJECT_PROGRESS, STATE_INJECT_DONE, STATE_INJECT_FAILED);
            env->ds.mcuAction->actPrime(primeParams, guidSubAction);
        }
        break;
    case STATE_INJECT_PROGRESS:
        LOG_INFO("PRIME[%d]: STATE_INJECT_PROGRESS\n", location);
        if (primeType == DS_DeviceDef::SUDS_PRIME_TYPE_MANUAL)
        {
            // Manual prime: register prime button release callback
            env->actionMgr->deleteActCompleted(guidStopBtnMonitor);
            guidStopBtnMonitor = Util::newGuid();
            QMetaObject::Connection conn = connect(env->ds.mcuData, &DS_McuData::signalDataChanged_PrimeBtnPressed, this, [=](bool pressed) {
                LOG_DEBUG("PRIME[%d]: Prime Button %s\n", location, pressed ? "PRESSED" : "RELEASED");
                if (state != STATE_INJECT_PROGRESS)
                {
                    // unexpected callback
                    env->actionMgr->deleteActCompleted(guidStopBtnMonitor);
                    return;
                }

                if (!pressed)
                {
                    LOG_DEBUG("PRIME[%d]: Prime button is released during manual prime\n", location);
                    env->actionMgr->deleteActCompleted(guidStopBtnMonitor);
                    env->ds.mcuAction->actStopAll();
                }
            });
            env->actionMgr->createActCompleted(guidStopBtnMonitor, conn, QString(__PRETTY_FUNCTION__) + ": STATE_INJECT_PROGRESS: PrimeBtnPressed");
        }
        break;
    case STATE_INJECT_DONE:
        LOG_INFO("PRIME[%d]: STATE_INJECT_DONE\n", location);
        actStatusBuf.err = "";
        if ( isAutoPrimePostAutoEmpty() && !isAutoPrimePostAutoEmptyCompleted() )
        {
            LOG_INFO("PRIME[%d]: First phase of extended auto prime done. Running second phase..\n", location);
            setState(STATE_INJECT_STARTED);
        }
        else
        {
            setState(STATE_PRESSURE_DECAY_STARTED);
        }
        break;
    case STATE_INJECT_FAILED:
        LOG_INFO("PRIME[%d]: STATE_INJECT_FAILED\n", location);

        // Translate the MCU error message. Note: error might be occured before action started and error string shall be T_PRIMEFAILED_XXX
        if (actStatusBuf.err.contains("NoSUDS"))
        {
            actStatusBuf.state = DS_ACTION_STATE_INVALID_STATE;
            actStatusBuf.err = "SUDS_REMOVED";
        }
        else if (actStatusBuf.err.contains("StopButtonPressed"))
        {
            actStatusBuf.state = DS_ACTION_STATE_ALLSTOP_BTN_ABORT;
            actStatusBuf.err = "USER_ABORT";
        }
        else if (actStatusBuf.err.contains("PlungerNotEngaged"))
        {
            actStatusBuf.state = DS_ACTION_STATE_INVALID_STATE;
            actStatusBuf.err = "PLUNGER_ENGAGE_FAULT";
        }
        else if (actStatusBuf.err.contains("InvalidStopcockPosition"))
        {
            actStatusBuf.state = DS_ACTION_STATE_INVALID_STATE;
            actStatusBuf.err = "STOPCOCK_FAILED";
        }

        if ( (primeType == DS_DeviceDef::SUDS_PRIME_TYPE_MANUAL) &&
             (actStatusBuf.state == DS_ACTION_STATE_ALLSTOP_BTN_ABORT) )
        {
            LOG_INFO("PRIME[%d]: Manual Prime completed by user\n", location);
            actStatusBuf.err = "";
            setState(STATE_INJECT_DONE);
        }
        else
        {
            setState(STATE_FAILED);
        }
        break;
    case STATE_FAILED:
        LOG_ERROR("PRIME[%d]: STATE_FAILED\n", location);
        setState(STATE_PRESSURE_DECAY_STARTED);
        break;
    case STATE_PRESSURE_DECAY_STARTED:
        LOG_INFO("PRIME[%d]: STATE_PRESSURE_DECAY_STARTED\n", location);
        if ( (primeType == DS_DeviceDef::SUDS_PRIME_TYPE_AUTO) ||
             (primeType == DS_DeviceDef::SUDS_PRIME_TYPE_SYRINGE_PRIME) )
        {
            int waitTime = env->ds.capabilities->get_Prime_PrimeSalinePressureDecayWait();
            if ( (primeParams.idx == SYRINGE_IDX_CONTRAST1) ||
                 (primeParams.idx == SYRINGE_IDX_CONTRAST2) )
            {
                waitTime = env->ds.capabilities->get_Prime_PrimeContrastPressureDecayWait();
            }

            LOG_INFO("PRIME[%d]: STATE_PRESSURE_DECAY_STARTED: Waiting for pressure decay (%dms)\n", location, waitTime);

            QTimer::singleShot(waitTime, this, [=] {
                setState(STATE_PRESSURE_DECAY_DONE);
            });
        }
        else
        {
            LOG_INFO("PRIME[%d]: STATE_PRESSURE_DECAY_STARTED: Not required. Skipping..\n", location);
            setState(STATE_PRESSURE_DECAY_DONE);
        }
        break;
    case STATE_PRESSURE_DECAY_DONE:
        LOG_INFO("PRIME[%d]: STATE_PRESSURE_DECAY_DONE\n", location);
        setState(STATE_STOPCOCK_CLOSE_SETTING);
        break;
    case STATE_STOPCOCK_CLOSE_SETTING:
        LOG_INFO("PRIME[%d]: STATE_STOPCOCK_CLOSE_SETTING\n", location);
        handleSubAction(STATE_STOPCOCK_CLOSE_PROGRESS, STATE_STOPCOCK_CLOSE_DONE, STATE_STOPCOCK_CLOSE_DONE);
        env->ds.deviceAction->actStopcock(location, DS_McuDef::STOPCOCK_POS_CLOSED, STOPCOCK_ACTION_TRIALS_LIMIT, guidSubAction);
        break;
    case STATE_STOPCOCK_CLOSE_PROGRESS:
        LOG_INFO("PRIME[%d]: STATE_STOPCOCK_CLOSE_PROGRESS\n", location);
        break;
    case STATE_STOPCOCK_CLOSE_DONE:
        LOG_INFO("PRIME[%d]: STATE_STOPCOCK_CLOSE_DONE\n", location);
        setState(STATE_DONE);
        break;
    case STATE_STOPCOCK_CLOSE_FAILED:
        LOG_ERROR("PRIME[%d]: STATE_STOPCOCK_CLOSE_FAILED\n", location);
        actStatusBuf.err = "STOPCOCK_FAILED";
        setState(STATE_DONE);
        break;
    case STATE_DONE:
        LOG_INFO("PRIME[%d]: STATE_DONE\n", location);

        extendedAutoPrimeCounter = 0;

        // Update SUDS state
        switch (primeType)
        {
        case DS_DeviceDef::SUDS_PRIME_TYPE_MANUAL:
            {
                double syringeVol = env->ds.deviceData->getFluidSourceSyringes()[location].currentVolumesTotal();
                env->ds.alertAction->activate("SRUAdvancedSaline", QString().asprintf("%.1f", (syringeVolBeforePrime - syringeVol)));
                if (actStatusBuf.err == "")
                {
                    env->ds.deviceAction->actSudsPrimeUpdateState();
                }
                else
                {
                    env->ds.deviceAction->actSudsSetNeedsPrime(true);
                }
            }
            break;
        case DS_DeviceDef::SUDS_PRIME_TYPE_AUTO:
        case DS_DeviceDef::SUDS_PRIME_TYPE_USER_DEFINED:
            if (isAutoPrimePostAutoEmpty())
            {
                // if MUDS was ejected, Reset ExtendedAutoPrime flag just to ensure we use correct Prime volume for next auto prime.
                LOG_INFO("Disabling ExtendedAutoPrime Volume\n");
                env->ds.workflowData->setUseExtendedAutoPrimeVolume(false);
            }
            env->ds.deviceAction->actSudsSetNeedsPrime(actStatusBuf.err != "");
            break;
        case DS_DeviceDef::SUDS_PRIME_TYPE_SYRINGE_PRIME:
        case DS_DeviceDef::SUDS_PRIME_TYPE_SYRINGE_AIR_RECOVERY:
        case DS_DeviceDef::SUDS_PRIME_TYPE_FLUID_PURGE:
            env->ds.deviceAction->actSudsSetNeedsPrime(true);
            env->ds.deviceData->setIsFirstAutoPrimeCompleted(false);
            break;
        default:
            break;
        }

        // Update Alerts
        switch (primeType)
        {
        case DS_DeviceDef::SUDS_PRIME_TYPE_AUTO:

            if (actStatusBuf.state == DS_ACTION_STATE_ALLSTOP_BTN_ABORT)
            {
                env->ds.alertAction->activate("InterruptedAllStopPressed", "Prime;RS0");
            }

            if ( (actStatusBuf.err == "USER_ABORT") ||
                 (actStatusBuf.err == "SUDS_REMOVED") )
            {
                // User stopped the auto prime - no need to raise alert
                env->ds.alertAction->deactivateWithReason("SRUPrimingSUDS", actStatusBuf.err);
            }
            else if (actStatusBuf.err != "")
            {
                env->ds.alertAction->activate("AutoPrimeFailed", actStatusBuf.err);
                env->ds.alertAction->deactivateWithReason("SRUPrimingSUDS", actStatusBuf.err);
            }
            else
            {
                env->ds.alertAction->deactivate("OutletAirDetected");
                env->ds.alertAction->deactivateWithReason("SRUPrimingSUDS", "AutoPrimeDone");
            }
            break;
        case DS_DeviceDef::SUDS_PRIME_TYPE_MANUAL:
            env->ds.alertAction->deactivate("SRUPrimingSUDS");
            break;
        case DS_DeviceDef::SUDS_PRIME_TYPE_SYRINGE_PRIME:
            env->ds.alertAction->deactivate("SRUPrimingMUDS");
            if (actStatusBuf.state == DS_ACTION_STATE_ALLSTOP_BTN_ABORT)
            {
                env->ds.alertAction->activate("InterruptedAllStopPressed", QString().asprintf("Prime;%s", ImrParser::ToImr_FluidSourceSyringeIdx(location).CSTR()));
            }
            break;
        case DS_DeviceDef::SUDS_PRIME_TYPE_SYRINGE_AIR_RECOVERY:
            if (actStatusBuf.state == DS_ACTION_STATE_ALLSTOP_BTN_ABORT)
            {
                env->ds.alertAction->activate("InterruptedAllStopPressed", QString().asprintf("Prime;%s", ImrParser::ToImr_FluidSourceSyringeIdx(location).CSTR()));
            }
            break;
        case DS_DeviceDef::SUDS_PRIME_TYPE_FLUID_PURGE:
            {
                env->ds.alertAction->deactivate("SRUPurgingFluid", ImrParser::ToImr_FluidSourceSyringeIdx(location));
                QVariantMap jsonMap;
                QVariantMap jsonMapFluidSource = ImrParser::ToImr_FluidSource(fluidSourceSyringe);
                double purgedVolume = syringeVolBeforePrime - fluidSourceSyringe.currentVolumesTotal();
                jsonMapFluidSource.insert("PurgedVolume", purgedVolume);
                jsonMap.insert(ImrParser::ToImr_FluidSourceSyringeIdx(location), jsonMapFluidSource);
                QString alertData = Util::qVarientToJsonData(jsonMap);
                env->ds.alertAction->activate("SRUPurgedFluid", alertData);
            }
            break;
        default:
            break;
        }

        setState(STATE_READY);

        // Update Syringe.IsBusy state
        fluidSourceSyringe.isBusy = false;
        env->ds.deviceData->setFluidSourceSyringe(location, fluidSourceSyringe);

        if (actStatusBuf.err == "")
        {
            actStatusBuf.state = DS_ACTION_STATE_COMPLETED;
        }

        actionCompleted(actStatusBuf);
        break;
    default:
        LOG_ERROR("PRIME[%d]: Unknown state(%d)\n", location, state);
        break;
    }
}

void DeviceSyringePrime::checkAutoPrimeCondition(DataServiceActionStatus *status)
{
    status->state = DS_ACTION_STATE_START_WAITING;
    status->err = "";

    DS_DeviceDef::FluidSource fluidSourceMuds = env->ds.deviceData->getFluidSourceMuds();
    DS_DeviceDef::FluidSource fluidSourceWasteContainer = env->ds.deviceData->getFluidSourceWasteContainer();
    DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
    double primeVolume = env->ds.workflowData->getAutoPrimeVolume();
    DS_McuDef::OutletDoorState outletDoorState = env->ds.mcuData->getOutletDoorState();
    DS_SystemDef::StatePath statePath = env->ds.systemData->getStatePath();
    double wcLevel = fluidSourceWasteContainer.currentVolumesTotal();

    if (!fluidSourceMuds.isReady)
    {
        status->state = DS_ACTION_STATE_INVALID_STATE;
        status->err = "MUDS_NOT_PRIMED";
    }
    else if (!env->ds.mcuData->getSudsInserted())
    {
        status->state = DS_ACTION_STATE_INVALID_STATE;
        status->err = "SUDS_REMOVED";
    }
    else if ( (wcLevel != DS_DeviceDef::WASTE_CONTAINER_LEVEL_UNKNOWN_COMM_DOWN) &&
              (!fluidSourceWasteContainer.isInstalled()) )
    {
        status->state = DS_ACTION_STATE_INVALID_STATE;
        status->err = "WASTE_CONTAINER_MISSING";
    }
     else if ( (wcLevel != DS_DeviceDef::WASTE_CONTAINER_LEVEL_UNKNOWN_COMM_DOWN) &&
               (wcLevel == DS_DeviceDef::WASTE_CONTAINER_LEVEL_FULL) )
    {
        status->state = DS_ACTION_STATE_INVALID_STATE;
        status->err = "WASTE_CONTAINER_FULL";
    }
    else if (outletDoorState == DS_McuDef::OUTLET_DOOR_STATE_OPEN)
    {
        status->state = DS_ACTION_STATE_INVALID_STATE;
        status->err = "OUTLET_DOOR_OPEN";
    }
    else if (fluidSourceSyringes[SYRINGE_IDX_SALINE].currentVolumesTotal() < primeVolume)
    {
        LOG_WARNING("checkAutoPrimeCondition(): Insufficient volume: %.1f < %.1f\n", fluidSourceSyringes[SYRINGE_IDX_SALINE].currentVolumesTotal(), primeVolume);

        env->ds.alertAction->activate("InsufficientVolumeForSUDSPrime", QString().asprintf("RS0;%.0f", primeVolume));
        status->state = DS_ACTION_STATE_INVALID_STATE;
        status->err = "INSUFFICIENT_FLUID";
    }
    else if ( (statePath == DS_SystemDef::STATE_PATH_READY_ARMED) ||
              (statePath == DS_SystemDef::STATE_PATH_BUSY_HOLDING) ||
              (statePath == DS_SystemDef::STATE_PATH_BUSY_FINISHING) ||
              (statePath == DS_SystemDef::STATE_PATH_BUSY_WAITING) ||
              (statePath == DS_SystemDef::STATE_PATH_EXECUTING) )
    {
        status->state = DS_ACTION_STATE_INVALID_STATE;
        status->err = "INVALID_INJECTOR_STATE";
    }
}

void DeviceSyringePrime::checkManualPrimeCondition(DataServiceActionStatus *status)
{
    status->state = DS_ACTION_STATE_START_WAITING;
    status->err = "";

    DS_DeviceDef::FluidSource fluidSourceMuds = env->ds.deviceData->getFluidSourceMuds();
    DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
    DS_DeviceDef::FluidSource fluidSourceSuds = env->ds.deviceData->getFluidSourceSuds();
    DS_McuDef::OutletDoorState outletDoorState = env->ds.mcuData->getOutletDoorState();
    DS_SystemDef::StatePath statePath = env->ds.systemData->getStatePath();

    if (!fluidSourceMuds.isReady)
    {
        status->state = DS_ACTION_STATE_INVALID_STATE;
        status->err = "MUDS_NOT_PRIMED";
    }
    else if (!fluidSourceSuds.isInstalled())
    {
        status->state = DS_ACTION_STATE_INVALID_STATE;
        status->err = "SUDS_REMOVED";
    }
    else if (fluidSourceSuds.needsReplaced)
    {
        status->state = DS_ACTION_STATE_INVALID_STATE;
        status->err = "SUDS_NEEDS_REPLACED";
    }
    else if (outletDoorState == DS_McuDef::OUTLET_DOOR_STATE_OPEN)
    {
        status->state = DS_ACTION_STATE_INVALID_STATE;
        status->err = "OUTLET_DOOR_OPEN";
    }
    else if (fluidSourceSyringes[location].currentVolumesTotal() <= 0)
    {
        LOG_WARNING("checkAutoPrimeCondition(): Insufficient volume: %.1f <= 0\n", fluidSourceSyringes[location].currentVolumesTotal());

        status->state = DS_ACTION_STATE_INVALID_STATE;
        status->err = "INSUFFICIENT_FLUID";
    }
    else if ( (statePath == DS_SystemDef::STATE_PATH_READY_ARMED) ||
              (statePath == DS_SystemDef::STATE_PATH_BUSY_HOLDING) ||
              (statePath == DS_SystemDef::STATE_PATH_BUSY_FINISHING) ||
              (statePath == DS_SystemDef::STATE_PATH_BUSY_WAITING) ||
              (statePath == DS_SystemDef::STATE_PATH_EXECUTING) )
    {
        status->state = DS_ACTION_STATE_INVALID_STATE;
        status->err = "INVALID_INJECTOR_STATE";
    }
}

void DeviceSyringePrime::checkSyringePrimeCondition(DataServiceActionStatus *status)
{
    status->state = DS_ACTION_STATE_START_WAITING;
    status->err = "";

    DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
    double primeVolume = env->ds.capabilities->get_Prime_SyringePrimeVol();
    DS_SystemDef::StatePath statePath = env->ds.systemData->getStatePath();

    if (!env->ds.mcuData->getSudsInserted())
    {
        status->state = DS_ACTION_STATE_INVALID_STATE;
        status->err = "SUDS_REMOVED";
    }
    else if (Util::isFloatVarGreaterThan(primeVolume, fluidSourceSyringes[location].currentVolumesTotal(), 1))
    {
        LOG_WARNING("checkAutoPrimeCondition(): Insufficient volume: %.1f <= %.1f\n", fluidSourceSyringes[location].currentVolumesTotal(), primeVolume);

        status->state = DS_ACTION_STATE_INVALID_STATE;
        status->err = "INSUFFICIENT_FLUID";
    }
    else if ( (statePath == DS_SystemDef::STATE_PATH_READY_ARMED) ||
              (statePath == DS_SystemDef::STATE_PATH_BUSY_HOLDING) ||
              (statePath == DS_SystemDef::STATE_PATH_BUSY_FINISHING) ||
              (statePath == DS_SystemDef::STATE_PATH_BUSY_WAITING) ||
              (statePath == DS_SystemDef::STATE_PATH_EXECUTING) )
    {
        status->state = DS_ACTION_STATE_INVALID_STATE;
        status->err = "INVALID_INJECTOR_STATE";
    }
}

void DeviceSyringePrime::checkSyringeAirRecoveryPrimeCondition(DataServiceActionStatus *status)
{
    status->state = DS_ACTION_STATE_START_WAITING;
    status->err = "";

    DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
    DS_McuDef::SyringeAirCheckDigests syringeAirCheckDigests = env->ds.mcuData->getSyringeAirCheckDigests();
    double airVolume = syringeAirCheckDigests[location].airVolume;
    double primeExtraVolume = env->ds.capabilities->get_AirCheck_SyringeAirRecoveryPrimeExtraVol();
    double primeVolume = airVolume + primeExtraVolume;
    DS_SystemDef::StatePath statePath = env->ds.systemData->getStatePath();

    if (!env->ds.mcuData->getSudsInserted())
    {
        status->state = DS_ACTION_STATE_INVALID_STATE;
        status->err = "SUDS_REMOVED";
    }
    else if (Util::isFloatVarGreaterThan(primeVolume, fluidSourceSyringes[location].currentVolumesTotal(), 1))
    {
        LOG_WARNING("checkSyringeAirRecoveryPrimeCondition(): Insufficient volume: %.1f < %.1f\n", fluidSourceSyringes[location].currentVolumesTotal(), primeVolume);
        status->state = DS_ACTION_STATE_INVALID_STATE;
        status->err = "INSUFFICIENT_FLUID";
    }
    else if ( (statePath == DS_SystemDef::STATE_PATH_READY_ARMED) ||
              (statePath == DS_SystemDef::STATE_PATH_BUSY_HOLDING) ||
              (statePath == DS_SystemDef::STATE_PATH_BUSY_FINISHING) ||
              (statePath == DS_SystemDef::STATE_PATH_BUSY_WAITING) ||
              (statePath == DS_SystemDef::STATE_PATH_EXECUTING) )
    {
        status->state = DS_ACTION_STATE_INVALID_STATE;
        status->err = "INVALID_INJECTOR_STATE";
    }
}

void DeviceSyringePrime::checkUserDefinedPrimeCondition(DataServiceActionStatus *status)
{
    status->state = DS_ACTION_STATE_START_WAITING;
    status->err = "";

    DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
    double primeVolume = primeParams.vol1;
    DS_SystemDef::StatePath statePath = env->ds.systemData->getStatePath();

    if (!env->ds.mcuData->getSudsInserted())
    {
        status->state = DS_ACTION_STATE_INVALID_STATE;
        status->err = "SUDS_REMOVED";
    }
    else if (Util::isFloatVarGreaterThan(primeVolume, fluidSourceSyringes[location].currentVolumesTotal(), 1))
    {
        LOG_WARNING("checkAutoPrimeCondition(): Insufficient volume: %.1f <= %.1f\n", fluidSourceSyringes[location].currentVolumesTotal(), primeVolume);

        status->state = DS_ACTION_STATE_INVALID_STATE;
        status->err = "INSUFFICIENT_FLUID";
    }
    else if ( (statePath == DS_SystemDef::STATE_PATH_READY_ARMED) ||
              (statePath == DS_SystemDef::STATE_PATH_BUSY_HOLDING) ||
              (statePath == DS_SystemDef::STATE_PATH_BUSY_FINISHING) ||
              (statePath == DS_SystemDef::STATE_PATH_BUSY_WAITING) ||
              (statePath == DS_SystemDef::STATE_PATH_EXECUTING) )
    {
        status->state = DS_ACTION_STATE_INVALID_STATE;
        status->err = "INVALID_INJECTOR_STATE";
    }
}


DS_McuDef::ActPrimeParams DeviceSyringePrime::getPrimeParams()
{
    DS_McuDef::ActPrimeParams params;

    switch (primeType)
    {
    case DS_DeviceDef::SUDS_PRIME_TYPE_SYRINGE_PRIME:
        params.idx = location;
        params.flow = env->ds.capabilities->get_Prime_SyringePrimeFlow();
        params.vol1 = env->ds.capabilities->get_Prime_SyringePrimeVol();
        params.vol2 = 0;
        params.operationName = "SyringePrime";
        LOG_INFO("PRIME[%d]: Syringe Prime (%.1fml, %.1fml/s)\n", location, params.vol1, params.flow);
        break;
    case DS_DeviceDef::SUDS_PRIME_TYPE_SYRINGE_AIR_RECOVERY:
        {
            DS_McuDef::SyringeAirCheckDigests syringeAirCheckDigests = env->ds.mcuData->getSyringeAirCheckDigests();
            double airVolume = syringeAirCheckDigests[location].airVolume;            
            double primeExtraVolume = env->ds.capabilities->get_AirCheck_SyringeAirRecoveryPrimeExtraVol();
            double primeVolume = airVolume + primeExtraVolume;

            params.idx = location;
            params.flow = env->ds.capabilities->get_AirCheck_SyringeAirRecoveryPrimeFlow();
            params.vol1 = primeVolume;
            params.vol2 = 0;
            params.operationName = "SyringeAirRecoveryPrime";
            LOG_INFO("PRIME[%d]: Syringe Air Recovery Prime (%.1fml, %.1fml/s)\n", location, params.vol1, params.flow);
        }
        break;
    case DS_DeviceDef::SUDS_PRIME_TYPE_AUTO:
        {
            if (isAutoPrimePostAutoEmpty())
            {
                params.idx = location;
                params.flow = env->ds.capabilities->get_Prime_AutoPrimeFlow();
                params.vol1 = env->ds.workflowData->getAutoPrimeVolume() * 0.5;

                if (extendedAutoPrimeCounter == 0)
                {
                    // first phase of AutoPrimePostAutoEmpty: no retries, no air check
                    params.vol2 = 0;
                    params.operationName = "AutoPrimePostAutoEmpty-FirstPhase";
                    LOG_INFO("PRIME[%d]: SUDS AutoPrimePostAutoEmpty-FirstPhase (%.1fml, %.1fml/s) NO AIRCHECK\n", location, params.vol1, params.flow);
                }
                else
                {
                    // second phase of AutoPrimePostAutoEmpty: air check, retry with same volume if air detected
                    params.vol2 = params.vol1;
                    params.operationName = "AutoPrimePostAutoEmpty-SecondPhase";
                    LOG_INFO("PRIME[%d]: SUDS AutoPrimePostAutoEmpty-SecondPhase (%.1fml, %.1fml, %.1fml/s) with aircheck\n", location, params.vol1, params.vol2, params.flow);
                }
            }
            else
            {
                params.idx = location;
                params.flow = env->ds.capabilities->get_Prime_AutoPrimeFlow();
                params.vol1 = env->ds.workflowData->getAutoPrimeVolume();
                params.vol2 = params.vol1;
                params.operationName = "AutoPrime";
                LOG_INFO("PRIME[%d]: SUDS Prime (%.1fml, %.1fml, %.1fml/s) with aircheck\n", location, params.vol1, params.vol2, params.flow);
            }
    }
        break;
    case DS_DeviceDef::SUDS_PRIME_TYPE_MANUAL:
        params.idx = SYRINGE_IDX_SALINE;
        params.flow = env->ds.capabilities->get_Prime_ManualPrimeFlow();
        params.vol1 = SYRINGE_VOLUME_PRIME_ALL;
        params.vol2 = 0;
        params.operationName = "AdvanceSaline";
        LOG_INFO("PRIME[%d]: Manual Prime (%.1fml, %.1fml/s)\n", SYRINGE_IDX_SALINE, SYRINGE_VOLUME_MAX, params.flow);
        break;
    case DS_DeviceDef::SUDS_PRIME_TYPE_FLUID_PURGE:
        params.idx = location;
        params.flow = env->ds.capabilities->get_FluidControl_FluidRemovalFlow();
        params.vol1 = SYRINGE_VOLUME_PRIME_ALL;
        params.vol2 = 0;
        params.operationName = "PurgeFluid";
        LOG_INFO("PRIME[%d]: Purge Fluid (%.1fml, %.1fml/s)\n", location, SYRINGE_VOLUME_MAX, params.flow);
        break;
    case DS_DeviceDef::SUDS_PRIME_TYPE_USER_DEFINED:
    default:
        // Params defined earlier
        params = primeParams;
        break;
    }

    return params;
}

bool DeviceSyringePrime::isAutoPrimePostAutoEmpty()
{
    return ( env->ds.workflowData->getUseExtendedAutoPrimeVolume() &&
             (primeType == DS_DeviceDef::SUDS_PRIME_TYPE_AUTO) &&
             (location == SYRINGE_IDX_SALINE) );

}

bool DeviceSyringePrime::isAutoPrimePostAutoEmptyCompleted()
{
    const int EXTENDED_AUTO_PRIME_PHASES = 2;
    return (extendedAutoPrimeCounter == EXTENDED_AUTO_PRIME_PHASES);
}
