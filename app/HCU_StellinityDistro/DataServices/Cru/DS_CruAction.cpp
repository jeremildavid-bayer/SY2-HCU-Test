#include <QNetworkRequest>
#include <QUrlQuery>
#include "Apps/AppManager.h"
#include "Common/ImrParser.h"
#include "DS_CruAction.h"
#include "DataServices/Cru/DS_CruData.h"
#include "DataServices/CfgLocal/DS_CfgLocal.h"
#include "DataServices/Exam/DS_ExamData.h"
#include "DataServices/ImrServer/Internal/ImrServer0/TestMsg.h"

DS_CruAction::DS_CruAction(QObject *parent, EnvGlobal *env_) :
    ActionBase(parent, env_)
{
    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });

    envLocal = new EnvLocal("DS_Cru-Action", "CRU_ACTION", LOG_LRG_SIZE_BYTES);
    msgHandler = new CruMsgHandler(this, env);
    cruLink = new CruLink(NULL, env);

}

DS_CruAction::~DS_CruAction()
{
    delete cruLink;
    delete envLocal;
    delete msgHandler;
}

void DS_CruAction::slotAppInitialised()
{
    /*
     * Get the CRU configuration through signal change so it is not request during runtime
     * This is try to avoid all the config QMap interactin during normal operation.
     *
     * Try to eliminate the crashing until the root cause is found
     */
    //QString cruType = env->ds.cfgLocal->getSettings_SruLink_ConnectionType().value.toString();
    connect(env->ds.cfgLocal, &DS_CfgLocal::signalConfigChanged_Settings_SruLink_ConnectionType, this, [=](const Config::Item &val, const Config::Item &prevVal) {
         cruType = val.value.toString();
         LOG_INFO("cruType: %s, wireless: %s wired: %s\n", cruType.CSTR(), cruIpWireless.CSTR(), cruIpWired.CSTR());
    });

    //env->ds.cfgLocal->getHidden_CruIpWireless().value.toString();
    connect(env->ds.cfgLocal, &DS_CfgLocal::signalConfigChanged_Hidden_CruIpWired, this, [=](const Config::Item &val, const Config::Item &prevVal) {
         cruIpWired = val.value.toString();
    });

    //env->ds.cfgLocal->getHidden_CruIpWired().value.toString();
    connect(env->ds.cfgLocal, &DS_CfgLocal::signalConfigChanged_Hidden_CruIpWireless, this, [=](const Config::Item &val, const Config::Item &prevVal) {
         cruIpWireless = val.value.toString();
    });

    //int cruPort = env->ds.cfgLocal->getHidden_CruPort().value.toInt();
    connect(env->ds.cfgLocal, &DS_CfgLocal::signalConfigChanged_Hidden_CruPort, this, [=](const Config::Item &val, const Config::Item &prevVal) {
         cruPort = val.value.toInt();
    });

    connect(this, &DS_CruAction::signalInvalidateAction, [=](QString url, QString method) {
        LOG_INFO("signalInvalidateAction(): url=%s, method=%s\n", url.CSTR(), method.CSTR());
        actInvalidate(url, method);
    });

    connect(this, &DS_CruAction::signalRxMsg, [=](QString guid, DataServiceActionStatus actStatus) {
        mutexMsgCommMap.lock();

        // Find ActionStatus from List
        if (!actMsgCommMap.contains(guid))
        {
            mutexMsgCommMap.unlock();
            LOG_WARNING("CruAction::signalRxMsg: Unexpected Reply Received. guid=%s, actStatus=%s\n", guid.CSTR(), ImrParser::ToImr_DataServiceActionStatusStr(actStatus).CSTR());
            return;
        }

        // ActionStatus found
        DataServiceActionStatus curStatus = actStatus;
        delete actMsgCommMap[guid];
        actMsgCommMap.remove(guid);

        mutexMsgCommMap.unlock();

        if (curStatus.state == DS_ACTION_STATE_COMPLETED)
        {
            DataServiceActionStatus newStatus = actHandleReply(curStatus.request, curStatus.arg, curStatus.reply);
            curStatus.state = newStatus.state;
            curStatus.err = newStatus.err;
        }
        else
        {
            curStatus.state = DS_ACTION_STATE_INTERNAL_ERR;
        }


        if (curStatus.request.contains(IMR_CRU_URL_DIGEST))
        {
            // Needs to hide the patient's name
            QJsonDocument document = QJsonDocument::fromJson(curStatus.reply.toUtf8());
            QVariantMap digestMap = document.toVariant().toMap();
            if (digestMap.contains("PatientName"))
            {
                QString patientXxx = digestMap["PatientName"].toString();
                patientXxx.replace(QRegularExpression("."), "*");
                digestMap.insert("PatientName", patientXxx);
            }
            curStatus.reply = Util::qVarientToJsonData(digestMap);
        }

        actionCompleted(curStatus);
    });

    connect(env->ds.cruData, &DS_CruData::signalDataChanged_CruLinkStatus, this, [=](const DS_CruDef::CruLinkStatus &curLinkStatus, const DS_CruDef::CruLinkStatus &prevLinkStatus) {
        if ( (prevLinkStatus.state == DS_CruDef::CRU_LINK_STATE_ACTIVE) &&
             (curLinkStatus.state != DS_CruDef::CRU_LINK_STATE_ACTIVE) )
        {
            mutexMsgCommMap.lock();

            LOG_WARNING("CRU Link is down. Processing all action status as CRU COMM DOWN..\n");

            QMap<QString, CruLinkMsgComm*>::const_iterator i = actMsgCommMap.cbegin();
            while (i != actMsgCommMap.cend())
            {
                CruLinkMsgComm *msgComm = i.value();
                DataServiceActionStatus curStatus = msgComm->getActStatus();
                curStatus.state = DS_ACTION_STATE_TIMEOUT;
                curStatus.err = "CRU COMM DOWN";
                actionStarted(curStatus, NULL);
                actionCompleted(curStatus, NULL);
                delete msgComm;
                i++;
            }
            actMsgCommMap.clear();
            mutexMsgCommMap.unlock();
        }
    });
}

