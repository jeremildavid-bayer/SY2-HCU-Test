#include "Apps/AppManager.h"
#include "DeviceMuds.h"
#include "Common/ImrParser.h"
#include "Common/Translator.h"
#include "Common/Util.h"
#include "DataServices/Device/DS_DeviceData.h"
#include "DataServices/Device/DS_DeviceAction.h"
#include "DataServices/Workflow/DS_WorkflowData.h"
#include "DataServices/Exam/DS_ExamData.h"
#include "DataServices/Exam/DS_ExamAction.h"
#include "DataServices/Mcu/DS_McuAction.h"
#include "DataServices/Mcu/Internal/McuAlarm.h"
#include "DataServices/Alert/DS_AlertAction.h"
#include "DataServices/Alert/DS_AlertData.h"
#include "DataServices/System/DS_SystemData.h"
#include "DataServices/Capabilities/DS_Capabilities.h"
#include "DataServices/CfgLocal/DS_CfgLocal.h"
#include "DataServices/CfgGlobal/DS_CfgGlobal.h"

DeviceMuds::DeviceMuds(QObject *parent, EnvGlobal *env_) :
    ActionBase(parent, env_)
{
    envLocal = new EnvLocal("DS_Device-Muds", "DEVICE_MUDS", LOG_MID_SIZE_BYTES);
    isActive = false;

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });

    // Initialisation
    connect(&tmrUseLifeUpdate, SIGNAL(timeout()), this, SLOT(slotUpdateUsedTimeStatus()));

    deviceMudsSod = new DeviceMudsSod(parent, env, envLocal);
    deviceMudsDisengage = new DeviceMudsDisengage(parent, env, envLocal);
    deviceMudsPreload = new DeviceMudsPreload(parent, env, envLocal);
}

DeviceMuds::~DeviceMuds()
{
    delete deviceMudsSod;
    delete deviceMudsDisengage;
    delete deviceMudsPreload;
    delete envLocal;
}

