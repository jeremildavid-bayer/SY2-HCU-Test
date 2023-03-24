#include "Apps/AppManager.h"
#include "MwlMonitor.h"
#include "DataServices/Mwl/DS_MwlData.h"
#include "DataServices/Cru/DS_CruData.h"
#include "DataServices/Exam/DS_ExamData.h"
#include "DataServices/Alert/DS_AlertAction.h"
#include "DataServices/System/DS_SystemData.h"
#include "DataServices/Workflow/DS_WorkflowData.h"
#include "Common/ImrParser.h"

MwlMonitor::MwlMonitor(QObject *parent, EnvGlobal *env_) :
    QObject(parent),
    env(env_)
{
    envLocal = new EnvLocal("DS_Mwl-Monitor", "MWL_MONITOR");
    isActive = false;

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });
}

MwlMonitor::~MwlMonitor()
{
    delete envLocal;
}

void MwlMonitor::slotAppInitialised()
{
    connect(env->ds.systemData, &DS_SystemData::signalDataChanged_StatePath, this, [=](DS_SystemDef::StatePath curStatePath, DS_SystemDef::StatePath prevStatePath) {
        if ( ( (prevStatePath == DS_SystemDef::STATE_PATH_IDLE) && (curStatePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING) ) ||
             ( (prevStatePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING) && (curStatePath == DS_SystemDef::STATE_PATH_IDLE) ) )
        {
            LOG_INFO("\n\n-----------------\nStatePath=%s\n\n", ImrParser::ToImr_StatePath(curStatePath).CSTR());
        }
    });

    connect(env->ds.workflowData, &DS_WorkflowData::signalDataChanged_WorkflowState, this, [=](DS_WorkflowDef::WorkflowState workflowState, DS_WorkflowDef::WorkflowState workflowStatePrev) {
        if ( (workflowState != DS_WorkflowDef::STATE_INACTIVE) &&
             (workflowStatePrev == DS_WorkflowDef::STATE_INACTIVE) )
        {
            isActive = true;
        }
        else if ( (workflowState == DS_WorkflowDef::STATE_INACTIVE) &&
                  (workflowStatePrev != DS_WorkflowDef::STATE_INACTIVE) )
        {
            isActive = false;
        }
    });

    connect(env->ds.mwlData, &DS_MwlData::signalDataChanged_SuiteName, this, [=] {
        handleSuiteNameChanged();
    });

    connect(env->ds.cruData, &DS_CruData::signalDataChanged_CruLinkStatus, this, [=] {
        handleSuiteNameChanged();
        handleCruLinkStateOrExamStateChanged();
    });

    connect(env->ds.examData, &DS_ExamData::signalDataChanged_ExamGuid, this, [=] {
        handleCruLinkStateOrExamStateChanged();
    });
}

void MwlMonitor::handleSuiteNameChanged()
{
    DS_CruDef::CruLinkStatus linkStatus = env->ds.cruData->getCruLinkStatus();

    if (linkStatus.state != DS_CruDef::CRU_LINK_STATE_ACTIVE)
    {
        // Reset Suite Name
        return;
    }

    QString suiteName = env->ds.mwlData->getSuiteName();

    if ( (suiteName == "") ||
         (suiteName == "-- ? --") ||
         (suiteName == "Default") )
    {
        env->ds.alertAction->activate("SuiteNameNotAssigned");
    }
    else
    {
        env->ds.alertAction->deactivate("SuiteNameNotAssigned");
    }
}

void MwlMonitor::handleCruLinkStateOrExamStateChanged()
{
    DS_CruDef::CruLinkStatus linkStatus = env->ds.cruData->getCruLinkStatus();
    bool isExamStarted = env->ds.examData->isExamStarted();
    QString prevPatientName = env->ds.mwlData->getPatientName();
    QString prevStudyDescription = env->ds.mwlData->getStudyDescription();

    if (linkStatus.state != DS_CruDef::CRU_LINK_STATE_ACTIVE)
    {
        if (!isExamStarted)
        {
            if (prevPatientName != "")
            {
                LOG_WARNING("handleCruLinkStateOrExamStateChanged(): CRU Link down while exam is not started. Setting PatientName to empty\n");
                env->ds.mwlData->setPatientName("");
            }

            if (prevStudyDescription != "")
            {
                LOG_WARNING("handleCruLinkStateOrExamStateChanged(): CRU Link down while exam is not started. Setting StudyDescription to empty\n");
                env->ds.mwlData->setStudyDescription("");
            }
        }
    }
}
