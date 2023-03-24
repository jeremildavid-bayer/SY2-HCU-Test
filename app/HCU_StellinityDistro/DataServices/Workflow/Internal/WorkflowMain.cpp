#include "Apps/AppManager.h"
#include "WorkflowMain.h"
#include "DataServices/Alert/DS_AlertAction.h"
#include "DataServices/Alert/DS_AlertData.h"
#include "DataServices/Device/DS_DeviceAction.h"
#include "DataServices/Workflow/DS_WorkflowData.h"
#include "DataServices/Workflow/DS_WorkflowAction.h"
#include "DataServices/Exam/DS_ExamData.h"
#include "DataServices/Exam/DS_ExamAction.h"
#include "DataServices/Device/DS_DeviceData.h"
#include "DataServices/Mcu/DS_McuAction.h"
#include "DataServices/Mcu/DS_McuData.h"
#include "DataServices/System/DS_SystemData.h"
#include "DataServices/Capabilities/DS_Capabilities.h"
#include "DataServices/CfgGlobal/DS_CfgGlobal.h"
#include "DataServices/CfgLocal/DS_CfgLocal.h"
#include "Common/ImrParser.h"

WorkflowMain::WorkflowMain(QObject *parent, EnvGlobal *env_) :
    ActionBaseExt(parent, env_)
{
    envLocal = new EnvLocal("DS_Workflow-Main", "WORKFLOW_MAIN", LOG_MID_SIZE_BYTES);

    workflowSod = new WorkflowSod(this, env);
    workflowMudsEject = new WorkflowMudsEject(this, env);
    workflowAdvanceProtocol = new WorkflowAdvanceProtocol(this, env);
    workflowFluidRemoval = new WorkflowFluidRemoval(this, env);
    workflowEndOfDayPurge = new WorkflowEndOfDayPurge(this, env);
    workflowAutoEmpty = new WorkflowAutoEmpty(this, env);
    workflowSyringeAirRecovery = new WorkflowSyringeAirRecovery(this, env);
    workflowSudsAirRecovery = new WorkflowSudsAirRecovery(this, env);
    workflowManualQualifiedDischarge = new WorkflowManualQualifiedDischarge(this, env);
    workflowAutomaticQualifiedDischarge = new WorkflowAutomaticQualifiedDischarge(this, env);
    workflowBattery = new WorkflowBattery(this, env);
    workflowShippingMode = new WorkflowShippingMode(this, env);
    workflowPreloadProtocol = new WorkflowPreloadProtocol(this, env);

    actStatusBuf.request = "Init";

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });
}

WorkflowMain::~WorkflowMain()
{
    delete workflowSod;
    delete workflowMudsEject;
    delete workflowAdvanceProtocol;
    delete workflowSudsAirRecovery;
    delete workflowFluidRemoval;
    delete workflowEndOfDayPurge;
    delete workflowAutoEmpty;
    delete workflowSyringeAirRecovery;
    delete workflowManualQualifiedDischarge;
    delete workflowBattery;
    delete workflowShippingMode;
    delete workflowPreloadProtocol;
    delete envLocal;
}

void WorkflowMain::slotAppInitialised()
{
    connect(env->ds.systemData, &DS_SystemData::signalDataChanged_StatePath, this, [=](DS_SystemDef::StatePath curStatePath, DS_SystemDef::StatePath prevStatePath) {
        if ( ( (prevStatePath == DS_SystemDef::STATE_PATH_IDLE) && (curStatePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING) ) ||
             ( (prevStatePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING) && (curStatePath == DS_SystemDef::STATE_PATH_IDLE) ) )
        {
            LOG_INFO("\n\n-----------------\nStatePath=%s\n\n", ImrParser::ToImr_StatePath(curStatePath).CSTR());
        }
    });

    connect(env->ds.systemData, &DS_SystemData::signalDataChanged_StatePath, this, [=]() {
        activate();
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_LinkState, this, [=]() {
        activate();
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_MudsLatched, this, [=](bool isLatched, bool prevIsLatched){
        handleMudsUnlatch(isLatched, prevIsLatched);
        if (prevIsLatched && !isLatched)
        {
            env->ds.workflowData->resetSyringesUsedInLastExam();
        }
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_MudsPresent, this, [=](bool present, bool prevPresent) {
        if (present == prevPresent)
        {
            return;
        }

        if (!present)
        {
            clearBottleFillCount();
        }
    });

    connect(env->ds.deviceData, &DS_DeviceData::signalDataChanged_FluidSourceSuds, this, [=](const DS_DeviceDef::FluidSource &fluidSourceSuds, const DS_DeviceDef::FluidSource &prevFluidSourceSuds) {
        if ( (prevFluidSourceSuds.isInstalled()) && (!fluidSourceSuds.isInstalled()) )
        {
            updateWorkflowErrorStatusFromSudsRemoved(fluidSourceSuds, prevFluidSourceSuds);
        }
    });

    connect(env->ds.workflowData, &DS_WorkflowData::signalDataChanged_WorkflowState, this, [=](DS_WorkflowDef::WorkflowState workflowState, DS_WorkflowDef::WorkflowState workflowStatePrev) {
        if ( (workflowState != DS_WorkflowDef::STATE_INACTIVE) &&
             (workflowStatePrev == DS_WorkflowDef::STATE_INACTIVE) )
        {
        }
        else if ( (workflowState == DS_WorkflowDef::STATE_INACTIVE) &&
                  (workflowStatePrev != DS_WorkflowDef::STATE_INACTIVE) )
        {
            resetWorkflowStatus();
            // Destroy action instances
            env->actionMgr->deleteActAll(actStatusBuf.guid);
            env->actionMgr->deleteActAll(guidSubAction);
        }
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_PrimeBtnPressed, this, [=](bool pressed) {
        DS_WorkflowDef::WorkflowState workflowState = env->ds.workflowData->getWorkflowState();
        if (workflowState == DS_WorkflowDef::STATE_INACTIVE)
        {
            return;
        }

        if (pressed)
        {
            QString guid = Util::newGuid();
            env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus){
                if (curStatus.state == DS_ACTION_STATE_STARTED)
                {
                    LOG_DEBUG("Manual Prime Started.\n");
                    env->actionMgr->onActionCompleted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus){
                        if (curStatus.state != DS_ACTION_STATE_COMPLETED)
                        {
                            LOG_ERROR("Action Failed Complete: %s\n", ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());
                        }

                        // Trigger the refill check regardless of complete state
                        env->ds.examAction->actAutoRefill("AfterManualPrime");
                    });
                }
                else
                {
                    LOG_ERROR("Action Failed Start: %s\n", ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());
                }
            });
            env->ds.deviceAction->actSudsManualPrime(guid);
        }
    });

    connect(env->ds.workflowData, &DS_WorkflowData::signalDataChanged_WorkflowErrorStatus, this, [=](const DS_WorkflowDef::WorkflowErrorStatus &workflowErrorStatus) {
        if (workflowErrorStatus.state != DS_WorkflowDef::WORKFLOW_ERROR_STATE_NONE)
        {
            LOG_WARNING("WorkflowErrorStatus = %s\n", Util::qVarientToJsonData(ImrParser::ToImr_WorkflowErrorStatus(workflowErrorStatus)).CSTR());
        }
    });
}

DataServiceActionStatus WorkflowMain::actAutoPrime(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "AutoPrime");

    // Clear Reprime Required Alert
    if (env->ds.alertAction->isActivated("RepreloadReprimeRequired", "", true))
    {
        env->ds.alertAction->deactivate("RepreloadReprimeRequired");
    }

    // If Executing Step exist, clear Preloaded flag for current/future steps
    env->ds.examAction->actResetPreloadSteps();

    DS_WorkflowDef::MudsSodStatus mudsSodStatus = env->ds.workflowData->getMudsSodStatus();
    if (!mudsSodStatus.syringeSodStatusAll[SYRINGE_IDX_SALINE].primed)
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = QString().asprintf("SYRINGE_NOT_PRIMED");
        actionStarted(status);
        env->ds.alertAction->activate("AutoPrimeFailed", status.err);
        return status;
    }

    DS_WorkflowDef::WorkflowState workflowState = env->ds.workflowData->getWorkflowState();

    if ( (workflowState != DS_WorkflowDef::STATE_NORMAL_WORKFLOW_READY) &&
         (workflowState != DS_WorkflowDef::STATE_NORMAL_WORKFLOW_SUDS_INSERT_WAITING) &&
         (workflowState != DS_WorkflowDef::STATE_SOD_DONE) &&
         (workflowState != DS_WorkflowDef::STATE_NORMAL_WORKFLOW_AUTO_PRIME_SALINE_WAITING) &&
         (workflowState != DS_WorkflowDef::STATE_SYRINGE_AIR_RECOVERY_DONE) &&
         (workflowState != DS_WorkflowDef::STATE_SUDS_AIR_RECOVERY_DONE) )
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = QString().asprintf("INVALID_STATE;%s", ImrParser::ToImr_WorkFlowState(workflowState).CSTR());
        actionStarted(status);
        env->ds.alertAction->activate("AutoPrimeFailed", status.err);
        return status;
    }

    actStatusBuf = status;
    actStatusBuf.state = DS_ACTION_STATE_STARTED;
    actionStarted(actStatusBuf);

    setState(DS_WorkflowDef::STATE_AUTO_PRIME_STARTED);

    return actStatusBuf;
}

DataServiceActionStatus WorkflowMain::actAutoEmpty(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "AutoEmpty");

    actStatusBuf = status;
    actStatusBuf.state = DS_ACTION_STATE_STARTED;
    actionStarted(actStatusBuf);

    setState(DS_WorkflowDef::STATE_AUTO_EMPTY_STARTED);

    return actStatusBuf;
}

DataServiceActionStatus WorkflowMain::actMudsSodWithNewFluid(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "MudsSodWithNewFluid");

    DS_WorkflowDef::WorkflowState workflowState = env->ds.workflowData->getWorkflowState();
    if ( (workflowState != DS_WorkflowDef::STATE_SOD_SUSPENDED) &&
         (workflowState != DS_WorkflowDef::STATE_NORMAL_WORKFLOW_READY) &&
         (workflowState != DS_WorkflowDef::STATE_NORMAL_WORKFLOW_SUDS_INSERT_WAITING) &&
         (workflowState != DS_WorkflowDef::STATE_NORMAL_WORKFLOW_AUTO_PRIME_SALINE_WAITING) &&
         (workflowState != DS_WorkflowDef::STATE_END_OF_DAY_PURGE_FAILED) &&
         (workflowState != DS_WorkflowDef::STATE_END_OF_DAY_PURGE_DONE) &&
         (workflowState != DS_WorkflowDef::STATE_FLUID_REMOVAL_FAILED) &&
         (workflowState != DS_WorkflowDef::STATE_FLUID_REMOVAL_DONE) )
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = QString().asprintf("InvalidState: %s", ImrParser::ToImr_WorkFlowState(workflowState).CSTR());
        actionStarted(status);
        return status;
    }

    actStatusBuf = status;
    actStatusBuf.state = DS_ACTION_STATE_STARTED;
    actionStarted(actStatusBuf);

    setState(DS_WorkflowDef::STATE_SOD_STARTED_BY_NEW_FLUID_LOADED);
    return actStatusBuf;
}

