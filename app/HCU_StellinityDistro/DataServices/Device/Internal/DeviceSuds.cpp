#include "Apps/AppManager.h"
#include "Common/Util.h"
#include "Common/ImrParser.h"
#include "DeviceSuds.h"
#include "DataServices/Workflow/DS_WorkflowData.h"
#include "DataServices/Device/DS_DeviceData.h"
#include "DataServices/Device/DS_DeviceAction.h"
#include "DataServices/Alert/DS_AlertAction.h"
#include "DataServices/Alert/DS_AlertData.h"
#include "DataServices/Exam/DS_ExamData.h"
#include "DataServices/Mcu/DS_McuData.h"
#include "DataServices/Mcu/DS_McuAction.h"
#include "DataServices/System/DS_SystemData.h"
#include "DataServices/Capabilities/DS_Capabilities.h"
#include "DataServices/Exam/DS_ExamAction.h"

DeviceSuds::DeviceSuds(QObject *parent, EnvGlobal *env_) :
    ActionBaseExt(parent, env_)
{
    envLocal = new EnvLocal("DS_Device-Suds", "DEVICE_SUDS", LOG_MID_SIZE_BYTES);

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });

    connect(env->appManager, &AppManager::signalAppStarted, this, [=]() {
        slotAppStarted();
    });

    state = STATE_INACTIVE;
    activeContrastSyringe = SYRINGE_IDX_NONE;

    processState();
}

DeviceSuds::~DeviceSuds()
{
    delete envLocal;
}

void DeviceSuds::slotAppStarted()
{
    setState(STATE_INIT);
}