void DS_CruAction::setCruLink(CruLink *cruLink_)
{
    cruLink = cruLink_;
}


DataServiceActionStatus DS_CruAction::actSendRequest(QString actGuid, QString url, QString method, QString body, int timeoutMs)
{
    DataServiceActionStatus status = actionInit(actGuid, url, method, body);

    status.state = DS_ACTION_STATE_STARTED;

    // Check TX conditions
    DS_CruDef::CruLinkStatus linkStatus = env->ds.cruData->getCruLinkStatus();

    if (url == "")
    {
        status.state = DS_ACTION_STATE_BAD_REQUEST;
        status.err = "Bad Request";
        actionStarted(status);
        return status;
    }
    else if ( (linkStatus.state != DS_CruDef::CRU_LINK_STATE_ACTIVE) &&
              (!url.contains(IMR_CRU_URL_PROFILE)) &&
              (!url.contains(IMR_CRU_URL_DIGEST)) )
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = QString().asprintf("Bad Link State (%s)", ImrParser::ToImr_CruLinkState(linkStatus.state).CSTR());
        actionStarted(status);
        return status;
    }

    QString cruIp;
    if (cruType == _L("WirelessEthernet"))
    {
        cruIp = this->cruIpWireless;
    }
    else if (cruType == _L("WiredEthernet"))
    {
        cruIp = cruIpWired;
    }
    else
    {
        status.state = DS_ACTION_STATE_INTERNAL_ERR;
        status.err = QString().asprintf("Bad Link Type (%s)", cruType.CSTR());
        actionStarted(status);
        return status;
    }

    status.request = QString().asprintf("http://%s:%d/cru/%s", cruIp.CSTR(), cruPort, url.CSTR());

    QString msgGuid = Util::newGuid();

    CruLinkMsgComm *cruLinkMsgComm = new CruLinkMsgComm(NULL, env, envLocal, msgGuid, status, timeoutMs);
    mutexMsgCommMap.lock();
    actMsgCommMap.insert(msgGuid, cruLinkMsgComm);
    mutexMsgCommMap.unlock();
    cruLinkMsgComm->start();

    actionStarted(status);
    return status;
}

DataServiceActionStatus DS_CruAction::actGetConfigs(QString actGuid)
{
    return actSendRequest(actGuid, IMR_CRU_URL_CONFIGS, "get");
}

DataServiceActionStatus DS_CruAction::actPutConfigs(QVariantMap cfgItems, QString actGuid)
{
    QString json = Util::qVarientToJsonData(cfgItems);
    return actSendRequest(actGuid, IMR_CRU_URL_CONFIGS, "put", json);
}

