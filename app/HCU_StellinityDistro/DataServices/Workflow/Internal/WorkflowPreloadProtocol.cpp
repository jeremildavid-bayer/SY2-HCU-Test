#include "Apps/AppManager.h"
#include "WorkflowPreloadProtocol.h"
#include "DataServices/Device/DS_DeviceData.h"
#include "DataServices/Exam/DS_ExamAction.h"
#include "DataServices/Exam/DS_ExamData.h"
#include "DataServices/Workflow/DS_WorkflowData.h"
#include "DataServices/Mcu/DS_McuData.h"
#include "DataServices/Alert/DS_AlertAction.h"
#include "DataServices/Device/DS_DeviceAction.h"

WorkflowPreloadProtocol::WorkflowPreloadProtocol(QObject *parent, EnvGlobal *env_) :
    ActionBaseExt(parent, env_)
{
    envLocal = new EnvLocal("DS_Workflow-PreloadProtocol", "WORKFLOW_PRELOAD_PROTOCOL");

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });
}

WorkflowPreloadProtocol::~WorkflowPreloadProtocol()
{
    delete envLocal;
}

void WorkflowPreloadProtocol::slotAppInitialised()
{
    connect(env->ds.workflowData, &DS_WorkflowData::signalDataChanged_WorkflowState, this, [=](DS_WorkflowDef::WorkflowState workflowState, DS_WorkflowDef::WorkflowState workflowStatePrev) {
        if ( (workflowState != DS_WorkflowDef::STATE_INACTIVE) &&
             (workflowStatePrev == DS_WorkflowDef::STATE_INACTIVE) )
        {
            env->ds.workflowData->setPreloadProtocolState(DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_READY);
            processState();
        }
        else if ( (workflowState == DS_WorkflowDef::STATE_INACTIVE) &&
                  (workflowStatePrev != DS_WorkflowDef::STATE_INACTIVE) )
        {
            // Destroy action instances
            env->actionMgr->deleteActAll(actStatusBuf.guid);
            env->actionMgr->deleteActAll(guidSubAction);
            env->actionMgr->deleteActAll(guidInjectionEndWaiting);

            env->ds.workflowData->setPreloadProtocolState(DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_INACTIVE);

            processState();
        }
    });
}

int WorkflowPreloadProtocol::getState()
{
    return env->ds.workflowData->getPreloadProtocolState();
}

QString WorkflowPreloadProtocol::getStateStr(int state)
{
    return ImrParser::ToImr_PreloadProtocolState((DS_WorkflowDef::PreloadProtocolState)state);
}

void WorkflowPreloadProtocol::setStateSynch(int newState)
{
    env->ds.workflowData->setPreloadProtocolState((DS_WorkflowDef::PreloadProtocolState)newState);
}

bool WorkflowPreloadProtocol::isSetStateReady()
{
    if (getState() == DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_INACTIVE)
    {
        return false;
    }
    return true;
}

