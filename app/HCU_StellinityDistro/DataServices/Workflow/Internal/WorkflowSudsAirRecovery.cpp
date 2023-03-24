#include "Apps/AppManager.h"
#include "WorkflowSudsAirRecovery.h"
#include "DataServices/Workflow/DS_WorkflowData.h"
#include "DataServices/Workflow/DS_WorkflowAction.h"
#include "DataServices/Device/DS_DeviceData.h"
#include "DataServices/Device/DS_DeviceAction.h"
#include "DataServices/Exam/DS_ExamData.h"
#include "DataServices/Alert/DS_AlertAction.h"
#include "Common/ImrParser.h"

WorkflowSudsAirRecovery::WorkflowSudsAirRecovery(QObject *parent, EnvGlobal *env_) :
    ActionBaseExt(parent, env_)
{
    envLocal = new EnvLocal("DS_Workflow-SudsAirRecovery", "WORKFLOW_SUDS_AIR_RECOVERY");

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });

    syringeAirCheckRequired = SYRINGE_IDX_NONE;
}

WorkflowSudsAirRecovery::~WorkflowSudsAirRecovery()
{
    delete envLocal;
}

void WorkflowSudsAirRecovery::slotAppInitialised()
{
    connect(env->ds.workflowData, &DS_WorkflowData::signalDataChanged_WorkflowState, this, [=](DS_WorkflowDef::WorkflowState workflowState, DS_WorkflowDef::WorkflowState workflowStatePrev) {
        if ( (workflowState != DS_WorkflowDef::STATE_INACTIVE) &&
             (workflowStatePrev == DS_WorkflowDef::STATE_INACTIVE) )
        {
            env->ds.workflowData->setSudsAirRecoveryState(DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_READY);
            processState();
        }
        else if ( (workflowState == DS_WorkflowDef::STATE_INACTIVE) &&
                  (workflowStatePrev != DS_WorkflowDef::STATE_INACTIVE) )
        {
            // Destroy action instances
            env->actionMgr->deleteActAll(actStatusBuf.guid);
            env->actionMgr->deleteActAll(guidSubAction);
            env->actionMgr->deleteActAll(guidStatePathIdleWaiting);

            env->ds.workflowData->setSudsAirRecoveryState(DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_INACTIVE);

            processState();
        }
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_MudsInserted, this, [=](bool inserted) {
        if (!inserted)
        {
            DS_WorkflowDef::SudsAirRecoveryState state = env->ds.workflowData->getSudsAirRecoveryState();
            if ( (state != DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_INACTIVE) &&
                 (state != DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_READY) )
            {
                LOG_WARNING("%s: Muds Removed. Operation abroted.\n", getStateStr(state).CSTR());
                actStatusBuf.err = "MUDS_REMOVED";
                actStatusBuf.state = DS_ACTION_STATE_INVALID_STATE;
                setState(DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_DONE);
            }
        }
    });
}

int WorkflowSudsAirRecovery::getState()
{
    return env->ds.workflowData->getSudsAirRecoveryState();
}

QString WorkflowSudsAirRecovery::getStateStr(int state)
{
    return ImrParser::ToImr_SudsAirRecoveryState((DS_WorkflowDef::SudsAirRecoveryState)state);
}

void WorkflowSudsAirRecovery::setStateSynch(int newState)
{
    env->ds.workflowData->setSudsAirRecoveryState((DS_WorkflowDef::SudsAirRecoveryState)newState);
}

bool WorkflowSudsAirRecovery::isSetStateReady()
{
    DS_WorkflowDef::SudsAirRecoveryState state = env->ds.workflowData->getSudsAirRecoveryState();
    if (state == DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_INACTIVE)
    {
        return false;
    }
    return true;
}

void WorkflowSudsAirRecovery::processState()
{
    DS_WorkflowDef::SudsAirRecoveryState state = env->ds.workflowData->getSudsAirRecoveryState();
    switch (state)
    {
    case DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_INACTIVE:
        LOG_INFO("STATE_INACTIVE\n");
        break;
    case DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_READY:
        LOG_INFO("STATE_READY\n");
        break;
    case DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_STARTED:
        LOG_INFO("STATE_STARTED\n");
        {
            env->ds.deviceAction->actSudsSetNeedsPrime(true);
            env->ds.deviceData->setIsFirstAutoPrimeCompleted(false);

            // Identify Contrast Syringe requires Prime
            DS_McuDef::InjectionProtocol mcuInjProtocol = env->ds.mcuData->getInjectionProtocol();
            int mcuCurPhaseIdx = env->ds.mcuData->getInjectDigest().phaseIdx;
            syringeAirCheckRequired = SYRINGE_IDX_NONE;

            for (int mcuPhaseIdx = mcuCurPhaseIdx; mcuPhaseIdx >= 0; mcuPhaseIdx--)
            {
                if ( (mcuInjProtocol.phases[mcuPhaseIdx].type == DS_McuDef::INJECTION_PHASE_TYPE_CONTRAST1) ||
                     (mcuInjProtocol.phases[mcuPhaseIdx].type == DS_McuDef::INJECTION_PHASE_TYPE_DUAL1) )
                {
                    syringeAirCheckRequired = SYRINGE_IDX_CONTRAST1;
                    break;
                }

                if ( (mcuInjProtocol.phases[mcuPhaseIdx].type == DS_McuDef::INJECTION_PHASE_TYPE_CONTRAST2) ||
                     (mcuInjProtocol.phases[mcuPhaseIdx].type == DS_McuDef::INJECTION_PHASE_TYPE_DUAL2) )
                {
                    syringeAirCheckRequired = SYRINGE_IDX_CONTRAST2;
                    break;
                }
            }

            LOG_WARNING("Syringe(%s) might have air. Need to perform SyringeAirCheck..\n", ImrParser::ToImr_FluidSourceSyringeIdx(syringeAirCheckRequired).CSTR());
            setState(DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_WAIT_INJECTION_COMPLETE);
        }
        break;
    //-----------------------------------
    case DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_WAIT_INJECTION_COMPLETE:
        LOG_INFO("STATE_WAIT_INJECTION_COMPLETE\n");
        {
            // Monitor StatePath state
            env->actionMgr->deleteActAll(guidStatePathIdleWaiting);
            guidStatePathIdleWaiting = Util::newGuid();

            QMetaObject::Connection conn = connect(env->ds.systemData, &DS_SystemData::signalDataChanged_StatePath, this, [=](DS_SystemDef::StatePath statePath) {
                DS_WorkflowDef::SudsAirRecoveryState curState = env->ds.workflowData->getSudsAirRecoveryState();
                if (curState != DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_WAIT_INJECTION_COMPLETE)
                {
                    env->actionMgr->deleteActCompleted(guidStatePathIdleWaiting);
                    return;
                }

                if (statePath == DS_SystemDef::STATE_PATH_IDLE)
                {
                    env->actionMgr->deleteActAll(guidStatePathIdleWaiting);
                    setState(DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_USER_START_WAITING);
                }
            });
            env->actionMgr->createActCompleted(guidStatePathIdleWaiting, conn, QString(__PRETTY_FUNCTION__) + ": STATE_WAIT_INJECTION_COMPLETE: StatePath");
        }
        break;
    //-----------------------------------
    case DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_USER_START_WAITING:
        LOG_INFO("STATE_USER_START_WAITING\n");
        env->actionMgr->deleteActAll(guidStatePathIdleWaiting);
        // Wait for user interaction
        break;
    case DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_USER_START_CONFIRMED:
        if (syringeAirCheckRequired == SYRINGE_IDX_NONE)
        {
            LOG_INFO("No contrast syringe is required for air check.\n");
            setState(DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_DONE_WITH_AUTO_PRIME_OPERATION_USER_WAITING);
        }
        else
        {
            LOG_INFO("Air Check will be performed in %s\n", ImrParser::ToImr_FluidSourceSyringeIdx(syringeAirCheckRequired).CSTR());
            syringePrimeCount = 0;
            setState(DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_SYRINGE_PRIME_STARTED);
        }
        break;
    //-----------------------------------
    case DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_SYRINGE_PRIME_STARTED:
        LOG_INFO("STATE_SYRINGE_PRIME_STARTED\n");
        syringePrimeCount++;
        handleSubAction(DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_SYRINGE_PRIME_PROGRESS, DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_SYRINGE_PRIME_DONE, DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_SYRINGE_PRIME_FAILED);
        env->ds.deviceAction->actSyringePrime(syringeAirCheckRequired, DS_DeviceDef::SUDS_PRIME_TYPE_SYRINGE_PRIME, guidSubAction);
        break;
    case DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_SYRINGE_PRIME_PROGRESS:
        LOG_INFO("STATE_SYRINGE_PRIME_PROGRESS\n");
        break;
    case DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_SYRINGE_PRIME_FAILED:
        LOG_ERROR("STATE_SYRINGE_PRIME_FAILED\n");
        setState(DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_OUTLET_AIR_CHECK_STARTED);
        break;
    case DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_SYRINGE_PRIME_DONE:
        LOG_INFO("STATE_SYRINGE_PRIME_DONE\n");
        setState(DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_OUTLET_AIR_CHECK_STARTED);
        break;
    //-----------------------------------
    case DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_OUTLET_AIR_CHECK_STARTED:
        LOG_INFO("STATE_OUTLET_AIR_CHECK_STARTED\n");
        if (env->ds.mcuData->getSudsBubbleDetected())
        {
            setState(DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_OUTLET_AIR_CHECK_FAILED);
        }
        else
        {
            setState(DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_OUTLET_AIR_CHECK_DONE);
        }
        break;
    case DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_OUTLET_AIR_CHECK_FAILED:
        LOG_ERROR("STATE_OUTLET_AIR_CHECK_FAILED\n");
        if (syringePrimeCount < SUDS_AIR_RECOVERY_SYRINGE_PRIME_TRIALS_LIMIT)
        {
            LOG_WARNING("PrimeRetry[%d]: Air Detected from outlet air sensor. Priming..\n", syringePrimeCount);
            setState(DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_SYRINGE_PRIME_STARTED);
        }
        else
        {
            LOG_WARNING("PrimeRetry[%d]: Air Detected from outlet air sensor. Go to next step anyway..\n", syringePrimeCount);
            setState(DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_OUTLET_AIR_CHECK_DONE);
        }
        break;
    case DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_OUTLET_AIR_CHECK_DONE:
        LOG_INFO("STATE_OUTLET_AIR_CHECK_DONE\n");
        setState(DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_SYRINGE_AIR_CHECK_STARTED);
        break;
    //-----------------------------------
    case DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_SYRINGE_AIR_CHECK_STARTED:
        LOG_INFO("STATE_SYRINGE_AIR_CHECK_STARTED\n");
        handleSubAction(DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_SYRINGE_AIR_CHECK_PROGRESS, DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_SYRINGE_AIR_CHECK_DONE, DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_SYRINGE_AIR_CHECK_FAILED);
        env->ds.deviceAction->actSyringeAirCheck(syringeAirCheckRequired, guidSubAction);
        break;
    case DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_SYRINGE_AIR_CHECK_PROGRESS:
        LOG_INFO("STATE_SYRINGE_AIR_CHECK_PROGRESS\n");
        break;
    case DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_SYRINGE_AIR_CHECK_FAILED:
        LOG_ERROR("STATE_SYRINGE_AIR_CHECK_FAILED\n");
        LOG_WARNING("Air Check failed (air detected or HW Fault) should be handled from DeviceSyringe.\n");
        setState(DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_DONE);
        break;
    case DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_SYRINGE_AIR_CHECK_DONE:
        LOG_INFO("STATE_SYRINGE_AIR_CHECK_DONE\n");
        LOG_INFO("Air Check completed. No Air is found in %s\n", ImrParser::ToImr_FluidSourceSyringeIdx(syringeAirCheckRequired).CSTR());
        setState(DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_DONE_WITH_AUTO_PRIME_OPERATION_USER_WAITING);
        break;
    //-----------------------------------
    case DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_DONE_WITH_AUTO_PRIME_OPERATION_USER_WAITING:
        LOG_INFO("SUDS_AIR_RECOVERY_STATE_DONE_WITH_AUTO_PRIME_OPERATION_USER_WAITING\n");
        // Wait for user interaction
        break;
    case DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_DONE_WITH_AUTO_PRIME_OPERATION_USER_CONFIRMED:
        LOG_INFO("SUDS_AIR_RECOVERY_STATE_DONE_WITH_AUTO_PRIME_OPERATION_USER_CONFIRMED\n");
        setState(DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_DONE);
        break;
    case DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_DONE:
        LOG_INFO("DONE\n");
        if (actStatusBuf.err == "")
        {
            actStatusBuf.state = DS_ACTION_STATE_COMPLETED;
        }

        actionCompleted(actStatusBuf);
        setState(DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_READY);
        break;
    default:
        LOG_ERROR("Invalid State (%d)\n", state);
        break;
    }
}

DataServiceActionStatus WorkflowSudsAirRecovery::actSudsAirRecoveryStart(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "SudsAirRecovery:Start");
    DS_WorkflowDef::WorkflowState workflowState = env->ds.workflowData->getWorkflowState();

    if (workflowState != DS_WorkflowDef::STATE_SUDS_AIR_RECOVERY_STARTED)
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "Invalid State";
        actionStarted(status);
        return status;
    }

    actStatusBuf = status;
    actStatusBuf.state = DS_ACTION_STATE_STARTED;

    actionStarted(actStatusBuf);

    setState(DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_STARTED);

    return actStatusBuf;
}

