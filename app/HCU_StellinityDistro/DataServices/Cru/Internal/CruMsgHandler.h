#ifndef CRU_MSG_HANDLER_H
#define CRU_MSG_HANDLER_H

#include "Common/Common.h"

class CruMsgHandler : public QObject
{
    Q_OBJECT
public:
    explicit CruMsgHandler(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~CruMsgHandler();

    QString handleMsg(QString url, QString method, QString replyMsg);

private:
    EnvGlobal *env;
    EnvLocal *envLocal;

    QString handleProfile(QString jsonStr);
    QString handleFluids(QString jsonStr);
    QString handleInjectionPlans(QString jsonStr);
    QString handleInjectionPlan(QString jsonStr);
    QString handleDigest(QString jsonStr);
    QString handleGetWorklistEntries(QString jsonStr);
    QString handleGetConfigs(QString jsonStr);
    QString handlePostUpdateInjectionParameter(QString jsonStr);
    QString handlePostSelectWorklistEntry(QString jsonStr);
};

#endif // CRU_MSG_HANDLER_H
