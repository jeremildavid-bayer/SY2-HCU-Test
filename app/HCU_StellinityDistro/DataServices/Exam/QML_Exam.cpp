#include "QML_Exam.h"
#include "DataServices/Exam/DS_ExamData.h"
#include "DataServices/Exam/DS_ExamAction.h"
#include "DataServices/Mcu/DS_McuData.h"
#include "DataServices/Alert/DS_AlertAction.h"
#include "DataServices/Device/DS_DeviceData.h"
#include "Common/ImrParser.h"

QML_Exam::QML_Exam(QObject *parent, EnvGlobal *env_) :
    QObject(parent),
    env(env_)
{
    envLocal = new EnvLocal("QML_Exam", "QML_EXAM");
    qmlSrc = env->qml.object->findChild<QObject*>("dsExam");
    env->qml.engine->rootContext()->setContextProperty("dsExamCpp", this);

    if (qmlSrc == NULL)
    {
        LOG_ERROR("Failed to assign qmlSrc.\n");
    }

    // Register Data Callbacks
    connect(env->ds.examData, &DS_ExamData::signalDataChanged_ExamProgressState, this, [=](DS_ExamDef::ExamProgressState examProgressState) {
        qmlSrc->setProperty("examProgressState", ImrParser::ToImr_ExamProgressState(examProgressState));
    });

    connect(env->ds.examData, &DS_ExamData::signalDataChanged_ExamGuid, this, [=](QString examGuid) {
        qmlSrc->setProperty("examGuid", examGuid);
    });

    connect(env->ds.examData, &DS_ExamData::signalDataChanged_ExamAdvanceInfo, this, [=](DS_ExamDef::ExamAdvanceInfo examAdvanceInfo) {
        qmlSrc->setProperty("examAdvanceInfo", ImrParser::ToImr_ExamAdvanceInfo(examAdvanceInfo));
    });

    connect(env->ds.examData, &DS_ExamData::signalDataChanged_InjectionRequestProcessStatus, this, [=](const DS_ExamDef::InjectionRequestProcessStatus &injectionRequestProcessStatus) {
        qmlSrc->setProperty("injectionRequestProcessStatus", ImrParser::ToImr_InjectionRequestProcessStatus(injectionRequestProcessStatus));
    });

    connect(env->ds.examData, &DS_ExamData::signalDataChanged_IsAirCheckNeeded, this, [=](bool isAirCheckNeeded) {
        qmlSrc->setProperty("isAirCheckNeeded", isAirCheckNeeded);
    });

    connect(env->ds.examData, &DS_ExamData::signalDataChanged_SelectedContrast, this, [=](const DS_DeviceDef::FluidInfo &selectedContrast) {
        qmlSrc->setProperty("selectedContrast", ImrParser::ToImr_FluidInfo(selectedContrast));
    });

    connect(env->ds.examData, &DS_ExamData::signalDataChanged_SUDSLength, this, [=](const QString &sudsLength) {
        qmlSrc->setProperty("selectedSUDSLength", sudsLength);
    });

    connect(env->ds.examData, &DS_ExamData::signalDataChanged_InjectionPlanPreview, this, [=](const DS_ExamDef::InjectionPlan &planPreview) {
        QVariantMap planViewMap = ImrParser::ToImr_InjectionPlan(planPreview);
        planViewMap.insert("SalineTotal", planPreview.getSalineTotal());
        planViewMap.insert("ContrastTotal", planPreview.getContrastTotal());

        DS_ExamDef::InjectionPlanTemplateGroups groups = env->ds.examData->getInjectionPlanTemplateGroups();
        DS_ExamDef::InjectionPlanDigest *planDigest = env->ds.examData->getPlanDigestFromTemplateGuid(groups, planPreview.templateGuid);
        if (planDigest != NULL)
        {
            planViewMap.insert("IsPersonalized", planDigest->isPersonalized);
        }
        qmlSrc->setProperty("planPreview", planViewMap);
    });

    connect(env->ds.examData, &DS_ExamData::signalDataChanged_ScannerInterlocks, this, [=](const DS_ExamDef::ScannerInterlocks &scannerInterlocks) {
        qmlSrc->setProperty("scannerInterlocks", ImrParser::ToImr_ScannerInterlocks(scannerInterlocks));
    });

    connect(env->ds.examData, &DS_ExamData::signalDataChanged_InjectionPlanTemplateGroups, this, [=](const DS_ExamDef::InjectionPlanTemplateGroups &groups_) {
        DS_ExamDef::InjectionPlanTemplateGroups groups = groups_;

        bool planTemplateGroupsReady = true;
        for (int groupIdx = 0; groupIdx < groups.length(); groupIdx++)
        {
            if (!groups[groupIdx].isReady())
            {
                // Group Data is still initialising..
                planTemplateGroupsReady = false;
                break;
            }
        }

        for (int groupIdx = 0; groupIdx < groups.length(); groupIdx++)
        {
            if (groups[groupIdx].name == "")
            {
                // Name is not defined. Not supposed to be shown in UI
                groups.removeAt(groupIdx);
                groupIdx--;
            }
        }

        qmlSrc->setProperty("planTemplateGroupsReady", planTemplateGroupsReady);
        if (planTemplateGroupsReady)
        {
            qmlSrc->setProperty("planTemplateGroups", ImrParser::ToImr_InjectionPlanTemplateGroups(groups));
        }
    });

    connect(env->ds.examData, &DS_ExamData::signalDataChanged_ExecutedSteps, this, [=](const DS_ExamDef::ExecutedSteps &executedSteps, const DS_ExamDef::ExecutedSteps &prevExecutedSteps) {
        qmlSrc->setProperty("executedSteps", ImrParser::ToImr_ExecutedSteps(executedSteps, NULL));

        QVariantMap stepProgressDigest;

        if ( (executedSteps.length() > 0) &&
             (prevExecutedSteps.length() > 0) )
        {
            if (executedSteps.last() != prevExecutedSteps.last())
            {
               // current stepProgressDigest is changed
               setInjectionProgressPlot(executedSteps, prevExecutedSteps);
               setInjectionProgressMonitor();
               stepProgressDigest = ImrParser::ToImr_ExecutedStep(executedSteps.last(), NULL);
            }
        }
        qmlSrc->setProperty("stepProgressDigest", stepProgressDigest);
    });

    connect(env->ds.examData, &DS_ExamData::signalDataChanged_InjectionPlan, this, [=](const DS_ExamDef::InjectionPlan &plan_, const DS_ExamDef::InjectionPlan &prevPlan_) {
        DS_ExamDef::InjectionPlan plan = plan_;
        DS_ExamDef::InjectionPlan prevPlan = prevPlan_;

        DS_SystemDef::StatePath statePath = env->ds.systemData->getStatePath();
        DS_ExamDef::ExecutedSteps executedSteps = env->ds.examData->getExecutedSteps();
        const DS_ExamDef::InjectionStep *executingStep = plan.getExecutingStep(statePath, executedSteps);
        DS_ExamDef::InjectionStep *prevExecutingStep = prevPlan.getExecutingStep(statePath, executedSteps);

        QVariantMap planMap = ImrParser::ToImr_InjectionPlan(plan);
        planMap.insert("SalineTotal", plan.getSalineTotal());
        planMap.insert("ContrastTotal", plan.getContrastTotal());
        planMap.insert("SalineTotalToInject", plan.getSalineTotalToInject(executedSteps));
        planMap.insert("ContrastTotalToInject", plan.getContrastTotalToInject(executedSteps));
        qmlSrc->setProperty("plan", planMap);

        setExecutingStep(plan, executingStep, prevExecutingStep);

        if ( ( (plan.templateGuid != DEFAULT_INJECT_PLAN_TEMPLATE_GUID) || (plan.isModifiedFromTemplate) ) &&
             ( (prevPlan.templateGuid == DEFAULT_INJECT_PLAN_TEMPLATE_GUID) && (!prevPlan.isModifiedFromTemplate) ) )
        {
            LOG_INFO("Injection plan is changed from Unmodified Default\n");
            QMetaObject::invokeMethod(qmlSrc, "slotPlanChangedFromUnmodifiedDefault");
        }

        if (executingStep == NULL)
        {
            LOG_ERROR("signalDataChanged_InjectionPlan(): Failed to get ExecutingStep.\n");
            return;
        }

        // Update Flow Adjusted State
        if ( (statePath == DS_SystemDef::STATE_PATH_EXECUTING) &&
             (executingStep->phases.length() > 0) &&
             (executingStep->phases.length() == prevExecutingStep->phases.length()) &&
             (executingStep->phases.last().flowRate != prevExecutingStep->phases.last().flowRate) )
        {
            const DS_ExamDef::InjectionPhase *executingPhase = &executingStep->phases[executingStep->phases.length() - 1];
            const DS_ExamDef::InjectionPhase *prevExecutingPhase = &prevExecutingStep->phases[executingStep->phases.length() - 1];
            const DS_ExamDef::ExecutedStep *stepProgressDigest = &executedSteps[executedSteps.length() - 1];
            const DS_ExamDef::PhaseProgressDigest *phaseProgressDigest = &stepProgressDigest->phaseProgress[stepProgressDigest->phaseIndex];
            double phaseVolumeRemaining = executingPhase->totalVol - phaseProgressDigest->injectedVolumes.total();
            quint64 prevPhaseTimeRemaingMs = (phaseVolumeRemaining * 1000) / prevExecutingPhase->flowRate;
            quint64 phaseTimeRemaingMs = (phaseVolumeRemaining * 1000) / executingPhase->flowRate;
            int phaseTimeAdjustMs = phaseTimeRemaingMs - prevPhaseTimeRemaingMs;

            LOG_INFO("Flow Adjust Performed. Phase[%d]: Flow: %.1f->%.1fml/s. VolumeRemainig=%.1fml, TimeRemaining=%lld->%lldms(diff=%dms)\n",
                     (int)executingStep->phases.length() - 1,
                     prevExecutingPhase->flowRate,
                     executingPhase->flowRate,
                     phaseVolumeRemaining,
                     prevPhaseTimeRemaingMs,
                     phaseTimeRemaingMs,
                     phaseTimeAdjustMs);

            QMetaObject::invokeMethod(qmlSrc, "slotInjectionPlotAdjustPhaseDuration", Q_ARG(QVariant, "PROGRESS"), Q_ARG(QVariant, stepProgressDigest->phaseIndex), Q_ARG(QVariant, phaseTimeAdjustMs));
        }
    });

    connect(env->ds.systemData, &DS_SystemData::signalDataChanged_StatePath, this, [=](DS_SystemDef::StatePath statePath, DS_SystemDef::StatePath prevStatePath) {
        DS_ExamDef::InjectionPlan plan = env->ds.examData->getInjectionPlan();
        DS_ExamDef::ExecutedSteps executedSteps = env->ds.examData->getExecutedSteps();
        const DS_ExamDef::InjectionStep *executingStep = plan.getExecutingStep(statePath, executedSteps);
        DS_ExamDef::InjectionStep *prevExecutingStep = plan.getExecutingStep(prevStatePath, executedSteps);

        setExecutingStep(plan, executingStep, prevExecutingStep);
        setInjectionProgressPlot(executedSteps, executedSteps);
        setInjectionProgressMonitor();
    });

    connect(env->ds.examData, &DS_ExamData::signalDataChanged_ExecutedSteps, this, [=] {
        setIsContrastSelectAllowed();
    });

    connect(env->ds.deviceData, &DS_DeviceData::signalDataChanged_FluidSourceBottles, this, [=](const DS_DeviceDef::FluidSources &fluidSourceBottles, const DS_DeviceDef::FluidSources &prevFluidSourceBottles) {
        if ( (fluidSourceBottles[SYRINGE_IDX_CONTRAST1].sourcePackages != prevFluidSourceBottles[SYRINGE_IDX_CONTRAST1].sourcePackages) ||
             (fluidSourceBottles[SYRINGE_IDX_CONTRAST2].sourcePackages != prevFluidSourceBottles[SYRINGE_IDX_CONTRAST2].sourcePackages) )
        {
            setIsContrastSelectAllowed();
        }
    });


    // Set Data
    qmlSrc->setProperty("defaultInjectPlanTemplateGuid", DEFAULT_INJECT_PLAN_TEMPLATE_GUID);

    // Init others
    connect(&tmrInjPlotUpdate, SIGNAL(timeout()), SLOT(slotUpdateReviewPlotSamples()));
}

