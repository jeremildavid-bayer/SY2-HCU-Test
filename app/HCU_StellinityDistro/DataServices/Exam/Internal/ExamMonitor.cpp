#include "Apps/AppManager.h"
#include "ExamMonitor.h"
#include "Common/ImrParser.h"
#include "DataServices/CfgGlobal/DS_CfgGlobal.h"
#include "DataServices/Exam/DS_ExamData.h"
#include "DataServices/Exam/DS_ExamAction.h"
#include "DataServices/Mcu/DS_McuData.h"
#include "DataServices/Device/DS_DeviceData.h"
#include "DataServices/System/DS_SystemData.h"
#include "DataServices/Workflow/DS_WorkflowData.h"
#include "DataServices/Alert/DS_AlertAction.h"
#include "DataServices/Alert/DS_AlertData.h"
#include "DataServices/Cru/DS_CruData.h"
#include "DataServices/Capabilities/DS_Capabilities.h"

ExamMonitor::ExamMonitor(QObject *parent, EnvGlobal *env_) :
    QObject(parent),
    env(env_)
{
    envLocal = new EnvLocal("DS_Exam-Monitor", "EXAM_MONITOR");

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });
}

ExamMonitor::~ExamMonitor()
{
    delete envLocal;
}

void ExamMonitor::slotAppInitialised()
{
    connect(env->ds.systemData, &DS_SystemData::signalDataChanged_StatePath, this, [=](DS_SystemDef::StatePath curStatePath, DS_SystemDef::StatePath prevStatePath) {
        if ( ( (prevStatePath == DS_SystemDef::STATE_PATH_IDLE) && (curStatePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING) ) ||
             ( (prevStatePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING) && (curStatePath == DS_SystemDef::STATE_PATH_IDLE) ) )
        {
            LOG_INFO("\n\n-----------------\nStatePath=%s\n\n", ImrParser::ToImr_StatePath(curStatePath).CSTR());
        }
    });

    connect(env->ds.examData, &DS_ExamData::signalDataChanged_InjectionPlanTemplateGroups, this, [=](const DS_ExamDef::InjectionPlanTemplateGroups &groups_) {
        DS_ExamDef::InjectionPlanTemplateGroups groups = groups_;

        // Change current planPreview if required
        DS_ExamDef::InjectionPlan planPreview = env->ds.examData->getInjectionPlanPreview();
        for (int groupIdx = 0; groupIdx < groups.length(); groupIdx++)
        {
            DS_ExamDef::InjectionPlanDigest *planDigest = groups[groupIdx].getPlanDigestFromTemplateGuid(planPreview.templateGuid);

            if (planDigest != NULL)
            {
                if (planDigest->plan == planPreview)
                {
                    LOG_DEBUG("Preview injection plan is already updated (guid=%s)\n", planDigest->plan.guid.CSTR());
                }
                else if (planDigest->plan.guid == EMPTY_GUID)
                {
                    LOG_DEBUG("Selected injection plan is not ready yet (guid=%s)\n", planDigest->plan.guid.CSTR());
                }
                else
                {
                    LOG_INFO("Preview injection plan is updated (guid=%s):\n%s\n", planDigest->plan.guid.CSTR(), Util::qVarientToJsonData(ImrParser::ToImr_InjectionPlan(planDigest->plan)).CSTR());
                    env->ds.examData->setInjectionPlanPreview(planDigest->plan);
                }
                break;
            }
        }
    });

    connect(env->ds.deviceData, &DS_DeviceData::signalDataChanged_FluidSourceSyringes, this, [=](const DS_DeviceDef::FluidSources &fluidSourceSyringes, const DS_DeviceDef::FluidSources &prevFluidSourceSyringes) {
        bool setNewContrast = false;

        auto syringeContrast1 = fluidSourceSyringes[SYRINGE_IDX_CONTRAST1];
        auto syringeContrast2 = fluidSourceSyringes[SYRINGE_IDX_CONTRAST2];
        auto prevSyringeContrast1 = prevFluidSourceSyringes[SYRINGE_IDX_CONTRAST1];
        auto prevSyringeContrast2 = prevFluidSourceSyringes[SYRINGE_IDX_CONTRAST2];

        double syringeContrast1Volume = syringeContrast1.currentAvailableVolumesTotal();
        double syringeContrast2Volume = syringeContrast2.currentAvailableVolumesTotal();
        double prevSyringeContrast1Volume = prevSyringeContrast1.currentAvailableVolumesTotal();
        double prevSyringeContrast2Volume = prevSyringeContrast2.currentAvailableVolumesTotal();

        double requiredVolume = env->ds.capabilities->get_AirCheck_SyringeAirCheckRequiredVol();

        if ( (syringeContrast1.sourcePackages != prevSyringeContrast1.sourcePackages) ||
             (syringeContrast2.sourcePackages != prevSyringeContrast2.sourcePackages) )
        {
            setNewContrast = true;
        }
        else if ( ((syringeContrast1Volume == 0) && (prevSyringeContrast1Volume != 0)) ||
                  ((syringeContrast2Volume == 0) && (prevSyringeContrast2Volume != 0)) )
        {
            setNewContrast = true;
        }
        else if ( (syringeContrast1.isReady != prevSyringeContrast1.isReady) ||
                  (syringeContrast2.isReady != prevSyringeContrast2.isReady) )
        {
            setNewContrast = true;
        }
        // auto-empty enabled syringes rely on the volume of the syringe to be larger than 0 to indicate that the syringe can be used for injection (refer to DS_ExamAction.cpp:actReloadSelectedContrast())
        // however, during fill, this method only sets new contrast before filling (sourcePackages change) so syringe never becomes available for injection until other actions trigger (eg, endexam)
        // so, extra trigger has been added to ensure that once the filling goes beyond required volume for air-check, we trigger setNewContrast
        // NOTE: maybe we can remove sourcepackages check and just use this one?
        else if ( ((prevSyringeContrast1Volume <= requiredVolume) && (syringeContrast1Volume > requiredVolume)) ||
                  ((prevSyringeContrast2Volume <= requiredVolume) && (syringeContrast2Volume > requiredVolume)) )
        {
            setNewContrast = true;
        }

        if (setNewContrast)
        {
            env->ds.examAction->actReloadSelectedContrast();
        }

        handleArmedStateWhileSyringesBusy();
    });

    connect(env->ds.examData, &DS_ExamData::signalDataChanged_InjectionPlan, this, [=](const DS_ExamDef::InjectionPlan &plan, const DS_ExamDef::InjectionPlan &prevPlan) {
        DS_DeviceDef::FluidSourceIdx contrastLocation = env->ds.examData->getSelectedContrast().location;
        env->ds.examAction->actSetContrastFluidLocation(contrastLocation);
        handlePersonalizedProtocol();

        handlePreloadProtocol(prevPlan);
        handleManualPrimeWhileInjectionPreloaded();

        if (plan.guid != prevPlan.guid)
        {
            if (env->ds.alertAction->isActivated("DeparameterizedProtocol"))
            {
                LOG_INFO("signalDataChanged_InjectionPlanPreview(): Plan selected(%s->%s). Deactivating 'DeparameterizedProtocol' alert..\n", prevPlan.guid.CSTR(), plan.guid.CSTR());
                env->ds.alertAction->deactivate("DeparameterizedProtocol");
            }
        }
    });

    connect(env->ds.examData, &DS_ExamData::signalDataChanged_ScannerInterlocks, this, [=] {
        handleArmedStateFromCruLinkStatusChanged();
    });

    connect(env->ds.cruData, &DS_CruData::signalDataChanged_CruLinkStatus, this, [=] {
        handlePersonalizedProtocol();
        handleArmedStateFromCruLinkStatusChanged();
    });

    connect(env->ds.systemData, &DS_SystemData::signalDataChanged_StatePath, this, [=](DS_SystemDef::StatePath statePath) {
        handleArmedStateWhileSyringesBusy();

        if (statePath == DS_SystemDef::STATE_PATH_ERROR)
        {
            DS_ExamDef::ExamProgressState examProgressState = env->ds.examData->getExamProgressState();
            if ( (examProgressState != DS_ExamDef::EXAM_PROGRESS_STATE_IDLE) &&
                 (examProgressState != DS_ExamDef::EXAM_PROGRESS_STATE_COMPLETED) )
            {
                LOG_WARNING("StatePath=%s. Exam is terminated\n", ImrParser::ToImr_StatePath(statePath).CSTR());
                env->ds.examAction->actExamEnd();
                return;
            }
        }
    });

    connect(env->ds.deviceData, &DS_DeviceData::signalDataChanged_FluidSourceSyringes, this, [=](const DS_DeviceDef::FluidSources &fluidSourceSyringes, const DS_DeviceDef::FluidSources &prevFluidSourceSyringes) {
        handleManualPrimeWhileInjectionPreloaded();
    });

    connect(&tmrExamTimeout, &QTimer::timeout, this, [=] {
        if (!env->ds.examData->isExamStarted())
        {
            //LOG_DEBUG("tmrExamTimeout(): Exam is not active\n");
            return;
        }

        int timeoutHours = env->ds.cfgGlobal->get_Configuration_Exam_ExamTimeoutPeriod();
        if (timeoutHours == 0)
        {
            //LOG_DEBUG("tmrExamTimeout(): Timeout Hours Cfg is 0\n");
            return;
        }

        DS_SystemDef::StatePath statePath = env->ds.systemData->getStatePath();
        if (statePath != DS_SystemDef::STATE_PATH_IDLE)
        {
            //LOG_DEBUG("tmrExamTimeout(): StatePath=%s\n", ImrParser::ToImr_StatePath(statePath).CSTR());
            return;
        }

        if (env->ds.alertAction->isActivated("TimeoutExamEnded", "", true))
        {
            //LOG_DEBUG("tmrExamTimeout(): TimeoutExamEnded alert is already active\n");
            return;
        }

        qint64 lastExamOrInjectStartedEpochMs;
        DS_ExamDef::ExecutedSteps executedSteps = env->ds.examData->getExecutedSteps();
        if (executedSteps.length() == 0)
        {
            lastExamOrInjectStartedEpochMs = env->ds.examData->getExamStartedAtEpochMs();
            //LOG_DEBUG("tmrExamTimeout(): lastExamOrInjectStarted = ExamStarted = %s\n", Util::qDateTimeToUtcDateTimeStr(QDateTime::fromMSecsSinceEpoch(lastExamOrInjectStartedEpochMs)).CSTR());
        }
        else
        {
            lastExamOrInjectStartedEpochMs = executedSteps.last().stepCompletedEpochMs;
            //LOG_DEBUG("tmrExamTimeout(): lastExamOrInjectStarted = Injection[%d].Completed = %s\n", executedSteps.length() - 1, Util::qDateTimeToUtcDateTimeStr(QDateTime::fromMSecsSinceEpoch(lastExamOrInjectStartedEpochMs)).CSTR());
        }

        qint64 curTimeEpochMs = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
        qint64 timePastMs = curTimeEpochMs - lastExamOrInjectStartedEpochMs;
        qint64 timePastHours = timePastMs / 3600000;

        if (timePastHours >= timeoutHours)
        {
            QString examGuid = env->ds.examData->getExamGuid();

            LOG_WARNING("tmrExamTimeout(): Exam Timeout Occurred: TimePast=%lldms, %lldhours, limit=%dhours\n", timePastMs, timePastHours, timeoutHours);
            env->ds.examAction->actExamEnd();
            env->ds.alertAction->activate("TimeoutExamEnded", examGuid);
        }
        else
        {
            //LOG_DEBUG("tmrExamTimeout(): Exam Timeout Monitoring..: TimePast=%lldms, %lldhours, limit=%dhours\n", timePastMs, timePastHours, timeoutHours);
        }

    });

    tmrExamTimeout.start(EXAM_TIMEOUT_MONITOR_INTERVAL_MS);
}

