#include "Apps/AppManager.h"
#include "DeviceStopcock.h"
#include "DataServices/Workflow/DS_WorkflowData.h"
#include "DataServices/Mcu/DS_McuAction.h"
#include "DataServices/Mcu/DS_McuData.h"
#include "DataServices/Alert/DS_AlertAction.h"
#include "DataServices/Alert/DS_AlertData.h"
#include "Common/Translator.h"
#include "Common/ImrParser.h"

DeviceStopcock::DeviceStopcock(QObject *parent, EnvGlobal *env_, EnvLocal *envLocal_, SyringeIdx location_) :
    ActionBaseExt(parent, env_),
    location(location_)
{
    envLocal = envLocal_;

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });

    actStopcockTargetPos = DS_McuDef::STOPCOCK_POS_UNKNOWN;
    state = STATE_SET_STOPCOCKS_READY;
}

DeviceStopcock::~DeviceStopcock()
{
}

void DeviceStopcock::slotAppInitialised()
{
    connect(env->ds.workflowData, &DS_WorkflowData::signalDataChanged_WorkflowState, this, [=](DS_WorkflowDef::WorkflowState workflowState, DS_WorkflowDef::WorkflowState workflowStatePrev) {
        if ( (workflowState != DS_WorkflowDef::STATE_INACTIVE) &&
             (workflowStatePrev == DS_WorkflowDef::STATE_INACTIVE) )
        {
        }
        else if ( (workflowState == DS_WorkflowDef::STATE_INACTIVE) &&
                  (workflowStatePrev != DS_WorkflowDef::STATE_INACTIVE) )
        {
        }
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_StopcockPosAll, this, [=](const DS_McuDef::StopcockPosAll &stopcockPosAll) {
        QVariantList activeAlerts = env->ds.alertData->getActiveAlerts();
        for (int alertIdx = 0; alertIdx < activeAlerts.length(); alertIdx++)
        {
            QVariantMap alert = activeAlerts[alertIdx].toMap();
            if (alert["CodeName"].toString() == "StopcockUnintendedMotionDetected")
            {
                QString dataStr = alert["Data"].toString();
                SyringeIdx syringeIdx = ImrParser::ToCpp_FluidSourceSyringeIdx(dataStr);
                if (stopcockPosAll[syringeIdx] == DS_McuDef::STOPCOCK_POS_FILL)
                {
                    LOG_INFO("SC Position[%s] Recovered from StopcockUnintendedMotionDetected. Deactivating alert..\n", dataStr.CSTR());
                    env->ds.alertAction->deactivate("StopcockUnintendedMotionDetected", dataStr);
                }
            }
        }
    });
}

void DeviceStopcock::processState()
{
    switch (state)
    {
    case STATE_SET_STOPCOCKS_READY:
        LOG_INFO("[%d]: STATE_SET_STOPCOCKS_READY\n", location);
        if (actStopcockStatusListBuf.length() > 0)
        {
            // There were SC requests while completing last requests.
            actStopcockStatusList = actStopcockStatusListBuf;
            actStopcockStatusListBuf.clear();
            setState(STATE_SET_STOPCOCKS_STARTED);
        }
        break;
    case STATE_SET_STOPCOCKS_STARTED:
        LOG_INFO("[%d]: STATE_SET_STOPCOCKS_STARTED\n", location);
        for (int actIdx = 0; actIdx < actStopcockStatusList.length(); actIdx++)
        {
            //LOG_DEBUG("[%d]: actStopcockStatus(guid=%s) signalActionStarted emitted\n", location, actStopcockStatusList.first().guid.CSTR());
            actStopcockStatusList[actIdx].state = DS_ACTION_STATE_STARTED;
            actionStarted(actStopcockStatusList[actIdx]);
        }
        setState(STATE_SET_STOPCOCKS_PROGRESS);
        break;
    case STATE_SET_STOPCOCKS_PROGRESS:
        LOG_INFO("[%d]: STATE_SET_STOPCOCKS_PROGRESS\n", location);
        {
            QString guid = Util::newGuid();
            env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
                if (state != STATE_SET_STOPCOCKS_PROGRESS)
                {
                    // Action Status received at wrong time
                    LOG_WARNING("[%d]: Trial%d: Unexpected stopcock action started callback received\n", location, actStopcockRetryCount);
                    return;
                }

                // Update action started status
                for (int actIdx = 0; actIdx < actStopcockStatusList.length(); actIdx++)
                {
                    QString actStopcockGuid = actStopcockStatusList[actIdx].guid;
                    actStopcockStatusList[actIdx] = curStatus;
                    actStopcockStatusList[actIdx].guid = actStopcockGuid;
                }

                if (curStatus.state == DS_ACTION_STATE_STARTED)
                {
                    env->actionMgr->onActionCompleted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
                        if (state != STATE_SET_STOPCOCKS_PROGRESS)
                        {
                            // Action Status received at wrong time
                            LOG_WARNING("[%d]: Trial%d: Unexpected stopcock action completed callback received\n", location, actStopcockRetryCount);
                            return;
                        }

                        // Update action completed status
                        for (int actIdx = 0; actIdx < actStopcockStatusList.length(); actIdx++)
                        {
                            QString actStopcockGuid = actStopcockStatusList[actIdx].guid;
                            actStopcockStatusList[actIdx] = curStatus;
                            actStopcockStatusList[actIdx].guid = actStopcockGuid;
                        }

                        if (curStatus.state == DS_ACTION_STATE_COMPLETED)
                        {
                            setState(STATE_SET_STOPCOCKS_DONE);
                        }
                        else
                        {
                            if (curStatus.state == DS_ACTION_STATE_TIMEOUT)
                            {
                                curStatus.err = "TIMEOUT";
                            }
                            LOG_WARNING("[%d]: Trial%d: ACTION_STATUS: %s\n", location, actStopcockRetryCount, ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());
                            setState(STATE_SET_STOPCOCKS_FAILED);
                        }
                    });
                }
                else
                {
                    LOG_ERROR("[%d]: Trial%d: ACTION_STATUS: %s\n", location, actStopcockRetryCount, ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());
                    setState(STATE_SET_STOPCOCKS_FAILED);
                }
            });
            env->ds.mcuAction->actStopcock(location, actStopcockTargetPos, guid);
        }
        break;
    case STATE_SET_STOPCOCKS_FAILED:
        LOG_INFO("[%d]: STATE_SET_STOPCOCKS_FAILED\n", location);
        actStopcockRetryCount++;
        if (actStopcockRetryCount < actStopcockRetryLimit)
        {
            setState(STATE_SET_STOPCOCKS_STARTED);
        }
        else
        {
            env->ds.alertAction->activate("StopcockRetryFailed", ImrParser::ToImr_FluidSourceSyringeIdx(location));
            while (actStopcockStatusList.length() > 0)
            {
                //LOG_ERROR("[%d]: STATE_SET_STOPCOCKS_FAILED: actStopcockStatus(guid=%s) signalActionCompleted Failed emitted. Err=%s\n", location, actStopcockStatusList.first().guid.CSTR(), actStopcockStatusList.first().err.CSTR());
                actionCompleted(actStopcockStatusList.first());
                actStopcockStatusList.removeFirst();
            }
            setState(STATE_SET_STOPCOCKS_READY);
        }
        break;
    case STATE_SET_STOPCOCKS_DONE:
        LOG_INFO("[%d]: STATE_SET_STOPCOCKS_DONE\n", location);

        while (actStopcockStatusList.length() > 0)
        {
            DataServiceActionStatus actStatus = actStopcockStatusList.first();
            actStopcockStatusList.pop_front();
            actStatus.state = DS_ACTION_STATE_COMPLETED;

            //LOG_DEBUG("[%d]: actStopcockStatus(guid=%s) signalActionCompleted emitted\n", location, actStatus.guid.CSTR());
            actionCompleted(actStatus);
        }
        setState(STATE_SET_STOPCOCKS_READY);
        break;
    }
}