QML_Exam::~QML_Exam()
{
    delete envLocal;
}

void QML_Exam::setExecutingStep(const DS_ExamDef::InjectionPlan &plan, const DS_ExamDef::InjectionStep *executingStep, const DS_ExamDef::InjectionStep *prevExecutingStep)
{
    if (executingStep == NULL)
    {
        qmlSrc->setProperty("executingStep", QVariant());
        qmlSrc->setProperty("reminders", QVariant());
        return;
    }

    if (executingStep == prevExecutingStep)
    {
        return;
    }

    QVariantMap executingStepImr = ImrParser::ToImr_InjectionStep(*executingStep);
    int stepIndex = plan.getStepIdxFromStepGuid(executingStep->guid);
    executingStepImr.insert("Index", stepIndex);
    qmlSrc->setProperty("executingStep", executingStepImr);
    qmlSrc->setProperty("reminders", ImrParser::ToImr_Reminders(executingStep->reminders));
}

void QML_Exam::setIsContrastSelectAllowed()
{
    qmlSrc->setProperty("isContrastSelectAllowed", env->ds.examData->isContrastSelectAllowed());
}

void QML_Exam::slotPlanChanged(QVariant jsonPlan)
{
    QVariantMap planMap = jsonPlan.toMap();
    QString err;
    DS_ExamDef::InjectionPlan newPlan = ImrParser::ToCpp_InjectionPlan(planMap, &err, true);

    if (err != "")
    {
        LOG_ERROR("SLOT_PLAN_CHANGED: Failed to parse injectionPlan. Err=%s\n", err.CSTR());
        return;
    }

    env->ds.examAction->actHandleInsertedInjectionStepOrPhase(newPlan);
    DataServiceActionStatus status = env->ds.examAction->actInjectionProgram(newPlan);
    if (status.state != DS_ACTION_STATE_COMPLETED)
    {
        newPlan = env->ds.examData->getInjectionPlan();

        // Failed to set plan. Revert to old one
        LOG_ERROR("SLOT_PLAN_CHANGED: Failed to set plan (ActStatus=%s). Reverting injectionPlan.\n", ImrParser::ToImr_DataServiceActionStatusStr(status).CSTR());
        qmlSrc->setProperty("plan", ImrParser::ToImr_InjectionPlan(newPlan));
    }
}