void ExamMonitor::handlePersonalizedProtocol()
{
    DS_CruDef::CruLinkStatus cruLinkStatus = env->ds.cruData->getCruLinkStatus();
    if (cruLinkStatus.state == DS_CruDef::CRU_LINK_STATE_INACTIVE)
    {
        DS_ExamDef::InjectionPlan plan = env->ds.examData->getInjectionPlan();
        if (plan.isPersonalized)
        {
            LOG_INFO("handlePersonalizedProtocol(): Plan is personalized but CRU Link is inactive.\n");
            env->ds.alertAction->activate("DeparameterizedProtocol");
        }
    }
}

void ExamMonitor::handleArmedStateFromCruLinkStatusChanged()
{
    DS_CruDef::CruLinkStatus cruLinkStatus = env->ds.cruData->getCruLinkStatus();
    if (cruLinkStatus.state == DS_CruDef::CRU_LINK_STATE_ACTIVE)
    {
        // CRU Link is good
        return;
    }

    DS_ExamDef::ScannerInterlocks scannerInterlocks = env->ds.examData->getScannerInterlocks();
    if (!scannerInterlocks.isIsiActive())
    {
        // Scanner connectivity is not active
        return;
    }

    DS_SystemDef::StatePath statePath = env->ds.systemData->getStatePath();

    if (statePath != DS_SystemDef::STATE_PATH_READY_ARMED)
    {
        // Injection is not ARMED
        return;
    }

    LOG_INFO("handleArmedStateFromCruLinkStatusChanged(): CRU Link is inactive AND Scanner connectivity active AND Armed\n");
    env->ds.examAction->actDisarm();
    env->ds.alertAction->activate("InjectionDisarmedByCommLoss");
}