DataServiceActionStatus WorkflowMain::actMudsEject(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "MudsEject");

    DS_WorkflowDef::WorkflowState workflowState = env->ds.workflowData->getWorkflowState();

    if ( (workflowState == DS_WorkflowDef::STATE_MUDS_EJECT_STARTED) ||
         (workflowState == DS_WorkflowDef::STATE_MUDS_EJECT_PROGRESS) ||
         (workflowState == DS_WorkflowDef::STATE_INACTIVE) )
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;

        DS_WorkflowDef::WorkflowErrorStatus errStatus;
        errStatus.state = DS_WorkflowDef::WORKFLOW_ERROR_STATE_MUDS_EJECT_FAILED;
        errStatus.syringeIndexFailed = SYRINGE_IDX_NONE;
        env->ds.workflowData->setWorkflowErrorStatus(errStatus);
        status.err = ImrParser::ToImr_WorkflowErrorState(errStatus.state);
        actionStarted(status);
        return status;
    }

    actStatusBuf = status;
    actStatusBuf.state = DS_ACTION_STATE_STARTED;
    actionStarted(actStatusBuf);

    setState(DS_WorkflowDef::STATE_MUDS_EJECT_STARTED);

    return actStatusBuf;
}

DataServiceActionStatus WorkflowMain::actSodRestart(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "SodRestart");

    DS_WorkflowDef::WorkflowState workflowState = env->ds.workflowData->getWorkflowState();

    if ( (workflowState != DS_WorkflowDef::STATE_SOD_SUSPENDED) &&
         (workflowState != DS_WorkflowDef::STATE_SOD_FAILED) )
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = QString().asprintf("InvalidState: %s", ImrParser::ToImr_WorkFlowState(workflowState).CSTR());
        actionStarted(status);
        return status;
    }

    actStatusBuf = status;

    actStatusBuf.state = DS_ACTION_STATE_STARTED;
    actionStarted(actStatusBuf);

    setState(DS_WorkflowDef::STATE_SOD_RESUMED);

    return actStatusBuf;
}

DataServiceActionStatus WorkflowMain::actSodAbort(DS_WorkflowDef::SodErrorState sodError, QString actGuid)
{
    return workflowSod->actSodAbort(sodError, actGuid);
}

DataServiceActionStatus WorkflowMain::actForceFill(SyringeIdx syringeIdx, QString actGuid)
{
    QString syringeIdxStr = ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdx);
    DataServiceActionStatus status = actionInit(actGuid, "ForceFill", QString().asprintf("%s", syringeIdxStr.CSTR()));

    DS_WorkflowDef::WorkflowState workflowState = env->ds.workflowData->getWorkflowState();
    DS_WorkflowDef::WorkflowErrorStatus workflowErrorStatus = env->ds.workflowData->getWorkflowErrorStatus();
    DS_WorkflowDef::MudsSodStatus mudsSodStatus = env->ds.workflowData->getMudsSodStatus();
    DS_DeviceDef::FluidSource fluidSrcSuds = env->ds.deviceData->getFluidSourceSuds();
    bool fillDuringSod = false;


    if ( (workflowState == DS_WorkflowDef::STATE_SOD_PROGRESS) ||
         (workflowState == DS_WorkflowDef::STATE_SOD_SUSPENDED) )
    {
        LOG_INFO("ACT_FORCE_FILL: Start fill %s during SOD\n", syringeIdxStr.CSTR());
        fillDuringSod = true;
    }
    else if ( (mudsSodStatus.syringeSodStatusAll[syringeIdx].primed) &&
              ( (workflowState == DS_WorkflowDef::STATE_NORMAL_WORKFLOW_READY) ||
                (workflowState == DS_WorkflowDef::STATE_NORMAL_WORKFLOW_SUDS_INSERT_WAITING) ||
                (workflowState == DS_WorkflowDef::STATE_NORMAL_WORKFLOW_AUTO_PRIME_SALINE_WAITING) ) )
    {
        LOG_INFO("ACT_FORCE_FILL: Start fill %s after SOD\n", syringeIdxStr.CSTR());
    }
    else if ( (!mudsSodStatus.syringeSodStatusAll[syringeIdx].primed) &&
              ( (workflowState == DS_WorkflowDef::STATE_NORMAL_WORKFLOW_READY) ||
                (workflowState == DS_WorkflowDef::STATE_NORMAL_WORKFLOW_SUDS_INSERT_WAITING) ||
                (workflowState == DS_WorkflowDef::STATE_NORMAL_WORKFLOW_AUTO_PRIME_SALINE_WAITING) ) )
    {
        // MUDS prime for given syringe is required after filling.
        if ( (workflowErrorStatus.state == DS_WorkflowDef::WORKFLOW_ERROR_STATE_NONE) &&
             (fluidSrcSuds.isReady) )
        {
            LOG_WARNING("ACT_FORCE_FILL: Start fill %s with SOD already completed, patient may be connected. Needs confirm Patient Disconnect.\n", syringeIdxStr.CSTR());

            DS_WorkflowDef::WorkflowErrorStatus workflowErrorStatus = env->ds.workflowData->getWorkflowErrorStatus();
            workflowErrorStatus.state = DS_WorkflowDef::WORKFLOW_ERROR_STATE_SOD_FILL_START_WITH_PATIENT_CONNECTED;
            workflowErrorStatus.syringeIndexFailed = syringeIdx;
            env->ds.workflowData->setWorkflowErrorStatus(workflowErrorStatus);
            status.err = "Fill Failed: " + syringeIdxStr;
            status.state = DS_ACTION_STATE_INVALID_STATE;
        }
        else
        {
            LOG_INFO("ACT_FORCE_FILL: Start fill %s during SOD\n", syringeIdxStr.CSTR());
            fillDuringSod = true;
        }
    }
    else
    {
        LOG_ERROR("ACT_FORCE_FILL: Cannot fill %s: Invalid state. WorkflowState=%s\n", syringeIdxStr.CSTR(), ImrParser::ToImr_WorkFlowState(workflowState).CSTR());
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "Bad Workflow State";
    }

    if (status.err != "")
    {
        actionStarted(status);
        return status;
    }

    bool syringeAirDetectedBeforeFill = env->ds.alertAction->isActivatedWithSyringeIdx("ReservoirAirDetected", syringeIdx);

    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);

        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            // Ready to fill
            if (env->ds.workflowData->getWorkflowErrorStatus().state == DS_WorkflowDef::WORKFLOW_ERROR_STATE_SOD_FILL_START_WITH_PATIENT_CONNECTED)
            {
                resetWorkflowStatus();
            }

            if (fillDuringSod)
            {
                // Remove any connected callbacks
                LOG_DEBUG("ACT_FORCE_FILL: Filling during SOD: Deleting suds and syringe monitor guids (fillDuringSOD is true)\n");
                env->actionMgr->deleteActAll(sudsMonitorGuid);
                env->actionMgr->deleteActAll(syringesBusyMonitorGuid);
                env->actionMgr->deleteActAll(statePathMonitorGuid);

                if (!env->ds.alertAction->isActivated("EnsureNoPatientConnected"))
                {
                    env->ds.alertAction->activate("EnsureNoPatientConnected");
                }
            }

            bool fillingNewContrastBottle = ((syringeIdx != SYRINGE_IDX_SALINE) && isNewBottleFill(syringeIdx));
            double curSyringeVolume = 0.0;
            if (fillingNewContrastBottle) {
                DS_DeviceDef::FluidSource fluidSourceSyringe = env->ds.deviceData->getFluidSourceSyringes().at(syringeIdx);
                curSyringeVolume = fluidSourceSyringe.currentVolumesTotal();

                LOG_INFO("ACT_FORCE_FILL: Looks like filling from new constast bottle: %s.\n", ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdx).CSTR());
            }

            env->actionMgr->onActionCompleted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
                actionCompleted(curStatus, &status);

                // Further action might required after refill
                bool fillCompleted = false;

                if ( (curStatus.state == DS_ACTION_STATE_COMPLETED) ||
                     (curStatus.state == DS_ACTION_STATE_ALLSTOP_BTN_ABORT) ||
                     (curStatus.err.contains("AIR_DETECTED")) )
                {
                    fillCompleted = true;
                }

                if (fillingNewContrastBottle)
                {
                    bool confirmNewContrastBottleFill = true;
                    if (curStatus.err.contains("AIR_DETECTED"))
                    {
                        DS_DeviceDef::FluidSource fluidSourceSyringe = env->ds.deviceData->getFluidSourceSyringes().at(syringeIdx);
                        double mininumFillVolume = env->ds.capabilities->get_Alert_NewBottleMinimumFillVolume();
                        if(!((fluidSourceSyringe.currentVolumesTotal() - curSyringeVolume) > mininumFillVolume))
                        {
                            LOG_INFO("ACT_FORCE_FILL: New Bottle fill couldn't fill minimum volume of %.2fml before detecting air.. Possibly empty bottle? Not incrementing count.\n", mininumFillVolume);
                            confirmNewContrastBottleFill = false;
                        }
                    }
                    else if (curStatus.err.contains("INSUFFICIENT_FLUID"))
                    {
                        LOG_INFO("ACT_FORCE_FILL: New Bottle fill completed with INSUFFICIENT_FLUID error. Not incrementing count.\n");
                        confirmNewContrastBottleFill = false;
                    }

                    if (confirmNewContrastBottleFill)
                    {
                        incrementBottleFillCount();
                        int count = env->ds.cfgLocal->get_Hidden_BottleFillCount();
                        LOG_INFO("ACT_FORCE_FILL: Successfully filled from new bottle: %s. Contrast Bottle fill count = %d\n", ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdx).CSTR(), count);
                    }
                }

                if (fillCompleted)
                {
                    // POST FILL OPERATION
                    DS_WorkflowDef::WorkflowState workflowState = env->ds.workflowData->getWorkflowState();

                    // Start SOD with new fluid
                    if (fillDuringSod)
                    {
                        if (workflowState == DS_WorkflowDef::STATE_SOD_PROGRESS)
                        {
                            // Post fill will be handled by other
                        }
                        else
                        {
                            DS_DeviceDef::FluidSource fluidSourceSuds = env->ds.deviceData->getFluidSourceSuds();
                            if (fluidSourceSuds.needsReplaced)
                            {
                                LOG_WARNING("ACT_FORCE_FILL: Fill is completed during SOD but SUDS needs replaced. MUDS Prime will be performed later.\n");
                                setState(DS_WorkflowDef::STATE_NORMAL_WORKFLOW_READY);
                            }
                            else
                            {
                                // Needed for change contrast Muds Prime
                                LOG_INFO("ACT_FORCE_FILL: Fill is completed during SOD. Starting MUDS Prime with New Fluid..\n");
                                actMudsSodWithNewFluid();
                            }
                        }

                        if (env->ds.alertAction->isActivated("EnsureNoPatientConnected"))
                        {
                            env->ds.alertAction->deactivate("EnsureNoPatientConnected");
                        }
                    }

                    // Resume Syringe Air Recovery Sequence with sufficient volume
                    if (env->ds.alertAction->isActivated("OutletAirDetected"))
                    {
                        LOG_WARNING("ACT_FORCE_FILL: Fill is completed but outlet air detected. Auto prime started..\n");
                        actAutoPrime();
                    }
                    else if ( (syringeAirDetectedBeforeFill) &&
                              (!env->ds.alertAction->isActivated("ReservoirAirDetected")) )
                    {
                        LOG_WARNING("ACT_FORCE_FILL: Fill is completed while waiting from Syringe Recovery Sequence BUT no air detected. Auto prime started..\n");
                        actAutoPrime();
                    }
                }
            });
        }
    });

    return env->ds.deviceAction->actSyringeFill(syringeIdx, false, guid);
}

