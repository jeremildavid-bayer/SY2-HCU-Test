#include "Apps/AppManager.h"
#include "DS_ExamAction.h"
#include "Common/ImrParser.h"
#include "DataServices/Exam/DS_ExamData.h"
#include "DataServices/Device/DS_DeviceData.h"
#include "DataServices/Device/DS_DeviceAction.h"
#include "DataServices/Cru/DS_CruData.h"
#include "DataServices/Cru/DS_CruAction.h"
#include "DataServices/Mcu/DS_McuAction.h"
#include "DataServices/System/DS_SystemAction.h"
#include "DataServices/System/DS_SystemData.h"
#include "DataServices/Alert/DS_AlertAction.h"
#include "DataServices/Workflow/DS_WorkflowAction.h"
#include "DataServices/Workflow/DS_WorkflowData.h"
#include "DataServices/CfgGlobal/DS_CfgGlobal.h"
#include "DataServices/Capabilities/DS_Capabilities.h"

DS_ExamAction::DS_ExamAction(QObject *parent, EnvGlobal *env_) :
    ActionBase(parent, env_)
{
    envLocal = new EnvLocal("DS_Exam-Action", "EXAM_ACTION", LOG_LRG_SIZE_BYTES);
    monitor = new ExamMonitor(this, env);
    injection = new ExamInjection(this, env);

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });
}

DS_ExamAction::~DS_ExamAction()
{
    delete monitor;
    delete injection;
    delete envLocal;
}

void DS_ExamAction::slotAppInitialised()
{
    connect(env->ds.systemData, &DS_SystemData::signalDataChanged_StatePath, this, [=](DS_SystemDef::StatePath curStatePath, DS_SystemDef::StatePath prevStatePath) {
        if ( ( (prevStatePath == DS_SystemDef::STATE_PATH_IDLE) && (curStatePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING) ) ||
             ( (prevStatePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING) && (curStatePath == DS_SystemDef::STATE_PATH_IDLE) ) )
        {
            LOG_INFO("\n\n-----------------\nStatePath=%s\n\n", ImrParser::ToImr_StatePath(curStatePath).CSTR());
        }
    });

    // if ExtendedSUDSAvailable is true, use Configuration_Behaviors_DefaultSUDSLength, otherwise force SUDS_LENGTH_NORMAL
    connect(env->ds.cfgGlobal, &DS_CfgGlobal::signalConfigChanged_Configuration_Behaviors_ExtendedSUDSAvailable, this, [=](const Config::Item &cfg) {
        bool isExtendedSUDSAvailable = cfg.value.toBool();
        resetSUDSLength(isExtendedSUDSAvailable);
    });
}

DataServiceActionStatus DS_ExamAction::actSetContrastFluidLocation(SyringeIdx contrastSyringeIdx, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "SetContrastFluidLocation", QString().asprintf("%s", ImrParser::ToImr_FluidSourceSyringeIdx(contrastSyringeIdx).CSTR()));

    if ( (contrastSyringeIdx != SYRINGE_IDX_CONTRAST1) &&
         (contrastSyringeIdx != SYRINGE_IDX_CONTRAST2) )
    {
        LOG_ERROR("actSetContrastFluidLocation(): Bad Arg: contrastSyringeIdx=%s\n", ImrParser::ToImr_FluidSourceSyringeIdx(contrastSyringeIdx).CSTR());
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "No Contrast Available";
        actionStarted(status);
        return status;
    }

    DS_DeviceDef::FluidSourceIdx contrastFluidLocation = (contrastSyringeIdx == SYRINGE_IDX_CONTRAST1) ? DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_2 : DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_3;
    return actSetContrastFluidLocation(contrastFluidLocation, actGuid);
}

DataServiceActionStatus DS_ExamAction::actSetContrastFluidLocation(DS_DeviceDef::FluidSourceIdx contrastFluidLocation, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "SetContrastFluidLocation", QString().asprintf("%s", ImrParser::ToImr_FluidSourceIdx(contrastFluidLocation).CSTR()));

    if ( (contrastFluidLocation != DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_2) &&
         (contrastFluidLocation != DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_3) )
    {
        LOG_ERROR("actSetContrastFluidLocation(): Bad Arg: ContrastFluidLocation=%s\n", ImrParser::ToImr_FluidSourceIdx(contrastFluidLocation).CSTR());
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "No Contrast Available";
        actionStarted(status);
        return status;
    }

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);

    DS_ExamDef::ExecutedSteps executedSteps = env->ds.examData->getExecutedSteps();
    DS_SystemDef::StatePath statePath = env->ds.systemData->getStatePath();

    DS_ExamDef::InjectionPlan plan = env->ds.examData->getInjectionPlan();
    plan.setContrastFluidLocation(statePath, executedSteps, contrastFluidLocation);
    env->ds.examData->setInjectionPlan(plan);

    DS_ExamDef::InjectionPlan planPreview = env->ds.examData->getInjectionPlanPreview();
    planPreview.setContrastFluidLocation(statePath, executedSteps, contrastFluidLocation);
    env->ds.examData->setInjectionPlanPreview(planPreview);

    SyringeIdx contrastSyringeIdx = (contrastFluidLocation == DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_2) ? SYRINGE_IDX_CONTRAST1 : SYRINGE_IDX_CONTRAST2;
    env->ds.deviceAction->actSudsSetActiveContrastSyringe(contrastSyringeIdx);

    status.state = DS_ACTION_STATE_COMPLETED;
    actionCompleted(status);
    return status;
}

DataServiceActionStatus DS_ExamAction::actSetContrastFluidLocationByPlan(DS_ExamDef::InjectionPlan plan, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "SetContrastFluidLocationByPlan", QString().asprintf("%s", Util::qVarientToJsonData(ImrParser::ToImr_InjectionPlan(plan)).CSTR()));

    DataServiceActionStatus statusBuf = actInjectionProgramReadyCheck(plan);

    if ( (statusBuf.state == DS_ACTION_STATE_INVALID_STATE) ||
         (statusBuf.state == DS_ACTION_STATE_BAD_REQUEST) )
    {
        actionStarted(statusBuf, &status);
        return statusBuf;
    }

    DS_ExamDef::ExecutedSteps executedSteps = env->ds.examData->getExecutedSteps();
    DS_DeviceDef::FluidInfo selectedContrast = env->ds.examData->getSelectedContrast();
    DS_DeviceDef::FluidSourceIdx newContrastLocation = plan.getCurContrastLocation(env->ds.systemData->getStatePath(), executedSteps);

    if (newContrastLocation != selectedContrast.location)
    {
        if (!env->ds.examData->isContrastSelectAllowed())
        {
            status.state = DS_ACTION_STATE_INVALID_STATE;
            status.err = "Contrast Select Prevented";
            actionStarted(status);
            return status;
        }

        LOG_INFO("actSetContrastFluidLocationByPlan(): Contrast Location is changed: %s -> %s\n", ImrParser::ToImr_FluidSourceIdx(selectedContrast.location).CSTR(), ImrParser::ToImr_FluidSourceIdx(newContrastLocation).CSTR());
        selectedContrast.location = newContrastLocation;
    }

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);

    env->ds.examData->setSelectedContrast(selectedContrast);
    actReloadSelectedContrast();

    status.state = DS_ACTION_STATE_COMPLETED;
    actionCompleted(status);

    return status;
}

DataServiceActionStatus DS_ExamAction::actReloadSelectedContrast(QString brand, double concentration, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "ReloadSelectedContrast", QString().asprintf("%s;%.1f", brand.CSTR(), concentration));

    DS_DeviceDef::FluidInfo selectedContrast = env->ds.examData->getSelectedContrast();
    DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
    bool sameContrasts = env->ds.deviceData->isSameContrastsLoaded();
    bool contrastLoadedSyringe1 = false;
    bool contrastLoadedSyringe2 = false;


    // ============= STEP1: Determine which contrast syringes are loaded
    if ( (brand == "") &&
         (concentration == 0) )
    {
        // Brand not specified
        contrastLoadedSyringe1 = true;
        contrastLoadedSyringe2 = true;
    }
    else
    {
        // Set ContrastLoadedSyringe flags
        for (int syringeIdx = 0; syringeIdx < SYRINGE_IDX_MAX; syringeIdx++)
        {
            if ( (syringeIdx != SYRINGE_IDX_CONTRAST1) &&
                 (syringeIdx != SYRINGE_IDX_CONTRAST2) )
            {
                // Not Contrast Type
                continue;
            }

            DS_DeviceDef::FluidSource *fluidSource = &fluidSourceSyringes[syringeIdx];
            bool *contrastLoadedSyringe = (syringeIdx == SYRINGE_IDX_CONTRAST1) ? &contrastLoadedSyringe1 : &contrastLoadedSyringe2;

            if (fluidSource->sourcePackages.length() > 0)
            {
                DS_DeviceDef::FluidPackage lastUsedFluidPackage = fluidSourceSyringes[syringeIdx].sourcePackages.first();
                if ( (lastUsedFluidPackage.brand == brand) &&
                     (lastUsedFluidPackage.concentration == concentration) )
                {
                    *contrastLoadedSyringe = true;
                }
            }
        }
    }

    // ============= STEP2: Get new location
    DS_DeviceDef::FluidSourceIdx newContrastLocation = selectedContrast.location;
    QString locationChangedReason;

    if ( (contrastLoadedSyringe1) && (contrastLoadedSyringe2) )
    {
        bool autoEmptyC1 = env->ds.cfgGlobal->isAutoEmptyEnabled(SYRINGE_IDX_CONTRAST1);
        bool autoEmptyC2 = env->ds.cfgGlobal->isAutoEmptyEnabled(SYRINGE_IDX_CONTRAST2);
        // When AutoEmpty is performed, sourcePackages from BC are cleared, and sameContrasts is false.
        // contrastAvailable should be set to false in this case and we check for the volume only when autoEmpty is enabled
        bool contrastAvailable1 = (autoEmptyC1) ? Util::isFloatVarGreaterThan(fluidSourceSyringes[SYRINGE_IDX_CONTRAST1].currentAvailableVolumesTotal() , 0.0) : fluidSourceSyringes[SYRINGE_IDX_CONTRAST1].readyForInjection();
        bool contrastAvailable2 = (autoEmptyC2) ? Util::isFloatVarGreaterThan(fluidSourceSyringes[SYRINGE_IDX_CONTRAST2].currentAvailableVolumesTotal() , 0.0) : fluidSourceSyringes[SYRINGE_IDX_CONTRAST2].readyForInjection();

        // set these values so qml side can also read
        env->ds.workflowData->setContrastAvailable1(contrastAvailable1);
        env->ds.workflowData->setContrastAvailable2(contrastAvailable2);

        // Both syringes ok, select (1)isReady && (2)older
        if ( (contrastAvailable1) && (contrastAvailable2) )
        {
            if (sameContrasts)
            {
                double switchOverAdjustmentLimitMin = env->ds.capabilities->get_FluidControl_SyringeVolumeSwitchOverAdjustmentLimitMin();
                newContrastLocation = DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_2;

                // Select preferred one: e.g. loaded earlier
                if ( (fluidSourceSyringes[SYRINGE_IDX_CONTRAST1].currentVolumesTotal() < switchOverAdjustmentLimitMin) &&
                     (fluidSourceSyringes[SYRINGE_IDX_CONTRAST2].currentVolumesTotal() >= switchOverAdjustmentLimitMin) )
                {
                    locationChangedReason = "Only One Contrast With Enough Vol";
                    newContrastLocation = DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_3;
                }
                else if ( (fluidSourceSyringes[SYRINGE_IDX_CONTRAST2].currentVolumesTotal() >= switchOverAdjustmentLimitMin) &&
                          (fluidSourceSyringes[SYRINGE_IDX_CONTRAST1].sourcePackages[0].loadedAtEpochMs > fluidSourceSyringes[SYRINGE_IDX_CONTRAST2].sourcePackages[0].loadedAtEpochMs) )
                {
                    locationChangedReason = "Loaded Ealier";
                    newContrastLocation = DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_3;
                }
                else
                {
                    locationChangedReason = "Default Contrast In Same Supplies";
                }

                // AutoEmpty syringe selection logic: Only when same contast is loaded, choose auto-empty enabled one.
                // newContrastLocation should be ealiest contrast at this point
                // this is XOR logic for boolean. only true when one of them is true
                if (!autoEmptyC1 != !autoEmptyC2)
                {
                    DS_DeviceDef::FluidSourceIdx autoEmptyEnabledFluidSourceIdx = autoEmptyC1 ? DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_2 : DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_3;
                    if (autoEmptyEnabledFluidSourceIdx != newContrastLocation)
                    {
                        locationChangedReason = "Changing to Auto-Empty enabled syringe";
                        newContrastLocation = autoEmptyEnabledFluidSourceIdx;
                    }
                }
            }
        }
        else if (contrastAvailable1)
        {
            newContrastLocation = DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_2;
            locationChangedReason = "Only One Contrast Available";
        }
        else if (contrastAvailable2)
        {
            newContrastLocation = DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_3;
            locationChangedReason = "Only One Contrast Available";
        }
        else
        {
            newContrastLocation = DS_DeviceDef::FLUID_SOURCE_IDX_UNKNOWN;
            locationChangedReason = "No Contrast Available";
        }
    }
    else if (contrastLoadedSyringe1)
    {
        newContrastLocation = DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_2;
        locationChangedReason = "Only One Contrast Loaded";
    }
    else if (contrastLoadedSyringe2)
    {
        newContrastLocation = DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_3;
        locationChangedReason = "Only One Contrast Loaded";
    }

    // ============= STEP3: Select new contrast location
    if (newContrastLocation != selectedContrast.location)
    {
        if (newContrastLocation == DS_DeviceDef::FLUID_SOURCE_IDX_UNKNOWN)
        {
            selectedContrast.unloadFluidPackage();
            env->ds.examData->setSelectedContrast(selectedContrast);
            LOG_INFO("actReloadSelectedContrast(): Contrast Unloaded: Selected Contrast=%s\n", Util::qVarientToJsonData(ImrParser::ToImr_FluidInfo(selectedContrast)).CSTR());
            status.state = DS_ACTION_STATE_INVALID_STATE;
            status.err = "No Contrast Available";
            actionStarted(status);
            return status;
        }

        DS_SystemDef::StatePath statePath = env->ds.systemData->getStatePath();
        if ( (statePath == DS_SystemDef::STATE_PATH_READY_ARMED) ||
             (statePath == DS_SystemDef::STATE_PATH_BUSY_HOLDING) ||
             (statePath == DS_SystemDef::STATE_PATH_BUSY_FINISHING) ||
             (statePath == DS_SystemDef::STATE_PATH_EXECUTING) )
        {
            // Cannot change the current active during arm/injection
            status.state = DS_ACTION_STATE_INVALID_STATE;
            status.err = "Injection In Progress";
            actionStarted(status);
            return status;
        }

        if (!env->ds.examData->isContrastSelectAllowed())
        {
            // Cannot change the current active during arm/injection
            status.state = DS_ACTION_STATE_INVALID_STATE;
            status.err = "Exam In Progress";
            actionStarted(status);
            return status;
        }

        LOG_INFO("actReloadSelectedContrast(): Contrast Location is changed: %s to %s, Reason=%s\n",
                 ImrParser::ToImr_FluidSourceIdx(selectedContrast.location).CSTR(),
                 ImrParser::ToImr_FluidSourceIdx(newContrastLocation).CSTR(),
                 locationChangedReason.CSTR());
        selectedContrast.location = newContrastLocation;
    }

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);

    // ============= STEP4: For selected location, update detail

    SyringeIdx syringeIdx = (selectedContrast.location == DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_2) ? SYRINGE_IDX_CONTRAST1 : SYRINGE_IDX_CONTRAST2;

    if (fluidSourceSyringes[syringeIdx].sourcePackages.length() > 0)
    {
        selectedContrast.fluidPackage = fluidSourceSyringes[syringeIdx].sourcePackages[0];
    }
    else
    {
        selectedContrast.unloadFluidPackage();
    }

    QString colorCode = "GREEN";

    if ( (!sameContrasts) &&
         (selectedContrast.location == DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_3) )
    {
        colorCode = "PURPLE";
    }

    if (selectedContrast.colorCode != colorCode)
    {
        LOG_DEBUG("actReloadSelectedContrast(): Selected Contrast Color Code changed from %s to %s\n", selectedContrast.colorCode.CSTR(), colorCode.CSTR());
        selectedContrast.colorCode = colorCode;
    }

    if (selectedContrast != env->ds.examData->getSelectedContrast())
    {
        LOG_DEBUG("actReloadSelectedContrast(): Selected Contrast=%s\n", Util::qVarientToJsonData(ImrParser::ToImr_FluidInfo(selectedContrast)).CSTR());
        env->ds.examData->setSelectedContrast(selectedContrast);
    }

    actSetContrastFluidLocation(selectedContrast.location);

    status.state = DS_ACTION_STATE_COMPLETED;
    actionCompleted(status);

    return status;
}

DataServiceActionStatus DS_ExamAction::actGetAvailableVolumesForInjection(DS_DeviceDef::FluidSourceIdx contrastFluidLocation, double &salineVolume, double &contrastVolume, double &salineVolumeAvailable, double &contrastVolumeAvailable, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "GetAvailableVolumesForInjection");

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);

    DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();

    // Get Saline Volume
    salineVolume = fluidSourceSyringes[SYRINGE_IDX_SALINE].currentAvailableVolumesTotal();
    salineVolume = ::floor(salineVolume);
    salineVolumeAvailable = salineVolume;

    // Get Contrast Volume
    if (env->ds.deviceData->isSameContrastsLoaded())
    {
        double switchOverAdjustmentLimitMin = env->ds.capabilities->get_FluidControl_SyringeVolumeSwitchOverAdjustmentLimitMin();

        double contrastVolume1 = fluidSourceSyringes[SYRINGE_IDX_CONTRAST1].currentAvailableVolumesTotal();
        double contrastVolumeAvailable1 = (contrastVolume1 < switchOverAdjustmentLimitMin) ? 0 : contrastVolume1;

        double contrastVolume2 = fluidSourceSyringes[SYRINGE_IDX_CONTRAST2].currentAvailableVolumesTotal();
        double contrastVolumeAvailable2 = (contrastVolume2 < switchOverAdjustmentLimitMin) ? 0 : contrastVolume2;

        contrastVolume = ::floor(contrastVolume1) + ::floor(contrastVolume2);
        contrastVolumeAvailable = ::floor(contrastVolumeAvailable1) + ::floor(contrastVolumeAvailable2);
    }
    else
    {
        contrastVolume = (contrastFluidLocation == DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_2) ? fluidSourceSyringes[SYRINGE_IDX_CONTRAST1].currentAvailableVolumesTotal() : fluidSourceSyringes[SYRINGE_IDX_CONTRAST2].currentAvailableVolumesTotal();
        contrastVolumeAvailable = contrastVolume;
    }

    contrastVolume = ::floor(contrastVolume);
    contrastVolumeAvailable = ::floor(contrastVolumeAvailable);

    status.state = DS_ACTION_STATE_COMPLETED;
    actionCompleted(status);

    return status;
}

DataServiceActionStatus DS_ExamAction::actInjectionSelect(DS_ExamDef::InjectionPlan plan, DS_ExamDef::InjectionPlan planTemplate, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "InjectionSelect", QString().asprintf("%s;%s", Util::qVarientToJsonData(ImrParser::ToImr_InjectionPlan(plan)).CSTR(), Util::qVarientToJsonData(ImrParser::ToImr_InjectionPlan(planTemplate)).CSTR()));

    plan.loadFromTemplate(planTemplate);
    status = actInjectionProgram(plan, actGuid);

    if (status.state == DS_ACTION_STATE_COMPLETED)
    {
        status.state = DS_ACTION_STATE_STARTED;
        actionStarted(status);
        status.state = DS_ACTION_STATE_COMPLETED;
        actionCompleted(status);
    }
    return status;
}

DataServiceActionStatus DS_ExamAction::actInjectionProgram(DS_ExamDef::InjectionPlan plan, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "InjectionProgram", QString().asprintf("%s", Util::qVarientToJsonData(ImrParser::ToImr_InjectionPlan(plan)).CSTR()));

    DS_ExamDef::InjectionRequestProcessStatus reqProcessStatus = env->ds.examData->getInjectionRequestProcessStatus();
    if (reqProcessStatus.state == "T_ARMING")
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = QString().asprintf("Bad State: %s", reqProcessStatus.state.CSTR());
        actionStarted(status);
        return status;
    }

    DataServiceActionStatus statusBuf = actInjectionProgramReadyCheck(plan);

    if ( (statusBuf.state == DS_ACTION_STATE_INVALID_STATE) ||
         (statusBuf.state == DS_ACTION_STATE_BAD_REQUEST) )
    {
        actionStarted(statusBuf, &status);
        return statusBuf;
    }

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);

    // Update ProgrammedAt Time for unexectued steps
    DS_ExamDef::ExecutedSteps executedSteps = env->ds.examData->getExecutedSteps();
    int nextExecutedStepIndex = plan.getNextExecuteStepIndex(executedSteps);
    for (int stepIdx = 0; stepIdx < plan.steps.length(); stepIdx++)
    {
        if (stepIdx >= nextExecutedStepIndex)
        {
            plan.steps[stepIdx].programmedAtEpochMs = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
        }
    }

    if (env->ds.examData->getInjectionPlan().templateGuid != plan.templateGuid)
    {
        QString alertData = QString().asprintf("%s;%s", plan.templateGuid.CSTR(), plan.name.CSTR());
        env->ds.alertAction->activate("ProtocolSelected", alertData);
    }


    env->ds.examData->setInjectionPlan(plan);

    status.state = DS_ACTION_STATE_COMPLETED;
    actionCompleted(status);

    return status;
}

