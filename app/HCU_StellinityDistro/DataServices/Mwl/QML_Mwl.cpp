#include "QML_Mwl.h"
#include "DataServices/Mwl/DS_MwlData.h"
#include "DataServices/Mwl/DS_MwlAction.h"
#include "DataServices/Exam/DS_ExamData.h"
#include "DataServices/Capabilities/DS_Capabilities.h"
#include "DataServices/Cru/DS_CruData.h"
#include "Common/ImrParser.h"

QML_Mwl::QML_Mwl(QObject *parent, EnvGlobal *env_) :
    QObject(parent),
    env(env_)
{
    envLocal = new EnvLocal("QML_Mwl", "QML_MWL");
    qmlSrc = env->qml.object->findChild<QObject*>("dsMwl");
    env->qml.engine->rootContext()->setContextProperty("dsMwlCpp", this);

    if (qmlSrc == NULL)
    {
        LOG_ERROR("Failed to assign qmlSrc.\n");
    }

    // Register Data Callbacks
    connect(env->ds.mwlData, &DS_MwlData::signalDataChanged_WorklistEntries, this, [=](const DS_MwlDef::WorklistEntries worklistEntries) {
        qmlSrc->setProperty("worklistEntries", ImrParser::ToImr_WorklistEntries(worklistEntries));
    });

    connect(env->ds.mwlData, &DS_MwlData::signalDataChanged_SuiteName, this, [=](QString suiteName) {
        qmlSrc->setProperty("suiteName", suiteName);
    });

    connect(env->ds.mwlData, &DS_MwlData::signalDataChanged_PatientName, this, [=](QString patientName) {
        qmlSrc->setProperty("patientName", patientName);
    });

    connect(env->ds.mwlData, &DS_MwlData::signalDataChanged_StudyDescription, this, [=](QString studyDescription) {
        qmlSrc->setProperty("studyDescription", studyDescription);
    });
}

QML_Mwl::~QML_Mwl()
{
    delete envLocal;
}

void QML_Mwl::slotPatientsReload()
{
    env->ds.mwlAction->actQueryWorklist();
}

void QML_Mwl::slotSelectWorklistEntry(QString studyUid)
{
    // Clear examAdvanceInfo so it can be force-refreshed when new data received from CRU.
    // Note, when HCU sets new data with bad value, CRU may invalidate the worklist entry
    DS_ExamDef::ExamAdvanceInfo examAdvanceInfo;
    env->ds.examData->setExamAdvanceInfo(examAdvanceInfo);

    env->ds.mwlAction->actSelectWorklistEntry(studyUid);
}


void QML_Mwl::slotDeselectWorklistEntry()
{
    // send empty studyUid to CRU
    slotSelectWorklistEntry("");
}