DataServiceActionStatus WorkflowMain::actSetAutoPrimeComplete(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "SetAutoPrimeComplete");
    DS_WorkflowDef::WorkflowState workflowState = env->ds.workflowData->getWorkflowState();

    if (workflowState != DS_WorkflowDef::STATE_NORMAL_WORKFLOW_READY)
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = QString().asprintf("InvalidState: %s", ImrParser::ToImr_WorkFlowState(workflowState).CSTR());
        actionStarted(status);
        return status;
    }

    status.state = DS_ACTION_STATE_STARTED;

    actionStarted(status);

    setState(DS_WorkflowDef::STATE_NORMAL_WORKFLOW_READY);

    status.state = DS_ACTION_STATE_COMPLETED;
    actionCompleted(status);

    return status;
}

DataServiceActionStatus WorkflowMain::actSudsAirRecoveryStart(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "SudsAirRecoveryStart");

    DS_WorkflowDef::WorkflowState workflowState = env->ds.workflowData->getWorkflowState();

    if (workflowState != DS_WorkflowDef::STATE_NORMAL_WORKFLOW_READY)
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = QString().asprintf("InvalidState: %s", ImrParser::ToImr_WorkFlowState(workflowState).CSTR());
        actionStarted(status);
        return status;
    }

    actStatusBuf = status;
    actStatusBuf.state = DS_ACTION_STATE_STARTED;
    actionStarted(actStatusBuf);

    setState(DS_WorkflowDef::STATE_SUDS_AIR_RECOVERY_STARTED);

    return actStatusBuf;
}

DataServiceActionStatus WorkflowMain::actSudsAirRecoveryResume(QString actGuid)
{
    return workflowSudsAirRecovery->actSudsAirRecoveryResume(actGuid);
}

DataServiceActionStatus WorkflowMain::actSyringeAirRecoveryStart(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "SyringeAirRecoveryStart");

    DS_WorkflowDef::WorkflowState workflowState = env->ds.workflowData->getWorkflowState();

    if ( (workflowState != DS_WorkflowDef::STATE_NORMAL_WORKFLOW_READY) &&
         (workflowState != DS_WorkflowDef::STATE_NORMAL_WORKFLOW_SUDS_INSERT_WAITING) &&
         (workflowState != DS_WorkflowDef::STATE_SOD_SUSPENDED) &&
         (workflowState != DS_WorkflowDef::STATE_SOD_DONE) &&
         (workflowState != DS_WorkflowDef::STATE_SUDS_AIR_RECOVERY_FAILED) &&
         (workflowState != DS_WorkflowDef::STATE_NORMAL_WORKFLOW_AUTO_PRIME_SALINE_WAITING) )
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = QString().asprintf("WorkFlowState=%s", ImrParser::ToImr_WorkFlowState(workflowState).CSTR());
        actionStarted(status);
        return status;
    }

    actStatusBuf = status;
    actStatusBuf.state = DS_ACTION_STATE_STARTED;
    actionStarted(actStatusBuf);

    setState(DS_WorkflowDef::STATE_SYRINGE_AIR_RECOVERY_STARTED);

    return actStatusBuf;
}

DataServiceActionStatus WorkflowMain::actSyringeAirRecoveryResume(QString actGuid)
{
    return workflowSyringeAirRecovery->actSyringeAirRecoveryResume(actGuid);
}

DataServiceActionStatus WorkflowMain::actFluidRemovalStart(SyringeIdx syringeIdx, bool isAutoEmpty, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "FluidRemovalStart", QString().asprintf("%s", ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdx).CSTR()));

    DS_WorkflowDef::WorkflowState workflowState = env->ds.workflowData->getWorkflowState();

    if (workflowState == DS_WorkflowDef::STATE_END_OF_DAY_PURGE_PROGRESS)
    {
        LOG_WARNING("ACT_FLUID_REMOVAL_START: Abort End of Day\n");
        env->ds.workflowAction->actEndOfDayPurgeAbort();
    }
    else if ( (workflowState == DS_WorkflowDef::STATE_SOD_STARTED) ||
              (workflowState == DS_WorkflowDef::STATE_SOD_STARTED_BY_NEW_FLUID_LOADED) ||
              (workflowState == DS_WorkflowDef::STATE_SOD_PROGRESS) )
    {
        LOG_WARNING("ACT_FLUID_REMOVAL_START: Abort SOD\n");
        env->ds.workflowAction->actSodAbort(DS_WorkflowDef::SOD_ERROR_STATE_USER_ABORT);
    }
    else if ( (workflowState != DS_WorkflowDef::STATE_NORMAL_WORKFLOW_READY) &&
              (workflowState != DS_WorkflowDef::STATE_NORMAL_WORKFLOW_SUDS_INSERT_WAITING) &&
              (workflowState != DS_WorkflowDef::STATE_NORMAL_WORKFLOW_AUTO_PRIME_SALINE_WAITING) &&
              (workflowState != DS_WorkflowDef::STATE_SOD_SUSPENDED) &&
              (workflowState != DS_WorkflowDef::STATE_FLUID_REMOVAL_FAILED) &&
              (workflowState != DS_WorkflowDef::STATE_AUTO_EMPTY_PROGRESS) )
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = QString().asprintf("WorkFlowState=%s", ImrParser::ToImr_WorkFlowState(workflowState).CSTR());
        LOG_WARNING("ACT_FLUID_REMOVAL_START: Error=%s\n", status.err.CSTR());
        actionStarted(status);
        return status;
    }

    actStatusBuf = status;
    actStatusBuf.state = DS_ACTION_STATE_STARTED;
    actionStarted(actStatusBuf);

    fluidRemovalIndex = syringeIdx;

    if (isAutoEmpty)
    {
        workflowFluidRemoval->actFluidRemovalStart(fluidRemovalIndex, true, actGuid);
    }
    else
    {
        setState(DS_WorkflowDef::STATE_FLUID_REMOVAL_STARTED);
    }

    return actStatusBuf;
}

DataServiceActionStatus WorkflowMain::actFluidRemovalResume(QString actGuid)
{
    return workflowFluidRemoval->actFluidRemovalResume(actGuid);
}

DataServiceActionStatus WorkflowMain::actFluidRemovalAbort(QString actGuid)
{
    return workflowFluidRemoval->actFluidRemovalAbort(actGuid);
}

DataServiceActionStatus WorkflowMain::actEndOfDayPurgeStart(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "EndOfDayPurgeStart");

    DS_WorkflowDef::WorkflowState workflowState = env->ds.workflowData->getWorkflowState();

    if ( (workflowState == DS_WorkflowDef::STATE_SOD_PROGRESS) ||
         (workflowState == DS_WorkflowDef::STATE_SOD_SUSPENDED) )
    {
        LOG_INFO("END_OF_DAY_PURGE_START: SOD is completed yet. Abort SOD first.\n");
        workflowSod->actSodAbort(DS_WorkflowDef::SOD_ERROR_STATE_USER_ABORT);
    }
    else if ( (workflowState != DS_WorkflowDef::STATE_NORMAL_WORKFLOW_READY) &&
              (workflowState != DS_WorkflowDef::STATE_NORMAL_WORKFLOW_SUDS_INSERT_WAITING) &&
              (workflowState != DS_WorkflowDef::STATE_NORMAL_WORKFLOW_AUTO_PRIME_SALINE_WAITING) )
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = QString().asprintf("%s", ImrParser::ToImr_WorkFlowState(workflowState).CSTR());
        actionStarted(status);
        return status;
    }

    env->ds.alertAction->activate("InitiatingEmptyDaySet");

    actStatusBuf = status;
    actStatusBuf.state = DS_ACTION_STATE_STARTED;
    actionStarted(actStatusBuf);

    setState(DS_WorkflowDef::STATE_END_OF_DAY_PURGE_STARTED);

    return actStatusBuf;
}

DataServiceActionStatus WorkflowMain::actEndOfDayPurgeResume(QString actGuid)
{
    return workflowEndOfDayPurge->actEndOfDayPurgeResume(actGuid);
}

DataServiceActionStatus WorkflowMain::actEndOfDayPurgeAbort(QString actGuid)
{
    return workflowEndOfDayPurge->actEndOfDayPurgeAbort(actGuid);
}

DataServiceActionStatus WorkflowMain::actQueAutomaticQualifiedDischarge(int batteryIdx, QString actGuid)
{
    return workflowAutomaticQualifiedDischarge->actForceQueAQD(batteryIdx, actGuid);
}

DataServiceActionStatus WorkflowMain::actCancelAutomaticQualifiedDischarge(QString actGuid)
{
    return workflowAutomaticQualifiedDischarge->actCancelAQD(actGuid);
}

DataServiceActionStatus WorkflowMain::actWorkflowBatteryChargeToStart(int batteryIdx, int target, QString actGuid)
{
    return workflowBattery->actBatteryChargeToStart(batteryIdx, target, actGuid);
}

DataServiceActionStatus WorkflowMain::actWorkflowBatteryAction(int batteryIdx, QString action, QString actGuid)
{
    return workflowBattery->actBatteryAction(batteryIdx, action, actGuid);
}

DataServiceActionStatus WorkflowMain::actWorkflowBatteryAbort(QString actGuid)
{
    return workflowBattery->actAbort(actGuid);
}

DataServiceActionStatus WorkflowMain::actManualQualifiedDischargeStart(int batteryIdx, DS_WorkflowDef::ManualQualifiedDischargeMethod methodType, QString actGuid)
{
    return workflowManualQualifiedDischarge->actManualQualifiedDischargeStart(batteryIdx, methodType, actGuid);
}

DataServiceActionStatus WorkflowMain::actManualQualifiedDischargeResume(QString actGuid)
{
    return workflowManualQualifiedDischarge->actManualQualifiedDischargeResume(actGuid);
}

DataServiceActionStatus WorkflowMain::actManualQualifiedDischargeAbort(QString actGuid)
{
    return workflowManualQualifiedDischarge->actManualQualifiedDischargeAbort(actGuid);
}

DataServiceActionStatus WorkflowMain::actShippingModeStart(int batteryIdx, int targetCharge, QString actGuid)
{
    return workflowShippingMode->actShippingModeStart(batteryIdx, targetCharge, actGuid);
}

DataServiceActionStatus WorkflowMain::actShippingModeResume(QString actGuid)
{
    return workflowShippingMode->actShippingModeResume(actGuid);
}

DataServiceActionStatus WorkflowMain::actShippingModeAbort(QString actGuid)
{
    return workflowShippingMode->actShippingModeAbort(actGuid);
}

DataServiceActionStatus WorkflowMain::actPreloadProtocolStart(bool userConfirmRequired, QString actGuid)
{
    return workflowPreloadProtocol->actPreloadProtocolStart(userConfirmRequired, actGuid);
}