DataServiceActionStatus DS_CruAction::actGetProfile(QString actGuid)
{
    return actSendRequest(actGuid, IMR_CRU_URL_PROFILE, "get");
}

DataServiceActionStatus DS_CruAction::actGetFluids(QString actGuid)
{
    return actSendRequest(actGuid, IMR_CRU_URL_FLUIDS, "get");
}

DataServiceActionStatus DS_CruAction::actPutFluids(DS_DeviceDef::FluidOptions fluidOptions, QString actGuid)
{
    QVariantMap map = ImrParser::ToImr_FluidOptions(fluidOptions);
    QString json = Util::qVarientToJsonData(map);
    return actSendRequest(actGuid, IMR_CRU_URL_FLUIDS, "put", json);
}

DataServiceActionStatus DS_CruAction::actDigest(QString actGuid)
{
    return actSendRequest(actGuid, IMR_CRU_URL_DIGEST, "get");
}

DataServiceActionStatus DS_CruAction::actGetInjectPlans(QString actGuid)
{
    return actSendRequest(actGuid, IMR_CRU_URL_INJECT_PLANS, "get");
}

DataServiceActionStatus DS_CruAction::actGetInjectPlan(QString planTemplateGuid, QString actGuid)
{
    QString url = IMR_CRU_URL_INJECT_PLAN + QString().asprintf("?guid=%s", planTemplateGuid.CSTR());
    return actSendRequest(actGuid, url, "get");
}

DataServiceActionStatus DS_CruAction::actPostBarcode(QString read, QString actGuid)
{
    // Add parameter as a query so that it can be properly encoded
    QUrlQuery query;
    query.addQueryItem("read", QUrl::toPercentEncoding(read));

    QUrl url(IMR_CRU_URL_BARCODE);
    url.setQuery(query);

    return actSendRequest(actGuid, url.toEncoded(), "post");
}

DataServiceActionStatus DS_CruAction::actExamStart(QString examGuid, QString actGuid)
{
    QString url = IMR_CRU_URL_START_EXAM + QString().asprintf("?guid=%s", examGuid.CSTR());

    //No need to use QUrlQuery as the guid cannot have special chars which need encoded

    return actSendRequest(actGuid, url, "post", "", CRU_START_EXAM_REPLY_TIMEOUT_MS); //The CRU can sometimes take quite-long to start an exam. If we get out of sync with the CRU StartExam, the SRU must be restarted. Give a little extra time here to avoid this...
}

DataServiceActionStatus DS_CruAction::actExamEnd(QString actGuid)
{    
    //No need to use QUrlQuery as the guid cannot have special chars which need encoded

    return actSendRequest(actGuid, IMR_CRU_URL_END_EXAM, "post");
}

DataServiceActionStatus DS_CruAction::actApplyLimits(QString actGuid)
{
    return actSendRequest(actGuid, IMR_CRU_URL_APPLY_LIMITS, "post");
}

DataServiceActionStatus DS_CruAction::actInvalidate(QString url, QString method, QString actGuid)
{
    int invalidateTimeout = CRU_REPLY_TIMEOUT_MS * 2;
    // providing loger timeout for invalidate because we don't have any logic to re-try when this times out.
    // TODO: make a re-try logic
    return actSendRequest(actGuid, url, method, "", invalidateTimeout);
}


DataServiceActionStatus DS_CruAction::actGetTestMessage(QString actGuid)
{
    return actSendRequest(actGuid, IMR_CRU_URL_TEST_MESSAGE, "get");
}

DataServiceActionStatus DS_CruAction::actPutTestMessage(int processingTimeMs, QString paramStr, QVariantList paramList, QVariantMap paramMap, QString actGuid)
{
    TestMsg testMsg;
    testMsg.params.insert("ProcessingMillis", processingTimeMs);
    testMsg.params.insert("TestString", paramStr);
    testMsg.params.insert("TestList", paramList);
    testMsg.params.insert("TestDictionary", paramMap);
    QString json = testMsg.serialize();
    return actSendRequest(actGuid, IMR_CRU_URL_TEST_MESSAGE, "put", json);
}

