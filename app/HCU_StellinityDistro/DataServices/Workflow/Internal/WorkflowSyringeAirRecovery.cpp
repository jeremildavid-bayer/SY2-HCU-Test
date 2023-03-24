#include "Apps/AppManager.h"
#include "WorkflowSyringeAirRecovery.h"
#include "DataServices/Workflow/DS_WorkflowData.h"
#include "DataServices/Alert/DS_AlertAction.h"
#include "DataServices/Device/DS_DeviceAction.h"
#include "DataServices/Device/DS_DeviceData.h"
#include "DataServices/Capabilities/DS_Capabilities.h"
#include "DataServices/Mcu/DS_McuAction.h"
#include "Common/ImrParser.h"

WorkflowSyringeAirRecovery::WorkflowSyringeAirRecovery(QObject *parent, EnvGlobal *env_) :
    ActionBaseExt(parent, env_)
{
    envLocal = new EnvLocal("DS_Workflow-SyringeAirRecovery", "WORKFLOW_SYRINGE_AIR_RECOVERY");

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });

    location = SYRINGE_IDX_NONE;
}

WorkflowSyringeAirRecovery::~WorkflowSyringeAirRecovery()
{
    delete envLocal;
}

void WorkflowSyringeAirRecovery::slotAppInitialised()
{
    connect(env->ds.workflowData, &DS_WorkflowData::signalDataChanged_WorkflowState, this, [=](DS_WorkflowDef::WorkflowState workflowState, DS_WorkflowDef::WorkflowState workflowStatePrev) {
        if ( (workflowState != DS_WorkflowDef::STATE_INACTIVE) &&
             (workflowStatePrev == DS_WorkflowDef::STATE_INACTIVE) )
        {
            env->ds.workflowData->setSyringeAirRecoveryState(DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_READY);
            processState();
        }
        else if ( (workflowState == DS_WorkflowDef::STATE_INACTIVE) &&
                  (workflowStatePrev != DS_WorkflowDef::STATE_INACTIVE) )
        {
            // Destroy action instances
            env->actionMgr->deleteActAll(actStatusBuf.guid);
            env->actionMgr->deleteActAll(guidSubAction);
            env->actionMgr->deleteActAll(guidSyringesIdleWaiting);
            env->actionMgr->deleteActAll(guidSudsWaiting);

            env->ds.workflowData->setSyringeAirRecoveryState(DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_INACTIVE);
            processState();
        }
    });
}

int WorkflowSyringeAirRecovery::getState()
{
    return env->ds.workflowData->getSyringeAirRecoveryState();
}

QString WorkflowSyringeAirRecovery::getStateStr(int state)
{
    return ImrParser::ToImr_SyringeAirRecoveryState((DS_WorkflowDef::SyringeAirRecoveryState)state);
}

void WorkflowSyringeAirRecovery::setStateSynch(int newState)
{
    env->ds.workflowData->setSyringeAirRecoveryState((DS_WorkflowDef::SyringeAirRecoveryState)newState);
}

bool WorkflowSyringeAirRecovery::isSetStateReady()
{
    if (getState() == DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_INACTIVE)
    {
        return false;
    }
    return true;
}

