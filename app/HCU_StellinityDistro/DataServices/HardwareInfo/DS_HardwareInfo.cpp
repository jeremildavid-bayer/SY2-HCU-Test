#include "Apps/AppManager.h"
#include "DS_HardwareInfo.h"
#include "Common/ImrParser.h"
#include "DataServices/Mcu/DS_McuAction.h"

DS_HardwareInfo::DS_HardwareInfo(QObject *parent, EnvGlobal *env_) :
    QObject(parent),
    env(env_)
{
    envLocal = new EnvLocal("DS_HardwareInfo", "HARDWARE_INFO");

    db = new Config(PATH_INFO_HARDWARE, env, envLocal);

    if (!db->isCfgOk())
    {
        db->initCfgFile();
    }

    connect(env->appManager, &AppManager::signalAppStarted, this, [=]() {
        slotAppStarted();
    });

    initConfig();
}

DS_HardwareInfo::~DS_HardwareInfo()
{
    delete db;
    delete envLocal;
}

void DS_HardwareInfo::initConfig()
{
    Config::Item cfgItem;
    Config *dbNew = new Config("", env, envLocal);

    QVariantMap calibrationInfoMap;
    calibrationInfoMap.insert("Info", "");
    calibrationInfoMap.insert("CompletedAt", QVariantMap());
    cfgItem = db->initItem("Hidden_CalibrationInfo",
                           0,
                           calibrationInfoMap,
                           "",
                           "object",
                           QVariantList(),
                           QVariant(0),
                           QVariant(0),
                           QVariant(0));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("General_ModelNumber",
                           0,
                           "ST001",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(5),
                           QVariant(32),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("General_SerialNumber",
                           1,
                           "10000000",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(3),
                           QVariant(10),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("General_BaseBoardType",
                           2,
                           "OCS",
                           "",
                           "list",
                           QVariantList() << "OCS" << "Battery" << "NoBattery",
                           QVariant(0),
                           QVariant(0),
                           QVariant(0));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Boards_Revision_PA1300_Main",
                           0,
                           "Xx",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(2),
                           QVariant(3),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Boards_Revision_PA1301_Top",
                           1,
                           "Xx",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(2),
                           QVariant(3),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Boards_Revision_PA1302_MotorModuleS0",
                           2,
                           "Xx",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(2),
                           QVariant(3),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Boards_Revision_PA1302_MotorModuleC1",
                           3,
                           "Xx",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(2),
                           QVariant(3),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Boards_Revision_PA1302_MotorModuleC2",
                           4,
                           "Xx",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(2),
                           QVariant(3),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Boards_Revision_PA1303_Stopcock",
                           5,
                           "Xx",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(2),
                           QVariant(3),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Boards_Revision_PA1303_Stopcock_Assy",
                           6,
                           "Xx",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(2),
                           QVariant(3),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Boards_Revision_PA1304_Base",
                           7,
                           "Xx",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(2),
                           QVariant(3),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Boards_Revision_PA1305_SUDSSensor",
                           8,
                           "Xx",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(2),
                           QVariant(3),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Boards_Revision_PA1308_BatteryA",
                           9,
                           "Xx",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(2),
                           QVariant(3),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Boards_Revision_PA1308_BatteryB",
                           10,
                           "Xx",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(2),
                           QVariant(3),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Boards_Revision_PA1309_Waste",
                           11,
                           "Xx",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(2),
                           QVariant(3),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Boards_Revision_PA1310_HCU_Q7_Module",
                           13,
                           "Xx",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(2),
                           QVariant(3),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Boards_Revision_PA1310_HCU_Carrier_Board",
                           14,
                           "Xx",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(2),
                           QVariant(3),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Boards_Revision_PA1310_HCU_Wifi_Module",
                           15,
                           "Xx",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(2),
                           QVariant(3),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Boards_Revision_PA1314_DoorLEDLeft",
                           16,
                           "Xx",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(2),
                           QVariant(3),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Boards_Revision_PA1314_DoorLEDRight",
                           17,
                           "Xx",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(2),
                           QVariant(3),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Boards_Revision_PA1315_CoreHeatMaintainer",
                           18,
                           "Xx",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(2),
                           QVariant(3),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Boards_Revision_PA1317_DoorHeatMaintainer",
                           19,
                           "Xx",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(2),
                           QVariant(3),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Boards_Revision_PA1318_BarcodeAdaptor",
                           20,
                           "Xx",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(2),
                           QVariant(3),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Boards_SerialNumber_PA1300_Main",
                           0,
                           "",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(0),
                           QVariant(15),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Boards_SerialNumber_PA1301_Top",
                           1,
                           "",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(0),
                           QVariant(15),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Boards_SerialNumber_PA1302_MotorModuleS0",
                           2,
                           "",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(0),
                           QVariant(15),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Boards_SerialNumber_PA1302_MotorModuleC1",
                           3,
                           "",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(0),
                           QVariant(15),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Boards_SerialNumber_PA1302_MotorModuleC2",
                           4,
                           "",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(0),
                           QVariant(15),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Boards_SerialNumber_PA1303_Stopcock",
                           5,
                           "",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(0),
                           QVariant(15),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Boards_SerialNumber_PA1303_Stopcock_Assy",
                           6,
                           "",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(0),
                           QVariant(15),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Boards_SerialNumber_PA1304_Base",
                           7,
                           "",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(0),
                           QVariant(15),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Boards_SerialNumber_PA1305_SUDSSensor",
                           8,
                           "",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(0),
                           QVariant(15),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Boards_SerialNumber_PA1308_BatteryA",
                           9,
                           "",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(0),
                           QVariant(15),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Boards_SerialNumber_PA1308_BatteryB",
                           10,
                           "",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(0),
                           QVariant(15),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Boards_SerialNumber_PA1309_Waste",
                           11,
                           "",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(0),
                           QVariant(15),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Boards_SerialNumber_PA1310_HCU_Q7_Module",
                           13,
                           "",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(0),
                           QVariant(15),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Boards_SerialNumber_PA1310_HCU_Carrier_Board",
                           14,
                           "",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(0),
                           QVariant(15),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Boards_SerialNumber_PA1310_HCU_Wifi_Module",
                           15,
                           "",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(0),
                           QVariant(15),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Boards_SerialNumber_PA1314_DoorLEDLeft",
                           16,
                           "",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(0),
                           QVariant(15),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Boards_SerialNumber_PA1314_DoorLEDRight",
                           17,
                           "",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(0),
                           QVariant(15),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Boards_SerialNumber_PA1315_CoreHeatMaintainer",
                           18,
                           "",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(0),
                           QVariant(15),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Boards_SerialNumber_PA1317_DoorHeatMaintainer",
                           19,
                           "",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(0),
                           QVariant(15),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Boards_SerialNumber_PA1318_BarcodeAdaptor",
                           20,
                           "",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(0),
                           QVariant(15),
                           QVariant(1));
    dbNew->set(cfgItem);

    // Set new db
    dbNew->setPathConfig(db->getPathConfig());
    dbNew->saveCfgFile();
    delete db;
    db = dbNew;
}

void DS_HardwareInfo::slotAppStarted()
{
    // Register Config Set Callbacks

    emitAllConfigChanged();
}

void DS_HardwareInfo::emitAllConfigChanged()
{
    EMIT_CONFIG_CHANGED_SIGNAL(Hidden_CalibrationInfo)
    EMIT_CONFIG_CHANGED_SIGNAL(General_ModelNumber)
    EMIT_CONFIG_CHANGED_SIGNAL(General_SerialNumber)
    EMIT_CONFIG_CHANGED_SIGNAL(General_BaseBoardType)
    EMIT_CONFIG_CHANGED_SIGNAL(Boards_Revision_PA1300_Main)
    EMIT_CONFIG_CHANGED_SIGNAL(Boards_Revision_PA1301_Top)
    EMIT_CONFIG_CHANGED_SIGNAL(Boards_Revision_PA1302_MotorModuleS0)
    EMIT_CONFIG_CHANGED_SIGNAL(Boards_Revision_PA1302_MotorModuleC1)
    EMIT_CONFIG_CHANGED_SIGNAL(Boards_Revision_PA1302_MotorModuleC2)
    EMIT_CONFIG_CHANGED_SIGNAL(Boards_Revision_PA1303_Stopcock)
    EMIT_CONFIG_CHANGED_SIGNAL(Boards_Revision_PA1303_Stopcock_Assy)
    EMIT_CONFIG_CHANGED_SIGNAL(Boards_Revision_PA1304_Base)
    EMIT_CONFIG_CHANGED_SIGNAL(Boards_Revision_PA1305_SUDSSensor)
    EMIT_CONFIG_CHANGED_SIGNAL(Boards_Revision_PA1308_BatteryA)
    EMIT_CONFIG_CHANGED_SIGNAL(Boards_Revision_PA1308_BatteryB)
    EMIT_CONFIG_CHANGED_SIGNAL(Boards_Revision_PA1309_Waste)
    EMIT_CONFIG_CHANGED_SIGNAL(Boards_Revision_PA1310_HCU_Q7_Module)
    EMIT_CONFIG_CHANGED_SIGNAL(Boards_Revision_PA1310_HCU_Carrier_Board)
    EMIT_CONFIG_CHANGED_SIGNAL(Boards_Revision_PA1310_HCU_Wifi_Module)
    EMIT_CONFIG_CHANGED_SIGNAL(Boards_Revision_PA1314_DoorLEDLeft)
    EMIT_CONFIG_CHANGED_SIGNAL(Boards_Revision_PA1314_DoorLEDRight)
    EMIT_CONFIG_CHANGED_SIGNAL(Boards_Revision_PA1315_CoreHeatMaintainer)
    EMIT_CONFIG_CHANGED_SIGNAL(Boards_Revision_PA1317_DoorHeatMaintainer)
    EMIT_CONFIG_CHANGED_SIGNAL(Boards_Revision_PA1318_BarcodeAdaptor)
    EMIT_CONFIG_CHANGED_SIGNAL(Boards_SerialNumber_PA1300_Main)
    EMIT_CONFIG_CHANGED_SIGNAL(Boards_SerialNumber_PA1301_Top)
    EMIT_CONFIG_CHANGED_SIGNAL(Boards_SerialNumber_PA1302_MotorModuleS)
    EMIT_CONFIG_CHANGED_SIGNAL(Boards_SerialNumber_PA1302_MotorModuleC1)
    EMIT_CONFIG_CHANGED_SIGNAL(Boards_SerialNumber_PA1302_MotorModuleC2)
    EMIT_CONFIG_CHANGED_SIGNAL(Boards_SerialNumber_PA1303_Stopcock)
    EMIT_CONFIG_CHANGED_SIGNAL(Boards_SerialNumber_PA1303_Stopcock_Assy)
    EMIT_CONFIG_CHANGED_SIGNAL(Boards_SerialNumber_PA1304_Base)
    EMIT_CONFIG_CHANGED_SIGNAL(Boards_SerialNumber_PA1305_SUDSSensor)
    EMIT_CONFIG_CHANGED_SIGNAL(Boards_SerialNumber_PA1308_BatteryA)
    EMIT_CONFIG_CHANGED_SIGNAL(Boards_SerialNumber_PA1308_BatteryB)
    EMIT_CONFIG_CHANGED_SIGNAL(Boards_SerialNumber_PA1309_Waste)
    EMIT_CONFIG_CHANGED_SIGNAL(Boards_SerialNumber_PA1310_HCU_Q7_Module)
    EMIT_CONFIG_CHANGED_SIGNAL(Boards_SerialNumber_PA1310_HCU_Carrier_Board)
    EMIT_CONFIG_CHANGED_SIGNAL(Boards_SerialNumber_PA1310_HCU_Wifi_Module)
    EMIT_CONFIG_CHANGED_SIGNAL(Boards_SerialNumber_PA1314_DoorLEDLeft)
    EMIT_CONFIG_CHANGED_SIGNAL(Boards_SerialNumber_PA1314_DoorLEDRight)
    EMIT_CONFIG_CHANGED_SIGNAL(Boards_SerialNumber_PA1315_CoreHeatMaintainer)
    EMIT_CONFIG_CHANGED_SIGNAL(Boards_SerialNumber_PA1317_DoorHeatMaintainer)
    EMIT_CONFIG_CHANGED_SIGNAL(Boards_SerialNumber_PA1318_BarcodeAdaptor)

    emit signalConfigChanged();
}

QVariantMap DS_HardwareInfo::getConfigs(QString *err)
{
    if (err != NULL)
    {
        if (db->parseError().error != QJsonParseError::NoError)
        {
            *err = db->parseError().errorString();
        }
        else
        {
            *err = "";
        }
    }
    return db->getCfgMap();
}

void DS_HardwareInfo::setConfigs(QVariantMap configs, bool setChangedAt, QString *errOut)
{
    QString err;
    QList<QString> listName = configs.keys();
    QVariantMap oldCfgMap = db->getCfgMap();

    if (err != "")
    {
        LOG_ERROR("Failed to get old config map. Err=%s\n", err.CSTR());
    }

    for (int i = 0; i < listName.length(); i++)
    {
        QString itemName = listName[i];
        QVariantMap map = configs[itemName].toMap();

        Config::Item item = ImrParser::ToCpp_ConfigItem(map, &err);

        if (err != "")
        {
            err = QString().asprintf("Config Item Parse Error (%s:%s)", itemName.CSTR(), err.CSTR());
            break;
        }

        SET_CONFIG_BY_NAME(item, Hidden_CalibrationInfo, setChangedAt)
        SET_CONFIG_BY_NAME(item, General_ModelNumber, setChangedAt)
        SET_CONFIG_BY_NAME(item, General_SerialNumber, setChangedAt)
        SET_CONFIG_BY_NAME(item, General_BaseBoardType, setChangedAt)
        SET_CONFIG_BY_NAME(item, Boards_Revision_PA1300_Main, setChangedAt)
        SET_CONFIG_BY_NAME(item, Boards_Revision_PA1301_Top, setChangedAt)
        SET_CONFIG_BY_NAME(item, Boards_Revision_PA1302_MotorModuleS0, setChangedAt)
        SET_CONFIG_BY_NAME(item, Boards_Revision_PA1302_MotorModuleC1, setChangedAt)
        SET_CONFIG_BY_NAME(item, Boards_Revision_PA1302_MotorModuleC2, setChangedAt)
        SET_CONFIG_BY_NAME(item, Boards_Revision_PA1303_Stopcock, setChangedAt)
        SET_CONFIG_BY_NAME(item, Boards_Revision_PA1303_Stopcock_Assy, setChangedAt)
        SET_CONFIG_BY_NAME(item, Boards_Revision_PA1304_Base, setChangedAt)
        SET_CONFIG_BY_NAME(item, Boards_Revision_PA1305_SUDSSensor, setChangedAt)
        SET_CONFIG_BY_NAME(item, Boards_Revision_PA1308_BatteryA, setChangedAt)
        SET_CONFIG_BY_NAME(item, Boards_Revision_PA1308_BatteryB, setChangedAt)
        SET_CONFIG_BY_NAME(item, Boards_Revision_PA1309_Waste, setChangedAt)
        SET_CONFIG_BY_NAME(item, Boards_Revision_PA1310_HCU_Q7_Module, setChangedAt)
        SET_CONFIG_BY_NAME(item, Boards_Revision_PA1310_HCU_Carrier_Board, setChangedAt)
        SET_CONFIG_BY_NAME(item, Boards_Revision_PA1310_HCU_Wifi_Module, setChangedAt)
        SET_CONFIG_BY_NAME(item, Boards_Revision_PA1314_DoorLEDLeft, setChangedAt)
        SET_CONFIG_BY_NAME(item, Boards_Revision_PA1314_DoorLEDRight, setChangedAt)
        SET_CONFIG_BY_NAME(item, Boards_Revision_PA1315_CoreHeatMaintainer, setChangedAt)
        SET_CONFIG_BY_NAME(item, Boards_Revision_PA1317_DoorHeatMaintainer, setChangedAt)
        SET_CONFIG_BY_NAME(item, Boards_Revision_PA1318_BarcodeAdaptor, setChangedAt)
        SET_CONFIG_BY_NAME(item, Boards_SerialNumber_PA1300_Main, setChangedAt)
        SET_CONFIG_BY_NAME(item, Boards_SerialNumber_PA1301_Top, setChangedAt)
        SET_CONFIG_BY_NAME(item, Boards_SerialNumber_PA1302_MotorModuleS, setChangedAt)
        SET_CONFIG_BY_NAME(item, Boards_SerialNumber_PA1302_MotorModuleC1, setChangedAt)
        SET_CONFIG_BY_NAME(item, Boards_SerialNumber_PA1302_MotorModuleC2, setChangedAt)
        SET_CONFIG_BY_NAME(item, Boards_SerialNumber_PA1303_Stopcock, setChangedAt)
        SET_CONFIG_BY_NAME(item, Boards_SerialNumber_PA1303_Stopcock_Assy, setChangedAt)
        SET_CONFIG_BY_NAME(item, Boards_SerialNumber_PA1304_Base, setChangedAt)
        SET_CONFIG_BY_NAME(item, Boards_SerialNumber_PA1305_SUDSSensor, setChangedAt)
        SET_CONFIG_BY_NAME(item, Boards_SerialNumber_PA1308_BatteryA, setChangedAt)
        SET_CONFIG_BY_NAME(item, Boards_SerialNumber_PA1308_BatteryB, setChangedAt)
        SET_CONFIG_BY_NAME(item, Boards_SerialNumber_PA1309_Waste, setChangedAt)
        SET_CONFIG_BY_NAME(item, Boards_SerialNumber_PA1310_HCU_Q7_Module, setChangedAt)
        SET_CONFIG_BY_NAME(item, Boards_SerialNumber_PA1310_HCU_Carrier_Board, setChangedAt)
        SET_CONFIG_BY_NAME(item, Boards_SerialNumber_PA1310_HCU_Wifi_Module, setChangedAt)
        SET_CONFIG_BY_NAME(item, Boards_SerialNumber_PA1314_DoorLEDLeft, setChangedAt)
        SET_CONFIG_BY_NAME(item, Boards_SerialNumber_PA1314_DoorLEDRight, setChangedAt)
        SET_CONFIG_BY_NAME(item, Boards_SerialNumber_PA1315_CoreHeatMaintainer, setChangedAt)
        SET_CONFIG_BY_NAME(item, Boards_SerialNumber_PA1317_DoorHeatMaintainer, setChangedAt)
        SET_CONFIG_BY_NAME(item, Boards_SerialNumber_PA1318_BarcodeAdaptor, setChangedAt)
    }

    if (oldCfgMap != db->getCfgMap())
    {
        emit signalConfigChanged();
    }

    *errOut = err;
}

void DS_HardwareInfo::updateCalibrationInfo(QString type)
{
    QString actGuid = Util::newGuid();
    env->actionMgr->onActionStarted(actGuid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus status) {
        LOG_INFO("GET_CALIBRATION_STATUS - %s\n", ImrParser::ToImr_DataServiceActionStatusStr(status).CSTR());

        if (status.state == DS_ACTION_STATE_STARTED)
        {
            QString calInfo = status.reply.replace(";", "\n");

            // Update Calibration Info
            Config::Item cfg = getHidden_CalibrationInfo();
            QVariantMap map = cfg.value.toMap();
            QVariantMap completedMap;
            if (map.contains("CompletedAt"))
            {
                completedMap = map["CompletedAt"].toMap();
            }
            completedMap.insert(type, Util::qDateTimeToUtcDateTimeStr(QDateTime::currentDateTimeUtc()));
            map.insert("Info", calInfo);
            map.insert("CompletedAt", completedMap);
            cfg.value = map;

            LOG_DEBUG("updateCalibrationInfo(): CalibrationInfo=%s\n", Util::qVarientToJsonData(ImrParser::ToImr_ConfigItem(cfg), false).CSTR());

            setHidden_CalibrationInfo(cfg);
        }
    });
    env->ds.mcuAction->actCalDigest(actGuid);
}
