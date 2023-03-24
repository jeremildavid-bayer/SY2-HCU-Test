#ifndef DS_HARDWAREINFO_H
#define DS_HARDWAREINFO_H

#include <QObject>
#include <QMutex>
#include <QString>
#include "Common/Common.h"
#include "Common/Config.h"
#include "DataServices/DataServicesMacros.h"

class DS_HardwareInfo : public QObject
{
    Q_OBJECT

public:
    explicit DS_HardwareInfo(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~DS_HardwareInfo();
    QVariantMap getConfigs(QString *err = NULL);
    void setConfigs(QVariantMap configs, bool setChangedAt, QString *err);
    void updateCalibrationInfo(QString type);

    static void registerDataTypesForThread()
    {
        qRegisterMetaType<Config::Item>("Config::Item");
    }

signals:
    void signalConfigChanged();

    // Define OUTGOING Signals (i.e. WHEN DATA CHANGED)
    CREATE_CONFIG_CHANGED_SIGNAL(Hidden_CalibrationInfo)
    CREATE_CONFIG_CHANGED_SIGNAL(General_ModelNumber)
    CREATE_CONFIG_CHANGED_SIGNAL(General_SerialNumber)
    CREATE_CONFIG_CHANGED_SIGNAL(General_BaseBoardType)
    CREATE_CONFIG_CHANGED_SIGNAL(Boards_Revision_PA1300_Main)
    CREATE_CONFIG_CHANGED_SIGNAL(Boards_Revision_PA1301_Top)
    CREATE_CONFIG_CHANGED_SIGNAL(Boards_Revision_PA1302_MotorModuleS0)
    CREATE_CONFIG_CHANGED_SIGNAL(Boards_Revision_PA1302_MotorModuleC1)
    CREATE_CONFIG_CHANGED_SIGNAL(Boards_Revision_PA1302_MotorModuleC2)
    CREATE_CONFIG_CHANGED_SIGNAL(Boards_Revision_PA1303_Stopcock)
    CREATE_CONFIG_CHANGED_SIGNAL(Boards_Revision_PA1303_Stopcock_Assy)
    CREATE_CONFIG_CHANGED_SIGNAL(Boards_Revision_PA1304_Base)
    CREATE_CONFIG_CHANGED_SIGNAL(Boards_Revision_PA1305_SUDSSensor)
    CREATE_CONFIG_CHANGED_SIGNAL(Boards_Revision_PA1308_BatteryA)
    CREATE_CONFIG_CHANGED_SIGNAL(Boards_Revision_PA1308_BatteryB)
    CREATE_CONFIG_CHANGED_SIGNAL(Boards_Revision_PA1309_Waste)
    CREATE_CONFIG_CHANGED_SIGNAL(Boards_Revision_PA1310_HCU_Q7_Module)
    CREATE_CONFIG_CHANGED_SIGNAL(Boards_Revision_PA1310_HCU_Carrier_Board)
    CREATE_CONFIG_CHANGED_SIGNAL(Boards_Revision_PA1310_HCU_Wifi_Module)
    CREATE_CONFIG_CHANGED_SIGNAL(Boards_Revision_PA1314_DoorLEDLeft)
    CREATE_CONFIG_CHANGED_SIGNAL(Boards_Revision_PA1314_DoorLEDRight)
    CREATE_CONFIG_CHANGED_SIGNAL(Boards_Revision_PA1315_CoreHeatMaintainer)
    CREATE_CONFIG_CHANGED_SIGNAL(Boards_Revision_PA1317_DoorHeatMaintainer)
    CREATE_CONFIG_CHANGED_SIGNAL(Boards_Revision_PA1318_BarcodeAdaptor)
    CREATE_CONFIG_CHANGED_SIGNAL(Boards_SerialNumber_PA1300_Main)
    CREATE_CONFIG_CHANGED_SIGNAL(Boards_SerialNumber_PA1301_Top)
    CREATE_CONFIG_CHANGED_SIGNAL(Boards_SerialNumber_PA1302_MotorModuleS)
    CREATE_CONFIG_CHANGED_SIGNAL(Boards_SerialNumber_PA1302_MotorModuleC1)
    CREATE_CONFIG_CHANGED_SIGNAL(Boards_SerialNumber_PA1302_MotorModuleC2)
    CREATE_CONFIG_CHANGED_SIGNAL(Boards_SerialNumber_PA1303_Stopcock)
    CREATE_CONFIG_CHANGED_SIGNAL(Boards_SerialNumber_PA1303_Stopcock_Assy)
    CREATE_CONFIG_CHANGED_SIGNAL(Boards_SerialNumber_PA1304_Base)
    CREATE_CONFIG_CHANGED_SIGNAL(Boards_SerialNumber_PA1305_SUDSSensor)
    CREATE_CONFIG_CHANGED_SIGNAL(Boards_SerialNumber_PA1308_BatteryA)
    CREATE_CONFIG_CHANGED_SIGNAL(Boards_SerialNumber_PA1308_BatteryB)
    CREATE_CONFIG_CHANGED_SIGNAL(Boards_SerialNumber_PA1309_Waste)
    CREATE_CONFIG_CHANGED_SIGNAL(Boards_SerialNumber_PA1310_HCU_Q7_Module)
    CREATE_CONFIG_CHANGED_SIGNAL(Boards_SerialNumber_PA1310_HCU_Carrier_Board)
    CREATE_CONFIG_CHANGED_SIGNAL(Boards_SerialNumber_PA1310_HCU_Wifi_Module)
    CREATE_CONFIG_CHANGED_SIGNAL(Boards_SerialNumber_PA1314_DoorLEDLeft)
    CREATE_CONFIG_CHANGED_SIGNAL(Boards_SerialNumber_PA1314_DoorLEDRight)
    CREATE_CONFIG_CHANGED_SIGNAL(Boards_SerialNumber_PA1315_CoreHeatMaintainer)
    CREATE_CONFIG_CHANGED_SIGNAL(Boards_SerialNumber_PA1317_DoorHeatMaintainer)
    CREATE_CONFIG_CHANGED_SIGNAL(Boards_SerialNumber_PA1318_BarcodeAdaptor)

private:
    // Creates GET/SET Properties
    CREATE_CONFIG_MEMBERS(Hidden_CalibrationInfo)
    CREATE_CONFIG_MEMBERS(General_ModelNumber)
    CREATE_CONFIG_MEMBERS_EX(General_SerialNumber, QString, toString);
    CREATE_CONFIG_MEMBERS(General_BaseBoardType)
    CREATE_CONFIG_MEMBERS(Boards_Revision_PA1300_Main)
    CREATE_CONFIG_MEMBERS(Boards_Revision_PA1301_Top)
    CREATE_CONFIG_MEMBERS(Boards_Revision_PA1302_MotorModuleS0)
    CREATE_CONFIG_MEMBERS(Boards_Revision_PA1302_MotorModuleC1)
    CREATE_CONFIG_MEMBERS(Boards_Revision_PA1302_MotorModuleC2)
    CREATE_CONFIG_MEMBERS(Boards_Revision_PA1303_Stopcock)
    CREATE_CONFIG_MEMBERS(Boards_Revision_PA1303_Stopcock_Assy)
    CREATE_CONFIG_MEMBERS(Boards_Revision_PA1304_Base)
    CREATE_CONFIG_MEMBERS(Boards_Revision_PA1305_SUDSSensor)
    CREATE_CONFIG_MEMBERS(Boards_Revision_PA1308_BatteryA)
    CREATE_CONFIG_MEMBERS(Boards_Revision_PA1308_BatteryB)
    CREATE_CONFIG_MEMBERS(Boards_Revision_PA1309_Waste)
    CREATE_CONFIG_MEMBERS(Boards_Revision_PA1310_HCU_Q7_Module)
    CREATE_CONFIG_MEMBERS(Boards_Revision_PA1310_HCU_Carrier_Board)
    CREATE_CONFIG_MEMBERS(Boards_Revision_PA1310_HCU_Wifi_Module)
    CREATE_CONFIG_MEMBERS(Boards_Revision_PA1314_DoorLEDLeft)
    CREATE_CONFIG_MEMBERS(Boards_Revision_PA1314_DoorLEDRight)
    CREATE_CONFIG_MEMBERS(Boards_Revision_PA1315_CoreHeatMaintainer)
    CREATE_CONFIG_MEMBERS(Boards_Revision_PA1317_DoorHeatMaintainer)
    CREATE_CONFIG_MEMBERS(Boards_Revision_PA1318_BarcodeAdaptor)
    CREATE_CONFIG_MEMBERS(Boards_SerialNumber_PA1300_Main)
    CREATE_CONFIG_MEMBERS(Boards_SerialNumber_PA1301_Top)
    CREATE_CONFIG_MEMBERS(Boards_SerialNumber_PA1302_MotorModuleS)
    CREATE_CONFIG_MEMBERS(Boards_SerialNumber_PA1302_MotorModuleC1)
    CREATE_CONFIG_MEMBERS(Boards_SerialNumber_PA1302_MotorModuleC2)
    CREATE_CONFIG_MEMBERS(Boards_SerialNumber_PA1303_Stopcock)
    CREATE_CONFIG_MEMBERS(Boards_SerialNumber_PA1303_Stopcock_Assy)
    CREATE_CONFIG_MEMBERS(Boards_SerialNumber_PA1304_Base)
    CREATE_CONFIG_MEMBERS(Boards_SerialNumber_PA1305_SUDSSensor)
    CREATE_CONFIG_MEMBERS(Boards_SerialNumber_PA1308_BatteryA)
    CREATE_CONFIG_MEMBERS(Boards_SerialNumber_PA1308_BatteryB)
    CREATE_CONFIG_MEMBERS(Boards_SerialNumber_PA1309_Waste)
    CREATE_CONFIG_MEMBERS(Boards_SerialNumber_PA1310_HCU_Q7_Module)
    CREATE_CONFIG_MEMBERS(Boards_SerialNumber_PA1310_HCU_Carrier_Board)
    CREATE_CONFIG_MEMBERS(Boards_SerialNumber_PA1310_HCU_Wifi_Module)
    CREATE_CONFIG_MEMBERS(Boards_SerialNumber_PA1314_DoorLEDLeft)
    CREATE_CONFIG_MEMBERS(Boards_SerialNumber_PA1314_DoorLEDRight)
    CREATE_CONFIG_MEMBERS(Boards_SerialNumber_PA1315_CoreHeatMaintainer)
    CREATE_CONFIG_MEMBERS(Boards_SerialNumber_PA1317_DoorHeatMaintainer)
    CREATE_CONFIG_MEMBERS(Boards_SerialNumber_PA1318_BarcodeAdaptor)

private:
    Config *db;
    EnvGlobal *env;
    EnvLocal *envLocal;

    void initConfig();
    void emitAllConfigChanged();

private slots:
    void slotAppStarted();
};

#endif // DS_HARDWAREINFO_H