void QML_Exam::slotPlanPreviewChanged(QVariant planPreview)
{
    QString err;
    DS_ExamDef::InjectionPlan newPlanPreview = ImrParser::ToCpp_InjectionPlan(planPreview.toMap(), &err, true);

    if (err == "")
    {
        DS_ExamDef::InjectionPlan planPreview = env->ds.examData->getInjectionPlanPreview();

        // Select from templates
        DS_ExamDef::InjectionPlanTemplateGroups groups = env->ds.examData->getInjectionPlanTemplateGroups();
        for (int groupIdx = 0; groupIdx < groups.length(); groupIdx++)
        {
            DS_ExamDef::InjectionPlanDigest *planDigest = groups[groupIdx].getPlanDigestFromTemplateGuid(newPlanPreview.templateGuid);
            if (planDigest != NULL)
            {
                planPreview.loadFromTemplate(planDigest->plan);
                env->ds.examData->setInjectionPlanPreview(planPreview);
                return;
            }
        }
    }
    else
    {
        LOG_ERROR("Failed to parse injection plan template. Err=%s\n", err.CSTR());
    }
}

void QML_Exam::slotDefaultPlanTemplateSelected()
{
    const DS_ExamDef::InjectionPlan *defaultPlan = env->ds.examData->getDefaultInjectionPlanTemplate();
    if (defaultPlan == NULL)
    {
        LOG_ERROR("Failed to find default plan in m_InjectionPlanTemplateGroups\n");
        return;
    }
    env->ds.examData->setInjectionPlanPreview(*defaultPlan);
}

void QML_Exam::slotLoadPlanTemplateFromPlan(QVariant planMap)
{
    QString err;
    DS_ExamDef::InjectionPlan plan = ImrParser::ToCpp_InjectionPlan(planMap.toMap(), &err, true);

    if (err == "")
    {
        DS_ExamDef::InjectionPlanTemplateGroups templateGroups = env->ds.examData->getInjectionPlanTemplateGroups();
        for (int groupIdx = 0; groupIdx < templateGroups.length(); groupIdx++)
        {
            DS_ExamDef::InjectionPlanDigest *planDigest = templateGroups[groupIdx].getPlanDigestFromTemplateGuid(plan.templateGuid);
            if (planDigest != NULL)
            {
                DS_ExamDef::InjectionPlan planPreview;
                planPreview.loadFromTemplate(planDigest->plan);
                env->ds.examData->setInjectionPlanPreview(planPreview);
                break;
            }
        }
    }
    else
    {
        LOG_ERROR("Failed to parse injectionPlan. Err=%s\n", err.CSTR());
    }
}

void QML_Exam::slotLoadPlanFromPlanPreview(QVariant planTemplateMap)
{
    QString err;
    DS_ExamDef::InjectionPlan planTemplate = ImrParser::ToCpp_InjectionPlan(planTemplateMap.toMap(), &err, true);

    if (err == "")
    {
        env->ds.examAction->actInjectionSelect(env->ds.examData->getInjectionPlan(), planTemplate);
    }
    else
    {
        LOG_ERROR("Failed to parse InjectionPlanTemplate. Err=%s\n", err.CSTR());
    }
}

void QML_Exam::slotExamProgressStateChanged(QString examProgressState)
{
    DS_ExamDef::ExamProgressState curState = env->ds.examData->getExamProgressState();
    DS_ExamDef::ExamProgressState newState = ImrParser::ToCpp_ExamProgressState(examProgressState);

    LOG_INFO("slotExamProgressStateChanged(): ExamProgressState = %s to %s\n", ImrParser::ToImr_ExamProgressState(curState).CSTR(), examProgressState.CSTR());

    switch (newState)
    {
    case DS_ExamDef::EXAM_PROGRESS_STATE_PREPARED:
        env->ds.examAction->actExamPrepare();
        break;
    case DS_ExamDef::EXAM_PROGRESS_STATE_STARTED:
        env->ds.examAction->actExamStart();
        break;
    case DS_ExamDef::EXAM_PROGRESS_STATE_COMPLETED:
        env->ds.examAction->actExamEnd();
        break;
    default:
        env->ds.examData->setExamProgressState(newState);
        break;
    }
}

void QML_Exam::slotAirCheckDone()
{
    env->ds.examData->setIsAirCheckNeeded(false);
}

void QML_Exam::slotScannerInterlocksCheckDone()
{
    env->ds.examAction->actScannerInterlocksBypass();

    LOG_INFO("slotScannerInterlocksCheckDone(): Scanner Interlocks Check is cleared\n");

    DS_ExamDef::ScannerInterlocks scannerInterlocks = env->ds.examData->getScannerInterlocks();
    scannerInterlocks.interfaceStatus = DS_ExamDef::SCANNER_INTERFACE_STATUS_ENABLED;
    env->ds.examData->setScannerInterlocks(scannerInterlocks);
}

void QML_Exam::slotAdjustInjectionVolume()
{
    env->ds.examAction->actAdjustInjectionVolumes();
}

void QML_Exam::slotInjectionArmed()
{
    const DS_ExamDef::InjectionStep *curStep = env->ds.examData->getExecutingStep();
    if (curStep == NULL)
    {
        LOG_ERROR("slotInjectionArmed(): ARM failed: Cannot get executing step\n");
        return;
    }

    DS_ExamDef::InjectionRequestProcessStatus status;
    status.requestedByHcu = true;
    status.state = "T_ARMING";
    env->ds.examData->setInjectionRequestProcessStatus(status);

    env->ds.examAction->actArm(curStep->isPreloaded ? DS_ExamDef::ARM_TYPE_PRELOAD_SECOND : DS_ExamDef::ARM_TYPE_NORMAL);
}