void DeviceSuds::slotAppInitialised()
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
            state = STATE_INIT;

            bool sudsInserted = env->ds.mcuData->getSudsInserted();
            updateSUDSInsertedAlerts(sudsInserted, sudsInserted);
            updateFluidSource();
            updateStateFromSudsInsertedChanged();
        }
        else if ( (workflowState == DS_WorkflowDef::STATE_INACTIVE) &&
                  (workflowStatePrev != DS_WorkflowDef::STATE_INACTIVE) )
        {
            state = STATE_INACTIVE;
        }
    });

    // Register SUDS state data
    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_SudsInserted, this, [=](bool inserted, bool prevInserted) {
        updateSUDSInsertedAlerts(inserted, prevInserted);
        updateFluidSource();
        updateStateFromSudsInsertedChanged();
        updateSudsPortExposedState();
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_MudsInserted, this, [=](bool mudsInserted) {
        if (state == STATE_INACTIVE)
        {
            return;
        }

        if (mudsInserted)
        {
            if (state == STATE_MISSING)
            {
                // Starts the flashing behaviour
                setState(STATE_MISSING);
            }
        }
        else
        {
            setState(STATE_MISSING);
        }
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_SudsBubbleDetected, this, [=](bool isSudsBubbleDetected){
        if (state == STATE_INACTIVE)
        {
            return;
        }

        if ( (state == STATE_PRIMING) &&
             (isSudsBubbleDetected) )
        {
            resetPrimeParameters(listPrimedFluids[0].syringeIdx, listPrimedFluids[0].fluidType);
        }
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_StopcockPosAll, this, [=]() {
        if (state == STATE_INACTIVE)
        {
            return;
        }

        updateFluidSourceWhileInjecting();

        if (state == STATE_MISSING || state == STATE_NOT_PRIMED || state == STATE_NEEDS_REPLACE)
        {
            // LEDs for these states are handled in processState
            return;
        }

        updateSudsLed(isFluidFlowingThroughSuds());
    });

    connect(env->ds.deviceData, &DS_DeviceData::signalDataChanged_MudsLineFluidSyringeIndex, this, [=]() {
        if (state == STATE_INACTIVE)
        {
            return;
        }

        updateFluidSourceWhileInjecting();
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_InjectorStatus, this, [=]() {
        if (state == STATE_INACTIVE)
        {
            return;
        }

        updateFluidSourceWhileInjecting();
    });

    // Register Priming callback
    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_SyringeStates, this, [=](const DS_McuDef::SyringeStates &curStates, const DS_McuDef::SyringeStates &prevStates) {
        if (state == STATE_INACTIVE)
        {
            return;
        }

        if (!env->ds.mcuData->getSudsInserted())
        {
            return;
        }

        updatePrimeProgressFromSyringeStateChanged(curStates, prevStates);
    });

    connect(env->ds.deviceData, &DS_DeviceData::signalDataChanged_FluidSourceSyringes, this, [=](const DS_DeviceDef::FluidSources &fluidSourceSyringes, const DS_DeviceDef::FluidSources &prevFluidSourceSyringes){
        if (state == STATE_INACTIVE)
        {
            return;
        }

        DS_McuDef::InjectorStatus mcuProgress = env->ds.mcuData->getInjectorStatus();
        if (mcuProgress.state != DS_McuDef::INJECTOR_STATE_IDLE)
        {
            // Injection is in progress. Let other handler to handle injection status.
            return;
        }

        if (env->ds.mcuData->getSudsInserted())
        {
            // Handle priming state
            SyringeIdx primeSyringeIdx = getPrimingSyringeIdxFromStopcockPos();

            //LOG_DEBUG("SignalDataChanged_FluidSourceSyringes(): primeSyringeIdx=%d\n", syringeIdx);

            if (primeSyringeIdx != SYRINGE_IDX_NONE)
            {
                if (fluidSourceSyringes[primeSyringeIdx].currentVolumesTotal() < prevFluidSourceSyringes[primeSyringeIdx].currentVolumesTotal())
                {
                    updatePrimeProgress(primeSyringeIdx);
                }
            }
        }

        if (fluidSourceSyringes[SYRINGE_IDX_SALINE].sourcePackages.length() != prevFluidSourceSyringes[SYRINGE_IDX_SALINE].sourcePackages.length())
        {
            // Handle missing state
            updateLedWhileMissing();
        }
    });

    connect(&tmrSudsPortExposedTimeout, &QTimer::timeout, this, [=] {
        tmrSudsPortExposedTimeout.stop();

        if ( (!env->ds.deviceData->getFluidSourceMuds().isReady) ||
             (env->ds.mcuData->getDoorState() == DS_McuDef::DOOR_OPEN) )
        {
            // keep the timer rolling but don't raise alert if it shouldn't be raised
            int timeoutSecs = tmrSudsPortExposedTimeout.interval();
            LOG_WARNING("tmrSudsPortExposedTimeout(): Timer expired but MUDS is not ready. Restart the timer with last interval=%dms\n", timeoutSecs);
            tmrSudsPortExposedTimeout.start(timeoutSecs);
            return;
        }

        if (env->ds.alertAction->isActivated("SUDSPortExposed", "", true))
        {
            // SUDS Port Exposed state is already notified to the user
            if (!env->ds.alertAction->isActivated("SUDSPortExposedNotifyUser"))
            {
                // User closed the popup moment ago. Needs to notify the user again
                int timeoutSecs = env->ds.capabilities->get_Alert_SUDSPortExposedAfterUserAckLimitSeconds();
                LOG_WARNING("tmrSudsPortExposedTimeout(): SUDS Port Exposed for %ds after last user ack\n", timeoutSecs);
                env->ds.alertAction->activate("SUDSPortExposedNotifyUser");
            }
            else
            {
                // This shouldn't happen
                QString err = "tmrSudsPortExposedTimeout(): Unexpected tmr operation: both SUDSPortExposed and SUDSPortExposedNotifyUser alerts active";
                LOG_ERROR("%s", err.CSTR());
                env->ds.alertAction->activate("HCUSoftwareError", err);
            }
        }
        else
        {
            // First time to raise SUDSPortExposed, SUDSPortExposedNotifyUser alerts
            int timeoutSecs = env->ds.capabilities->get_Alert_SUDSPortExposedLimitSeconds();
            LOG_WARNING("tmrSudsPortExposedTimeout(): SUDS Port Exposed for %ds\n", timeoutSecs);

            QString alertData = QString().asprintf("%d", timeoutSecs);
            env->ds.alertAction->activate("SUDSPortExposed", alertData);

            // Notify to user
            env->ds.alertAction->activate("SUDSPortExposedNotifyUser");
        }
    });

    connect(env->ds.alertData, &DS_AlertData::signalDataChanged_ActiveAlerts, this, [=](const QVariantList &activeAlerts, const QVariantList &prevActiveAlerts) {
        bool prevAlertSUDSPortExposedActive = env->ds.alertAction->isActivated(prevActiveAlerts, "SUDSPortExposed", "", true);
        bool curAlertSUDSPortExposedActive = env->ds.alertAction->isActivated("SUDSPortExposed", "", true);

        bool prevAlertSUDSPortExposedNotifyUserActive = env->ds.alertAction->isActivated(prevActiveAlerts, "SUDSPortExposedNotifyUser", "", true);
        bool curAlertSUDSPortExposedNotifyUserActive = env->ds.alertAction->isActivated("SUDSPortExposedNotifyUser", "", true);

        bool prevAlertEnsureNoPatientConnectedActive = env->ds.alertAction->isActivated(prevActiveAlerts, "EnsureNoPatientConnected", "", true);
        bool curAlertEnsureNoPatientConnectedActive = env->ds.alertAction->isActivated("EnsureNoPatientConnected", "", true);

        if (curAlertSUDSPortExposedActive)
        {
            if ( (prevAlertSUDSPortExposedNotifyUserActive) &&
                 (!curAlertSUDSPortExposedNotifyUserActive) )
            {
                int timeoutSecs = env->ds.capabilities->get_Alert_SUDSPortExposedAfterUserAckLimitSeconds();
                LOG_INFO("signalDataChanged_ActiveAlerts(): SUDSPortExposed alert ACTIVE but warning popup closed by the user. Will notify again in %ds..\n", timeoutSecs);

                tmrSudsPortExposedTimeout.stop();
                tmrSudsPortExposedTimeout.start(timeoutSecs * 1000);
            }
        }

        if (curAlertSUDSPortExposedNotifyUserActive)
        {
            if ( (prevAlertSUDSPortExposedActive) &&
                 (!curAlertSUDSPortExposedActive) )
            {
                LOG_INFO("signalDataChanged_ActiveAlerts(): SUDSPortExposed alert INACTIVE while warning popup open. Deactivating SUDSPortExposedNotifyUser alert\n");
                tmrSudsPortExposedTimeout.stop();

                QTimer::singleShot(1, [=] {
                    // Makesure all signalDataChanged_XXX() are handled before change the Data.
                    if (env->ds.alertAction->isActivated("SUDSPortExposedNotifyUser", "", true))
                    {
                        env->ds.alertAction->deactivate("SUDSPortExposedNotifyUser");
                    }
                });
            }
        }

        if (curAlertEnsureNoPatientConnectedActive && !prevAlertEnsureNoPatientConnectedActive)
        {
               DS_McuDef::ActLedParams params;
               params.setColorRed();
               params.isFlashing = true;
               env->ds.deviceAction->actLeds(LED_IDX_SUDS1, params);

               DS_DeviceDef::FluidSource fluidSourceSuds = env->ds.deviceData->getFluidSourceSuds();
               fluidSourceSuds.isReady = false;
               fluidSourceSuds.isBusy = false;
               env->ds.deviceData->setFluidSourceSuds(fluidSourceSuds);

        }
        else if (prevAlertEnsureNoPatientConnectedActive && !curAlertEnsureNoPatientConnectedActive)
        {
            bool isSudsInserted = env->ds.mcuData->getSudsInserted();
            if (!isSudsInserted)
            {
                updateLedWhileMissing();
            }
        }


    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_SyringeVols, this, [=](const QList<double> &vols, const QList<double> &prevVols) {
        if (isFluidFlowingThroughSuds())
        {
            updateSudsContent(vols, prevVols);
        }
    });

    updateSudsPortExposedState();
}

QString DeviceSuds::getSudsContent()
{
    QString ret = "";

    const DS_DeviceDef::FluidSource fluidSourceSuds = env->ds.deviceData->getFluidSourceSuds();

    for (int i = 0; i < fluidSourceSuds.currentFluidKinds.length(); i++)
    {
        ret = fluidSourceSuds.currentFluidKinds[i];
        if (ret == "Contrast")
        {
            return ret;
        }
    }

    return ret;
}

// returns if SUDs should set needs replace
// Basically, SUDS should be marked as USED and it should have had firstAutoPrimeCompleted
//    this is very complicated, however this was added as later requirement
// Also refer to comments in WorkflowMain::freeAutoPrime
// IsSudsUsed will be set to true when injection occurs or preload occurs
// IsSudsUsed will be set to false only when "freeAutoPrime" completes
// isFirstAutoPrimeCompleted will be set to true when syringe prime completes and should remain true during exam (unless it's freeAutoPrime case)
// isFirstAutoPrimeCompleted will be set to false when freeAutoPrime starts until freeAutoPrime's syringe prime completes
bool DeviceSuds::isSudsNeedsReplace()
{
    return (env->ds.deviceData->getIsSudsUsed() && env->ds.deviceData->getIsFirstAutoPrimeCompleted());
}

bool DeviceSuds::isFluidFlowingThroughSuds()
{
    // Return if no Suds installed
    bool sudsInserted = env->ds.mcuData->getSudsInserted();
    if (!sudsInserted)
    {
        return false;
    }

    bool flowingThroughSuds = false;
    const DS_McuDef::StopcockPosAll stopcockPosAll = env->ds.mcuData->getStopcockPosAll();
    DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
    for (int scIdx = 0; scIdx < stopcockPosAll.length(); scIdx++)
    {
        if (stopcockPosAll[scIdx] == DS_McuDef::STOPCOCK_POS_MOVING)
        {
            // Stopcock is still moving.
            return false;
        }
        else
        {
            // even if it's in inject position, if there are no sourcePackages in syringe, it's not injecting from it
            flowingThroughSuds |= ((fluidSourceSyringes[(SyringeIdx)scIdx].sourcePackages.length() != 0) && (stopcockPosAll[scIdx] == DS_McuDef::STOPCOCK_POS_INJECT));
        }
    }

    if (!flowingThroughSuds)
    {
        // nothing is flowing through suds
        return false;
    }

    return true;
}

double DeviceSuds::getPatientLineVolume()
{
    bool extendedSUDS = (env->ds.examData->getSUDSLength() == SUDS_LENGTH_EXTENDED);
    return (extendedSUDS ? env->ds.capabilities->get_FluidControl_PatientLineVolumeExtended()
                         : env->ds.capabilities->get_FluidControl_PatientLineVolume());
}

void DeviceSuds::updateSudsContent(const QList<double> &vols, const QList<double> &prevVols)
{
    QString curFluidKind = "";
    double totalInjectedVol = 0.0;
    const DS_McuDef::StopcockPosAll stopcockPosAll = env->ds.mcuData->getStopcockPosAll();
    for (int scIdx = 0; scIdx < stopcockPosAll.length(); scIdx++)
    {
        if (stopcockPosAll[scIdx] == DS_McuDef::STOPCOCK_POS_INJECT)
        {
            if ((scIdx == SYRINGE_IDX_CONTRAST1) || (scIdx == SYRINGE_IDX_CONTRAST2))
            {
                curFluidKind = "Contrast";
            }
            // if it is aready Contrast, don't overwrite with Flush
            // check is not needed but it's here as safety check in case other definitions change
            else if (curFluidKind != "Contrast")
            {
                curFluidKind = "Flush";
            }
        }
        totalInjectedVol += qMax(0.0, (prevVols[scIdx] - vols[scIdx]));
    }

    if (curFluidKind == "")
    {
        LOG_ERROR("updateSudsContent(): Fluid is flowing through SUDS but curFluidKind is nothing!\n");
    }

    DS_DeviceDef::FluidSource fluidSourceSuds = env->ds.deviceData->getFluidSourceSuds();
    if ((fluidSourceSuds.currentFluidKinds.length() == 0) || (fluidSourceSuds.currentFluidKinds.last() != curFluidKind))
    {
        fluidSourceSuds.currentFluidKinds.append(curFluidKind.CSTR());
        fluidSourceSuds.currentVolumes.append(0.0);
    }

    fluidSourceSuds.currentVolumes.last() += totalInjectedVol;

    double patientLineCapacity = getPatientLineVolume();
    double volumesTotal = fluidSourceSuds.currentVolumesTotal();
    if (volumesTotal > patientLineCapacity)
    {
        fluidSourceSuds.currentVolumes.first() -= (volumesTotal - patientLineCapacity);
        if (fluidSourceSuds.currentVolumes.first() <= 0.0)
        {
            fluidSourceSuds.currentVolumes.removeAt(0);
            fluidSourceSuds.currentFluidKinds.removeAt(0);
        }
    }

    if (fluidSourceSuds.currentVolumes.first() == 0.0)
    {
        fluidSourceSuds.currentVolumes.removeAt(0);
        fluidSourceSuds.currentFluidKinds.removeAt(0);
    }

    env->ds.deviceData->setFluidSourceSuds(fluidSourceSuds);

    QString log = "updateSudsContent(): Flowing through suds - [";
    for (int scIdx = 0; scIdx < stopcockPosAll.length(); scIdx++)
    {
        double flowingVolume = qMax(0.0, (prevVols[scIdx] - vols[scIdx]));
        log.append(QString().asprintf(" %f%s", ((stopcockPosAll[scIdx] == DS_McuDef::STOPCOCK_POS_INJECT) ? flowingVolume : 0.0f), ((scIdx == stopcockPosAll.length() - 1) ? "" : ",")));
    }
    log.append(QString().asprintf(" ]"));
    LOG_DEBUG("%s\n", log.CSTR());

    log = "updateSudsContent(): currentFluidKinds , currentVolumes = [";
    for (int i = 0; i < fluidSourceSuds.currentFluidKinds.length(); i++)
    {
        log.append(QString().asprintf(" %s%s", fluidSourceSuds.currentFluidKinds[i].CSTR(), ((i==fluidSourceSuds.currentFluidKinds.length() - 1) ? "" : ",")));
    }
    log.append(QString().asprintf(" ], ["));
    for (int i = 0; i < fluidSourceSuds.currentVolumes.length(); i++)
    {
        log.append(QString().asprintf(" %f%s", fluidSourceSuds.currentVolumes[i], ((i==fluidSourceSuds.currentVolumes.length() - 1) ? "" : ",")));
    }
    log.append(QString().asprintf(" ]"));
    LOG_DEBUG("%s\n", log.CSTR());
}

void DeviceSuds::updateSUDSInsertedAlerts(bool inserted, bool prevInserted)
{
    LOG_INFO("updateSUDSInsertedAlerts(): SUDS state changed from %s to %s\n", prevInserted ? "INSERTED" : "REMOVED", inserted ? "INSERTED" : "REMOVED");

    if (inserted != prevInserted)
    {
        if (inserted)
        {
            env->ds.alertAction->activate("SRUInsertedSUDS");
        }
        else
        {
            env->ds.alertAction->activate("SRURemovedSUDS");
        }
    }

    if (!inserted)
    {
        if (state != STATE_INACTIVE)
        {
            if (env->ds.alertAction->isActivated("SUDSChangeRequired"))
            {
                env->ds.alertAction->deactivate("SUDSChangeRequired");
            }

            if (env->ds.alertAction->isActivated("SUDSReinsertedPrimeRequired"))
            {
                env->ds.alertAction->deactivate("SUDSReinsertedPrimeRequired");
            }
        }
    }
}

void DeviceSuds::updateFluidSource()
{
    if (state == STATE_INACTIVE)
    {
        return;
    }

    bool sudsInserted = env->ds.mcuData->getSudsInserted();
    DS_DeviceDef::FluidSource fluidSourceSuds = env->ds.deviceData->getFluidSourceSuds();
    if (sudsInserted)
    {
        if (!fluidSourceSuds.isInstalled())
        {
            fluidSourceSuds.setIsInstalled(true);
        }
        env->ds.deviceData->setFluidSourceSuds(fluidSourceSuds);
    }
    else
    {
        if (fluidSourceSuds.isInstalled())
        {
            fluidSourceSuds.setIsInstalled(false);
        }
        fluidSourceSuds.isBusy = false;
        fluidSourceSuds.isReady = false;
        env->ds.deviceData->setFluidSourceSuds(fluidSourceSuds);
    }
}

void DeviceSuds::updateStateFromSudsInsertedChanged()
{
    if (state == STATE_INACTIVE)
    {
        return;
    }

    bool sudsInserted = env->ds.mcuData->getSudsInserted();
    if (sudsInserted)
    {
        LOG_DEBUG("updateStateFromSudsInsertedChanged(): New SUDS Inserted - STATE_NOT_PRIMED\n");
        setState(STATE_NOT_PRIMED);
    }
    else
    {
        LOG_DEBUG("updateStateFromSudsInsertedChanged(): SUDS Removed - STATE_MISSING\n");
        setState(STATE_MISSING);
    }
}

void DeviceSuds::updateSudsPortExposedState()
{
    bool sudsInserted = env->ds.mcuData->getSudsInserted();

    if (sudsInserted)
    {
        // SUDS inserted, deactivate all SUDSPortExposed related alerts
        tmrSudsPortExposedTimeout.stop();
        if (env->ds.alertAction->isActivated("SUDSPortExposed", "", true))
        {
            env->ds.alertAction->deactivate("SUDSPortExposed");
        }
        return;
    }

    DS_DeviceDef::FluidSource fluidSourceMuds = env->ds.deviceData->getFluidSourceMuds();
    if (!fluidSourceMuds.isReady)
    {
        // SOD Not completed yet
        return;
    }

    if ( (env->ds.alertAction->isActivated("SUDSPortExposed", "", true)) ||
         (env->ds.alertAction->isActivated("SUDSPortExposedNotifyUser")) )
    {
         LOG_ERROR("updateSudsPortExposedState(): Unexpected State: One of SUDSPortExposed and SUDSPortExposedNotifyUser alerts active\n");
    }


    int timeoutSecs = env->ds.capabilities->get_Alert_SUDSPortExposedLimitSeconds();
    tmrSudsPortExposedTimeout.stop();

    if (timeoutSecs > 0)
    {
        LOG_INFO("updateSudsPortExposedState(): SudsPortExposedTimer is started with timeout=%ds\n", timeoutSecs);
        tmrSudsPortExposedTimeout.start(timeoutSecs * 1000);
    }
}

void DeviceSuds::updateFluidSourceWhileInjecting()
{
    DS_McuDef::StopcockPosAll stopcockPosAll = env->ds.mcuData->getStopcockPosAll();

    for (int scIdx = 0; scIdx < stopcockPosAll.length(); scIdx++)
    {
        if (stopcockPosAll[scIdx] == DS_McuDef::STOPCOCK_POS_MOVING)
        {
            // Stopcock is still moving. Update fliud source later
            return;
        }
    }

    DS_McuDef::InjectorStatus mcuProgress = env->ds.mcuData->getInjectorStatus();
    DS_DeviceDef::FluidSource fluidSourceSuds = env->ds.deviceData->getFluidSourceSuds();

    switch (mcuProgress.state)
    {
        case DS_McuDef::INJECTOR_STATE_DELIVERING:
        {
            fluidSourceSuds.isBusy = true;
        }
        break;
        case DS_McuDef::INJECTOR_STATE_HOLDING:
        case DS_McuDef::INJECTOR_STATE_PHASE_PAUSED:
        case DS_McuDef::INJECTOR_STATE_COMPLETING:
        {
            fluidSourceSuds.isBusy = false;
        }
        break;
        case DS_McuDef::INJECTOR_STATE_IDLE:
        case DS_McuDef::INJECTOR_STATE_READY_START:
        case DS_McuDef::INJECTOR_STATE_COMPLETED:
        default:
        {
        }
        break;
    }

    env->ds.deviceData->setFluidSourceSuds(fluidSourceSuds);
}

void DeviceSuds::updateSudsLed(bool injecting)
{
    SyringeIdx curSyringeIdx = SYRINGE_IDX_NONE;
    const DS_McuDef::StopcockPosAll stopcockPosAll = env->ds.mcuData->getStopcockPosAll();
    for (int scIdx = 0; scIdx < stopcockPosAll.length(); scIdx++)
    {
        if (stopcockPosAll[scIdx] == DS_McuDef::STOPCOCK_POS_INJECT)
        {
            curSyringeIdx = (SyringeIdx)scIdx;
        }
    }

    if (curSyringeIdx == SYRINGE_IDX_NONE)
    {
        for (int scIdx = 0; scIdx < stopcockPosAll.length(); scIdx++)
        {
            if (stopcockPosAll[scIdx] == DS_McuDef::STOPCOCK_POS_MOVING)
            {
                // There are no injecting syringes because they are still moving. don't update LEDs
                LOG_DEBUG("updateSudsLed(): Stopcocks are still moving. Not updating LEDs\n");
                return;
            }
        }
    }

    DS_McuDef::ActLedParams params;
    params.isFlashing = injecting;

    bool ensureNoPatientConnectedAlertActive = env->ds.alertAction->isActivated("EnsureNoPatientConnected", "", true);

    if (injecting) // currently flowing
    {
        if (curSyringeIdx == SYRINGE_IDX_SALINE)
        {
            params.setColorSaline();
        }
        else if (env->ds.deviceData->isSameContrastsLoaded() || curSyringeIdx == SYRINGE_IDX_CONTRAST1)
        {
            params.setColorContrast1();
        }
        else
        {
            params.setColorContrast2();
        }
    }
    else if (ensureNoPatientConnectedAlertActive) //Force set to red...
    {
        params.isFlashing = true;
        params.setColorRed();
    }
    else // not flowing. match SUDS color
    {
        if (getSudsContent() == "Contrast")
        {
            if (env->ds.deviceData->isSameContrastsLoaded() || activeContrastSyringe == SYRINGE_IDX_CONTRAST1)
            {
                params.setColorContrast1();
            }
            else if (activeContrastSyringe == SYRINGE_IDX_CONTRAST2)
            {
                params.setColorContrast2();
            }
        }
        else
        {
            params.setColorSaline();
        }
    }


    LOG_DEBUG("updateSudsLed(): MLFluid=%s, McuProgress=%s, Params[%s]=%s\n",
              ImrParser::ToImr_FluidSourceSyringeIdx(curSyringeIdx).CSTR(),
              ImrParser::ToImr_InjectorState(env->ds.mcuData->getInjectorStatus().state).CSTR(),
              ImrParser::ToImr_LedIndex(LED_IDX_SUDS1).CSTR(),
              Util::qVarientToJsonData(ImrParser::ToImr_ActLedParams(params)).CSTR());

    env->ds.deviceAction->actLeds(LED_IDX_SUDS1, params);
}

DataServiceActionStatus DeviceSuds::actPrimeInit(SyringeIdx syringeIdx, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "InitPrime", QString().asprintf("%s", ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdx).CSTR()));

    if (!env->ds.mcuData->getSudsInserted())
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "SUDS_REMOVED";
        actionStarted(status);
        return status;
    }

    initPrime(syringeIdx);

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);
    status.state = DS_ACTION_STATE_COMPLETED;
    actionCompleted(status);
    return status;
}