DataServiceActionStatus DeviceStopcock::actStopcock(DS_McuDef::StopcockPos pos, int retryLimit, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "Stopcock", QString().asprintf("%s;%s;%d", ImrParser::ToImr_FluidSourceSyringeIdx(location).CSTR(), ImrParser::ToImr_StopcockPos(pos).CSTR(), retryLimit));

    if (state == STATE_SET_STOPCOCKS_DONE)
    {
        LOG_INFO("[%d]: ACT_STOPCOCK: Stopcock is about to complete current action. Will start current request soon\n", location);
        status.state = DS_ACTION_STATE_START_WAITING;
        //status.err = "Stopcock action is in progress";

        if ( (actStopcockStatusListBuf.length() > 0) &&
             (pos != actStopcockTargetPos) )
        {
            status.state = DS_ACTION_STATE_BAD_REQUEST;
            status.err = "Stopcock position conflict";
            actionStarted(status);
            return status;
        }

        actStopcockStatusListBuf.append(status);
        actStopcockTargetPos = pos;
        actStopcockRetryLimit = retryLimit;
        actStopcockRetryCount = 0;
        return status;

    }
    else if ( (actStopcockStatusList.length() > 0) &&
              (pos != actStopcockTargetPos) )
    {
        status.state = DS_ACTION_STATE_BAD_REQUEST;
        status.err = "Stopcock position conflict";
        actionStarted(status);
        return status;
    }

    if ( (actStopcockStatusList.length() > 0) &&
         (pos == actStopcockTargetPos) )
    {
        LOG_INFO("[%d]: ACT_STOPCOCK: Same action is already in progress\n", location);
    }

    // Save current action status to send it later when done
    actStopcockStatusList.append(status);
    actStopcockTargetPos = pos;
    actStopcockRetryLimit = retryLimit;
    actStopcockRetryCount = 0;

    setState(STATE_SET_STOPCOCKS_STARTED);

    return status;
}