void DeviceMuds::slotAppInitialised()
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
            isActive = true;

            updateInsufficientVolumeAlerts();
            updateSyringeIsReadyState();
            updateMudsAlerts();
            updateSyringeInstalledState();
            handleSyringeVolsChanged();
            setFluidSourceMudsStatus();
            handleInjectionProgressChanged();
            slotUpdateUsedTimeStatus();
        }
        else if ( (workflowState == DS_WorkflowDef::STATE_INACTIVE) &&
                  (workflowStatePrev != DS_WorkflowDef::STATE_INACTIVE) )
        {
            isActive = false;
        }

        DS_DeviceDef::FluidSource fluidSourceMuds = env->ds.deviceData->getFluidSourceMuds();
        switch (workflowState)
        {
        case DS_WorkflowDef::STATE_SOD_DONE:
            if (!fluidSourceMuds.isReady)
            {
                LOG_INFO("signalDataChanged_WorkflowState(): workflowState=%s, fluidSourceMuds.isReady=true\n", ImrParser::ToImr_WorkFlowState(workflowState).CSTR());
                fluidSourceMuds.isReady = true;
                env->ds.deviceData->setFluidSourceMuds(fluidSourceMuds);
            }
            break;
        default:
            break;
        }
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_MudsPresent, this, [=](bool present, bool prevPresent) {
        if (present == prevPresent)
        {
            return;
        }

        if (present)
        {
            env->ds.alertAction->activate("SRUInsertedMUDS");
        }
        else
        {
            env->ds.alertAction->activate("SRURemovedMUDS");
        }

        if (!isActive)
        {
            return;
        }

        updateMudsAlerts();
        updateSyringeInstalledState();
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_MudsLatched, this, [=] {
        if (!isActive)
        {
            return;
        }

        updateMudsAlerts();
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_DoorState, this, [=] {
        if (!isActive)
        {
            return;
        }

        updateMudsAlerts();
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_SyringeVols, this, [=] {
        if (!isActive)
        {
            return;
        }

        handleSyringeVolsChanged();
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_PlungerStates, this, [=] {
        if (!isActive)
        {
            return;
        }

        handleSyringeVolsChanged();
        updateSyringeInstalledState();
    });

    connect(env->ds.systemData, &DS_SystemData::signalDataChanged_StatePath, this, [=]() {
        if (!isActive)
        {
            return;
        }

        handleInjectionProgressChanged();
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_StopcockPosAll, this, [=]() {
        if (!isActive)
        {
            return;
        }

        setFluidSourceMudsStatus();
    });

    connect(env->ds.alertData, &DS_AlertData::signalDataChanged_ActiveAlerts, this, [=]() {
        if (!isActive)
        {
            return;
        }

        QTimer::singleShot(1, [=] {
            // Asynch process: ActiveAlerts can be changed from updateInsufficientVolumeAlerts()
            updateInsufficientVolumeAlerts();
        });
        updateSyringeIsReadyState();
    });

    connect(env->ds.workflowData, &DS_WorkflowData::signalDataChanged_MudsSodStatus, this, [=]() {
        if (!isActive)
        {
            return;
        }

        updateInsufficientVolumeAlerts();
        updateSyringeIsReadyState();
    });

    connect(env->ds.deviceData, &DS_DeviceData::signalDataChanged_FluidSourceMuds, this, [=](const DS_DeviceDef::FluidSource &fluidSourceMuds, const DS_DeviceDef::FluidSource &prevFluidSourceMuds) {
        if (!isActive)
        {
            return;
        }

        if (fluidSourceMuds.isInstalled() != prevFluidSourceMuds.isInstalled())
        {
            updateInsufficientVolumeAlerts();
            slotUpdateUsedTimeStatus();
        }
    });

    connect(env->ds.deviceData, &DS_DeviceData::signalDataChanged_FluidSourceSyringes, this, [=]() {
        if (!isActive)
        {
            return;
        }

        updateInsufficientVolumeAlerts();
        setFluidSourceMudsStatus();
        QTimer::singleShot(1, [=] { updateSyringeIsReadyState(); });
    });

    connect(env->ds.deviceData, &DS_DeviceData::signalDataChanged_FluidSourceSuds, this, [=]() {
        if (!isActive)
        {
            return;
        }

        updateInsufficientVolumeAlerts();
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_InjectorStatus, this, [=]() {
        if (!isActive)
        {
            return;
        }

        setFluidSourceMudsStatus();
    });

    connect(env->ds.examData, &DS_ExamData::signalDataChanged_ExecutedSteps, this, [=] {
        if (!isActive)
        {
            return;
        }

        handleInjectionProgressChanged();
    });

    connect(env->ds.cfgGlobal, &DS_CfgGlobal::signalConfigChanged_Configuration_UseLife_MUDSUseLifeLimitEnforced, this, [=]() {
        if (!isActive)
        {
            return;
        }
        slotUpdateUsedTimeStatus();
    });

    connect(env->ds.cfgGlobal, &DS_CfgGlobal::signalConfigChanged_Configuration_UseLife_MUDSUseLifeLimitHours, this, [=]() {
        if (!isActive)
        {
            return;
        }
        slotUpdateUsedTimeStatus();
    });

    connect(env->ds.cfgGlobal, &DS_CfgGlobal::signalConfigChanged_Configuration_UseLife_MUDSUseLifeGraceHours, this, [=]() {
        if (!isActive)
        {
            return;
        }
        slotUpdateUsedTimeStatus();
    });

    configureContrastBottleCountReachedAlert();
}

void DeviceMuds::updateMudsAlerts()
{
    DS_McuDef::DoorState doorState = env->ds.mcuData->getDoorState();
    bool mudsLatched = env->ds.mcuData->getMudsLatched();
    bool mudsPresent = env->ds.mcuData->getMudsPresent();

    if ( (doorState == DS_McuDef::DOOR_CLOSED) &&
         (mudsPresent) &&
         (!mudsLatched) )
    {
        env->ds.alertAction->activate("MUDSRestraintUnlatched");
    }
    else
    {
        env->ds.alertAction->deactivate("MUDSRestraintUnlatched");
    }
}

void DeviceMuds::updateInsufficientVolumeAlerts()
{
    DS_DeviceDef::FluidSource fluidSourceMuds = env->ds.deviceData->getFluidSourceMuds();
    if (!fluidSourceMuds.isInstalled())
    {
        return;
    }

    DS_WorkflowDef::MudsSodStatus mudsSodStatus = env->ds.workflowData->getMudsSodStatus();
    DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
    DS_DeviceDef::FluidSource fluidSourceSuds = env->ds.deviceData->getFluidSourceSuds();
    double syringeAirCheckVolRequired = env->ds.capabilities->get_AirCheck_SyringeAirCheckRequiredVol();
    double autoPrimeVolRequired = env->ds.workflowData->getAutoPrimeVolume();

    for (int syrIdx = 0; syrIdx < SYRINGE_IDX_MAX; syrIdx++)
    {
        SyringeIdx syringeIdx = (SyringeIdx)syrIdx;

        if ( (fluidSourceSyringes[syringeIdx].isBusy) ||
             (fluidSourceSyringes[syringeIdx].needsReplaced) ||
             (!mudsSodStatus.syringeSodStatusAll[syringeIdx].plungerEngaged) )
        {
            continue;
        }

        QString syringeIdxStr = ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdx).CSTR();
        double syringeVol = fluidSourceSyringes[syringeIdx].currentVolumesTotal();

        // ------------------------------
        // Update Alert: InsufficientVolumeForCalSlack
        double syringeCalSlackVolRequired = env->ds.deviceData->getSyringeCalSlackVolRequired(syringeIdx);

        if ( (!mudsSodStatus.syringeSodStatusAll[syringeIdx].calSlackDone) &&
             (!mudsSodStatus.syringeSodStatusAll[syringeIdx].actionInProgress) &&
             (syringeVol > 0) &&
             (Util::isFloatVarGreaterThan(syringeCalSlackVolRequired, syringeVol)) )
        {
            QString alertData = QString().asprintf("%s;%.0f", syringeIdxStr.CSTR(), syringeCalSlackVolRequired);
            if (!env->ds.alertAction->isActivated("InsufficientVolumeForCalSlack", alertData))
            {
                LOG_WARNING("updateInsufficientVolumeAlerts(): InsufficientVolumeForCalSlack: Syringe=%s: PrimeRequiredVol=%.1f, NotPrimed && SyringeVol=%.1f\n", syringeIdxStr.CSTR(), syringeCalSlackVolRequired, syringeVol);
                env->ds.alertAction->activate("InsufficientVolumeForCalSlack", alertData);
            }
        }
        else
        {
            if (env->ds.alertAction->isActivatedWithSyringeIdx("InsufficientVolumeForCalSlack", syringeIdx))
            {
                LOG_INFO("updateInsufficientVolumeAlerts(): SufficientVolumeForCalSlack: Syringe=%s: CalSlackDone=%s, SyringeVol=%.1f\n", syringeIdxStr.CSTR(), mudsSodStatus.syringeSodStatusAll[syringeIdx].calSlackDone ? "TRUE" : "FALSE", syringeVol);
                env->ds.alertAction->deactivateFromSyringeIdx("InsufficientVolumeForCalSlack", syringeIdx);
            }
        }

        // ------------------------------
        // Update Alert: InsufficientVolumeForMUDSPrime
        double syringePrimeVolRequired = env->ds.deviceData->getSyringePrimeVolRequired(syringeIdx);

        if ( (mudsSodStatus.syringeSodStatusAll[syringeIdx].calSlackDone) &&
             (!mudsSodStatus.syringeSodStatusAll[syringeIdx].primed) &&
             (!mudsSodStatus.syringeSodStatusAll[syringeIdx].actionInProgress) &&
             (syringeVol > 0) &&
             (Util::isFloatVarGreaterThan(syringePrimeVolRequired, syringeVol)) )
        {
            QString alertData = QString().asprintf("%s;%.0f", syringeIdxStr.CSTR(), syringePrimeVolRequired);
            if (!env->ds.alertAction->isActivated("InsufficientVolumeForMUDSPrime", alertData))
            {
                LOG_WARNING("updateInsufficientVolumeAlerts(): InsufficientVolumeForMUDSPrime: Syringe=%s: PrimeRequiredVol=%.1f, NotPrimed && SyringeVol=%.1f\n", syringeIdxStr.CSTR(), syringePrimeVolRequired, syringeVol);
                env->ds.alertAction->activate("InsufficientVolumeForMUDSPrime", alertData);
            }
        }
        else
        {
            if (env->ds.alertAction->isActivatedWithSyringeIdx("InsufficientVolumeForMUDSPrime", syringeIdx))
            {
                LOG_INFO("updateInsufficientVolumeAlerts(): SufficientVolumeForMUDSPrime: Syringe=%s: Primed=%s, SyringeVol=%.1f\n", syringeIdxStr.CSTR(), mudsSodStatus.syringeSodStatusAll[syringeIdx].primed ? "TRUE" : "FALSE", syringeVol);
                env->ds.alertAction->deactivateFromSyringeIdx("InsufficientVolumeForMUDSPrime", syringeIdx);
            }
        }

        // ------------------------------
        // Update Alert: InsufficientVolumeForReservoirAirCheck
        if ( (mudsSodStatus.syringeSodStatusAll[syringeIdx].isCompleted()) &&
             (syringeVol >= 0) &&
             (Util::isFloatVarGreaterThan(syringeAirCheckVolRequired, syringeVol, 1)) )
        {
            // Alert should be raised from DeviceSyringeAirCheck
        }
        else
        {
            if (env->ds.alertAction->isActivatedWithSyringeIdx("InsufficientVolumeForReservoirAirCheck", syringeIdx))
            {
                LOG_INFO("updateInsufficientVolumeAlerts(): SufficientVolumeForReservoirAirCheck: Syringe=%s: SODDone=%s, SyringeVol=%.1f\n", syringeIdxStr.CSTR(), mudsSodStatus.syringeSodStatusAll[syringeIdx].isCompleted() ? "TRUE" : "FALSE", syringeVol);
                env->ds.alertAction->deactivateFromSyringeIdx("InsufficientVolumeForReservoirAirCheck", syringeIdx);
            }
        }

        // ------------------------------
        // Update Alert: InsufficientVolumeForReservoirAirRecovery
        DS_McuDef::SyringeAirCheckDigests syringeAirCheckDigests = env->ds.mcuData->getSyringeAirCheckDigests();
        double airVolume = syringeAirCheckDigests[syringeIdx].airVolume;
        double primeExtraVolume = env->ds.capabilities->get_AirCheck_SyringeAirRecoveryPrimeExtraVol();
        double syringeAirRecoveryVolRequired = syringeAirCheckVolRequired + airVolume + primeExtraVolume;
        bool reservoirAirRecoveryRequired = env->ds.alertAction->isActivatedWithSyringeIdx("ReservoirAirDetected", syringeIdx);

        if ( (reservoirAirRecoveryRequired) &&
             (mudsSodStatus.syringeSodStatusAll[syringeIdx].isCompleted()) &&
             (syringeVol >= 0) &&
             (Util::isFloatVarGreaterThan(syringeAirRecoveryVolRequired, syringeVol, 1)) )
        {
            // Alert should be raised from WorkflowSyringeAirRecovery
        }
        else
        {
            if (env->ds.alertAction->isActivatedWithSyringeIdx("InsufficientVolumeForReservoirAirRecovery", syringeIdx))
            {
                LOG_INFO("updateInsufficientVolumeAlerts(): SufficientVolumeForReservoirAirRecovery: Syringe=%s: reservoirAirRecoveryRequired=%s, SODDone=%s, SyringeVol=%.1f\n",
                         syringeIdxStr.CSTR(), reservoirAirRecoveryRequired ? "TRUE" : "FALSE", mudsSodStatus.syringeSodStatusAll[syringeIdx].isCompleted() ? "TRUE" : "FALSE", syringeVol);
                env->ds.alertAction->deactivateFromSyringeIdx("InsufficientVolumeForReservoirAirRecovery", syringeIdx);
            }
        }

        // ------------------------------
        // Update Alert: InsufficientVolumeForSUDSPrime
        if (syringeIdx == SYRINGE_IDX_SALINE)
        {
            if (fluidSourceSuds.isBusy)
            {
                // wait until SUDS is idle
            }
            else if ( (mudsSodStatus.syringeSodStatusAll[syringeIdx].isCompleted()) &&
                      (fluidSourceSuds.isInstalled()) &&
                      (!fluidSourceSuds.isReady) &&
                      (!fluidSourceSuds.needsReplaced) &&
                      (syringeVol >= 0) &&
                      (Util::isFloatVarGreaterThan(autoPrimeVolRequired, syringeVol, 1)) )
            {
                // Alert should be raised from DeviceSyringePrime
            }
            else
            {
                if (env->ds.alertAction->isActivatedWithSyringeIdx("InsufficientVolumeForSUDSPrime", syringeIdx))
                {
                    LOG_INFO("updateInsufficientVolumeAlerts(): SufficientVolumeForSUDSPrime: Syringe=%s: SODDone=%s, SyringeVol=%.1f\n",
                             syringeIdxStr.CSTR(), mudsSodStatus.syringeSodStatusAll[syringeIdx].isCompleted() ? "TRUE" : "FALSE", syringeVol);

                    env->ds.alertAction->deactivateFromSyringeIdx("InsufficientVolumeForSUDSPrime", syringeIdx);
                }

                if (env->ds.alertAction->isActivated("AutoPrimeFailed", "INSUFFICIENT_FLUID"))
                {
                    env->ds.alertAction->deactivate("AutoPrimeFailed", "INSUFFICIENT_FLUID");
                }
            }
        }
    }
}

void DeviceMuds::updateSyringeInstalledState()
{
    bool mudsPresent = env->ds.mcuData->getMudsPresent();
    DS_McuDef::PlungerStates plungerStates = env->ds.mcuData->getPlungerStates();

    DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
    for (int syringeIdx = 0; syringeIdx < fluidSourceSyringes.length(); syringeIdx++)
    {
        bool plungerEngaged = (plungerStates[syringeIdx] == DS_McuDef::PLUNGER_STATE_ENGAGED);
        bool syringeIsInstalled = (mudsPresent || plungerEngaged);

        // Update Syringe Installed State
        if (fluidSourceSyringes[syringeIdx].isInstalled() != syringeIsInstalled)
        {
            fluidSourceSyringes[syringeIdx].setIsInstalled(syringeIsInstalled);
        }
    }

    env->ds.deviceData->setFluidSourceSyringes(fluidSourceSyringes);
}

void DeviceMuds::updateSyringeIsReadyState()
{
    DS_WorkflowDef::MudsSodStatus mudsSodStatus = env->ds.workflowData->getMudsSodStatus();
    DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
    QVariantList activeAlerts = env->ds.alertData->getActiveAlerts();

    for (int syrIdx = 0; syrIdx < fluidSourceSyringes.length(); syrIdx++)
    {
        SyringeIdx syringeIdx = (SyringeIdx)syrIdx;
        fluidSourceSyringes[syringeIdx].isReady = mudsSodStatus.syringeSodStatusAll[syringeIdx].isCompleted();
        if (fluidSourceSyringes[syringeIdx].isReady)
        {
            if (fluidSourceSyringes[syringeIdx].needsReplaced)
            {
                fluidSourceSyringes[syringeIdx].isReady = false;
            }
        }

        if (fluidSourceSyringes[syringeIdx].isReady)
        {
            // Check if air is detected
            QString syringeIdxStr = ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdx);
            for (int alertIdx = 0; alertIdx < activeAlerts.length(); alertIdx++)
            {
                QString alertCodeName = activeAlerts[alertIdx].toMap()[_L("CodeName")].toString();
                QString alertData = activeAlerts[alertIdx].toMap()[_L("Data")].toString();
                if ( (alertCodeName == "ReservoirAirDetected") ||
                     (alertCodeName == "ReservoirAirCheckFailed") ||
                     (alertCodeName == "InsufficientVolumeForReservoirAirCheck") ||
                     (alertCodeName == "InsufficientVolumeForReservoirAirRecovery") )
                {
                    if (alertData.contains(syringeIdxStr))
                    {
                        // Air is detected in the syringe OR
                        // Air Detection cannot be started OR
                        // Air Recovery cannot be started
                        fluidSourceSyringes[syringeIdx].isReady = false;
                        break;
                    }
                }
            }
        }
    }

    env->ds.deviceData->setFluidSourceSyringes(fluidSourceSyringes);
}

void DeviceMuds::handleSyringeVolsChanged()
{
    QList<double> syringeVols = env->ds.mcuData->getSyringeVols();
    DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
    DS_McuDef::PlungerStates plungerStates = env->ds.mcuData->getPlungerStates();

    for (int syringeIdx = 0; syringeIdx < SYRINGE_IDX_MAX; syringeIdx++)
    {
        double syringeVol = syringeVols[syringeIdx];
        QString adjustedReason = "";

        double volMinAdjustmentMin = env->ds.capabilities->get_FluidControl_SyringeVolumeAdjustmentLimitMin();
        double volMinAdjustmentMax = env->ds.capabilities->get_FluidControl_SyringeVolumeAdjustmentLimitMax();

        // LIE_FEATURE: MCU Syringe Volume Adjustment: Volume adjusted due to FD features
        if ( (fluidSourceSyringes[syringeIdx].sourcePackages.length() == 0) &&
             (plungerStates[syringeIdx] == DS_McuDef::PLUNGER_STATE_ENGAGED) )
        {
            if (fluidSourceSyringes[syringeIdx].currentVolumes[0] != 0)
            {
                adjustedReason = "Engaged & NotFilled";
            }
            syringeVol = 0;
        }
        else if (syringeVol < 0)
        {
            adjustedReason = "Vol < 0";
            syringeVol = 0;
        }
        else if (syringeVol > SYRINGE_VOLUME_MAX)
        {
            adjustedReason = QString().asprintf("Vol > %.1f", SYRINGE_VOLUME_MAX);
            syringeVol = SYRINGE_VOLUME_MAX;
        }
        else if ( (syringeVol >= volMinAdjustmentMax) &&
                  (syringeVol < SYRINGE_VOLUME_MAX) )
        {
            adjustedReason = QString().asprintf("Vol >= %.1f", volMinAdjustmentMax);
            syringeVol = SYRINGE_VOLUME_MAX;
        }
        else if ( (syringeVol > 0) &&
                  (syringeVol < volMinAdjustmentMin) )
        {
            adjustedReason = QString().asprintf("Vol < %.1f", volMinAdjustmentMin);
            syringeVol = 0;
        }

        if (adjustedReason != "")
        {
            if (plungerStates[syringeIdx] == DS_McuDef::PLUNGER_STATE_ENGAGED)
            {
                LOG_INFO("HandleSyringeVolsChanged: Syringe[%d].Volume is adjusted from %.1f to %.1f. Reason=(%s)\n", syringeIdx, syringeVols[syringeIdx], syringeVol, adjustedReason.CSTR());
            }
        }

        fluidSourceSyringes[syringeIdx].currentVolumes[0] = syringeVol;
    }

    env->ds.deviceData->setFluidSourceSyringes(fluidSourceSyringes);
}

void DeviceMuds::handleInjectionProgressChanged()
{
    DS_SystemDef::StatePath statePath = env->ds.systemData->getStatePath();

    if ( (statePath != DS_SystemDef::STATE_PATH_READY_ARMED) &&
         (statePath != DS_SystemDef::STATE_PATH_EXECUTING) &&
         (statePath != DS_SystemDef::STATE_PATH_BUSY_HOLDING) &&
         (statePath != DS_SystemDef::STATE_PATH_BUSY_FINISHING) )
    {
        // Not injecting state.
        return;
    }

    const DS_ExamDef::InjectionStep *executingStep = env->ds.examData->getExecutingStep();
    const DS_ExamDef::InjectionPhase *executingPhase = env->ds.examData->getExecutingPhase();

    QString err = "";
    if (executingStep == NULL)
    {
        err = "HandleInjectionProgressChanged(): Failed to get executing step\n";
    }
    else if (executingPhase == NULL)
    {
        err = "HandleInjectionProgressChanged(): Failed to get executing phase\n";
    }

    if (err != "")
    {
        LOG_ERROR("%s", err.CSTR());
        env->ds.alertAction->activate("HCUInternalSoftwareError", err);
        return;
    }

    // Update FluidSourceSyringes
    DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
    bool syringeBusy1 = false, syringeBusy2 = false, syringeBusy3 = false;

    if ( (statePath == DS_SystemDef::STATE_PATH_EXECUTING) &&
         (executingPhase->type == DS_ExamDef::INJECTION_PHASE_TYPE_FLUID) )
    {
        if (executingPhase->contrastPercentage == 0)
        {
            syringeBusy1 = true;
            syringeBusy2 = false;
            syringeBusy3 = false;
        }
        else
        {
            syringeBusy1 = (executingPhase->contrastPercentage < 100);
            if (executingStep->contrastFluidLocation == DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_2)
            {
                syringeBusy2 = true;
                syringeBusy3 = false;
            }
            else
            {
                syringeBusy2 = false;
                syringeBusy3 = true;
            }
        }
    }
    fluidSourceSyringes[SYRINGE_IDX_SALINE].isBusy = syringeBusy1;
    fluidSourceSyringes[SYRINGE_IDX_CONTRAST1].isBusy = syringeBusy2;
    fluidSourceSyringes[SYRINGE_IDX_CONTRAST2].isBusy = syringeBusy3;

    /*LOG_DEBUG("fluidSourceSyringes.IsBusy= %s, %s, %s\n",
              fluidSourceSyringes[SYRINGE_IDX_SALINE].isBusy ? "BUSY" : "-",
              fluidSourceSyringes[SYRINGE_IDX_CONTRAST1].isBusy ? "BUSY" : "-",
              fluidSourceSyringes[SYRINGE_IDX_CONTRAST2].isBusy ? "BUSY" : "-");*/

    env->ds.deviceData->setFluidSourceSyringes(fluidSourceSyringes);
}

void DeviceMuds::setFluidSourceMudsStatus()
{
    DS_DeviceDef::FluidSource fluidSourceMuds = env->ds.deviceData->getFluidSourceMuds();
    DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();

    SyringeIdx activeSyringeIdx = SYRINGE_IDX_NONE;
    SyringeIdx mudsLineFluidSyringeIdx = env->ds.deviceData->getMudsLineFluidSyringeIndex();

    DS_McuDef::StopcockPosAll stopcockPosAll = env->ds.mcuData->getStopcockPosAll();

    // Check if any SC is in moving state.
    for (int syringeIdx = 0; syringeIdx < fluidSourceSyringes.length(); syringeIdx++)
    {
        if ( (stopcockPosAll[syringeIdx] != DS_McuDef::STOPCOCK_POS_FILL) &&
             (stopcockPosAll[syringeIdx] != DS_McuDef::STOPCOCK_POS_INJECT) &&
             (stopcockPosAll[syringeIdx] != DS_McuDef::STOPCOCK_POS_CLOSED) )
        {
            LOG_DEBUG("SetFluidSourceMudsStatus(): Stopcock[%s]=%s: Cannot determine ActiveSyringe\n",
                      ImrParser::ToImr_FluidSourceSyringeIdx((SyringeIdx)syringeIdx).CSTR(),
                      ImrParser::ToImr_StopcockPos(stopcockPosAll[syringeIdx]).CSTR());
            return;
        }
    }

    // 1st Check: ML should be set from busy syringe
    // syringeIdx is interated from end to front so that activeSyringeIdx is set to C1/2 instead of S0 during dualflow state.
    for (int syringeIdx = fluidSourceSyringes.length() - 1; syringeIdx >= 0; syringeIdx--)
    {
        if ( (fluidSourceSyringes[syringeIdx].isBusy) &&
             (stopcockPosAll[syringeIdx] == DS_McuDef::STOPCOCK_POS_INJECT) )
        {
            activeSyringeIdx = (SyringeIdx)syringeIdx;
            LOG_DEBUG("SetFluidSourceMudsStatus(): ActiveSyringeIdx=%s..\n", ImrParser::ToImr_FluidSourceSyringeIdx(activeSyringeIdx).CSTR());

            if (mudsLineFluidSyringeIdx != activeSyringeIdx)
            {
                LOG_DEBUG("SetFluidSourceMudsStatus(): Syringe=BUSY AND Stopcock=%s: ActiveSyringeIdx=%s..\n",
                          ImrParser::ToImr_StopcockPos(stopcockPosAll[syringeIdx]).CSTR(),
                          ImrParser::ToImr_FluidSourceSyringeIdx(activeSyringeIdx).CSTR());
            }
            break;
        }
    }

    // 2nd Check: ML should be set to busy when Injecting NOT filling
    DS_McuDef::InjectorStatus injectorStatus = env->ds.mcuData->getInjectorStatus();
    DS_McuDef::InjectDigest injectDigest = env->ds.mcuData->getInjectDigest();

    if (injectorStatus.state == DS_McuDef::INJECTOR_STATE_DELIVERING)
    {
        bool injectStatusOk = true;

        DS_McuDef::InjectionProtocol mcuInjProtocol = env->ds.mcuData->getInjectionProtocol();
        if ( (injectDigest.phaseIdx < 0) ||
             (injectDigest.phaseIdx >= mcuInjProtocol.phases.length()) )
        {
            injectStatusOk = false;
        }
        else
        {
            DS_McuDef::InjectionPhase *curPhase = &mcuInjProtocol.phases[injectDigest.phaseIdx];
            switch (curPhase->type)
            {
            case DS_McuDef::INJECTION_PHASE_TYPE_SALINE:
                activeSyringeIdx = SYRINGE_IDX_SALINE;
                break;
            case DS_McuDef::INJECTION_PHASE_TYPE_CONTRAST1:
            case DS_McuDef::INJECTION_PHASE_TYPE_DUAL1:
                activeSyringeIdx = SYRINGE_IDX_CONTRAST1;
                break;
            case DS_McuDef::INJECTION_PHASE_TYPE_CONTRAST2:
            case DS_McuDef::INJECTION_PHASE_TYPE_DUAL2:
                activeSyringeIdx = SYRINGE_IDX_CONTRAST2;
                break;
            case DS_McuDef::INJECTION_PHASE_TYPE_PAUSE:
                break;
            default:
                injectStatusOk = false;
                break;
            }
        }

        if ( (activeSyringeIdx != SYRINGE_IDX_NONE) &&
             (mudsLineFluidSyringeIdx != activeSyringeIdx) )
        {
            LOG_DEBUG("SetFluidSourceMudsStatus(): Injecting in %s..\n", ImrParser::ToImr_FluidSourceSyringeIdx(activeSyringeIdx).CSTR());
        }

        if (!injectStatusOk)
        {
            QString err = QString().asprintf("Bad Injection Status\n\nInjectDigest=%s\n\ninjectorStatus=%s\n\nmcuInjProtocol=%s\n",
                                            ImrParser::ToImr_InjectDigestStr(injectDigest).CSTR(),
                                            Util::qVarientToJsonData(ImrParser::ToImr_McuInjectorStatus(injectorStatus)).CSTR(),
                                            Util::qVarientToJsonData(ImrParser::ToImr_McuInjectionProtocol(mcuInjProtocol)).CSTR());
            LOG_ERROR("setFluidSourceMudsStatus(): %s", err.CSTR());
            env->ds.alertAction->activate("HCUInternalSoftwareError", err);
        }
    }

    bool mudsIsBusy = (activeSyringeIdx != SYRINGE_IDX_NONE);
    if (fluidSourceMuds.isBusy != mudsIsBusy)
    {
        fluidSourceMuds.isBusy = mudsIsBusy;
        LOG_DEBUG("SetFluidSourceMudsStatus(): ActiveSyringeIdx=%s, FluidSourceMuds.isBusy=%s\n", ImrParser::ToImr_FluidSourceSyringeIdx(activeSyringeIdx).CSTR(), fluidSourceMuds.isBusy ? "BUSY" : "IDLE");
    }

    // Set mudsLineFluidSyringeIdx and Muds.SourcePackages
    if (activeSyringeIdx != SYRINGE_IDX_NONE)
    {
        if (fluidSourceSyringes[activeSyringeIdx].sourcePackages.length() > 0)
        {
            fluidSourceMuds.sourcePackages.clear();
            fluidSourceMuds.sourcePackages.append(fluidSourceSyringes[activeSyringeIdx].sourcePackages.at(0));
            fluidSourceMuds.currentFluidKinds.clear();
            fluidSourceMuds.currentFluidKinds.append((activeSyringeIdx == SYRINGE_IDX_SALINE) ? "Flush" : "Contrast");

            if (mudsLineFluidSyringeIdx != activeSyringeIdx)
            {
                mudsLineFluidSyringeIdx = activeSyringeIdx;
                LOG_DEBUG("SetFluidSourceMudsStatus(): Syringe is injecting to ML, MudsLineFluidSyringeIdx=%s\n", ImrParser::ToImr_FluidSourceSyringeIdx(mudsLineFluidSyringeIdx).CSTR());
            }
        }
        else
        {
            if (fluidSourceMuds.isReady)
            {
                // No fluid package available. E.g. Purging air
                LOG_ERROR("SetFluidSourceMudsStatus(): Unable to find the SourcePackage information, ActiveSyringeIndex=%s\n", ImrParser::ToImr_FluidSourceSyringeIdx(activeSyringeIdx).CSTR());
            }

            if (mudsLineFluidSyringeIdx != SYRINGE_IDX_NONE)
            {
                mudsLineFluidSyringeIdx = SYRINGE_IDX_NONE;
                LOG_DEBUG("SetFluidSourceMudsStatus(): Syringe.SourcePackages=empty, MudsLineFluidSyringeIdx=%s\n", ImrParser::ToImr_FluidSourceSyringeIdx(mudsLineFluidSyringeIdx).CSTR());
            }
        }
    }
    else
    {
        if (fluidSourceMuds.sourcePackages.length() == 0)
        {
            if (mudsLineFluidSyringeIdx != SYRINGE_IDX_NONE)
            {
                mudsLineFluidSyringeIdx = SYRINGE_IDX_NONE;
                LOG_DEBUG("SetFluidSourceMudsStatus(): Muds.SourcePackages=empty, MudsLineFluidSyringeIdx=%s\n", ImrParser::ToImr_FluidSourceSyringeIdx(mudsLineFluidSyringeIdx).CSTR());
            }
        }
    }

    // Set NeedsReplaced field
    fluidSourceMuds.needsReplaced = fluidSourceSyringes[SYRINGE_IDX_SALINE].needsReplaced || fluidSourceSyringes[SYRINGE_IDX_CONTRAST1].needsReplaced || fluidSourceSyringes[SYRINGE_IDX_CONTRAST2].needsReplaced;

    env->ds.deviceData->setMudsLineFluidSyringeIndex(mudsLineFluidSyringeIdx);
    env->ds.deviceData->setFluidSourceMuds(fluidSourceMuds);
}

DataServiceActionStatus DeviceMuds::actMudsInit(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "MudsInit");
    DS_DeviceDef::FluidSource fluidSourceMuds = env->ds.deviceData->getFluidSourceMuds();
    DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
    DS_WorkflowDef::MudsSodStatus mudsSodStatus = env->ds.workflowData->getMudsSodStatus();

    fluidSourceMuds.installedAtEpochMs = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();

    if ( (env->ds.alertAction->isActivated("UsedMUDSDetected")) ||
         (env->ds.alertAction->isActivated("PlungerEngagementFault", "", true)) )
    {
        LOG_ERROR("actMudsInit(): ------------ Unexpected alerts activated (UsedMUDSDetected or PlungerEngagementFault) ------------\n");

        mudsSodStatus.init();
        env->ds.alertAction->activate("UsedMUDSDetected");
        status.err = "UNKNOWN_USED_MUDS";
        status.state = DS_ACTION_STATE_INTERNAL_ERR;
        actionStarted(status);
    }
    else if (isNewMudsDetected())
    {
        LOG_INFO("actMudsInit(): ------------ New MUDS Detected ------------\n");
        mudsSodStatus.init();

        status.reply = "NEW_MUDS";
        status.state = DS_ACTION_STATE_STARTED;
        actionStarted(status);
        status.state = DS_ACTION_STATE_COMPLETED;
        actionStarted(status);
    }
    else if (isLastUsedMudsDetected())
    {
        LOG_INFO("actMudsInit(): ------------ Last Used MUDS Detected ------------\n");

        status.reply = "USED_MUDS";

        DS_DeviceDef::FluidSources lastFluidSources = env->ds.deviceData->getLastFluidSources();

        LOG_INFO("actMudsInit(): Restoring SYRINGES information..\n");

        for (int syringeIdx = 0; syringeIdx < SYRINGE_IDX_MAX; syringeIdx++)
        {
            const DS_DeviceDef::FluidSource *lastFluidSourceSyringe = &lastFluidSources[DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_1 + syringeIdx];

            fluidSourceSyringes[syringeIdx].installedAtEpochMs = lastFluidSourceSyringe->installedAtEpochMs;
            fluidSourceSyringes[syringeIdx].currentFluidKinds.clear();
            fluidSourceSyringes[syringeIdx].currentFluidKinds.append((syringeIdx == SYRINGE_IDX_SALINE) ? "Flush" : "Contrast");
            fluidSourceSyringes[syringeIdx].sourcePackages = lastFluidSourceSyringe->sourcePackages;
            fluidSourceSyringes[syringeIdx].needsReplaced = lastFluidSourceSyringe->needsReplaced;
            fluidSourceSyringes[syringeIdx].isReady = lastFluidSourceSyringe->isReady;
            //fluidSourceSyringes[syringeIdx].currentVolumes = lastFluidSourceSyringe->currentVolumes;

            if (!mudsSodStatus.syringeSodStatusAll[syringeIdx].plungerEngaged)
            {
                LOG_WARNING("actMudsInit(): Used MUDS found but syringe[%d] is not engaged.\n", syringeIdx);
                status.reply = "NEW_MUDS";
            }
        }

        env->ds.deviceData->setFluidSourceSyringes(fluidSourceSyringes);

        LOG_INFO("actMudsInit(): Restoring BOTTLES information..\n");
        DS_DeviceDef::FluidSources fluidSourceBottles = env->ds.deviceData->getFluidSourceBottles();
        for (int syringeIdx = 0; syringeIdx < SYRINGE_IDX_MAX; syringeIdx++)
        {
            const DS_DeviceDef::FluidSource *lastFluidSourceBottle = &lastFluidSources[DS_DeviceDef::FLUID_SOURCE_IDX_BOTTLE_1 + syringeIdx];

            fluidSourceBottles[syringeIdx].installedAtEpochMs = lastFluidSourceBottle->installedAtEpochMs;
            fluidSourceBottles[syringeIdx].currentFluidKinds.clear();
            fluidSourceBottles[syringeIdx].currentFluidKinds.append((syringeIdx == SYRINGE_IDX_SALINE) ? "Flush" : "Contrast");

            fluidSourceBottles[syringeIdx].currentVolumes.clear();
            if (lastFluidSourceBottle->currentVolumes.length() > 0)
            {
                fluidSourceBottles[syringeIdx].currentVolumes.append(lastFluidSourceBottle->currentVolumes[0]);
            }
            else
            {
                LOG_WARNING("actMudsInit(): Unable to get currentVolume from last digest\n");
                fluidSourceBottles[syringeIdx].currentVolumes.append(0);
            }

            fluidSourceBottles[syringeIdx].sourcePackages = lastFluidSourceBottle->sourcePackages;
            fluidSourceBottles[syringeIdx].needsReplaced = lastFluidSourceBottle->needsReplaced;
            // .IsReady should not be updated as it represents the spike state which is updated in real time
        }
        env->ds.deviceData->setFluidSourceBottles(fluidSourceBottles);

        // Correct bottle states
        for (int syrIdx = 0; syrIdx < SYRINGE_IDX_MAX; syrIdx++)
        {
            SyringeIdx syringeIdx = (SyringeIdx)syrIdx;

            // Check if Syringe has no package filled before but bottle is loaded.
            // If so, unload bottle (it is expected that the bottle is never filled after loaded)
            if ( (fluidSourceBottles[syringeIdx].isInstalled()) &&
                 (fluidSourceBottles[syringeIdx].sourcePackages.length() > 0) &&
                 (fluidSourceSyringes[syringeIdx].sourcePackages.length() == 0) )
            {
                LOG_WARNING("actMudsInit(): Bottle[%d] is loaded but never filled. User needs to re-select. Unloading bottle.\n", syringeIdx);
                env->ds.deviceAction->actBottleUnload(syringeIdx);
            }
        }

        // Restore Selected Contrast
        DS_ExamDef::ExecutedSteps executedSteps = env->ds.examData->getExecutedSteps();
        if (executedSteps.length() > 0)
        {
            // SelectedContrast is not defined yet. Get it from last executed step
            DS_DeviceDef::FluidSourceIdx activeContrastLocation = executedSteps.last().activeContrastLocation;
            SyringeIdx activeContrastSyringeIdx = ImrParser::ToCpp_FluidSourceSyringeIdx(ImrParser::ToImr_FluidSourceIdx(activeContrastLocation));

            if (fluidSourceSyringes[activeContrastSyringeIdx].sourcePackages.length() > 0)
            {
                DS_DeviceDef::FluidPackage lastUsedFluidPackage = fluidSourceSyringes[activeContrastSyringeIdx].sourcePackages.first();
                LOG_INFO("actMudsInit(): Last Selected contrast is %s\n", Util::qVarientToJsonData(ImrParser::ToImr_FluidPackage(lastUsedFluidPackage), false).CSTR());
                env->ds.examAction->actReloadSelectedContrast(lastUsedFluidPackage.brand, lastUsedFluidPackage.concentration);
            }
            else
            {
                LOG_WARNING("actMudsInit(): Step is executed but cannot identify selected contrast\n");
            }
        }

        // Restore syringe volumes - Syringe volumes must have been adjusted with empty source packages
        handleSyringeVolsChanged();

        LOG_INFO("actMudsInit(): Restoring MUDS information..\n");
        const DS_DeviceDef::FluidSource *lastFluidSourceMuds = &lastFluidSources[DS_DeviceDef::FLUID_SOURCE_IDX_MUDS];
        fluidSourceMuds.installedAtEpochMs = lastFluidSourceMuds->installedAtEpochMs;
        fluidSourceMuds.currentFluidKinds = lastFluidSourceMuds->currentFluidKinds;
        fluidSourceMuds.sourcePackages = lastFluidSourceMuds->sourcePackages;
        fluidSourceMuds.isReady = lastFluidSourceMuds->isReady;

        status.state = DS_ACTION_STATE_STARTED;
        actionStarted(status);
        status.state = DS_ACTION_STATE_COMPLETED;
        actionStarted(status);
    }
    else
    {
        LOG_ERROR("actMudsInit(): ------------ Unknown Used MUDS inserted (vol=%.1f,%.1f,%.1fml) ------------\n",
                  fluidSourceSyringes[SYRINGE_IDX_SALINE].currentVolumesTotal(),
                  fluidSourceSyringes[SYRINGE_IDX_CONTRAST1].currentVolumesTotal(),
                  fluidSourceSyringes[SYRINGE_IDX_CONTRAST2].currentVolumesTotal());

        mudsSodStatus.init();
        env->ds.alertAction->activate("UsedMUDSDetected");
        status.err = "UNKNOWN_USED_MUDS";
        status.state = DS_ACTION_STATE_INTERNAL_ERR;
        actionStarted(status);
    }

    mudsSodStatus.identified = true;
    env->ds.workflowData->setMudsSodStatus(mudsSodStatus);
    env->ds.deviceData->setFluidSourceMuds(fluidSourceMuds);

    return status;
}

bool DeviceMuds::isNewMudsDetected()
{
    double newMudsVolOffset = env->ds.capabilities->get_FluidControl_NewMudsVolOffset();
    QList<double> curVols = env->ds.mcuData->getSyringeVols();
    bool isNewMuds = false;

    if ( (curVols[SYRINGE_IDX_SALINE] >= newMudsVolOffset) ||
         (curVols[SYRINGE_IDX_CONTRAST1] >= newMudsVolOffset) ||
         (curVols[SYRINGE_IDX_CONTRAST2] >= newMudsVolOffset) )
    {
        isNewMuds = true;
    }
    else
    {
        isNewMuds = false;
    }

    LOG_INFO("isNewMudsDetected(): vol=%.1f,%.1f,%.1fml, newMudsVolOffset=%.1fml, isNewMuds=%s\n",
             curVols[SYRINGE_IDX_SALINE],
             curVols[SYRINGE_IDX_CONTRAST1],
             curVols[SYRINGE_IDX_CONTRAST2],
             newMudsVolOffset,
             isNewMuds ? "TRUE" : "FALSE");

    return isNewMuds;
}

bool DeviceMuds::isLastUsedMudsDetected()
{
    DS_DeviceDef::FluidSources lastFluidSources = env->ds.deviceData->getLastFluidSources();
    DS_DeviceDef::FluidSource lastFluidSourceMuds = lastFluidSources[DS_DeviceDef::FLUID_SOURCE_IDX_MUDS];

    if (!lastFluidSourceMuds.isInstalled())
    {
        LOG_WARNING("isLastUsedMudsDetected(): Bad lastFluidSourceMuds. Data=%s\n", Util::qVarientToJsonData(ImrParser::ToImr_FluidSource(lastFluidSourceMuds)).CSTR());
        return false;
    }

    QList<double> lastSyringeVols = env->ds.mcuData->getLastSyringeVols();
    QList<double> curVols = env->ds.mcuData->getSyringeVols();

    double reusedMudsValidationOffsetMl = env->ds.capabilities->get_FluidControl_ReusedMudsValidationOffset();

    if ( (abs(lastSyringeVols[SYRINGE_IDX_SALINE] - curVols[SYRINGE_IDX_SALINE]) <= reusedMudsValidationOffsetMl) &&
         (abs(lastSyringeVols[SYRINGE_IDX_CONTRAST1] - curVols[SYRINGE_IDX_CONTRAST1]) <= reusedMudsValidationOffsetMl) &&
         (abs(lastSyringeVols[SYRINGE_IDX_CONTRAST2] - curVols[SYRINGE_IDX_CONTRAST2]) <= reusedMudsValidationOffsetMl) )
    {
        LOG_INFO("isLastUsedMudsDetected(): Current MUDS is expected to last installed one. %.1f->%.1f, %.1f->%.1f, %.1f->%.1f\n",
                 lastSyringeVols[SYRINGE_IDX_SALINE], curVols[SYRINGE_IDX_SALINE],
                 lastSyringeVols[SYRINGE_IDX_CONTRAST1], curVols[SYRINGE_IDX_CONTRAST1],
                 lastSyringeVols[SYRINGE_IDX_CONTRAST2], curVols[SYRINGE_IDX_CONTRAST2]);
    }
    else
    {
        LOG_WARNING("isLastUsedMudsDetected(): Volumes(ml) changed: AllowedRangeDiff=%.1fml, %.1f->%.1f, %.1f->%.1f, %.1f->%.1f\n",
                    reusedMudsValidationOffsetMl,
                    lastSyringeVols[SYRINGE_IDX_SALINE], curVols[SYRINGE_IDX_SALINE],
                    lastSyringeVols[SYRINGE_IDX_CONTRAST1], curVols[SYRINGE_IDX_CONTRAST1],
                    lastSyringeVols[SYRINGE_IDX_CONTRAST2], curVols[SYRINGE_IDX_CONTRAST2]);
        return false;
    }

    return true;
}

void DeviceMuds::slotUpdateUsedTimeStatus()
{
    tmrUseLifeUpdate.stop();
    DS_DeviceDef::FluidSource fluidSourceMuds = env->ds.deviceData->getFluidSourceMuds();

    if ( (!fluidSourceMuds.isInstalled()) ||
         (env->ds.alertAction->isActivated("MUDSEjectedRemovalRequired", "", true)) )
    {
        if ( (env->ds.alertAction->isActivated("MUDSUseLifeExceeded")) ||
             (env->ds.alertAction->isActivated("MUDSUseLifeLimitEnforced")) )
        {
            LOG_INFO("slotUpdateUsedTimeStatus(): MUDS NOT Installed OR UseLifeLimitEnforced=OFF OR Removel Waiting: Deactivating MUDSUseLifeExceeded/MUDSUseLifeLimitEnforced alerts..\n");
            env->ds.alertAction->deactivate("MUDSUseLifeExceeded");
            env->ds.alertAction->deactivate("MUDSUseLifeLimitEnforced");
        }
        return;
    }

    qint64 useLifeHour = env->ds.cfgGlobal->get_Configuration_UseLife_MUDSUseLifeLimitHours();
    qint64 useLifeMs = useLifeHour * 60 * 60 * 1000;
    qint64 useTimeMs = QDateTime::currentMSecsSinceEpoch() - fluidSourceMuds.installedAtEpochMs;
    qint64 useLifeRemainingMs = useLifeMs - useTimeMs;

    if (useLifeRemainingMs <= 0)
    {
        bool useLifeLimitEnforced = env->ds.cfgGlobal->get_Configuration_UseLife_MUDSUseLifeLimitEnforced();
        qint64 useLifeGraceHour = env->ds.cfgGlobal->get_Configuration_UseLife_MUDSUseLifeGraceHours();
        qint64 useLifeGraceRemainingMs = useLifeRemainingMs + (useLifeGraceHour * 60 * 60 * 1000);

        /*LOG_INFO("UsedTimeSecs=%lld, MudsUseLifeHours=%lld, UseLifeRemainingSecs=%lld, useLifeLimitEnforced=%s, MUDSUseLifeGraceHours=%lld, useLifeGraceRemainingSecs=%lld\n",
                 useTimeMs / 1000,
                 useLifeHour,
                 useLifeRemainingMs / 1000,
                 useLifeLimitEnforced ? "TRUE" : "FALSE",
                 useLifeGraceHour,
                 useLifeGraceRemainingMs / 1000);*/

        // MUDS Use Life Exceeded

        if (!env->ds.alertAction->isActivated("MUDSUseLifeExceeded"))
        {
            LOG_WARNING("slotUpdateUsedTimeStatus(): UseLifeExceeded: UsedTimeHours(%lld) > LimitHours(%lld)\n", useTimeMs / (60 * 60 * 1000), useLifeHour);
            env->ds.alertAction->activate("MUDSUseLifeExceeded");
        }

        if ( (useLifeLimitEnforced) &&
             (useLifeGraceRemainingMs <= 0) )
        {
            // MUDS UseLifeGraceHours is expired
            //LOG_INFO("MUDSUseLifeExceeded=TRUE, MUDSUseLifeLimitEnforced=TRUE\n");

            if (!env->ds.alertAction->isActivated("MUDSUseLifeLimitEnforced"))
            {
                LOG_WARNING("slotUpdateUsedTimeStatus(): (UseLifeExceeded AND Enforced): UsedTimeHours(%lld) > (LimitHours(%lld) + GraceHours(%lld))\n",
                             useTimeMs / (60 * 60 * 1000), useLifeHour, useLifeGraceHour);

               env->ds.alertAction->activate("MUDSUseLifeLimitEnforced");
            }
        }
        else
        {
            // MUDS UseLifeGraceHours is not expired
            if (env->ds.alertAction->isActivated("MUDSUseLifeLimitEnforced"))
            {
                LOG_INFO("slotUpdateUsedTimeStatus(): UseLifeExceeded AND NotEnforced, UsedLifeGraceRemainingSec(%lldsec)\n", useLifeGraceRemainingMs / 1000);
                env->ds.alertAction->deactivate("MUDSUseLifeLimitEnforced");
            }

            tmrUseLifeUpdate.start(FLUID_SOURCE_USE_LIFE_CHECK_INTERVAL_MS);
        }
    }
    else
    {
        // MUDS Use Life is not Exceeded
        if ( (env->ds.alertAction->isActivated("MUDSUseLifeExceeded")) ||
             (env->ds.alertAction->isActivated("MUDSUseLifeLimitEnforced")) )
        {
            LOG_INFO("slotUpdateUsedTimeStatus(): NotUseLifeExceeded AND NotEnforced\n");
            env->ds.alertAction->deactivate("MUDSUseLifeExceeded");
            env->ds.alertAction->deactivate("MUDSUseLifeLimitEnforced");
        }

        // Wait until UseLifeExceeded
        //LOG_DEBUG("MUDSUseLifeExceeded=FALSE, MUDSUseLifeLimitEnforced=FALSE, UsedLifeRemainingSec(%lldsec)\n", useLifeRemainingMs / 1000);
        tmrUseLifeUpdate.start(FLUID_SOURCE_USE_LIFE_CHECK_INTERVAL_MS);
    }
}

DataServiceActionStatus DeviceMuds::actMudsPurgeAirAll(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "MudsPurgeAirAll");

    QString guid = Util::newGuid();

    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            env->ds.alertAction->activate("SRUPurgingFactoryMUDS");

            // Set SUDS LED red during air-purge
            DS_McuDef::ActLedParams params;
            params.setColorRed();
            params.isFlashing = true;
            env->ds.deviceAction->actLeds(LED_IDX_SUDS1, params);

            // Set the syringes isBusy to true
            DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
            fluidSourceSyringes[SYRINGE_IDX_SALINE].isBusy = true;
            fluidSourceSyringes[SYRINGE_IDX_CONTRAST1].isBusy = true;
            fluidSourceSyringes[SYRINGE_IDX_CONTRAST2].isBusy = true;
            env->ds.deviceData->setFluidSourceSyringes(fluidSourceSyringes);

            env->actionMgr->onActionCompleted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
                if (curStatus.state == DS_ACTION_STATE_ALLSTOP_BTN_ABORT)
                {
                    env->ds.alertAction->activate("InterruptedAllStopPressed", "PurgeAir;RS0;RC1;RC2");
                }

                env->ds.alertAction->deactivate("SRUPurgingFactoryMUDS");

                // Set the syringes isBusy to false
                DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
                fluidSourceSyringes[SYRINGE_IDX_SALINE].isBusy = false;
                fluidSourceSyringes[SYRINGE_IDX_CONTRAST1].isBusy = false;
                fluidSourceSyringes[SYRINGE_IDX_CONTRAST2].isBusy = false;

                env->ds.deviceData->setFluidSourceSyringes(fluidSourceSyringes);

                actionCompleted(curStatus, &status);
            });
        }
        actionStarted(curStatus, &status);
    });

    env->ds.mcuAction->actPurgeAll(guid);

    status.state = DS_ACTION_STATE_START_WAITING;
    return status;
}