DataServiceActionStatus DeviceSuds::actPrimeUpdateState(SyringeIdx syringeIdx, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "PrimeUpdateState", QString().asprintf("%s", ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdx).CSTR()));

    updatePrimeProgress(syringeIdx);

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);
    status.state = DS_ACTION_STATE_COMPLETED;
    actionCompleted(status);
    return status;
}

DataServiceActionStatus DeviceSuds::actSetActiveContrastLocation(SyringeIdx syringeIdx, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "PrimeUpdateState", QString().asprintf("%s", ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdx).CSTR()));

    activeContrastSyringe = syringeIdx;

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);
    status.state = DS_ACTION_STATE_COMPLETED;
    actionCompleted(status);
    return status;
}

void DeviceSuds::updateLedWhileMissing()
{
    if (state == STATE_INACTIVE)
    {
        return;
    }

    if (state != STATE_MISSING)
    {
        // Don't update LED color
        return;
    }

    bool ensureNoPatientConnectedAlertActive = env->ds.alertAction->isActivated("EnsureNoPatientConnected", "", true);
    if (ensureNoPatientConnectedAlertActive)
    {
        return;
    }

    DS_McuDef::ActLedParams params;
    params.setColorOff();
    params.isFlashing = false;

    if (env->ds.mcuData->getMudsInserted())
    {
        DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
        if (fluidSourceSyringes[SYRINGE_IDX_SALINE].sourcePackages.length() > 0)
        {
            // Saline is loaded. SUDS should be waiting state.
            params.setColorWhite();
            params.isFlashing = true;
        }
    }
    env->ds.deviceAction->actLeds(LED_IDX_SUDS1, params);
}