DataServiceActionStatus DS_ExamAction::actInjectionProgramReadyCheck(DS_ExamDef::InjectionPlan plan, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "InjectionProgramReadyCheck");

    actionStarted(status);

    // Check statePath
    DS_SystemDef::StatePath statePath = env->ds.systemData->getStatePath();

    if ( (statePath != DS_SystemDef::STATE_PATH_IDLE) &&
         (statePath != DS_SystemDef::STATE_PATH_EXECUTING) )
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "T_PROGRAMFAILED_InvalidState";
        actionStarted(status);
        return status;
    }

    // Check preloading state
    DS_WorkflowDef::PreloadProtocolState preloadProtocolState = env->ds.workflowData->getPreloadProtocolState();
    if (preloadProtocolState != DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_READY)
    {
        // Preload is in progress
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "T_PROGRAMFAILED_InvalidState";
        actionStarted(status);
        return status;
    }

    // Check step counts
    int maxSteps = env->ds.capabilities->get_Hidden_MaxSteps();
    if ( (plan.steps.length() == 0) ||
         (plan.steps.length() > maxSteps) )
    {
        status.state = DS_ACTION_STATE_BAD_REQUEST;
        QString argStr = QString().asprintf("Bad Step Count (%d)", (int)plan.steps.length());
        status.err = "T_PROGRAMFAILED_InvalidProtocol " + argStr;
        actionStarted(status);
        return status;
    }

    // Check if contrast location is an actual contrast location
    DS_ExamDef::ExecutedSteps executedSteps = env->ds.examData->getExecutedSteps();
    DS_DeviceDef::FluidSourceIdx curContrastLocation = plan.getCurContrastLocation(statePath, executedSteps);
    if ( (curContrastLocation != DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_2) &&
         (curContrastLocation != DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_3) )
    {
        status.state = DS_ACTION_STATE_BAD_REQUEST;
        QString argStr = QString().asprintf("Bad Contrast Location (%s)", ImrParser::ToImr_FluidSourceIdx(curContrastLocation).CSTR());
        status.err = "T_PROGRAMFAILED_InvalidProtocol " + argStr;
        actionStarted(status);
        return status;
    }

    // Check phase counts, pressure limit
    int maxPhases = env->ds.capabilities->get_Hidden_MaxPhases();
    for (int stepIdx = 0; stepIdx < plan.steps.length(); stepIdx++)
    {
        if ( (plan.steps[stepIdx].phases.length() == 0) ||
             (plan.steps[stepIdx].phases.length() > maxPhases) )
        {
            status.state = DS_ACTION_STATE_BAD_REQUEST;
            QString argStr = QString().asprintf("Step%d: Bad Phase Count (%d)", stepIdx, (int)plan.steps[stepIdx].phases.length());
            status.err = "T_PROGRAMFAILED_InvalidProtocol " + argStr;
            actionStarted(status);
            return status;
        }

        if ( (plan.steps[stepIdx].pressureLimit < PRESSURE_LIMIT_KPA_MIN) ||
             (plan.steps[stepIdx].pressureLimit > PRESSURE_LIMIT_KPA_MAX) )
        {
            status.state = DS_ACTION_STATE_BAD_REQUEST;
            QString argStr = QString().asprintf("Step%d: Bad Pressure Limit (%dkPa)", stepIdx, plan.steps[stepIdx].pressureLimit);
            status.err = "T_PROGRAMFAILED_InvalidProtocol " + argStr;
            actionStarted(status);
            return status;
        }
    }

    bool planContainsContrastPhase = false;

    quint64 delayMsMin = env->ds.capabilities->get_Hidden_InjectionPhaseDelayMsMin();
    quint64 delayMsMax = env->ds.capabilities->get_Hidden_InjectionPhaseDelayMsMax();
    double flowMin = env->ds.capabilities->get_Hidden_InjectionPhaseFlowRateMin();
    double flowMax = env->ds.capabilities->get_Hidden_InjectionPhaseFlowRateMax();
    double volMin = env->ds.capabilities->get_Hidden_InjectionPhaseVolMin();
    double volMax = env->ds.capabilities->get_Hidden_InjectionPhaseVolMax();

    // Check phase type/volume/flow/duration
    for (int stepIdx = 0; stepIdx < plan.steps.length(); stepIdx++)
    {
        for (int phaseIdx = 0; phaseIdx < plan.steps[stepIdx].phases.length(); phaseIdx++)
        {
            const DS_ExamDef::InjectionPhase *phase = &plan.steps[stepIdx].phases[phaseIdx];

            if (phase->type == DS_ExamDef::INJECTION_PHASE_TYPE_DELAY)
            {
                if ( (phase->durationMs < delayMsMin) ||
                     (phase->durationMs > delayMsMax) )
                {
                    status.state = DS_ACTION_STATE_BAD_REQUEST;
                    QString argStr = QString().asprintf("Step%d: Phase%d: Bad Duration (%llds)", stepIdx, phaseIdx, phase->durationMs / 1000);
                    status.err = "T_PROGRAMFAILED_InvalidProtocol " + argStr;
                    actionStarted(status);
                    return status;
                }
            }
            else if (phase->type == DS_ExamDef::INJECTION_PHASE_TYPE_FLUID)
            {
                planContainsContrastPhase |= (phase->contrastPercentage > 0);

                if ( (Util::isFloatVarGreaterThan(flowMin, phase->flowRate)) ||
                     (Util::isFloatVarGreaterThan(phase->flowRate, flowMax)) )
                {
                    status.state = DS_ACTION_STATE_BAD_REQUEST;
                    QString argStr = QString().asprintf("Step%d: Phase%d: Bad Flow Rate (%.1fml/s)", stepIdx, phaseIdx, phase->flowRate / 1000);
                    status.err = "T_PROGRAMFAILED_InvalidProtocol " + argStr;
                    actionStarted(status);
                    return status;
                }

                double curPhasevolMax = ( (phase->contrastPercentage == 0) || (phase->contrastPercentage == 100) ) ? volMax : (volMax * 2);

                if ( (Util::isFloatVarGreaterThan(volMin, phase->totalVol)) ||
                     (Util::isFloatVarGreaterThan(phase->totalVol, curPhasevolMax)) )
                {
                    status.state = DS_ACTION_STATE_BAD_REQUEST;
                    QString argStr = QString().asprintf("Step%d: Phase%d: Bad Volume (%.1fml)", stepIdx, phaseIdx, phase->totalVol / 1000);
                    status.err = "T_PROGRAMFAILED_InvalidProtocol " + argStr;
                    actionStarted(status);
                    return status;
                }
            }
            else
            {
                status.state = DS_ACTION_STATE_BAD_REQUEST;
                QString argStr = QString().asprintf("Step%d: Phase%d: Bad Type", stepIdx, phaseIdx);
                status.err = "T_PROGRAMFAILED_InvalidProtocol " + argStr;
                actionStarted(status);
                return status;
            }
        }
    }

    if (env->ds.examData->getInjectionPlan().getCurContrastLocation(statePath, executedSteps) != curContrastLocation)
    {
        if (planContainsContrastPhase)
        {
            SyringeIdx contrastSyringeIdx = (curContrastLocation == DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_2) ? SYRINGE_IDX_CONTRAST1 : SYRINGE_IDX_CONTRAST2;
            DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();

            if ( (fluidSourceSyringes[SYRINGE_IDX_CONTRAST1].sourcePackages.length() > 0) ||
                 (fluidSourceSyringes[SYRINGE_IDX_CONTRAST2].sourcePackages.length() > 0) )
            {
                if ( (fluidSourceSyringes[SYRINGE_IDX_CONTRAST1].isReady) ||
                     (fluidSourceSyringes[SYRINGE_IDX_CONTRAST2].isReady) )
                {
                    if (!fluidSourceSyringes[contrastSyringeIdx].isReady)
                    {
                        status.state = DS_ACTION_STATE_BAD_REQUEST;
                        QString argStr = QString().asprintf("Bad Contrast Selected %s", ImrParser::ToImr_FluidSourceIdx(curContrastLocation).CSTR());
                        status.err = "T_PROGRAMFAILED_InvalidProtocol " + argStr;
                        actionStarted(status);
                        return status;
                    }
                }
            }
        }
    }

    actionStarted(status);
    status.state = DS_ACTION_STATE_COMPLETED;
    actionCompleted(status);
    return status;
}

DataServiceActionStatus DS_ExamAction::actInjectionStart(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "InjectionStart");

    DS_ExamDef::InjectionRequestProcessStatus reqProcessStatus = env->ds.examData->getInjectionRequestProcessStatus();
    reqProcessStatus.state = "T_INJECTSTARTING";
    env->ds.examData->setInjectionRequestProcessStatus(reqProcessStatus);

    DS_McuDef::InjectorStatus injectorStatus = env->ds.mcuData->getInjectorStatus();

    // Check Condition: Injector State
    if ( (injectorStatus.state != DS_McuDef::INJECTOR_STATE_READY_START) &&
         (injectorStatus.state != DS_McuDef::INJECTOR_STATE_HOLDING) )
    {
        LOG_ERROR("actInjectionStart(): Invalid State: injectorStatus.state=%s\n", ImrParser::ToImr_InjectorState(injectorStatus.state).CSTR());
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "T_INJECTSTARTFAILED_InvalidState";
        reqProcessStatus.state = status.err;
        env->ds.examData->setInjectionRequestProcessStatus(reqProcessStatus);
        actionStarted(status);
        return status;
    }

    DS_ExamDef::ScannerInterlocks scannerInterlocks = env->ds.examData->getScannerInterlocks();
    DS_CruDef::CruLinkStatus linkStatus = env->ds.cruData->getCruLinkStatus();

    if (injectorStatus.state == DS_McuDef::INJECTOR_STATE_READY_START)
    {
        // Check Condition: Scanner Start Locked Out
        if (linkStatus.state != DS_CruDef::CRU_LINK_STATE_ACTIVE)
        {
            // Link is not up, skip ISI check
        }
        else if (scannerInterlocks.startLockedOut)
        {
            status.state = DS_ACTION_STATE_INVALID_STATE;
            status.err = "T_INJECTSTARTFAILED_ScannerStartLockedOut";
            reqProcessStatus.state = status.err;
            env->ds.examData->setInjectionRequestProcessStatus(reqProcessStatus);
            actionStarted(status);
            return status;
        }
        else if (!scannerInterlocks.scannerReady)
        {
            if ( (scannerInterlocks.interfaceStatus == DS_ExamDef::SCANNER_INTERFACE_STATUS_UNKNOWN) ||
                 (scannerInterlocks.interfaceStatus == DS_ExamDef::SCANNER_INTERFACE_STATUS_DISABLED_PERMANENTLY) ||
                 (scannerInterlocks.interfaceStatus == DS_ExamDef::SCANNER_INTERFACE_STATUS_DISABLED_TEMPORARILY) )
            {
                // Ok to start
            }
            else
            {
                status.state = DS_ACTION_STATE_INVALID_STATE;
                status.err = "T_INJECTSTARTFAILED_ScannerDiagnosticDeviceNotReady";
                reqProcessStatus.state = status.err;
                env->ds.examData->setInjectionRequestProcessStatus(reqProcessStatus);
                actionStarted(status);
                return status;
            }
        }
    }

    if (injectorStatus.state == DS_McuDef::INJECTOR_STATE_HOLDING)
    {
        // Check Condition: Scanner Resume Locked Out
        if (linkStatus.state != DS_CruDef::CRU_LINK_STATE_ACTIVE)
        {
            // Link is not up, skip ISI check
        }
        else if (scannerInterlocks.resumeLockedOut)
        {
            status.state = DS_ACTION_STATE_INVALID_STATE;
            status.err = "T_INJECTSTARTFAILED_ScannerResumeLockedOut";
            reqProcessStatus.state = status.err;
            env->ds.examData->setInjectionRequestProcessStatus(reqProcessStatus);
            actionStarted(status);
            return status;
        }
    }

    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        if (curStatus.err != "")
        {
            DS_ExamDef::InjectionRequestProcessStatus reqProcessStatus = env->ds.examData->getInjectionRequestProcessStatus();
            reqProcessStatus.state = curStatus.err;
            env->ds.examData->setInjectionRequestProcessStatus(reqProcessStatus);
        }
        actionStarted(curStatus, &status);

        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            env->actionMgr->onActionCompleted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
                DS_ExamDef::InjectionRequestProcessStatus reqProcessStatus = env->ds.examData->getInjectionRequestProcessStatus();
                if (curStatus.err != "")
                {
                    reqProcessStatus.state = curStatus.err;
                }
                else
                {
                    reqProcessStatus.state = "T_INJECTSTARTED";
                }
                env->ds.examData->setInjectionRequestProcessStatus(reqProcessStatus);

                env->ds.deviceData->setIsSudsUsed(true);
                actionCompleted(curStatus, &status);
            });
        }
    });

    return env->ds.mcuAction->actInjectStart(guid);
}

DataServiceActionStatus DS_ExamAction::actInjectionStop(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "InjectionStop");

    // Check Condition: StatePath
    DS_SystemDef::StatePath statePath = env->ds.systemData->getStatePath();
    if ( (statePath != DS_SystemDef::STATE_PATH_EXECUTING) &&
         (statePath != DS_SystemDef::STATE_PATH_BUSY_HOLDING) )
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "T_INJECTSTOPFAILED_InvalidState";
        actionStarted(status);
        return status;
    }

    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);

        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            // Update ExecutedSteps
            DS_ExamDef::ExecutedSteps executedSteps = env->ds.examData->getExecutedSteps();
            if (executedSteps.length() > 0)
            {
                DS_ExamDef::ExecutedStep stepProgressDigest = executedSteps.last();
                if (stepProgressDigest.phaseIndex >= 0)
                {
                    stepProgressDigest.phaseProgress[stepProgressDigest.phaseIndex].state = DS_ExamDef::INJECTION_PHASE_PROGRESS_STATE_ABORTED;
                    executedSteps[executedSteps.length() - 1] = stepProgressDigest;
                    env->ds.examData->setExecutedSteps(executedSteps);
                }
            }

            actionCompleted(curStatus, &status);
        }
        else
        {
            LOG_WARNING("actInjectionStop(): Status=%s\n", ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());
        }
    });

    return env->ds.mcuAction->actInjectStop(guid);
}

DataServiceActionStatus DS_ExamAction::actInjectionHold(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "InjectionHold");

    // Check Condition: StatePath
    DS_SystemDef::StatePath statePath = env->ds.systemData->getStatePath();
    if (statePath != DS_SystemDef::STATE_PATH_EXECUTING)
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "T_INJECTHOLDFAILED_InvalidState";
        actionStarted(status);
        return status;
    }

    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);

        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            env->actionMgr->onActionCompleted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
                actionCompleted(curStatus, &status);
            });
        }
    });

    return env->ds.mcuAction->actInjectHold(guid);
}

DataServiceActionStatus DS_ExamAction::actInjectionJump(int jumpToIdx, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "InjectionJump", QString().asprintf("%d", jumpToIdx), "", true);

    DS_SystemDef::StatePath statePath = env->ds.systemData->getStatePath();
    if ( (statePath != DS_SystemDef::STATE_PATH_EXECUTING) &&
         (statePath != DS_SystemDef::STATE_PATH_BUSY_HOLDING) )
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "T_INJECTJUMPFAILED_InvalidState";
        actionStarted(status);
        return status;
    }

    DS_ExamDef::ExecutedSteps executedSteps = env->ds.examData->getExecutedSteps();

    if (executedSteps.length() == 0)
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "T_INJECTJUMPFAILED_InvalidState";
        actionStarted(status);
        return status;
    }

    const DS_ExamDef::InjectionStep *step = env->ds.examData->getExecutingStep();
    DS_ExamDef::ExecutedStep stepProgressDigest = executedSteps.last();

    if (step->isPreloaded)
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "T_INJECTJUMPFAILED_InvalidState";
        actionStarted(status);
        return status;
    }

    if ( (jumpToIdx <= stepProgressDigest.phaseIndex) ||
         (jumpToIdx >= step->phases.length()) )
    {
        status.state = DS_ACTION_STATE_BAD_REQUEST;
        status.err = "T_INJECTJUMPFAILED_InvalidTargetPhase";
        actionStarted(status);
        return status;
    }

    qint64 injectionStartedSinceMs = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch() - stepProgressDigest.stepTriggeredEpochMs;
    if (injectionStartedSinceMs < INJECTION_SKIP_PHASE_ENABLE_WAITING_MS)
    {
        LOG_WARNING("actInjectionJump: Jump rejected: request made too early (%lld < %d)\n", injectionStartedSinceMs, INJECTION_SKIP_PHASE_ENABLE_WAITING_MS);
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "T_INJECTJUMPFAILED_InvalidState";
        actionStarted(status);
        return status;
    }

    int jumpFrom = stepProgressDigest.phaseIndex;

    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);

        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            // Update ExecutedSteps
            DS_ExamDef::ExecutedSteps executedSteps = env->ds.examData->getExecutedSteps();
            if (executedSteps.length() > 0)
            {
                DS_ExamDef::ExecutedStep stepProgressDigest = executedSteps.last();
                if (stepProgressDigest.phaseIndex >= 0)
                {
                    for (int phaseIdx = stepProgressDigest.phaseIndex; phaseIdx < jumpToIdx; phaseIdx++)
                    {
                        // If there isn't a phase progress digest for this index, add it
                        if (phaseIdx >= stepProgressDigest.phaseProgress.length())
                        {
                            DS_ExamDef::PhaseProgressDigest phaseProgressDigest;
                            stepProgressDigest.phaseProgress.append(phaseProgressDigest);
                            LOG_WARNING("actInjectJump(): Missing phaseProgressDigest added. stepProgressDigest.phaseProgress.length=%d\n", (int)stepProgressDigest.phaseProgress.length());
                        }
                        stepProgressDigest.phaseProgress[phaseIdx].state = DS_ExamDef::INJECTION_PHASE_PROGRESS_STATE_JUMPED;
                    }
                    executedSteps[executedSteps.length() - 1] = stepProgressDigest;
                    env->ds.examData->setExecutedSteps(executedSteps);


                    QString alertData = QString().asprintf("%d", jumpFrom);
                    env->ds.alertAction->activate("InjectionPhaseJumped", alertData);
                }
            }
            actionCompleted(curStatus, &status);
        }
        else
        {
            LOG_WARNING("actInjectJump(): Status=%s\n", ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());
        }
    });

    return env->ds.mcuAction->actInjectJump(stepProgressDigest.phaseIndex, jumpToIdx, guid);
}

DataServiceActionStatus DS_ExamAction::actResetPreloadSteps(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "actResetPreloadSteps");
    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);

    DS_ExamDef::InjectionStep clearedStep;

    env->ds.examData->setPreloadedStep1(clearedStep);
    env->ds.examData->setPreloadedStep2(clearedStep);

    DS_ExamDef::InjectionPlan injectionPlan = env->ds.examData->getInjectionPlan();
    DS_ExamDef::ExecutedSteps executedSteps = env->ds.examData->getExecutedSteps();
    for (int stepIdx = executedSteps.length(); stepIdx < injectionPlan.steps.length(); stepIdx++)
    {
        injectionPlan.steps[stepIdx].isPreloaded = false;
    }
    env->ds.examData->setInjectionPlan(injectionPlan);

    status.state = DS_ACTION_STATE_COMPLETED;
    actionCompleted(status);

    return status;
}