DataServiceActionStatus DeviceMuds::actMudsFindPlungerAll(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "MudsFindPlungerAll");
    status.state = DS_ACTION_STATE_START_WAITING;

    QString guid = Util::newGuid();

    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            // Set the syringes isBusy to true
            DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
            fluidSourceSyringes[SYRINGE_IDX_SALINE].isBusy = true;
            fluidSourceSyringes[SYRINGE_IDX_CONTRAST1].isBusy = true;
            fluidSourceSyringes[SYRINGE_IDX_CONTRAST2].isBusy = true;

            env->ds.deviceData->setFluidSourceSyringes(fluidSourceSyringes);



            env->actionMgr->onActionCompleted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
                // Set the syringes isBusy to false
                DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
                fluidSourceSyringes[SYRINGE_IDX_SALINE].isBusy = false;
                fluidSourceSyringes[SYRINGE_IDX_CONTRAST1].isBusy = false;
                fluidSourceSyringes[SYRINGE_IDX_CONTRAST2].isBusy = false;

                env->ds.deviceData->setFluidSourceSyringes(fluidSourceSyringes);



                actionCompleted(curStatus, &status);
            });
        }
        actionStarted(curStatus, &status);

    });

    env->ds.mcuAction->actFindPlungerAll(guid);

    return status;
}