void WorkflowSyringeAirRecovery::processState()
{
    DS_WorkflowDef::SyringeAirRecoveryState state = (DS_WorkflowDef::SyringeAirRecoveryState)getState();
    switch (state)
    {
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_INACTIVE:
        LOG_INFO("STATE_INACTIVE\n");
        break;
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_READY:
        LOG_INFO("STATE_READY\n");
        break;
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_STARTED:
        LOG_INFO("STATE_STARTED\n");
        {
            actStatusBuf.err = "";

            // Get location
            QVariantMap activeAlert = env->ds.alertAction->getActiveAlert("ReservoirAirDetected", "", true);
            if (activeAlert["GUID"] == EMPTY_GUID)
            {
                // Air recovery is completed for all syringes
                setState(DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_USER_END_WAITING);
                return;
            }

            QString dataStr = activeAlert["Data"].toString();
            QStringList dataStrList = dataStr.split(";");

            location = SYRINGE_IDX_NONE;

            if (dataStrList.length() > 1)
            {
                location = ImrParser::ToCpp_FluidSourceSyringeIdx(dataStrList[0]);
            }

            // Ensure we have a valid location.
            if (location == SYRINGE_IDX_NONE)
            {
                actStatusBuf.state = DS_ACTION_STATE_INVALID_STATE;
                actStatusBuf.err = "BAD SYRINGE INDEX: " + dataStr;
                setState(DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_FAILED_OPERATION_FAILED);
                return;
            }

            // Continue with the general air recovery sequence.
            setState(DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_USER_START_WAITING_1);
        }
        break;
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_MAX_AIR_DETECTED:
        LOG_INFO("SYRINGE_AIR_RECOVERY[%d]: SYRINGE_AIR_RECOVERY_STATE_MAX_AIR_DETECTED\n", location);
        // Wait for user interaction
        break;
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_USER_START_WAITING_1:
        LOG_INFO("SYRINGE_AIR_RECOVERY[%d]: STATE_USER_START_WAITING_1\n", location);
        // Wait for user interaction
        break;
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_VOLUME_CHECKING:
        LOG_INFO("SYRINGE_AIR_RECOVERY[%d]: STATE_VOLUME_CHECKING\n", location);
        {
            // Deactivate last alert before start new recovery
            env->ds.alertAction->deactivate("ReservoirAirRecoveryFailed");

            DS_McuDef::SyringeAirCheckDigests syringeAirCheckDigests = env->ds.mcuData->getSyringeAirCheckDigests();
            double airVolume = syringeAirCheckDigests[location].airVolume;

            // If the max air volume was detected, the recovery is ejecting the MUDS.
            double maxAirVolume = env->ds.capabilities->get_AirCheck_SyringeAirRecoveryMaxAirAmount();
            if (airVolume >= maxAirVolume)
            {
                setState(DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_MAX_AIR_DETECTED);
                return;
            }

            // Check volume
            DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
            double primeExtraVolume = env->ds.capabilities->get_AirCheck_SyringeAirRecoveryPrimeExtraVol();
            double primeExtraVolumeWithRetry = env->ds.capabilities->get_Prime_SyringePrimeVol() * SYRINGE_AIR_RECOVERY_EXTRA_PRIME_TRIALS_LIMIT;
            double syringeAirRecoveryVolRequired = airVolume + primeExtraVolume + primeExtraVolumeWithRetry;

            if (fluidSourceSyringes[location].currentVolumesTotal() < syringeAirRecoveryVolRequired)
            {
                env->ds.alertAction->activate("InsufficientVolumeForReservoirAirRecovery", QString().asprintf("%s;%.0f", ImrParser::ToImr_FluidSourceSyringeIdx(location).CSTR(), syringeAirRecoveryVolRequired));
                actStatusBuf.state = DS_ACTION_STATE_INVALID_STATE;
                actStatusBuf.err = "INSUFFICIENT_FLUID";
                setState(DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_FAILED_INSUFFICIENT_VOLUME);
                return;
            }

            setState(DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_USER_START_WAITING_2);
        }
        break;
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_USER_START_WAITING_2:
        LOG_INFO("SYRINGE_AIR_RECOVERY[%d]: STATE_USER_START_WAITING_2\n", location);
        // Wait for user interaction
        break;
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_SYRINGES_IDLE_WAITING:
        LOG_INFO("SYRINGE_AIR_RECOVERY[%d]: STATE_SYRINGES_IDLE_WAITING\n", location);
        {
            // Syringes ready callback connection
            env->actionMgr->deleteActAll(guidSyringesIdleWaiting);
            guidSyringesIdleWaiting = Util::newGuid();
            QMetaObject::Connection conn = connect(env->ds.deviceData, &DS_DeviceData::signalDataChanged_FluidSourceSyringes, this, [=] {
                DS_WorkflowDef::SyringeAirRecoveryState state = (DS_WorkflowDef::SyringeAirRecoveryState)getState();

                if ( (state != DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_SYRINGES_IDLE_WAITING) &&
                     (state != DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_SUDS_INSERT_WAITING) )
                {
                    // unexpected state
                    env->actionMgr->deleteActAll(guidSyringesIdleWaiting);
                    return;
                }

                if (env->ds.deviceData->getFluidSourceSyringesBusy())
                {
                    // Wait until syringe become ready
                    if (state == DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_SUDS_INSERT_WAITING)
                    {
                        setState(DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_SYRINGES_IDLE_WAITING);
                    }
                }
                else
                {
                    if (state == DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_SYRINGES_IDLE_WAITING)
                    {
                        setState(DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_SUDS_INSERT_WAITING);
                    }
                }
            });
            env->actionMgr->createActCompleted(guidSyringesIdleWaiting, conn, QString(__PRETTY_FUNCTION__) + ": SYRINGE_AIR_RECOVERY_STATE_SYRINGES_IDLE_WAITING: FluidSourceSyringes");

            // Go to next state if possible
            if (!env->ds.deviceData->getFluidSourceSyringesBusy())
            {
                setState(DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_SUDS_INSERT_WAITING);
            }
        }
        break;
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_SUDS_INSERT_WAITING:
        LOG_INFO("SYRINGE_AIR_RECOVERY[%d]: STATE_SUDS_INSERT_WAITING\n", location);
        {
            // Suds Connect Waiting callback connection
            env->actionMgr->deleteActAll(guidSudsWaiting);
            guidSudsWaiting = Util::newGuid();

            QMetaObject::Connection conn = connect(env->ds.mcuData, &DS_McuData::signalDataChanged_SudsInserted, this, [=](bool sudsInserted) {
                DS_WorkflowDef::SyringeAirRecoveryState state = env->ds.workflowData->getSyringeAirRecoveryState();
                if (state != DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_SUDS_INSERT_WAITING)
                {
                    env->actionMgr->deleteActAll(guidSudsWaiting);
                    return;
                }

                if (sudsInserted)
                {
                    env->actionMgr->deleteActAll(guidSudsWaiting);
                    setState(DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_VACUUM_CYCLE_STARTED);
                }
            });
            env->actionMgr->createActCompleted(guidSudsWaiting, conn, QString(__PRETTY_FUNCTION__) + ": SYRINGE_AIR_RECOVERY_STATE_SUDS_INSERT_WAITING: SudsInserted");

            // Go to next state if possible
            if (env->ds.mcuData->getSudsInserted())
            {
                setState(DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_VACUUM_CYCLE_STARTED);
            }
        }
        break;
    //-----------------------------------
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_VACUUM_CYCLE_STARTED:
        LOG_INFO("SYRINGE_AIR_RECOVERY[%d]: STATE_VACUUM_CYCLE_STARTED\n", location);
        {
            env->actionMgr->deleteActAll(guidSyringesIdleWaiting);
            env->actionMgr->deleteActAll(guidSudsWaiting);
            env->actionMgr->deleteActAll(guidSubAction);

            // Set Fluid source to busy during recovery sequence
            DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
            fluidSourceSyringes[location].isBusy = true;
            env->ds.deviceData->setFluidSourceSyringes(fluidSourceSyringes);

            if (fluidSourceSyringes[location].currentVolumesTotal() > SYRINGE_AIR_RECOVERY_SYRINGE_VACUUM_VOL_MAX)
            {
                LOG_INFO("SYRINGE_AIR_RECOVERY[%d]: Syringe volume=%.1fml > %.1fml, Skipping Syringe Vacuum.\n", location, fluidSourceSyringes[location].currentVolumesTotal(), SYRINGE_AIR_RECOVERY_SYRINGE_VACUUM_VOL_MAX);
                setState(DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_VACUUM_CYCLE_DONE);
            }
            else
            {
                handleSubAction(DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_VACUUM_CYCLE_PROGRESS, DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_VACUUM_CYCLE_DONE, DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_VACUUM_CYCLE_FAILED);
                env->ds.mcuAction->actSyringeRecoveryVacuum(location, guidSubAction);
            }
        }
        break;
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_VACUUM_CYCLE_PROGRESS:
        LOG_INFO("SYRINGE_AIR_RECOVERY[%d]: STATE_VACUUM_CYCLE_PROGRESS\n", location);
        break;
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_VACUUM_CYCLE_FAILED:
        LOG_ERROR("SYRINGE_AIR_RECOVERY[%d]: STATE_VACUUM_CYCLE_FAILED\n", location);
        setState(DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_FAILED_HW_FAULT);
        break;
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_VACUUM_CYCLE_DONE:
        LOG_INFO("SYRINGE_AIR_RECOVERY[%d]: STATE_VACUUM_CYCLE_DONE\n", location);
        setState(DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_PRIME_STARTED);
        break;
    //-----------------------------------
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_PRIME_STARTED:
        LOG_INFO("SYRINGE_AIR_RECOVERY[%d]: STATE_PRIME_STARTED\n", location);
        {
            handleSubAction(DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_PRIME_PROGRESS, DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_PRIME_DONE, DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_PRIME_FAILED);
            env->ds.deviceAction->actSyringePrime(location, DS_DeviceDef::SUDS_PRIME_TYPE_SYRINGE_AIR_RECOVERY, guidSubAction);
        }
        break;
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_PRIME_PROGRESS:
        LOG_INFO("SYRINGE_AIR_RECOVERY[%d]: STATE_PRIME_PROGRESS\n", location);
        break;
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_PRIME_FAILED:
        LOG_ERROR("SYRINGE_AIR_RECOVERY[%d]: STATE_PRIME_FAILED\n", location);
        if (actStatusBuf.err == "USER_ABORT")
        {
            setState(DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_FAILED_USER_ABORT);
        }
        else if (actStatusBuf.err == _L("SUDS_REMOVED"))
        {
            setState(DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_FAILED_SUDS_REMOVED);
        }
        else
        {
            setState(DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_FAILED_HW_FAULT);
        }
        break;
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_PRIME_DONE:
        LOG_INFO("SYRINGE_AIR_RECOVERY[%d]: STATE_PRIME_DONE\n", location);
        {
            extraPrimeCount = 0;
            setState(DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_OUTLET_AIR_CHECK_STARTED);
        }
        break;
    //-----------------------------------
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_OUTLET_AIR_CHECK_STARTED:
        LOG_INFO("SYRINGE_AIR_RECOVERY[%d]: STATE_OUTLET_AIR_CHECK_STARTED\n", location);
        if (env->ds.mcuData->getSudsBubbleDetected())
        {
            setState(DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_OUTLET_AIR_CHECK_FAILED);
        }
        else
        {
            setState(DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_OUTLET_AIR_CHECK_DONE);
        }
        break;
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_OUTLET_AIR_CHECK_FAILED:
        LOG_ERROR("SYRINGE_AIR_RECOVERY[%d]: STATE_OUTLET_AIR_CHECK_FAILED\n", location);
        if (extraPrimeCount < SYRINGE_AIR_RECOVERY_EXTRA_PRIME_TRIALS_LIMIT)
        {
            LOG_WARNING("SYRINGE_AIR_RECOVERY[%d]: PrimeRetry[%d]: Air detected from outlet air sensor. Priming extra volume..\n", location, extraPrimeCount);
            setState(DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_EXTRA_PRIME_STARTED);
        }
        else
        {
            setState(DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_FAILED_OPERATION_FAILED);
        }
        break;
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_OUTLET_AIR_CHECK_DONE:
        LOG_INFO("SYRINGE_AIR_RECOVERY[%d]: STATE_OUTLET_AIR_CHECK_DONE\n", location);
        {
            LOG_INFO("SYRINGE_AIR_RECOVERY[%d]: Air Recovery Sequence for Syringe(%s) is completed\n", location, ImrParser::ToImr_FluidSourceSyringeIdx(location).CSTR());
            DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
            fluidSourceSyringes[location].isBusy = false;
            env->ds.deviceData->setFluidSourceSyringes(fluidSourceSyringes);

            env->ds.alertAction->deactivateFromSyringeIdx("ReservoirAirDetected", location);
            setState(DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_STARTED);
        }
        break;
    //-----------------------------------
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_EXTRA_PRIME_STARTED:
        LOG_INFO("SYRINGE_AIR_RECOVERY[%d]: STATE_EXTRA_PRIME_STARTED\n", location);
        {
            extraPrimeCount++;
            handleSubAction(DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_EXTRA_PRIME_PROGRESS, DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_EXTRA_PRIME_DONE, DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_EXTRA_PRIME_FAILED);
            env->ds.deviceAction->actSyringePrime(location, DS_DeviceDef::SUDS_PRIME_TYPE_SYRINGE_PRIME, guidSubAction);
        }
        break;
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_EXTRA_PRIME_PROGRESS:
        LOG_INFO("SYRINGE_AIR_RECOVERY[%d]: STATE_EXTRA_PRIME_PROGRESS\n", location);
        break;
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_EXTRA_PRIME_FAILED:
        LOG_ERROR("SYRINGE_AIR_RECOVERY[%d]: STATE_EXTRA_PRIME_FAILED\n", location);
        setState(DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_OUTLET_AIR_CHECK_STARTED);
        break;
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_EXTRA_PRIME_DONE:
        LOG_INFO("SYRINGE_AIR_RECOVERY[%d]: STATE_EXTRA_PRIME_DONE\n", location);
        setState(DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_OUTLET_AIR_CHECK_STARTED);
        break;
    //-----------------------------------
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_FAILED_SUDS_REMOVED:
        LOG_ERROR("SYRINGE_AIR_RECOVERY[%d]: STATE_FAILED_SUDS_REMOVED\n", location);
        setState(DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_SUSPENDED);
        break;
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_FAILED_USER_ABORT:
        LOG_ERROR("SYRINGE_AIR_RECOVERY[%d]: STATE_FAILED_USER_ABORT\n", location);
        setState(DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_SUSPENDED);
        break;
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_FAILED_INSUFFICIENT_VOLUME:
        LOG_ERROR("SYRINGE_AIR_RECOVERY[%d]: STATE_FAILED_INSUFFICIENT_VOLUME\n", location);
        setState(DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_FAILED);
        break;
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_FAILED_HW_FAULT:
        LOG_ERROR("SYRINGE_AIR_RECOVERY[%d]: STATE_FAILED_HW_FAULT\n", location);
        setState(DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_SUSPENDED);
        break;
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_FAILED_OPERATION_FAILED:
        LOG_ERROR("SYRINGE_AIR_RECOVERY[%d]: STATE_FAILED_OPERATION_FAILED\n", location);
        {
            DS_McuDef::SyringeAirCheckDigests syringeAirCheckDigests = env->ds.mcuData->getSyringeAirCheckDigests();
            env->ds.alertAction->activate("ReservoirAirRecoveryFailed", QString().asprintf("%s;%.1f", ImrParser::ToImr_FluidSourceSyringeIdx(location).CSTR(), syringeAirCheckDigests[location].airVolume));
            setState(DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_SUSPENDED);
        }
        break;
        //-----------------------------------
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_SUSPENDED:
        LOG_WARNING("SYRINGE_AIR_RECOVERY[%d]: STATE_SUSPENDED\n", location);
        {
            DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
            fluidSourceSyringes[location].isBusy = false;
            env->ds.deviceData->setFluidSourceSyringes(fluidSourceSyringes);
        }
        // Wait for user interaction
        break;
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_FAILED:
        LOG_ERROR("SYRINGE_AIR_RECOVERY[%d]: STATE_FAILED\n", location);
        setState(DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_DONE);
        break;
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_USER_END_WAITING:
        LOG_INFO("SYRINGE_AIR_RECOVERY[%d]: STATE_USER_END_WAITING\n", location);
        // Wait for user interaction
        break;
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_DONE:
        LOG_INFO("SYRINGE_AIR_RECOVERY[%d]: STATE_DONE\n", location);
        {
            DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
            fluidSourceSyringes[location].isBusy = false;

            env->ds.deviceData->setFluidSourceSyringes(fluidSourceSyringes);

            if (actStatusBuf.err == "")
            {
                actStatusBuf.state = DS_ACTION_STATE_COMPLETED;
            }

            actionCompleted(actStatusBuf);

            setState(DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_READY);
        }
        break;
    default:
        LOG_ERROR("SYRINGE_AIR_RECOVERY[%d]: Invalid State (%d)\n", location, state);
        break;
    }
}