SyringeIdx DeviceSuds::getPrimingSyringeIdxFromStopcockPos()
{
    int stopcockInjPosCount = 0;
    SyringeIdx syringeIdx;

    DS_McuDef::StopcockPosAll stopcockPosAll = env->ds.mcuData->getStopcockPosAll();
    for (int scIdx = 0; scIdx < stopcockPosAll.length(); scIdx++)
    {
        if (stopcockPosAll[scIdx] == DS_McuDef::STOPCOCK_POS_INJECT)
        {
            syringeIdx = (SyringeIdx)scIdx;
            stopcockInjPosCount++;
        }
    }

    if (stopcockInjPosCount != 1)
    {
        // NOT PRIMING: more than one stopcock has inject position
        return SYRINGE_IDX_NONE;
    }

    return syringeIdx;
}

void DeviceSuds::updatePrimeProgressFromVolChanged(QList<double> curVols, QList<double> prevVols)
{
    SyringeIdx syringeIdx = getPrimingSyringeIdxFromStopcockPos();

    //LOG_DEBUG("updatePrimeProgressFromVolChanged syringeIdx=%d\n", syringeIdx);

    if (syringeIdx == SYRINGE_IDX_NONE)
    {
        return;
    }

    if (curVols[syringeIdx] < prevVols[syringeIdx])
    {
        updatePrimeProgress(syringeIdx);
    }
}

void DeviceSuds::updatePrimeProgressFromSyringeStateChanged(DS_McuDef::SyringeStates curStates, DS_McuDef::SyringeStates prevStates)
{
    SyringeIdx syringeIdx = getPrimingSyringeIdxFromStopcockPos();

    //LOG_DEBUG("updatePrimeProgressFromSyringeStateChanged syringeIdx=%d\n", syringeIdx);

    if (syringeIdx != SYRINGE_IDX_NONE)
    {
        if (curStates[syringeIdx] != prevStates[syringeIdx])
        {
            LOG_DEBUG("SyringeState = %s -> %s\n",
                      ImrParser::ToImr_SyringeState(prevStates[syringeIdx]).CSTR(),
                      ImrParser::ToImr_SyringeState(curStates[syringeIdx]).CSTR());
            updatePrimeProgress(syringeIdx);
        }
    }
}

