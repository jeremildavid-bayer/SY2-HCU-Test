#include "Apps/AppManager.h"
#include "DeviceSyringeAirCheck.h"
#include "DataServices/Workflow/DS_WorkflowAction.h"
#include "DataServices/Workflow/DS_WorkflowData.h"
#include "DataServices/Alert/DS_AlertAction.h"
#include "DataServices/Alert/DS_AlertData.h"
#include "DataServices/Device/DS_DeviceAction.h"
#include "DataServices/Device/DS_DeviceData.h"
#include "DataServices/Mcu/DS_McuAction.h"
#include "DataServices/Mcu/DS_McuData.h"
#include "DataServices/Capabilities/DS_Capabilities.h"

DeviceSyringeAirCheck::DeviceSyringeAirCheck(QObject *parent, EnvGlobal *env_, EnvLocal *envLocal_, SyringeIdx location_) :
    ActionBaseExt(parent, env_),
    location(location_)
{
    envLocal = envLocal_;

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });

    state = STATE_READY;
}

DeviceSyringeAirCheck::~DeviceSyringeAirCheck()
{
}

void DeviceSyringeAirCheck::slotAppInitialised()
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
        }
    });
}

bool DeviceSyringeAirCheck::isBusy()
{
    return state != STATE_READY;
}

DataServiceActionStatus DeviceSyringeAirCheck::actAirCheck(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "AirCheck", QString().asprintf("%d", location));

    actStatusBuf = status;
    actStatusBuf.state = DS_ACTION_STATE_STARTED;
    actionStarted(actStatusBuf);

    setState(STATE_STARTED);

    return actStatusBuf;
}