DataServiceActionStatus DeviceMuds::actMudsPurgeFluid(QList<bool> purgeSyringe, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "MudsPurgeFluid");
    status.state = DS_ACTION_STATE_START_WAITING;

    QString guid = Util::newGuid();

    // Save syringe volumes before purge start
    DS_DeviceDef::FluidSources lastFluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();

    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            // Set the syringes isBusy to true
            DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
            for (int syrIdx = 0; syrIdx < lastFluidSourceSyringes.length(); syrIdx++)
            {
                SyringeIdx syringeIdx = (SyringeIdx)syrIdx;
                if ( (lastFluidSourceSyringes[syringeIdx].currentVolumesTotal() > 0) && (purgeSyringe[syrIdx]) )
                {
                    env->ds.alertAction->activate("SRUPurgingFluid", ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdx));
                    fluidSourceSyringes[syringeIdx].isBusy = true;
                }
            }

            env->ds.deviceData->setFluidSourceSyringes(fluidSourceSyringes);

            // Stop purge if SUDS is removed
            QString sudsInsertMonitorGuid = Util::newGuid();
            QMetaObject::Connection conn = connect(env->ds.mcuData, &DS_McuData::signalDataChanged_SudsInserted, this, [=](bool sudsInserted) {
                if (!sudsInserted)
                {
                    env->ds.mcuAction->actStopAll();
                    DataServiceActionStatus newStatus;
                    newStatus.state = DS_ACTION_STATE_INVALID_STATE;
                    newStatus.err = "SUDS_REMOVED";
                    actionCompleted(newStatus, &status);
                }
            });
            env->actionMgr->createActCompleted(sudsInsertMonitorGuid, conn, QString(__PRETTY_FUNCTION__) + ": actMudsPurgeFluid(): SudsInserted");

            env->actionMgr->onActionCompleted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
                env->actionMgr->deleteActCompleted(sudsInsertMonitorGuid);

                // Set the syringes isBusy to false
                DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();

                for (int syrIdx = 0; syrIdx < lastFluidSourceSyringes.length(); syrIdx++)
                {
                    SyringeIdx syringeIdx = (SyringeIdx)syrIdx;
                    if ( (lastFluidSourceSyringes[syringeIdx].currentVolumesTotal() > 0) && (purgeSyringe[syrIdx]) )
                    {
                        env->ds.alertAction->deactivate("SRUPurgingFluid", ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdx));
                        fluidSourceSyringes[syringeIdx].isBusy = false;
                    }
                }

                env->ds.deviceData->setFluidSourceSyringes(fluidSourceSyringes);

                for (int syrIdx = 0; syrIdx < SYRINGE_IDX_MAX; syrIdx++)
                {
                    SyringeIdx syringeIdx = (SyringeIdx)syrIdx;

                    // AlertData format is { <Location>: <FluidSource> }
                    double purgedVolume = lastFluidSourceSyringes[syringeIdx].currentVolumesTotal() - fluidSourceSyringes[syringeIdx].currentVolumesTotal();
                    if ( (purgedVolume > 0) && (purgeSyringe[syrIdx]) )
                    {
                        QVariantMap jsonMap;
                        QVariantMap jsonMapFluidSource = ImrParser::ToImr_FluidSource(fluidSourceSyringes[syringeIdx]);
                        jsonMapFluidSource.insert("PurgedVolume", purgedVolume);
                        jsonMap.insert(ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdx), jsonMapFluidSource);
                        QString alertData = Util::qVarientToJsonData(jsonMap);
                        env->ds.alertAction->activate("SRUPurgedFluid", alertData);
                    }
                }

                if (curStatus.state == DS_ACTION_STATE_ALLSTOP_BTN_ABORT)
                {
                    env->ds.alertAction->activate("InterruptedAllStopPressed", "PurgeFluid;RS0;RC1;RC2");
                }

                DataServiceActionStatus newStatus = curStatus;
                actionCompleted(newStatus, &status);
            });
        }
        actionStarted(curStatus, &status);
    });

    // Piston command only supports singal index, contrasts or all as syringe index input
    int calcIndex = 0;
    DS_McuDef::ActPistonParams pistonParams;
    pistonParams.flow = SYRINGE_FLOW_PURGE_FLUID;
    pistonParams.vol = SYRINGE_VOLUME_FILL_ALL;
    if (purgeSyringe[0])
    {
        calcIndex += 4;
    }
    if (purgeSyringe[1])
    {
        calcIndex += 2;
    }
    if (purgeSyringe[2])
    {
        calcIndex += 1;
    }

    switch (calcIndex) {
    case 4: // 100
        pistonParams.idx=SYRINGE_IDX_SALINE;
        break;
    case 2: // 010
        pistonParams.idx=SYRINGE_IDX_CONTRAST1;
        break;
    case 1: // 001
        pistonParams.idx=SYRINGE_IDX_CONTRAST2;
        break;
    case 3: // 011
        pistonParams.arg = "contrasts";
        [[fallthrough]];
    default: // 111 and other cases
        env->ds.mcuAction->actPistonAll(pistonParams, guid);
        return status;
    }

    env->ds.mcuAction->actPiston(pistonParams, guid);
    return status;
}