DataServiceActionStatus DS_ExamAction::actGetPreloadStepsFromStep(DS_ExamDef::InjectionStep &preloadStep, DS_ExamDef::InjectionStep &injectStep, const DS_ExamDef::InjectionStep &srcStep, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "actGetPreloadStepsFromStep");
    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);

    DS_ExamDef::InjectionStep step = srcStep;

    // Set original indexes
    for (int phaseIdx = 0; phaseIdx < step.phases.length(); phaseIdx++)
    {
        step.phases[phaseIdx].originalPhaseIdx = phaseIdx;
    }

    preloadStep = step;
    preloadStep.isPreloaded = false;
    preloadStep.phases.clear();

    injectStep = step;
    injectStep.isPreloaded = false;
    injectStep.phases.clear();

    // ------------------------------
    // Prepare Preload Step Part1 (preload to patient line for Xml before injection begins)
    preloadStep.pressureLimit = step.pressureLimit;

    // Get first delivery MCU phase
    DS_McuDef::InjectionProtocol mcuInjectProtocol;
    actGetMcuProtocolFromStep(step, mcuInjectProtocol);
    const DS_McuDef::InjectionPhase *firstDeliveryPhase = mcuInjectProtocol.getFirstDeliveryPhase();

    // Get total preload volume
    double preloadVolLeft = getPreloadVolumeForSyringeIdx(SYRINGE_IDX_SALINE);

    if (firstDeliveryPhase != NULL)
    {
        if ( (firstDeliveryPhase->type == DS_McuDef::INJECTION_PHASE_TYPE_CONTRAST1) ||
             (firstDeliveryPhase->type == DS_McuDef::INJECTION_PHASE_TYPE_DUAL1) )
        {
            preloadVolLeft = getPreloadVolumeForSyringeIdx(SYRINGE_IDX_CONTRAST1);
        }
        else if ( (firstDeliveryPhase->type == DS_McuDef::INJECTION_PHASE_TYPE_CONTRAST2) ||
                  (firstDeliveryPhase->type == DS_McuDef::INJECTION_PHASE_TYPE_DUAL2) )
        {
            preloadVolLeft = getPreloadVolumeForSyringeIdx(SYRINGE_IDX_CONTRAST2);
        }
    }


    // Generate preloadStep
    double preloadPhaseFlowRate = env->ds.capabilities->get_Preload_PreloadFlowRate();

    while (step.phases.length() > 0)
    {
        DS_ExamDef::InjectionPhase *phase = &step.phases[0];

        if (phase->type == DS_ExamDef::INJECTION_PHASE_TYPE_FLUID)
        {
            // Add preload phases
            double loadVol = qMin(preloadVolLeft, phase->totalVol);
            DS_ExamDef::InjectionPhase preloadPhase = *phase;
            preloadPhase.totalVol = loadVol;
            preloadPhase.durationMs = (preloadPhase.totalVol * 1000) / preloadPhase.flowRate;
            preloadPhase.originalPhaseIdx = phase->originalPhaseIdx;
            preloadStep.phases.append(preloadPhase);

            // Update remaining volumes
            preloadVolLeft -= loadVol;

            // Update orignial phase (take out preloaded volume)
            phase->totalVol -= loadVol;
            phase->durationMs = (phase->totalVol * 1000) / phase->flowRate;

            if (Util::isFloatVarGreaterThan(phase->totalVol, 0))
            {
                // PL is fully loaded with some remaining phases
                break;
            }
            else
            {
                step.phases.pop_front();
            }

            if (preloadVolLeft <= 0)
            {
                // No volume left for preload part1
                break;
            }
        }
        else
        {
            // Non-delivery phase, don't put to preload part1
            preloadStep.phases.append(*phase);
            step.phases.pop_front();
        }
    }

    // If 'Under Preloaded', fill remaining with Saline.
    if (Util::isFloatVarGreaterThan(preloadVolLeft, 0))
    {
        DS_ExamDef::InjectionPhase phase;
        phase.type = DS_ExamDef::INJECTION_PHASE_TYPE_FLUID;
        phase.totalVol = preloadVolLeft;
        phase.flowRate = preloadPhaseFlowRate;
        phase.durationMs = (phase.totalVol * 1000) / phase.flowRate;
        phase.contrastPercentage = 0;
        phase.originalPhaseIdx = srcStep.phases.length();
        preloadStep.phases.append(phase);
    }

    // ------------------------------
    // Prepare Preload Step Part2
    injectStep.pressureLimit = step.pressureLimit;

    // Prepare ALL phases for Filler & Inject states
    QList<DS_ExamDef::InjectionPhase> fillerPhases, injectPhases;
    fillerPhases = preloadStep.phases + step.phases;
    injectPhases = fillerPhases;

    int fillPhaseIdx = preloadStep.phases.length();
    int injectPhaseIdx = 0;

    while (injectPhaseIdx < injectPhases.length())
    {
        DS_ExamDef::InjectionPhase curPhase;
        DS_ExamDef::InjectionPhase *injectPhase = &injectPhases[injectPhaseIdx];
        DS_ExamDef::InjectionPhase *fillPhase;

        if (injectPhase->type == DS_ExamDef::INJECTION_PHASE_TYPE_DELAY)
        {
            curPhase = *injectPhase;
        }
        else
        {
            if (fillPhaseIdx >= fillerPhases.length())
            {
                // All phases filled
                fillPhase = NULL;
                curPhase.type = DS_ExamDef::INJECTION_PHASE_TYPE_FLUID;
                curPhase.contrastPercentage = 0;
            }
            else
            {
                fillPhase = &fillerPhases[fillPhaseIdx];
                curPhase.type = fillPhase->type;
            }

            if (curPhase.type == DS_ExamDef::INJECTION_PHASE_TYPE_DELAY)
            {
                fillPhaseIdx++;
                // cannot fill pause type, fill for next phase
                continue;
            }
            else
            {
                curPhase.totalVol = (fillPhase == NULL) ? injectPhase->totalVol : qMin(injectPhase->totalVol, fillPhase->totalVol);
                curPhase.contrastPercentage = (fillPhase == NULL) ? 0 : fillPhase->contrastPercentage;
                curPhase.flowRate = injectPhase->flowRate;
                curPhase.durationMs = (curPhase.totalVol * 1000) / curPhase.flowRate;
                curPhase.originalPhaseIdx = injectPhase->originalPhaseIdx;

                if (fillPhase != NULL)
                {
                    if (curPhase.type == DS_ExamDef::INJECTION_PHASE_TYPE_DELAY)
                    {
                        fillPhaseIdx++;
                    }
                    else
                    {
                        fillPhase->totalVol -= curPhase.totalVol;
                        if (fillPhase->totalVol == 0)
                        {
                            fillPhaseIdx++;
                        }
                    }
                }
            }
        }

        if (curPhase.type == DS_ExamDef::INJECTION_PHASE_TYPE_DELAY)
        {
            injectPhaseIdx++;
        }
        else
        {
            injectPhase->totalVol -= curPhase.totalVol;
            if (injectPhase->totalVol == 0)
            {
                injectPhaseIdx++;
            }
        }

        if (curPhase.originalPhaseIdx < srcStep.phases.length())
        {
            // Add current phase to Preload2 only if it is real phase
            injectStep.phases.append(curPhase);
        }
    }

    // For injectStep, Merge same delivering phases
    for (int phaseIdx = 0; phaseIdx < injectStep.phases.length() - 1; phaseIdx++)
    {
        DS_ExamDef::InjectionPhase &curPhase = injectStep.phases[phaseIdx];
        const DS_ExamDef::InjectionPhase &nextPhase = injectStep.phases[phaseIdx + 1];

        if ( (curPhase.type != DS_ExamDef::INJECTION_PHASE_TYPE_DELAY) &&
             (curPhase.type == nextPhase.type) &&
             (curPhase.originalPhaseIdx == nextPhase.originalPhaseIdx) &&
             (curPhase.flowRate == nextPhase.flowRate) &&
             (curPhase.contrastPercentage == nextPhase.contrastPercentage) )
        {
            curPhase.totalVol += nextPhase.totalVol;
            curPhase.durationMs = (curPhase.totalVol * 1000) / curPhase.flowRate;
            injectStep.phases.removeAt(phaseIdx + 1);
            phaseIdx--;
        }
    }

    // Complete Preload Step Part1:
    // 1. Set all flow rate to fixed value
    // 2. Remove delay phases
    for (int phaseIdx = 0; phaseIdx < preloadStep.phases.length(); phaseIdx++)
    {
        DS_ExamDef::InjectionPhase *phase = &preloadStep.phases[phaseIdx];

        if (phase->type == DS_ExamDef::INJECTION_PHASE_TYPE_FLUID)
        {
            phase->flowRate = preloadPhaseFlowRate;
            phase->durationMs = (phase->totalVol * 1000) / phase->flowRate;
        }
        else
        {
            preloadStep.phases.removeAt(phaseIdx);
            phaseIdx--;
        }
    }

    // For preloadStep, Merge same delivering phases
    for (int phaseIdx = 0; phaseIdx < preloadStep.phases.length() - 1; phaseIdx++)
    {
        DS_ExamDef::InjectionPhase &curPhase = preloadStep.phases[phaseIdx];
        const DS_ExamDef::InjectionPhase &nextPhase = preloadStep.phases[phaseIdx + 1];

        if ( (curPhase.type == nextPhase.type) &&
             (curPhase.flowRate == nextPhase.flowRate) &&
             (curPhase.contrastPercentage == nextPhase.contrastPercentage) )
        {
            curPhase.totalVol += nextPhase.totalVol;
            curPhase.durationMs = (curPhase.totalVol * 1000) / curPhase.flowRate;
            preloadStep.phases.removeAt(phaseIdx + 1);
            phaseIdx--;
        }
    }

    status.state = DS_ACTION_STATE_COMPLETED;
    actionCompleted(status);

    LOG_INFO("actGetPreloadStepsFromStep():\n\n");
    LOG_INFO("    InputPhases=%s\n\n", Util::qVarientToJsonData(ImrParser::ToImr_InjectionPhases(srcStep.phases), false).CSTR());
    LOG_INFO("    Preload1Phases=%s\n\n", Util::qVarientToJsonData(ImrParser::ToImr_InjectionPhases(preloadStep.phases), false).CSTR());
    LOG_INFO("    Preload2Phases=%s\n\n", Util::qVarientToJsonData(ImrParser::ToImr_InjectionPhases(injectStep.phases), false).CSTR());

    return status;
}

DataServiceActionStatus DS_ExamAction::actGetMcuProtocolFromStep(const DS_ExamDef::InjectionStep &step, DS_McuDef::InjectionProtocol &injectProtocol, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "GetMcuProtocolFromStep");
    DS_ExamDef::InjectionPlan plan = env->ds.examData->getInjectionPlan();
    DS_ExamDef::ExamField catheterType = env->ds.examData->getExamAdvanceInfo().examInputs["CatheterType"];
    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);

    injectProtocol.init();
    injectProtocol.pressureLimit = step.pressureLimit;
    injectProtocol.maximumFlowRateReduction = env->ds.cfgGlobal->get_Configuration_Behaviors_MaximumFlowRateReduction();
    injectProtocol.pressureLimitSensitivity = env->ds.cfgGlobal->get_Configuration_Behaviors_PressureLimitSensitivity();
    if (plan.injectionSettings.contains("MaximumFlowrateReductionPercentage"))
    {
        injectProtocol.maximumFlowRateReduction = plan.injectionSettings["MaximumFlowrateReductionPercentage"].toInt();
    }

    if (catheterType.name != "")
    {
         QJsonDocument jsonDoc = QJsonDocument::fromJson(catheterType.value.toUtf8());
         QVariantMap jsonMap = jsonDoc.toVariant().toMap();
         QString pressureLimitSensitivityType = jsonMap["PressureLimitSensitivity"].toString();
         //if the string is invalid tyep sensitivy will be PRESSURE_LIMIT_SENSITIVITY_TYPE_INVALID
         injectProtocol.pressureLimitSensitivity = ImrParser::ToCpp_PressureLimitSensitivityType(pressureLimitSensitivityType);
    }

    DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
    double contrastSyringeAvailable1 = fluidSourceSyringes[SYRINGE_IDX_CONTRAST1].currentAvailableVolumesTotal();
    double contrastSyringeAvailable2 = fluidSourceSyringes[SYRINGE_IDX_CONTRAST2].currentAvailableVolumesTotal();
    SyringeIdx preferredContrastSyringe = (step.contrastFluidLocation == DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_2) ? SYRINGE_IDX_CONTRAST1 : SYRINGE_IDX_CONTRAST2;
    double contrastVolumeLeft1 = (preferredContrastSyringe == SYRINGE_IDX_CONTRAST1) ? contrastSyringeAvailable1 : contrastSyringeAvailable2;
    double contrastVolumeLeft2 = 0;

    if (env->ds.deviceData->isSameContrastsLoaded())
    {
        contrastVolumeLeft2 = (preferredContrastSyringe == SYRINGE_IDX_CONTRAST1) ? contrastSyringeAvailable2 : contrastSyringeAvailable1;
    }

    injectProtocol.phases.clear();

    bool crossOverVolumeTrimmed = false;

    for (int hcuPhaseIdx = 0; hcuPhaseIdx < step.phases.length(); hcuPhaseIdx++)
    {
        DS_McuDef::InjectionPhases mcuPhasesBuf;
        bool crossOverVolumeTrimmedForCurPhase = false;
        DataServiceActionStatus statusBuf = actGetMcuPhasesFromHcuPhase(step.phases[hcuPhaseIdx], mcuPhasesBuf, contrastVolumeLeft1, contrastVolumeLeft2, preferredContrastSyringe, crossOverVolumeTrimmedForCurPhase);
        crossOverVolumeTrimmed |= crossOverVolumeTrimmedForCurPhase;

        LOG_DEBUG("actGetMcuProtocolFromStep(): PreferredSyringe=%s, ContrastAvailable={%.1f, %.1f}, HCU_Phase=%s, MCU_Phase=%s, crossOverVolumeTrimmed=%s\n",
                  ImrParser::ToImr_FluidSourceSyringeIdx(preferredContrastSyringe).CSTR(),
                  contrastVolumeLeft1, contrastVolumeLeft2,
                  Util::qVarientToJsonData(ImrParser::ToImr_InjectionPhase(step.phases[hcuPhaseIdx])).CSTR(),
                  Util::qVarientToJsonData(ImrParser::ToImr_McuInjectionPhases(mcuPhasesBuf)).CSTR(),
                  crossOverVolumeTrimmed ? "true" : "false");

        if (statusBuf.state != DS_ACTION_STATE_COMPLETED)
        {
            if (status.state == DS_ACTION_STATE_STARTED)
            {
                // Status needs update
                status.state = statusBuf.state;
                status.err = statusBuf.err;

                if ( (statusBuf.err == "InsufficientVolume") ||
                     (statusBuf.err == "InsufficientVolumeWithCrossOver") )
                {
                    // Conversion failed due to insufficient volume
                    if (crossOverVolumeTrimmed)
                    {
                        // Insufficient volume and one of the phase is converted with trimmed volume due to insufficient volume for cross over
                        status.err = "InsufficientVolumeWithCrossOver";
                    }
                }
                else
                {
                    // This shouldn't happen as arm condition check would have checked it.
                    QString err = QString().asprintf("CREATE_MCU_ARM_PARAMS: Failed to convert phase%d: %s\n", hcuPhaseIdx, ImrParser::ToImr_DataServiceActionStatusStr(status).CSTR());
                    LOG_ERROR("%s", err.CSTR());
                    env->ds.alertAction->activate("HCUInternalSoftwareError", err);
                }
            }
        }

        if (mcuPhasesBuf.length() > 1)
        {
            // Multiple MCU phases required for given HCU phase (injecting with same contrast from two syringes)
            injectProtocol.twoContrastInjectPhaseIndex = injectProtocol.phases.length();
            LOG_DEBUG("actGetMcuProtocolFromStep: twoContrastInjectPhaseIndex = %d\n", injectProtocol.twoContrastInjectPhaseIndex);
        }

        for (int phaseIdx = 0; phaseIdx < mcuPhasesBuf.length(); phaseIdx++)
        {
            injectProtocol.phases.append(mcuPhasesBuf[phaseIdx]);
        }
    }

    actionCompleted(status);
    return status;
}

DataServiceActionStatus DS_ExamAction::actGetHcuPhaseIdxFromMcuPhaseIdx(int &hcuPhaseIdx, int mcuPhaseIdx, const DS_McuDef::InjectionProtocol *mcuInjectProtocol, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "GetHcuPhaseIdxFromMcuPhaseIdx", QString().asprintf("%d", mcuPhaseIdx));

    const DS_ExamDef::InjectionStep *executingStep = env->ds.examData->getExecutingStep();
    if (executingStep == NULL)
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "Invalid ExecutingStep";
        actionStarted(status);
    }

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status, NULL, false);

    DS_McuDef::InjectionProtocol mcuInjProtocolBuf;

    if (mcuInjectProtocol == NULL)
    {
        mcuInjProtocolBuf = env->ds.mcuData->getInjectionProtocol();
        mcuInjectProtocol = &mcuInjProtocolBuf;
    }
    int twoContrastInjectPhaseIndex = mcuInjectProtocol->twoContrastInjectPhaseIndex;
    hcuPhaseIdx = DS_McuDef::getHcuInjectPhaseIdxFromMcuPhaseIdx(mcuPhaseIdx, twoContrastInjectPhaseIndex);

    if (executingStep->isPreloaded)
    {
        // hcuPhaseIdx is phase index for preloadStep2
        DS_ExamDef::InjectionStep preloadStep2 = env->ds.examData->getPreloadedStep2();

        // Get phase index for Original Step
        hcuPhaseIdx = preloadStep2.phases[hcuPhaseIdx].originalPhaseIdx;
    }

    status.state = DS_ACTION_STATE_COMPLETED;
    actionCompleted(status, NULL, false);
    return status;
}

DataServiceActionStatus DS_ExamAction::actGetHcuPhaseInjectedVolsFromMcuInjectDigest(DS_ExamDef::FluidVolumes &injectedVols, const DS_McuDef::InjectDigest &injectDigest, int curPhaseIndex, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "GetHcuPhaseInjectedVolsFromMcuInjectDigest");
    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status, NULL, false);

    const DS_ExamDef::InjectionStep *executingStep = env->ds.examData->getExecutingStep();
    if (executingStep == NULL)
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "Invalid ExecutingStep";
        actionStarted(status, NULL, false);
    }

    injectedVols.init();

    // Update injected volumes
    for (int syringeIdx = 0; syringeIdx < SYRINGE_IDX_MAX; syringeIdx++)
    {
        double injectedVol = 0.0;

        // Get injectedVol for given hcu phase
        // Note, there might be two mcu phase indexes for one hcu phase
        for (int mcuPhaseIdx = 0; mcuPhaseIdx <= injectDigest.phaseIdx; mcuPhaseIdx++)
        {
            int hcuPhaseIdx;
            env->ds.examAction->actGetHcuPhaseIdxFromMcuPhaseIdx(hcuPhaseIdx, mcuPhaseIdx);
            if (hcuPhaseIdx == curPhaseIndex)
            {
                injectedVol += injectDigest.phaseInjectDigests[mcuPhaseIdx].volumes[syringeIdx];
            }
        }
        injectedVols.volumes[syringeIdx] = injectedVol;
    }

    status.state = DS_ACTION_STATE_COMPLETED;
    actionCompleted(status, NULL, false);
    return status;
}

DataServiceActionStatus DS_ExamAction::actGetPreloadedInjectDigest(DS_McuDef::InjectDigest &injectDigestDst, const DS_McuDef::InjectDigest &injectDigestSrc, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "GetPreloadedInjectDigest", QString().asprintf("%s", ImrParser::ToImr_InjectDigestStr(injectDigestSrc).CSTR()));

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status, NULL, false);

    const DS_ExamDef::InjectionStep *executingStep = env->ds.examData->getExecutingStep();
    injectDigestDst = injectDigestSrc;

    for (int mcuPhaseIdx = 0; mcuPhaseIdx <= injectDigestDst.phaseIdx; mcuPhaseIdx++)
    {
        // For each phase, correct the reservoir volumes...

        // Get HCU phase
        int hcuPhaseIdx;
        env->ds.examAction->actGetHcuPhaseIdxFromMcuPhaseIdx(hcuPhaseIdx, mcuPhaseIdx);

        const DS_ExamDef::InjectionPhase *hcuPhase = &executingStep->phases[hcuPhaseIdx];
        DS_McuDef::PhaseInjectDigest *mcuPhaseInjectDigest = &injectDigestDst.phaseInjectDigests[mcuPhaseIdx];

        if (hcuPhase->type == DS_ExamDef::INJECTION_PHASE_TYPE_FLUID)
        {
            double injectedVol = mcuPhaseInjectDigest->getTotalVolume();

            // For delivery phase, update syringe location
            SyringeIdx contrastSyringeIdx1 = (executingStep->contrastFluidLocation == DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_2) ? SYRINGE_IDX_CONTRAST1 : SYRINGE_IDX_CONTRAST2;
            SyringeIdx contrastSyringeIdx2 = (contrastSyringeIdx1 == SYRINGE_IDX_CONTRAST1) ? SYRINGE_IDX_CONTRAST2 : SYRINGE_IDX_CONTRAST1;
            mcuPhaseInjectDigest->volumes[SYRINGE_IDX_SALINE] = injectedVol * (100 - hcuPhase->contrastPercentage) * 0.01;
            mcuPhaseInjectDigest->volumes[contrastSyringeIdx1] = injectedVol * hcuPhase->contrastPercentage * 0.01;
            mcuPhaseInjectDigest->volumes[contrastSyringeIdx2] = 0;
        }
    }

    status.state = DS_ACTION_STATE_COMPLETED;
    actionCompleted(status, NULL, false);
    return status;
}