DataServiceActionStatus WorkflowMain::actPreloadProtocolResume(QString actGuid)
{
    return workflowPreloadProtocol->actPreloadProtocolResume(actGuid);
}

DataServiceActionStatus WorkflowMain::actPreloadProtocolAbort(QString actGuid)
{
    return workflowPreloadProtocol->actPreloadProtocolAbort(actGuid);
}

void WorkflowMain::setSystemToReady()
{
    setState(DS_WorkflowDef::STATE_NORMAL_WORKFLOW_READY);
}

void WorkflowMain::activate()
{
    DS_McuDef::LinkState linkState = env->ds.mcuData->getLinkState();
    DS_SystemDef::StatePath statePath = env->ds.systemData->getStatePath();
    DS_WorkflowDef::WorkflowState workflowState = env->ds.workflowData->getWorkflowState();

    bool activeState = (statePath == DS_SystemDef::STATE_PATH_IDLE) ||
                       (statePath == DS_SystemDef::STATE_PATH_READY_ARMED) ||
                       (statePath == DS_SystemDef::STATE_PATH_EXECUTING) ||
                       (statePath == DS_SystemDef::STATE_PATH_BUSY_WAITING) ||
                       (statePath == DS_SystemDef::STATE_PATH_BUSY_FINISHING) ||
                       (statePath == DS_SystemDef::STATE_PATH_BUSY_HOLDING);

    activeState &= (linkState == DS_McuDef::LINK_STATE_CONNECTED);

    if (activeState)
    {
        if (workflowState == DS_WorkflowDef::STATE_INACTIVE)
        {
            // activate
            actStatusBuf.guid = "";
            env->ds.workflowData->setWorkflowState(DS_WorkflowDef::STATE_INIT);
            processState();
        }
    }
    else
    {
        if (workflowState != DS_WorkflowDef::STATE_INACTIVE)
        {
            // deactivate
            env->actionMgr->deleteActAll(actStatusBuf.guid);
            env->actionMgr->deleteActAll(sudsMonitorGuid);
            env->actionMgr->deleteActAll(syringesBusyMonitorGuid);
            env->actionMgr->deleteActAll(statePathMonitorGuid);

            QTimer::singleShot(500, this, [=](){
                // Give some time to complete other actions (e.g. Abort from EOF Purge) before set to inactive
                env->ds.workflowData->setWorkflowState(DS_WorkflowDef::STATE_INACTIVE);
                processState();
            });
        }
    }
}

int WorkflowMain::getState()
{
    return env->ds.workflowData->getWorkflowState();
}

QString WorkflowMain::getStateStr(int state)
{
    return ImrParser::ToImr_WorkFlowState((DS_WorkflowDef::WorkflowState)state);
}

void WorkflowMain::setStateSynch(int newState)
{
    env->ds.workflowData->setWorkflowState((DS_WorkflowDef::WorkflowState)newState);
}

bool WorkflowMain::isSetStateReady()
{
    if (getState() == DS_WorkflowDef::STATE_INACTIVE)
    {
        return false;
    }
    return true;
}

