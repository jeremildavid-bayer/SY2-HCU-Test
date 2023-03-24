#include "Apps/AppManager.h"
#include "ExamInjection.h"
#include "Common/ImrParser.h"
#include "DataServices/Exam/DS_ExamData.h"
#include "DataServices/Exam/DS_ExamAction.h"
#include "DataServices/Device/DS_DeviceData.h"
#include "DataServices/Device/DS_DeviceAction.h"
#include "DataServices/Alert/DS_AlertAction.h"
#include "DataServices/Alert/DS_AlertData.h"
#include "DataServices/Mcu/DS_McuData.h"
#include "DataServices/Mcu/DS_McuAction.h"
#include "DataServices/System/DS_SystemData.h"
#include "DataServices/System/DS_SystemAction.h"
#include "DataServices/Workflow/DS_WorkflowAction.h"
#include "DataServices/Workflow/DS_WorkflowData.h"

ExamInjection::ExamInjection(QObject *parent, EnvGlobal *env_) :
    QObject(parent),
    env(env_)
{
    envLocal = new EnvLocal("DS_Exam-Injection", "EXAM_INJECTION", LOG_MID_SIZE_BYTES);

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });
}

ExamInjection::~ExamInjection()
{
    delete envLocal;
}

void ExamInjection::slotAppInitialised()
{
    connect(env->ds.systemData, &DS_SystemData::signalDataChanged_StatePath, this, [=](DS_SystemDef::StatePath curStatePath, DS_SystemDef::StatePath prevStatePath) {
        if ( ( (prevStatePath == DS_SystemDef::STATE_PATH_IDLE) && (curStatePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING) ) ||
             ( (prevStatePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING) && (curStatePath == DS_SystemDef::STATE_PATH_IDLE) ) )
        {
            LOG_INFO("\n\n-----------------\nStatePath=%s\n\n", ImrParser::ToImr_StatePath(curStatePath).CSTR());
        }
    });

    connect(&tmrInjectionProgressUpdate, SIGNAL(timeout()), SLOT(slotMcuDataChanged_InjectionProgress()));

    connect(env->ds.systemData, &DS_SystemData::signalDataChanged_StatePath, this, [=](DS_SystemDef::StatePath curStatePath, DS_SystemDef::StatePath prevStatePath) {
        if (prevStatePath == curStatePath)
        {
            return;
        }

        QString examGuid = env->ds.examData->getExamGuid();
        if ( (curStatePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING) &&
             (env->ds.examData->isExamStarted()) )
        {
            LOG_WARNING("signalDataChanged_StatePath: Service Mode Entered: Exam Ending..\n");
            // Service mode entered. End current exam
            env->ds.examAction->actExamEnd();
            env->ds.alertAction->activate("ServiceModeExamEnded", examGuid);
        }

        // AutoFill if programmed fluid exceeds amount in syringe
        if ( (curStatePath == DS_SystemDef::STATE_PATH_IDLE) &&
             (prevStatePath == DS_SystemDef::STATE_PATH_BUSY_FINISHING) )
        {
            DS_ExamDef::ExecutedSteps executedSteps = env->ds.examData->getExecutedSteps();

            if (executedSteps.length() > 0)
            {
                DS_ExamDef::ExecutedStep stepProgressDigest = executedSteps.last();
                QString mcuTerminatedReason = stepProgressDigest.mcuTerminatedReason;
                mcuTerminatedReason = "COMPLETED_" + mcuTerminatedReason;

                if (mcuTerminatedReason == ImrParser::ToImr_InjectionCompleteReason(DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_MUDS_UNLATCHED))
                {
                    LOG_WARNING("signalDataChanged_StatePath: Injection aborted due to MUDS unlatched. Ending exam..\n");
                    env->ds.examAction->actExamEnd();
                }
            }
            else
            {
                LOG_ERROR("signalDataChanged_StatePath: StatePath changed from IDLE to BusyFinishing but no executedSteps found\n");
            }

            if (env->ds.alertAction->isActivated("OutletAirDetected"))
            {
                LOG_WARNING("signalDataChanged_StatePath: SUDS Recovery should be in progress. Auto-Refill is not started\n");
            }
            else
            {
                env->ds.examAction->actAutoRefill("AfterInjection");
            }
        }
    });

    connect(env->ds.examData, &DS_ExamData::signalDataChanged_ExamProgressState, this, [=](DS_ExamDef::ExamProgressState state) {
       LOG_INFO("ExamProgressState = %s\n", ImrParser::ToImr_ExamProgressState(state).CSTR());
    });

    connect(env->ds.examData, &DS_ExamData::signalDataChanged_InjectionPlan, this, [=](const DS_ExamDef::InjectionPlan &plan, const DS_ExamDef::InjectionPlan &prevPlan) {
        // Update some plan parameters after plan changed
        QTimer::singleShot(1, [=] {
            handleInjectionPlanChanged();
        });

        if ( (plan.getContrastTotal() != prevPlan.getContrastTotal()) ||
             (plan.getSalineTotal() != prevPlan.getSalineTotal()) )
        {
            // If any fluid volumes changed in the plan, trigger the auto refill
            LOG_DEBUG("signalDataChanged_InjectionPlan(): Plan Volume changed (S: %.1fml->%.1fml, C: %.1fml->%.1fml). Refill started\n", prevPlan.getSalineTotal(), plan.getSalineTotal(), prevPlan.getContrastTotal(), plan.getContrastTotal());
            env->ds.examAction->actAutoRefill("InjectionPlanChanged");
        }
        else if (plan.getLastStepContrastLocation() != prevPlan.getLastStepContrastLocation())
        {
            // If the contrast fluid location changed in the plan, trigger the auto refill
            LOG_DEBUG("signalDataChanged_InjectionPlan(): Last step contrast location changed (%d->%d). Refill started\n", prevPlan.getLastStepContrastLocation(), plan.getLastStepContrastLocation());
            env->ds.examAction->actAutoRefill("PlanContrastLocationChanged");
        }
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_StopcockPosAll, this, [=] {
        handleInjectDigest();
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_InjectDigest, this, [=] {
        handleInjectDigest();
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_InjectorStatus, this, [=](DS_McuDef::InjectorStatus curProgress, DS_McuDef::InjectorStatus prevProgress) {
        DS_WorkflowDef::PreloadProtocolState preloadProtocolState = env->ds.workflowData->getPreloadProtocolState();
        if (preloadProtocolState != DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_READY)
        {
            // Preload is in progress
            return;
        }

        // Handle injection progress
        slotMcuDataChanged_InjectionProgress();

        // Activate/deactive alert
        if ( (prevProgress.state != DS_McuDef::INJECTOR_STATE_HOLDING) &&
             (curProgress.state == DS_McuDef::INJECTOR_STATE_HOLDING) )
        {
            env->ds.alertAction->activate("InjectionAsyncHold");
        }

        if ( (prevProgress.state == DS_McuDef::INJECTOR_STATE_HOLDING) &&
             (curProgress.state != DS_McuDef::INJECTOR_STATE_HOLDING) )
        {
            env->ds.alertAction->deactivate("InjectionAsyncHold");
        }

        if ( (prevProgress.state == DS_McuDef::INJECTOR_STATE_IDLE) &&
             (curProgress.state == DS_McuDef::INJECTOR_STATE_READY_START) )
        {
            const DS_ExamDef::InjectionStep *executingStep = env->ds.examData->getExecutingStep();
            if (executingStep != NULL)
            {
                env->ds.alertAction->activate("InjectionArmed", Util::qVarientToJsonData(ImrParser::ToImr_InjectionStep(*executingStep)));
            }

            // Deactivate Alert CatheterLimitsExceededAccepted so that it can be shown next time the injector is Armed, if applicable.
            if (env->ds.alertAction->isActivated("CatheterLimitsExceededAccepted", "", true))
            {
                env->ds.alertAction->deactivate("CatheterLimitsExceededAccepted");
            }

            // Deactiviate the InsufficientVolumeForStepsAccepted alert so that it can be shown next time the injector is Armed, if applicable.
            if (env->ds.alertAction->isActivated("InsufficientVolumeForStepsAccepted", "", true))
            {
                env->ds.alertAction->deactivate("InsufficientVolumeForStepsAccepted");
            }
        }

        if ( (prevProgress.state == DS_McuDef::INJECTOR_STATE_READY_START) &&
             ( (curProgress.state == DS_McuDef::INJECTOR_STATE_DELIVERING) ||
               (curProgress.state == DS_McuDef::INJECTOR_STATE_PHASE_PAUSED) ) )
        {
            env->ds.alertAction->activate("InjectionStarted");
        }

        if ( ( (prevProgress.state == DS_McuDef::INJECTOR_STATE_DELIVERING) ||
               (prevProgress.state == DS_McuDef::INJECTOR_STATE_HOLDING) ||
               (prevProgress.state == DS_McuDef::INJECTOR_STATE_PHASE_PAUSED) ||
               (prevProgress.state == DS_McuDef::INJECTOR_STATE_HOLDING) ) &&
             (curProgress.state == DS_McuDef::INJECTOR_STATE_COMPLETING) )
        {
            DS_ExamDef::ExecutedSteps executedSteps = env->ds.examData->getExecutedSteps();

            if (executedSteps.length() == 0)
            {
                QString err = QString().asprintf("Bad Executed Step data while Injection Progress changed. InjectorState=%s->%s\n", ImrParser::ToImr_InjectorState(prevProgress.state).CSTR(), ImrParser::ToImr_InjectorState(curProgress.state).CSTR());
                LOG_ERROR("%s", err.CSTR());
                env->ds.alertAction->activate("HCUInternalSoftwareError", err);
            }
            else
            {
                DS_SystemDef::StatePath curStatePath = env->ds.systemData->getStatePath();
                const DS_ExamDef::ExecutedStep stepProgressDigest = executedSteps.last();
                QString examGuid = env->ds.examData->getExamGuid();
                DS_ExamDef::InjectionPlan injectionPlan = env->ds.examData->getInjectionPlan();
                DS_ExamDef::InjectionStep *executingStep = injectionPlan.getExecutingStep(curStatePath, executedSteps);


                if (stepProgressDigest.terminationReason != DS_ExamDef::STEP_TERMINATION_REASON_NORMAL)
                {
                    // Injection aborted. Raise InjectionAborted alert
                    QString alertData = QString().asprintf("%s;%s;%s;%s;%s;%s", examGuid.CSTR(), injectionPlan.guid.CSTR(), executingStep->guid.CSTR(), ImrParser::ToImr_StepTerminationReason(stepProgressDigest.terminationReason).CSTR(), stepProgressDigest.mcuTerminatedReason.CSTR(), stepProgressDigest.terminatedReasonMessage.CSTR());
                    env->ds.alertAction->activate("InjectionAborted", alertData);
                }

                QVariantMap alertData;
                alertData.insert("ProgrammedPlan", ImrParser::ToImr_InjectionPlan(injectionPlan));
                alertData.insert("StepProgress", ImrParser::ToImr_ExecutedStep(stepProgressDigest));
                alertData.insert("CurrentExamGuid", examGuid);
                alertData.insert("FluidSources", ImrParser::ToImr_FluidSourceSyringes(env->ds.deviceData->getFluidSourceSyringes()));
                env->ds.alertAction->activate("InjectionStopped", Util::qVarientToJsonData(alertData));
            }
        }
    });

    connect(env->ds.alertData, &DS_AlertData::signalDataChanged_ActiveFatalAlerts, this, [=](const QVariantList &activeFatalAlerts) {
        if (activeFatalAlerts.length() == 0)
        {
            return;
        }

        // Abort injection if fatal alert is raised during injection
        DS_SystemDef::StatePath statePath = env->ds.systemData->getStatePath();
        if ( (statePath == DS_SystemDef::STATE_PATH_EXECUTING) ||
             (statePath == DS_SystemDef::STATE_PATH_BUSY_HOLDING) )
        {
            LOG_ERROR("Fatal Alerts(%s) occurred during injection\n", Util::qVarientToJsonData(activeFatalAlerts).CSTR());
            DS_ExamDef::ExecutedSteps executedSteps = env->ds.examData->getExecutedSteps();
            if (executedSteps.length() == 0)
            {
                QString err = QString().asprintf("Cannot get current stepProgress digest.\n");
                LOG_ERROR("%s", err.CSTR());
                env->ds.alertAction->activate("HCUInternalSoftwareError", err);
                return;
            }

            DS_ExamDef::ExecutedStep stepProgressDigest = executedSteps.last();

            if (stepProgressDigest.terminationReason == DS_ExamDef::STEP_TERMINATION_REASON_INVALID)
            {
                LOG_WARNING("Termination Reason is not set yet. Updating termination reason to '%s' and aborting the injection. activeFatalAlerts=%s\n",
                         ImrParser::ToImr_StepTerminationReason(DS_ExamDef::STEP_TERMINATION_REASON_ABORTED_BY_HARDWARE_FAULT).CSTR(),
                         Util::qVarientToJsonData(activeFatalAlerts).CSTR());

                // Setting injection temination reason if not set already
                stepProgressDigest.terminationReason = DS_ExamDef::STEP_TERMINATION_REASON_ABORTED_BY_HARDWARE_FAULT;
                stepProgressDigest.terminatedReasonMessage = Util::qVarientToJsonData(activeFatalAlerts);
                executedSteps[executedSteps.length() - 1] = stepProgressDigest;
                env->ds.examData->setExecutedSteps(executedSteps);
                env->ds.examAction->actInjectionStop();
            }
        }
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_LinkState, this, [=](DS_McuDef::LinkState linkState) {
        // Abort injection if MCU comm is down during injection
        DS_SystemDef::StatePath statePath = env->ds.systemData->getStatePath();
        if ( (statePath == DS_SystemDef::STATE_PATH_EXECUTING) ||
             (statePath == DS_SystemDef::STATE_PATH_BUSY_HOLDING) )
        {
            if (linkState != DS_McuDef::LINK_STATE_CONNECTED)
            {
                LOG_ERROR("MCU comm down during injection. Aborting injection..\n");

                DS_ExamDef::ExecutedSteps executedSteps = env->ds.examData->getExecutedSteps();
                if (executedSteps.length() == 0)
                {
                    QString err = QString().asprintf("Cannot get current stepProgress digest.\n");
                    LOG_ERROR("%s", err.CSTR());
                    env->ds.alertAction->activate("HCUInternalSoftwareError", err);
                    return;
                }

                DS_ExamDef::ExecutedStep stepProgressDigest = executedSteps.last();
                stepProgressDigest.terminationReason = DS_ExamDef::STEP_TERMINATION_REASON_ABORTED_BY_INJECTOR_CRITICAL_ERROR;
                stepProgressDigest.terminatedReasonMessage = "MCU COMM DOWN";
                executedSteps[executedSteps.length() - 1] = stepProgressDigest;
                env->ds.examData->setExecutedSteps(executedSteps);

                QVariantMap alertData;
                alertData.insert("ProgrammedPlan", ImrParser::ToImr_InjectionPlan(env->ds.examData->getInjectionPlan()));
                alertData.insert("StepProgress", ImrParser::ToImr_ExecutedStep(stepProgressDigest));
                env->ds.alertAction->activate("InjectionStopped", Util::qVarientToJsonData(alertData));

                env->ds.systemAction->actSetStatePath(DS_SystemDef::STATE_PATH_ERROR);
            }
        }
    });
}

void ExamInjection::handleInjectDigest()
{
    DS_WorkflowDef::PreloadProtocolState preloadProtocolState = env->ds.workflowData->getPreloadProtocolState();
    if (preloadProtocolState != DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_READY)
    {
        // Preload is in progress
        return;
    }

    // Check Stopcock Status
    DS_McuDef::InjectDigest injectDigest = env->ds.mcuData->getInjectDigest();
    for (int syringeIdx = 0; syringeIdx < injectDigest.syringeInjectDigests.length(); syringeIdx++)
    {
        DS_McuDef::StopcockPos scPos = injectDigest.syringeInjectDigests[syringeIdx].scPos;
        if ( (scPos != DS_McuDef::STOPCOCK_POS_INJECT) &&
             (scPos != DS_McuDef::STOPCOCK_POS_CLOSED) )
        {
            //LOG_DEBUG("HandleInjectDigest: SC[%d]: Unexpected position=%s. Exiting..\n", syringeIdx, ImrParser::ToImr_StopcockPos(stopcockPosAll[syringeIdx]).CSTR());
            return;
        }
    }

    // Check McuProgress
    DS_McuDef::InjectorStatus mcuProgress = env->ds.mcuData->getInjectorStatus();

    if ( (mcuProgress.state != DS_McuDef::INJECTOR_STATE_DELIVERING) &&
         (mcuProgress.state != DS_McuDef::INJECTOR_STATE_HOLDING) &&
         (mcuProgress.state != DS_McuDef::INJECTOR_STATE_PHASE_PAUSED) &&
         (mcuProgress.state != DS_McuDef::INJECTOR_STATE_COMPLETING) )
    {
        LOG_INFO("handleInjectDigest(): Unexpected MCU State(%s). Exiting\n", ImrParser::ToImr_InjectorState(mcuProgress.state).CSTR());
        return;
    }

    // Update InjectionAdaptiveFlow Alert
    bool isPressureLimiting = ( (injectDigest.adaptiveFlowState == DS_McuDef::ADAPTIVE_FLOW_STATE_ACTIVE) ||
                                (injectDigest.adaptiveFlowState == DS_McuDef::ADAPTIVE_FLOW_STATE_CRITICAL) );

    bool adaptiveFlowIsActive = env->ds.alertAction->isActivated("InjectionAdaptiveFlow");
    if (isPressureLimiting)
    {
        if (!adaptiveFlowIsActive)
        {
            env->ds.alertAction->activate("InjectionAdaptiveFlow");
            LOG_WARNING("Adaptive Flow (Pressure Limiting) Activated\n");
        }
    }
    else
    {
        if (adaptiveFlowIsActive)
        {
            env->ds.alertAction->deactivate("InjectionAdaptiveFlow");
            LOG_INFO("Adaptive Flow (Pressure Limiting) Deactivated\n");
        }
    }

    DS_McuDef::InjectionProtocol mcuInjProtocol = env->ds.mcuData->getInjectionProtocol();
    const DS_McuDef::InjectionPhase *mcuInjPhase = &mcuInjProtocol.phases[injectDigest.phaseIdx];

    // LIE_FEATURE: Only set FlowRate if the stopcock position is at inject position.
    double monitoredFlowRate = 0;
    for (int syringeIdx = 0; syringeIdx < injectDigest.syringeInjectDigests.length(); syringeIdx++)
    {
        DS_McuDef::SyringeInjectDigest &syringeInjectDigest = injectDigest.syringeInjectDigests[syringeIdx];
        syringeInjectDigest.flowRate = (syringeInjectDigest.scPos == DS_McuDef::STOPCOCK_POS_INJECT) ? syringeInjectDigest.flowRate : 0;
        monitoredFlowRate += syringeInjectDigest.flowRate;
    }

    // LIE_FEATURE: If reported FlowRate is too low AND Is NOT PressureLimiting, adjust to programmed level.
    bool flowRateTooLow = false;

    if (monitoredFlowRate < 0)
    {
        // Make sure the flow rate is (>= 0). Just for cover up the pressure relief case.
        monitoredFlowRate = 0;
        flowRateTooLow = true;
    }
    else if ( (!isPressureLimiting) &&
              (monitoredFlowRate > 0) &&
              (Util::isFloatVarGreaterThan(mcuInjPhase->flow, monitoredFlowRate)) )
    {
        flowRateTooLow = true;
    }

    if (flowRateTooLow)
    {
        // Get upFactor so each syringe's flow rate is increased by upFactor
        double upFactor = ((mcuInjPhase->flow - monitoredFlowRate) / monitoredFlowRate) + 1;

        for (int syringeIdx = 0; syringeIdx < injectDigest.syringeInjectDigests.length(); syringeIdx++)
        {
            injectDigest.syringeInjectDigests[syringeIdx].flowRate = injectDigest.syringeInjectDigests[syringeIdx].flowRate * upFactor;
        }

        LOG_WARNING("HandleInjectDigest: Delivered flow rate is too low, %.1f < %.1f (programmed). Adjusted to %.1f\n",
                    monitoredFlowRate, mcuInjPhase->flow, mcuInjPhase->flow);
    }

    if ( (mcuInjPhase->type == DS_McuDef::INJECTION_PHASE_TYPE_CONTRAST1) ||
         (mcuInjPhase->type == DS_McuDef::INJECTION_PHASE_TYPE_CONTRAST2) ||
         (mcuInjPhase->type == DS_McuDef::INJECTION_PHASE_TYPE_DUAL1) ||
         (mcuInjPhase->type == DS_McuDef::INJECTION_PHASE_TYPE_DUAL2) ||
         (mcuInjPhase->type == DS_McuDef::INJECTION_PHASE_TYPE_SALINE) )
    {
        // Adjust Injected Volumes if required
        DS_McuDef::PhaseInjectDigest *phaseInjectDigest = &injectDigest.phaseInjectDigests[injectDigest.phaseIdx];

        if ( (mcuInjPhase->type == DS_McuDef::INJECTION_PHASE_TYPE_CONTRAST1) ||
             (mcuInjPhase->type == DS_McuDef::INJECTION_PHASE_TYPE_CONTRAST2) ||
             (mcuInjPhase->type == DS_McuDef::INJECTION_PHASE_TYPE_DUAL1) ||
             (mcuInjPhase->type == DS_McuDef::INJECTION_PHASE_TYPE_DUAL2) )
        {
            // LIE_FEATURE: Delivered contrast volume shall be trimmed to the programmed volume
            SyringeIdx contrastSyringe = ( (mcuInjPhase->type == DS_McuDef::INJECTION_PHASE_TYPE_CONTRAST1) || (mcuInjPhase->type == DS_McuDef::INJECTION_PHASE_TYPE_DUAL1) ) ? SYRINGE_IDX_CONTRAST1 : SYRINGE_IDX_CONTRAST2;

            double contrastProgrammedVol = mcuInjPhase->vol * mcuInjPhase->mix * 0.01;
            double contrastDeliveredVol = phaseInjectDigest->volumes[contrastSyringe];

            if (Util::isFloatVarGreaterThan(contrastDeliveredVol, contrastProgrammedVol, 1))
            {
                LOG_WARNING("HandleInjectDigest(): Delivered Contrast Volume is too high, %.1f > %.1f(programmed). Adjusting to Programmed\n", contrastDeliveredVol, contrastProgrammedVol);
                phaseInjectDigest->volumes[contrastSyringe] = contrastProgrammedVol;
            }
        }

        if ( (mcuInjPhase->type == DS_McuDef::INJECTION_PHASE_TYPE_SALINE) ||
             (mcuInjPhase->type == DS_McuDef::INJECTION_PHASE_TYPE_DUAL1) ||
             (mcuInjPhase->type == DS_McuDef::INJECTION_PHASE_TYPE_DUAL2) )
        {
            // LIE_FEATURE: Delivered saline volume shall be trimmed except the current phase is (Dual Phase && Pulsing Occurred)
            double salineProgrammedVol = mcuInjPhase->vol * (100 - mcuInjPhase->mix) * 0.01;
            double salineDeliveredVol = phaseInjectDigest->volumes[SYRINGE_IDX_SALINE];

            if (Util::isFloatVarGreaterThan(salineDeliveredVol, salineProgrammedVol, 1))
            {
                if (phaseInjectDigest->pulsingActivated)
                {
                    LOG_WARNING("HandleInjectDigest(): Delivered Saline Volume is too high, %.1f > %.1f(programmed). Pulsing Activated, NOT Adjusting to Programmed\n", salineDeliveredVol, salineProgrammedVol);
                }
                else
                {
                    LOG_WARNING("HandleInjectDigest(): Delivered Saline Volume is too high, %.1f > %.1f(programmed). Adjusting to Programmed\n", salineDeliveredVol, salineProgrammedVol);
                    phaseInjectDigest->volumes[SYRINGE_IDX_SALINE] = salineProgrammedVol;
                }
            }
        }
    }

    // Prevent unwanted dataChanged signal emission. Note, current function (handleInjectDigest()) is triggered when injectDigest changed.
    env->ds.mcuData->setDataLocked(true);
    env->ds.mcuData->setInjectDigest(injectDigest);
    env->ds.mcuData->setDataLocked(false);

    slotMcuDataChanged_InjectionProgress();
}

void ExamInjection::slotMcuDataChanged_InjectionProgress()
{
    tmrInjectionProgressUpdate.stop();

    DS_WorkflowDef::PreloadProtocolState preloadProtocolState = env->ds.workflowData->getPreloadProtocolState();
    if (preloadProtocolState != DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_READY)
    {
        // Preload is in progress
        return;
    }

    DS_SystemDef::StatePath curStatePath = env->ds.systemData->getStatePath();

    if ( (curStatePath == DS_SystemDef::STATE_PATH_OFF_UNREACHABLE) ||
         (curStatePath == DS_SystemDef::STATE_PATH_ON_REACHABLE) ||
         (curStatePath == DS_SystemDef::STATE_PATH_STARTUP_UNKNOWN) ||
         (curStatePath == DS_SystemDef::STATE_PATH_ERROR) ||
         (curStatePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING) ||
         (curStatePath == DS_SystemDef::STATE_PATH_SERVICING) )
    {
        // Bad StatePath
        LOG_DEBUG("INJ_PROGRESS: Unexpected StatePath(%s)\n", ImrParser::ToImr_StatePath(curStatePath).CSTR());
        return;
    }

    DS_McuDef::InjectorStatus mcuProgress = env->ds.mcuData->getInjectorStatus();
    DS_SystemDef::StatePath newStatePath = getStatePathFromMcuInjectorState(mcuProgress.state);
    DS_ExamDef::InjectionPlan injectionPlan = env->ds.examData->getInjectionPlan();
    DS_ExamDef::ExecutedSteps executedSteps = env->ds.examData->getExecutedSteps();
    DS_ExamDef::InjectionStep *executingStep = injectionPlan.getExecutingStep(curStatePath, executedSteps);
    DS_ExamDef::ExecutedStep stepProgressDigest;
    DS_ExamDef::ExamProgressState examProgressState = env->ds.examData->getExamProgressState();

    // Debounce is required to ensure the IsFinishitng state never missed
    static int isFinishingStateDebounceCount = 0;

    if (executedSteps.length() > 0)
    {
        stepProgressDigest = executedSteps.last();
    }

    if (curStatePath != newStatePath)
    {
        LOG_INFO("INJ_PROGRESS: StatePath will change from '%s' to '%s'\n", ImrParser::ToImr_StatePath(curStatePath).CSTR(), ImrParser::ToImr_StatePath(newStatePath).CSTR());
    }

    // ------------------------------------------------
    // Handle Unexpected State Change
    if ( (curStatePath == DS_SystemDef::STATE_PATH_EXECUTING) &&
         (newStatePath == DS_SystemDef::STATE_PATH_IDLE) )
    {
        newStatePath = DS_SystemDef::STATE_PATH_BUSY_FINISHING;
        LOG_WARNING("INJ_PROGRESS: Unexpected StatePath Changed. Setting current path to %s\n", ImrParser::ToImr_StatePath(newStatePath).CSTR());
    }

    // ------------------------------------------------
    // Handle INJECTION DISARMED State
    if ( (curStatePath == DS_SystemDef::STATE_PATH_READY_ARMED) &&
         (newStatePath == DS_SystemDef::STATE_PATH_IDLE) )
    {
        LOG_INFO("INJ_PROGRESS: Injection DISARMED\n");
        handleDisarmedState();
    }

    // ------------------------------------------------
    // Handle INJECTION ARMED State
    if ( (curStatePath == DS_SystemDef::STATE_PATH_IDLE) &&
         (newStatePath == DS_SystemDef::STATE_PATH_READY_ARMED) )
    {
        if (executingStep == NULL)
        {
            QString err = QString().asprintf("INJ_PROGRESS: Failed to get ExecutingStep during ARM\n");
            LOG_ERROR("%s", err.CSTR());
            env->ds.alertAction->activate("HCUInternalSoftwareError", err);
            return;
        }

        LOG_INFO("INJ_PROGRESS: Injection ARMED\n");

        executingStep->programmedAtEpochMs = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
        examProgressState = DS_ExamDef::EXAM_PROGRESS_STATE_INJECTION_EXECUTION;
        isFinishingStateDebounceCount = 0;
    }

    // ------------------------------------------------
    // Handle INJECTION STARTED State
    if ( (curStatePath == DS_SystemDef::STATE_PATH_READY_ARMED) &&
         ( (newStatePath == DS_SystemDef::STATE_PATH_EXECUTING) || (newStatePath == DS_SystemDef::STATE_PATH_BUSY_FINISHING) ) )
    {
        QList<double> syringeVols = env->ds.mcuData->getSyringeVols();
        DS_McuDef::InjectionProtocol mcuInjProtocol = env->ds.mcuData->getInjectionProtocol();
        LOG_INFO("INJ_PROGRESS: Injection STARTED: SyringeVols=%s, McuProtocol=%s\n",
                 Util::qVarientToJsonData(ImrParser::ToImr_DoubleList(syringeVols)).CSTR(),
                 Util::qVarientToJsonData(ImrParser::ToImr_McuInjectionProtocol(mcuInjProtocol), false).CSTR());

        // Injection is started
        stepProgressDigest.init();
        stepProgressDigest.stepTriggeredEpochMs = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
        stepProgressDigest.terminationReason = DS_ExamDef::STEP_TERMINATION_REASON_INVALID;

        if (executingStep == NULL)
        {
            QString err = QString().asprintf("INJ_PROGRESS: Failed to get ExecutingStep executing after armed\n");
            LOG_ERROR("%s", err.CSTR());
            env->ds.alertAction->activate("HCUInternalSoftwareError", err);
        }
        else
        {
            stepProgressDigest.programmedStep = executingStep->guid;
            stepProgressDigest.stepIndex = executingStep->getIndex(injectionPlan);
            stepProgressDigest.activeContrastLocation = executingStep->contrastFluidLocation;
        }

        executedSteps.append(stepProgressDigest);
    }

    // ------------------------------------------------
    // Handle INJECTION PAUSED State
    if ( (curStatePath == DS_SystemDef::STATE_PATH_EXECUTING) &&
         (newStatePath == DS_SystemDef::STATE_PATH_BUSY_HOLDING) )
    {
        // User Pause during injection
    }

    // ------------------------------------------------
    // Handle INJECTION RESUMED State
    if ( (curStatePath == DS_SystemDef::STATE_PATH_BUSY_HOLDING) &&
         (newStatePath == DS_SystemDef::STATE_PATH_EXECUTING) )
    {
        // Resumed from User Pause
    }

    // ------------------------------------------------
    // Handle INJECTION EXECUTING State
    if ( (newStatePath == DS_SystemDef::STATE_PATH_EXECUTING) ||
         (newStatePath == DS_SystemDef::STATE_PATH_BUSY_HOLDING) ||
         (newStatePath == DS_SystemDef::STATE_PATH_BUSY_FINISHING) )
    {
        //LOG_DEBUG("INJ_PROGRESS: Injection EXECUTING OR BUSY_HOLDING\n");
        DS_ExamDef::InjectionStep *executingStep = injectionPlan.getExecutingStep(DS_SystemDef::STATE_PATH_EXECUTING, executedSteps);

        if (executingStep == NULL)
        {
            QString err = QString().asprintf("INJ_PROGRESS: Failed to get ExecutingStep while executing\n");
            LOG_ERROR("%s", err.CSTR());
            env->ds.alertAction->activate("HCUInternalSoftwareError", err);
            return;
        }

        int curPhaseIdx = stepProgressDigest.phaseIndex;

        if ( (curPhaseIdx < 0) &&
             (curPhaseIdx >= executingStep->phases.length()) )
        {
            QString err = QString().asprintf("INJ_PROGRESS: Bad Phase Index(%d)\n", curPhaseIdx);
            LOG_ERROR("%s", err.CSTR());
            env->ds.alertAction->activate("HCUInternalSoftwareError", err);
            return;
        }

        DS_McuDef::InjectDigest injectDigest = env->ds.mcuData->getInjectDigest();

        int newPhaseIdx;
        env->ds.examAction->actGetHcuPhaseIdxFromMcuPhaseIdx(newPhaseIdx, injectDigest.phaseIdx);

        if (curPhaseIdx != newPhaseIdx)
        {
            // Phase is changed
            LOG_INFO("INJ_PROGRESS: ===== McuPhaseIdx=%d, Phase Transition %d -> %d\n", injectDigest.phaseIdx, curPhaseIdx, newPhaseIdx);

            // Check any phase has been missed
            int expectedNextPhaseIndex = stepProgressDigest.phaseIndex + 1;

            while (expectedNextPhaseIndex < newPhaseIdx)
            {
                // Prepare new phase progress digest for the missed phases
                DS_ExamDef::PhaseProgressDigest missedPhaseProgressDigest;

                //If there isn't a phase progress digest for this index, add it
                if (expectedNextPhaseIndex >= stepProgressDigest.phaseProgress.length())
                {
                    LOG_WARNING("INJ_PROGRESS: No PhaseProgressDigest available for phaseIndex(%d). Adding a PhaseProgressDigeset..\n", expectedNextPhaseIndex);
                    stepProgressDigest.phaseProgress.append(missedPhaseProgressDigest);
                }

                // Get the missed phase progress digest
                missedPhaseProgressDigest = stepProgressDigest.phaseProgress[expectedNextPhaseIndex];

                if (missedPhaseProgressDigest.state == DS_ExamDef::INJECTION_PHASE_PROGRESS_STATE_JUMPED)
                {
                    LOG_WARNING("INJ_PROGRESS: Phase[%d] was skipped totally. Creating expected progress (0%%).\n", expectedNextPhaseIndex);
                    missedPhaseProgressDigest.progress = 0;
                }
                else
                {
                    LOG_WARNING("INJ_PROGRESS: Phase[%d] progress was not observed. Creating expected progress (100%%)..\n", expectedNextPhaseIndex);
					missedPhaseProgressDigest = getPhaseProgressDigest(stepProgressDigest, executingStep->phases[expectedNextPhaseIndex], expectedNextPhaseIndex);
                    missedPhaseProgressDigest.state = DS_ExamDef::INJECTION_PHASE_PROGRESS_STATE_COMPLETED;
                    missedPhaseProgressDigest.progress = 100;
                }

                stepProgressDigest.phaseProgress[expectedNextPhaseIndex] = missedPhaseProgressDigest;
                LOG_DEBUG("INJ_PROGRESS: Phase[%d]: %s\n", expectedNextPhaseIndex, Util::qVarientToJsonData(ImrParser::ToImr_PhaseProgressDigest(missedPhaseProgressDigest)).CSTR());

                expectedNextPhaseIndex++;
            }

            // Set the last phase progress after we added progress for all the missed phases
            setLastPhaseProgress(stepProgressDigest, executingStep);
            curPhaseIdx = newPhaseIdx;

            if (expectedNextPhaseIndex >= stepProgressDigest.phaseProgress.length())
            {
                // Prepare new phase progress digest
                DS_ExamDef::PhaseProgressDigest phaseProgressDigest;

                // update current phase index
                stepProgressDigest.phaseIndex = newPhaseIdx;
                stepProgressDigest.userHoldStartedAtEpochMs = 0;
                stepProgressDigest.userHoldPerformedMs = 0;
                stepProgressDigest.preventingBackflowSaline = false;

                // Add new phaseProgressDigest
                phaseProgressDigest.state = DS_ExamDef::INJECTION_PHASE_PROGRESS_STATE_PROGRESS;
                stepProgressDigest.phaseProgress.append(phaseProgressDigest);
                LOG_INFO("INJ_PROGRESS: stepProgressDigest.phaseProgress.length=%d\n", (int)stepProgressDigest.phaseProgress.length());
            }
            else
            {
                LOG_WARNING("INJ_PROGRESS: Unexpected stepProgressDigest size (phaseIdx=%d < digest.size=%d). Possibly, phase progressed while PhaseJump is processing.\n", expectedNextPhaseIndex, (int)stepProgressDigest.phaseProgress.length());
            }
        }

        if ( (newStatePath == DS_SystemDef::STATE_PATH_BUSY_HOLDING) &&
             (curStatePath != DS_SystemDef::STATE_PATH_BUSY_HOLDING) )
        {
            LOG_INFO("INJ_PROGRESS: USER HOLD started\n");
            stepProgressDigest.userHoldStartedAtEpochMs = stepProgressDigest.phaseProgress[curPhaseIdx].elapsedMillisFromPhaseStart - stepProgressDigest.userHoldPerformedMs;
        }

        if (newStatePath == DS_SystemDef::STATE_PATH_BUSY_HOLDING)
        {
            stepProgressDigest.userHoldPerformedMs = stepProgressDigest.phaseProgress[curPhaseIdx].elapsedMillisFromPhaseStart - stepProgressDigest.userHoldStartedAtEpochMs;
        }

        if ( (newStatePath == DS_SystemDef::STATE_PATH_BUSY_FINISHING) &&
             (stepProgressDigest.phaseProgress[curPhaseIdx].state == DS_ExamDef::INJECTION_PHASE_PROGRESS_STATE_PROGRESS) )
        {
            // Mark current phase state as completed
            /*LOG_DEBUG("INJ_PROGRESS: Phase[%d]: StatePath is '%s' but the state is still %s. Setting the state to '%s'\n",
                        curPhaseIdx,
                        ImrParser::ToImr_StatePath(newStatePath).CSTR(),
                        ImrParser::ToImr_InjectionPhaseProgressState(stepProgressDigest.phaseProgress[curPhaseIdx].state).CSTR(),
                        ImrParser::ToImr_InjectionPhaseProgressState(DS_ExamDef::INJECTION_PHASE_PROGRESS_STATE_COMPLETED).CSTR());*/
            stepProgressDigest.phaseProgress[curPhaseIdx].state = DS_ExamDef::INJECTION_PHASE_PROGRESS_STATE_COMPLETED;
        }

        if (curPhaseIdx > 0)
        {
            // Update previous phaseProgress: Phase transition may happened too quickly, phaseProgress is not updated during that time.
            // Don't touch the last phase progress if it was skipped
            DS_ExamDef::PhaseProgressDigest lastPhaseProgress = getPhaseProgressDigest(stepProgressDigest, executingStep->phases[curPhaseIdx - 1], curPhaseIdx - 1);
            if ( (lastPhaseProgress != stepProgressDigest.phaseProgress[curPhaseIdx - 1]) &&
                 (lastPhaseProgress.state != DS_ExamDef::INJECTION_PHASE_PROGRESS_STATE_JUMPED) )
            {
                stepProgressDigest.phaseProgress[curPhaseIdx - 1] = lastPhaseProgress;
                LOG_DEBUG("INJ_PROGRESS: Phase[%d]: %s\n", curPhaseIdx - 1, Util::qVarientToJsonData(ImrParser::ToImr_PhaseProgressDigest(stepProgressDigest.phaseProgress[curPhaseIdx - 1])).CSTR());
            }
        }


        // Update AdaptiveFlowState
        switch (injectDigest.adaptiveFlowState)
        {
        case DS_McuDef::ADAPTIVE_FLOW_STATE_ACTIVE:
            stepProgressDigest.adaptiveFlowState = DS_ExamDef::ADAPTIVE_FLOW_STATE_ACTIVE;
            break;
        case DS_McuDef::ADAPTIVE_FLOW_STATE_CRITICAL:
            stepProgressDigest.adaptiveFlowState = DS_ExamDef::ADAPTIVE_FLOW_STATE_CRITICAL;
            break;
        case DS_McuDef::ADAPTIVE_FLOW_STATE_OFF:
        default:
            stepProgressDigest.adaptiveFlowState = DS_ExamDef::ADAPTIVE_FLOW_STATE_OFF;
            break;
        }

        // Update HasPressureLimited
        bool isPressureLimiting = ( (injectDigest.adaptiveFlowState == DS_McuDef::ADAPTIVE_FLOW_STATE_ACTIVE) ||
                                    (injectDigest.adaptiveFlowState == DS_McuDef::ADAPTIVE_FLOW_STATE_CRITICAL) );

        if ( (isPressureLimiting) &&
             (!stepProgressDigest.hasPressureLimited) )
        {
            LOG_WARNING("INJ_PROGRESS(): Has Pressure Limited\n");
            stepProgressDigest.hasPressureLimited = true;
        }

        // Update HasPreventedBackflowSaline
        if ( (stepProgressDigest.preventingBackflowSaline) &&
             (!stepProgressDigest.hasPreventedBackflowSaline) )
        {
            LOG_WARNING("INJ_PROGRESS(): Has Prevented Backflow Saline\n");
            stepProgressDigest.hasPreventedBackflowSaline = true;
        }

        for (int syringeIdx = 0; syringeIdx < injectDigest.syringeInjectDigests.length(); syringeIdx++)
        {
            stepProgressDigest.instantaneousRates.flowRates[syringeIdx] = injectDigest.syringeInjectDigests[syringeIdx].flowRate;
        }

        // Update preventingBackflowSaline
        const DS_McuDef::PhaseInjectDigest *phaseInjectDigest = &injectDigest.phaseInjectDigests[injectDigest.phaseIdx];
        if (phaseInjectDigest->pulsingActivated)
        {
            if (!stepProgressDigest.preventingBackflowSaline)
            {
                //LOG_WARNING("HandleInjectDigest(): Delivered Saline Volume is too high, %.1f > %.1f(programmed). Preventing Backflow Saline. NOT Adjusting to Programmed\n", salineDeliveredVol, salineProgrammedVol);
                LOG_WARNING("INJ_PROGRESS(): preventingBackflowSaline = TRUE\n");
                stepProgressDigest.preventingBackflowSaline = true;
            }
        }

        // Update current phaseProgress
        if (curPhaseIdx < executingStep->phases.length())
        {
            DS_ExamDef::PhaseProgressDigest phaseProgress = getPhaseProgressDigest(stepProgressDigest, executingStep->phases[curPhaseIdx], curPhaseIdx);
            if (phaseProgress != stepProgressDigest.phaseProgress[curPhaseIdx])
            {
                stepProgressDigest.phaseProgress[curPhaseIdx] = phaseProgress;
                LOG_DEBUG("INJ_PROGRESS: Phase[%d]: %s\n", curPhaseIdx, Util::qVarientToJsonData(ImrParser::ToImr_PhaseProgressDigest(phaseProgress)).CSTR());

                if (newStatePath == DS_SystemDef::STATE_PATH_BUSY_FINISHING)
                {
                    LOG_INFO("INJ_PROGRESS: PhaseProgressDigest is changed while %s. Stay the state as %s until stablised.\n", ImrParser::ToImr_StatePath(newStatePath).CSTR(), ImrParser::ToImr_StatePath(DS_SystemDef::STATE_PATH_EXECUTING).CSTR());
                    isFinishingStateDebounceCount = 0;
                }
            }

            // Update Active Contrast Location
            if (executingStep->phases[curPhaseIdx].type == DS_ExamDef::INJECTION_PHASE_TYPE_FLUID)
            {
                DS_DeviceDef::FluidSourceIdx newActiveContrastLocation = getActiveContrastLocation(stepProgressDigest, injectDigest.phaseIdx);
                if (stepProgressDigest.activeContrastLocation != newActiveContrastLocation)
                {
                    LOG_INFO("INJ_PROGRESS: ActiveContrastLocation is changed from %s to %s\n",
                             ImrParser::ToImr_FluidSourceIdx(stepProgressDigest.activeContrastLocation).CSTR(), ImrParser::ToImr_FluidSourceIdx(newActiveContrastLocation).CSTR());
                    stepProgressDigest.activeContrastLocation = newActiveContrastLocation;
                }

                injectionPlan.setContrastFluidLocation(newStatePath, executedSteps, stepProgressDigest.activeContrastLocation);
            }
        }
        else
        {
            LOG_ERROR("INJ_PROGRESS: Bad CurrentPhaseIdx=%d, Phases=%d\n", curPhaseIdx, (int)executingStep->phases.length());
        }

        // We also need to LIE instantaneous rates for preload. What we received from MCU should be re-translated for preload
        if (executingStep->isPreloaded)
        {
            double totalFlowRate = 0;
            for (int syringeIdx = 0; syringeIdx < injectDigest.syringeInjectDigests.length(); syringeIdx++)
            {
                stepProgressDigest.instantaneousRates.flowRates[syringeIdx] = 0;
                totalFlowRate += injectDigest.syringeInjectDigests[syringeIdx].flowRate;
            }

            SyringeIdx syringeIdx = (executingStep->contrastFluidLocation == DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_2) ? SYRINGE_IDX_CONTRAST1 : SYRINGE_IDX_CONTRAST2;
            stepProgressDigest.instantaneousRates.flowRates[syringeIdx] = totalFlowRate * (executingStep->phases[curPhaseIdx].contrastPercentage / 100.0);
            stepProgressDigest.instantaneousRates.flowRates[SYRINGE_IDX_SALINE] = totalFlowRate * ((100.0 - executingStep->phases[curPhaseIdx].contrastPercentage) / 100.0);
        }

        // Make sure the progress is updated without rely on mcu data change.
        if (!tmrInjectionProgressUpdate.isActive())
        {
            tmrInjectionProgressUpdate.start(MCU_LINK_HEART_BEAT_INTERVAL_MS * 2);
        }
    }

    // ------------------------------------------------
    // Handle INJECTION COMPLETING State
    if ( ( (curStatePath == DS_SystemDef::STATE_PATH_READY_ARMED) ||
           (curStatePath == DS_SystemDef::STATE_PATH_EXECUTING) ||
           (curStatePath == DS_SystemDef::STATE_PATH_BUSY_HOLDING) ) &&
         (newStatePath == DS_SystemDef::STATE_PATH_BUSY_FINISHING) )
    {
        // Ensure that preventing backflow is turned off in the step progress digest since the phase is done
        stepProgressDigest.preventingBackflowSaline = false;

        if (stepProgressDigest.terminationReason != DS_ExamDef::STEP_TERMINATION_REASON_INVALID)
        {
            // Terminated reason is already set
        }
        else
        {
            env->ds.examAction->actGetInjectionTerminationReason(stepProgressDigest.terminationReason, stepProgressDigest.mcuTerminatedReason, stepProgressDigest.terminatedReasonMessage, mcuProgress.completeStatus);
        }
        setLastPhaseProgress(stepProgressDigest, executingStep);
    }

    if (newStatePath == DS_SystemDef::STATE_PATH_BUSY_FINISHING)
    {
        isFinishingStateDebounceCount++;
    }

    // ------------------------------------------------
    // Handle INJECTION COMPLETED State
    if ( (curStatePath == DS_SystemDef::STATE_PATH_BUSY_FINISHING) &&
         (newStatePath == DS_SystemDef::STATE_PATH_IDLE) )
    {
        LOG_DEBUG("INJ_PROGRESS: Injection is completed.\n");
        for (int phaseIdx = 0; phaseIdx < stepProgressDigest.phaseProgress.length(); phaseIdx++)
        {
            LOG_DEBUG("INJ_PROGRESS: Phase[%d]: %s\n", phaseIdx, Util::qVarientToJsonData(ImrParser::ToImr_PhaseProgressDigest(stepProgressDigest.phaseProgress[phaseIdx])).CSTR());
        }

        if (env->ds.alertAction->isActivated("InjectionPreventingBackflowSaline"))
        {
            LOG_INFO("INJ_PROGRESS: Injection completed and InjectionPreventingBackflowSaline alert is active. De-activating...\n");
            env->ds.alertAction->deactivate("InjectionPreventingBackflowSaline");
        }

        if (executingStep == NULL)
        {
            QString err = QString().asprintf("INJ_PROGRESS: Failed to get ExecutingStep after finishing injection\n");
            LOG_ERROR("%s", err.CSTR());
            env->ds.alertAction->activate("HCUInternalSoftwareError", err);
            return;
        }

        // Update last phase progress state
        setLastPhaseProgress(stepProgressDigest, executingStep);
        stepProgressDigest.stepCompletedEpochMs = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();

        // Update ExpectedCompleteAt
        if (stepProgressDigest.terminationReason == DS_ExamDef::STEP_TERMINATION_REASON_NORMAL)
        {
            stepProgressDigest.stepCompleteExpectedEpochMs = stepProgressDigest.stepCompletedEpochMs;
            LOG_INFO("INJ_PROGRESS: Injection completed, duration=%lldms\n", stepProgressDigest.stepCompleteExpectedEpochMs - stepProgressDigest.stepTriggeredEpochMs);
        }
        else
        {
            stepProgressDigest.stepCompleteExpectedEpochMs = stepProgressDigest.stepTriggeredEpochMs;

            for (int phaseIdx = 0; phaseIdx < executingStep->phases.length(); phaseIdx++)
            {
                if (phaseIdx < stepProgressDigest.phaseProgress.length() - 1)
                {
                    // Add duration for completed phases
                    stepProgressDigest.stepCompleteExpectedEpochMs += stepProgressDigest.phaseProgress[phaseIdx].elapsedMillisFromPhaseStart;
                }
                else if (phaseIdx == stepProgressDigest.phaseProgress.length() - 1)
                {
                    // Add duration for aborted phase
                    const DS_ExamDef::PhaseProgressDigest *phaseProgressDigest = &stepProgressDigest.phaseProgress[phaseIdx];
                    const DS_ExamDef::InjectionPhase *phase = &executingStep->phases[phaseIdx];

                    if (phase->type == DS_ExamDef::INJECTION_PHASE_TYPE_FLUID)
                    {
                        double remainingVol = phase->totalVol * ((100 - phaseProgressDigest->progress) * 0.01);
                        qint64 remainingMs = (remainingVol / phase->flowRate) * 1000;
                        stepProgressDigest.stepCompleteExpectedEpochMs += phaseProgressDigest->elapsedMillisFromPhaseStart + remainingMs;
                    }
                    else
                    {
                        double phaseProgress = stepProgressDigest.phaseProgress[phaseIdx].progress;
                        int phaseProgressInt = (int)(Util::isFloatVarGreaterThan(phaseProgress, 1) ? phaseProgress : 1);
                        stepProgressDigest.stepCompleteExpectedEpochMs += (stepProgressDigest.phaseProgress[phaseIdx].elapsedMillisFromPhaseStart * 100) / phaseProgressInt;
                    }
                }
                else
                {
                    // Add duration for uncompleted phases
                    stepProgressDigest.stepCompleteExpectedEpochMs += executingStep->phases[phaseIdx].durationMs;
                }
            }
            LOG_WARNING("INJ_PROGRESS: Injection aborted(%s), duration=%lldms, expectedCompleteDuration=%lldms\n",
                        ImrParser::ToImr_StepTerminationReason(stepProgressDigest.terminationReason).CSTR(),
                        stepProgressDigest.stepCompletedEpochMs - stepProgressDigest.stepTriggeredEpochMs,
                        stepProgressDigest.stepCompleteExpectedEpochMs - stepProgressDigest.stepTriggeredEpochMs);
        }
    }

    // ------------------------------------------------
    // Update Data
    if ( (newStatePath == DS_SystemDef::STATE_PATH_EXECUTING) ||
         (newStatePath == DS_SystemDef::STATE_PATH_BUSY_HOLDING) ||
         (newStatePath == DS_SystemDef::STATE_PATH_BUSY_FINISHING))
    {
        // Update InjectionPreventingBackflowSaline alert
        bool preventingBackflowIsActive = env->ds.alertAction->isActivated("InjectionPreventingBackflowSaline");
        if (stepProgressDigest.preventingBackflowSaline)
        {
            if (!preventingBackflowIsActive)
            {
                env->ds.alertAction->activate("InjectionPreventingBackflowSaline");
                LOG_WARNING("INJ_PROGRESS: Preventing Backflow Saline Activated\n");
            }
        }
        else
        {
            if (preventingBackflowIsActive)
            {
                env->ds.alertAction->deactivate("InjectionPreventingBackflowSaline");
                LOG_INFO("INJ_PROGRESS: Preventing Backflow Saline Deactivated\n");
            }
        }
    }

    // Update current step progress digest
    if (executedSteps.length() > 0)
    {
        executedSteps[executedSteps.length() - 1] = stepProgressDigest;
    }

    // Set HCU data with DataLocked flag
    DS_ExamDef::ExecutedSteps prevExecutedSteps = env->ds.examData->getExecutedSteps();
    DS_ExamDef::InjectionPlan prevInjectionPlan = env->ds.examData->getInjectionPlan();
    DS_ExamDef::ExamProgressState prevExamProgressState = env->ds.examData->getExamProgressState();
    DS_SystemDef::StatePath prevStatePath = env->ds.systemData->getStatePath();

    env->ds.systemData->setDigestLocked(true);
    env->ds.systemData->setDataLocked(true);
    env->ds.examData->setDataLocked(true);
    env->ds.examData->setExecutedSteps(executedSteps);
    env->ds.examData->setInjectionPlan(injectionPlan);
    env->ds.examData->setExamProgressState(examProgressState);

    if (newStatePath != prevStatePath)
    {
        QString newStatePathStr = ImrParser::ToImr_StatePath(newStatePath);
        QString prevStatePathStr = ImrParser::ToImr_StatePath(prevStatePath);

        if ( (newStatePath == DS_SystemDef::STATE_PATH_BUSY_FINISHING) &&
             (isFinishingStateDebounceCount < INJECTION_IS_FINISHING_STATE_DEBOUNCE_MAX) )
        {
            LOG_DEBUG("INJ_PROGRESS: Busy/Finishing transition debounce(count=%d/%d) in progress. Keeping statePath=%s (actual=%s)\n",
                      isFinishingStateDebounceCount,
                      INJECTION_IS_FINISHING_STATE_DEBOUNCE_MAX,
                      prevStatePathStr.CSTR(),
                      newStatePathStr.CSTR());
            newStatePath = prevStatePath;

            // Make sure this function gets called again during Debounce in progress
            tmrInjectionProgressUpdate.stop();
            tmrInjectionProgressUpdate.start(MCU_LINK_HEART_BEAT_INTERVAL_MS);
        }
        else if (prevStatePath == DS_SystemDef::STATE_PATH_ERROR)
        {
            LOG_WARNING("INJ_PROGRESS: Current StatePath=%s. Failed to set new StatePath(%s)\n", prevStatePathStr.CSTR(), newStatePathStr.CSTR());
            newStatePath = prevStatePath;
        }
        else if ( ( (prevStatePath == DS_SystemDef::STATE_PATH_EXECUTING) ||
                    (prevStatePath == DS_SystemDef::STATE_PATH_BUSY_HOLDING) ) &&
                  (newStatePath == DS_SystemDef::STATE_PATH_IDLE) )
        {
            LOG_WARNING("INJ_PROGRESS: Bad StatePath transition(%s->%s). StatePath=%s is added between..\n",
                        prevStatePathStr.CSTR(), newStatePathStr.CSTR(), ImrParser::ToImr_StatePath(DS_SystemDef::STATE_PATH_BUSY_FINISHING).CSTR());
            newStatePath = DS_SystemDef::STATE_PATH_BUSY_FINISHING;
            env->ds.systemAction->actSetStatePath(newStatePath);

            // Make sure this function gets called again to set to IDLE state
            tmrInjectionProgressUpdate.stop();
            tmrInjectionProgressUpdate.start(MCU_LINK_HEART_BEAT_INTERVAL_MS);
        }
        else
        {
            LOG_DEBUG("INJ_PROGRESS: Setting New StatePath=%s..\n", ImrParser::ToImr_StatePath(newStatePath).CSTR());
            env->ds.systemAction->actSetStatePath(newStatePath);
        }

        if (newStatePath == DS_SystemDef::STATE_PATH_BUSY_FINISHING)
        {
            // Handle Busy/Finishing state after Debounce
            LOG_INFO("INJ_PROGRESS: Injection FINISHING with reason (MCU=%s, HCU=%s)\n",
                     ImrParser::ToImr_InjectionCompleteStatus(mcuProgress.completeStatus).CSTR(),
                     ImrParser::ToImr_StepTerminationReason(stepProgressDigest.terminationReason).CSTR());

            if (executingStep == NULL)
            {
                QString err = QString().asprintf("INJ_PROGRESS: Failed to get ExecutingStep during finishing\n");
                LOG_ERROR("%s", err.CSTR());
                env->ds.alertAction->activate("HCUInternalSoftwareError", err);
            }
            else
            {
                if (stepProgressDigest.terminationReason == DS_ExamDef::STEP_TERMINATION_REASON_ABORTED_BY_AIR_DETECTION)
                {
                    env->ds.alertAction->activate("OutletAirDetected");
                    env->ds.workflowAction->actSudsAirRecoveryStart();
                }

                activateAlertSalineReservoirContrastPossible(stepProgressDigest, executingStep);
            }
        }
    }

    env->ds.systemData->setDataLocked(false);
    env->ds.examData->setDataLocked(false);
    env->ds.systemData->setDigestLocked(false);

    // Force to emit dataChanged signals that may be missed during locked state
    if (executedSteps != prevExecutedSteps)
    {
        emit env->ds.examData->signalDataChanged_ExecutedSteps(executedSteps, prevExecutedSteps);
    }

    if (injectionPlan != prevInjectionPlan)
    {
        emit env->ds.examData->signalDataChanged_InjectionPlan(injectionPlan, prevInjectionPlan);
    }

    if (examProgressState != prevExamProgressState)
    {
        emit env->ds.examData->signalDataChanged_ExamProgressState(examProgressState, prevExamProgressState);
    }

    if (newStatePath != prevStatePath)
    {
        emit env->ds.systemData->signalDataChanged_StatePath(newStatePath, prevStatePath);
    }

    injectionPrgressUpdatedAtEpochMs = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
}

DS_SystemDef::StatePath ExamInjection::getStatePathFromMcuInjectorState(DS_McuDef::InjectorState state)
{
    switch (state)
    {
    case DS_McuDef::INJECTOR_STATE_READY_START:
        return DS_SystemDef::STATE_PATH_READY_ARMED;
    case DS_McuDef::INJECTOR_STATE_DELIVERING:
        return DS_SystemDef::STATE_PATH_EXECUTING;
    case DS_McuDef::INJECTOR_STATE_HOLDING:
        return DS_SystemDef::STATE_PATH_BUSY_HOLDING;
    case DS_McuDef::INJECTOR_STATE_PHASE_PAUSED:
        return DS_SystemDef::STATE_PATH_EXECUTING;
    case DS_McuDef::INJECTOR_STATE_COMPLETING:
        return DS_SystemDef::STATE_PATH_BUSY_FINISHING;
    case DS_McuDef::INJECTOR_STATE_COMPLETED:
    case DS_McuDef::INJECTOR_STATE_IDLE:
    default:
        return DS_SystemDef::STATE_PATH_IDLE;
    }
}

void ExamInjection::updateMaxPressure(DS_ExamDef::PhaseProgressDigest &phaseProgressDigest, int phaseIndex, int curPressureKpa)
{
    if (phaseProgressDigest.maxPressure < curPressureKpa)
    {
        LOG_INFO("getPhaseProgressDigest(): phase[%d]: phaseProgressDigest.maxPressure = %d -> %dkPa\n", phaseIndex, phaseProgressDigest.maxPressure, curPressureKpa);
        phaseProgressDigest.maxPressure = curPressureKpa;
    }
}

void ExamInjection::updateMaxFlowRate(DS_ExamDef::PhaseProgressDigest &phaseProgressDigest, int phaseIndex, double curFlowRateTotal)
{
    if (Util::isFloatVarGreaterThan(curFlowRateTotal, phaseProgressDigest.maxFlowRate))
    {
        LOG_INFO("getPhaseProgressDigest(): phase[%d]: New Max FlowRate = %.1f -> %.1f ml/s\n", phaseIndex, phaseProgressDigest.maxFlowRate, curFlowRateTotal);
        phaseProgressDigest.maxFlowRate = curFlowRateTotal;
    }
}

DS_ExamDef::PhaseProgressDigest ExamInjection::getPhaseProgressDigest(const DS_ExamDef::ExecutedStep &stepProgress, const DS_ExamDef::InjectionPhase &phase, int phaseIndex)
{
    DS_McuDef::InjectDigest injectDigest = env->ds.mcuData->getInjectDigest();
    int curPressureKpa = env->ds.mcuData->getPressure();
    double curFlowRateTotal = stepProgress.instantaneousRates.getTotal();
    DS_ExamDef::PhaseProgressDigest phaseProgressDigest = stepProgress.phaseProgress[phaseIndex];
    const DS_ExamDef::InjectionStep *executingStep = env->ds.examData->getExecutingStep();

    if (executingStep->isPreloaded)
    {
        DS_McuDef::InjectDigest newInjectDigest;
        env->ds.examAction->actGetPreloadedInjectDigest(newInjectDigest, injectDigest);
        env->ds.examAction->actGetHcuPhaseInjectedVolsFromMcuInjectDigest(phaseProgressDigest.injectedVolumes, newInjectDigest, phaseIndex);
    }
    else
    {
        // Update injected volumes
        env->ds.examAction->actGetHcuPhaseInjectedVolsFromMcuInjectDigest(phaseProgressDigest.injectedVolumes, injectDigest, phaseIndex);
    }

    // Update ElapsedMillisFromPhaseStart
    if (phaseProgressDigest.state == DS_ExamDef::INJECTION_PHASE_PROGRESS_STATE_PROGRESS)
    {
        quint64 elapsedMillisFromPhaseStart = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch() - stepProgress.stepTriggeredEpochMs;
        for (int prevPhaseIdx = 0; prevPhaseIdx < phaseIndex; prevPhaseIdx++)
        {
            // Find elapsedMillisFromPhaseStart by take out the time spent from previous phases
            elapsedMillisFromPhaseStart -= stepProgress.phaseProgress[prevPhaseIdx].elapsedMillisFromPhaseStart;
        }
        phaseProgressDigest.elapsedMillisFromPhaseStart = elapsedMillisFromPhaseStart;

        // Update max pressure & max flow rate
        updateMaxPressure(phaseProgressDigest, phaseIndex, curPressureKpa);
        updateMaxFlowRate(phaseProgressDigest, phaseIndex, curFlowRateTotal);
    }
    // Leaving this part commented out for future max pressure update implementation
    // else if (phaseProgressDigest.state == DS_ExamDef::INJECTION_PHASE_PROGRESS_STATE_COMPLETED)
    // {
    //     // Update max pressure & max flow rate
    //     updateMaxPressure(phaseProgressDigest, phaseIndex, curPressureKpa);
    //     updateMaxFlowRate(phaseProgressDigest, phaseIndex, curFlowRateTotal);
    // }

    if ( (phaseProgressDigest.state == DS_ExamDef::INJECTION_PHASE_PROGRESS_STATE_ABORTED) ||
         (phaseProgressDigest.state == DS_ExamDef::INJECTION_PHASE_PROGRESS_STATE_JUMPED) )
    {
        // no need to update the phase progress
    }
    else
    {
        DS_SystemDef::StatePath statePath = env->ds.systemData->getStatePath();
        if (statePath == DS_SystemDef::STATE_PATH_BUSY_HOLDING)
        {
            // No need to update progress
        }
        else
        {
            // Update progress
            double curProgress = phaseProgressDigest.progress;
            if (phase.type == DS_ExamDef::INJECTION_PHASE_TYPE_FLUID)
            {
                curProgress = (phaseProgressDigest.injectedVolumes.total() * 100) / phase.totalVol;
            }
            else if (phase.type == DS_ExamDef::INJECTION_PHASE_TYPE_DELAY)
            {
                if (phaseProgressDigest.state == DS_ExamDef::INJECTION_PHASE_PROGRESS_STATE_PROGRESS)
                {
                    curProgress = ((phaseProgressDigest.elapsedMillisFromPhaseStart - stepProgress.userHoldPerformedMs) * 100.00) / phase.durationMs;
                }
            }

            // Trime down progress at most 100%
            curProgress = Util::isFloatVarGreaterThan(100, curProgress) ? curProgress : 100;

            // Ensure curProgress value is up to date
            if (Util::isFloatVarGreaterThan(phaseProgressDigest.progress, curProgress))
            {
                curProgress = phaseProgressDigest.progress;
            }
            phaseProgressDigest.progress = Util::roundFloat(curProgress, 2);
        }
        //LOG_DEBUG("getPhaseProgressDigest(): phase[%d]: statePath=%s, phaseProgressDigest.progress=%f\n", phaseIndex, ImrParser::ToImr_StatePath(statePath).CSTR(), phaseProgressDigest.progress);
    }
    return phaseProgressDigest;
}

void ExamInjection::setLastPhaseProgress(DS_ExamDef::ExecutedStep &stepProgressDigest, const DS_ExamDef::InjectionStep *executingStep)
{
    if (stepProgressDigest.phaseProgress.length() > 0)
    {
        // Update previous phaseProgress: Phase transition may happened too quickly, phaseProgress is not updated during that time.
        int lastPhaseIdx = stepProgressDigest.phaseProgress.length() - 1;
        DS_ExamDef::PhaseProgressDigest lastPhaseProgressDigest = getPhaseProgressDigest(stepProgressDigest, executingStep->phases[lastPhaseIdx], lastPhaseIdx);

        if (lastPhaseProgressDigest.state == DS_ExamDef::INJECTION_PHASE_PROGRESS_STATE_JUMPED)
        {
            LOG_INFO("setLastPhaseProgress(): Phase[%d] is skipped. Not touching progress data.\n", stepProgressDigest.phaseIndex);
            return;
        }

        if (stepProgressDigest.terminationReason != DS_ExamDef::STEP_TERMINATION_REASON_INVALID)
        {
            // Injection completed
            if (stepProgressDigest.terminationReason == DS_ExamDef::STEP_TERMINATION_REASON_NORMAL)
            {
                LOG_INFO("setLastPhaseProgress(): Phase[%d] is completed.\n", stepProgressDigest.phaseIndex);
                lastPhaseProgressDigest.state = DS_ExamDef::INJECTION_PHASE_PROGRESS_STATE_COMPLETED;
                lastPhaseProgressDigest.progress = 100;
            }
            else
            {
                // Injection completed with abnormal state
                LOG_INFO("setLastPhaseProgress(): Phase[%d] is aborted.\n", stepProgressDigest.phaseIndex);
                lastPhaseProgressDigest.state = DS_ExamDef::INJECTION_PHASE_PROGRESS_STATE_ABORTED;
            }
        }
        else if (lastPhaseProgressDigest.state == DS_ExamDef::INJECTION_PHASE_PROGRESS_STATE_ABORTED)
        {
            LOG_INFO("setLastPhaseProgress(): Phase[%d] is aborted.\n", stepProgressDigest.phaseIndex);
        }
        else
        {
            LOG_INFO("setLastPhaseProgress(): Phase[%d] is completed.\n", stepProgressDigest.phaseIndex);
            lastPhaseProgressDigest.state = DS_ExamDef::INJECTION_PHASE_PROGRESS_STATE_COMPLETED;
            lastPhaseProgressDigest.progress = 100;
        }

        stepProgressDigest.phaseProgress[lastPhaseIdx] = lastPhaseProgressDigest;
        LOG_DEBUG("INJ_PROGRESS: Phase[%d]: %s\n", lastPhaseIdx, Util::qVarientToJsonData(ImrParser::ToImr_PhaseProgressDigest(lastPhaseProgressDigest)).CSTR());
    }
}

DS_DeviceDef::FluidSourceIdx ExamInjection::getActiveContrastLocation(const DS_ExamDef::ExecutedStep &stepProgressDigest, int mcuPhaseIdx)
{
    DS_McuDef::InjectionProtocol mcuInjProtocol = env->ds.mcuData->getInjectionProtocol();
    DS_DeviceDef::FluidSourceIdx newActiveContrastLocation = stepProgressDigest.activeContrastLocation;

    switch (mcuInjProtocol.phases[mcuPhaseIdx].type)
    {
    case DS_McuDef::INJECTION_PHASE_TYPE_CONTRAST1:
    case DS_McuDef::INJECTION_PHASE_TYPE_DUAL1:
        newActiveContrastLocation = DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_2;
        break;
    case DS_McuDef::INJECTION_PHASE_TYPE_CONTRAST2:
    case DS_McuDef::INJECTION_PHASE_TYPE_DUAL2:
        newActiveContrastLocation = DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_3;
        break;
    default:
        break;
    }

    return newActiveContrastLocation;
}

void ExamInjection::handleDisarmedState()
{
    DS_McuDef::InjectorStatus mcuProgress = env->ds.mcuData->getInjectorStatus();

    // Handle disarm reason
    switch (mcuProgress.completeStatus.reason)
    {
    case DS_McuDef::INJECTION_COMPLETE_REASON_DISARMED_SUDS_MISSING:
        env->ds.alertAction->activate("InjectionDisarmedByUser", "SUDS");
        break;
    case DS_McuDef::INJECTION_COMPLETE_REASON_DISARMED_HCU_ABORT:
        {
            DS_ExamDef::InjectionRequestProcessStatus reqProcessStatus = env->ds.examData->getInjectionRequestProcessStatus();
            QString requester = ( (!reqProcessStatus.requestedByHcu) && (reqProcessStatus.state == "T_DISARMING") ) ? "CRU" : "SRU";
            env->ds.alertAction->activate("InjectionDisarmedByUser", requester);
        }
        break;
    case DS_McuDef::INJECTION_COMPLETE_REASON_DISARMED_STOP_BUTTON_ABORT:
        env->ds.alertAction->activate("InjectionDisarmedByUser", "AllStop");
        break;
    case DS_McuDef::INJECTION_COMPLETE_REASON_DISARMED_ARM_TIMEOUT:
        env->ds.alertAction->activate("InjectionDisarmedByTimeout");
        break;
    case DS_McuDef::INJECTION_COMPLETE_REASON_DISARMED_MUDS_UNLATCHED:
        env->ds.alertAction->activate("InjectionDisarmedByMUDSUnlatched");
        break;
    default:
        break;
    }
}

void ExamInjection::handleInjectionPlanChanged()
{
    DS_ExamDef::InjectionPlan plan = env->ds.examData->getInjectionPlan();

    // Update Test Injection Steps
    for (int stepIdx = 0; stepIdx < plan.steps.length(); stepIdx++)
    {
        if (plan.steps[stepIdx].isTestInjection)
        {
            // For test injection, force isNotScannerSynchronized value to true
            plan.steps[stepIdx].isNotScannerSynchronized = true;
        }
    }

    // Update IsModifiedFromTemplate
    DS_ExamDef::InjectionPlanTemplateGroups groups = env->ds.examData->getInjectionPlanTemplateGroups();
    DS_ExamDef::InjectionPlanDigest *planDigest = env->ds.examData->getPlanDigestFromTemplateGuid(groups, plan.templateGuid);

    if ( (plan.templateGuid != DEFAULT_INJECT_PLAN_TEMPLATE_GUID) &&
         (planDigest == NULL) )
    {
        // HCU will not have templates for Connect.CT and ISI as these have zero steps
        LOG_INFO("Failed to get a plan template for template guid(%s)\n", plan.templateGuid.CSTR());
    }

    // Only determine if the plan is modified if we have the template to compare to
    if (planDigest != NULL)
    {        
        bool cruCommsActive =  (env->ds.cruData->getCruLinkStatus().state == DS_CruDef::CRU_LINK_STATE_ACTIVE);
        plan.updateIsModifiedFromTemplated(planDigest->plan, cruCommsActive);
    }

    env->ds.examData->setInjectionPlan(plan);
}

void ExamInjection::activateAlertSalineReservoirContrastPossible(const DS_ExamDef::ExecutedStep &stepProgressDigest, const DS_ExamDef::InjectionStep *executingStep)
{
    // Raise SalineReservoirContrastPossible alert if occurred
    if ( (stepProgressDigest.terminationReason != DS_ExamDef::STEP_TERMINATION_REASON_ABORTED_BY_OVER_PRESSURE) &&
         (stepProgressDigest.terminationReason != DS_ExamDef::STEP_TERMINATION_REASON_ABORTED_BY_STALLING) )
    {
        LOG_DEBUG("activateAlertSalineReservoirContrastPossible(): NOT Occurred. Reason: TerminationReason is NOT OverPressure nor Stall\n");
        return;
    }

    const DS_ExamDef::InjectionPhase *lastPhase = &executingStep->phases[stepProgressDigest.phaseIndex];
    if (!lastPhase->isDualPhase())
    {
        LOG_DEBUG("activateAlertSalineReservoirContrastPossible(): NOT Occurred. Reason: Last phase is NOT dual phase\n");
        return;
    }

    if (lastPhase->contrastPercentage <= 60)
    {
        LOG_DEBUG("activateAlertSalineReservoirContrastPossible(): NOT Occurred. Reason: Last dual phase ratio is NOT greater than 60%%\n");
        return;
    }

    if (Util::isFloatVarGreaterThan(lastPhase->flowRate, 3))
    {
        LOG_DEBUG("activateAlertSalineReservoirContrastPossible(): NOT Occurred. Reason: Last dual phase flow rate is NOT less/equal to 3ml/s\n");
        return;
    }

    // Get First Fluid Phase Index
    int firstFluidPhaseIdx = -1;
    for (int phaseIdx = 0; phaseIdx < executingStep->phases.length(); phaseIdx++)
    {
        if (executingStep->phases[phaseIdx].type == DS_ExamDef::INJECTION_PHASE_TYPE_FLUID)
        {
            firstFluidPhaseIdx = phaseIdx;
            break;
        }
    }

    if (stepProgressDigest.phaseIndex == firstFluidPhaseIdx)
    {
        LOG_DEBUG("activateAlertSalineReservoirContrastPossible(): Occurred. Reason: Last dual phase is the first fluid phase\n");
        env->ds.alertAction->activate("SalineReservoirContrastPossible");
        return;
    }

    int precedingContrastPhaseIdx = -1;
    for (int precedingPhaseIdx = stepProgressDigest.phaseIndex - 1; precedingPhaseIdx >= 0; precedingPhaseIdx--)
    {
        const DS_ExamDef::InjectionPhase *precedingPhase = &executingStep->phases[precedingPhaseIdx];
        if (precedingPhase->isContrastPhase())
        {
            precedingContrastPhaseIdx = precedingPhaseIdx;
            break;
        }
        else if (precedingPhase->type == DS_ExamDef::INJECTION_PHASE_TYPE_DELAY)
        {
            // Ignore delay phase
            continue;
        }
        else
        {
            break;
        }
    }

    if (precedingContrastPhaseIdx == -1)
    {
        LOG_DEBUG("activateAlertSalineReservoirContrastPossible(): NOT Occurred. Reason: Preceding phases are NOT contrast phases\n");
        return;
    }

    double contrastDelivered = 0;
    for (int precedingPhaseIdx = precedingContrastPhaseIdx; precedingPhaseIdx >= 0; precedingPhaseIdx--)
    {
        const DS_ExamDef::InjectionPhase *precedingPhase = &executingStep->phases[precedingPhaseIdx];
        if (precedingPhase->isContrastPhase())
        {
            contrastDelivered += stepProgressDigest.phaseProgress[precedingPhaseIdx].injectedVolumes.total();
        }
        else if (precedingPhase->type == DS_ExamDef::INJECTION_PHASE_TYPE_DELAY)
        {
            // Ignore delay phase
            continue;
        }
        else
        {
            break;
        }
    }

    if (contrastDelivered >= 20)
    {
        LOG_DEBUG("activateAlertSalineReservoirContrastPossible(): NOT Occurred. Reason: Preceding contrast phases delivered NOT lesss than 20\n");
        return;
    }

    LOG_INFO("activateAlertSalineReservoirContrastPossible(): Occurred\n");
    env->ds.alertAction->activate("SalineReservoirContrastPossible");
}