void QML_Exam::slotInjectionStarted()
{
    DS_ExamDef::InjectionRequestProcessStatus status;
    status.requestedByHcu = true;
    status.state = "T_INJECTSTARTING";
    env->ds.examData->setInjectionRequestProcessStatus(status);

    env->ds.examAction->actInjectionStart("");
}

void QML_Exam::slotInjectionPaused()
{
    env->ds.examAction->actInjectionHold();
}

void QML_Exam::slotInjectionAborted()
{
    DS_SystemDef::StatePath statePath = env->ds.systemData->getStatePath();
    if (statePath == DS_SystemDef::STATE_PATH_READY_ARMED)
    {
        DS_ExamDef::InjectionRequestProcessStatus reqProcessStatus;
        reqProcessStatus.requestedByHcu = true;
        reqProcessStatus.state = "T_DISARMING";
        env->ds.examData->setInjectionRequestProcessStatus(reqProcessStatus);
        env->ds.examAction->actDisarm();
    }
    else if ( (statePath == DS_SystemDef::STATE_PATH_EXECUTING) ||
              (statePath == DS_SystemDef::STATE_PATH_BUSY_HOLDING) )
    {
        env->ds.examAction->actInjectionStop();
    }
    else
    {
        // Reset ARM Status
        DS_ExamDef::InjectionRequestProcessStatus reqProcessStatus;
        env->ds.examData->setInjectionRequestProcessStatus(reqProcessStatus);
    }
}

void QML_Exam::slotInjectionSkipped()
{
    DS_ExamDef::ExecutedSteps executedSteps = env->ds.examData->getExecutedSteps();
    if (executedSteps.length() > 0)
    {
        int nextIndex = executedSteps.last().phaseIndex + 1;
        env->ds.examAction->actInjectionJump(nextIndex);
    }
}

void QML_Exam::slotInjectionRepeat()
{
    env->ds.examAction->actInjectionRepeat();
}

void QML_Exam::slotInjectionFlowAdjusted(bool up)
{
    env->ds.examAction->actInjectionAdjustFlowRate(up ? FLOW_ADJUST_DELTA : -FLOW_ADJUST_DELTA);
}

void QML_Exam::slotPressureLimitChanged(int pressureLimitKpa)
{
    // Adjust value to valid range
    pressureLimitKpa = qMax(pressureLimitKpa, PRESSURE_LIMIT_KPA_MIN);
    pressureLimitKpa = qMin(pressureLimitKpa, PRESSURE_LIMIT_KPA_MAX);

    DS_ExamDef::InjectionPlan plan = env->ds.examData->getInjectionPlan();
    int nextExecuteStepIdx = plan.getNextExecuteStepIndex(env->ds.examData->getExecutedSteps());
    if (nextExecuteStepIdx != -1)
    {
        for (int stepIdx = nextExecuteStepIdx; stepIdx < plan.steps.length(); stepIdx++)
        {
            plan.steps[stepIdx].pressureLimit = pressureLimitKpa;
            LOG_DEBUG("slotPressureLimitChanged(): Plan.steps[%d].PressureLimit = %dkPa\n", stepIdx, pressureLimitKpa);
        }
        env->ds.examData->setInjectionPlan(plan);
    }
}

void QML_Exam::slotContrastTypeChanged(QString brand, double concentration)
{
    env->ds.examAction->actReloadSelectedContrast(brand, concentration);
}

void QML_Exam::slotSUDSLengthChanged(QString length)
{
    env->ds.examData->setSUDSLength(length);
}

void QML_Exam::slotUpdateReviewPlot(int stepIndex)
{
    // Open plot data file
    QString filePath = QString(PATH_INJECTION_PLOT_DATA_PREFIX) + QString().asprintf("%d", stepIndex) + PATH_INJECTION_PLOT_DATA_EXT;
    injPlotDataFile.setFileName(filePath);

    if (!injPlotDataFile.open(QFile::ReadOnly | QFile::Text))
    {
        LOG_ERROR("UPDATE_REVIEW_PLOT: Failed to open plot file(err=%s)\n", injPlotDataFile.errorString().CSTR());
        return;
    }

    // Init plot and set plot range
    QMetaObject::invokeMethod(qmlSrc, "slotInjectionPlotSetIsLoading", Q_ARG(QVariant, "REVIEW"), Q_ARG(QVariant, true));
    QMetaObject::invokeMethod(qmlSrc, "slotInjectionPlotReset", Q_ARG(QVariant, "REVIEW"));

    // Start updating plot
    tmrInjPlotUpdate.start(INJECTION_REVIEW_PLOT_DRAW_INTERVAL_MS);
}

void QML_Exam::slotUpdateReviewPlotSamples()
{
    tmrInjPlotUpdate.stop();

    int sampleRemaining = INJECTION_REVIEW_PLOT_DRAW_SAMPLES;

    while (sampleRemaining-- >= 0)
    {
        if (injPlotDataFile.atEnd())
        {
            break;
        }

        QString lineStr = injPlotDataFile.readLine();
        lineStr.replace("\n", "");
        QStringList args = lineStr.split(":");
        QString plotAction = args[0];
        QStringList plotActionArgs;
        if (args.length() > 1)
        {
            if (args[1].length() > 0)
            {
                plotActionArgs = args[1].split(",");
            }
        }

        //LOG_DEBUG("UPDATE_REVIEW_PLOT: INJ_PLOT_WRITE: action=%s, args=%s\n", plotAction.CSTR(), plotActionArgs.join(",").CSTR());

        switch (plotActionArgs.length())
        {
        case 0: QMetaObject::invokeMethod(qmlSrc, plotAction.CSTR(), Q_ARG(QVariant, "REVIEW")); break;
        case 1: QMetaObject::invokeMethod(qmlSrc, plotAction.CSTR(), Q_ARG(QVariant, "REVIEW"), Q_ARG(QVariant, plotActionArgs[0])); break;
        case 2: QMetaObject::invokeMethod(qmlSrc, plotAction.CSTR(), Q_ARG(QVariant, "REVIEW"), Q_ARG(QVariant, plotActionArgs[0]), Q_ARG(QVariant, plotActionArgs[1])); break;
        case 3: QMetaObject::invokeMethod(qmlSrc, plotAction.CSTR(), Q_ARG(QVariant, "REVIEW"), Q_ARG(QVariant, plotActionArgs[0]), Q_ARG(QVariant, plotActionArgs[1]), Q_ARG(QVariant, plotActionArgs[2])); break;
        case 4: QMetaObject::invokeMethod(qmlSrc, plotAction.CSTR(), Q_ARG(QVariant, "REVIEW"), Q_ARG(QVariant, plotActionArgs[0]), Q_ARG(QVariant, plotActionArgs[1]), Q_ARG(QVariant, plotActionArgs[2]), Q_ARG(QVariant, plotActionArgs[3])); break;
        case 5: QMetaObject::invokeMethod(qmlSrc, plotAction.CSTR(), Q_ARG(QVariant, "REVIEW"), Q_ARG(QVariant, plotActionArgs[0]), Q_ARG(QVariant, plotActionArgs[1]), Q_ARG(QVariant, plotActionArgs[2]), Q_ARG(QVariant, plotActionArgs[3]), Q_ARG(QVariant, plotActionArgs[4])); break;
        default:
            LOG_WARNING("UPDATE_REVIEW_PLOT: INJ_PLOT_WRITE: Bad plot action argument length(action=%s, args=%s)\n", plotAction.CSTR(), plotActionArgs.join(",").CSTR());
        }
    }

    if (injPlotDataFile.atEnd())
    {
        // Plot done
        QMetaObject::invokeMethod(qmlSrc, "slotInjectionPlotSetIsLoading", Q_ARG(QVariant, "REVIEW"), Q_ARG(QVariant, false));
        injPlotDataFile.close();
        return;
    }
    else
    {
        // Plot has more samples
        tmrInjPlotUpdate.start(INJECTION_REVIEW_PLOT_DRAW_INTERVAL_MS);
    }
}