void WorkflowMain::processState()
{
    DS_WorkflowDef::WorkflowState state = (DS_WorkflowDef::WorkflowState)getState();

    if ( (isMotorPositionFaultActive()) &&
         (state != DS_WorkflowDef::STATE_INIT_FAILED) &&
         ( (state < DS_WorkflowDef::STATE_MUDS_EJECT_STARTED) || (state > DS_WorkflowDef::STATE_MUDS_EJECT_DONE) ) )
    {
        LOG_ERROR("INIT FAILED: MotorPositionFaultActive AND State=%s\n", ImrParser::ToImr_WorkFlowState(state).CSTR());
        setState(DS_WorkflowDef::STATE_INIT_FAILED);
        return;
    }

    switch (state)
    {
    case DS_WorkflowDef::STATE_INACTIVE:
        LOG_INFO("STATE_INACTIVE\n");
        break;
    // ==========================================================
    // INIT STATES
    case DS_WorkflowDef::STATE_INIT:
        LOG_INFO("STATE_INIT\n");
        actStatusBuf = actionInit("", "SOD");
        env->ds.workflowData->setSodErrorState(DS_WorkflowDef::SOD_ERROR_STATE_NONE);
        setState(DS_WorkflowDef::STATE_INIT_STOPCOCK_SETTING);
        break;
    case DS_WorkflowDef::STATE_INIT_STOPCOCK_SETTING:
        LOG_INFO("STATE_INIT_STOPCOCK_SETTING\n");
        clearMonitorConnections();
        handleSubAction(DS_WorkflowDef::STATE_INIT_STOPCOCK_PROGRESS,
                        DS_WorkflowDef::STATE_INIT_STOPCOCK_DONE,
                        DS_WorkflowDef::STATE_INIT_STOPCOCK_FAILED);
        env->ds.deviceAction->actStopcock(DS_McuDef::STOPCOCK_POS_FILL, DS_McuDef::STOPCOCK_POS_FILL, DS_McuDef::STOPCOCK_POS_FILL, STOPCOCK_ACTION_TRIALS_LIMIT, guidSubAction);
        break;
    case DS_WorkflowDef::STATE_INIT_STOPCOCK_PROGRESS:
        LOG_INFO("STATE_INIT_STOPCOCK_PROGRESS\n");
        break;
    case DS_WorkflowDef::STATE_INIT_STOPCOCK_DONE:
        LOG_INFO("STATE_INIT_STOPCOCK_FILL_DONE\n");
        setState(DS_WorkflowDef::STATE_MUDS_INSERT_WAITING);
        break;
    case DS_WorkflowDef::STATE_INIT_STOPCOCK_FAILED:
        LOG_WARNING("STATE_INIT_STOPCOCK_FAILED\n");
        setState(DS_WorkflowDef::STATE_INIT_FAILED);
        break;
    case DS_WorkflowDef::STATE_INIT_FAILED:
        LOG_WARNING("STATE_INIT_FAILED\n");
        break;
    // ==========================================================
    // INSERTION STATES
    case DS_WorkflowDef::STATE_MUDS_INSERT_WAITING:
        LOG_INFO("STATE_MUDS_INSERT_WAITING\n");
        if (env->ds.mcuData->getMudsInserted())
        {
            setState(DS_WorkflowDef::STATE_MUDS_INSERTED);
        }
        else
        {
            // Deactivate any alerts that are supposed to be gone when MUDS is ejected
            env->ds.alertAction->deactivate("MUDSEjectedRemovalRequired");
            env->ds.alertAction->deactivate("UsedMUDSDetected");

            env->actionMgr->deleteActAll(mudsInsertWaitingMonitorGuid);
            mudsInsertWaitingMonitorGuid = Util::newGuid();
            QMetaObject::Connection conn = connect(env->ds.mcuData, &DS_McuData::signalDataChanged_MudsInserted, this, [=](bool mudsInserted) {
                if (env->ds.workflowData->getWorkflowState() != DS_WorkflowDef::STATE_MUDS_INSERT_WAITING)
                {
                    // unexpected callback
                }
                else if (mudsInserted)
                {
                    env->actionMgr->deleteActAll(mudsInsertWaitingMonitorGuid);
                    setState(DS_WorkflowDef::STATE_MUDS_INSERTED);
                }
            });
            env->actionMgr->createActCompleted(mudsInsertWaitingMonitorGuid, conn, QString(__PRETTY_FUNCTION__) + ": STATE_MUDS_INSERT_WAITING: MudsInserted");
        }
        break;
    case DS_WorkflowDef::STATE_MUDS_INSERTED:
        LOG_INFO("STATE_MUDS_INSERTED\n");
        {
            if (env->ds.alertAction->isActivated("MUDSEjectedRemovalRequired", "", true))
            {
                LOG_WARNING("STATE_MUDS_INSERTED: MUDSEjectedRemovalRequired Alert activated. MUDS Eject started\n");
                env->ds.workflowAction->actMudsEject();
            }
            else
            {
                setState(DS_WorkflowDef::STATE_SOD_STARTED);
            }
        }
        break;
    // ==========================================================
    // START OF DAY STATES
    case DS_WorkflowDef::STATE_SOD_STARTED:
        LOG_INFO("STATE_SOD_STARTED\n");
        clearMonitorConnections();
        handleSubAction(DS_WorkflowDef::STATE_SOD_PROGRESS, DS_WorkflowDef::STATE_SOD_DONE, DS_WorkflowDef::STATE_SOD_FAILED);
        workflowSod->actSodStart(guidSubAction);
        break;
    case DS_WorkflowDef::STATE_SOD_STARTED_BY_NEW_FLUID_LOADED:
        LOG_INFO("STATE_SOD_STARTED_BY_NEW_FLUID_LOADED\n");
        clearMonitorConnections();
        handleSubAction(DS_WorkflowDef::STATE_SOD_PROGRESS, DS_WorkflowDef::STATE_SOD_DONE, DS_WorkflowDef::STATE_SOD_FAILED);
        workflowSod->actMudsSodWithNewFluid(guidSubAction);
        break;
    case DS_WorkflowDef::STATE_SOD_PROGRESS:
        LOG_INFO("STATE_SOD_PROGRESS\n");
        break;
    case DS_WorkflowDef::STATE_SOD_FAILED:
        LOG_WARNING("STATE_SOD_FAILED\n");
        {
            DS_WorkflowDef::SodErrorState sodErrorState = env->ds.workflowData->getSodErrorState();
            LOG_ERROR("STATE_SOD_FAILED: SodErrorState=%s\n", ImrParser::ToImr_SodErrorState(sodErrorState).CSTR());
            setState(DS_WorkflowDef::STATE_SOD_SUSPENDED);
            actionCompleted(actStatusBuf);
        }
        break;
    case DS_WorkflowDef::STATE_SOD_SUSPENDED:
        LOG_INFO("STATE_SOD_SUSPENDED\n");

        // TODO: S2SRUSW 2615 - MOVE THIS--------------------------
        if (env->ds.alertAction->isActivated("SRUClearingInletFluid", "", true))
        {
            SyringeIdx airDetectedSyringe = SYRINGE_IDX_NONE;
            for (int syringeIdx = 0; syringeIdx < SYRINGE_IDX_MAX; syringeIdx++)
            {
                if (env->ds.alertAction->isActivatedWithSyringeIdx("SRUClearingInletFluid", (SyringeIdx)syringeIdx))
                {
                    airDetectedSyringe = (SyringeIdx)syringeIdx;
                    break;
                }
            }
            LOG_WARNING("STATE_SOD_SUSPENDED: Inlet Fluid is not removed during last Fluid Removal. Activating ReservoirAirDetected..\n");
            env->ds.alertAction->deactivate("SRUClearingInletFluid");
            if (env->ds.alertAction->isActivated("ReservoirAirDetected", "", true))
            {
                LOG_ERROR("STATE_SOD_SUSPENDED: Unexpected ReservoirAirDetected is activated!\n");
            }
            else
            {
                if (airDetectedSyringe != SYRINGE_IDX_NONE)
                {
                    env->ds.alertAction->activate("ReservoirAirDetected", QString().asprintf("%s;%.1f", ImrParser::ToImr_FluidSourceSyringeIdx(airDetectedSyringe).CSTR(), FLUID_REMOVAL_INLET_LINE_WITHDRAW_VOL));
                }
            }
            setState(DS_WorkflowDef::STATE_SOD_FAILED);
            return;
        }
        //--------------------------TODO S2SRUSW 2615 - MOVE THIS--------------------------

        if (env->ds.alertAction->isActivated("ReservoirAirDetected", "", true))
        {
            LOG_WARNING("STATE_SOD_SUSPENDED: Air detected in one of the syringes during SOD. Syringe Air Recovery is started..\n");
            env->ds.workflowAction->actSyringeAirRecoveryStart();
        }
        break;
    case DS_WorkflowDef::STATE_SOD_RESUMED:
        LOG_INFO("STATE_SOD_RESUMED\n");
        clearMonitorConnections();
        handleSubAction(DS_WorkflowDef::STATE_SOD_PROGRESS, DS_WorkflowDef::STATE_SOD_DONE, DS_WorkflowDef::STATE_SOD_FAILED);
        workflowSod->actSodRestart(guidSubAction);
        break;
    case DS_WorkflowDef::STATE_SOD_DONE:
        LOG_INFO("STATE_SOD_DONE\n");
        {
            // Ensure first auto prime is not true after SOD
            DS_DeviceDef::FluidSource fluidSourceSuds = env->ds.deviceData->getFluidSourceSuds();
            env->ds.deviceData->setIsFirstAutoPrimeCompleted(false);
            actionCompleted(actStatusBuf);

            // POST SOD PROCESS
            if ( fluidSourceSuds.isInstalled() )
            {
                actAutoPrime();
            }
            else
            {
                setState(DS_WorkflowDef::STATE_NORMAL_WORKFLOW_SUDS_INSERT_WAITING);
            }
        }
        break;
    // ==========================================================
    // NORMAL DAY STATES
    case DS_WorkflowDef::STATE_NORMAL_WORKFLOW_READY:
        LOG_INFO("STATE_NORMAL_WORKFLOW_READY\n");
        {
            clearMonitorConnections();
            resetWorkflowStatus();

            // SUDS Removal and Auto-Prime handling
            waitForSudsToBeRemoved(DS_WorkflowDef::STATE_NORMAL_WORKFLOW_READY);
        }
        break;
    case DS_WorkflowDef::STATE_NORMAL_WORKFLOW_SUDS_INSERT_WAITING:
        LOG_INFO("STATE_NORMAL_WORKFLOW_SUDS_INSERT_WAITING\n");
        {
            DS_DeviceDef::FluidSource fluidSourceSuds = env->ds.deviceData->getFluidSourceSuds();

            if (env->ds.deviceData->getFluidSourceSyringesBusy())
            {
                waitForSyringesToBecomeIdle(DS_WorkflowDef::STATE_NORMAL_WORKFLOW_SUDS_INSERT_WAITING);
            }
            else if (!fluidSourceSuds.isInstalled())
            {
                waitForSudsToBeInserted(DS_WorkflowDef::STATE_NORMAL_WORKFLOW_SUDS_INSERT_WAITING);
            }
            else
            {
                handleSudsReinsertion();
            }
        }
        break;
    case DS_WorkflowDef::STATE_NORMAL_WORKFLOW_AUTO_PRIME_SALINE_WAITING:
        LOG_INFO("STATE_NORMAL_WORKFLOW_AUTO_PRIME_SALINE_WAITING\n");
        {
            double autoPrimeVolRequired = env->ds.workflowData->getAutoPrimeVolume();
            DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
            DS_DeviceDef::FluidSource fluidSourceSuds = env->ds.deviceData->getFluidSourceSuds();

            if (!fluidSourceSuds.isInstalled())
            {
                setState(DS_WorkflowDef::STATE_NORMAL_WORKFLOW_SUDS_INSERT_WAITING);
            }
            else if (env->ds.deviceData->getFluidSourceSyringesBusy())
            {
                LOG_DEBUG("STATE_NORMAL_WORKFLOW_AUTO_PRIME_SALINE_WAITING: At least one syringe is busy. Waiting..\n");
                waitForSyringesToBecomeIdle(DS_WorkflowDef::STATE_NORMAL_WORKFLOW_AUTO_PRIME_SALINE_WAITING);
            }
            else if (fluidSourceSyringes[SYRINGE_IDX_SALINE].currentVolumesTotal() < autoPrimeVolRequired)
            {
                LOG_INFO("STATE_NORMAL_WORKFLOW_AUTO_PRIME_SALINE_WAITING: Needs more saline. vol=%.1fml, prime required=%.1fml\n", fluidSourceSyringes[SYRINGE_IDX_SALINE].currentVolumesTotal(), autoPrimeVolRequired);

                env->actionMgr->deleteActAll(syringeVolMonitorGuid);
                syringeVolMonitorGuid = Util::newGuid();
                QMetaObject::Connection conn = connect(env->ds.deviceData, &DS_DeviceData::signalDataChanged_FluidSourceSyringes, this, [=](const DS_DeviceDef::FluidSources &fluidSourceSyringes) {
                    if (env->ds.workflowData->getWorkflowState() != DS_WorkflowDef::STATE_NORMAL_WORKFLOW_AUTO_PRIME_SALINE_WAITING)
                    {
                        // unexpected callback
                        env->actionMgr->deleteActAll(syringeVolMonitorGuid);
                    }
                    else if (fluidSourceSyringes[SYRINGE_IDX_SALINE].currentVolumesTotal() >= autoPrimeVolRequired)
                    {
                        // finally, got enough fluids, recalculate and hopefuly jump to next stage
                        env->actionMgr->deleteActAll(syringeVolMonitorGuid);
                        setState(DS_WorkflowDef::STATE_NORMAL_WORKFLOW_AUTO_PRIME_SALINE_WAITING);
                    }
                });
                env->actionMgr->createActCompleted(syringeVolMonitorGuid, conn, QString(__PRETTY_FUNCTION__) + ": STATE_NORMAL_WORKFLOW_AUTO_PRIME_SALINE_WAITING: FluidSourceSyringes");


                // SUDS Removal and Auto-Prime handling
                waitForSudsToBeRemoved(DS_WorkflowDef::STATE_NORMAL_WORKFLOW_AUTO_PRIME_SALINE_WAITING);
            }
            else
            {
                LOG_INFO("STATE_NORMAL_WORKFLOW_AUTO_PRIME_SALINE_WAITING: Starting Auto Prime..\n");
                actAutoPrime();
            }
        }
        break;
    // ==========================================================
    // AUTO PRIME STATES
    case DS_WorkflowDef::STATE_AUTO_PRIME_STARTED:
        LOG_INFO("STATE_AUTO_PRIME_STARTED\n");

        if (env->ds.alertAction->isActivated("ReservoirAirDetected", "", true))
        {
            LOG_WARNING("STATE_AUTO_PRIME_STARTED: Auto prime cannot be started as Reservoir Air Recovery needs to be performed\n");
            actStatusBuf.state = DS_ACTION_STATE_INVALID_STATE;
            actStatusBuf.err = "Syringe Not Ready";
            setState(DS_WorkflowDef::STATE_AUTO_PRIME_FAILED);
            return;
        }

        clearMonitorConnections();
        handleSubAction(DS_WorkflowDef::STATE_AUTO_PRIME_PROGRESS,
                        DS_WorkflowDef::STATE_AUTO_PRIME_DONE,
                        DS_WorkflowDef::STATE_AUTO_PRIME_FAILED);
        env->ds.deviceAction->actSudsAutoPrime(guidSubAction);
        break;
    case DS_WorkflowDef::STATE_AUTO_PRIME_PROGRESS:
        LOG_INFO("STATE_AUTO_PRIME_PROGRESS\n");
        waitForSudsToBeRemoved(DS_WorkflowDef::STATE_AUTO_PRIME_PROGRESS);
        break;
    case DS_WorkflowDef::STATE_AUTO_PRIME_FAILED:
        LOG_WARNING("STATE_AUTO_PRIME_FAILED: ACTION_STATUS: %s\n", ImrParser::ToImr_DataServiceActionStatusStr(actStatusBuf).CSTR());
        if (actStatusBuf.err.contains(_L("INSUFFICIENT_FLUID")))
        {
            LOG_WARNING("STATE_AUTO_PRIME_FAILED: Refill started..\n");
            env->ds.examAction->actAutoRefill("AutoPrimeInsufficientFluid");
            setState(DS_WorkflowDef::STATE_NORMAL_WORKFLOW_AUTO_PRIME_SALINE_WAITING);
            return;
        }
        else if (actStatusBuf.err.contains(_L("SUDS_REMOVED")))
        {
            setState(DS_WorkflowDef::STATE_NORMAL_WORKFLOW_SUDS_INSERT_WAITING);
        }
        else if (actStatusBuf.err.contains(_L("USER_ABORT")))
        {
            setState(DS_WorkflowDef::STATE_NORMAL_WORKFLOW_READY);
        }
        else
        {
            setState(DS_WorkflowDef::STATE_NORMAL_WORKFLOW_READY);
        }

        actionCompleted(actStatusBuf);

        env->ds.examAction->actAutoRefill("AfterAutoPrime");
        break;
    case DS_WorkflowDef::STATE_AUTO_PRIME_DONE:
        LOG_INFO("STATE_AUTO_PRIME_DONE\n");
        actionCompleted(actStatusBuf);
        setState(DS_WorkflowDef::STATE_NORMAL_WORKFLOW_READY);

        env->ds.examAction->actAutoRefill("AfterAutoPrime");
        break;
    // ==========================================================
    // AUTO EMPTY
    case DS_WorkflowDef::STATE_AUTO_EMPTY_STARTED:
        LOG_INFO("STATE_AUTO_EMPTY_STARTED\n");
        clearMonitorConnections();
        resetWorkflowStatus();
        handleSubAction(DS_WorkflowDef::STATE_AUTO_EMPTY_PROGRESS,
                        DS_WorkflowDef::STATE_AUTO_EMPTY_DONE,
                        DS_WorkflowDef::STATE_AUTO_EMPTY_FAILED);
        workflowAutoEmpty->actAutoEmptyStart(guidSubAction);
        break;
    case DS_WorkflowDef::STATE_AUTO_EMPTY_PROGRESS:
        LOG_INFO("STATE_AUTO_EMPTY_PROGRESS\n");
        break;
    case DS_WorkflowDef::STATE_AUTO_EMPTY_FAILED:
        LOG_WARNING("STATE_AUTO_EMPTY_FAILED\n");
        resetWorkflowState();
        actionCompleted(actStatusBuf);
        break;
    case DS_WorkflowDef::STATE_AUTO_EMPTY_DONE:
        LOG_INFO("STATE_AUTO_EMPTY_DONE\n");
        {
            resetWorkflowState();
            actionCompleted(actStatusBuf);
            // Auto Empty is completed. Start auto prime
            setState(DS_WorkflowDef::STATE_NORMAL_WORKFLOW_AUTO_PRIME_SALINE_WAITING);

            env->ds.examAction->actReloadSelectedContrast();
        }
        break;
    // ==========================================================
    // MUDS EJECT
    case DS_WorkflowDef::STATE_MUDS_EJECT_STARTED:
        LOG_INFO("STATE_MUDS_EJECT_STARTED\n");
        clearMonitorConnections();
        handleSubAction(DS_WorkflowDef::STATE_MUDS_EJECT_PROGRESS,
                        DS_WorkflowDef::STATE_MUDS_EJECT_DONE,
                        DS_WorkflowDef::STATE_MUDS_EJECT_FAILED);
        workflowMudsEject->actMudsEjectStart(guidSubAction);
        break;
    case DS_WorkflowDef::STATE_MUDS_EJECT_PROGRESS:
        LOG_INFO("STATE_MUDS_EJECT_PROGRESS\n");
        break;
    case DS_WorkflowDef::STATE_MUDS_EJECT_FAILED:
        LOG_WARNING("STATE_MUDS_EJECT_FAILED\n");
        actionCompleted(actStatusBuf);
        setState(DS_WorkflowDef::STATE_NORMAL_WORKFLOW_READY);
        break;
    case DS_WorkflowDef::STATE_MUDS_EJECT_DONE:
        {
            LOG_INFO("STATE_MUDS_EJECT_DONE\n");
            actionCompleted(actStatusBuf);
            if (env->ds.workflowData->getUseExtendedAutoPrimeVolume())
            {
                // if MUDS was ejected, Reset ExtendedAutoPrime flag just to ensure we use correct Prime volume for next auto prime.
                LOG_INFO("Disabling ExtendedAutoPrime Volume\n");
                env->ds.workflowData->setUseExtendedAutoPrimeVolume(false);
            }

            // clear workflowErrorStatus.
            DS_WorkflowDef::WorkflowErrorStatus workflowErrorStatus = env->ds.workflowData->getWorkflowErrorStatus();
            workflowErrorStatus.state = DS_WorkflowDef::WORKFLOW_ERROR_STATE_NONE;
            env->ds.workflowData->setWorkflowErrorStatus(workflowErrorStatus);

            setState(DS_WorkflowDef::STATE_MUDS_INSERT_WAITING);
        }
        break;
    // ==========================================================
    // FLUID REMOVAL
    case DS_WorkflowDef::STATE_FLUID_REMOVAL_STARTED:
        LOG_INFO("STATE_FLUID_REMOVAL_STARTED\n");
        clearMonitorConnections();
        resetWorkflowStatus();
        handleSubAction(DS_WorkflowDef::STATE_FLUID_REMOVAL_PROGRESS,
                        DS_WorkflowDef::STATE_FLUID_REMOVAL_DONE,
                        DS_WorkflowDef::STATE_FLUID_REMOVAL_FAILED);
        workflowFluidRemoval->actFluidRemovalStart(fluidRemovalIndex, false, guidSubAction);
        break;
    case DS_WorkflowDef::STATE_FLUID_REMOVAL_PROGRESS:
        LOG_INFO("STATE_FLUID_REMOVAL_PROGRESS\n");
        break;
    case DS_WorkflowDef::STATE_FLUID_REMOVAL_FAILED:
        LOG_WARNING("STATE_FLUID_REMOVAL_FAILED\n");
        actionCompleted(actStatusBuf);

        // Check if Fluid Removal is performed as part of AutoEmpty, If so, handle AutoEmpty state
        {
            DS_WorkflowDef::AutoEmptyState autoEmptyState = env->ds.workflowData->getAutoEmptyState();
            if ( (autoEmptyState == DS_WorkflowDef::AUTO_EMPTY_STATE_FLUID_REMOVAL_PROGRESS) ||
                 (autoEmptyState == DS_WorkflowDef::AUTO_EMPTY_STATE_FLUID_REMOVAL_FAILED) )
            {
                setState(DS_WorkflowDef::STATE_AUTO_EMPTY_FAILED);
                return;
            }
        }

        resetWorkflowState();
        break;
    case DS_WorkflowDef::STATE_FLUID_REMOVAL_DONE:
        LOG_INFO("STATE_FLUID_REMOVAL_DONE\n");
        actionCompleted(actStatusBuf);

        // Check if Fluid Removal is performed as part of AutoEmpty, If so, handle AutoEmpty state
        {
            DS_WorkflowDef::AutoEmptyState autoEmptyState = env->ds.workflowData->getAutoEmptyState();
            if ( (autoEmptyState == DS_WorkflowDef::AUTO_EMPTY_STATE_FLUID_REMOVAL_PROGRESS) ||
                 (autoEmptyState == DS_WorkflowDef::AUTO_EMPTY_STATE_FLUID_REMOVAL_DONE) )
            {
                setState(DS_WorkflowDef::STATE_AUTO_EMPTY_DONE);
                return;
            }
        }
        resetWorkflowState();

        break;
    // ==========================================================
    // SYRINGE AIR RECOVERY
    case DS_WorkflowDef::STATE_SYRINGE_AIR_RECOVERY_STARTED:
        LOG_INFO("STATE_SYRINGE_AIR_RECOVERY_STARTED\n");
        clearMonitorConnections();
        handleSubAction(DS_WorkflowDef::STATE_SYRINGE_AIR_RECOVERY_PROGRESS,
                        DS_WorkflowDef::STATE_SYRINGE_AIR_RECOVERY_DONE,
                        DS_WorkflowDef::STATE_SYRINGE_AIR_RECOVERY_FAILED);
        workflowSyringeAirRecovery->actSyringeAirRecoveryStart(guidSubAction);
        break;
    case DS_WorkflowDef::STATE_SYRINGE_AIR_RECOVERY_PROGRESS:
        LOG_INFO("STATE_SYRINGE_AIR_RECOVERY_PROGRESS\n");
        break;
    case DS_WorkflowDef::STATE_SYRINGE_AIR_RECOVERY_FAILED:
        LOG_WARNING("STATE_SYRINGE_AIR_RECOVERY_FAILED\n");

        actionCompleted(actStatusBuf);
        resetWorkflowState();
        break;
    case DS_WorkflowDef::STATE_SYRINGE_AIR_RECOVERY_DONE:
        LOG_INFO("STATE_SYRINGE_AIR_RECOVERY_DONE\n");
        actionCompleted(actStatusBuf);
        if (env->ds.deviceData->getFluidSourceMuds().isReady)
        {
            actAutoPrime();
        }
        else
        {
            // SOD is not completed. Resume SOD.
            setState(DS_WorkflowDef::STATE_SOD_STARTED_BY_NEW_FLUID_LOADED);
        }
        break;
    // ==========================================================
    // SUDS AIR RECOVERY
    case DS_WorkflowDef::STATE_SUDS_AIR_RECOVERY_STARTED:
        LOG_INFO("STATE_SUDS_AIR_RECOVERY_STARTED\n");
        clearMonitorConnections();
        handleSubAction(DS_WorkflowDef::STATE_SUDS_AIR_RECOVERY_PROGRESS,
                        DS_WorkflowDef::STATE_SUDS_AIR_RECOVERY_DONE,
                        DS_WorkflowDef::STATE_SUDS_AIR_RECOVERY_FAILED);
        workflowSudsAirRecovery->actSudsAirRecoveryStart(guidSubAction);
        break;
    case DS_WorkflowDef::STATE_SUDS_AIR_RECOVERY_PROGRESS:
        LOG_INFO("STATE_SUDS_AIR_RECOVERY_PROGRESS\n");
        break;
    case DS_WorkflowDef::STATE_SUDS_AIR_RECOVERY_FAILED:
        LOG_WARNING("STATE_SUDS_AIR_RECOVERY_FAILED\n");

        actionCompleted(actStatusBuf);
        resetWorkflowState();
        break;
    case DS_WorkflowDef::STATE_SUDS_AIR_RECOVERY_DONE:
        LOG_INFO("STATE_SUDS_AIR_RECOVERY_DONE\n");
        actionCompleted(actStatusBuf);
        actAutoPrime();
        break;
    // ==========================================================
    // END OF DAY PURGE
    case DS_WorkflowDef::STATE_END_OF_DAY_PURGE_STARTED:
        LOG_INFO("STATE_END_OF_DAY_PURGE_STARTED\n");
        clearMonitorConnections();
        handleSubAction(DS_WorkflowDef::STATE_END_OF_DAY_PURGE_PROGRESS,
                        DS_WorkflowDef::STATE_END_OF_DAY_PURGE_DONE,
                        DS_WorkflowDef::STATE_END_OF_DAY_PURGE_FAILED);
        workflowEndOfDayPurge->actEndOfDayPurgeStart(guidSubAction);
        break;
    case DS_WorkflowDef::STATE_END_OF_DAY_PURGE_PROGRESS:
        LOG_INFO("STATE_END_OF_DAY_PURGE_PROGRESS\n");
        break;
    case DS_WorkflowDef::STATE_END_OF_DAY_PURGE_FAILED:
        LOG_WARNING("STATE_END_OF_DAY_PURGE_FAILED\n");
        {
            if ( (actStatusBuf.err.contains("SUDS_REMOVED")) ||
                 (actStatusBuf.err.contains("USER_ABORT")) )
            {
                env->ds.deviceAction->actSudsSetNeedsPrime(true);
                env->ds.deviceData->setIsFirstAutoPrimeCompleted(false);
            }

            resetWorkflowState();

            actionCompleted(actStatusBuf);
        }
        break;
    case DS_WorkflowDef::STATE_END_OF_DAY_PURGE_DONE:
        LOG_INFO("STATE_END_OF_DAY_PURGE_DONE\n");
        {
            actionCompleted(actStatusBuf);
            env->ds.deviceAction->actSudsSetNeedsPrime(true);

            resetWorkflowState();
        }
        break;

    // ==========================================================
    // ADVANCE PROTOCOL

    default:
        LOG_ERROR("Unknown State(%d)\n", state);
        break;
    }
}

