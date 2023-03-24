#include "Apps/AppManager.h"
#include "DeviceMudsDisengage.h"
#include "Common/ImrParser.h"
#include "DataServices/Workflow/DS_WorkflowData.h"
#include "DataServices/Alert/DS_AlertAction.h"
#include "DataServices/Device/DS_DeviceData.h"
#include "DataServices/Mcu/DS_McuAction.h"

DeviceMudsDisengage::DeviceMudsDisengage(QObject *parent, EnvGlobal *env_, EnvLocal *envLocal_) :
    ActionBaseExt(parent, env_)
{
    envLocal = envLocal_;

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });

    state = STATE_READY;
}

DeviceMudsDisengage::~DeviceMudsDisengage()
{
}

void DeviceMudsDisengage::slotAppInitialised()
{
    connect(env->ds.workflowData, &DS_WorkflowData::signalDataChanged_WorkflowState, this, [=](DS_WorkflowDef::WorkflowState workflowState, DS_WorkflowDef::WorkflowState workflowStatePrev) {
        if ( (workflowState != DS_WorkflowDef::STATE_INACTIVE) &&
             (workflowStatePrev == DS_WorkflowDef::STATE_INACTIVE) )
        {
            state = STATE_READY;
            processState();
        }
        else if ( (workflowState == DS_WorkflowDef::STATE_INACTIVE) &&
                  (workflowStatePrev != DS_WorkflowDef::STATE_INACTIVE) )
        {
            env->actionMgr->deleteActAll(actStatusBuf.guid);
            state = STATE_INACTIVE;
        }
    });
}

DataServiceActionStatus DeviceMudsDisengage::actDisengageAll(int retryLimit, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "MudsDisengageAll", QString().asprintf("%d", retryLimit));

    if ( (state != STATE_READY) &&
         (state != STATE_INACTIVE) )
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = QString().asprintf("Disengage in progress, state=%d", state);
        actionStarted(status);
        return status;
    }

    actStatusBuf = status;
    actStatusBuf.state = DS_ACTION_STATE_STARTED;

    disengageRetryLimit = retryLimit;
    disengageRetryCount = 0;

    setState(STATE_STARTED);

    actionStarted(actStatusBuf);
    return actStatusBuf;
}

bool DeviceMudsDisengage::isSetStateReady()
{
    if (state == STATE_INACTIVE)
    {
        return false;
    }
    return true;
}

void DeviceMudsDisengage::processState()
{
    switch (state)
    {
    case STATE_INACTIVE:
        LOG_INFO("DISENGAGE: STATE_INACTIVE\n");
        break;
    case STATE_READY:
        LOG_INFO("DISENGAGE: STATE_READY\n");
        break;
    case STATE_STARTED:
        LOG_INFO("DISENGAGE: STATE_STARTED\n");
        {
            DS_DeviceDef::FluidSource fluidSourceMuds = env->ds.deviceData->getFluidSourceMuds();
            fluidSourceMuds.isReady = false;
            env->ds.deviceData->setFluidSourceMuds(fluidSourceMuds);

            handleSubAction(STATE_PROGRESS, STATE_DONE, STATE_FAILED);
            env->ds.mcuAction->actDisengageAll(guidSubAction);
        }
        break;
    case STATE_PROGRESS:
        LOG_INFO("DISENGAGE: STATE_PROGRESS\n");
        break;
    case STATE_FAILED:
        LOG_ERROR("DISENGAGE: STATE_FAILED: Trial%d: Status=%s\n", disengageRetryCount, ImrParser::ToImr_DataServiceActionStatusStr(actStatusBuf).CSTR());
        if (actStatusBuf.err.contains("USER_ABORT"))
        {
            actionCompleted(actStatusBuf);
            setState(STATE_READY);
        }
        else
        {
            disengageRetryCount++;
            if (disengageRetryCount < disengageRetryLimit)
            {
                int retryIntervalMs = MCU_LINK_HEART_BEAT_INTERVAL_MS * 6;
                LOG_INFO("DISENGAGE: STATE_FAILED: Retrying in %dms..\n", retryIntervalMs);
                QTimer::singleShot(retryIntervalMs, this, [=] {
                    // Wait until MCU is ready to run command. MCU is probably busy Stopping All actions
                    setState(STATE_STARTED);
                });
            }
            else
            {
                QString syringeIdxStr = "Unknown";

                // Find syringe index with activated alert
                for (int syrIdx = 0; syrIdx < SYRINGE_IDX_MAX; syrIdx++)
                {
                    SyringeIdx syringeIdx = (SyringeIdx)syrIdx;
                    if (env->ds.alertAction->isLastOccurredWithSyringeIdx("PlungerDisengagementFault", syringeIdx))
                    {
                        syringeIdxStr = ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdx);
                        LOG_INFO("DISENGAGE: STATE_FAILED: PlungerDisengagementFault alert found with SyringeIdx=%s\n", syringeIdxStr.CSTR());
                        break;
                    }
                }

                if (syringeIdxStr == "Unknown")
                {
                    LOG_ERROR("DISENGAGE: STATE_FAILED: Failed to find PlungerDisengagementFault alert with syringeIndex\n");
                }

                env->ds.alertAction->activate("PlungerDisengagementRetryFailed", syringeIdxStr);
                actionCompleted(actStatusBuf);
                setState(STATE_READY);
            }
        }
        break;
    case STATE_DONE:
        LOG_INFO("DISENGAGE: STATE_DONE\n");
        actionCompleted(actStatusBuf);
        setState(STATE_READY);
        break;
    default:
        LOG_ERROR("DISENGAGE: Unknown State(%d)\n", state);
        break;
    }
}