void ExamMonitor::handleArmedStateWhileSyringesBusy()
{
    DS_SystemDef::StatePath statePath = env->ds.systemData->getStatePath();
    if (statePath == DS_SystemDef::STATE_PATH_READY_ARMED)
    {
        DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
        if ( (fluidSourceSyringes[SYRINGE_IDX_SALINE].isBusy) ||
             (fluidSourceSyringes[SYRINGE_IDX_CONTRAST1].isBusy) ||
             (fluidSourceSyringes[SYRINGE_IDX_CONTRAST2].isBusy) )
        {
            LOG_WARNING("signalDataChanged_FluidSourceSyringes(): Syringes busy while ARMED: Disarming..\n");
            env->ds.examAction->actDisarm();
        }
    }
}


void ExamMonitor::handlePreloadProtocol(const DS_ExamDef::InjectionPlan &prevPlan)
{
    DS_WorkflowDef::PreloadProtocolState preloadProtocolState = env->ds.workflowData->getPreloadProtocolState();
    if (preloadProtocolState != DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_READY)
    {
        // Preload is in progress.
        return;
    }

    if (env->ds.alertAction->isActivated("RepreloadReprimeRequired", "", true))
    {
        // RepreloadReprimeRequired alert is already active
        return;
    }

    DS_SystemDef::StatePath statePath = env->ds.systemData->getStatePath();

    if (statePath != DS_SystemDef::STATE_PATH_IDLE)
    {
        // StatePath is not IDLE. The preloaded injection should only be handled when current injection is programmed (when statePath is IDLE)
        // Note, the step guid may be changed when the injection started.
        return;
    }

    DS_ExamDef::ExecutedSteps executedSteps = env->ds.examData->getExecutedSteps();
    DS_ExamDef::InjectionPlan prevPlanBuf = prevPlan;
    DS_ExamDef::InjectionStep *prevStep = prevPlanBuf.getExecutingStep(statePath, executedSteps);

    DS_ExamDef::ExamProgressState examProgressState = env->ds.examData->getExamProgressState();

    if ( (examProgressState == DS_ExamDef::EXAM_PROGRESS_STATE_IDLE) ||
         (examProgressState == DS_ExamDef::EXAM_PROGRESS_STATE_PREPARED) ||
         (examProgressState == DS_ExamDef::EXAM_PROGRESS_STATE_COMPLETING) ||
         (examProgressState == DS_ExamDef::EXAM_PROGRESS_STATE_COMPLETED) )
    {
        // Exam is not started
        return;
    }

    if ( (prevStep == NULL) ||
         (!prevStep->isPreloaded) )
    {
        // No preload step was programmed
        return;
    }

    // Confirm previous preload step part1 is same as current preload step part1
    DS_ExamDef::InjectionStep prevPreloadStep1 = env->ds.examData->getPreloadedStep1();

    if (prevPreloadStep1.guid == EMPTY_GUID)
    {
        // No step was preloaded
        return;
    }

    const DS_ExamDef::InjectionStep *curStep = env->ds.examData->getExecutingStep();

    QString reprimeRequiredReason = "";

    if (curStep == NULL)
    {
        reprimeRequiredReason = "StepRemoved";
    }
    else if (Util::areFloatVarsEqual(curStep->getFluidTotal(), 0))
    {
        //This condition is to check if the injection only has delay phases.
        reprimeRequiredReason = "FluidPhasesNotPresent";
    }
    else if (curStep->guid != prevStep->guid)
    {
        LOG_WARNING("handlePreloadProtocol(): Step GUID changed from %s to %s\n", prevStep->guid.CSTR(), curStep->guid.CSTR());
        reprimeRequiredReason = "StepRelocated";
    }
    else
    {
        // Prepare new preload step1 & step2
        DS_ExamDef::InjectionStep curStepData = *curStep;
        DS_ExamDef::InjectionStep curPreloadStep1, curPreloadStep2;
        env->ds.examAction->actGetPreloadStepsFromStep(curPreloadStep1, curPreloadStep2, curStepData);

        // Previous preload step part 1 is changed
        prevPreloadStep1.isPreloaded = false;
        curPreloadStep1.isPreloaded = false;

        // Confirm previous preload step part1 is same as current preload step part1
        bool preloadStep1VolumeChanged = false;
        if (prevPreloadStep1.phases.length() != curPreloadStep1.phases.length())
        {
            preloadStep1VolumeChanged = true;
        }
        else
        {
            for (int phaseIdx = 0; phaseIdx < prevPreloadStep1.phases.length(); phaseIdx++)
            {
                const DS_ExamDef::InjectionPhase *prevPhase = &prevPreloadStep1.phases[phaseIdx];
                const DS_ExamDef::InjectionPhase *curPhase = &curPreloadStep1.phases[phaseIdx];

                if ( (prevPhase->type != curPhase->type) ||
                     (prevPhase->contrastPercentage != curPhase->contrastPercentage) ||
                     (prevPhase->totalVol != curPhase->totalVol) )
                {
                    preloadStep1VolumeChanged = true;
                    break;
                }
            }
        }
        // Check if the location changed first, because due to location change, even if the same
        // contrast types are loaded, it results in different preloaded volumes.
        if ( (prevPreloadStep1.getContrastTotal() != 0) &&
             (prevPreloadStep1.contrastFluidLocation != curPreloadStep1.contrastFluidLocation) )
        {
            reprimeRequiredReason = "ContrastFluidLocationChanged";
        }
        else if (preloadStep1VolumeChanged)
        {
            reprimeRequiredReason = "PreloadedInjectionChanged";
        }

        // always Update preload step2
        env->ds.examData->setPreloadedStep2(curPreloadStep2);
    }

    if (reprimeRequiredReason != "")
    {
        LOG_WARNING("handlePreloadProtocol(): Reprime Required: Reason=%s\n", reprimeRequiredReason.CSTR());
        env->ds.alertAction->activate("RepreloadReprimeRequired", reprimeRequiredReason);

        // If reprimeRequiredReason is raised after SUDS has been taken out, update error state so that next insertion of the suds can correctly request prime
        DS_DeviceDef::FluidSource fluidSourceSuds = env->ds.deviceData->getFluidSourceSuds();
        DS_WorkflowDef::WorkflowErrorStatus workflowErrorStatus = env->ds.workflowData->getWorkflowErrorStatus();
        if (!fluidSourceSuds.isInstalled() && (workflowErrorStatus.state == DS_WorkflowDef::WORKFLOW_ERROR_STATE_NORMAL_SUDS_REMOVED_AFTER_FIRST_PRIMED))
        {
            workflowErrorStatus.state = DS_WorkflowDef::WORKFLOW_ERROR_STATE_NORMAL_SUDS_REMOVED_AFTER_FIRST_PRIMED_AND_NEXT_PRIME_FAILED;
            env->ds.workflowData->setWorkflowErrorStatus(workflowErrorStatus);
        }
    }
}


