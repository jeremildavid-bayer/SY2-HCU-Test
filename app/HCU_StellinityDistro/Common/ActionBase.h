#ifndef ACTION_BASE_H
#define ACTION_BASE_H

#include "Common/Common.h"
#include "Common/ActionManager.h"
#include "Common/ImrParser.h"

class ActionBase : public QObject
{
    Q_OBJECT
public:
    ActionBase(QObject *parent, EnvGlobal *env_) :
        QObject(parent),
        env(env_)
    {
    }

    ~ActionBase()
    {
    }

    void logActionStatus(const DataServiceActionStatus &status, bool verbose)
    {
        if (status.err == "")
        {
            if (verbose)
            {
                LOG_INFO("ACTION_STATUS: %s\n", ImrParser::ToImr_DataServiceActionStatusStr(status).CSTR());
            }
        }
        else if ( (status.state == DS_ACTION_STATE_ALLSTOP_BTN_ABORT) ||
                  (status.state == DS_ACTION_STATE_USER_ABORT) )
        {
            LOG_WARNING("ACTION_STATUS: %s\n", ImrParser::ToImr_DataServiceActionStatusStr(status).CSTR());
        }
        else
        {
            LOG_ERROR("ACTION_STATUS: %s\n", ImrParser::ToImr_DataServiceActionStatusStr(status).CSTR());
        }
    }

    DataServiceActionStatus actionInit(QString guid, QString request, QString arg = "", QString arg2 = "", bool verbose = false)
    {
        DataServiceActionStatus status(guid, request, arg, arg2);
        logActionStatus(status, verbose);
        return status;
    }

    void actionStarted(DataServiceActionStatus &status, const DataServiceActionStatus *originalStatus = NULL, bool verbose = true)
    {
        if (originalStatus != NULL)
        {
            status.guid = originalStatus->guid;
            status.request = originalStatus->request;
            status.arg = originalStatus->arg;
            status.arg2 = originalStatus->arg2;
        }

        if (status.state == DS_ACTION_STATE_INIT)
        {
            status.state = DS_ACTION_STATE_STARTED;
        }
        logActionStatus(status, verbose);

        if (status.guid != "")
        {
            emit env->actionMgr->signalActionStarted(status);
        }
    }

    void actionCompleted(DataServiceActionStatus &status, const DataServiceActionStatus *originalStatus = NULL, bool verbose = true)
    {
        if (originalStatus != NULL)
        {
            status.guid = originalStatus->guid;
            status.request = originalStatus->request;
            status.arg = originalStatus->arg;
            status.arg2 = originalStatus->arg2;
        }

        if (status.state == DS_ACTION_STATE_STARTED)
        {
            status.state = DS_ACTION_STATE_COMPLETED;
        }

        logActionStatus(status, verbose);

        if (status.guid != "")
        {
            emit env->actionMgr->signalActionCompleted(status);
        }
    }

protected:
    EnvGlobal *env;
    EnvLocal *envLocal;
};

#endif // ACTION_BASE_H