DataServiceActionStatus WorkflowSyringeAirRecovery::actSyringeAirRecoveryStart(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "SyringeAirRecovery:Start");
    DS_WorkflowDef::WorkflowState workflowState = env->ds.workflowData->getWorkflowState();

    if (workflowState != DS_WorkflowDef::STATE_SYRINGE_AIR_RECOVERY_STARTED)
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "Invalid State";
        actionStarted(status);
        return status;
    }

    actStatusBuf = status;
    actStatusBuf.state = DS_ACTION_STATE_STARTED;

    actionStarted(actStatusBuf);

    setState(DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_STARTED);

    return actStatusBuf;
}

DataServiceActionStatus WorkflowSyringeAirRecovery::actSyringeAirRecoveryResume(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "SyringeAirRecovery:Resume");

    DS_WorkflowDef::WorkflowState workflowState = env->ds.workflowData->getWorkflowState();
    if (workflowState != DS_WorkflowDef::STATE_SYRINGE_AIR_RECOVERY_PROGRESS)
    {
        status.err = "Invalid Workflow State: " + ImrParser::ToImr_WorkFlowState(workflowState);
        status.state = DS_ACTION_STATE_INVALID_STATE;
        actionStarted(status);
        return status;
    }

    DS_WorkflowDef::SyringeAirRecoveryState state = env->ds.workflowData->getSyringeAirRecoveryState();
    DS_WorkflowDef::SyringeAirRecoveryState newState;

    switch (state)
    {
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_USER_START_WAITING_1:
        newState = DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_VOLUME_CHECKING;
        break;
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_USER_START_WAITING_2:
        newState = DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_SYRINGES_IDLE_WAITING;
        break;
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_SUSPENDED:
        newState = DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_VOLUME_CHECKING;
        break;
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_USER_END_WAITING:
        newState = DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_DONE;
        break;
    default:
        status.err = "Invalid Syringe Air Recovery State: " + ImrParser::ToImr_SyringeAirRecoveryState(state);
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