DataServiceActionStatus WorkflowSudsAirRecovery::actSudsAirRecoveryResume(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "SudsAirRecovery:Resume");

    DS_WorkflowDef::WorkflowState workflowState = env->ds.workflowData->getWorkflowState();
    if ( (workflowState != DS_WorkflowDef::STATE_SUDS_AIR_RECOVERY_STARTED) &&
         (workflowState != DS_WorkflowDef::STATE_SUDS_AIR_RECOVERY_PROGRESS) )
    {
        status.err = "Invalid Workflow State: " + ImrParser::ToImr_WorkFlowState(workflowState);
        status.state = DS_ACTION_STATE_INVALID_STATE;
        actionStarted(status);
        return status;
    }

    DS_WorkflowDef::SudsAirRecoveryState state = env->ds.workflowData->getSudsAirRecoveryState();
    DS_WorkflowDef::SudsAirRecoveryState newState;

    switch (state)
    {
    case DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_USER_START_WAITING:
        newState = DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_USER_START_CONFIRMED;
        break;
    case DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_DONE_WITH_AUTO_PRIME_OPERATION_USER_WAITING:
        newState = DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_DONE_WITH_AUTO_PRIME_OPERATION_USER_CONFIRMED;
        break;
    default:
        status.err = "Invalid Suds Air Recovery State: " + ImrParser::ToImr_SudsAirRecoveryState(state);
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
