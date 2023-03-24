#ifndef DS_SYSTEM_DATA_H
#define DS_SYSTEM_DATA_H

#include "Common/Common.h"
#include "DS_SystemDef.h"
#include "DataServices/DataServicesMacros.h"

class DS_SystemData : public QObject
{
    Q_OBJECT

public:
    explicit DS_SystemData(QObject *parent = 0, EnvGlobal * = NULL);
    ~DS_SystemData();

    static void registerDataTypesForThread()
    {
        qRegisterMetaType<DS_SystemDef::StatePath>("DS_SystemDef::StatePath");
        qRegisterMetaType<DS_SystemDef::NetworkSettingParams>("DS_SystemDef::NetworkSettingParams");
        qRegisterMetaType<DS_SystemDef::IwconfigParams>("DS_SystemDef::IwconfigParams");
        qRegisterMetaType<DS_SystemDef::HcuTemperatureParams>();
    }

signals:
    // Define OUTGOING Signals (i.e. WHEN DATA CHANGED)
    CREATE_DATA_CHANGED_SIGNAL(bool, DataLocked)
    CREATE_DATA_CHANGED_SIGNAL(bool, DigestLocked)
    CREATE_DATA_CHANGED_SIGNAL(QString, Version)
    CREATE_DATA_CHANGED_SIGNAL(QString, BuildDate)
    CREATE_DATA_CHANGED_SIGNAL(QString, BuildType)
    CREATE_DATA_CHANGED_SIGNAL(DS_SystemDef::StatePath, StatePath)
    CREATE_DATA_CHANGED_SIGNAL(double, DiskMainFreeSpaceMB)
    CREATE_DATA_CHANGED_SIGNAL(double, DiskUserFreeSpaceMB)
    CREATE_DATA_CHANGED_SIGNAL(QDateTime, Uptime)
    CREATE_DATA_CHANGED_SIGNAL(bool, UsbStickInserted)
    CREATE_DATA_CHANGED_SIGNAL(DS_SystemDef::HcuTemperatureParams, HcuTemperatureParams)
    CREATE_DATA_CHANGED_SIGNAL(QString, CompatibleMcuVersionPrefix)
    CREATE_DATA_CHANGED_SIGNAL(QString, CompatibleMcuCommandVersion)
    CREATE_DATA_CHANGED_SIGNAL(DS_SystemDef::NetworkSettingParams, NetworkSettingParams)
    CREATE_DATA_CHANGED_SIGNAL(DS_SystemDef::IwconfigParams, IwconfigParams)

private:
    // Creates GET/SET Properties
    CREATE_DATA_MEMBERS(bool, DataLocked)
    CREATE_DATA_MEMBERS(bool, DigestLocked)
    CREATE_DATA_MEMBERS(QString, Version)
    CREATE_DATA_MEMBERS(QString, BuildDate)
    CREATE_DATA_MEMBERS(QString, BuildType)
    CREATE_DATA_MEMBERS(DS_SystemDef::StatePath, StatePath)
    CREATE_DATA_MEMBERS(double, DiskMainFreeSpaceMB)
    CREATE_DATA_MEMBERS(double, DiskUserFreeSpaceMB)
    CREATE_DATA_MEMBERS(QDateTime, Uptime)
    CREATE_DATA_MEMBERS(bool, UsbStickInserted)
    CREATE_DATA_MEMBERS(DS_SystemDef::HcuTemperatureParams, HcuTemperatureParams)
    CREATE_DATA_MEMBERS(QString, CompatibleMcuVersionPrefix)
    CREATE_DATA_MEMBERS(QString, CompatibleMcuCommandVersion)
    CREATE_DATA_MEMBERS(DS_SystemDef::NetworkSettingParams, NetworkSettingParams)
    CREATE_DATA_MEMBERS(DS_SystemDef::IwconfigParams, IwconfigParams)

    EnvGlobal *env;

public slots:
    void slotAppStarted();
};

#endif // DS_MCU_DATA_H