void WorkflowMain::waitForSyringesToBecomeIdle(DS_WorkflowDef::WorkflowState curState)
{
    LOG_DEBUG("Waiting for syringes to become Idle: curState=%s\n", ImrParser::ToImr_WorkFlowState(curState).CSTR());
    env->actionMgr->deleteActAll(syringesBusyMonitorGuid);
    syringesBusyMonitorGuid = Util::newGuid();
    QMetaObject::Connection conn = connect(env->ds.deviceData, &DS_DeviceData::signalDataChanged_FluidSourceSyringes, this, [=](const DS_DeviceDef::FluidSources &fluidSourceSyringes) {
        if (env->ds.workflowData->getWorkflowState() != curState)
        {
            // unexpected callback
            env->actionMgr->deleteActAll(syringesBusyMonitorGuid);
        }
        else if ( (!fluidSourceSyringes[SYRINGE_IDX_SALINE].isBusy) &&
                  (!fluidSourceSyringes[SYRINGE_IDX_CONTRAST1].isBusy) &&
                  (!fluidSourceSyringes[SYRINGE_IDX_CONTRAST2].isBusy) )
        {
            // Set current state again in order to re-process current state
            env->actionMgr->deleteActAll(syringesBusyMonitorGuid);
            setState(curState);
        }
    });
    env->actionMgr->createActCompleted(syringesBusyMonitorGuid, conn, QString(__PRETTY_FUNCTION__) + ": waitForSyringesToBecomeIdle: FluidSourceSyringes");
}