void QML_Exam::setInjectionProgressPlotWriteHeader()
{
    DS_ExamDef::InjectionPlan plan = env->ds.examData->getInjectionPlan();
    DS_ExamDef::ExecutedSteps executedSteps = env->ds.examData->getExecutedSteps();

    if (executedSteps.length() == 0)
    {
        LOG_ERROR("setInjectionProgressPlotWriteHeader(): executedSteps.length=0\n");
        return;
    }

    const DS_ExamDef::ExecutedStep *progressDigest = &executedSteps.last();
    const DS_ExamDef::InjectionStep *executingStep = &plan.steps[executedSteps.length() - 1];

    // Write header
    injPlotDataFile.write(QString().asprintf("slotInjectionPlotSetRange:0,%lld\n", progressDigest->stepCompleteExpectedEpochMs - progressDigest->stepTriggeredEpochMs).CSTR());
    injPlotDataFile.write(QString().asprintf("slotInjectionPlotSetContrastColor:%s\n", env->ds.examData->getSelectedContrast().colorCode.CSTR()).CSTR());
    injPlotDataFile.write(QString().asprintf("slotInjectionPlotSetMaxPressure:%d\n", executingStep->pressureLimit).CSTR());

    quint64 totalDuration = 0;

    // Set phase info
    for (int phaseIdx = 0; phaseIdx < executingStep->phases.length(); phaseIdx++)
    {
        const DS_ExamDef::InjectionPhase *phase = &executingStep->phases[phaseIdx];

        injPlotDataFile.write(QString().asprintf("slotInjectionPlotSetPhaseInfo:%d,%lld,%s,%d\n", phaseIdx, totalDuration, ImrParser::ToImr_InjectionPhaseType(phase->type).CSTR(), phase->contrastPercentage).CSTR());

        if (phaseIdx < progressDigest->phaseProgress.length())
        {
            // Completed phase
            const DS_ExamDef::PhaseProgressDigest *phaseProgressDigest = &progressDigest->phaseProgress[phaseIdx];
            if (phaseProgressDigest->state == DS_ExamDef::INJECTION_PHASE_PROGRESS_STATE_JUMPED)
            {
                totalDuration += phaseProgressDigest->elapsedMillisFromPhaseStart;
            }
            else if (phase->type == DS_ExamDef::INJECTION_PHASE_TYPE_FLUID)
            {
                double remainingVol = phase->totalVol * ((100 - phaseProgressDigest->progress) * 0.01);
                qint64 remainingMs = (remainingVol / phase->flowRate) * 1000;
                totalDuration += phaseProgressDigest->elapsedMillisFromPhaseStart + remainingMs;
            }
            else
            {
                double phaseProgress = phaseProgressDigest->progress;
                // using minimum value of 1 to avoid divide by zero
                int phaseProgressInt = (int)(Util::isFloatVarGreaterThan(phaseProgress, 1) ? phaseProgress : 1);
                totalDuration += (phaseProgressDigest->elapsedMillisFromPhaseStart * 100) / phaseProgressInt;
            }
        }
        else
        {
            // Incompleted phase
            totalDuration += phase->durationMs;
        }
    }

    injPlotDataFile.write(QString().asprintf("slotInjectionPlotSetEndTime:%d,%lld\n", (int)executingStep->phases.length(), totalDuration).CSTR());
}