DataServiceActionStatus DeviceMuds::actMudsEngageAll(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "MudsEngageAll");

    QString guid = Util::newGuid();

    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);

        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            env->actionMgr->onActionCompleted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
                // Update mudsSodStatus
                DS_WorkflowDef::MudsSodStatus mudsSodStatus = env->ds.workflowData->getMudsSodStatus();
                DS_McuDef::PlungerStates plungerStates = env->ds.mcuData->getPlungerStates();

                curStatus.state = DS_ACTION_STATE_COMPLETED;
                for (int syringeIdx = 0; syringeIdx < SYRINGE_IDX_MAX; syringeIdx++)
                {
                    if (plungerStates[syringeIdx] == DS_McuDef::PLUNGER_STATE_ENGAGED)
                    {
                        mudsSodStatus.syringeSodStatusAll[syringeIdx].plungerEngaged = true;
                    }
                    else
                    {
                        LOG_ERROR("MudsEngageAll: Failed to engage, syringeIdx=%d, plungerState=%s\n", syringeIdx, ImrParser::ToImr_PlungerState(plungerStates[syringeIdx]).CSTR());
                        curStatus.state = DS_ACTION_STATE_INTERNAL_ERR;
                        if (curStatus.err.length() > 0)
                        {
                            curStatus.err += ", ";
                        }
                        curStatus.err += QString().asprintf("%d:%s", syringeIdx, ImrParser::ToImr_PlungerState(plungerStates[syringeIdx]).CSTR());
                        mudsSodStatus.syringeSodStatusAll[syringeIdx].plungerEngaged = false;
                    }

                }

                env->ds.workflowData->setMudsSodStatus(mudsSodStatus);
                actionCompleted(curStatus, &status);
            });
        }
    });

    return env->ds.mcuAction->actEngageAll(guid);
}