void DeviceSuds::updatePrimeProgress(SyringeIdx syringeIdx)
{
    if (syringeIdx == SYRINGE_IDX_NONE)
    {
        if (state != STATE_NOT_PRIMED)
        {
            LOG_DEBUG("UpdatePrimeProgress(): No Action Syringe Index found. Setting STATE_NOT_PRIMED..\n");
            setState(STATE_NOT_PRIMED);
        }
        return;
    }

    if ( (state != STATE_PRIMING) &&
         (state != STATE_NOT_PRIMED) &&
         (state != STATE_PRIMED) )
    {
        LOG_DEBUG("UpdatePrimeProgress(): Unexpected State=%d\n", state);
        return;
    }

    DS_WorkflowDef::WorkflowState workflowState = env->ds.workflowData->getWorkflowState();
    if ( (workflowState == DS_WorkflowDef::STATE_SYRINGE_AIR_RECOVERY_PROGRESS) ||
         (workflowState == DS_WorkflowDef::STATE_FLUID_REMOVAL_PROGRESS) ||
         (workflowState == DS_WorkflowDef::STATE_AUTO_EMPTY_PROGRESS) )
    {
        if (state != STATE_NOT_PRIMED)
        {
            LOG_DEBUG("UpdatePrimeProgress(): WorkflowState=%s. Setting STATE_NOT_PRIMED..\n", ImrParser::ToImr_WorkFlowState(workflowState).CSTR());
            setState(STATE_NOT_PRIMED);
        }
        return;
    }

    DS_WorkflowDef::MudsSodStatus mudsSodStatus = env->ds.workflowData->getMudsSodStatus();
    if (!mudsSodStatus.syringeSodStatusAll[syringeIdx].isCompleted())
    {
        if (state != STATE_NOT_PRIMED)
        {
            LOG_DEBUG("UpdatePrimeProgress(): SOD[%d] is not completed. Setting STATE_NOT_PRIMED..\n", syringeIdx);
            setState(STATE_NOT_PRIMED);
        }
        return;
    }

    DS_SystemDef::StatePath curStatePath = env->ds.systemData->getStatePath();
    if (curStatePath != DS_SystemDef::STATE_PATH_IDLE)
    {
        // Don't set SUDS state when current state path is not IDLE.
        // E.g. during injection SUDS is not priming.
        LOG_DEBUG("UpdatePrimeProgress(): Invalid StatePath(%s)\n", ImrParser::ToImr_StatePath(curStatePath).CSTR());
        return;
    }

    if (state == STATE_NOT_PRIMED)
    {
        initPrime(syringeIdx);
    }

    LOG_INFO("UpdatePrimeProgress(): Syringe[%d]: Priming..\n", syringeIdx);

    // Reset the Start Volume if air detected
    if (env->ds.mcuData->getSudsBubbleDetected())
    {
        resetPrimeParameters(listPrimedFluids[0].syringeIdx, listPrimedFluids[0].fluidType);
        LOG_WARNING("UpdatePrimeProgress(): SUDS Air Detected, Prime values reset done\n");
    }

    if (listPrimedFluids.length() == 0)
    {
        LOG_WARNING("UpdatePrimeProgress(): listPrimedFluids.length = 0\n");
        return;
    }

    QList<double> curVols = env->ds.mcuData->getSyringeVols();
    double primedVol = primeStartSyringeVol - curVols[listPrimedFluids[0].syringeIdx];
    double primeTotalVol = env->ds.workflowData->getAutoPrimeVolume();
    listPrimedFluids[0].primedVol = qMax(listPrimedFluids[0].primedVol, primedVol);

    // Update ListPrimeVol array; i.e. remove old primed fluid
    double totalPrimed = 0;
    for (int primedFluidIdx = 0; primedFluidIdx < listPrimedFluids.length(); primedFluidIdx++)
    {
        totalPrimed += listPrimedFluids[primedFluidIdx].primedVol;
        if (Util::isFloatVarGreaterThan(totalPrimed, primeTotalVol, 1))
        {
            double volToWasteBin = totalPrimed - primeTotalVol;
            if (Util::isFloatVarGreaterThan(listPrimedFluids[primedFluidIdx].primedVol, volToWasteBin, 1))
            {
                listPrimedFluids[primedFluidIdx].primedVol = listPrimedFluids[primedFluidIdx].primedVol - volToWasteBin;
            }
            else
            {
                LOG_DEBUG("UpdatePrimeProgress(): Removing item[%d]: syringeIdx=%s, fluidType=%s as vol=%.1f > %.1fml\n",
                          primedFluidIdx,
                          ImrParser::ToImr_FluidSourceSyringeIdx(listPrimedFluids[primedFluidIdx].syringeIdx).CSTR(),
                          ImrParser::ToImr_FluidSourceSyringeIdx(listPrimedFluids[primedFluidIdx].fluidType).CSTR(),
                          listPrimedFluids[primedFluidIdx].primedVol,
                          totalPrimed - primeTotalVol);
                listPrimedFluids.removeAt(primedFluidIdx);
                totalPrimed = 0;
                primedFluidIdx = -1;
            }
        }
    }

    // Update prime pressure data
    if (listPrimedPressures.length() == 0)
    {
        //This is the first primed pressure reading
        primeStartEpochMs = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
    }
    PrimedPressureData pressureData;
    pressureData.pressure = env->ds.mcuData->getPressure();
    pressureData.timeMs = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch() - primeStartEpochMs;
    listPrimedPressures.append(pressureData);

    // Calculate progress percent
    if (listPrimedFluids.length() == 0)
    {
        LOG_WARNING("UpdatePrimeProgress(): listPrimedFluids.length = 0\n");
        return;
    }

    // Update primed percentage. If it is really primed before, the prime percentage stays same (i.e. 100%)
    int curFluidPrimePercent = (listPrimedFluids[0].primedVol * 100) / primeTotalVol;
    primedPercentage = qMax(curFluidPrimePercent, primedPercentage);

    DS_McuDef::SyringeState syringeState = env->ds.mcuData->getSyringeStates()[syringeIdx];

    LOG_DEBUG("UpdatePrimeProgress(): Primed: syringe=%s, fluidType=%s, primedVol=%.1f, total=%.1f, syringeState=%s: CurPrimed=%d%%, Overall=%d%%\n",
              ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdx).CSTR(),
              ImrParser::ToImr_FluidSourceSyringeIdx(listPrimedFluids[0].fluidType).CSTR(),
              listPrimedFluids[0].primedVol,
              primeTotalVol,
              ImrParser::ToImr_SyringeState(syringeState).CSTR(),
              curFluidPrimePercent,
              primedPercentage);

    if (primedPercentage >= 100)
    {
        // Check if the priming is in progress
        switch (syringeState)
        {
            case DS_McuDef::SYRINGE_STATE_PROCESSING:
                LOG_DEBUG("UpdatePrimeProgress(): Suds primed 100%% but prime action is still in progress\n");
                break;
            case DS_McuDef::SYRINGE_STATE_AIR_DETECTED:
            case DS_McuDef::SYRINGE_STATE_OVER_PRESSURE:
            case DS_McuDef::SYRINGE_STATE_OVER_CURRENT:
                LOG_WARNING("UpdatePrimeProgress(): Unexpected syringeState(%s). Setting STATE_PRIME_FAILED..\n", ImrParser::ToImr_SyringeState(syringeState).CSTR());
                setState(STATE_PRIME_FAILED);
                break;
            default:
                LOG_DEBUG("UpdatePrimeProgress(): Suds primed 100%% with SyringeState(%s). Setting STATE_PRIMED..\n", ImrParser::ToImr_SyringeState(syringeState).CSTR());
                setState(STATE_PRIMED);
                break;
        }
    }
    else
    {
        // Priming is still less than 100%
        switch (syringeState)
        {
        case DS_McuDef::SYRINGE_STATE_PROCESSING:
            if (state != STATE_PRIMING)
            {
                setState(STATE_PRIMING);
            }
            break;
        case DS_McuDef::SYRINGE_STATE_USER_ABORT:
            LOG_DEBUG("UpdatePrimeProgress(): Manual Prime Stopped. Setting NotPrimed..\n");
            actSetStateNotPrimed(false);
            break;
        case DS_McuDef::SYRINGE_STATE_COMPLETED:
            LOG_DEBUG("UpdatePrimeProgress(): Syringe Action Completed with primed=%d%%. Setting NotPrimed..\n", primedPercentage);
            actSetStateNotPrimed(false);
            break;
        case DS_McuDef::SYRINGE_STATE_SUDS_REMOVED:
        case DS_McuDef::SYRINGE_STATE_OVER_PRESSURE:
        case DS_McuDef::SYRINGE_STATE_OVER_CURRENT:
        case DS_McuDef::SYRINGE_STATE_AIR_DETECTED:
        default:
            LOG_WARNING("UpdatePrimeProgress(): Unexpected syringeState(%s). Setting STATE_PRIME_FAILED..\n", ImrParser::ToImr_SyringeState(syringeState).CSTR());
            setState(STATE_PRIME_FAILED);
            break;
        }
    }
}