void QML_Exam::setInjectionProgressPlotWrite(QString plotAction, QVariantList args)
{
    static QString plotAction0 = "";
    static QString plotAction1 = "";
    static QStringList argsStrList0;
    static QStringList argsStrList1;

    QStringList argsStrList;
    for (int i = 0; i < args.length(); i++)
    {
        argsStrList.append(args[i].toString());
    }

    if (plotAction == _L("start"))
    {
        plotAction0 = "";
        plotAction1 = "";
    }
    else if (plotAction == _L("complete"))
    {
        if ( (injPlotDataFile.isOpen()) &&
             (injPlotDataFile.isWritable()) )
        {
            injPlotDataFile.seek(0);
            QString fileText = injPlotDataFile.readAll();
            injPlotDataFile.resize(0);

            // Write plot header at top of the file
            setInjectionProgressPlotWriteHeader();
            injPlotDataFile.write(fileText.CSTR());

            // complete the plot
            QString newLine = plotAction0 + ":" + argsStrList0.join(",") + "\n";
            injPlotDataFile.write(newLine.CSTR());
            injPlotDataFile.close();
        }

        QMetaObject::invokeMethod(qmlSrc, "slotInjectionPlotReset", Q_ARG(QVariant, "PROGRESS"));
        return;
    }
    else
    {
        // Apply action
        switch (args.length())
        {
        case 0: QMetaObject::invokeMethod(qmlSrc, plotAction.CSTR(), Q_ARG(QVariant, "PROGRESS")); break;
        case 1: QMetaObject::invokeMethod(qmlSrc, plotAction.CSTR(), Q_ARG(QVariant, "PROGRESS"), Q_ARG(QVariant, args[0])); break;
        case 2: QMetaObject::invokeMethod(qmlSrc, plotAction.CSTR(), Q_ARG(QVariant, "PROGRESS"), Q_ARG(QVariant, args[0]), Q_ARG(QVariant, args[1])); break;
        case 3: QMetaObject::invokeMethod(qmlSrc, plotAction.CSTR(), Q_ARG(QVariant, "PROGRESS"), Q_ARG(QVariant, args[0]), Q_ARG(QVariant, args[1]), Q_ARG(QVariant, args[2])); break;
        case 4: QMetaObject::invokeMethod(qmlSrc, plotAction.CSTR(), Q_ARG(QVariant, "PROGRESS"), Q_ARG(QVariant, args[0]), Q_ARG(QVariant, args[1]), Q_ARG(QVariant, args[2]), Q_ARG(QVariant, args[3])); break;
        case 5: QMetaObject::invokeMethod(qmlSrc, plotAction.CSTR(), Q_ARG(QVariant, "PROGRESS"), Q_ARG(QVariant, args[0]), Q_ARG(QVariant, args[1]), Q_ARG(QVariant, args[2]), Q_ARG(QVariant, args[3]), Q_ARG(QVariant, args[4])); break;
        default:
            LOG_ERROR("INJ_PLOT_WRITE: Bad plot action argument length(action=%s, args=%s)\n", plotAction.CSTR(), argsStrList.join(",").CSTR());
            return;
        }
    }

    if ( (injPlotDataFile.isOpen()) &&
         (injPlotDataFile.isWritable()) )
    {
        bool okToWriteLastLine = true;

        // Check if last line is ok to be written to file
        // Why? We don't want to put too much data points to the file.
        // We need to compress the data file as maximum as possible to redraw the plot
        if ( (plotAction == _L("slotInjectionPlotAddData")) ||
             (plotAction == _L("slotInjectionPlotAddUserPausedData")) )
        {
            if ( (plotAction0 == plotAction) && (plotAction1 == plotAction) )
            {
                if ( (argsStrList0[2] == argsStrList[2]) &&
                     (argsStrList1[2] == argsStrList[2]) )
                {
                    okToWriteLastLine = false;

                    if (plotAction == _L("slotInjectionPlotAddUserPausedData"))
                    {
                        // Accumulate paused time
                        qint64 pausedMs = argsStrList[3].toLongLong();
                        qint64 pausedMs0 = argsStrList0[3].toLongLong();
                        argsStrList[3] = QString().asprintf("%lld", pausedMs + pausedMs0);
                    }
                }
            }
        }
        else if ( (plotAction0 == "") || (plotAction1 == "") )
        {
            okToWriteLastLine = false;
        }

        if (okToWriteLastLine)
        {
            QString newLine = plotAction0 + ":" + argsStrList0.join(",") + "\n";
            injPlotDataFile.write(newLine.CSTR());
        }

        plotAction1 = plotAction0;
        argsStrList1 = argsStrList0;
        plotAction0 = plotAction;
        argsStrList0 = argsStrList;
    }
}

void QML_Exam::setInjectionProgressMonitor()
{
    DS_SystemDef::StatePath statePath = env->ds.systemData->getStatePath();

    if ( (statePath != DS_SystemDef::STATE_PATH_READY_ARMED) &&
         (statePath != DS_SystemDef::STATE_PATH_EXECUTING) &&
         (statePath != DS_SystemDef::STATE_PATH_BUSY_HOLDING) &&
         (statePath != DS_SystemDef::STATE_PATH_BUSY_FINISHING) )
    {
        return;
    }

    DS_ExamDef::InjectionPlan plan = env->ds.examData->getInjectionPlan();
    DS_ExamDef::ExecutedSteps executedSteps = env->ds.examData->getExecutedSteps();
    const DS_ExamDef::InjectionStep *executingStep = plan.getExecutingStep(statePath, executedSteps);

    int pressureKpa = 0;
    double flowRateTotal = 0;
    double volSaline = 0;
    double volContrast = 0;
    bool isPressureLimiting = false;
    int pressureLimit = PRESSURE_LIMIT_KPA_MAX;

    if (executingStep == NULL)
    {
        LOG_WARNING("setInjectionProgressMonitor(): Failed to get ExecutingStep. StatePath=%s, plan.steps=%s, executedSteps=%s\n",
                    ImrParser::ToImr_StatePath(statePath).CSTR(),
                    Util::qVarientToJsonData(ImrParser::ToImr_InjectionPlan(plan)).CSTR(),
                    Util::qVarientToJsonData(ImrParser::ToImr_ExecutedSteps(executedSteps)).CSTR());
    }
    else
    {
        pressureLimit = executingStep->pressureLimit;
    }

    if (statePath == DS_SystemDef::STATE_PATH_READY_ARMED)
    {
        // reset monitor
    }
    else
    {
        if (executedSteps.length() == 0)
        {
            QString err = QString().asprintf("Failed to get StepProgressDigest.\n");
            LOG_ERROR("setInjectionProgressMonitor(): %s\n", err.CSTR());
            env->ds.alertAction->activate("HCUInternalSoftwareError", err);
            return;
        }

        DS_ExamDef::ExecutedStep stepProgressDigest = executedSteps.last();
        pressureKpa = env->ds.mcuData->getPressure();
        flowRateTotal = Util::roundFloat(stepProgressDigest.instantaneousRates.getTotal(), 1);

        for (int phaseIdx = 0; phaseIdx < stepProgressDigest.phaseProgress.length(); phaseIdx++)
        {
            volSaline += stepProgressDigest.phaseProgress[phaseIdx].injectedVolumes.volumes[SYRINGE_IDX_SALINE];
            volContrast += (stepProgressDigest.phaseProgress[phaseIdx].injectedVolumes.volumes[SYRINGE_IDX_CONTRAST1] + stepProgressDigest.phaseProgress[phaseIdx].injectedVolumes.volumes[SYRINGE_IDX_CONTRAST2]);
        }

        isPressureLimiting = ( (stepProgressDigest.adaptiveFlowState == DS_ExamDef::ADAPTIVE_FLOW_STATE_ACTIVE) ||
                               (stepProgressDigest.adaptiveFlowState == DS_ExamDef::ADAPTIVE_FLOW_STATE_CRITICAL) );
    }

    QMetaObject::invokeMethod(qmlSrc, "slotInjectionMonitorUpdate",
                              Q_ARG(QVariant, pressureLimit),
                              Q_ARG(QVariant, isPressureLimiting),
                              Q_ARG(QVariant, pressureKpa),
                              Q_ARG(QVariant, flowRateTotal),
                              Q_ARG(QVariant, volContrast),
                              Q_ARG(QVariant, volSaline));
}