DataServiceActionStatus DS_ExamAction::actGetPulseSalineVolume(const DS_ExamDef::InjectionStep &step, double &salineVol, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "GetPulseSalineVolume");

    double pulseSalineVol = 0;

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);

    // Prepare HCU phase indexes that may be the MCU pulse dual phase
    QList<int> pulseDualPhaseCandidates; // holds HCU phase index
    const DS_ExamDef::InjectionPhase *prevDeliveringHcuPhase = NULL;
    for (int phaseIdx = 0; phaseIdx < step.phases.length(); phaseIdx++)
    {
        const DS_ExamDef::InjectionPhase *phase = &step.phases[phaseIdx];
        bool isPulseDualPhaseCandidate = false;

        if ( (phase->isDualPhase()) &&
             (phase->contrastPercentage >= 65) )
        {
            bool prevPhaseIsContrastType = ( (prevDeliveringHcuPhase != NULL) && (prevDeliveringHcuPhase->isContrastPhase()) );

            if (!prevPhaseIsContrastType)
            {
                isPulseDualPhaseCandidate = true;
            }
            else if (prevDeliveringHcuPhase->totalVol < 20)
            {
                isPulseDualPhaseCandidate = true;
            }
            else if (Util::isFloatVarGreaterThan(phase->flowRate, prevDeliveringHcuPhase->flowRate))
            {
                isPulseDualPhaseCandidate = true;
            }
        }

        if (isPulseDualPhaseCandidate)
        {
            pulseDualPhaseCandidates.append(phaseIdx);
        }
    }

    if (pulseDualPhaseCandidates.length() == 0)
    {
        // no candidates found
        status.state = DS_ACTION_STATE_COMPLETED;
        actionCompleted(status, NULL, false);
    }

    DS_ExamDef::PulseSalineVolumeLookupRows pulseSalineVolLut = env->ds.examData->getPulseSalineVolumeLookupRows();

    // HCU candidates exist. Further process required
    DS_McuDef::InjectionProtocol mcuProtocol;

    actGetMcuProtocolFromStep(step, mcuProtocol);

    const DS_McuDef::InjectionPhase *prevDeliveringMcuPhase = NULL;

    for (int mcuPhaseIdx = 0; mcuPhaseIdx < mcuProtocol.phases.length(); mcuPhaseIdx++)
    {
        const DS_McuDef::InjectionPhase *mcuPhase = &mcuProtocol.phases[mcuPhaseIdx];

        if ( (mcuPhase->type == DS_McuDef::INJECTION_PHASE_TYPE_DUAL1) ||
             (mcuPhase->type == DS_McuDef::INJECTION_PHASE_TYPE_DUAL2) )
        {
            // Check if current mcu phase is a candidate
            int hcuPhaseIdx;
            actGetHcuPhaseIdxFromMcuPhaseIdx(hcuPhaseIdx, mcuPhaseIdx, &mcuProtocol);

            if (pulseDualPhaseCandidates.contains(hcuPhaseIdx))
            {
                // Check further for pulse dual phase
                bool isPulseDualPhase = false;
                if (prevDeliveringMcuPhase == NULL)
                {
                    LOG_DEBUG("actGetPulseSalineVolume(): McuPhase[%d]: No Previous Delivering phase -> Pulse Dual Phase\n", mcuPhaseIdx);
                    isPulseDualPhase = true;
                }
                else if (mcuPhase->type == prevDeliveringMcuPhase->type)
                {
                    if ( (prevDeliveringMcuPhase->mix < 65) && (mcuPhase->mix >= 65) )
                    {
                        LOG_DEBUG("actGetPulseSalineVolume(): McuPhase[%d]: PreviousDualPhaseMix(%d%%) AND CurrentDualPhaseMix(%d%%) -> Pulse Dual Phase\n", mcuPhaseIdx, prevDeliveringMcuPhase->mix, mcuPhase->mix);
                        isPulseDualPhase = true;
                    }
                    else if ( (prevDeliveringMcuPhase->mix >= 65) &&
                              (mcuPhase->mix >= prevDeliveringMcuPhase->mix) &&
                              (Util::isFloatVarGreaterThan(mcuPhase->flow, prevDeliveringMcuPhase->flow)) )
                    {
                        LOG_DEBUG("actGetPulseSalineVolume(): McuPhase[%d]: PreviousDualPhaseMix(%d%%) AND CurrentDualPhaseMix(%d%%) AND FlowRates(%.1f,%.1f) -> Pulse Dual Phase\n",
                                  mcuPhaseIdx, prevDeliveringMcuPhase->mix, mcuPhase->mix, prevDeliveringMcuPhase->flow, mcuPhase->flow);
                        isPulseDualPhase = true;
                    }
                }

                if (isPulseDualPhase)
                {
                    // Get saline volume
                    for (int lutRow = 0; lutRow < pulseSalineVolLut.length(); lutRow++)
                    {
                        if ( (mcuPhase->mix == pulseSalineVolLut[lutRow].contrastPercentage) &&
                             (::ceil(mcuPhase->flow) == ::ceil(pulseSalineVolLut[lutRow].flowRate)) )
                        {
                            LOG_DEBUG("actGetPulseSalineVolume(): Phase[%d](mix=%d, flow=%.1f): Adding pulseSalineVolLut[%d].salineVol (%.1fml) = %.1fml\n",
                                      mcuPhaseIdx, mcuPhase->mix, mcuPhase->flow,
                                      lutRow, pulseSalineVolLut[lutRow].salineVol, pulseSalineVol);
                            pulseSalineVol += pulseSalineVolLut[lutRow].salineVol;
                        }
                    }
                }
            }
        }

        if ( (mcuPhase->type == DS_McuDef::INJECTION_PHASE_TYPE_CONTRAST1) ||
             (mcuPhase->type == DS_McuDef::INJECTION_PHASE_TYPE_CONTRAST2) ||
             (mcuPhase->type == DS_McuDef::INJECTION_PHASE_TYPE_SALINE) ||
             (mcuPhase->type == DS_McuDef::INJECTION_PHASE_TYPE_DUAL1) ||
             (mcuPhase->type == DS_McuDef::INJECTION_PHASE_TYPE_DUAL2) )
        {
            prevDeliveringMcuPhase = mcuPhase;
        }
    }

    salineVol = pulseSalineVol;

    status.state = DS_ACTION_STATE_COMPLETED;
    actionCompleted(status, NULL, false);
    return status;
}

DataServiceActionStatus DS_ExamAction::actArmReadyCheck(DS_ExamDef::ArmType type, const DS_ExamDef::InjectionStep &curStep, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "ArmReadyCheck", QString().asprintf("%s;%s", ImrParser::ToImr_ArmType(type).CSTR(), Util::qVarientToJsonData(ImrParser::ToImr_InjectionStep(curStep)).CSTR()));

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);

    // Check Condition: StatePath
    DS_SystemDef::StatePath statePath = env->ds.systemData->getStatePath();
    if (statePath != DS_SystemDef::STATE_PATH_IDLE)
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "T_ARMFAILED_StateNotIdle";
        actionCompleted(status);
        return status;
    }

    // checks for all arm types including preload
    if ( !(armReadyCheckIsInjectionValid(curStep, status) &&
           armReadyCheckMudsCheck(status) &&
           armReadyCheckSudsCheck(status) &&
           armReadyCheckP3TUnusableConcentration(status) &&
           armReadyCheckSI2009NonLoadedConcentrationCheck(status) &&
           armReadyCheckSyringeReadyCheck(curStep, status) &&
           armReadyCheckInjectorBusyCheck(status) &&
           armReadyCheckP3TParameters(curStep, status) &&
           armReadyCheckInjectionInsufficientVolumeCheck(type, curStep, status)))
    {
        return status;
    }

    // checks for actual injection (not preload)
    if ( (type == DS_ExamDef::ARM_TYPE_NORMAL) || (type == DS_ExamDef::ARM_TYPE_PRELOAD_SECOND) )
    {
        if ( !(armReadyCheckRemainingStepsInsufficientVolumeCheck(curStep, status) &&
               armReadyCheckWaitForCruDataCheck(status) &&
               armReadyCheckExamInputProgressCheck(status) &&
               armReadyCheckCatheterLimitCheck(curStep, status) &&
               armReadyCheckScannerInterlocksCheck(status) &&
               armReadyCheckAirCheckOK(status)))
        {
            return status;
        }
    }

    status.state = DS_ACTION_STATE_COMPLETED;
    actionCompleted(status);
    return status;
}

DataServiceActionStatus DS_ExamAction::actArm(DS_ExamDef::ArmType type, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "Arm", QString().asprintf("%s", ImrParser::ToImr_ArmType(type).CSTR()));

    DS_ExamDef::InjectionRequestProcessStatus reqProcessStatus = env->ds.examData->getInjectionRequestProcessStatus();
    reqProcessStatus.state = "T_ARMING";
    env->ds.examData->setInjectionRequestProcessStatus(reqProcessStatus);

    if ( (type == DS_ExamDef::ARM_TYPE_NORMAL) ||
         (type == DS_ExamDef::ARM_TYPE_PRELOAD_FIRST) ||
         (type == DS_ExamDef::ARM_TYPE_PRELOAD_SECOND) )
    {
        // Check Condition: Exam Started
        if ( (env->ds.cruData->getLicenseEnabledPatientStudyContext()) &&
             (!env->ds.examData->isExamStarted()) &&
             (env->ds.cruData->getCruLinkStatus().state == DS_CruDef::CRU_LINK_STATE_ACTIVE) )
        {
            // Failed to arm. Arm condition check failed.
            status.state = DS_ACTION_STATE_INVALID_STATE;
            if (type == DS_ExamDef::ARM_TYPE_PRELOAD_FIRST)
            {
                status.err = "T_PreloadProtocolFailed_ExamNotStarted";
            }
            else
            {
                // "T_ARMFAILED_InsufficientVolume" also uses space to include more data. Following this example
                status.err = "T_ARMFAILED_ExamNotStarted";

                if (env->ds.examData->getExamAdvanceInfo().guid != EMPTY_GUID)
                {
                    // NOTE: there is space in front
                    status.err += " T_StartExamFor_XXX";
                }
            }
            reqProcessStatus.state = status.err;
            env->ds.examData->setInjectionRequestProcessStatus(reqProcessStatus);
            actionStarted(status);
            return status;
        }
    }

    DS_ExamDef::ExamProgressState examProgressState = env->ds.examData->getExamProgressState();

    if ( (type == DS_ExamDef::ARM_TYPE_NORMAL) ||
         (type == DS_ExamDef::ARM_TYPE_PRELOAD_FIRST) ||
         (type == DS_ExamDef::ARM_TYPE_PRELOAD_SECOND) )
    {
        if ( (examProgressState == DS_ExamDef::EXAM_PROGRESS_STATE_IDLE) ||
             (examProgressState == DS_ExamDef::EXAM_PROGRESS_STATE_PREPARED) ||
             (examProgressState == DS_ExamDef::EXAM_PROGRESS_STATE_PATIENT_SELECTION) ||
             (examProgressState == DS_ExamDef::EXAM_PROGRESS_STATE_PROTOCOL_SELECTION) ||
             (examProgressState == DS_ExamDef::EXAM_PROGRESS_STATE_PROTOCOL_MODIFICATION) )
        {
            LOG_INFO("actArm: Arm is requested form other source, e.g. CRU\n");
            actExamStart();
        }
    }

    // Reload selected contrast before arm. Make sure the volume check is performed from preferred contrast
    actReloadSelectedContrast();

    // Check for ARM conditions
    const DS_ExamDef::InjectionStep *curStepBuf = env->ds.examData->getExecutingStep();
    if (curStepBuf == NULL)
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        reqProcessStatus.state = status.err;
        env->ds.examData->setInjectionRequestProcessStatus(reqProcessStatus);
        actionStarted(status);
        return status;
    }

    if (curStepBuf->isPreloaded)
    {
        LOG_INFO("actArm(): Current step is already preloaded. Arm with %s type\n", ImrParser::ToImr_ArmType(type).CSTR());
        type = DS_ExamDef::ARM_TYPE_PRELOAD_SECOND;
    }


    // Prepare preload steps
    DS_ExamDef::InjectionStep step = *curStepBuf;
    DS_ExamDef::InjectionStep preloadStep1, preloadStep2;

    if (type == DS_ExamDef::ARM_TYPE_PRELOAD_FIRST)
    {
        actGetPreloadStepsFromStep(preloadStep1, preloadStep2, step);
        env->ds.examData->setPreloadedStep1(preloadStep1);
        env->ds.examData->setPreloadedStep2(preloadStep2);
    }
    else if (type == DS_ExamDef::ARM_TYPE_PRELOAD_SECOND)
    {
        preloadStep2 = env->ds.examData->getPreloadedStep2();
    }

    // Arm check with given step
    DataServiceActionStatus armCheckStatus;
    if (type == DS_ExamDef::ARM_TYPE_PRELOAD_FIRST)
    {
        armCheckStatus = actArmReadyCheck(type, preloadStep1);
    }
    else if (type == DS_ExamDef::ARM_TYPE_PRELOAD_SECOND)
    {
        armCheckStatus = actArmReadyCheck(type, preloadStep2);
    }
    else
    {
        armCheckStatus = actArmReadyCheck(type, step);
    }

    if (armCheckStatus.state != DS_ACTION_STATE_COMPLETED)
    {
        // Failed to arm. Arm condition check failed.
        reqProcessStatus.state = armCheckStatus.err;
        env->ds.examData->setInjectionRequestProcessStatus(reqProcessStatus);
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = armCheckStatus.err;
        actionStarted(status);
        return status;
    }

    // Check fluid removal state and abort if needed
    DS_WorkflowDef::WorkflowState workflowState = env->ds.workflowData->getWorkflowState();
    if (workflowState == DS_WorkflowDef::STATE_FLUID_REMOVAL_PROGRESS)
    {
        LOG_INFO("actArm: ARM requested while workflowState=%s. Aborting Fluid Removal process..\n", ImrParser::ToImr_WorkFlowState(workflowState).CSTR());
        env->ds.workflowAction->actFluidRemovalAbort();
    }

    // Convert HCU Step to MCU Injection Structure
    DS_McuDef::InjectionProtocol mcuInjectProtocol;
    switch (type)
    {
    case DS_ExamDef::ARM_TYPE_PRELOAD_FIRST:
        actGetMcuProtocolFromStep(preloadStep1, mcuInjectProtocol);
        break;
    case DS_ExamDef::ARM_TYPE_PRELOAD_SECOND:
        actGetMcuProtocolFromStep(preloadStep2, mcuInjectProtocol);
        break;
    case DS_ExamDef::ARM_TYPE_NORMAL:
    default:
        actGetMcuProtocolFromStep(step, mcuInjectProtocol);
        break;
    }

    LOG_DEBUG("actArm: Converted MCU Injection =\n%s\n", Util::qVarientToJsonData(ImrParser::ToImr_McuInjectionProtocol(mcuInjectProtocol)).CSTR());

    if (mcuInjectProtocol.phases.length() == 0)
    {
        status.state = DS_ACTION_STATE_INTERNAL_ERR;
        status.err = "T_ARMFAILED_InvalidInjection";
        reqProcessStatus.state = status.err;
        env->ds.examData->setInjectionRequestProcessStatus(reqProcessStatus);
        actionStarted(status);
        return status;
    }

    if (type == DS_ExamDef::ARM_TYPE_PRELOAD_FIRST)
    {
        // Preload shall be processed with non-injection method
        status.state = DS_ACTION_STATE_COMPLETED;
        actionCompleted(status);

        reqProcessStatus.state = "T_ARMED";
        env->ds.examData->setInjectionRequestProcessStatus(reqProcessStatus);

        return status;
    }
    else
    {
        // Send the arm command
        QString mcuActionGuid = Util::newGuid();
        env->actionMgr->onActionStarted(mcuActionGuid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
            if (curStatus.err != "")
            {
                DS_ExamDef::InjectionRequestProcessStatus reqProcessStatus = env->ds.examData->getInjectionRequestProcessStatus();
                reqProcessStatus.state = curStatus.err;
                env->ds.examData->setInjectionRequestProcessStatus(reqProcessStatus);
            }
           actionStarted(curStatus, &status);

           if (curStatus.state == DS_ACTION_STATE_STARTED)
           {
               env->actionMgr->onActionCompleted(mcuActionGuid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
                   DS_ExamDef::InjectionRequestProcessStatus reqProcessStatus = env->ds.examData->getInjectionRequestProcessStatus();
                   if (curStatus.err != "")
                   {
                       reqProcessStatus.state = curStatus.err;
                   }
                   else
                   {
                       reqProcessStatus.state = "T_ARMED";
                       // When armed, clear injectDigest
                       DS_McuDef::InjectDigest injectDigest;
                       injectDigest.init();
                       env->ds.mcuData->setInjectDigest(injectDigest);
                   }
                   env->ds.examData->setInjectionRequestProcessStatus(reqProcessStatus);
                   actionCompleted(curStatus, &status);
               });
           }
        });

        return env->ds.mcuAction->actArm(mcuInjectProtocol, mcuActionGuid);
    }
}

DataServiceActionStatus DS_ExamAction::actDisarm(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "Disarm");

    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);
        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            env->actionMgr->onActionCompleted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
                actionCompleted(curStatus, &status);
            });
        }
    });

    return env->ds.mcuAction->actDisarm(guid);
}

DataServiceActionStatus DS_ExamAction::actGetInjectionTerminationReason(DS_ExamDef::StepTerminationReason &hcuTerminatedReason,
                                                                        QString &mcuTerminatedReason,
                                                                        QString &mcuTerminatedReasonMessage,
                                                                        const DS_McuDef::InjectionCompleteStatus &mcuInjectionCompletedStatus,
                                                                        QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "actGetInjectionTerminationReason");

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);

    switch (mcuInjectionCompletedStatus.reason)
    {
    case DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_NORMAL:
        hcuTerminatedReason = DS_ExamDef::STEP_TERMINATION_REASON_NORMAL;
        break;
    case DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_ALLSTOP_ABORT:
    case DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_USER_ABORT:
        hcuTerminatedReason = DS_ExamDef::STEP_TERMINATION_REASON_ABORTED_BY_REQUEST;
        break;
    case DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_HOLD_TIMEOUT:
        hcuTerminatedReason = DS_ExamDef::STEP_TERMINATION_REASON_ABORTED_BY_HOLD_TIMEOUT;
        break;
    case DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_OVER_PRESSURE:
        hcuTerminatedReason = DS_ExamDef::STEP_TERMINATION_REASON_ABORTED_BY_OVER_PRESSURE;
        break;
    case DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_AIR_DETECTED:
        hcuTerminatedReason = DS_ExamDef::STEP_TERMINATION_REASON_ABORTED_BY_AIR_DETECTION;
        break;
    case DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_SUDS_MISSING:
        hcuTerminatedReason = DS_ExamDef::STEP_TERMINATION_REASON_ABORTED_BY_DISPOSABLE_REMOVAL;
        break;
    case DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_MOTOR_STALL_1:
    case DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_MOTOR_STALL_2:
    case DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_MOTOR_STALL_3:
        hcuTerminatedReason = DS_ExamDef::STEP_TERMINATION_REASON_ABORTED_BY_STALLING;
        break;
    case DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_HCU_COMM_DOWN:
        hcuTerminatedReason = DS_ExamDef::STEP_TERMINATION_REASON_ABORTED_BY_INJECTOR_COMM_LOSS;
        break;
    case DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_BATTERY_CRITICAL:
        hcuTerminatedReason = DS_ExamDef::STEP_TERMINATION_REASON_ABORTED_BY_CRITICAL_BATTERY_LEVEL;
        break;
    case DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_ALARM_ABORT:
    case DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_OVER_CURRENT:
    case DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_OVER_TEMPERATURE:
    case DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_MOTOR_SLIP_1:
    case DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_MOTOR_SLIP_2:
    case DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_MOTOR_SLIP_3:
    case DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_MUDS_UNLATCHED:
    case DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_STOPCOCK_COMM_ERROR:
        hcuTerminatedReason = DS_ExamDef::STEP_TERMINATION_REASON_ABORTED_BY_HARDWARE_FAULT;
        break;
    default:
        hcuTerminatedReason = DS_ExamDef::STEP_TERMINATION_REASON_UNKNOWN;
        break;
    }

    mcuTerminatedReason = ImrParser::ToImr_InjectionCompleteReason(mcuInjectionCompletedStatus.reason);
    mcuTerminatedReasonMessage = mcuInjectionCompletedStatus.message;

    status.state = DS_ACTION_STATE_COMPLETED;
    actionCompleted(status);

    return status;
}

DataServiceActionStatus DS_ExamAction::actScannerInterlocksBypass(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "ScannerInterlocksBypass");

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);

    env->ds.alertAction->activate("ScannerInterlocksBypassed");
    env->ds.examData->setIsScannerInterlocksCheckNeeded(false);

    status.state = DS_ACTION_STATE_COMPLETED;
    actionCompleted(status);

    return status;
}

DataServiceActionStatus DS_ExamAction::actExamReset(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "ExamReset");

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);

    // Reset Exam Guid
    env->ds.examData->setExamGuid(EMPTY_GUID);

    // Reset Exam Advance Info
    DS_ExamDef::ExamAdvanceInfo examAdvanceInfo;
    env->ds.examData->setExamAdvanceInfo(examAdvanceInfo);

    // Reset Step history
    DS_ExamDef::ExecutedSteps executedSteps;
    env->ds.examData->setExecutedSteps(executedSteps);

    // Load plan from template with last contrastFluidLocation
    DS_ExamDef::InjectionPlan plan = env->ds.examData->getInjectionPlan();
    DS_DeviceDef::FluidSourceIdx lastContrastFluidUsed = (plan.steps.length() == 0) ? DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_2 : plan.steps.last().contrastFluidLocation;
    const DS_ExamDef::InjectionPlan *defaultPlanTemplate = env->ds.examData->getDefaultInjectionPlanTemplate();
    plan.loadFromTemplate(*defaultPlanTemplate);
    env->ds.examData->setInjectionPlan(plan);
    actSetContrastFluidLocation(lastContrastFluidUsed);

    env->ds.examData->setIsScannerInterlocksCheckNeeded(true);

    status.state = DS_ACTION_STATE_COMPLETED;
    actionCompleted(status);

    return status;
}