bool DeviceSuds::isSetStateReady()
{
    if (state == STATE_INACTIVE)
    {
        return false;
    }
    return true;
}

void DeviceSuds::processState()
{
    DS_DeviceDef::FluidSource fluidSourceSuds = env->ds.deviceData->getFluidSourceSuds();

    switch (state)
    {
    case STATE_INIT:
        LOG_INFO("STATE_INIT\n");
        setState(STATE_MISSING);
        break;
    case STATE_INACTIVE:
        LOG_INFO("STATE_INACTIVE\n");
        break;
    case STATE_MISSING:
        LOG_INFO("STATE_MISSING\n");
        {
            env->ds.examData->setIsAirCheckNeeded(true);
            listPrimedFluids.clear();

            updateLedWhileMissing();

            fluidSourceSuds.isReady = false;
            fluidSourceSuds.isBusy = false;
            fluidSourceSuds.needsReplaced = false;
            env->ds.deviceData->setFluidSourceSuds(fluidSourceSuds);

            primedPercentage = 0;

            deactivateEnsureNoPatientConnectedAlert();
        }
        break;
    case STATE_NOT_PRIMED:
        LOG_INFO("STATE_NOT_PRIMED\n");
        {
            if (fluidSourceSuds.needsReplaced)
            {
                setState(STATE_NEEDS_REPLACE);
                break;
            }

            if(isFluidFlowingThroughSuds())
            {
                updateSudsLed(true);
            }
            else
            {
                DS_McuDef::ActLedParams params;
                params.setColorRed();
                params.isFlashing = true;
                env->ds.deviceAction->actLeds(LED_IDX_SUDS1, params);
            }

            fluidSourceSuds.isReady = false;
            fluidSourceSuds.isBusy = false;
            env->ds.deviceData->setFluidSourceSuds(fluidSourceSuds);

            activateEnsureNoPatientConnectedAlert();
        }
        break;
    case STATE_PRIMING:
        LOG_INFO("STATE_PRIMING\n");
        if (listPrimedFluids.length() > 0)
        {
            fluidSourceSuds.isReady = false;
            fluidSourceSuds.isBusy = true;
            env->ds.deviceData->setFluidSourceSuds(fluidSourceSuds);

            activateEnsureNoPatientConnectedAlert();
        }
        break;
    case STATE_PRIMED:
        LOG_INFO("STATE_PRIMED\n");
        if (fluidSourceSuds.needsReplaced)
        {
            setState(STATE_NEEDS_REPLACE);
            break;
        }

        if (listPrimedFluids.length() > 0)
        {
            DS_McuDef::ActLedParams params;
            params.isFlashing = false;

            // when priming is done, we want to match LED color to SUDS color in device manager.
            // i.e if there is ANY contrast in PL, show contrast color regardless of what was pushed last
            if (getSudsContent() == "Contrast")
            {
                if (env->ds.deviceData->isSameContrastsLoaded() || activeContrastSyringe == SYRINGE_IDX_CONTRAST1)
                {
                    params.setColorContrast1();
                }
                else if (activeContrastSyringe == SYRINGE_IDX_CONTRAST2)
                {
                    params.setColorContrast2();
                }
                else
                {
                    // it shouldn't reach here.. but safety net
                    params.setColorSaline();
                }
            }
            else
            {
                params.setColorSaline();
            }

            LOG_INFO("STATE_PRIMED: Primed with %s\n", ImrParser::ToImr_FluidSourceSyringeIdx(listPrimedFluids[0].fluidType).CSTR());
            dumpListPrimedFluids();


            env->ds.deviceAction->actLeds(LED_IDX_SUDS1, params);

            fluidSourceSuds.isReady = true;
            fluidSourceSuds.isBusy = false;
            env->ds.deviceData->setFluidSourceSuds(fluidSourceSuds);
            env->ds.deviceData->setIsFirstAutoPrimeCompleted(true);

            primedPercentage = 100;

            // If we have at least 5 data points, evaluate the result of the prime.
            // We need to do this as this code can be hit multiple times for the
            // same prime event. The "5" acts as a debounce for us.
            if (listPrimedPressures.length() > 5)
            {
                evaluatePrimePressures();
            }

            deactivateEnsureNoPatientConnectedAlert();
        }
        break;
    case STATE_NEEDS_REPLACE:
        LOG_INFO("STATE_NEEDS_REPLACE\n");
        {
            fluidSourceSuds.isReady = false;
            fluidSourceSuds.isBusy = false;
            fluidSourceSuds.needsReplaced = true;
            env->ds.deviceData->setFluidSourceSuds(fluidSourceSuds);
            env->ds.deviceData->setIsFirstAutoPrimeCompleted(false);

            if (env->ds.mcuData->getSudsInserted())
            {
                DS_McuDef::ActLedParams params;
                params.isFlashing = false;
                params.setColorOrange();
                env->ds.deviceAction->actLeds(LED_IDX_SUDS1, params);
                env->ds.alertAction->activate("SUDSChangeRequired");
            }
            else
            {
                setState(STATE_MISSING);
            }

            deactivateEnsureNoPatientConnectedAlert();
        }
        break;
    case STATE_PRIME_FAILED:
        LOG_INFO("STATE_PRIME_FAILED\n");
        {
            if (fluidSourceSuds.needsReplaced)
            {
                setState(STATE_NEEDS_REPLACE);
                break;
            }

            DS_McuDef::ActLedParams params;
            params.setColorRed();
            params.isFlashing = true;
            env->ds.deviceAction->actLeds(LED_IDX_SUDS1, params);

            fluidSourceSuds.isReady = false;
            fluidSourceSuds.isBusy = false;
            env->ds.deviceData->setFluidSourceSuds(fluidSourceSuds);

            if (listPrimedFluids.length() > 0)
            {
                resetPrimeParameters(listPrimedFluids[0].syringeIdx, listPrimedFluids[0].fluidType);
            }

            activateEnsureNoPatientConnectedAlert();
        }
        break;
    default:
        break;
    }
}

DataServiceActionStatus DeviceSuds::actSetNeedsReplace(bool needed, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "actSetNeedsReplace");

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);

    if (env->ds.mcuData->getSudsInserted())
    {
        bool forceSetNeedsReplace = shouldForceSetNeedsReplace();
        if ((needed && isSudsNeedsReplace()) || forceSetNeedsReplace)
        {
            LOG_DEBUG("actSetNeedsReplace(): Forced to set to STATE_NEEDS_REPLACE state\n");
            setState(STATE_NEEDS_REPLACE);
        }
        else
        {
            DS_DeviceDef::FluidSource fluidSourceSuds = env->ds.deviceData->getFluidSourceSuds();
            fluidSourceSuds.needsReplaced = false;
            env->ds.deviceData->setFluidSourceSuds(fluidSourceSuds);
        }
    }

    status.state = DS_ACTION_STATE_COMPLETED;
    actionCompleted(status);

    return status;
}