void WorkflowMain::waitForSudsToBeInserted(DS_WorkflowDef::WorkflowState curState)
{
    LOG_DEBUG("%s: waitForSudsToBeInserted(): Waiting for SUDS to be Inserted..\n", getStateStr(curState).CSTR());

    env->actionMgr->deleteActAll(sudsMonitorGuid);
    sudsMonitorGuid = Util::newGuid();
    QMetaObject::Connection conn = connect(env->ds.deviceData, &DS_DeviceData::signalDataChanged_FluidSourceSuds, this, [=](const DS_DeviceDef::FluidSource &fluidSourceSuds) {
        if (env->ds.workflowData->getWorkflowState() != curState)
        {
            // unexpected callback
            env->actionMgr->deleteActAll(sudsMonitorGuid);
        }
        else if (fluidSourceSuds.isInstalled())
        {
            QString stateStr = getStateStr(curState);
            LOG_DEBUG("%s: waitForSudsToBeInserted(): SUDS Inserted. Reprocess the current state..\n", stateStr.CSTR());

            // Set current state again in order to re-process current state
            env->actionMgr->deleteActAll(sudsMonitorGuid);
            setState(curState);
        }
    });
    env->actionMgr->createActCompleted(sudsMonitorGuid, conn, QString(__PRETTY_FUNCTION__) + ": waitForSudsToBeInserted: FluidSourceSuds");
}


void WorkflowMain::waitForSudsToBeRemoved(DS_WorkflowDef::WorkflowState curState)
{
    LOG_DEBUG("%s: waitForSudsToBeRemoved(): Waiting for SUDS to be Removed..\n", getStateStr(curState).CSTR());

    env->actionMgr->deleteActAll(sudsMonitorGuid);
    sudsMonitorGuid = Util::newGuid();
    QMetaObject::Connection conn = connect(env->ds.deviceData, &DS_DeviceData::signalDataChanged_FluidSourceSuds, this, [=](const DS_DeviceDef::FluidSource &fluidSourceSuds) {
        if (env->ds.workflowData->getWorkflowState() != curState)
        {
            // unexpected callback
            env->actionMgr->deleteActAll(sudsMonitorGuid);
        }
        else if (!fluidSourceSuds.isInstalled())
        {
            QString stateStr = getStateStr(curState);
            DS_SystemDef::StatePath curStatePath = env->ds.systemData->getStatePath();
            if ( (curStatePath == DS_SystemDef::STATE_PATH_IDLE) ||
                 (curStatePath == DS_SystemDef::STATE_PATH_READY_ARMED) ||
                 (curStatePath == DS_SystemDef::STATE_PATH_EXECUTING) ||
                 (curStatePath == DS_SystemDef::STATE_PATH_BUSY_WAITING) ||
                 (curStatePath == DS_SystemDef::STATE_PATH_BUSY_FINISHING) ||
                 (curStatePath == DS_SystemDef::STATE_PATH_BUSY_HOLDING) )
            {
                LOG_DEBUG("%s: waitForSudsToBeRemoved(): SUDS Removed. Setting state to %s\n", stateStr.CSTR(), getStateStr(DS_WorkflowDef::STATE_NORMAL_WORKFLOW_SUDS_INSERT_WAITING).CSTR());
                env->actionMgr->deleteActAll(sudsMonitorGuid);
                setState(DS_WorkflowDef::STATE_NORMAL_WORKFLOW_SUDS_INSERT_WAITING);
            }
            else
            {
                LOG_WARNING("%s: waitForSudsToBeRemoved(): SUDS Removed and unexpected StatePath=%s\n", stateStr.CSTR(), ImrParser::ToImr_StatePath(curStatePath).CSTR());
            }
        }
    });
    env->actionMgr->createActCompleted(sudsMonitorGuid, conn, QString(__PRETTY_FUNCTION__) + ": waitForSudsToBeRemoved: fluidSourceSuds");
}

void WorkflowMain::waitAndHandleExamEnded()
{
    DS_ExamDef::ExamProgressState examState = env->ds.examData->getExamProgressState();

    LOG_DEBUG("Listening for exam to be ended. Current exam state=%s\n", ImrParser::ToImr_ExamProgressState(examState).CSTR());

    env->actionMgr->deleteActAll(examEndedMonitorGuid);
    examEndedMonitorGuid = Util::newGuid();
    QMetaObject::Connection conn = connect(env->ds.examData, &DS_ExamData::signalDataChanged_ExamProgressState, this, [=](DS_ExamDef::ExamProgressState examState) {
        if (env->ds.workflowData->getWorkflowState() != DS_WorkflowDef::STATE_NORMAL_WORKFLOW_READY)
        {
            // unexpected callback
            env->actionMgr->deleteActAll(examEndedMonitorGuid);
            return;
        }

        if (examState == DS_ExamDef::EXAM_PROGRESS_STATE_COMPLETED)
        {
            env->actionMgr->deleteActAll(examEndedMonitorGuid);
            resetWorkflowState();
        }
    });
    env->actionMgr->createActCompleted(examEndedMonitorGuid, conn, QString(__PRETTY_FUNCTION__) + ": waitAndHandleExamEnded: ExamProgressState");
}

void WorkflowMain::handleMudsUnlatch(bool curLatched, bool prevLatched)
{
    DS_WorkflowDef::WorkflowState workflowState = env->ds.workflowData->getWorkflowState();
    if ( (workflowState == DS_WorkflowDef::STATE_INACTIVE) ||
         (workflowState == DS_WorkflowDef::STATE_INIT) ||
         (workflowState == DS_WorkflowDef::STATE_INIT_STOPCOCK_SETTING) ||
         (workflowState == DS_WorkflowDef::STATE_INIT_STOPCOCK_PROGRESS) ||
         (workflowState == DS_WorkflowDef::STATE_INIT_STOPCOCK_DONE) ||
         (workflowState == DS_WorkflowDef::STATE_MUDS_INSERT_WAITING) ||
         (workflowState == DS_WorkflowDef::STATE_MUDS_EJECT_PROGRESS) )
    {
        return;
    }

    if ( (curLatched != prevLatched) &&
         (!curLatched) )
    {
        // Only stop all if something is active
        DS_McuDef::SyringeStates syringeStates = env->ds.mcuData->getSyringeStates();
        if ( (syringeStates[SYRINGE_IDX_SALINE] == DS_McuDef::SYRINGE_STATE_PROCESSING) ||
             (syringeStates[SYRINGE_IDX_CONTRAST1] == DS_McuDef::SYRINGE_STATE_PROCESSING) ||
             (syringeStates[SYRINGE_IDX_CONTRAST2] == DS_McuDef::SYRINGE_STATE_PROCESSING) )
        {
            QString guid = Util::newGuid();
            env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus){
                if (curStatus.state == DS_ACTION_STATE_STARTED)
                {
                    LOG_WARNING("handleMudsUnlatch(): MUDS unlatched while syringes are in action. Ejecting MUDS..\n");
                    startDelayedMudsEject();
                }
            });

            LOG_WARNING("handleMudsUnlatch(): MUDS unlatched while syringes are in action. Stopping all actions\n");
            env->ds.mcuAction->actStopAll(guid);
        }
        else
        {
            startDelayedMudsEject();
        }
    }
}

void WorkflowMain::startDelayedMudsEject(int delayMs)
{
    QTimer::singleShot(delayMs, this, [=](){
        DS_WorkflowDef::SodErrorState sodErrorState = env->ds.workflowData->getSodErrorState();
        if ( (sodErrorState != DS_WorkflowDef::SOD_ERROR_STATE_PURGE_FAILED) &&
             (sodErrorState != DS_WorkflowDef::SOD_ERROR_STATE_PURGE_FAILED_MUDS_REMOVED) )
        {
            LOG_WARNING("startDelayedMudsEject(): Abort SOD as MUDS unlatched. Current SodErr=%s\n", ImrParser::ToImr_SodErrorState(sodErrorState).CSTR());
            env->ds.workflowAction->actSodAbort(DS_WorkflowDef::SOD_ERROR_STATE_MUDS_LATCH_LIFTED);
        }

        LOG_INFO("startDelayedMudsEject(): Ejecting MUDS after unlatched..\n");
        env->ds.workflowAction->actMudsEject();
    });
}

bool WorkflowMain::isMudsPrimeStartRequired()
{
    // Check if SOD needs to be restarted
    DS_WorkflowDef::MudsSodStatus mudsSodStatus = env->ds.workflowData->getMudsSodStatus();
    DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
    bool mudsPrimeNeeded = false;

    for (int syringeIdx = 0; syringeIdx < fluidSourceSyringes.length(); syringeIdx++)
    {
        // This check for 0ml may cause issues because of the new zero position being at a higher position.
        if ( (!mudsSodStatus.syringeSodStatusAll[syringeIdx].primed) &&
              mudsSodStatus.syringeSodStatusAll[syringeIdx].calSlackDone &&
             (fluidSourceSyringes[syringeIdx].currentVolumesTotal() > 0))
        {
            LOG_INFO("isMudsPrimeStartRequired(): Muds Prime Needed for %s, currentVol=%.1fml\n",
                     ImrParser::ToImr_FluidSourceSyringeIdx((SyringeIdx)syringeIdx).CSTR(),
                     fluidSourceSyringes[syringeIdx].currentVolumesTotal());
            mudsPrimeNeeded = true;
            break;
        }
    }

    if (!mudsPrimeNeeded)
    {
        LOG_INFO("isMudsPrimeStartRequired(): Muds Prime NOT Needed\n");
    }
    return mudsPrimeNeeded;
}

void WorkflowMain::resetWorkflowState()
{
    DS_DeviceDef::FluidSource fluidSourceSuds = env->ds.deviceData->getFluidSourceSuds();
    if (fluidSourceSuds.isInstalled())
    {
        LOG_INFO("resetWorkflowState(): MudsPrimeDone: Start Normal Workflow..\n");
        setState(DS_WorkflowDef::STATE_NORMAL_WORKFLOW_READY);
    }
    else
    {
        LOG_INFO("resetWorkflowState(): MudsPrimeDone: SUDS insert waiting..\n");
        setState(DS_WorkflowDef::STATE_NORMAL_WORKFLOW_SUDS_INSERT_WAITING);
    }
}

bool WorkflowMain::isMotorPositionFaultActive()
{
    QVariantList activeAlertList = env->ds.alertData->getActiveAlerts();

    if (activeAlertList.length() == 0)
    {
        return false;
    }

    for(int i = 0; i < activeAlertList.length(); i++)
    {
        QVariantMap alert = activeAlertList[i].toMap();
        if (alert[_L("CodeName")].toString() == _L("MotorPositionFault"))
        {
            return true;
        }
    }

    return false;
}

void WorkflowMain::clearMonitorConnections()
{
    // Clear any connections from previous state
    env->actionMgr->deleteActAll(sudsMonitorGuid);
    env->actionMgr->deleteActAll(syringesBusyMonitorGuid);
    env->actionMgr->deleteActAll(syringeVolMonitorGuid);
    env->actionMgr->deleteActAll(examEndedMonitorGuid);
    env->actionMgr->deleteActAll(statePathMonitorGuid);
    env->actionMgr->deleteActAll(mudsInsertWaitingMonitorGuid);
    env->actionMgr->deleteActAll(guidSubAction);
}

