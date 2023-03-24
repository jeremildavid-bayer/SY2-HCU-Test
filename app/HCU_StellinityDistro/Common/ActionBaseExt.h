#ifndef ACTION_BASE_EXT_H
#define ACTION_BASE_EXT_H

#include "Common/Common.h"
#include "Common/ActionBase.h"
#include "Common/ImrParser.h"
#include "Common/ActionManager.h"

class ActionBaseExt : public ActionBase
{
public:
    ActionBaseExt(QObject *parent, EnvGlobal *env_) :
        ActionBase(parent, env_)
    {
        listStateChangeRequests.clear();
    }

    ~ActionBaseExt()
    {
    }

    virtual bool isSetStateReady()
    {
        return true;
    }

    virtual int getState()
    {
        return state;
    }

    virtual QString getStateStr(int state_)
    {
        return QString().asprintf("%d", state_);
    }

    virtual void setState(int newState)
    {
        if (isSetStateReady())
        {
            setStateAsynch(newState);
        }
    }

    virtual void setStateSynch(int newState)
    {
        state = newState;
    }

    virtual void setStateAsynch(int newState)
    {
        if (listStateChangeRequests.length() > 0)
        {
            if (listStateChangeRequests.length() > 1)
            {
                LOG_ERROR("SET_STATE_ASYNCH: Too many State Change Requests(%d) not processed\n", (int)listStateChangeRequests.length());
            }

            //LOG_DEBUG("SET_STATE_ASYNCH: Processing last State Change Request(state=%d)\n", listStateChangeRequests.first());
            listStateChangeRequests.clear();
            processState();
        }

        listStateChangeRequests.append(newState);
        setStateSynch(newState);

        if (env->state == EnvGlobal::STATE_RUNNING)
        {
            QTimer::singleShot(PROCESS_STATE_TRANSITION_DELAY_MS, this, [=](){
                if (env->state == EnvGlobal::STATE_EXITING)
                {
                    return;
                }

                // See if the state needs to be processed
                if (listStateChangeRequests.length() > 0)
                {
                    //LOG_DEBUG("SET_STATE_ASYNCH: Processing last State Change Request(state=%d)\n", listStateChangeRequests.first());
                    listStateChangeRequests.clear();
                    processState();
                }
                else
                {
                    //LOG_DEBUG("SET_STATE_ASYNCH: Last State Change Request was processed by other new request\n");
                }
            });
        }
        else
        {
            listStateChangeRequests.clear();
            processState();
        }
    }

    virtual void processState() = 0;

    void handleSubAction(int progressState, int completeState, int errorState)
    {
        env->actionMgr->deleteActAll(guidSubAction);
        guidSubAction = Util::newGuid();

        env->actionMgr->onActionStarted(guidSubAction, QString().asprintf("%s: %d", envLocal->moduleName.CSTR(), progressState), [=](DataServiceActionStatus curStatus) {
            LOG_DEBUG("HANDLE_SUB_ACTION: ACTION_STATUS: %s\n", ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());
            setState(progressState);

            if (curStatus.state == DS_ACTION_STATE_STARTED)
            {
                // Action started
            }
            else
            {
                actStatusBuf.state = curStatus.state;
                actStatusBuf.err = curStatus.err;
                setState(errorState);
            }
        });

        env->actionMgr->onActionCompleted(guidSubAction, QString().asprintf("%s: %d", envLocal->moduleName.CSTR(), progressState), [=](DataServiceActionStatus curStatus) {
            LOG_DEBUG("HANDLE_SUB_ACTION: ACTION_STATUS: %s\n", ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());
            int curState = getState();
            if (curState != progressState)
            {
                // Unexpected proressState
                LOG_WARNING("HANDLE_SUB_ACTION: Action completed during unexpected state(%s), expected state=%s\n", getStateStr(curState).CSTR(), getStateStr(progressState).CSTR());
            }
            else if (curStatus.state == DS_ACTION_STATE_COMPLETED)
            {
                setState(completeState);
            }
            else
            {
                actStatusBuf.state = curStatus.state;
                actStatusBuf.err = curStatus.err;
                setState(errorState);
            }
       });
    }

protected:
    DataServiceActionStatus actStatusBuf;
    int state;
    QString guidSubAction;

private:
    QList<int> listStateChangeRequests;
};

#endif // ACTION_BASE_EXT_H
