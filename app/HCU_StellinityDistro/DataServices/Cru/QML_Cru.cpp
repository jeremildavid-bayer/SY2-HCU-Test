#include "QML_Cru.h"
#include "Common/Util.h"
#include "DataServices/Cru/DS_CruData.h"
#include "DataServices/Cru/DS_CruAction.h"
#include "Common/ImrParser.h"

QML_Cru::QML_Cru(QObject *parent, EnvGlobal *env_) :
    QObject(parent),
    env(env_)
{
    envLocal = new EnvLocal("QML_Cru", "QML_CRU");
    qmlSrc = env->qml.object->findChild<QObject*>("dsCru");
    env->qml.engine->rootContext()->setContextProperty("dsCruCpp", this);

    if (qmlSrc == NULL)
    {
        LOG_ERROR("Failed to assign qmlSrc.\n");
    }

    // Register Data Callbacks
    connect(env->ds.cruData, &DS_CruData::signalDataChanged_CruLinkStatus, this, [=](const DS_CruDef::CruLinkStatus &cruLinkStatus) {
        qmlSrc->setProperty("cruLinkStatus", ImrParser::ToImr_CruLinkStatus(cruLinkStatus));
    });

    connect(env->ds.cruData, &DS_CruData::signalDataChanged_WifiSsid, this, [=](QString wifiSsid) {
        qmlSrc->setProperty("wifiSsid", wifiSsid);
    });

    connect(env->ds.cruData, &DS_CruData::signalDataChanged_WifiPassword, this, [=](QString wifiPassword) {
        qmlSrc->setProperty("wifiPassword", wifiPassword);
    });

    connect(env->ds.cruData, &DS_CruData::signalDataChanged_LicenseEnabledWorklistSelection, this, [=](bool licenseEnabled) {
        qmlSrc->setProperty("licenseEnabledWorklistSelection", licenseEnabled);
    });

    connect(env->ds.cruData, &DS_CruData::signalDataChanged_LicenseEnabledPatientStudyContext, this, [=](bool licenseEnabled) {
        qmlSrc->setProperty("licenseEnabledPatientStudyContext", licenseEnabled);
    });

}

QML_Cru::~QML_Cru()
{
    delete envLocal;
}

void QML_Cru::slotGetTestMessage()
{
    QString actGuid = Util::newGuid();
    env->actionMgr->onActionStarted(actGuid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus status) {
        if (status.state == DS_ACTION_STATE_STARTED)
        {
            env->actionMgr->onActionCompleted(actGuid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus status) {
                if (status.state == DS_ACTION_STATE_COMPLETED)
                {
                    qmlSrc->setProperty("testMessageState", "OK");
                }
                else
                {
                    qmlSrc->setProperty("testMessageState", QString().asprintf("ERROR: RX Failed (%s)", status.err.CSTR()));
                }
            });
        }
        else
        {
            qmlSrc->setProperty("testMessageState", QString().asprintf("ERROR: TX Failed (%s)", status.err.CSTR()));
        }
    });
    env->ds.cruAction->actGetTestMessage(actGuid);
}

void QML_Cru::slotPutTestMessage(int messageByteLen, int processingTimeMs)
{
    QString message;
    message.resize(messageByteLen);

    QString actGuid = Util::newGuid();
    env->actionMgr->onActionStarted(actGuid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus status) {
        if (status.state == DS_ACTION_STATE_STARTED)
        {
            env->actionMgr->onActionCompleted(actGuid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus status) {
                if (status.state == DS_ACTION_STATE_COMPLETED)
                {
                    qmlSrc->setProperty("testMessageState", "OK");
                }
                else
                {
                    qmlSrc->setProperty("testMessageState", QString().asprintf("ERROR: RX Failed (%s)", status.err.CSTR()));
                }
            });
        }
        else
        {
            qmlSrc->setProperty("testMessageState", QString().asprintf("ERROR: TX Failed (%s)", status.err.CSTR()));
        }
    });

    env->ds.cruAction->actPutTestMessage(processingTimeMs, message, QVariantList(), QVariantMap(), actGuid);
}

void QML_Cru::slotPostUpdateInjectionParameter(QVariantMap param)
{
    DS_ExamDef::InjectionPersonalizationInput personalizationInput = ImrParser::ToCpp_InjectionPersonalizationInput(param);
    env->ds.cruAction->actPostUpdateInjectionParameter(personalizationInput);
}

void QML_Cru::slotPostUpdateExamField(QVariantMap param)
{
    DS_ExamDef::ExamField examField = ImrParser::ToCpp_ExamField(param);
    env->ds.cruAction->actPostUpdateExamField(examField);
}

void QML_Cru::slotPostUpdateExamFieldParameter(QVariantMap param)
{
    DS_ExamDef::ExamFieldParameter examFieldParameter = ImrParser::ToCpp_ExamFieldParameter(param);
    env->ds.cruAction->actPostUpdateExamFieldParameter(examFieldParameter);
}

void QML_Cru::slotPostUpdateLinkedAccession(QVariantMap entry, bool isLinked)
{
    DS_MwlDef::WorklistEntry entryCpp = ImrParser::ToCpp_WorklistEntry(entry);
    env->ds.cruAction->actPostUpdateLinkedAccession(entryCpp, isLinked);
}

void QML_Cru::slotApplyLimits()
{
    QString actGuid = Util::newGuid();
    env->ds.cruAction->actApplyLimits(actGuid);
}