void WorkflowPreloadProtocol::processState()
{
    DS_WorkflowDef::PreloadProtocolState state = (DS_WorkflowDef::PreloadProtocolState)getState();

    switch (state)
    {
    case DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_INACTIVE:
        LOG_INFO("PRELOAD_PROTOCOL_STATE_INACTIVE\n");
        break;
    case DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_READY:
        {
            LOG_INFO("PRELOAD_PROTOCOL_STATE_READY\n");
            actStatusBuf.err = "";
            env->actionMgr->deleteActAll(actStatusBuf.guid);
            env->actionMgr->deleteActAll(guidSubAction);
            env->actionMgr->deleteActAll(guidInjectionEndWaiting);

            if (env->ds.alertAction->isActivated("RepreloadReprimeRequired", "", true))
            {
                env->ds.alertAction->deactivate("RepreloadReprimeRequired");
            }

            if (env->ds.alertAction->isActivated("SRUPreloadingSUDS", "", true))
            {
                env->ds.alertAction->deactivate("SRUPreloadingSUDS");
            }
        }
        break;
    case DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_STARTED:
        LOG_INFO("PRELOAD_PROTOCOL_STATE_STARTED\n");
        if (userConfirmRequired)
        {
            setState(DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_USER_CONFIRM_WAITING);
        }
        else
        {
            setState(DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_PRELOAD_ARM_STARTED);
        }
        break;
    case DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_USER_CONFIRM_WAITING:
        LOG_INFO("PRELOAD_PROTOCOL_STATE_USER_CONFIRM_WAITING\n");
        // Wait for user interaction
        break;
    case DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_PRELOAD_ARM_STARTED:
        LOG_INFO("PRELOAD_PROTOCOL_STATE_PRELOAD_ARM_STARTED\n");
        {
            // Clear Reprime Required Alert
            if (!env->ds.alertAction->isActivated("SRUPreloadingSUDS", "", true))
            {
                env->ds.alertAction->activate("SRUPreloadingSUDS");
            }

            env->ds.deviceData->setIsSudsUsed(true);
            env->ds.examAction->actResetPreloadSteps();
            handleSubAction(DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_PRELOAD_ARM_PROGRESS, DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_PRELOAD_ARM_DONE, DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_PRELOAD_ARM_FAILED);

            // For preload arm, action process can complete within the actArm(). Therefore, it is required to set the current state to progress state.
            setState(DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_PRELOAD_ARM_PROGRESS);

            env->ds.examAction->actArm(DS_ExamDef::ARM_TYPE_PRELOAD_FIRST, guidSubAction);
        }
        break;
    case DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_PRELOAD_ARM_PROGRESS:
        LOG_INFO("PRELOAD_PROTOCOL_STATE_PRELOAD_ARM_PROGRESS\n");
        break;
    case DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_PRELOAD_ARM_FAILED:
        LOG_ERROR("PRELOAD_PROTOCOL_STATE_PRELOAD_ARM_FAILED: ARM Failed: Status=%s\n", ImrParser::ToImr_DataServiceActionStatusStr(actStatusBuf).CSTR());
        setState(DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_FAILED);
        break;
    case DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_PRELOAD_ARM_DONE:
        LOG_INFO("PRELOAD_PROTOCOL_STATE_PRELOAD_ARM_DONE\n");
        setState(DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_PRELOAD_INJECTION_STARTED);
        break;
    case DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_PRELOAD_INJECTION_STARTED:
        {
            LOG_INFO("PRELOAD_PROTOCOL_STATE_PRELOAD_INJECTION_STARTED\n");
            DS_ExamDef::InjectionStep preloadStep = env->ds.examData->getPreloadedStep1();

            DS_McuDef::InjectionProtocol mcuInjectProtocol;
            env->ds.examAction->actGetMcuProtocolFromStep(preloadStep, mcuInjectProtocol);

            handleSubAction(DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_PRELOAD_INJECTION_PROGRESS, DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_PRELOAD_INJECTION_DONE, DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_PRELOAD_INJECTION_FAILED);
            env->ds.deviceAction->actMudsPreloadPrime(mcuInjectProtocol, guidSubAction);
        }
        break;
    case DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_PRELOAD_INJECTION_PROGRESS:
        LOG_INFO("PRELOAD_PROTOCOL_STATE_PRELOAD_INJECTION_PROGRESS\n");
        break;
    case DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_PRELOAD_INJECTION_FAILED:
        LOG_ERROR("PRELOAD_PROTOCOL_STATE_PRELOAD_INJECTION_FAILED: status=%s\n", ImrParser::ToImr_DataServiceActionStatusStr(actStatusBuf).CSTR());
        setState(DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_FAILED);
        break;
    case DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_PRELOAD_INJECTION_END_WAITING:
        LOG_INFO("PRELOAD_PROTOCOL_STATE_PRELOAD_INJECTION_END_WAITING\n");
        break;
    case DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_PRELOAD_INJECTION_DONE:
        LOG_INFO("PRELOAD_PROTOCOL_STATE_PRELOAD_INJECTION_DONE\n");
        setState(DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_DONE);
        break;
    case DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_USER_ABORTED:
        LOG_WARNING("PRELOAD_PROTOCOL_STATE_USER_ABORTED\n");
        actStatusBuf.err = "USER_ABORT";
        actStatusBuf.state = DS_ACTION_STATE_USER_ABORT;
        actionCompleted(actStatusBuf);
        setState(DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_READY);
        break;
    case DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_FAILED:
        LOG_ERROR("PRELOAD_PROTOCOL_STATE_FAILED: Status=%s\n", ImrParser::ToImr_DataServiceActionStatusStr(actStatusBuf).CSTR());
        actionCompleted(actStatusBuf);
        setState(DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_READY);
        break;
    case DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_DONE:
        LOG_INFO("PRELOAD_PROTOCOL_STATE_DONE\n");
        {
            LOG_INFO("PRELOAD_PROTOCOL_STATE_DONE: Setting SUDS as primed.\n");

            DS_ExamDef::InjectionPlan plan = env->ds.examData->getInjectionPlan();
            DS_ExamDef::InjectionStep *executingStep = plan.getExecutingStep(env->ds.systemData->getStatePath(), env->ds.examData->getExecutedSteps());
            if (executingStep != NULL)
            {
                QString alertData = QString().asprintf("%s;%s;%s", plan.guid.CSTR(), executingStep->guid.CSTR(), env->ds.examData->getSUDSLength().CSTR());
                env->ds.alertAction->activate("ProtocolPreloaded", alertData);

                executingStep->isPreloaded = true;
                env->ds.examData->setInjectionPlan(plan);
            }
            env->ds.examAction->actAutoRefill("PreloadProtocolCompleted");
            actionCompleted(actStatusBuf);
            setState(DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_READY);
        }
        break;
    default:
        LOG_ERROR("PRELOAD_PROTOCOL_STATE_UNKNOWN (%s)\n", ImrParser::ToImr_PreloadProtocolState(state).CSTR());
        break;
    }
}