DataServiceActionStatus DS_ExamAction::actExamRestore(QString examGuid,
                                                      qint64 examStartedAtEpochMs,
                                                      DS_ExamDef::ExamProgressState examProgressState,
                                                      DS_ExamDef::InjectionPlan injectionPlan,
                                                      DS_ExamDef::ExecutedSteps executedSteps,
                                                      QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "ExamRestore", QString().asprintf("%s;%s;%s;%s", examGuid.CSTR(), ImrParser::ToImr_ExamProgressState(examProgressState).CSTR(), Util::qVarientToJsonData(ImrParser::ToImr_InjectionPlan(injectionPlan), false).CSTR(), Util::qVarientToJsonData(ImrParser::ToImr_ExecutedSteps(executedSteps), false).CSTR()));

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);

    env->ds.examData->setExamGuid(examGuid);
    env->ds.examData->setExamStartedAtEpochMs(examStartedAtEpochMs);
    env->ds.examData->setExecutedSteps(executedSteps);
    env->ds.examData->setInjectionPlan(injectionPlan);
    env->ds.examData->setExamProgressState(examProgressState);

    status.state = DS_ACTION_STATE_COMPLETED;
    actionCompleted(status);

    return status;
}

DataServiceActionStatus DS_ExamAction::actExamPrepare(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "ExamPrepare");

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);

    actExamReset();

    if (env->ds.cruData->getLicenseEnabledPatientStudyContext())
    {
        env->ds.examData->setExamProgressState(DS_ExamDef::EXAM_PROGRESS_STATE_PATIENT_SELECTION);
    }
    else
    {
        env->ds.examData->setExamProgressState(DS_ExamDef::EXAM_PROGRESS_STATE_PROTOCOL_SELECTION);
    }

    status.state = DS_ACTION_STATE_COMPLETED;
    actionCompleted(status);

    return status;
}

DataServiceActionStatus DS_ExamAction::actExamStart(bool startedFromCru, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "ExamStart");

    if (env->ds.examData->isExamStarted())
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = QString().asprintf("Exam Already Started");
        actionStarted(status);
        return status;
    }

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);

    // Get the exam guid before calling actExamPrepare, as that will reset the exam values
    QString examGuid = env->ds.examData->getExamAdvanceInfo().guid;
    if (examGuid == EMPTY_GUID)
    {
        // Exam guid is not specified. Get new exam guid
        examGuid = Util::newGuid();
    }

    DS_ExamDef::ExamProgressState examProgressState = env->ds.examData->getExamProgressState();
    LOG_INFO("actExamStart(): Exam started from while ExamState is %s..\n", ImrParser::ToImr_ExamProgressState(examProgressState).CSTR());
    if (examProgressState == DS_ExamDef::EXAM_PROGRESS_STATE_IDLE)
    {
        LOG_INFO("actExamStart(): Exam started but exam is not prepared yet. Preparing..\n");
        env->ds.examAction->actExamPrepare();
    }

    env->ds.examData->setExamGuid(examGuid);
    env->ds.alertAction->activate("ExamStarted", examGuid);
    env->ds.examData->setExamStartedAtEpochMs(QDateTime::currentDateTimeUtc().toMSecsSinceEpoch());

    // Get current exam progress state
    env->ds.examData->setExamProgressState(DS_ExamDef::EXAM_PROGRESS_STATE_STARTED);

    // Set next progress state after 'Started'
    if ( (examProgressState == DS_ExamDef::EXAM_PROGRESS_STATE_IDLE) ||
         (examProgressState == DS_ExamDef::EXAM_PROGRESS_STATE_PATIENT_SELECTION) )
    {
        examProgressState = DS_ExamDef::EXAM_PROGRESS_STATE_PROTOCOL_SELECTION;
    }
    LOG_INFO("actExamStart(): Exam started. Set Exam Progress State to %s\n", ImrParser::ToImr_ExamProgressState(examProgressState).CSTR());
    env->ds.examData->setExamProgressState(examProgressState);

    if (startedFromCru)
    {
        LOG_INFO("actExamStart(): Exam started from CRU\n");
    }
    else
    {
        QString guid = Util::newGuid();
        env->actionMgr->onActionCompleted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus status) {
            if (status.state == DS_ACTION_STATE_COMPLETED)
            {
                LOG_INFO("actExamStart(): Start exam is notified to CRU with exam guid(%s).\n", examGuid.CSTR());
            }
            else
            {
                LOG_ERROR("actExamStart(): Failed to start new exam in CRU, err=%s\n", status.err.CSTR());
            }
        });
        env->ds.cruAction->actExamStart(examGuid, guid);
    }

    status.state = DS_ACTION_STATE_COMPLETED;
    actionCompleted(status);

    return status;
}

DataServiceActionStatus DS_ExamAction::actExamEnd(bool endedFromCru, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "ExamEnd");

    DS_ExamDef::ExamProgressState examProgressState = env->ds.examData->getExamProgressState();
    if ( (examProgressState == DS_ExamDef::EXAM_PROGRESS_STATE_IDLE) ||
         (examProgressState == DS_ExamDef::EXAM_PROGRESS_STATE_COMPLETED) )
    {
        // Exam is not started
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "Exam Not Started";
        actionStarted(status);
        LOG_WARNING("actExamEnd(): EndExam requested while examProgressState=%s. Exam cannot be ended.\n", ImrParser::ToImr_ExamProgressState(examProgressState).CSTR());
        return status;
    }

    DS_SystemDef::StatePath statePath = env->ds.systemData->getStatePath();
    if ( (statePath == DS_SystemDef::STATE_PATH_EXECUTING) ||
         (statePath == DS_SystemDef::STATE_PATH_BUSY_HOLDING) ||
         (statePath == DS_SystemDef::STATE_PATH_BUSY_FINISHING) )
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "Injecting";
        actionStarted(status);
        LOG_WARNING("actExamEnd(): EndExam requested while statePath=%s. Exam cannot be ended.\n", ImrParser::ToImr_StatePath(statePath).CSTR());
        return status;
    }

    if (statePath == DS_SystemDef::STATE_PATH_READY_ARMED)
    {
        LOG_WARNING("actExamEnd(): ExamEnd requested while statePath=%s. Setting the statePath to %s before Exam End.\n", ImrParser::ToImr_StatePath(statePath).CSTR(), ImrParser::ToImr_StatePath(DS_SystemDef::STATE_PATH_IDLE).CSTR());
        env->ds.mcuAction->actDisarm();
        env->ds.systemAction->actSetStatePath(DS_SystemDef::STATE_PATH_IDLE);
    }

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);

    QString curExamGuid = env->ds.examData->getExamGuid();
    LOG_INFO("actExamEnd(): Ending Exam (GUID=%s)..\n", curExamGuid.CSTR());

    env->ds.examData->setExamProgressState(DS_ExamDef::EXAM_PROGRESS_STATE_COMPLETED);

    // reset SUDSLength to default
    bool isExtendedSUDSAvailable =  (env->ds.cfgGlobal->get_Configuration_Behaviors_ExtendedSUDSAvailable());
    resetSUDSLength(isExtendedSUDSAvailable);

    // Confirm if disposables need reset
    const DS_ExamDef::InjectionStep *step = env->ds.examData->getExecutingStep();
    if ( (env->ds.examData->isExamStarted()) ||
         ( (step != NULL) && (step->isPreloaded) ) )
    {
        DS_WorkflowDef::SodErrorState sodErrorState = env->ds.workflowData->getSodErrorState();
        LOG_INFO("actExamEnd(): ExamEnd processing while SodError=%s\n", ImrParser::ToImr_SodErrorState(sodErrorState).CSTR());

        // Reset Disposable states
        if ( (sodErrorState == DS_WorkflowDef::SOD_ERROR_STATE_REUSE_MUDS_WAITING_SUDS_REMOVAL) ||
             (sodErrorState == DS_WorkflowDef::SOD_ERROR_STATE_SYRINGE_PRIME_FAILED_SUDS_REMOVED) )
        {
            LOG_WARNING("actExamEnd(): ExamEnd processing while SodError=%s. Error State will not be reset.\n", ImrParser::ToImr_SodErrorState(sodErrorState).CSTR());
        }
        else
        {
            env->ds.deviceAction->actSudsSetNeedsReplace();
            LOG_DEBUG("actExamEnd(): Setting SodErrorState to %s\n", ImrParser::ToImr_SodErrorState(DS_WorkflowDef::SOD_ERROR_STATE_NONE).CSTR());
            env->ds.workflowData->setSodErrorState(DS_WorkflowDef::SOD_ERROR_STATE_NONE);
        }
    }

    if (env->ds.alertAction->isActivated("SUDSReinsertedPrimeRequired", "", true))
    {
        env->ds.alertAction->deactivate("SUDSReinsertedPrimeRequired");
    }

    if (env->ds.alertAction->isActivated("RepreloadReprimeRequired", "", true))
    {
        env->ds.alertAction->deactivate("RepreloadReprimeRequired");
    }

    // Deactivate Alert CatheterLimitsExceededAccepted when exam ended
    if (env->ds.alertAction->isActivated("CatheterLimitsExceededAccepted", "", true))
    {
        env->ds.alertAction->deactivate("CatheterLimitsExceededAccepted");
    }

    if (env->ds.alertAction->isActivated("InsufficientVolumeForStepsAccepted", "", true))
    {
        env->ds.alertAction->deactivate("InsufficientVolumeForStepsAccepted");
    }

    // Clear the workflow error status, it may have outdated priming errors
    DS_WorkflowDef::WorkflowErrorStatus workflowErrorStatus = env->ds.workflowData->getWorkflowErrorStatus();
    workflowErrorStatus.state = DS_WorkflowDef::WORKFLOW_ERROR_STATE_NONE;
    env->ds.workflowData->setWorkflowErrorStatus(workflowErrorStatus);

    if (!env->ds.examData->isExamStarted())
    {
        LOG_INFO("EXAM_END: Exam never started. Silently exiting exam");
        env->ds.examData->setExamProgressState(DS_ExamDef::EXAM_PROGRESS_STATE_IDLE);
    }
    else
    {
        if (endedFromCru)
        {
            LOG_INFO("EXAM_END: Exam is ended from CRU.\n");
            env->ds.examData->setExamProgressState(DS_ExamDef::EXAM_PROGRESS_STATE_IDLE);
        }
        else
        {
            LOG_DEBUG("EXAM_END: Exam is ended from SRU. Notifying CRU that the exam is ended..\n");
            env->ds.examData->setExamGuid(EMPTY_GUID);

            QString guid = Util::newGuid();
            env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
                if (curStatus.state == DS_ACTION_STATE_STARTED)
                {
                    env->actionMgr->onActionCompleted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
                        if (curStatus.state == DS_ACTION_STATE_COMPLETED)
                        {
                            LOG_INFO("EXAM_END: ExamEnd is notified to CRU. Status=%s\n", ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());
                        }
                        else
                        {
                            LOG_ERROR("EXAM_END: Failed to complete ExamEnd in CRU. Status=%s\n", ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());
                        }
                        env->ds.examData->setExamProgressState(DS_ExamDef::EXAM_PROGRESS_STATE_IDLE);
                    });
                }
                else
                {
                    LOG_ERROR("EXAM_END: Failed to start ExamEnd in CRU. Status=%s\n", ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());
                    env->ds.examData->setExamProgressState(DS_ExamDef::EXAM_PROGRESS_STATE_IDLE);
                }
            });

            env->ds.cruAction->actExamEnd(guid);
        }

        // Raise the alert regardless of which component (SRU or CRU) ended the exam
        env->ds.alertAction->activate("ExamEnded", curExamGuid);
    }

    // Must be done before actExamReset;
    storeSyringesUsedInLastExam();

    LOG_DEBUG("EXAM_END: Resetting exam..\n");
    actExamReset();

    LOG_DEBUG("EXAM_END: Auto Refill started..\n");
    actAutoRefill("AfterExam");

    LOG_DEBUG("EXAM_END: Reload Selected Contrast..\n");
    actReloadSelectedContrast();

    status.state = DS_ACTION_STATE_COMPLETED;
    actionCompleted(status);

    env->ds.alertAction->deactivate("InjectionDisarmedByTimeout");
    env->ds.alertAction->deactivate("InjectionDisarmedByCommLoss");
    env->ds.alertAction->deactivate("ISI2InjectionDisarmedByCommLoss");

    return status;
}

DataServiceActionStatus DS_ExamAction::actInjectionAdjustFlowRate(double delta, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "InjectionAdjustFlowRate", QString().asprintf("%.1f", delta));

    DS_SystemDef::StatePath statePath = env->ds.systemData->getStatePath();
    const DS_ExamDef::InjectionStep *executingStep = env->ds.examData->getExecutingStep();
    DS_ExamDef::ExecutedSteps executedSteps = env->ds.examData->getExecutedSteps();

    if (statePath != DS_SystemDef::STATE_PATH_EXECUTING)
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "T_ADJFLOWFAILED_InvalidState";
        actionStarted(status);
        return status;
    }
    else if (executedSteps.length() == 0)
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "T_ADJFLOWFAILED_InvalidState";
        actionStarted(status);
        return status;
    }
    else if ( (executedSteps.last().adaptiveFlowState == DS_ExamDef::ADAPTIVE_FLOW_STATE_ACTIVE) ||
              (executedSteps.last().adaptiveFlowState == DS_ExamDef::ADAPTIVE_FLOW_STATE_CRITICAL) )
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "T_ADJFLOWFAILED_InvalidState";
        actionStarted(status);
        return status;
    }
    else if (executingStep == NULL)
    {
        status.state = DS_ACTION_STATE_INTERNAL_ERR;
        status.err = "T_ADJFLOWFAILED_InvalidInjection";
        actionStarted(status);
        return status;
    }
    else if (!executingStep->isTestInjection)
    {
        status.state = DS_ACTION_STATE_INTERNAL_ERR;
        status.err = "T_ADJFLOWFAILED_NotTestInjection";
        actionStarted(status);
        return status;
    }
    else if (executingStep->phases.length() > 1)
    {
        status.state = DS_ACTION_STATE_INTERNAL_ERR;
        status.err = "T_ADJFLOWFAILED_NotSinglePhaseTestInjection";
        actionStarted(status);
        return status;
    }

    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            // Handle when action completed
            env->actionMgr->onActionCompleted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
                // Prepare executing step for update
                DS_ExamDef::InjectionPlan injectionPlan = env->ds.examData->getInjectionPlan();
                DS_ExamDef::InjectionStep *executingStep = injectionPlan.getExecutingStep(env->ds.systemData->getStatePath(), env->ds.examData->getExecutedSteps());
                DS_ExamDef::ExecutedSteps executedSteps = env->ds.examData->getExecutedSteps();

                if ( (executingStep == NULL) ||
                     (executedSteps.length() == 0) )
                {
                    curStatus.state = DS_ACTION_STATE_INTERNAL_ERR;
                    curStatus.err = "Failed to get executing step";
                    LOG_ERROR("actInjectionAdjustFlowRate(): %s", status.err.CSTR());

                    QString err = curStatus.err;
                    env->ds.alertAction->activate("HCUInternalSoftwareError", err);

                    actionCompleted(curStatus, &status);
                    return;
                }

                DS_ExamDef::ExecutedStep stepProgressDigest = executedSteps.last();
                int curPhaseIdx = stepProgressDigest.phaseIndex;

                // Update MCU Injection
                DS_McuDef::InjectDigest injectDigest = env->ds.mcuData->getInjectDigest();
                DS_McuDef::InjectionProtocol mcuInjProtocol = env->ds.mcuData->getInjectionProtocol();
                double flowMin = env->ds.capabilities->get_Hidden_InjectionPhaseFlowRateMin();
                double flowMax = env->ds.capabilities->get_Hidden_InjectionPhaseFlowRateMax();

                for (int phaseIdx = injectDigest.phaseIdx; phaseIdx < mcuInjProtocol.phases.length(); phaseIdx++)
                {
                     LOG_INFO("actInjectionAdjustFlowRate(): McuInjection.Phase[%d].flowRate = %.1f -> %.1f\n",
                              phaseIdx,
                              mcuInjProtocol.phases[phaseIdx].flow,
                              mcuInjProtocol.phases[phaseIdx].flow + delta);
                     mcuInjProtocol.phases[phaseIdx].flow = qMin(mcuInjProtocol.phases[phaseIdx].flow + delta, flowMax);
                     mcuInjProtocol.phases[phaseIdx].flow = qMax(mcuInjProtocol.phases[phaseIdx].flow, flowMin);
                     mcuInjProtocol.phases[phaseIdx].setDeliveryDuration();
                }
                env->ds.mcuData->setInjectionProtocol(mcuInjProtocol);


                // Update InjectionPlan
                for (int phaseIdx = curPhaseIdx; phaseIdx < executingStep->phases.length(); phaseIdx++)
                {
                     LOG_INFO("actInjectionAdjustFlowRate(): Step.Phase[%d].flowRate = %.1f -> %.1f\n",
                              phaseIdx,
                              executingStep->phases[phaseIdx].flowRate,
                              executingStep->phases[phaseIdx].flowRate + delta);
                     executingStep->phases[phaseIdx].flowRate = qMin(executingStep->phases[phaseIdx].flowRate + delta, flowMax);
                     executingStep->phases[phaseIdx].flowRate = qMax(executingStep->phases[phaseIdx].flowRate, flowMin);
                     executingStep->phases[phaseIdx].setDeliveryDuration();
                }
                env->ds.examAction->actInjectionProgram(injectionPlan);

                // Update ExecutedStep
                for (int phaseIdx = curPhaseIdx; phaseIdx < stepProgressDigest.phaseProgress.length(); phaseIdx++)
                {
                    stepProgressDigest.phaseProgress[phaseIdx].wasFlowAdjusted = true;
                }

                executedSteps[executedSteps.length() - 1] = stepProgressDigest;
                env->ds.examData->setExecutedSteps(executedSteps);

                if (curStatus.state == DS_ACTION_STATE_COMPLETED)
                {
                    env->ds.alertAction->activate("InjectionRateAdjusted", Util::qVarientToJsonData(ImrParser::ToImr_InjectionStep(*executingStep)));
                }

                actionCompleted(curStatus, &status);
            });
        }
        actionStarted(curStatus, &status);
    });

    return env->ds.mcuAction->actAdjflow(delta, guid);
}

DataServiceActionStatus DS_ExamAction::actInjectionRepeat(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "InjectionRepeat");

    DS_ExamDef::InjectionPlan injPlan = env->ds.examData->getInjectionPlan();
    int repeatStepIdx = injPlan.getLastExecutedStepIndex(env->ds.examData->getExecutedSteps());
    if ( (repeatStepIdx >= 0) &&
         (repeatStepIdx < injPlan.steps.length()) )
    {
        // Create new repeated step
        DS_ExamDef::InjectionStep newStep = injPlan.steps[repeatStepIdx];
        newStep.guid = Util::newGuid();
        newStep.isRepeated = true;
        newStep.isPreloaded = false;
        int newStepIdx = repeatStepIdx + 1;

        if (newStepIdx >= injPlan.steps.length())
        {
            injPlan.steps.append(newStep);
        }
        else
        {
            injPlan.steps.insert(newStepIdx, newStep);
        }

        status.state = DS_ACTION_STATE_STARTED;
        actionStarted(status);
        env->ds.examData->setInjectionPlan(injPlan);
        status.state = DS_ACTION_STATE_COMPLETED;
        actionCompleted(status);
    }
    else
    {
        status.state = DS_ACTION_STATE_BAD_REQUEST;
        status.err = "Bad Step Index";
        actionStarted(status);
    }
    return status;
}

DataServiceActionStatus DS_ExamAction::actHandleInsertedInjectionStepOrPhase(DS_ExamDef::InjectionPlan &plan, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "HandleInsertedInjectionStepOrPhase", QString().asprintf("%s", Util::qVarientToJsonData(ImrParser::ToImr_InjectionPlan(plan)).CSTR()));

    DS_ExamDef::InjectionPlan prevPlan = env->ds.examData->getInjectionPlan();

    if (plan == prevPlan)
    {
        // Plan not changed
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "Plan not changed";
        actionStarted(status);
        return status;
    }

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);

    bool insertFound = true;

    if (plan.getTestInjectionCount() > prevPlan.getTestInjectionCount())
    {
        // Test Injection added
        LOG_DEBUG("actHandleInsertedInjectionStepOrPhase(): Test Injection added\n");
        handleInsertedTestInjectionStep(plan);
    }
    else if (plan.steps.length() > prevPlan.steps.length())
    {
        // Injection Step added
        LOG_DEBUG("actHandleInsertedInjectionStepOrPhase(): Injection Step added\n");
        handleInsertedInjectionStep(plan, prevPlan);
    }
    else if (plan.steps.length() < prevPlan.steps.length())
    {
        // Injection Step removed
        LOG_DEBUG("actHandleInsertedInjectionStepOrPhase(): Injection Step removed\n");
    }
    else if (plan.getPhaseCount() == prevPlan.getPhaseCount())
    {
        // Phase is moved from one step to another
        LOG_DEBUG("actHandleInsertedInjectionStepOrPhase(): Phase is moved between steps\n");
    }
    else
    {
        insertFound = false;
        for (int stepIdx = 0; stepIdx < plan.steps.length(); stepIdx++)
        {
            if (plan.steps[stepIdx].phases.length() > prevPlan.steps[stepIdx].phases.length())
            {
                // Injection phase added
                LOG_DEBUG("actHandleInsertedInjectionStepOrPhase(): Phase is added to step[%d]\n", stepIdx);
                handleInsertedInjectionPhase(plan.steps[stepIdx], prevPlan.steps[stepIdx]);
                insertFound = true;
            }
        }
    }

    if (insertFound)
    {
        status.state = DS_ACTION_STATE_COMPLETED;
    }
    else
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "No step or phase inserted";
    }

    actionCompleted(status);
    return status;
}

