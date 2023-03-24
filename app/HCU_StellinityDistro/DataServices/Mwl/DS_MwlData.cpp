#include "Apps/AppManager.h"
#include "DS_MwlData.h"
#include "DataServices/Capabilities/DS_Capabilities.h"
#include "DataServices/Cru/DS_CruData.h"
#include "Common/ImrParser.h"

DS_MwlData::DS_MwlData(QObject *parent, EnvGlobal *env_) :
    QObject(parent),
    env(env_)
{
    envLocal = new EnvLocal("DS_Mwl-Data", "MWL_DATA");

    connect(env->appManager, &AppManager::signalAppStarted, this, [=]() {
        slotAppStarted();
    });

    m_DataLocked = false;

    SET_LAST_DATA(DataLocked)
    SET_LAST_DATA(SuiteName)
    SET_LAST_DATA(PatientName)
    SET_LAST_DATA(StudyDescription)
    SET_LAST_DATA(WorklistEntries)
}

DS_MwlData::~DS_MwlData()
{
    delete envLocal;
}

void DS_MwlData::slotAppStarted()
{
    if (env->ds.capabilities->get_Developer_AdvanceModeDevModeEnabled())
    {
        QVariantMap cruSampleData = env->ds.cruData->getSampleData();

        if (cruSampleData.contains("WorklistEntries"))
        {
            QString err;
            QVariantList list = cruSampleData["WorklistEntries"].toList();
            m_WorklistEntries = ImrParser::ToCpp_WorklistEntries(list, &err);

            if (err != "")
            {
                LOG_ERROR("slotAppStarted(): Parse Error (src=%s, err=%s)\n", Util::qVarientToJsonData(list).CSTR(), err.CSTR());
            }
        }
        else
        {
            LOG_ERROR("slotAppStarted(): Cannot load WorklistEntries from CRU Sample Data\n");
        }
    }

    EMIT_DATA_CHANGED_SIGNAL(SuiteName)
    EMIT_DATA_CHANGED_SIGNAL(PatientName)
    EMIT_DATA_CHANGED_SIGNAL(StudyDescription)
    EMIT_DATA_CHANGED_SIGNAL(WorklistEntries)
}

const DS_MwlDef::WorklistEntry *DS_MwlData::getWorklistEntry(QString studyUid) const
{
    for (int entryIdx = 0; entryIdx < m_WorklistEntries.length(); entryIdx++)
    {
        if (m_WorklistEntries[entryIdx].studyInstanceUid == studyUid)
        {
            return &m_WorklistEntries[entryIdx];
        }
    }

    return NULL;
}