DataServiceActionStatus DeviceSuds::actSetStateNotPrimed(bool clearProgress, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "actSetStateNotPrimed");

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);

    if (env->ds.mcuData->getSudsInserted())
    {
        DS_DeviceDef::FluidSource fluidSourceSuds = env->ds.deviceData->getFluidSourceSuds();
        if (fluidSourceSuds.needsReplaced)
        {
            LOG_DEBUG("actSetStateNotPrimed(): SUDS NeedsReplaced. Setting to STATE_NEEDS_REPLACE..\n");
            setState(STATE_NEEDS_REPLACE);
        }
        else
        {
            if (clearProgress)
            {
                primedPercentage = 0;
            }

            fluidSourceSuds.currentFluidKinds.clear();
            fluidSourceSuds.currentVolumes.clear();

            LOG_DEBUG("actSetStateNotPrimed(): Setting to STATE_NOT_PRIMED..\n");
            setState(STATE_NOT_PRIMED);
        }
    }

    status.state = DS_ACTION_STATE_COMPLETED;
    actionCompleted(status);

    return status;
}

DataServiceActionStatus DeviceSuds::actSetStatePrimed(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "actSetStateNotPrimed");

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);

    LOG_DEBUG("actSetStatePrimed(): Forced to set to PRIMED state\n");
    if (env->ds.mcuData->getSudsInserted())
    {
        DS_DeviceDef::FluidSource fluidSourceMuds = env->ds.deviceData->getFluidSourceMuds();
        DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
        PrimedFluid lastPrimedFluid;

        if (fluidSourceMuds.sourcePackages.length() == 0)
        {
            LOG_ERROR("actSetStatePrimed(): Setting to Primed State but ML has no fluid.\n");
            fluidSourceMuds.sourcePackages.append(fluidSourceSyringes[SYRINGE_IDX_SALINE].sourcePackages[0]);
        }

        // Find previously prime fluid
        for (int syringeIdx = 0; syringeIdx < fluidSourceSyringes.length(); syringeIdx++)
        {
            if ( (fluidSourceSyringes[syringeIdx].sourcePackages.length() > 0) &&
                 (fluidSourceSyringes[syringeIdx].sourcePackages[0].isSameFluidPackage(fluidSourceMuds.sourcePackages[0])) )
            {
                lastPrimedFluid.syringeIdx = (SyringeIdx)syringeIdx;
                lastPrimedFluid.fluidType = (SyringeIdx)syringeIdx;
                break;
            }
        }

        if (lastPrimedFluid.syringeIdx == SYRINGE_IDX_NONE)
        {
            LOG_ERROR("actSetStatePrimed(): Previously primed fluid cannot be determined:\n"
                      "sourcePackages[RS0]=%s\n"
                      "sourcePackages[RC1]=%s\n"
                      "sourcePackages[RC2]=%s\n"
                      "sourcePackages[ML]=%s\n",
                      Util::qVarientToJsonData(ImrParser::ToImr_FluidPackages(fluidSourceSyringes[SYRINGE_IDX_SALINE].sourcePackages)).CSTR(),
                      Util::qVarientToJsonData(ImrParser::ToImr_FluidPackages(fluidSourceSyringes[SYRINGE_IDX_CONTRAST1].sourcePackages)).CSTR(),
                      Util::qVarientToJsonData(ImrParser::ToImr_FluidPackages(fluidSourceSyringes[SYRINGE_IDX_CONTRAST2].sourcePackages)).CSTR(),
                      Util::qVarientToJsonData(ImrParser::ToImr_FluidPackages(fluidSourceMuds.sourcePackages)).CSTR());
        }

        lastPrimedFluid.primedVol = env->ds.workflowData->getAutoPrimeVolume();
        listPrimedFluids.clear();
        listPrimedFluids.push_front(lastPrimedFluid);
        LOG_DEBUG("actSetStatePrimed(): last prime fluid added\n");
        dumpListPrimedFluids();
        setState(STATE_PRIMED);
    }

    status.state = DS_ACTION_STATE_COMPLETED;
    actionCompleted(status);

    return status;
}

void DeviceSuds::initPrime(SyringeIdx syringeIdx)
{
    // Reset prime pressure data
    resetPrimePressures();

    // Set the starting prime manifold fluid
    primeStartManifoldHasContrast = false;
    DS_DeviceDef::FluidSource fluidSourceSuds = env->ds.deviceData->getFluidSourceSuds();
    for (int i = 0; i < fluidSourceSuds.currentFluidKinds.length(); i++)
    {
        if (fluidSourceSuds.currentFluidKinds[i] == "Contrast")
        {
            primeStartManifoldHasContrast = true;
            break;
        }
    }

    // Prepare PrimedFluid
    PrimedFluid item;
    item.syringeIdx = syringeIdx;

    if ( (item.syringeIdx == SYRINGE_IDX_CONTRAST2) &&
         (env->ds.deviceData->isSameContrastsLoaded()) )
    {
        item.fluidType = SYRINGE_IDX_CONTRAST1;
    }
    else
    {
        item.fluidType = syringeIdx;
    }

    if ( (listPrimedFluids.length() == 0) ||
         (listPrimedFluids[0].fluidType != item.fluidType) )
    {
        item.primedVol = 0;
        LOG_DEBUG("initPrime(): Adding item: syringeIdx=%s, fluidType=%s, %.1fml\n",
                  ImrParser::ToImr_FluidSourceSyringeIdx(item.syringeIdx).CSTR(),
                  ImrParser::ToImr_FluidSourceSyringeIdx(item.fluidType).CSTR(),
                  item.primedVol);
        listPrimedFluids.push_front(item);
        dumpListPrimedFluids();
    }

    listPrimedFluids[0].syringeIdx = item.syringeIdx;

    QList<double> vols = env->ds.mcuData->getSyringeVols();
    primeStartSyringeVol = vols[listPrimedFluids[0].syringeIdx] + listPrimedFluids[0].primedVol;
    LOG_DEBUG("initPrime(): PrimeStartSyringeVol = %f\n", primeStartSyringeVol);

    DS_WorkflowDef::WorkflowState workflowState = env->ds.workflowData->getWorkflowState();
    if ( (workflowState == DS_WorkflowDef::STATE_SYRINGE_AIR_RECOVERY_PROGRESS) ||
         (workflowState == DS_WorkflowDef::STATE_FLUID_REMOVAL_PROGRESS) ||
         (workflowState == DS_WorkflowDef::STATE_AUTO_EMPTY_PROGRESS) )
    {
        setState(STATE_NOT_PRIMED);
    }
    else
    {
        setState(STATE_PRIMING);
    }
}

void DeviceSuds::resetPrimeParameters(SyringeIdx syringeIdx, SyringeIdx fluidType)
{
    LOG_DEBUG("resetPrimeParameters()");
    primeStartSyringeVol = env->ds.mcuData->getSyringeVols()[syringeIdx];
    PrimedFluid item;
    item.syringeIdx = syringeIdx;
    item.fluidType = fluidType;
    item.primedVol = 0;
    listPrimedFluids.clear();
    listPrimedFluids.push_front(item);

    primedPercentage = 0;

    LOG_DEBUG("resetPrimeParameters():\n");
    dumpListPrimedFluids();

    // Reset prime pressures as well.
    resetPrimePressures();
}

void DeviceSuds::resetPrimePressures()
{
    listPrimedPressures.clear();
    primeStartManifoldHasContrast = false;
}