// returns true if volumes can be adjusted. adjusted step is returned in newStep
bool DS_ExamAction::canAdjustInjectionVolumes(DS_ExamDef::InjectionStep *newStep, QString *err)
{
    // Check Condition: Current step valid check
    DS_ExamDef::InjectionPlan injectionPlan = env->ds.examData->getInjectionPlan();
    DS_ExamDef::InjectionStep *curStepBuf = injectionPlan.getExecutingStep(env->ds.systemData->getStatePath(), env->ds.examData->getExecutedSteps());
    DS_ExamDef::InjectionStep preLoadedStep2;

    if (curStepBuf == NULL)
    {
        *err = "No Step ";
        return false;
    }

    DS_ExamDef::InjectionStep curStep = *curStepBuf;

    double salineVolume, contrastVolume, salineVolumeAvailable, contrastVolumeAvailable;
    if (curStepBuf->isPreloaded)
    {
        preLoadedStep2 = env->ds.examData->getPreloadedStep2();
        LOG_INFO("canAdjustInjectionVolumes(): Current Preloaded Step = \n%s\n\n", Util::qVarientToJsonData(ImrParser::ToImr_InjectionStep(preLoadedStep2), false).CSTR());
        actGetAvailableVolumesForInjection(preLoadedStep2.contrastFluidLocation, salineVolume, contrastVolume, salineVolumeAvailable, contrastVolumeAvailable);

        // For Preload, get difference in volumes from current vs preload injections, and adjust the "available volumes" accordingly.
        // This allows us to use the existing truncation mechanism, without the problematic
        // converstion from truncated-preload-injection to normal injection.
        double contrastDiff = curStep.getContrastTotal() - preLoadedStep2.getContrastTotal() ;
        double salineDiff = curStep.getSalineTotal() - preLoadedStep2.getSalineTotal();

        LOG_INFO("canAdjustInjectionVolumes(): Since step is preloaded, adding %.1f to %.1f for contrast, and adding %.1f to %.1f for saline.\n\n",
                 contrastDiff, contrastVolumeAvailable, salineDiff, salineVolumeAvailable);

        contrastVolumeAvailable += contrastDiff;
        salineVolumeAvailable += salineDiff;

        if (contrastVolumeAvailable < 0)
            contrastVolumeAvailable = 0;
        if (salineVolumeAvailable < 0)
            salineVolumeAvailable = 0;
    }
    else
    {
        LOG_INFO("canAdjustInjectionVolumes(): Current Step = \n%s\n\n", Util::qVarientToJsonData(ImrParser::ToImr_InjectionStep(curStep), false).CSTR());
        actGetAvailableVolumesForInjection(curStep.contrastFluidLocation, salineVolume, contrastVolume, salineVolumeAvailable, contrastVolumeAvailable);
    }

    QString errStr;
    DS_ExamDef::InjectionStep newStep1, newStep2;

    LOG_DEBUG("canAdjustInjectionVolumes(): Volume adjustment1: Start from Saline first..\n");
    newStep1 = getStepWithAdjustedInjectionVolume(curStep, salineVolumeAvailable, false, &errStr);
    if (errStr != "")
    {
        *err = QString().asprintf("Failed to adjust saline phase: %s", errStr.CSTR());
        return false;
    }

    newStep1 = getStepWithAdjustedInjectionVolume(newStep1, contrastVolumeAvailable, true, &errStr);
    if (errStr != "")
    {
        *err = QString().asprintf("Failed to adjust contrast phase: %s", errStr.CSTR());
        return false;
    }

    LOG_DEBUG("canAdjustInjectionVolumes(): Volume adjustment1: Start from Contrast first..\n");
    newStep2 = getStepWithAdjustedInjectionVolume(curStep, contrastVolumeAvailable, true, &errStr);
    if (errStr != "")
    {
        *err = QString().asprintf("Failed to adjust contrast phase: %s", errStr.CSTR());
        return false;
    }

    newStep2 = getStepWithAdjustedInjectionVolume(newStep2, salineVolumeAvailable, false, &errStr);
    if (errStr != "")
    {
        *err = QString().asprintf("Failed to adjust saline phase: %s", errStr.CSTR());
        return false;
    }

    *newStep = ((newStep1.getContrastTotal() + newStep1.getSalineTotal()) >= (newStep2.getContrastTotal() + newStep2.getSalineTotal())) ? newStep1 : newStep2;

    if (*newStep == curStep)
    {
        *err = QString().asprintf("Failed to adjust: Algorithm Error");
        return false;
    }

    if (newStep->phases.length() == 0)
    {
        *err = QString().asprintf("No Phases left after truncating.");
        return false;
    }

    LOG_INFO("canAdjustInjectionVolumes(): New Step = \n%s\n\n", Util::qVarientToJsonData(ImrParser::ToImr_InjectionStep(*newStep), false).CSTR());

    return true;
}

DataServiceActionStatus DS_ExamAction::actAdjustInjectionVolumes(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "AdjustInjectionVolumes");

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);


    DS_ExamDef::ExecutedSteps executedSteps = env->ds.examData->getExecutedSteps();
    DS_SystemDef::StatePath statePath = env->ds.systemData->getStatePath();
    DS_ExamDef::InjectionPlan origPlan = env->ds.examData->getInjectionPlan();
    DS_ExamDef::InjectionStep *origStepBuf = origPlan.getExecutingStep(statePath, executedSteps);

    if (origStepBuf == NULL)
    {
        status.err = "Failed to get executing step ";
        status.state = DS_ACTION_STATE_INVALID_STATE;
        actionCompleted(status);
        return status;
    }

    const DS_ExamDef::InjectionStep origStep = *origStepBuf;

    DS_ExamDef::InjectionStep adjustedStep;
    QString err = "";
    if (canAdjustInjectionVolumes(&adjustedStep, &err))
    {
        DS_ExamDef::InjectionPlan injectionPlan = env->ds.examData->getInjectionPlan();
        DS_ExamDef::InjectionStep *curStep = injectionPlan.getExecutingStep(statePath, executedSteps);

        if (curStep == NULL)
        {
            status.err = "Failed to get new executing step ";
            status.state = DS_ACTION_STATE_INVALID_STATE;
            actionCompleted(status);
            return status;
        }


        // Update plan with new step
        int stepIndex = curStep->getIndex(injectionPlan);
        injectionPlan.steps[stepIndex] = adjustedStep;
        env->ds.examData->setInjectionPlan(injectionPlan);


        // Raise alert
        double origContrastVol = origStep.getContrastTotal();
        double origSalineVol = origStep.getSalineTotal();
        double truncatedContrastVol = origContrastVol - adjustedStep.getContrastTotal();
        double truncatedSalineVol = origSalineVol - adjustedStep.getSalineTotal();

        QString alertData = QString().asprintf("%d;%.1f;%.1f;%.1f;%.1f;%s",
                                              stepIndex, origContrastVol, origSalineVol, truncatedContrastVol, truncatedSalineVol, injectionPlan.guid.CSTR());
        env->ds.alertAction->activate("InjectionTruncated", alertData);

        status.state = DS_ACTION_STATE_COMPLETED;
    }
    else
    {
        status.state = DS_ACTION_STATE_INTERNAL_ERR;
        status.err = err;
    }

    actionCompleted(status);

    return status;
}

DataServiceActionStatus DS_ExamAction::actAutoRefill(QString reason, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "AutoRefill", reason);

    DS_DeviceDef::FluidSource fluidSourceMuds = env->ds.deviceData->getFluidSourceMuds();
    if ( (!fluidSourceMuds.isInstalled()) ||
         (!fluidSourceMuds.isReady) ||
         (fluidSourceMuds.isBusy) )
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "MUDS not ready";
        actionStarted(status);
        return status;
    }

    DS_SystemDef::StatePath statePath = env->ds.systemData->getStatePath();
    if (statePath != DS_SystemDef::STATE_PATH_IDLE)
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "Bad State Path";
        actionStarted(status);
        return status;
    }

    DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();

    LOG_INFO("actAutoRefill(): Refill requested by %s\n", reason.CSTR());

    if (reason == "AfterExam")
    {
        // Refill all syringes
    }
    else if (reason == "AutoPrimeInsufficientFluid")
    {
        // Do not force a system wide refill specifically for priming (saline only) if the saline location is not ready
        if (!env->ds.deviceData->isRefillReady(SYRINGE_IDX_SALINE))
        {
            // Saline syringe is not ready for refill
            LOG_WARNING("actAutoRefill(): SALINE Refill not ready\n");

            status.state = DS_ACTION_STATE_INVALID_STATE;
            status.err = "Refill Not Ready";
            actionStarted(status);
            return status;
        }
    }
    else
    {
        DS_ExamDef::ExamProgressState examProgressState = env->ds.examData->getExamProgressState();

        if ( (examProgressState == DS_ExamDef::EXAM_PROGRESS_STATE_IDLE) ||
             (examProgressState == DS_ExamDef::EXAM_PROGRESS_STATE_PREPARED) ||
             (examProgressState == DS_ExamDef::EXAM_PROGRESS_STATE_PATIENT_SELECTION) ||
             (examProgressState == DS_ExamDef::EXAM_PROGRESS_STATE_PROTOCOL_SELECTION) )
        {
            status.state = DS_ACTION_STATE_INVALID_STATE;
            status.err = "Exam is not in progress";
            actionStarted(status);
            return status;
        }

        // Get current executing step
        const DS_ExamDef::InjectionStep *step = env->ds.examData->getExecutingStep();
        if (step == NULL)
        {
            status.state = DS_ACTION_STATE_INTERNAL_ERR;
            status.err = "Invalid executing step";
            actionStarted(status);
            return status;
        }

        const DS_ExamDef::InjectionStep curStep = *step;

        // Get required fluid volumes for current executing step
        double salineInjectVolume = curStep.getSalineTotal();
        double salineVolumeAvailable = fluidSourceSyringes[SYRINGE_IDX_SALINE].currentVolumesTotal();
        double contrastInjectVolume = curStep.getContrastTotal();
        double contrastVolumeAvailable;
        SyringeIdx contrastSyringe = (curStep.contrastFluidLocation == DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_2) ? SYRINGE_IDX_CONTRAST1 : SYRINGE_IDX_CONTRAST2;
        SyringeIdx contrastSyringe2 = (contrastSyringe == SYRINGE_IDX_CONTRAST1) ? SYRINGE_IDX_CONTRAST2 : SYRINGE_IDX_CONTRAST1;

        if (env->ds.deviceData->isSameContrastsLoaded())
        {
            contrastVolumeAvailable = fluidSourceSyringes[SYRINGE_IDX_CONTRAST1].currentVolumesTotal() + fluidSourceSyringes[SYRINGE_IDX_CONTRAST2].currentVolumesTotal();
        }
        else
        {
            contrastVolumeAvailable = fluidSourceSyringes[contrastSyringe].currentVolumesTotal();
        }

        // Floor both available volumes as the user only sees whole integer values
        salineVolumeAvailable = ::floor(salineVolumeAvailable);
        contrastVolumeAvailable = ::floor(contrastVolumeAvailable);

        // Add extra saline volume for dual phases
        double extraSalineVol;
        env->ds.examAction->actGetPulseSalineVolume(curStep, extraSalineVol);

        if (extraSalineVol > 0)
        {
            LOG_INFO("actAutoRefill(): Extra Saline Volume (%.1fml) required\n", extraSalineVol);
            salineInjectVolume += extraSalineVol;
        }

        // Check required volume
        if ( (salineVolumeAvailable >= salineInjectVolume) &&
             (contrastVolumeAvailable >= contrastInjectVolume) )
        {
            status.state = DS_ACTION_STATE_INVALID_STATE;
            status.err = "Refill Not Required";
            actionStarted(status);
            return status;
        }

        bool salineRefillReady = false;
        bool contrastRefillReady = false;

        if (Util::isFloatVarGreaterThan(salineInjectVolume, salineVolumeAvailable, 1))
        {
            QString syringeStr = ImrParser::ToImr_FluidSourceSyringeIdx(SYRINGE_IDX_SALINE);
            LOG_INFO("actAutoRefill(): Available SalineVolume(%.1fml) < SalineInjectVolume(%.1fml). Refill is required\n", salineVolumeAvailable, salineInjectVolume);

            if (env->ds.deviceData->isRefillReady(SYRINGE_IDX_SALINE))
            {
                LOG_INFO("actAutoRefill(): %s: Refill will start soon..\n", syringeStr.CSTR());
                salineRefillReady = true;
            }
            else
            {
                LOG_WARNING("actAutoRefill(): %s: Refill not ready\n", syringeStr.CSTR());
            }
        }

        if (Util::isFloatVarGreaterThan(contrastInjectVolume, contrastVolumeAvailable, 1))
        {
            QString syringeStr1 = ImrParser::ToImr_FluidSourceSyringeIdx(contrastSyringe);
            QString syringeStr2 = ImrParser::ToImr_FluidSourceSyringeIdx(contrastSyringe2);
            bool sameContrasts = env->ds.deviceData->isSameContrastsLoaded();
            LOG_INFO("actAutoRefill(): Available ContrastVolume(%.1fml) < ContrastInjectVolume(%.1fml). Refill is required\n", contrastVolumeAvailable, contrastInjectVolume);

            if (env->ds.deviceData->isRefillReady(contrastSyringe))
            {
                LOG_INFO("actAutoRefill(): %s: Refill will start soon..\n", syringeStr1.CSTR());
                contrastRefillReady = true;
            }
            else if ( (sameContrasts) &&
                      (env->ds.deviceData->isRefillReady(contrastSyringe2)) )
            {
                LOG_INFO("actAutoRefill(): Primary syringe(%s) is not ready BUT secondary syringe(%s) is ready.\n", syringeStr1.CSTR(), syringeStr2.CSTR());
                LOG_INFO("actAutoRefill(): %s: Refill will start soon..\n", syringeStr2.CSTR());
                contrastRefillReady = true;
            }
            else
            {
                LOG_WARNING("actAutoRefill(): %s: Refill not ready\n", syringeStr1.CSTR());
                if (sameContrasts)
                {
                    LOG_WARNING("actAutoRefill(): %s: Refill not ready\n", syringeStr2.CSTR());
                }
            }
        }

        if ( (!salineRefillReady) &&
             (!contrastRefillReady) )
        {
            // No syringe is ready for refill
            status.state = DS_ACTION_STATE_INVALID_STATE;
            status.err = "Refill Not Ready";
            actionStarted(status);
            return status;
        }
    }

    status.state = DS_ACTION_STATE_START_WAITING;

    // Handle Fill Action for each syringe
    if (actGuid == "")
    {
        actGuid = Util::newGuid();
    }

    FillActionStatus fillActionStatus;
    fillActionStatusMap.insert(actGuid, fillActionStatus);

    for (int syrIdx = 0; syrIdx < SYRINGE_IDX_MAX; syrIdx++)
    {
        SyringeIdx syringeIdx = (SyringeIdx)syrIdx;
        QString syringeIdxStr = ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdx);

        fillActionStatusMap[actGuid].started[syringeIdx] = false;
        fillActionStatusMap[actGuid].completed[syringeIdx] = false;
        fillActionStatusMap[actGuid].actGuid[syringeIdx] = Util::newGuid();

        // Check if syringes are set for auto-empty and skip them
        if (env->ds.cfgGlobal->isAutoEmptyEnabled(syringeIdx))
        {
            LOG_WARNING("actAutoRefill(): %s: Is set for AutoEmpty. Refill not performed\n", syringeIdxStr.CSTR());
            fillActionStatusMap[actGuid].started[syringeIdx] = true;
            fillActionStatusMap[actGuid].completed[syringeIdx] = true;
            continue;
        }

        // Check syringes are ready
        if (!env->ds.deviceData->isRefillReady(syringeIdx))
        {
            LOG_WARNING("actAutoRefill(): %s: Is NOT Ready. Refill not started\n", syringeIdxStr.CSTR());
            fillActionStatusMap[actGuid].started[syringeIdx] = true;
            fillActionStatusMap[actGuid].completed[syringeIdx] = true;
            continue;
        }

        LOG_DEBUG("actAutoRefill(): %s: Refill started\n", syringeIdxStr.CSTR());

        env->actionMgr->onActionStarted(fillActionStatusMap[actGuid].actGuid[syringeIdx], __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
            if (!fillActionStatusMap.contains(actGuid))
            {
                // Action start failed by other syringe
                LOG_WARNING("actAutoRefill(): Action failed by other syringe\n");
                return;
            }

            if ( (curStatus.state != DS_ACTION_STATE_STARTED) &&
                 (curStatus.state != DS_ACTION_STATE_START_WAITING) )
            {
                LOG_ERROR("actAutoRefill(): %s: Failed to start fill action, status=%s\n", syringeIdxStr.CSTR(), ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());
                actionStarted(curStatus, &status);
                fillActionStatusMap.remove(actGuid);
                return;
            }

            LOG_WARNING("actAutoRefill(): %s: Fill Started\n", syringeIdxStr.CSTR());

            if (curStatus.state == DS_ACTION_STATE_STARTED)
            {
                fillActionStatusMap[actGuid].started[syringeIdx] = true;

                // Check all actions started
                bool allActionsStarted = true;
                for (int curSyringeIdx = 0; curSyringeIdx < ARRAY_LEN(fillActionStatusMap[actGuid].started); curSyringeIdx++)
                {
                    if (!fillActionStatusMap[actGuid].started[curSyringeIdx])
                    {
                        allActionsStarted = false;
                        break;
                    }
                }

                if (allActionsStarted)
                {
                    // All actions stared, emit signal
                    actionStarted(curStatus, &status);
                }
            }
        });

        env->actionMgr->onActionCompleted(fillActionStatusMap[actGuid].actGuid[syringeIdx], __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
            if (!fillActionStatusMap.contains(actGuid))
            {
                // Action completed failed by other syringe
                LOG_WARNING("actAutoRefill(): Action failed by other syringe\n");
                return;
            }

            if (curStatus.state != DS_ACTION_STATE_COMPLETED)
            {
                LOG_ERROR("actAutoRefill(): %s: Fill Failed: status=%s\n", syringeIdxStr.CSTR(), ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());
                // Don't complete the AutoFill action yet. Waiting until others are done with filling.
            }
            else
            {
                LOG_INFO("actAutoRefill(): %s: Fill Completed: status=%s\n", syringeIdxStr.CSTR(), ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());
            }

            fillActionStatusMap[actGuid].completed[syringeIdx] = true;

            bool allActionsCompleted = true;
            for (int curSyringeIdx = 0; curSyringeIdx < SYRINGE_IDX_MAX; curSyringeIdx++)
            {
                if (!fillActionStatusMap[actGuid].completed[curSyringeIdx])
                {
                    allActionsCompleted = false;
                    break;
                }
            }

            if (allActionsCompleted)
            {
                // All actions completed, emit signal
                LOG_DEBUG("actAutoRefill(): Completed all fill actions, actGuid=%s\n", actGuid.CSTR());
                // NOTE: Setting state to DS_ACTION_STATE_COMPLETED while there might be failures in some filling.
                curStatus.state = DS_ACTION_STATE_COMPLETED;
                actionCompleted(curStatus, &status);
                fillActionStatusMap.remove(actGuid);
            }
        });
    }

    // Start fill actions
    LOG_DEBUG("actAutoRefill(): Starting Actions....\n");

    bool refillStarted = false;
    for (int syrIdx = 0; syrIdx < SYRINGE_IDX_MAX; syrIdx++)
    {
        SyringeIdx syringeIdx = (SyringeIdx)syrIdx;
        QString syringeIdxStr = ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdx);

        if (!fillActionStatusMap[actGuid].completed[syringeIdx])
        {
            LOG_INFO("actAutoRefill(): %s: Starting Fill: guid=%s\n", syringeIdxStr.CSTR(), fillActionStatusMap[actGuid].actGuid[syringeIdx].CSTR());
            refillStarted = true;
            QTimer::singleShot(1, [=] {
                // Start fill with single timer. Reason: let this loop complete before a possible 'All Action Completed' callback triggered.
                env->ds.deviceAction->actSyringeFill((SyringeIdx)syringeIdx, true, fillActionStatusMap[actGuid].actGuid[syringeIdx]);
            });
        }
        else
        {
            LOG_INFO("actAutoRefill(): %s: FILL NOT REQUIRED!\n", syringeIdxStr.CSTR());
        }
    }

    if (!refillStarted)
    {
        // No refill started from any syringe
        status.err = "Syringe Not Ready";
        status.state = DS_ACTION_STATE_INVALID_STATE;
        actionStarted(status);
    }

    return status;
}