DataServiceActionStatus DS_CruAction::actPostUpdateInjectionParameter(const DS_ExamDef::InjectionPersonalizationInput &personalizationInput, QString actGuid)
{
    QVariantMap map = ImrParser::ToImr_InjectionPersonalizationInput(personalizationInput);

    QString name = map["Name"].toString();
    QString value = map["Value"].toString();

    // Add parameters as a query so that they can be properly encoded
    QUrlQuery query;
    query.addQueryItem("name", QUrl::toPercentEncoding(name));
    query.addQueryItem("value", QUrl::toPercentEncoding(value));

    QUrl url(IMR_CRU_URL_UPDATE_INJECTION_PARAMTER);
    url.setQuery(query);


    return actSendRequest(actGuid, url.toString(), "post");
}

DataServiceActionStatus DS_CruAction::actPostUpdateExamField(const DS_ExamDef::ExamField &examField, QString actGuid)
{
    QVariantMap examFieldMap = ImrParser::ToImr_ExamField(examField);

    DS_ExamDef::ExamAdvanceInfo examAdvanceInfo = env->ds.examData->getExamAdvanceInfo();
    QString name = examFieldMap["Name"].toString();
    QString value = examFieldMap["Value"].toString();


    // Add parameters as a query so that they can be properly encoded
    QUrlQuery query;
    query.addQueryItem("examGuid", examAdvanceInfo.guid.CSTR());
    query.addQueryItem("name", QUrl::toPercentEncoding(name));
    query.addQueryItem("value", QUrl::toPercentEncoding(value));

    QUrl url(IMR_CRU_URL_UPDATE_EXAM_FIELD);
    url.setQuery(query);

    return actSendRequest(actGuid, url.toEncoded(), "post");
}

DataServiceActionStatus DS_CruAction::actPostUpdateExamFieldParameter(const DS_ExamDef::ExamFieldParameter &examFieldParameter, QString actGuid)
{
    QVariantMap map = ImrParser::ToImr_ExamFieldParameter(examFieldParameter);

    DS_ExamDef::ExamAdvanceInfo examAdvanceInfo = env->ds.examData->getExamAdvanceInfo();
    QString name = map["Name"].toString();
    QString value = map["Value"].toString();

    // Add parameters as a query so that they can be properly encoded
    QUrlQuery query;
    query.addQueryItem("examGuid", examAdvanceInfo.guid.CSTR());
    query.addQueryItem("name", QUrl::toPercentEncoding(name));
    query.addQueryItem("value", QUrl::toPercentEncoding(value));

    QUrl url(IMR_CRU_URL_UPDATE_EXAM_FIELD_PARAMETER);
    url.setQuery(query);
    QByteArray  url_array = url.toEncoded(QUrl::FullyEncoded);

    return actSendRequest(actGuid, url.toEncoded(), "post");
}

DataServiceActionStatus DS_CruAction::actPostUpdateLinkedAccession(const DS_MwlDef::WorklistEntry &entry, bool isLinked, QString actGuid)
{
    DS_ExamDef::ExamAdvanceInfo examAdvanceInfo = env->ds.examData->getExamAdvanceInfo();

    // Add parameters as a query so that they can be properly encoded
    QUrlQuery query;
    query.addQueryItem("examGuid", examAdvanceInfo.guid.CSTR());
    query.addQueryItem("studyInstanceUid", entry.studyInstanceUid.CSTR());
    query.addQueryItem("linked", isLinked ? "true" : "false");

    QUrl url(IMR_CRU_URL_UPDATE_LINKED_ACCESSION);
    url.setQuery(query);

    return actSendRequest(actGuid, url.toEncoded(), "post");
}

DataServiceActionStatus DS_CruAction::actHandleReply(QString url, QString method, QString replyMsg)
{
    DataServiceActionStatus status;
    status.reply = replyMsg;
    status.err = msgHandler->handleMsg(url, method, replyMsg);

    if (status.err == "")
    {
        status.state = DS_ACTION_STATE_COMPLETED;
    }
    else
    {
        status.state = DS_ACTION_STATE_INTERNAL_ERR;
    }

    return status;
}

bool DS_CruAction::isRequestFromCru(quint32 ipv4Addr)
{
    quint8 *ipv4 = (quint8*)&ipv4Addr;
    QString peerAddressStr = QString().asprintf("%d.%d.%d.%d", ipv4[3], ipv4[2], ipv4[1], ipv4[0]);

    if ( (peerAddressStr == cruIpWired) || (peerAddressStr == cruIpWireless) )
    {
        return true;
    }
    return false;
}