// Refer to DF-154753 for technical analysis/details.
void DeviceSuds::evaluatePrimePressures()
{
    if (listPrimedPressures.length() == 0)
    {
        LOG_ERROR("Function 'evaluatePrimePressures' was called with no prime pressure data to record.\n");
        return;
    }

    double slopeScore = 0;
    bool success = evaluatePrimePressuresSlopeScore(slopeScore);
    if (success)
    {
        // The cutoff numbers are not meant to be inclusive
        double slopeCutoffLow = env->ds.capabilities->get_Alert_SUDSPrimeDataSlopeCutoffLow();
        double slopeCutoffHigh = env->ds.capabilities->get_Alert_SUDSPrimeDataSlopeCutoffHigh();

        // Create alert data; Result can be 'NEW', 'USED', or 'UNCERTAIN'
        QString result = "UNCERTAIN";
        if (slopeScore > slopeCutoffHigh)
            result = "NEW";
        else if (slopeScore < slopeCutoffLow)
            result = "USED";
        QString manifoldHadContrast = primeStartManifoldHasContrast ? "true" : "false";
        QString alertData = QString().asprintf("%s;%f;%s;", result.CSTR(), slopeScore, manifoldHadContrast.CSTR());
        for (int primeIdx = 0; primeIdx < listPrimedPressures.length(); primeIdx++)
        {
            if (primeIdx > 0)
            {
                alertData += ";";
            }
            alertData += QString().asprintf("%d,%d", listPrimedPressures[primeIdx].timeMs, listPrimedPressures[primeIdx].pressure);
        }

        LOG_INFO("Raising SRUPrimePressureData alert with %d data points and a result of %s.\n", (int)listPrimedPressures.length(), result.CSTR());
        env->ds.alertAction->activate("SRUPrimePressureData", alertData);
    }

    // Now that we made our evaluation, reset the pressure data.
    resetPrimePressures();
}

bool DeviceSuds::evaluatePrimePressuresSlopeScore(double &slopeScore)
{
    int startSlopeTime = env->ds.capabilities->get_Alert_SUDSPrimeDataSlopeStartTime();
    int endSlopeTime = env->ds.capabilities->get_Alert_SUDSPrimeDataSlopeEndTime();
    int startSteadyStateTime = env->ds.capabilities->get_Alert_SUDSPrimeDataSteadyStateStartTime();
    int endSteadyStateTime = env->ds.capabilities->get_Alert_SUDSPrimeDataSteadyStateEndTime();

    if (listPrimedPressures.length() < 5)
    {
        LOG_ERROR("evaluatePrimePressuresSlopeScore(): pressure count is < 5. Exiting.\n");
        return false; // The number of pressure recordings is too small
    }    

    // STEP 1 - Get the steady state pressure value

    // Get the steady state pressures for normalizing
    QList<int> steadyStatePressures;
    for (int i = 0; i < listPrimedPressures.length(); i++)
    {
        PrimedPressureData pressureData = listPrimedPressures[i];
        if ( (pressureData.timeMs >= startSteadyStateTime) &&
             (pressureData.timeMs <= endSteadyStateTime) )
        {
            steadyStatePressures.append(pressureData.pressure);
        }
    }

    // Validate that we have enough pressures to get a steady state pressure value
    if (steadyStatePressures.length() == 0)
    {
        LOG_ERROR("evaluatePrimePressuresSlopeScore(): steady state pressures logged between times %d and %d is zero. Exiting.\n",
                  startSteadyStateTime, endSteadyStateTime);
        return false; // The steady state pressure is not usable, data is invalid
    }

    // Get the steady state pressure
    double steadyStatePressure = getSteadyStatePressure(steadyStatePressures);

    // Validate that the steady state pressure is within the expected range
    if (steadyStatePressure < 100)
    {
        LOG_ERROR("evaluatePrimePressuresSlopeScore(): steady state pressure %f is < 100. Invalid. Exiting.\n", steadyStatePressure);
        return false; // The steady state pressure is not usable, data is invalid
    }

    // STEP 2 - Normalize and gather core pressure data

    //Divide all pressure values by the steady state values for the range that we are interested in
    QList<int> x_values;
    QList<double> y_values;
    for (int i = 0; i < listPrimedPressures.length(); i ++)
    {
        PrimedPressureData data = listPrimedPressures[i];
        if ( (data.timeMs >= startSlopeTime) &&
             (data.timeMs <= endSlopeTime))
        {
            x_values.append(data.timeMs);
            y_values.append((data.pressure / (double)steadyStatePressure));
        }
    }

    // Validate that we have an expected number or normalized pressures within the expected range
    if (x_values.length() < 4)
    {
        LOG_ERROR("evaluatePrimePressuresSlopeScore(): pressures recorded within %d and %d is %d. Invalid. Exiting.\n", startSlopeTime, endSlopeTime, (int)x_values.length());
        return false; // The steady state pressure is not usable, data is invalid
    }

    // STEP 3 - Compute a linear regression using the method of least squares and return the slope

    double slope = getLinearRegressionSlope(x_values, y_values);
    slopeScore = slope * 10000; // make the slope score readable...

    LOG_INFO("evaluatePrimePressuresSlopeScore(): calculated the following slope for %d pressure data points: %f.\n", (int)x_values.length(), slopeScore);
    return true;
}

double DeviceSuds::getSteadyStatePressure(QList<int> pressures)
{
    // Iterate through the steady state pressures to get the "mode", else "mean"
    double steadyStatePressure = 0;
    int maxCount = 0;
    int totalPressure = 0;
    for (int i = 0; i < pressures.length(); i++)
    {
        int count = 0;
        for (int j= 0; j < pressures.length(); j++)
        {
            if (pressures[j] == pressures[i])
            {
                count++;
            }
        }

        if (count > maxCount)
        {
            maxCount = count;
            steadyStatePressure = pressures[i];
        }

        totalPressure += pressures[i];
    }
    if (steadyStatePressure == 0)
    {
        steadyStatePressure = (totalPressure / pressures.length());
        LOG_DEBUG("getSteadyStatePressure(): MEAN steady state pressure is %f.\n", steadyStatePressure);
    }
    else
    {
        LOG_DEBUG("getSteadyStatePressure(): MODE steady state pressure is %f.\n", steadyStatePressure);
    }

    return steadyStatePressure;
}

double DeviceSuds::getLinearRegressionSlope(QList<int> x_values, QList<double> y_values)
{
    // Perform linear regression formula
    double sum_x = 0;
    double sum_y = 0;
    double sum_xy = 0;
    double sum_xx = 0;

    int n = x_values.length();
    for (int i = 0; i < n; i++)
    {
        int x = x_values[i];
        double y = y_values[i];
        sum_x += x;
        sum_y += y;
        sum_xy += x * y;
        sum_xx += x * x;
    }

    // Return calculated slope
    return (n * sum_xy - sum_x * sum_y) / (n * sum_xx - sum_x * sum_x);
}

void DeviceSuds::dumpListPrimedFluids()
{
    for (int fluidIdx = 0; fluidIdx < listPrimedFluids.length(); fluidIdx++)
    {
        LOG_DEBUG("dumpListPrimedFluids(): ListPrimedFluids[%d]: syringeIdx=%s, FluidType=%s: %.1fml\n",
                  fluidIdx,
                  ImrParser::ToImr_FluidSourceSyringeIdx(listPrimedFluids[fluidIdx].syringeIdx).CSTR(),
                  ImrParser::ToImr_FluidSourceSyringeIdx(listPrimedFluids[fluidIdx].fluidType).CSTR(),
                  listPrimedFluids[fluidIdx].primedVol);
    };
}

// determine conditions for force setting needs replace
bool DeviceSuds::shouldForceSetNeedsReplace()
{
    return (env->ds.workflowData->getSodErrorState() == DS_WorkflowDef::SOD_ERROR_STATE_REUSE_MUDS_WAITING_SUDS_REMOVAL);
}

void DeviceSuds::activateEnsureNoPatientConnectedAlert()
{
    if (!env->ds.alertAction->isActivated("EnsureNoPatientConnected"))
    {
        env->ds.alertAction->activate("EnsureNoPatientConnected");
    }
}

void DeviceSuds::deactivateEnsureNoPatientConnectedAlert()
{
    if (env->ds.alertAction->isActivated("EnsureNoPatientConnected"))
    {
        env->ds.alertAction->deactivate("EnsureNoPatientConnected");
    }
}
