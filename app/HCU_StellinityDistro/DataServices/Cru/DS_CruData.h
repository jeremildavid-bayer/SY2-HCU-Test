#ifndef DS_CRU_DATA_H
#define DS_CRU_DATA_H

#include "DS_CruDef.h"
#include "Common/Common.h"
#include "DataServices/DataServicesMacros.h"

class DS_CruData : public QObject
{
    Q_OBJECT

public:
    explicit DS_CruData(QObject *parent = 0, EnvGlobal * = NULL);
    ~DS_CruData();

    static void registerDataTypesForThread()
    {
        qRegisterMetaType<DS_CruDef::CruLinkStatus>("DS_CruDef::CruLinkStatus");
    }

signals:
    // Define OUTGOING Signals (i.e. WHEN DATA CHANGED)
    CREATE_DATA_CHANGED_SIGNAL(bool, DataLocked)
    CREATE_DATA_CHANGED_SIGNAL(bool, LicenseEnabledWorklistSelection)
    CREATE_DATA_CHANGED_SIGNAL(bool, LicenseEnabledPatientStudyContext)
    CREATE_DATA_CHANGED_SIGNAL(QString, SerialNumber)
    CREATE_DATA_CHANGED_SIGNAL(QString, SoftwareVersion)
    CREATE_DATA_CHANGED_SIGNAL(DS_CruDef::CruLinkStatus, CruLinkStatus)
    CREATE_DATA_CHANGED_SIGNAL(qint64, CurrentUtcNowEpochSec) // UTC
    CREATE_DATA_CHANGED_SIGNAL(QString, WifiSsid)
    CREATE_DATA_CHANGED_SIGNAL(QString, WifiPassword)
    CREATE_DATA_CHANGED_SIGNAL(QVariantMap, SampleData)

private:
    // Creates GET/SET Properties
    CREATE_DATA_MEMBERS(bool, DataLocked)
    CREATE_DATA_MEMBERS(bool, LicenseEnabledWorklistSelection)
    CREATE_DATA_MEMBERS(bool, LicenseEnabledPatientStudyContext)
    CREATE_DATA_MEMBERS(QString, SerialNumber)
    CREATE_DATA_MEMBERS(QString, SoftwareVersion)
    CREATE_DATA_MEMBERS(DS_CruDef::CruLinkStatus, CruLinkStatus)
    CREATE_DATA_MEMBERS(qint64, CurrentUtcNowEpochSec)
    CREATE_DATA_MEMBERS(QString, WifiSsid)
    CREATE_DATA_MEMBERS(QString, WifiPassword)
    CREATE_DATA_MEMBERS(QVariantMap, SampleData)

    EnvGlobal *env;
    EnvLocal *envLocal;

private slots:
    void slotAppStarted();
};

#endif // DS_CRU_DATA_H