DataServiceActionStatus DeviceMuds::actMudsDisengageAll(int retryLimit, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "MudsDisengage");

    QString guid = Util::newGuid();

    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);

        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            // Set the syringes isBusy to true
            DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
            fluidSourceSyringes[SYRINGE_IDX_SALINE].isBusy = true;
            fluidSourceSyringes[SYRINGE_IDX_CONTRAST1].isBusy = true;
            fluidSourceSyringes[SYRINGE_IDX_CONTRAST2].isBusy = true;

            env->ds.deviceData->setFluidSourceSyringes(fluidSourceSyringes);

            env->actionMgr->onActionCompleted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
                // Set the syringes isBusy to false
                DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
                fluidSourceSyringes[SYRINGE_IDX_SALINE].isBusy = false;
                fluidSourceSyringes[SYRINGE_IDX_CONTRAST1].isBusy = false;
                fluidSourceSyringes[SYRINGE_IDX_CONTRAST2].isBusy = false;
                env->ds.deviceData->setFluidSourceSyringes(fluidSourceSyringes);

                // Reset last syringe vols: Prevent reuse the ejected MUDS
                //QList<double> lastSyringeVols = env->ds.mcuData->getSyringeVols();
                //LOG_INFO("actMudsDisengageAll(): Setting last syringe vols=%s\n", Util::qVarientToJsonData(ImrParser::ToImr_DoubleList(lastSyringeVols)).CSTR());
                //env->ds.mcuData->setLastSyringeVols(lastSyringeVols);

                actionCompleted(curStatus, &status);
            });
        }
    });

    return deviceMudsDisengage->actDisengageAll(retryLimit, guid);
}