void QML_Exam::setInjectionProgressPlot(DS_ExamDef::ExecutedSteps executedSteps, DS_ExamDef::ExecutedSteps prevExecutedSteps)
{
    static DS_SystemDef::StatePath lastStatePath = DS_SystemDef::STATE_PATH_IDLE;
    static qint64 lastPlotUpdatedEpochMs = 0;
    static int lastPhaseIdx = 0;
    DS_SystemDef::StatePath statePath = env->ds.systemData->getStatePath();
    qint64 curTimeEpochMs = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();

    if ( (lastStatePath == DS_SystemDef::STATE_PATH_BUSY_FINISHING) &&
         (statePath == DS_SystemDef::STATE_PATH_IDLE) )
    {
        // Injection is completed or aborted
        setInjectionProgressPlotWrite("complete");
    }

    if ( (statePath != DS_SystemDef::STATE_PATH_READY_ARMED) &&
         (statePath != DS_SystemDef::STATE_PATH_EXECUTING) &&
         (statePath != DS_SystemDef::STATE_PATH_BUSY_HOLDING) &&
         (statePath != DS_SystemDef::STATE_PATH_BUSY_FINISHING) )
    {
        lastPlotUpdatedEpochMs = curTimeEpochMs;
        lastStatePath = statePath;
        return;
    }

    const DS_ExamDef::InjectionPlan &plan = env->ds.examData->getInjectionPlan();
    const DS_ExamDef::InjectionStep *executingStep = env->ds.examData->getExecutingStep();
    if (executingStep == NULL)
    {
        LOG_ERROR("setInjectionProgressPlot(): Failed to get ExecutingStep.\n");
        return;
    }

    if (statePath == DS_SystemDef::STATE_PATH_READY_ARMED)
    {
        setInjectionProgressPlotArmed(plan, *executingStep);
    }
    else
    {
        if (executedSteps.length() == 0)
        {
            LOG_ERROR("setInjectionProgressPlot(): Failed to get StepProgressDigest.\n");
            return;
        }

        const DS_ExamDef::ExecutedStep &stepProgressDigest = executedSteps.last();

        qint64 timePastMs = 0;
        for (int phaseIndex = 0; phaseIndex <= stepProgressDigest.phaseIndex; phaseIndex++)
        {
            timePastMs += stepProgressDigest.phaseProgress[phaseIndex].elapsedMillisFromPhaseStart;
        }

        if ( (stepProgressDigest.phaseIndex < 0) ||
             (stepProgressDigest.stepTriggeredEpochMs <= 0) ||
             (timePastMs < 0) )
        {
            LOG_ERROR("Bad stepProgressDigest(phaseIdx=%d, stepTriggered=%lld, timePastMs=%lld. Failed to update progress plot.\n", stepProgressDigest.phaseIndex, stepProgressDigest.stepTriggeredEpochMs, timePastMs);
            return;
        }

        const DS_ExamDef::InjectionPhase &phaseData = executingStep->phases[stepProgressDigest.phaseIndex];

        if (statePath == DS_SystemDef::STATE_PATH_EXECUTING)
        {
            setInjectionProgressPlotExecuting(timePastMs, lastStatePath, stepProgressDigest, *executingStep, phaseData, lastPhaseIdx);
        }
        else if (statePath == DS_SystemDef::STATE_PATH_BUSY_HOLDING)
        {
            setInjectionProgressPlotBusyHolding(timePastMs, lastStatePath, stepProgressDigest, curTimeEpochMs, lastPlotUpdatedEpochMs);
        }
        else if (statePath == DS_SystemDef::STATE_PATH_BUSY_FINISHING)
        {
            setInjectionProgressPlotBusyFinishing(timePastMs, lastStatePath, stepProgressDigest);
        }

        lastPhaseIdx = stepProgressDigest.phaseIndex;

        // Update Pressure Limiting State
        if (prevExecutedSteps.length() > 0)
        {
            DS_ExamDef::ExecutedStep prevExecutedStep = prevExecutedSteps.last();

            bool isPressureLimiting = ( (stepProgressDigest.adaptiveFlowState == DS_ExamDef::ADAPTIVE_FLOW_STATE_ACTIVE) ||
                                        (stepProgressDigest.adaptiveFlowState == DS_ExamDef::ADAPTIVE_FLOW_STATE_CRITICAL) );


            bool wasPressureLimiting = ( (prevExecutedStep.adaptiveFlowState == DS_ExamDef::ADAPTIVE_FLOW_STATE_ACTIVE) ||
                                         (prevExecutedStep.adaptiveFlowState == DS_ExamDef::ADAPTIVE_FLOW_STATE_CRITICAL) );

            if ( (!wasPressureLimiting) &&
                 (isPressureLimiting) )
            {
                setInjectionProgressPlotWrite("slotInjectionPlotPressureLimitingActive", QVariantList() << true);
                setInjectionProgressPlotWrite("slotInjectionPlotAddPressureLimitStartSeries", QVariantList() << stepProgressDigest.phaseIndex << timePastMs << (statePath == DS_SystemDef::STATE_PATH_BUSY_HOLDING));
            }
            else if ( (wasPressureLimiting) &&
                      (!isPressureLimiting) )
            {
                setInjectionProgressPlotWrite("slotInjectionPlotPressureLimitingActive", QVariantList() << false);
                setInjectionProgressPlotWrite("slotInjectionPlotAddPressureLimitEndSeries", QVariantList() << stepProgressDigest.phaseIndex << timePastMs << (statePath == DS_SystemDef::STATE_PATH_BUSY_HOLDING));
            }
        }
    }

    lastPlotUpdatedEpochMs = curTimeEpochMs;
    lastStatePath = statePath;
}

void QML_Exam::setInjectionProgressPlotArmed(const DS_ExamDef::InjectionPlan &plan, const DS_ExamDef::InjectionStep &executingStep)
{
    // Init InjectionPlotData file
    QString filePath = QString(PATH_INJECTION_PLOT_DATA_PREFIX) + QString().asprintf("%d", executingStep.getIndex(plan)) + PATH_INJECTION_PLOT_DATA_EXT;
    if (injPlotDataFile.isOpen())
    {
        LOG_DEBUG("InjPlotData file is already open. Close and open again.\n");
        injPlotDataFile.close();
    }

    injPlotDataFile.setFileName(filePath);
    if (injPlotDataFile.open(QFile::ReadWrite | QFile::Text))
    {
        LOG_DEBUG("InjPlotData file opened successfully (path=%s).\n", filePath.CSTR());
        injPlotDataFile.resize(0);
        injPlotDataFile.seek(0);
    }

    if ( (!injPlotDataFile.isOpen()) ||
         (!injPlotDataFile.isWritable()) )
    {
        LOG_ERROR("InjPlotData file is failed to open for writing. (err=%s)\n", injPlotDataFile.errorString().CSTR());
    }

    QMetaObject::invokeMethod(qmlSrc, "slotInjectionPlotReset", Q_ARG(QVariant, "PROGRESS"));
    QMetaObject::invokeMethod(qmlSrc, "slotInjectionPlotSetContrastColor", Q_ARG(QVariant, "PROGRESS"), Q_ARG(QVariant, env->ds.examData->getSelectedContrast().colorCode));
    QMetaObject::invokeMethod(qmlSrc, "slotInjectionPlotSetMaxPressure", Q_ARG(QVariant, "PROGRESS"), Q_ARG(QVariant, executingStep.pressureLimit));

    quint64 totalDuration = 0;

    // Set phase info
    for (int phaseIdx = 0; phaseIdx < executingStep.phases.length(); phaseIdx++)
    {
        const DS_ExamDef::InjectionPhase *phase = &executingStep.phases[phaseIdx];
        QMetaObject::invokeMethod(qmlSrc, "slotInjectionPlotSetPhaseInfo", Q_ARG(QVariant, "PROGRESS"), Q_ARG(QVariant, phaseIdx), Q_ARG(QVariant, totalDuration), Q_ARG(QVariant, ImrParser::ToImr_InjectionPhaseType(phase->type)), Q_ARG(QVariant, phase->contrastPercentage));
        totalDuration += phase->durationMs;
    }

    QMetaObject::invokeMethod(qmlSrc, "slotInjectionPlotSetEndTime", Q_ARG(QVariant, "PROGRESS"), Q_ARG(QVariant, executingStep.phases.length()), Q_ARG(QVariant, totalDuration));

    // Set reminder info
    for (int reminderIdx = 0; reminderIdx < executingStep.reminders.length(); reminderIdx++)
    {
        qint64 reminderAt = executingStep.reminders[reminderIdx].postStepTriggerDelayMs;
        if (executingStep.reminders[reminderIdx].startAfterStepCompletes)
        {
            reminderAt += executingStep.getDurationMsTotal();
        }
        QMetaObject::invokeMethod(qmlSrc, "slotInjectionPlotAddReminder", Q_ARG(QVariant, "PROGRESS"), Q_ARG(QVariant, reminderAt));
    }

    // Start plot
    setInjectionProgressPlotWrite("start");
}