DS_ExamDef::InjectionStep DS_ExamAction::getStepWithAdjustedInjectionVolume(const DS_ExamDef::InjectionStep &curStep, double volumeAvailable, bool isContrast, QString *err)
{
    *err = "";

    volumeAvailable = ::floor(volumeAvailable);
    DS_ExamDef::InjectionStep newStep = curStep;

    while (true)
    {
        double injectVolume;

        if (isContrast)
        {
            injectVolume = newStep.getContrastTotal();
        }
        else
        {
            injectVolume = newStep.getSalineTotal();

            // Add extra saline volume for dual phases
            double extraSalineVol;
            env->ds.examAction->actGetPulseSalineVolume(newStep, extraSalineVol);
            if (extraSalineVol > 0)
            {
                LOG_INFO("getStepWithAdjustedInjectionVolume(): Extra Saline Volume (%.1fml) required\n", extraSalineVol);
                injectVolume += extraSalineVol;
            }
        }
        injectVolume = ::ceil(injectVolume);

        if (volumeAvailable >= injectVolume)
        {
            // fluid is sufficient
            break;
        }

        // Get last fluid phase
        int lastFluidPhaseIdx = -1;

        for (int phaseIdx = newStep.phases.length() - 1; phaseIdx >= 0; phaseIdx--)
        {
            if (newStep.phases[phaseIdx].type == DS_ExamDef::INJECTION_PHASE_TYPE_FLUID)
            {
                if ( (newStep.phases[phaseIdx].isDualPhase()) ||
                     ( (isContrast) && (newStep.phases[phaseIdx].isContrastPhase()) ) ||
                     ( (!isContrast) && (newStep.phases[phaseIdx].isSalinePhase()) ) )
                {
                    lastFluidPhaseIdx = phaseIdx;
                    break;
                }
            }
        }

        if (lastFluidPhaseIdx == -1)
        {
            *err = "Failed to get last fluid phase";
            LOG_WARNING("ADJUST_INJ_VOLUME: Failed to get last fluid phase\n");
            break;;
        }

        DS_ExamDef::InjectionPhase *lastFluidPhase = &newStep.phases[lastFluidPhaseIdx];
        double volumeCut = injectVolume - volumeAvailable;
        int fluidPercent = isContrast ? lastFluidPhase->contrastPercentage : (100 - lastFluidPhase->contrastPercentage);
        double phaseVol = lastFluidPhase->totalVol * fluidPercent * 0.01;
        double otherPhaseVol = lastFluidPhase->totalVol * (100 - fluidPercent) * 0.01;
        double lastPhaseVol = lastFluidPhase->totalVol;

        if (volumeCut >= phaseVol)
        {
            // needs to delete this phase
            LOG_INFO("ADJUST_INJ_VOLUME: %s%d%%: Volume cut required = %.0fml. Last phase volume = %.0fml, Last phase(idx=%d) removed\n",
                     isContrast ? "CONTRAST" : "SALINE", fluidPercent, volumeCut, lastPhaseVol, lastFluidPhaseIdx);
            newStep.phases.removeAt(lastFluidPhaseIdx);
        }
        else if (phaseVol > 0)
        {
            // adjust phase volume
            double newFluidVolume = phaseVol - volumeCut;
            double newOtherFluidVolume = (newFluidVolume * otherPhaseVol) / phaseVol;

            lastFluidPhase->totalVol = newFluidVolume + newOtherFluidVolume;
            lastFluidPhase->setDeliveryDuration();

            LOG_INFO("ADJUST_INJ_VOLUME: %s%d%%: Volume cut required = %.0fml. Last phase volume = %.0fml -> %.0fml\n",
                     isContrast ? "CONTRAST" : "SALINE", fluidPercent, volumeCut, lastPhaseVol, lastFluidPhase->totalVol);

            if (lastFluidPhase->totalVol == lastPhaseVol)
            {
                LOG_WARNING("ADJUST_INJ_VOLUME: Unable to change last phase volume\n");
                break;
            }
        }
    }

    // Floor all phases
    for (int phaseIdx = 0; phaseIdx < newStep.phases.length(); phaseIdx++)
    {
        if (newStep.phases[phaseIdx].type == DS_ExamDef::INJECTION_PHASE_TYPE_FLUID)
        {
            newStep.phases[phaseIdx].totalVol = ::floor(newStep.phases[phaseIdx].totalVol);
            newStep.phases[phaseIdx].setDeliveryDuration();
        }
    }

    if (curStep.phases != newStep.phases)
    {
        LOG_INFO("ADJUST_INJ_VOLUME: %s: Phases Changed\nOld Phases:\n%s\n\nNew Phases:\n%s\n\n",
                 isContrast ? "CONTRAST" : "SALINE",
                 Util::qVarientToJsonData(ImrParser::ToImr_InjectionPhases(curStep.phases), false).CSTR(),
                 Util::qVarientToJsonData(ImrParser::ToImr_InjectionPhases(newStep.phases), false).CSTR());
    }
    else
    {
        LOG_DEBUG("ADJUST_INJ_VOLUME: %s: No Phases Changed\nOld Phases:\n%s\n\n",
                  isContrast ? "CONTRAST" : "SALINE",
                  Util::qVarientToJsonData(ImrParser::ToImr_InjectionPhases(curStep.phases), false).CSTR());
    }

    return newStep;
}

void DS_ExamAction::handleInsertedTestInjectionStep(DS_ExamDef::InjectionPlan &plan)
{
    // Test injection is added: Flow rate should be Max flowrate of all subsequent steps and phases
    int addedStepIdx = env->ds.examData->getExecutedSteps().length();

    // Get maximum flow rate in future steps
    double preferredFlowRate = DEFAULT_PHASE_FLOW;
    for (int stepIdx = addedStepIdx; stepIdx < plan.steps.length(); stepIdx++)
    {
        DS_ExamDef::InjectionStep *step = &plan.steps[stepIdx];
        if (!step->isTestInjection)
        {
            for (int phaseIdx = 0; phaseIdx < step->phases.length(); phaseIdx++)
            {
                if (step->phases[phaseIdx].type == DS_ExamDef::INJECTION_PHASE_TYPE_FLUID)
                {
                    preferredFlowRate = qMax(preferredFlowRate, step->phases[phaseIdx].flowRate);
                }
            }
        }
    }

    // Set maximum flow rate to future test injections
    for (int stepIdx = addedStepIdx; stepIdx < plan.steps.length(); stepIdx++)
    {
        DS_ExamDef::InjectionStep *step = &plan.steps[stepIdx];
        if ( (step->isTestInjection) && (!step->isRepeated) )
        {
            // Reset Template GUID for added (non repeated) injection
            step->templateGuid = EMPTY_GUID;

            for (int phaseIdx = 0; phaseIdx < step->phases.length(); phaseIdx++)
            {
                if (step->phases[phaseIdx].type == DS_ExamDef::INJECTION_PHASE_TYPE_FLUID)
                {
                    step->phases[phaseIdx].flowRate = preferredFlowRate;
                    step->phases[phaseIdx].setDeliveryDuration();
                }
            }
            LOG_INFO("Test injection added, index=%d. Test injection flow rate is set to maximum flow rate(%.1fml/s)\n", stepIdx, preferredFlowRate);
        }
    }
}

void DS_ExamAction::handleInsertedInjectionStep(DS_ExamDef::InjectionPlan &plan, const DS_ExamDef::InjectionPlan &prevPlan)
{
    // Find added injection step index
    int addedStepIdx = plan.steps.length() - 1;
    for (int stepIdx = 0; stepIdx < prevPlan.steps.length(); stepIdx++)
    {
        if (plan.steps[stepIdx].guid != prevPlan.steps[stepIdx].guid)
        {
            addedStepIdx = stepIdx;
            break;
        }
    }

    if (plan.steps[addedStepIdx].isRepeated)
    {
        return;
    }

    double preferredFlowRate = DEFAULT_PHASE_FLOW;

    // Get flow rate of last phase of previous step
    for (int stepIdx = addedStepIdx - 1; stepIdx >= 0; stepIdx--)
    {
        DS_ExamDef::InjectionStep *step = &plan.steps[stepIdx];
        for (int phaseIdx = step->phases.length() - 1; phaseIdx >= 0; phaseIdx--)
        {
            if (step->phases[phaseIdx].type == DS_ExamDef::INJECTION_PHASE_TYPE_FLUID)
            {
                preferredFlowRate = step->phases[phaseIdx].flowRate;
                stepIdx = -1;
                break;
            }
        }
    }

    if (preferredFlowRate == DEFAULT_PHASE_FLOW)
    {
        // If previous method didn't work, get flow rate of first phase of next step
        for (int stepIdx = addedStepIdx + 1; stepIdx < plan.steps.length(); stepIdx++)
        {
            DS_ExamDef::InjectionStep *step = &plan.steps[stepIdx];
            for (int phaseIdx = 0; phaseIdx < step->phases.length(); phaseIdx++)
            {
                if (step->phases[phaseIdx].type == DS_ExamDef::INJECTION_PHASE_TYPE_FLUID)
                {
                    preferredFlowRate = step->phases[phaseIdx].flowRate;
                    stepIdx = plan.steps.length();
                    break;
                }
            }
        }
    }

    // Apply Preferred Flow Rate
    DS_ExamDef::InjectionStep *step = &plan.steps[addedStepIdx];
    for (int phaseIdx = 0; phaseIdx < step->phases.length(); phaseIdx++)
    {
        if (step->phases[phaseIdx].type == DS_ExamDef::INJECTION_PHASE_TYPE_FLUID)
        {
            step->phases[phaseIdx].flowRate = preferredFlowRate;
            step->phases[phaseIdx].setDeliveryDuration();
        }
    }

    // Reset Template GUID for added (non repeated) injection
    step->templateGuid = EMPTY_GUID;

    LOG_INFO("Injection step added, index=%d. Injection flow rate is set to preferred flow rate(%.1fml/s)\n", addedStepIdx, preferredFlowRate);
}

void DS_ExamAction::handleInsertedInjectionPhase(DS_ExamDef::InjectionStep &step, const DS_ExamDef::InjectionStep &prevStep)
{
    // Find added injection phase index
    int addedPhaseIdx = step.phases.length() - 1;
    for (int phaseIdx = 0; phaseIdx < prevStep.phases.length(); phaseIdx++)
    {
        if (step.phases[phaseIdx] != prevStep.phases[phaseIdx])
        {
            addedPhaseIdx = phaseIdx;
            break;
        }
    }

    if (step.phases[addedPhaseIdx].type != DS_ExamDef::INJECTION_PHASE_TYPE_FLUID)
    {
        return;
    }

    double preferredFlowRate = DEFAULT_PHASE_FLOW;
    bool preferredFlowRateFound = false;

    // Get flow rate of previous phase
    for (int phaseIdx = addedPhaseIdx - 1; phaseIdx >= 0; phaseIdx--)
    {
        DS_ExamDef::InjectionPhase *phase = &step.phases[phaseIdx];
        if (phase->type == DS_ExamDef::INJECTION_PHASE_TYPE_FLUID)
        {
            preferredFlowRate = phase->flowRate;
            preferredFlowRateFound = true;
            break;
        }
    }

    if (!preferredFlowRateFound)
    {
        // Preferred flow rate not found. Get flow rate of next phase
        for (int phaseIdx = addedPhaseIdx + 1; phaseIdx < step.phases.length(); phaseIdx++)
        {
            DS_ExamDef::InjectionPhase *phase = &step.phases[phaseIdx];
            if (phase->type == DS_ExamDef::INJECTION_PHASE_TYPE_FLUID)
            {
                preferredFlowRate = phase->flowRate;
                preferredFlowRateFound = true;
                break;
            }
        }
    }

    // Apply Preferred Flow Rate
    DS_ExamDef::InjectionPhase *phase = &step.phases[addedPhaseIdx];
    phase->flowRate = preferredFlowRate;
    phase->setDeliveryDuration();

    if (preferredFlowRateFound)
    {
        LOG_INFO("Injection phase added, index=%d. Injection flow rate is set to preferred flow rate(%.1fml/s)\n", addedPhaseIdx, preferredFlowRate);
    }
    else
    {
        LOG_INFO("Injection phase added, index=%d. Injection flow rate is set to default flow rate(%.1fml/s)\n", addedPhaseIdx, preferredFlowRate);
    }
}


DataServiceActionStatus DS_ExamAction::actGetMcuPhasesFromHcuPhase(const DS_ExamDef::InjectionPhase &hcuPhase, DS_McuDef::InjectionPhases &mcuPhases, double &contrastVolumeAvailable1, double &contrastVolumeAvailable2, SyringeIdx preferredContrastSyringe, bool &crossOverVolumeTrimmed , QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "actGetMcuPhasesFromHcuPhase", QString().asprintf("%s;%.1f;%.1f%s", Util::qVarientToJsonData(ImrParser::ToImr_InjectionPhase(hcuPhase)).CSTR(), contrastVolumeAvailable1, contrastVolumeAvailable2, ImrParser::ToImr_FluidSourceSyringeIdx(preferredContrastSyringe).CSTR()));
    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);

    DS_McuDef::InjectionPhase phaseBuf;

    int phaseIdx = 0;

    if (hcuPhase.type == DS_ExamDef::INJECTION_PHASE_TYPE_DELAY)
    {
        mcuPhases.append(phaseBuf);
        mcuPhases[phaseIdx].type = DS_McuDef::INJECTION_PHASE_TYPE_PAUSE;
        mcuPhases[phaseIdx].vol = 0;
        mcuPhases[phaseIdx].flow = 0;
        mcuPhases[phaseIdx].duration = hcuPhase.durationMs;
        mcuPhases[phaseIdx].mix = 0;
    }
    else if (hcuPhase.type == DS_ExamDef::INJECTION_PHASE_TYPE_FLUID)
    {
        mcuPhases.append(phaseBuf);
        mcuPhases[phaseIdx].mix = hcuPhase.contrastPercentage;

        if (mcuPhases[phaseIdx].mix == 0)
        {
            // Single delivery phase
            mcuPhases[phaseIdx].type = DS_McuDef::INJECTION_PHASE_TYPE_SALINE;
            mcuPhases[phaseIdx].vol = hcuPhase.totalVol;
            mcuPhases[phaseIdx].flow = hcuPhase.flowRate;
            mcuPhases[phaseIdx].duration = hcuPhase.durationMs;
        }
        else
        {
            // Dual Phase
            double injectVolTotal = hcuPhase.totalVol;
            double injectVolContrast = (injectVolTotal * mcuPhases[phaseIdx].mix) / 100;

            if (contrastVolumeAvailable1 == 0)
            {
                // Has to use next syringe
                if (injectVolContrast <= contrastVolumeAvailable2)
                {
                    // Next syringe has enough volume: Use next syringe
                    if (mcuPhases[phaseIdx].mix == 100)
                    {
                        mcuPhases[phaseIdx].type = (preferredContrastSyringe == SYRINGE_IDX_CONTRAST1) ? DS_McuDef::INJECTION_PHASE_TYPE_CONTRAST2 : DS_McuDef::INJECTION_PHASE_TYPE_CONTRAST1;
                    }
                    else
                    {
                        mcuPhases[phaseIdx].type = (preferredContrastSyringe == SYRINGE_IDX_CONTRAST1) ? DS_McuDef::INJECTION_PHASE_TYPE_DUAL2 : DS_McuDef::INJECTION_PHASE_TYPE_DUAL1;
                    }
                    mcuPhases[phaseIdx].vol = injectVolTotal;
                    mcuPhases[phaseIdx].duration = hcuPhase.durationMs;
                    mcuPhases[phaseIdx].flow = hcuPhase.flowRate;
                    contrastVolumeAvailable2 -= injectVolContrast;
                }
                else
                {
                    if (status.state == DS_ACTION_STATE_STARTED)
                    {
                        // Not sufficient volume
                        status.state = DS_ACTION_STATE_INVALID_STATE;
                        status.err = crossOverVolumeTrimmed ? "InsufficientVolumeWithCrossOver" : "InsufficientVolume";
                    }
                }
            }
            else if (injectVolContrast <= contrastVolumeAvailable1)
            {
                // Use preferred contrast
                if (mcuPhases[phaseIdx].mix == 100)
                {
                    mcuPhases[phaseIdx].type = (preferredContrastSyringe == SYRINGE_IDX_CONTRAST1) ? DS_McuDef::INJECTION_PHASE_TYPE_CONTRAST1 : DS_McuDef::INJECTION_PHASE_TYPE_CONTRAST2;
                }
                else
                {
                    mcuPhases[phaseIdx].type = (preferredContrastSyringe == SYRINGE_IDX_CONTRAST1) ? DS_McuDef::INJECTION_PHASE_TYPE_DUAL1 : DS_McuDef::INJECTION_PHASE_TYPE_DUAL2;
                }
                mcuPhases[phaseIdx].vol = injectVolTotal;
                mcuPhases[phaseIdx].duration = hcuPhase.durationMs;
                mcuPhases[phaseIdx].flow = hcuPhase.flowRate;
                contrastVolumeAvailable1 -= injectVolContrast;
            }
            else
            {
                // Use both contrasts. Switchover shall occur
                double switchOverAdjustmentLimitMin = env->ds.capabilities->get_FluidControl_SyringeVolumeSwitchOverAdjustmentLimitMin();
                bool sameContrasts = env->ds.deviceData->isSameContrastsLoaded();

                if ( (contrastVolumeAvailable1 > 0) &&
                     (contrastVolumeAvailable1 < switchOverAdjustmentLimitMin) )
                {
                    // First syringe volume is too small for the switch over. Only use second one.
                    contrastVolumeAvailable1 = 0;
                    mcuPhases.removeLast();
                    if (sameContrasts)
                    {
                        crossOverVolumeTrimmed = true;
                    }
                    return actGetMcuPhasesFromHcuPhase(hcuPhase, mcuPhases, contrastVolumeAvailable1, contrastVolumeAvailable2, preferredContrastSyringe, crossOverVolumeTrimmed, actGuid);
                }

                if (contrastVolumeAvailable2 < switchOverAdjustmentLimitMin)
                {
                    // Second syringe volume is too small for the switch over. Cannot use for switch over.
                    /*if (sameContrasts)
                    {
                        S2SRUSW-3077: Second syringe is too small, don't even try to switch over
                        crossOverVolumeTrimmed = true;
                    }*/
                    contrastVolumeAvailable2 = 0;
                }

                double injectVolContrast1 = contrastVolumeAvailable1;
                double injectVolTotal1 = (injectVolContrast1 * 100) / mcuPhases[phaseIdx].mix;
                double injectVolContrast2 = injectVolContrast - injectVolContrast1;

                if (Util::isFloatVarGreaterThan(injectVolContrast2, contrastVolumeAvailable2, 1))
                {
                    if (status.state == DS_ACTION_STATE_STARTED)
                    {
                        status.state = DS_ACTION_STATE_INVALID_STATE;
                        status.err = crossOverVolumeTrimmed ? "InsufficientVolumeWithCrossOver" : "InsufficientVolume";
                    }
                }

                double injectVolTotal2 = (injectVolContrast2 * 100) / mcuPhases[phaseIdx].mix;

                if (mcuPhases[phaseIdx].mix == 100)
                {
                    mcuPhases[phaseIdx].type = (preferredContrastSyringe == SYRINGE_IDX_CONTRAST1) ? DS_McuDef::INJECTION_PHASE_TYPE_CONTRAST1 : DS_McuDef::INJECTION_PHASE_TYPE_CONTRAST2;
                }
                else
                {
                    mcuPhases[phaseIdx].type = (preferredContrastSyringe == SYRINGE_IDX_CONTRAST1) ? DS_McuDef::INJECTION_PHASE_TYPE_DUAL1 : DS_McuDef::INJECTION_PHASE_TYPE_DUAL2;
                }
                mcuPhases[phaseIdx].vol = injectVolTotal1;
                mcuPhases[phaseIdx].flow = hcuPhase.flowRate;
                mcuPhases[phaseIdx].setDeliveryDuration();
                contrastVolumeAvailable1 -= injectVolContrast1;

                // extra phase needed
                mcuPhases.append(phaseBuf);
                phaseIdx++;

                mcuPhases[phaseIdx].mix = hcuPhase.contrastPercentage;
                if (mcuPhases[phaseIdx].mix == 100)
                {
                    mcuPhases[phaseIdx].type = (preferredContrastSyringe == SYRINGE_IDX_CONTRAST1) ? DS_McuDef::INJECTION_PHASE_TYPE_CONTRAST2 : DS_McuDef::INJECTION_PHASE_TYPE_CONTRAST1;
                }
                else
                {
                    mcuPhases[phaseIdx].type = (preferredContrastSyringe == SYRINGE_IDX_CONTRAST1) ? DS_McuDef::INJECTION_PHASE_TYPE_DUAL2 : DS_McuDef::INJECTION_PHASE_TYPE_DUAL1;
                }
                mcuPhases[phaseIdx].vol = injectVolTotal2;
                mcuPhases[phaseIdx].flow = hcuPhase.flowRate;
                mcuPhases[phaseIdx].setDeliveryDuration();
                contrastVolumeAvailable2 -= injectVolContrast2;
            }
        }
    }

    actionCompleted(status);
    return status;
}