void ExamMonitor::handleManualPrimeWhileInjectionPreloaded()
{
    static double manualPrimedVolTotal = 0;
    static double lastSalineSyringeVol = 0;

    if (env->ds.alertAction->isActivated("SRUFillingMUDS", "", true))
    {
        // Filling in progress
        return;
    }

    DS_DeviceDef::FluidSource fluidSourceSyringe = env->ds.deviceData->getFluidSourceSyringes()[SYRINGE_IDX_SALINE];
    double salineSyringeVol = fluidSourceSyringe.currentVolumesTotal();

    DS_SystemDef::StatePath statePath = env->ds.systemData->getStatePath();
    if (statePath != DS_SystemDef::STATE_PATH_IDLE)
    {
        lastSalineSyringeVol = salineSyringeVol;
        return;
    }

    const DS_ExamDef::InjectionStep *curStep = env->ds.examData->getExecutingStep();
    if ( (curStep == NULL) ||
         (!curStep->isPreloaded) )
    {
        manualPrimedVolTotal = 0;
        lastSalineSyringeVol = salineSyringeVol;
        return;
    }

    bool extendedSUDS = (env->ds.examData->getSUDSLength() == SUDS_LENGTH_EXTENDED);
    double primeRequiredTotalVol = (extendedSUDS ? env->ds.capabilities->get_Preload_SalineVolumeExtended()
                                                 : env->ds.capabilities->get_Preload_SalineVolume());

    if (salineSyringeVol < lastSalineSyringeVol)
    {
        double manualPrimedVol = lastSalineSyringeVol - salineSyringeVol;
        manualPrimedVolTotal += manualPrimedVol;

        LOG_DEBUG("handleManualPrimeWhileInjectionPreloaded(): Manually primed (%.1fml) while injection is preloaded.\n", manualPrimedVolTotal);

        if (manualPrimedVolTotal >= primeRequiredTotalVol)
        {
            // Fully manual primed
            LOG_INFO("handleManualPrimeWhileInjectionPreloaded(): Fully manually primed (%.1fml >= %.1fml) while injection is preloaded. Reset preload steps..\n", manualPrimedVolTotal, primeRequiredTotalVol);
            env->ds.examAction->actResetPreloadSteps();
        }
    }

    lastSalineSyringeVol = salineSyringeVol;
}