DataServiceActionStatus WorkflowPreloadProtocol::actPreloadProtocolStart(bool userConfirmRequired_, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "PreloadProtocolStart", QString().asprintf("%s", userConfirmRequired_ ? "UserConfirmRequired" : "UserConfirmSkip"));

    actStatusBuf = status;
    actStatusBuf.state = DS_ACTION_STATE_STARTED;

    actionStarted(actStatusBuf);

    userConfirmRequired = userConfirmRequired_;
    setState(DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_STARTED);

    return actStatusBuf;
}

DataServiceActionStatus WorkflowPreloadProtocol::actPreloadProtocolResume(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "PreloadProtocolResume");

    DS_WorkflowDef::PreloadProtocolState state = (DS_WorkflowDef::PreloadProtocolState)getState();
    DS_WorkflowDef::PreloadProtocolState newState;

    switch (state)
    {
    case DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_USER_CONFIRM_WAITING:
        newState = DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_PRELOAD_ARM_STARTED;
        break;
    default:
        status.err = "Invalid Preload Protocol State: " + getStateStr(state);
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

DataServiceActionStatus WorkflowPreloadProtocol::actPreloadProtocolAbort(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "actPreloadProtocolAbort");

    DS_WorkflowDef::PreloadProtocolState state = (DS_WorkflowDef::PreloadProtocolState)getState();
    DS_WorkflowDef::PreloadProtocolState newState;

    switch (state)
    {
    case DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_USER_CONFIRM_WAITING:
        newState = DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_USER_ABORTED;
        break;
    default:
        status.err = "Invalid Preload Protocol State: " + getStateStr(state);
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
