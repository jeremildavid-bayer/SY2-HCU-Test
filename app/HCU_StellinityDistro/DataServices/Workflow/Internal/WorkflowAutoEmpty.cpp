#include "Apps/AppManager.h"
#include "WorkflowAutoEmpty.h"
#include "DataServices/Workflow/DS_WorkflowData.h"
#include "DataServices/Mcu/DS_McuData.h"
#include "DataServices/Workflow/DS_WorkflowAction.h"
#include "DataServices/Device/DS_DeviceData.h"
#include "DataServices/Device/DS_DeviceAction.h"
#include "DataServices/Alert/DS_AlertAction.h"
#include "DataServices/CfgGlobal/DS_CfgGlobal.h"
#include "DataServices/CfgLocal/DS_CfgLocal.h"
#include "DataServices/Mcu/DS_McuAction.h"

WorkflowAutoEmpty::WorkflowAutoEmpty(QObject *parent, EnvGlobal *env_) :
    ActionBaseExt(parent, env_)
{
    envLocal = new EnvLocal("DS_Workflow-AutoEmpty", "WORKFLOW_AUTO_EMPTY");

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });

    autoEmptySyringeIdxList.clear();
}

WorkflowAutoEmpty::~WorkflowAutoEmpty()
{
    delete envLocal;
}

void WorkflowAutoEmpty::slotAppInitialised()
{
    connect(env->ds.workflowData, &DS_WorkflowData::signalDataChanged_WorkflowState, this, [=](DS_WorkflowDef::WorkflowState workflowState, DS_WorkflowDef::WorkflowState workflowStatePrev) {
        if ( (workflowState != DS_WorkflowDef::STATE_INACTIVE) &&
             (workflowStatePrev == DS_WorkflowDef::STATE_INACTIVE) )
        {
            env->ds.workflowData->setAutoEmptyState(DS_WorkflowDef::AUTO_EMPTY_STATE_READY);
            processState();
        }
        else if ( (workflowState == DS_WorkflowDef::STATE_INACTIVE) &&
                  (workflowStatePrev != DS_WorkflowDef::STATE_INACTIVE) )
        {
            env->ds.workflowData->setAutoEmptyState(DS_WorkflowDef::AUTO_EMPTY_STATE_INACTIVE);
            processState();
        }
    });
}

int WorkflowAutoEmpty::getState()
{
    return env->ds.workflowData->getAutoEmptyState();
}

QString WorkflowAutoEmpty::getStateStr(int state)
{
    return ImrParser::ToImr_AutoEmptyState((DS_WorkflowDef::AutoEmptyState)state);
}

void WorkflowAutoEmpty::setStateSynch(int newState)
{
    env->ds.workflowData->setAutoEmptyState((DS_WorkflowDef::AutoEmptyState)newState);
}

bool WorkflowAutoEmpty::isSetStateReady()
{
    if (getState() == DS_WorkflowDef::AUTO_EMPTY_STATE_INACTIVE)
    {
        return false;
    }
    return true;
}