void WorkflowMain::resetWorkflowStatus()
{
    DS_WorkflowDef::WorkflowErrorStatus workflowErrorStatus = env->ds.workflowData->getWorkflowErrorStatus();
    workflowErrorStatus.state = DS_WorkflowDef::WORKFLOW_ERROR_STATE_NONE;
    workflowErrorStatus.syringeIndexFailed = SYRINGE_IDX_NONE;
    env->ds.workflowData->setWorkflowErrorStatus(workflowErrorStatus);
}

bool WorkflowMain::allInjectionsCompleted()
{
    const DS_ExamDef::InjectionPlan plan = env->ds.examData->getInjectionPlan();
    const DS_ExamDef::ExecutedSteps executedSteps = env->ds.examData->getExecutedSteps();
    return (( plan.steps.length() == executedSteps.length() ));
}

// There is concept of AutoPrime and FreeAutoPrime.
// FreeAutoPrime is what the user gets when they insert (supposedly) new SUDS and it auto-primes automatically without any user interaction
// User should get ONE FreeAutoPrime per Exam.
// Internally, they are both actAutoPrime however separating FreeAutoPrime to a function so reader can see the difference
void WorkflowMain::freeAutoPrime()
{
    env->ds.alertAction->activate("InitiatingAutoPrime");

    // treating re-inserted SUDS as new suds.
    env->ds.deviceData->setIsFirstAutoPrimeCompleted(false);

    // Instead of syncing setIsSudsUsed to act Completed, this will be linked to isFirstAutoPrimeCompleted
    // This is because IsFirstAutoPrimeComplete will be set much earlier than actAutoPrime completed.
    //   (i.e only after syringe priming is finished whereas actAutoPrime completes after all stopcock movements)
    // However actSetNeedsReplace relies on combination of both values and will not work correctly in corner cases (especially when the values aren't synced)
    // Also reference comments in actSetNeedsReplace
    QString autoPrimeCompletedMonitorGuid = Util::newGuid();
    QMetaObject::Connection conn = connect(env->ds.deviceData, &DS_DeviceData::signalDataChanged_IsFirstAutoPrimeCompleted, this, [=](bool isFirstAutoPrimeCompleted) {
        if (isFirstAutoPrimeCompleted)
        {
            env->ds.deviceData->setIsSudsUsed(false);
            env->actionMgr->deleteActAll(autoPrimeCompletedMonitorGuid);
        }
    });
    env->actionMgr->createActCompleted(autoPrimeCompletedMonitorGuid, conn, QString(__PRETTY_FUNCTION__) + ": Waiting for autoPrime to finish");

    actAutoPrime();
}

void WorkflowMain::handleSudsReinsertion()
{
    DS_SystemDef::StatePath statePath = env->ds.systemData->getStatePath();

    if (statePath == DS_SystemDef::STATE_PATH_IDLE)
    {
        LOG_DEBUG("handleSudsReinsertion(): StatePath is Idle\n");

        if (env->ds.deviceData->getFluidSourceSyringesBusy())
        {
            LOG_DEBUG("handleSudsReinsertion(): Waiting for Syringes Become Idle..\n");
            waitForSyringesToBecomeIdle(DS_WorkflowDef::STATE_NORMAL_WORKFLOW_SUDS_INSERT_WAITING);
        }
        else
        {
            DS_WorkflowDef::WorkflowErrorStatus workflowErrorStatus = env->ds.workflowData->getWorkflowErrorStatus();

            LOG_DEBUG("handleSudsReinsertion(): workflowErrorStatus=%s\n", Util::qVarientToJsonData(ImrParser::ToImr_WorkflowErrorStatus(workflowErrorStatus)).CSTR());

            if (workflowErrorStatus.state == DS_WorkflowDef::WORKFLOW_ERROR_STATE_NORMAL_SUDS_REMOVED_AFTER_FIRST_PRIMED)
            {
                if (env->ds.examData->isExamStarted() && allInjectionsCompleted() && env->ds.deviceData->getIsSudsUsed())
                {
                    LOG_INFO("handleSudsReinsertion(): Suds re-inserted after all injections are done. Auto-Priming...\n");
                    // actAutoPrime will set state to STATE_NORMAL_WORKFLOW_READY
                    freeAutoPrime();
                }
                else
                {
                    LOG_DEBUG("handleSudsReinsertion(): Suds re-inserted after first primed.\n");
                    env->ds.alertAction->activate("SUDSReinsertedPrimeRequired");
                    setState(DS_WorkflowDef::STATE_NORMAL_WORKFLOW_READY);
                }
            }
            else if (workflowErrorStatus.state == DS_WorkflowDef::WORKFLOW_ERROR_STATE_NORMAL_SUDS_REMOVED_AFTER_FIRST_PRIMED_AND_NEXT_PRIME_FAILED)
            {
                if (env->ds.examData->isExamStarted() && allInjectionsCompleted() && env->ds.deviceData->getIsSudsUsed())
                {
                    LOG_INFO("handleSudsReinsertion(): first freeAutoPrime didn't complete, re-doing freeAutoPrime");
                    // actAutoPrime will set state to STATE_NORMAL_WORKFLOW_READY
                    freeAutoPrime();
                }
                else
                {
                    LOG_INFO("handleSudsReinsertion(): Suds re-inserted after first prime done but next prime failed.\n");
                    env->ds.alertAction->activate("SUDSReinsertedPrimeRequired", "");
                    setState(DS_WorkflowDef::STATE_NORMAL_WORKFLOW_READY);
                }
            }
            else
            {
                resetWorkflowStatus();

                if (isMudsPrimeStartRequired())
                {
                    LOG_DEBUG("handleSudsReinsertion(): SudsInserted: Start MUDS Prime\n");
                    actMudsSodWithNewFluid();
                }
                else
                {
                    if ( (env->ds.cfgGlobal->isAutoEmptyEnabled(SYRINGE_IDX_SALINE)) ||
                         (env->ds.cfgGlobal->isAutoEmptyEnabled(SYRINGE_IDX_CONTRAST1)) ||
                         (env->ds.cfgGlobal->isAutoEmptyEnabled(SYRINGE_IDX_CONTRAST2)) )
                    {
                        LOG_DEBUG("handleSudsReinsertion(): SudsInserted for new exam with AutoEmptyEnabled: Start purging..\n");
                        env->ds.workflowAction->actAutoEmpty();
                    }
                    else
                    {
                        LOG_DEBUG("handleSudsReinsertion(): SudsInserted for new exam: Start Auto Prime\n");
                        freeAutoPrime();
                    }
                }
            }
        }
    }
    else
    {
        LOG_DEBUG("handleSudsReinsertion(): StatePath is not Idle, waiting for Idle..\n");

        env->actionMgr->deleteActAll(statePathMonitorGuid);
        statePathMonitorGuid = Util::newGuid();
        QMetaObject::Connection conn = connect(env->ds.systemData, &DS_SystemData::signalDataChanged_StatePath, this, [=](DS_SystemDef::StatePath statePath) {
            if (statePath != DS_SystemDef::STATE_PATH_IDLE)
            {
                // Do nothing, keep waiting
            }
            else
            {
                env->actionMgr->deleteActAll(statePathMonitorGuid);
                setState(DS_WorkflowDef::STATE_NORMAL_WORKFLOW_SUDS_INSERT_WAITING);
            }
        });

        env->actionMgr->createActCompleted(statePathMonitorGuid, conn, QString(__PRETTY_FUNCTION__) + ": handleSudsReinsertion: StatePath");
    }
}

void WorkflowMain::updateWorkflowErrorStatusFromSudsRemoved(const DS_DeviceDef::FluidSource &fluidSourceSuds, const DS_DeviceDef::FluidSource &prevFluidSourceSuds)
{
    // Updating WorkflowErrorStatus from Last SUDS Removed
    // NOTE: This call should be called before any of following callbacks:
    //       - waitForSyringesToBecomeIdle()
    //       - waitForSudsToBeInserted()
    //       - handleSudsReinsertion()

    DS_WorkflowDef::WorkflowErrorStatus workflowErrorStatus = env->ds.workflowData->getWorkflowErrorStatus();

    if (prevFluidSourceSuds.needsReplaced)
    {
        LOG_DEBUG("updateWorkflowErrorStatusFromSudsRemoved(): SUDS needs replaced, Auto Prime required.\n");
        workflowErrorStatus.state = DS_WorkflowDef::WORKFLOW_ERROR_STATE_NONE;
    }
    else if (env->ds.deviceData->getIsFirstAutoPrimeCompleted())
    {
        if (prevFluidSourceSuds.isReady)
        {
            LOG_DEBUG("updateWorkflowErrorStatusFromSudsRemoved(): SUDS Removed after first primed.\n");
            workflowErrorStatus.state = DS_WorkflowDef::WORKFLOW_ERROR_STATE_NORMAL_SUDS_REMOVED_AFTER_FIRST_PRIMED;
        }
        else
        {
            LOG_WARNING("updateWorkflowErrorStatusFromSudsRemoved(): SUDS Removed while SUDS is not ready\n");
            workflowErrorStatus.state = DS_WorkflowDef::WORKFLOW_ERROR_STATE_NORMAL_SUDS_REMOVED_AFTER_FIRST_PRIMED_AND_NEXT_PRIME_FAILED;
        }

        // Handle auto prime state when exam ended
        waitAndHandleExamEnded();
    }
    else
    {
        LOG_DEBUG("updateWorkflowErrorStatusFromSudsRemoved(): First Auto Prime required.\n");
        workflowErrorStatus.state = DS_WorkflowDef::WORKFLOW_ERROR_STATE_NORMAL_SUDS_REMOVED_BEFORE_FIRST_PRIME;
    }

    env->ds.workflowData->setWorkflowErrorStatus(workflowErrorStatus);
}

void WorkflowMain::clearBottleFillCount()
{
    LOG_INFO("clearBottleFillCount(): Bottle Fill Count is set to 0\n");
    env->ds.cfgLocal->set_Hidden_BottleFillCount(0);
}

void WorkflowMain::incrementBottleFillCount()
{
    int count = env->ds.cfgLocal->get_Hidden_BottleFillCount();
    env->ds.cfgLocal->set_Hidden_BottleFillCount(count + 1);
}

bool WorkflowMain::isNewBottleFill(const SyringeIdx syringeIdx)
{
    DS_DeviceDef::FluidSource fluidSourceBottle = env->ds.deviceData->getFluidSourceBottles().at(syringeIdx);
    DS_DeviceDef::FluidSource fluidSourceSyringe = env->ds.deviceData->getFluidSourceSyringes().at(syringeIdx);
    bool lastFillWasInsufficient = env->ds.alertAction->isActivatedWithSyringeIdx("InsufficientVolumeForReservoirAirCheck", syringeIdx);
    // if a fill ends up with InsufficientVolumeForReserviorAirCheck, we treat it same as needsReplaced. Any previous fill that raised this alert did not increment the counter.
    // if fluidSourceBottle.needsReplaced, means it was last observed empty
    // if fluidSourceSyringe.sourcePackages.length() is 0, means it's filling for the first time in this syringe (after MUDS installation)
    return (fluidSourceBottle.needsReplaced || (fluidSourceSyringe.sourcePackages.length() == 0) || lastFillWasInsufficient);
}