void DeviceSyringeAirCheck::processState()
{
    switch (state)
    {
    case STATE_READY:
        LOG_INFO("AIR_CHECK[%d]: STATE_READY\n", location);
        break;
    case STATE_STARTED:
        LOG_INFO("AIR_CHECK[%d]: STATE_STARTED\n", location);
        {
            double requiredVolume = env->ds.capabilities->get_AirCheck_SyringeAirCheckRequiredVol();

            // Deactivate possible failed alerts
            env->ds.alertAction->deactivateFromSyringeIdx("ReservoirAirCheckFailed", location);

            DS_McuDef::SyringeStates syringeStates = env->ds.mcuData->getSyringeStates();

            if ( (syringeStates[location] == DS_McuDef::SYRINGE_STATE_PROCESSING) ||
                 (syringeStates[location] == DS_McuDef::SYRINGE_STATE_STOP_PENDING) )
            {
                actStatusBuf.state = DS_ACTION_STATE_INVALID_STATE;
                actStatusBuf.err = "PISTON_MOVING";
                setState(STATE_FAILED);
                return;
            }

            DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
            if (fluidSourceSyringes[location].currentVolumesTotal() < requiredVolume)
            {
                actStatusBuf.state = DS_ACTION_STATE_INVALID_STATE;
                actStatusBuf.err = "INSUFFICIENT_FLUID";
                setState(STATE_FAILED);
                return;
            }

            // Set Syringe to busy
            fluidSourceSyringes[location].isBusy = true;
            env->ds.deviceData->setFluidSourceSyringes(fluidSourceSyringes);
            setState(STATE_SYRINGE_AIR_CHECK_STARTED);
        }
        break;
    // ==========================================
    case STATE_SYRINGE_AIR_CHECK_STARTED:
        LOG_INFO("AIR_CHECK[%d]: STATE_SYRINGE_AIR_CHECK_STARTED\n", location);
        env->ds.alertAction->activate("SRUAirCheckingMUDS", ImrParser::ToImr_FluidSourceSyringeIdx(location));
        handleSubAction(STATE_SYRINGE_AIR_CHECK_PROGRESS,STATE_SYRINGE_AIR_CHECK_DONE, STATE_SYRINGE_AIR_CHECK_FAILED);
        env->ds.mcuAction->actSyringeAirCheck(location, guidSubAction);
        break;
    case STATE_SYRINGE_AIR_CHECK_PROGRESS:
        LOG_INFO("AIR_CHECK[%d]: STATE_SYRINGE_AIR_CHECK_PROGRESS\n", location);
        break;
    case STATE_SYRINGE_AIR_CHECK_FAILED:
        LOG_ERROR("AIR_CHECK[%d]: STATE_SYRINGE_AIR_CHECK_FAILED: status=%s\n", location, ImrParser::ToImr_DataServiceActionStatusStr(actStatusBuf).CSTR());
        setState(STATE_SYRINGE_GET_AIR_VOLUME_STARTED);
        break;
    case STATE_SYRINGE_AIR_CHECK_DONE:
        LOG_INFO("AIR_CHECK[%d]: STATE_SYRINGE_AIR_CHECK_DONE\n", location);
        setState(STATE_SYRINGE_GET_AIR_VOLUME_STARTED);
        break;
    // ==========================================
    case STATE_SYRINGE_GET_AIR_VOLUME_STARTED:
        LOG_INFO("AIR_CHECK[%d]: STATE_SYRINGE_GET_AIR_VOLUME_STARTED\n", location);
        handleSubAction(STATE_SYRINGE_GET_AIR_VOLUME_PROGRESS, STATE_SYRINGE_GET_AIR_VOLUME_DONE, STATE_SYRINGE_GET_AIR_VOLUME_FAILED);
        env->ds.mcuAction->actGetSyringeAirVol(location, guidSubAction);
        break;
    case STATE_SYRINGE_GET_AIR_VOLUME_PROGRESS:
        LOG_INFO("AIR_CHECK[%d]: STATE_SYRINGE_GET_AIR_VOLUME_PROGRESS\n", location);
        break;
    case STATE_SYRINGE_GET_AIR_VOLUME_FAILED:
        LOG_ERROR("AIR_CHECK[%d]: STATE_SYRINGE_GET_AIR_VOLUME_FAILED\n", location);
        setState(STATE_FAILED);
        break;
    case STATE_SYRINGE_GET_AIR_VOLUME_DONE:
        LOG_INFO("AIR_CHECK[%d]: STATE_SYRINGE_GET_AIR_VOLUME_DONE\n", location);
        {
            DS_McuDef::SyringeAirCheckDigests syringeAirCheckDigests = env->ds.mcuData->getSyringeAirCheckDigests();
            double airVolume = syringeAirCheckDigests[location].airVolume;
            double airVolume2 = syringeAirCheckDigests[location].airVolume2;
            LOG_INFO("AIR_CHECK[%d]: STATE_SYRINGE_GET_AIR_VOLUME_DONE: Calculated AirVolume = %.1fml.\n", location, airVolume);           

            DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
            double syringeVolume = fluidSourceSyringes[location].currentVolumesTotal();
            QString alertData = QString().asprintf("%s;%.1f;%.000001f;%.000001f", ImrParser::ToImr_FluidSourceSyringeIdx(location).CSTR(), syringeVolume, airVolume, airVolume2);
            env->ds.alertAction->activate("SRUReservoirAirCheckEnded", alertData);

            if (actStatusBuf.err == "")
            {
                setState(STATE_DONE);
            }
            else
            {
                setState(STATE_FAILED);
            }
        }
        break;
    case STATE_FAILED:
        LOG_ERROR("AIR_CHECK[%d]: STATE_FAILED: Status=%s\n", location, ImrParser::ToImr_DataServiceActionStatusStr(actStatusBuf).CSTR());
        if (actStatusBuf.err == "INSUFFICIENT_FLUID")
        {
            double requiredVolume = env->ds.capabilities->get_AirCheck_SyringeAirCheckRequiredVol();
            env->ds.alertAction->activate("InsufficientVolumeForReservoirAirCheck", QString().asprintf("%s;%.0f", ImrParser::ToImr_FluidSourceSyringeIdx(location).CSTR(), requiredVolume));
        }
        else if ( (actStatusBuf.err == "AIR_DETECTED") ||
                  (actStatusBuf.err == "INVALID_STATE") )
        {
            // Set SUDS state to 'Not Ready To Use'
            env->ds.deviceAction->actSudsSetNeedsPrime(true);
            env->ds.deviceData->setIsFirstAutoPrimeCompleted(false);

            env->ds.alertAction->deactivateFromSyringeIdx("ReservoirAirDetected", location);

            DS_McuDef::SyringeAirCheckDigests syringeAirCheckDigests = env->ds.mcuData->getSyringeAirCheckDigests();
            double airVolume = syringeAirCheckDigests[location].airVolume;
            env->ds.alertAction->activate("ReservoirAirDetected", QString().asprintf("%s;%.1f", ImrParser::ToImr_FluidSourceSyringeIdx(location).CSTR(), airVolume));
        }
        else
        {
            QString alertData = QString().asprintf("%s;%s", ImrParser::ToImr_FluidSourceSyringeIdx(location).CSTR(), actStatusBuf.err.CSTR());
            env->ds.alertAction->activate("ReservoirAirCheckFailed", alertData);
        }
        setState(STATE_DONE);
        break;
    case STATE_DONE:
        LOG_INFO("AIR_CHECK[%d]: STATE_DONE\n", location);
        {
            if (actStatusBuf.err == "")
            {
                env->ds.alertAction->deactivateFromSyringeIdx("ReservoirAirDetected", location);
                actStatusBuf.state = DS_ACTION_STATE_COMPLETED;
            }

            DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
            fluidSourceSyringes[location].isBusy = false;
            env->ds.deviceData->setFluidSourceSyringes(fluidSourceSyringes);
            env->ds.alertAction->deactivate("SRUAirCheckingMUDS", ImrParser::ToImr_FluidSourceSyringeIdx(location));
            actionCompleted(actStatusBuf);

            setState(STATE_READY);

            // Start SyringeAirRecovery Sequence if required
            if (env->ds.alertAction->isActivated("ReservoirAirDetected", "", true))
            {
                LOG_WARNING("AIR_CHECK[%d]: Syringe Air Recovery will be started soon..\n", location);

                // Make sure MUDS SOD is aborted so that WorflowMain can start the Syringe Air Recovery sequence.
                env->ds.deviceAction->actMudsSodAbort();

                QTimer::singleShot(500, this, [=]{
                    env->ds.workflowAction->actSyringeAirRecoveryStart();
                });
            }
        }
        break;
    default:
        LOG_ERROR("AIR_CHECK[%d]: Unknown state(%d)\n", location, state);
        break;
    }
}