double DS_ExamAction::getPreloadVolumeForSyringeIdx(SyringeIdx idx)
{
    bool extendedSUDS = (env->ds.examData->getSUDSLength() == SUDS_LENGTH_EXTENDED);
    double volume = 0.0f;

    switch (idx)
    {
    case SYRINGE_IDX_SALINE:
        volume = (extendedSUDS ? env->ds.capabilities->get_Preload_SalineVolumeExtended() : env->ds.capabilities->get_Preload_SalineVolume());
        break;
    case SYRINGE_IDX_CONTRAST1:
        volume = (extendedSUDS ? env->ds.capabilities->get_Preload_Contrast1VolumeExtended() : env->ds.capabilities->get_Preload_Contrast1Volume());
        break;
    case SYRINGE_IDX_CONTRAST2:
        volume = (extendedSUDS ? env->ds.capabilities->get_Preload_Contrast2VolumeExtended() : env->ds.capabilities->get_Preload_Contrast2Volume());
        break;
    default:
        LOG_ERROR("getPreloadVolumeForSyringeIdx() : Volume requested for invalid Syringe!\n");
        break;
    }
    return volume;
}

void DS_ExamAction::resetSUDSLength(bool extendedSUDSAAvailable)
{
    if (extendedSUDSAAvailable)
    {
        QString sudsLengthDefault = env->ds.cfgGlobal->get_Configuration_Behaviors_DefaultSUDSLength();
        env->ds.examData->setSUDSLength(sudsLengthDefault);
    }
    else
    {
        // if ExtendedSUDSAvailable is false, force default to normal length
        env->ds.examData->setSUDSLength(SUDS_LENGTH_NORMAL);
    }
}

void DS_ExamAction::storeSyringesUsedInLastExam()
{
    QList<bool> syringesUsed;
    for (int i = (int)SYRINGE_IDX_START; i < (int)SYRINGE_IDX_MAX; i++)
    {
        syringesUsed.append(false);
    }

    DS_ExamDef::ExecutedSteps executedSteps = env->ds.examData->getExecutedSteps();
    for (auto executedStep : executedSteps)
    {
        for (auto phase : executedStep.phaseProgress)
        {
            auto volumes = phase.injectedVolumes.volumes;
            for (int i = 0; i < volumes.length(); i++)
            {
                syringesUsed[i] |=  Util::isFloatVarGreaterThan(volumes[i], 0.0);
            }
        }
    }
    env->ds.workflowData->setSyringesUsedInLastExam(syringesUsed);
    LOG_DEBUG("storeSyringesUsedInLastExam(): Syringes used in this exam = [ RS %s, RC1 %s, RC2 %s ]\n", syringesUsed[SYRINGE_IDX_SALINE] ? "USED" : "NOT_USED", syringesUsed[SYRINGE_IDX_CONTRAST1] ? "USED" : "NOT_USED", syringesUsed[SYRINGE_IDX_CONTRAST2] ? "USED" : "NOT_USED");
}

bool DS_ExamAction::armReadyCheckIsInjectionValid(const DS_ExamDef::InjectionStep &curStep, DataServiceActionStatus &status)
{
    DS_ExamDef::InjectionPlan plan = env->ds.examData->getInjectionPlan();
    int stepIndex = plan.getStepIdxFromStepGuid(curStep.guid);
    if (stepIndex == -1) {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "T_ARMFAILED_InvalidInjection";
        actionCompleted(status);
        return false;
    }

    // Check Condition: Current step valid check
    for (int phaseIdx = 0; phaseIdx < curStep.phases.length(); phaseIdx++)
    {
        if ( (curStep.phases[phaseIdx].type != DS_ExamDef::INJECTION_PHASE_TYPE_DELAY) &&
             (curStep.phases[phaseIdx].type != DS_ExamDef::INJECTION_PHASE_TYPE_FLUID) )
        {
            status.state = DS_ACTION_STATE_BAD_REQUEST;
            status.err = "T_ARMFAILED_InvalidInjection";
            actionCompleted(status);
            return false;
        }
    }

    return true;
}

bool DS_ExamAction::armReadyCheckMudsCheck(DataServiceActionStatus &status)
{
    // Check Condition: is Muds inserted
    if (!env->ds.mcuData->getMudsInserted())
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "T_ARMFAILED_MUDSNotInserted";
        actionCompleted(status);
        return false;
    }

    // Check Condition: MUDS Life
    if (env->ds.alertAction->isActivated("MUDSUseLifeLimitEnforced"))
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "T_ARMFAILED_MUDSUseLifeExceeded";
        actionCompleted(status);
        return false;
    }
    return true;
}

bool DS_ExamAction::armReadyCheckSudsCheck(DataServiceActionStatus &status)
{
    // Check Condition: SUDS connection
    if (!env->ds.mcuData->getSudsInserted())
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "T_ARMFAILED_PatientLineNotConnected";
        actionCompleted(status);
        return false;
    }

    // Check Condition: SUDS primed state
    DS_DeviceDef::FluidSource fluidSourceSuds = env->ds.deviceData->getFluidSourceSuds();
    if (fluidSourceSuds.needsReplaced)
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "T_ARMFAILED_PatientLineNeedsReplaced";
        actionCompleted(status);
        return false;
    }

    // Check Condition: SUDS is primed
    if (!fluidSourceSuds.isReady)
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "T_ARMFAILED_PatientLineNotReady";
        actionCompleted(status);
        return false;
    }

    // Check Condition: Repreload or Reprime Required
    if (env->ds.alertAction->isActivated("RepreloadReprimeRequired", "", true))
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "T_ARMFAILED_PatientLineNotReady";
        actionCompleted(status);
        return false;
    }

    return true;
}


bool DS_ExamAction::armReadyCheckWaitForCruDataCheck(DataServiceActionStatus &status)
{
    // CRU is connected but examInputsProgress hasn't been sent through
    if ( (env->ds.cruData->getCruLinkStatus().state == DS_CruDef::CRU_LINK_STATE_ACTIVE) &&
         (env->ds.cruData->getLicenseEnabledPatientStudyContext()) &&
         (env->ds.examData->getExamAdvanceInfo().examInputsProgress == -1) )
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "T_ARMFAILED_ExamSyncNeeded";
        actionCompleted(status);
        return false;
    }
    return true;
}

bool DS_ExamAction::armReadyCheckExamInputProgressCheck(DataServiceActionStatus &status)
{
    // Check Condition: License enabled and Patient Mendatory field Not Populated (ExamInputsProgress != 100)
    if ( (env->ds.cruData->getCruLinkStatus().state == DS_CruDef::CRU_LINK_STATE_ACTIVE) &&
         (env->ds.cruData->getLicenseEnabledPatientStudyContext()) &&
         (env->ds.examData->getExamAdvanceInfo().examInputsProgress != 100) )
    {         
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "T_ARMFAILED_ExamInputsNotPopulated";
        actionCompleted(status);
        return false;
    }
    return true;
}


bool DS_ExamAction::armReadyCheckP3TParameters(const DS_ExamDef::InjectionStep &curStep, DataServiceActionStatus &status)
{
    DS_ExamDef::InjectionPlan plan = env->ds.examData->getInjectionPlan();

    // Check Condition: P3T Parameters Not Populated only if DeparameterizedProtocol alert is not active
    if (!env->ds.alertAction->isActivated("DeparameterizedProtocol", "", true))
    {
        for (int inputIdx = 0; inputIdx < plan.personalizationInputs.length(); inputIdx++)
        {
            if (!plan.personalizationInputs[inputIdx].isValueVisible)
            {
                status.state = DS_ACTION_STATE_INVALID_STATE;
                status.err = "T_ARMFAILED_ParametersNotPopulated";
                actionCompleted(status);
                return false;
            }
        }

        for (int inputIdx = 0; inputIdx < curStep.personalizationInputs.length(); inputIdx++)
        {
            if (!curStep.personalizationInputs[inputIdx].isValueVisible)
            {
                status.state = DS_ACTION_STATE_INVALID_STATE;
                status.err = "T_ARMFAILED_ParametersNotPopulated";
                actionCompleted(status);
                return false;
            }
        }
    }

    return true;
}

bool DS_ExamAction::armReadyCheckP3TUnusableConcentration(DataServiceActionStatus &status)
{
    // Check Condition: P3TUnusableConcentration alert is active
    QVariantMap activeAlert = env->ds.alertAction->getActiveAlert("P3TUnusableConcentration", "", true);
    if (activeAlert["GUID"] != EMPTY_GUID)
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "T_ARMFAILED_P3TUnusableConcentration;" + activeAlert["Data"].toString();
        actionCompleted(status);
        return false;
    }

    return true;
}

bool DS_ExamAction::armReadyCheckSI2009NonLoadedConcentrationCheck(DataServiceActionStatus &status)
{
    // Check Condition: SI2009NonLoadedConcentration alert is active
    QVariantMap activeAlert = env->ds.alertAction->getActiveAlert("SI2009NonLoadedConcentration", "", true);
    if (activeAlert["GUID"] != EMPTY_GUID)
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "T_ARMFAILED_SI2009NonLoadedConcentration;" + activeAlert["Data"].toString();
        actionCompleted(status);
        return false;
    }

    return true;
}

bool DS_ExamAction::armReadyCheckSyringeReadyCheck(const DS_ExamDef::InjectionStep &curStep, DataServiceActionStatus &status) {
    // Check Condition: Syringe Ready State
    DS_DeviceDef::FluidSources fluidSourceSyringe = env->ds.deviceData->getFluidSourceSyringes();
    QList<SyringeIdx> listSyringesUsed;
    SyringeIdx contrastSyringeIdx = ImrParser::ToCpp_FluidSourceSyringeIdx(ImrParser::ToImr_FluidSourceIdx(curStep.contrastFluidLocation));

    for (int phaseIdx = 0; phaseIdx < curStep.phases.length(); phaseIdx++)
    {
        if (curStep.phases[phaseIdx].type == DS_ExamDef::INJECTION_PHASE_TYPE_FLUID)
        {
            if (curStep.phases[phaseIdx].contrastPercentage == 0)
            {
                if (!listSyringesUsed.contains(SYRINGE_IDX_SALINE))
                {
                    listSyringesUsed.append(SYRINGE_IDX_SALINE);
                }
            }
            else if (curStep.phases[phaseIdx].contrastPercentage == 100)
            {
                if (!listSyringesUsed.contains(contrastSyringeIdx))
                {
                    listSyringesUsed.append(contrastSyringeIdx);
                }
            }
            else
            {
                if (!listSyringesUsed.contains(contrastSyringeIdx))
                {
                    listSyringesUsed.append(contrastSyringeIdx);
                }

                if (!listSyringesUsed.contains(SYRINGE_IDX_SALINE))
                {
                    listSyringesUsed.append(SYRINGE_IDX_SALINE);
                }
            }
        }
        if (listSyringesUsed.length() >= 2)
        {
            break;
        }
    }

    for (int syringeIdx = 0; syringeIdx < listSyringesUsed.length(); syringeIdx++)
    {
        SyringeIdx usedSyringeIdx = listSyringesUsed[syringeIdx];
        if (!fluidSourceSyringe[usedSyringeIdx].isReady)
        {
            status.state = DS_ACTION_STATE_INVALID_STATE;
            status.err = "T_ARMFAILED_ReservoirNotReady";
            actionCompleted(status);
            return false;
        }
    }

    return true;
}

bool DS_ExamAction::armReadyCheckInjectorBusyCheck(DataServiceActionStatus &status)
{
    // Check Condition: Check injector is busy
    if (env->ds.deviceData->getFluidSourceSyringesBusy())
    {
        LOG_WARNING("actArmReadyCheck: ARM Failed - Injector is busy (Syringe)\n");
        status.state = DS_ACTION_STATE_BUSY;
        status.err = "T_ARMFAILED_InjectorIsBusy";
        actionCompleted(status);
        return false;
    }

    if (env->ds.deviceData->getFluidSourceMuds().isBusy)
    {
        LOG_WARNING("actArmReadyCheck: ARM Failed - Injector is busy (MUDS)\n");
        status.state = DS_ACTION_STATE_BUSY;
        status.err = "T_ARMFAILED_InjectorIsBusy";
        actionCompleted(status);
        return false;
    }

    if (env->ds.deviceData->getFluidSourceSuds().isBusy)
    {
        LOG_WARNING("actArmReadyCheck: ARM Failed - Injector is busy (SUDS)\n");
        status.state = DS_ACTION_STATE_BUSY;
        status.err = "T_ARMFAILED_InjectorIsBusy";
        actionCompleted(status);
        return false;
    }

    return true;
}

bool DS_ExamAction::armReadyCheckCatheterLimitCheck(const DS_ExamDef::InjectionStep &curStep, DataServiceActionStatus &status)
{
    DS_ExamDef::InjectionPlan plan = env->ds.examData->getInjectionPlan();
    int stepIndex = plan.getStepIdxFromStepGuid(curStep.guid);
    // Check Condition: Check injection flowrate or pressurelimit has exceeded CatheterType or CatheterPlacement limit
    if ( !env->ds.alertAction->isActivated("CatheterLimitsExceededAccepted", "", true) )
    {
        for (int idx = stepIndex ; idx < plan.steps.length() ; idx++)
        {
            for (int noticeIdx = 0 ; noticeIdx < plan.steps[idx].personalizationNotices.length() ; noticeIdx++)
            {
                // name is an enumeration of type InjectionPersonalizationNoticeType
                auto personalizationNoticeName = plan.steps[idx].personalizationNotices[noticeIdx].name;

                if ( (personalizationNoticeName == DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_FLOW_RATE_EXCEEDS_CATHETER_PLACEMENT_LIMIT) ||
                     (personalizationNoticeName == DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_FLOW_RATE_EXCEEDS_CATHETER_TYPE_LIMIT) ||
                     (personalizationNoticeName == DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_PRESSURE_LIMIT_EXCEEDS_CATHETER_TYPE_LIMIT) )
                {
                    status.state = DS_ACTION_STATE_INVALID_STATE;
                    status.err = QString().asprintf("T_ARMFAILED_CatheterCheckNeeded");
                    LOG_INFO("actArmReadyCheck: Armed Injection Above Catheter Limits");
                    actionCompleted(status);
                    return false;
                }
            }
        }
    }

    return true;
}

bool DS_ExamAction::armReadyCheckInjectionInsufficientVolumeCheck(DS_ExamDef::ArmType type, const DS_ExamDef::InjectionStep &curStep, DataServiceActionStatus &status)
{
    // Check Condition: Check fluid volume
    // Note: The inject volume value is used as ceil() and available volume value is used as floor()
    double salineInjectVolume = ::ceil(curStep.getSalineTotal());
    double contrastInjectVolume = ::ceil(curStep.getContrastTotal());

    // Add extra saline volume for dual phases
    double extraSalineVol;
    env->ds.examAction->actGetPulseSalineVolume(curStep, extraSalineVol);
    if (extraSalineVol > 0)
    {
        LOG_INFO("actArmReadyCheck: Extra Saline Volume (%.1fml) required\n", extraSalineVol);
        salineInjectVolume += extraSalineVol;
    }

    double salineVolumeAvailable, salineVolume, contrastVolumeAvailable, contrastVolume;
    actGetAvailableVolumesForInjection(curStep.contrastFluidLocation, salineVolume, contrastVolume, salineVolumeAvailable, contrastVolumeAvailable);

    DS_McuDef::InjectionProtocol mcuInjectProtocol;
    DataServiceActionStatus mcuProtocolConvertStatus = actGetMcuProtocolFromStep(curStep, mcuInjectProtocol);

    if ( (Util::isFloatVarGreaterThan(salineInjectVolume, salineVolumeAvailable, 1)) ||
         (Util::isFloatVarGreaterThan(contrastInjectVolume, contrastVolumeAvailable, 1)) ||
         (mcuProtocolConvertStatus.err == "InsufficientVolume") ||
         (mcuProtocolConvertStatus.err == "InsufficientVolumeWithCrossOver") )
    {
        DS_ExamDef::InjectionStep adjustedStep;
        bool autoAdjustVolPossible = true;

        if (type == DS_ExamDef::ARM_TYPE_PRELOAD_FIRST)
        {
            LOG_INFO("actArmReadyCheck: InsufficientVolume: AutoAdjust NOT Possible: Type is %s\n", ImrParser::ToImr_ArmType(type).CSTR());
            autoAdjustVolPossible = false;
        }
        else
        {
            QString err = "";
            autoAdjustVolPossible = canAdjustInjectionVolumes(&adjustedStep, &err);
            if (!autoAdjustVolPossible)
            {
                LOG_INFO("actArmReadyCheck: InsufficientVolume: AutoAdjust NOT Possible: Reason is %s\n", err.CSTR());
            }
        }

        double salineAdjustRequired = 0;
        double contrastAdjustRequired = 0;
        if (salineVolumeAvailable < salineInjectVolume)
        {
            salineAdjustRequired = qMax(0.0, (salineInjectVolume - salineVolumeAvailable));
        }

        if (contrastVolumeAvailable < contrastInjectVolume)
        {
            contrastAdjustRequired = qMax(0.0, (contrastInjectVolume - contrastVolumeAvailable));
        }

        status.state = DS_ACTION_STATE_INVALID_STATE;
        // Injector error messages must have a space between the name and the data
        status.err = QString().asprintf("T_ARMFAILED_InsufficientVolume %.1f;%.1f;%s", salineAdjustRequired, contrastAdjustRequired, autoAdjustVolPossible ? "true" : "false");
        actionCompleted(status);
        return false;
    }

    return true;
}

bool DS_ExamAction::armReadyCheckRemainingStepsInsufficientVolumeCheck(const DS_ExamDef::InjectionStep &curStep, DataServiceActionStatus &status)
{
    double salineVolumeAvailable, salineVolume, contrastVolumeAvailable, contrastVolume;
    actGetAvailableVolumesForInjection(curStep.contrastFluidLocation, salineVolume, contrastVolume, salineVolumeAvailable, contrastVolumeAvailable);

    // Check Condition: Insufficient Volume for Remaining Steps
    if (!env->ds.alertAction->isActivated("InsufficientVolumeForStepsAccepted", "", true))
    {
        DS_ExamDef::InjectionPlan plan = env->ds.examData->getInjectionPlan();
        DS_ExamDef::ExecutedSteps executedSteps = env->ds.examData->getExecutedSteps();
        int stepIndex = plan.getStepIdxFromStepGuid(curStep.guid);

        double requiredPlanContrastVol = ::ceil(plan.getContrastTotalToInject(executedSteps));
        double requiredPlanSalineVol = ::ceil(plan.getSalineTotalToInject(executedSteps));
        if ( (Util::isFloatVarGreaterThan(requiredPlanContrastVol, contrastVolumeAvailable, 1)) ||
             (Util::isFloatVarGreaterThan(requiredPlanSalineVol, salineVolumeAvailable, 1)) )
        {
            status.state = DS_ACTION_STATE_INVALID_STATE;
            // Injector error messages must have a space between the name and the data
            status.err = QString().asprintf("T_ARMFAILED_InsufficientVolumeForSteps %d;%.1f;%.1f;%.1f;%.1f;%s",
                                           stepIndex, requiredPlanContrastVol, requiredPlanSalineVol, contrastVolumeAvailable, salineVolumeAvailable, plan.guid.CSTR());
            actionCompleted(status);
            return false;
        }
    }

    return true;
}

bool DS_ExamAction::armReadyCheckScannerInterlocksCheck(DataServiceActionStatus &status)
{
    // Check Condition: Scanner Interlocks Check
    DS_CruDef::CruLinkStatus linkStatus = env->ds.cruData->getCruLinkStatus();
    DS_ExamDef::ScannerInterlocks scannerInterlocks = env->ds.examData->getScannerInterlocks();

    if ( (linkStatus.state == DS_CruDef::CRU_LINK_STATE_ACTIVE) && (scannerInterlocks.armLockedOut) )
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        if (scannerInterlocks.interfaceStatus == DS_ExamDef::SCANNER_INTERFACE_STATUS_ENABLED)
        {
            status.err = "T_ARMFAILED_ScannerArmLockedOutStatusTypeEnabled";
        }
        else
        {
            status.err = "T_ARMFAILED_ScannerArmLockedOut";
        }
        actionCompleted(status);
        return false;
    }

    // Check Condition: Scanner Interlocks while CRU link is down
    bool IsScannerInterlocksCheckNeeded = env->ds.examData->getIsScannerInterlocksCheckNeeded();

    if ( (linkStatus.state != DS_CruDef::CRU_LINK_STATE_ACTIVE) && (IsScannerInterlocksCheckNeeded) )
    {
        if (scannerInterlocks.isIsiActive())
        {
            status.state = DS_ACTION_STATE_INVALID_STATE;
            status.err = "T_ARMFAILED_ScannerInterlocksBypassed";
            actionCompleted(status);
            return false;
        }
    }

    return true;
}

bool DS_ExamAction::armReadyCheckAirCheckOK(DataServiceActionStatus &status)
{
    // Check Condition: Air Check Needed
    if (env->ds.examData->getIsAirCheckNeeded())
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "T_ARMFAILED_AirCheckNeeded";
        actionCompleted(status);
        return false;
    }

    return true;
}