void QML_Exam::setInjectionProgressPlotExecuting(qint64 timePastMs, DS_SystemDef::StatePath lastStatePath, const DS_ExamDef::ExecutedStep &stepProgressDigest, const DS_ExamDef::InjectionStep &executingStep, const DS_ExamDef::InjectionPhase &phaseData, int lastPhaseIdx)
{
    DS_SystemDef::StatePath statePath = env->ds.systemData->getStatePath();
    int pressureKpa = env->ds.mcuData->getPressure();

    if (lastStatePath == DS_SystemDef::STATE_PATH_READY_ARMED)
    {
        // injection started
        setInjectionProgressPlotWrite("slotInjectionPlotAddPhaseSeries", QVariantList() << ImrParser::ToImr_InjectionPhaseType(phaseData.type) << phaseData.contrastPercentage);
        setInjectionProgressPlotWrite("slotInjectionPlotAddData", QVariantList() << 0 << 0 << 0);
    }
    else if ( (stepProgressDigest.phaseIndex > 0) &&
              (stepProgressDigest.phaseIndex > lastPhaseIdx) )
    {
        qint64 lastPhaseTimePastMs = timePastMs - stepProgressDigest.phaseProgress[stepProgressDigest.phaseIndex].elapsedMillisFromPhaseStart;
        int pressureKpa = env->ds.mcuData->getPressure();

        // phase completed - finish last phase plot and continue next one
        //LOG_DEBUG("slotInjectionPlotAddData: Adding data for last missing phase\n");
        setInjectionProgressPlotWrite("slotInjectionPlotAddData", QVariantList() << lastPhaseIdx << lastPhaseTimePastMs << pressureKpa);

        // Add missing phases and new phase
        for (int phaseIndex = lastPhaseIdx + 1; phaseIndex <= stepProgressDigest.phaseIndex; phaseIndex++)
        {
            setInjectionProgressPlotWrite("slotInjectionPlotAddPhaseSeries", QVariantList() << ImrParser::ToImr_InjectionPhaseType(executingStep.phases[phaseIndex].type) << executingStep.phases[phaseIndex].contrastPercentage);
            if (stepProgressDigest.phaseProgress[phaseIndex - 1].state == DS_ExamDef::INJECTION_PHASE_PROGRESS_STATE_JUMPED)
            {
                setInjectionProgressPlotWrite("slotInjectionPlotSetPhaseSkipped", QVariantList() << phaseIndex);
            }
        }

        // update current phase - start from last phase ended
        setInjectionProgressPlotWrite("slotInjectionPlotAddData", QVariantList() << stepProgressDigest.phaseIndex << lastPhaseTimePastMs << pressureKpa);
    }
    else if (lastStatePath != statePath)
    {
        // phase resumed - finish paused plot and resume
        setInjectionProgressPlotWrite("slotInjectionPlotAddData", QVariantList() << stepProgressDigest.phaseIndex << timePastMs << pressureKpa);
        setInjectionProgressPlotWrite("slotInjectionPlotAddUserResumeSeries", QVariantList() << stepProgressDigest.phaseIndex << timePastMs);
    }

    setInjectionProgressPlotWrite("slotInjectionPlotAddData", QVariantList() << stepProgressDigest.phaseIndex << timePastMs << pressureKpa);
}

void QML_Exam::setInjectionProgressPlotBusyHolding(qint64 timePastMs, DS_SystemDef::StatePath lastStatePath, const DS_ExamDef::ExecutedStep &stepProgressDigest, qint64 curTimeEpochMs, qint64 lastPlotUpdatedEpochMs)
{
    DS_SystemDef::StatePath statePath = env->ds.systemData->getStatePath();
    int pressureKpa = env->ds.mcuData->getPressure();

    if (statePath != lastStatePath)
    {
        // Phase just paused
        setInjectionProgressPlotWrite("slotInjectionPlotAddData", QVariantList() << stepProgressDigest.phaseIndex << timePastMs << pressureKpa);
        setInjectionProgressPlotWrite("slotInjectionPlotAddUserPauseSeries", QVariantList() << timePastMs);
    }
    else
    {
        // Phase still paused
        qint64 pausedMs = curTimeEpochMs - lastPlotUpdatedEpochMs;
        setInjectionProgressPlotWrite("slotInjectionPlotAddUserPausedData", QVariantList() << stepProgressDigest.phaseIndex << timePastMs << pressureKpa << pausedMs);
    }
}

void QML_Exam::setInjectionProgressPlotBusyFinishing(qint64 timePastMs, DS_SystemDef::StatePath lastStatePath, const DS_ExamDef::ExecutedStep &stepProgressDigest)
{
    if (lastStatePath != DS_SystemDef::STATE_PATH_BUSY_FINISHING)
    {
        setInjectionProgressPlotWrite("slotInjectionPlotPressureLimitingActive", QVariantList() << false);
        setInjectionProgressPlotWrite("slotInjectionPlotSetTerminatedTime", QVariantList() << timePastMs << ImrParser::ToImr_StepTerminationReason(stepProgressDigest.terminationReason));
    }

    int pressureKpa = env->ds.mcuData->getPressure();
    setInjectionProgressPlotWrite("slotInjectionPlotAddData", QVariantList() << stepProgressDigest.phaseIndex << timePastMs << pressureKpa);
}