void WorkflowAutoEmpty::processState()
{
    DS_WorkflowDef::AutoEmptyState state = (DS_WorkflowDef::AutoEmptyState)getState();
    switch (state)
    {
    case DS_WorkflowDef::AUTO_EMPTY_STATE_INACTIVE:
        LOG_INFO("AUTO_EMPTY_STATE_INACTIVE\n");
        break;
    case DS_WorkflowDef::AUTO_EMPTY_STATE_READY:
        LOG_INFO("AUTO_EMPTY_STATE_READY\n");
        env->actionMgr->deleteActAll(actStatusBuf.guid);
        env->actionMgr->deleteActAll(guidSubAction);
        break;
    case DS_WorkflowDef::AUTO_EMPTY_STATE_STARTED:
        LOG_INFO("AUTO_EMPTY_STATE_STARTED\n");
        // Determine the syringe to auto-empty
        if (autoEmptySyringeIdxList.length() > 0)
        {
            emptySyringeIdx = autoEmptySyringeIdxList[0];
            autoEmptySyringeIdxList.pop_front();
            setState(DS_WorkflowDef::AUTO_EMPTY_STATE_FLUID_REMOVAL_STARTED);
        }
        else
        {
            LOG_INFO("AUTO_EMPTY_STATE_STARTED: All syringes are purged\n");
            setState(DS_WorkflowDef::AUTO_EMPTY_STATE_DONE);
            return;
        }
        break;

    //==================================================================
    case DS_WorkflowDef::AUTO_EMPTY_STATE_FLUID_REMOVAL_STARTED:
        LOG_INFO("AUTO_EMPTY_STATE_FLUID_REMOVAL_STARTED[%s]\n", ImrParser::ToImr_FluidSourceSyringeIdx(emptySyringeIdx).CSTR());
        env->actionMgr->deleteActAll(guidSubAction);
        handleSubAction(DS_WorkflowDef::AUTO_EMPTY_STATE_FLUID_REMOVAL_PROGRESS, DS_WorkflowDef::AUTO_EMPTY_STATE_FLUID_REMOVAL_DONE, DS_WorkflowDef::AUTO_EMPTY_STATE_FLUID_REMOVAL_FAILED);
        env->ds.workflowAction->actFluidRemovalStart(emptySyringeIdx, true, guidSubAction);
        break;
    case DS_WorkflowDef::AUTO_EMPTY_STATE_FLUID_REMOVAL_PROGRESS:
        LOG_INFO("AUTO_EMPTY_STATE_FLUID_REMOVAL_PROGRESS[%s]\n", ImrParser::ToImr_FluidSourceSyringeIdx(emptySyringeIdx).CSTR());
        break;
    case DS_WorkflowDef::AUTO_EMPTY_STATE_FLUID_REMOVAL_DONE:
        LOG_INFO("AUTO_EMPTY_STATE_FLUID_REMOVAL_DONE[%s]\n", ImrParser::ToImr_FluidSourceSyringeIdx(emptySyringeIdx).CSTR());
        // Raise applicable alert for the operation
        env->ds.alertAction->activate("SRUAutoEmptied", ImrParser::ToImr_FluidSourceSyringeIdx(emptySyringeIdx).CSTR());
        // Continue with remaining syringes
        setState(DS_WorkflowDef::AUTO_EMPTY_STATE_STARTED);
        break;
    case DS_WorkflowDef::AUTO_EMPTY_STATE_FLUID_REMOVAL_FAILED:
        LOG_ERROR("AUTO_EMPTY_STATE_FLUID_REMOVAL_FAILED[%s]\n", ImrParser::ToImr_FluidSourceSyringeIdx(emptySyringeIdx).CSTR());
        setState(DS_WorkflowDef::AUTO_EMPTY_STATE_FAILED);
        break;
    case DS_WorkflowDef::AUTO_EMPTY_STATE_FAILED:
        LOG_ERROR("AUTO_EMPTY_STATE_FAILED: Status=%s\n", ImrParser::ToImr_DataServiceActionStatusStr(actStatusBuf).CSTR());
        actionCompleted(actStatusBuf);
        setState(DS_WorkflowDef::AUTO_EMPTY_STATE_READY);
        break;
    case DS_WorkflowDef::AUTO_EMPTY_STATE_DONE:
        LOG_INFO("AUTO_EMPTY_STATE_DONE\n");
        actionCompleted(actStatusBuf);
        setState(DS_WorkflowDef::AUTO_EMPTY_STATE_READY);
        break;
    default:
        LOG_ERROR("Invalid State to Process (%d)\n", state);
        setState(DS_WorkflowDef::AUTO_EMPTY_STATE_READY);
        break;
    }
}

DataServiceActionStatus WorkflowAutoEmpty::actAutoEmptyStart(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "AutoEmpty:Start");

    DS_WorkflowDef::WorkflowState workflowState = env->ds.workflowData->getWorkflowState();
    DS_WorkflowDef::AutoEmptyState autoEmptyState = env->ds.workflowData->getAutoEmptyState();
    if ( (workflowState != DS_WorkflowDef::STATE_AUTO_EMPTY_STARTED) &&
         (autoEmptyState != DS_WorkflowDef::AUTO_EMPTY_STATE_READY) )
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = QString().asprintf("%s", ImrParser::ToImr_AutoEmptyState(autoEmptyState).CSTR());
        actionStarted(status);
        return status;
    }

    fillAutoEmptySyringeList(autoEmptySyringeIdxList);

    if (autoEmptySyringeIdxList.length() > 0)
    {
        // Next prime after auto-empty requires extended auto prime volume.
        LOG_INFO("actAutoEmptyStart(): Enabling ExtendedAutoPrime Volume\n");
        env->ds.workflowData->setUseExtendedAutoPrimeVolume(true);
    }
    else
    {
        env->ds.workflowData->setUseExtendedAutoPrimeVolume(false);
    }

    // Ready to start
    actStatusBuf = status;
    actStatusBuf.state = DS_ACTION_STATE_STARTED;
    actionStarted(actStatusBuf);

    setState(DS_WorkflowDef::AUTO_EMPTY_STATE_STARTED);
    return actStatusBuf;
}

void WorkflowAutoEmpty::fillAutoEmptySyringeList(QList<SyringeIdx> &autoEmptySyringeList)
{
    autoEmptySyringeIdxList.clear();

    for (int idx = SYRINGE_IDX_SALINE; idx < SYRINGE_IDX_MAX; idx++)
    {
        if (shouldAutoEmptySyringe((SyringeIdx)idx))
        {
            autoEmptySyringeList.push_front((SyringeIdx)idx);
        }
    }
}

bool WorkflowAutoEmpty::shouldAutoEmptySyringe(SyringeIdx idx)
{
    auto fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
    auto syringesUsedInLastExam = env->ds.cfgLocal->get_Hidden_SyringesUsedInLastExam();
    auto fluidSourceSyringe = fluidSourceSyringes[idx];
    bool isSyringeReady = fluidSourceSyringe.readyForInjection(); // check if syringe went through SOD
    bool autoEmptyEnabledInConfig = env->ds.cfgGlobal->isAutoEmptyEnabled(idx);
    bool syringeUsedInLastExam = (syringesUsedInLastExam.count() > idx) && syringesUsedInLastExam[idx].toBool();
    return (isSyringeReady && autoEmptyEnabledInConfig && syringeUsedInLastExam);
}
