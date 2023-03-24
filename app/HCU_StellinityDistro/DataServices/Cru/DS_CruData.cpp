#include "Apps/AppManager.h"
#include "DS_CruData.h"
#include "DataServices/CfgLocal/DS_CfgLocal.h"
#include "DataServices/Capabilities/DS_Capabilities.h"

DS_CruData::DS_CruData(QObject *parent, EnvGlobal *env_) :
    QObject(parent),
    env(env_)
{
    envLocal = new EnvLocal("DS_Cru-Data", "CRU_DATA");

    connect(env->appManager, &AppManager::signalAppStarted, this, [=]() {
        slotAppStarted();
    });

    m_DataLocked = false;

    m_CruLinkStatus.type = DS_CruDef::CRU_LINK_TYPE_UNKNOWN;
    m_CruLinkStatus.state = DS_CruDef::CRU_LINK_STATE_INACTIVE;
    m_CruLinkStatus.quality = DS_CruDef::CRU_LINK_QUALITY_UNKNOWN;

    m_LicenseEnabledWorklistSelection = false;
    m_LicenseEnabledPatientStudyContext = false;


    QFile fileBuf(QString(PATH_RESOURCES_DEFAULT_CONFIG) + "/SampleCruData.json");
    if (fileBuf.open(QFile::ReadOnly | QFile::Text))
    {
        QString jsonStr = fileBuf.readAll();
        fileBuf.close();

        QJsonParseError parseErr;
        QJsonDocument document = QJsonDocument::fromJson(jsonStr.toUtf8(), &parseErr);

        if (parseErr.error == QJsonParseError::NoError)
        {
            m_SampleData = document.toVariant().toMap();
        }
        else
        {
            LOG_ERROR("Failed to load %s. ParseErr=%s\n", fileBuf.fileName().CSTR(), parseErr.errorString().CSTR());
        }
    }
    else
    {
        LOG_WARNING("Failed to open %s\n", fileBuf.fileName().CSTR());
    }

    SET_LAST_DATA(DataLocked)
    SET_LAST_DATA(LicenseEnabledWorklistSelection)
    SET_LAST_DATA(LicenseEnabledPatientStudyContext)
    SET_LAST_DATA(SerialNumber)
    SET_LAST_DATA(SoftwareVersion)
    SET_LAST_DATA(CruLinkStatus)
    SET_LAST_DATA(CurrentUtcNowEpochSec)
    SET_LAST_DATA(WifiSsid)
    SET_LAST_DATA(WifiPassword)
}

DS_CruData::~DS_CruData()
{
    delete envLocal;
}

void DS_CruData::slotAppStarted()
{
    if (env->ds.capabilities->get_Developer_AdvanceModeDevModeEnabled())
    {
        m_LicenseEnabledWorklistSelection = true;
        m_LicenseEnabledPatientStudyContext = true;
    }

    EMIT_DATA_CHANGED_SIGNAL(LicenseEnabledWorklistSelection)
    EMIT_DATA_CHANGED_SIGNAL(LicenseEnabledPatientStudyContext)
    EMIT_DATA_CHANGED_SIGNAL(SerialNumber)
    EMIT_DATA_CHANGED_SIGNAL(SoftwareVersion)
    EMIT_DATA_CHANGED_SIGNAL(CruLinkStatus)
    EMIT_DATA_CHANGED_SIGNAL(WifiSsid)
    EMIT_DATA_CHANGED_SIGNAL(WifiPassword)
}