DataServiceActionStatus DeviceMuds::actMudsPreload(const DS_McuDef::InjectionProtocol &injectionProtocol, QString actGuid)
{
    return deviceMudsPreload->actMudsPreload(injectionProtocol, actGuid);
}

DataServiceActionStatus DeviceMuds::actMudsSodStart(QString actGuid)
{
    return deviceMudsSod->actSodStart(actGuid);
}

DataServiceActionStatus DeviceMuds::actMudsSodAbort(QString actGuid)
{
    return deviceMudsSod->actSodAbort(actGuid);
}

void DeviceMuds::updateContrastBottleCountReachedAlert(int bottleCount)
{
    int bottleFillLimit = env->ds.capabilities->get_Alert_ContrastBottleCountReached_Limit();
    if (bottleCount >= bottleFillLimit)
    {
        env->ds.alertAction->activate("ContrastBottleCountReached");
    }
    else
    {
        env->ds.alertAction->deactivate("ContrastBottleCountReached");
    }
}

void DeviceMuds::configureContrastBottleCountReachedAlert()
{
    connect(env->ds.cfgLocal, &DS_CfgLocal::signalConfigChanged_Hidden_BottleFillCount, this, [=](const Config::Item &cfg) {
        if (env->ds.cfgGlobal->get_Configuration_Behaviors_Country() == "United States")
        {
            updateContrastBottleCountReachedAlert(cfg.value.toInt());
        }
    });

    connect(env->ds.cfgGlobal, &DS_CfgGlobal::signalConfigChanged_Configuration_Behaviors_Country, this, [=](const Config::Item &cfg, const Config::Item &prevCfg) {
        if (cfg.value.toString() != prevCfg.value.toString())
        {
            // bottle count alert is only for United States
            if (cfg.value.toString() == "United States")
            {
                int bottleFillCount = env->ds.cfgLocal->get_Hidden_BottleFillCount();
                updateContrastBottleCountReachedAlert(bottleFillCount);
            }
            else
            {
                env->ds.alertAction->deactivate("ContrastBottleCountReached");
            }
        }
    });
}
