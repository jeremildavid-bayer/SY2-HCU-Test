#ifndef DS_CRU_ACTION_H
#define DS_CRU_ACTION_H

#include "Common/Common.h"
#include "Common/Util.h"
#include "Common/ActionBase.h"
#include "DS_CruDef.h"
#include "Internal/CruLink.h"
#include "Internal/CruMsgHandler.h"
#include "Internal/CruLinkMsgComm.h"

class DS_CruAction : public ActionBase
{
    Q_OBJECT
public:
    explicit DS_CruAction(QObject *parent = 0, EnvGlobal *env = NULL);
    ~DS_CruAction();
    void setCruLink(CruLink *cruLink_);
    bool isRequestFromCru(quint32 ipv4Addr);

    DataServiceActionStatus actSendRequest(QString guid, QString url, QString methodStr, QString body = "", int timeoutMs = CRU_REPLY_TIMEOUT_MS);
    DataServiceActionStatus actHandleReply(QString url, QString method, QString replyMsg);
    DataServiceActionStatus actGetConfigs(QString actGuid = "");
    DataServiceActionStatus actPutConfigs(QVariantMap cfgItems, QString actGuid = "");
    DataServiceActionStatus actGetProfile(QString actGuid = "");
    DataServiceActionStatus actGetFluids(QString actGuid = "");
    DataServiceActionStatus actPutFluids(DS_DeviceDef::FluidOptions fluidOptions, QString actGuid = "");
    DataServiceActionStatus actDigest(QString actGuid = "");
    DataServiceActionStatus actGetInjectPlans(QString actGuid = "");
    DataServiceActionStatus actGetInjectPlan(QString planTemplateGuid, QString actGuid = "");
    DataServiceActionStatus actPostBarcode(QString read, QString actGuid = "");
    DataServiceActionStatus actExamStart(QString examGuid, QString actGuid = "");
    DataServiceActionStatus actExamEnd(QString actGuid = "");
    DataServiceActionStatus actApplyLimits(QString actGuid = "");
    DataServiceActionStatus actInvalidate(QString url, QString method = "get", QString actGuid = "");
    DataServiceActionStatus actGetTestMessage(QString actGuid = "");
    DataServiceActionStatus actPutTestMessage(int processingTimeMs = 0, QString paramStr = "", QVariantList paramList = QVariantList(), QVariantMap paramMap = QVariantMap(), QString actGuid = "");
    DataServiceActionStatus actPostUpdateInjectionParameter(const DS_ExamDef::InjectionPersonalizationInput &personalizationInput, QString actGuid = "");
    DataServiceActionStatus actPostUpdateExamField(const DS_ExamDef::ExamField &examField, QString actGuid = "");
    DataServiceActionStatus actPostUpdateExamFieldParameter(const DS_ExamDef::ExamFieldParameter &examFieldParameter, QString actGuid = "");
    DataServiceActionStatus actPostUpdateLinkedAccession(const DS_MwlDef::WorklistEntry &entry, bool isLinked, QString actGuid = "");

private:
    CruMsgHandler *msgHandler;
    CruLink *cruLink;
    QMap<QString, CruLinkMsgComm*> actMsgCommMap;
    QMutex mutexMsgCommMap;

    //Getting from the local configuration to avoid access during runtime
    QString cruType;
    QString cruIpWireless;
    QString cruIpWired;
    int cruPort;

private slots:
    void slotAppInitialised();

signals:
    void signalRxMsg(QString guid, DataServiceActionStatus actStatus);
    void signalInvalidateAction(QString url, QString method);
};

#endif // DS_CRU_ACTION_H
