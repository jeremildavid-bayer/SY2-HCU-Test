#include "Apps/AppManager.h"
#include "Common/ImrParser.h"
#include "DS_CfgLocal.h"
#include "DataServices/Cru/DS_CruAction.h"
#include "DataServices/Cru/DS_CruData.h"
#include "DataServices/CfgGlobal/DS_CfgGlobal.h"
#include "DataServices/System/DS_SystemData.h"
#include "DataServices/Workflow/DS_WorkflowData.h"
#include "DataServices/Device/DS_DeviceData.h"
#include "DataServices/Exam/DS_ExamData.h"


DS_CfgLocal::DS_CfgLocal(QObject *parent, EnvGlobal *env_) :
    QObject(parent),
    env(env_)
{
    envLocal = new EnvLocal("DS_CfgLocal", "CFG_LOCAL", LOG_LRG_SIZE_BYTES);

    QString dbFileName = QString(PATH_CONFIG_LOCAL) + ((env->ds.systemData->getBuildType() == BUILD_TYPE_PROD) ? ".PROD" : "");
    db = new Config(dbFileName, env, envLocal);

    if (!db->isCfgOk())
    {
        db->initCfgFile();
    }

    connect(env->appManager, &AppManager::signalAppStarted, this, [=]() {
        slotAppStarted();
    });

    initConfig();
}

DS_CfgLocal::~DS_CfgLocal()
{
    delete db;
    delete envLocal;
}

void DS_CfgLocal::initConfig()
{
    Config::Item cfgItem;
    Config *dbNew = new Config("", env, envLocal);

    cfgItem = db->initItem("Settings_SruLink_ConnectionType",
                           0,
                           "WirelessEthernet",
                           "",
                           "list",
                           QVariantList() << "WirelessEthernet" << "WiredEthernet",
                           QVariant(0),
                           QVariant(0),
                           QVariant(0),
                           Config::LegacyKeyNames() << "Networking_ConnectionType");
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Settings_Sound_NormalAudioLevel",
                           0,
                           DS_CfgLocalDef::SOUND_VOLUME_LEVEL_DEFAULT,
                           "",
                           "int",
                           QVariantList(),
                           QVariant(DS_CfgLocalDef::SOUND_VOLUME_LEVEL_MUTE),
                           QVariant(DS_CfgLocalDef::SOUND_VOLUME_LEVEL_MAX),
                           QVariant(1),
                           Config::LegacyKeyNames() << "Sound_NormalAudioLevel");
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Settings_Sound_InjectionAudioLevel",
                           1,
                           DS_CfgLocalDef::SOUND_VOLUME_LEVEL_DEFAULT,
                           "",
                           "int",
                           QVariantList(),
                           QVariant(DS_CfgLocalDef::SOUND_VOLUME_LEVEL_MUTE),
                           QVariant(DS_CfgLocalDef::SOUND_VOLUME_LEVEL_MAX),
                           QVariant(1),
                           Config::LegacyKeyNames() << "Sound_InjectionAudioLevel");
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Settings_Sound_NotificationAudioLevel",
                           2,
                           DS_CfgLocalDef::SOUND_VOLUME_LEVEL_DEFAULT,
                           "",
                           "int",
                           QVariantList(),
                           QVariant(DS_CfgLocalDef::SOUND_VOLUME_LEVEL_MIN),
                           QVariant(DS_CfgLocalDef::SOUND_VOLUME_LEVEL_MAX),
                           QVariant(1),
                           Config::LegacyKeyNames() << "Sound_NotificationAudioLevel");
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Settings_Sound_SudsPrimedAudioLevel",
                           3,
                           DS_CfgLocalDef::SOUND_VOLUME_LEVEL_DEFAULT,
                           "",
                           "int",
                           QVariantList(),
                           QVariant(DS_CfgLocalDef::SOUND_VOLUME_LEVEL_MUTE),
                           QVariant(DS_CfgLocalDef::SOUND_VOLUME_LEVEL_MAX),
                           QVariant(1),
                           Config::LegacyKeyNames() << "Sound_SudsPrimedAudioLevel");
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Settings_Sound_KeyClicksEnabled",
                           4,
                           true,
                           "",
                           "bool",
                           QVariantList(),
                           QVariant(),
                           QVariant(),
                           QVariant(),
                           Config::LegacyKeyNames() << "Sound_KeyClicksEnabled");
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Settings_Display_ScreenBrightness",
                           0,
                           DS_CfgLocalDef::SCREEN_BRIGHTNESS_LEVEL_DEFAULT,
                           "",
                           "int",
                           QVariantList(),
                           QVariant(DS_CfgLocalDef::SCREEN_BRIGHTNESS_LEVEL_MIN),
                           QVariant(DS_CfgLocalDef::SCREEN_BRIGHTNESS_LEVEL_MAX),
                           QVariant(1),
                           Config::LegacyKeyNames() << "Display_ScreenBrightness");
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Settings_Display_ScreenOffTimeoutMinutes",
                           1,
                           0,
                           "",
                           "int",
                           QVariantList(),
                           QVariant(0),
                           QVariant(30),
                           QVariant(5),
                           Config::LegacyKeyNames() << "Display_ScreenOffTimeoutMinutes");
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Settings_Display_Theme",
                           2,
                           "twilight",
                           "",
                           "list",
                           QVariantList() << "twilight" << "purity",
                           QVariant(0),
                           QVariant(0),
                           QVariant(0),
                           Config::LegacyKeyNames() << "Display_Theme");
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Settings_Injector_MoodLightColor",
                           0,
                           "imaxeon",
                           "",
                           "list",
                           QVariantList() << "off" << "imaxeon" << "kipper" << "heartbeat",
                           QVariant(0),
                           QVariant(0),
                           QVariant(0),
                           Config::LegacyKeyNames() << "Device_MoodLightColor");
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Settings_Injector_PMReminderFrequency",
                           1,
                           "0",
                           "",
                           "list",
                           QVariantList() << "0" << "7" << "30" << "365" << "-1",
                           QVariant(0),
                           QVariant(0),
                           QVariant(0));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Settings_Injector_EmptyContrastFirst",
                           2,
                           false,
                           "",
                           "bool",
                           QVariantList(),
                           QVariant(0),
                           QVariant(0),
                           QVariant(0));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("ServiceCalibration_IADCalibrationMethod",
                           0,
                           "MIDPOINT",
                           "",
                           "list",
                           QVariantList() << "MIDPOINT" << "MIDRAIL",
                           QVariant(0),
                           QVariant(0),
                           QVariant(0));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Hidden_ScreenY",
                           0,
                           0,
                           "",
                           "int",
                           QVariantList(),
                           QVariant(0),
                           QVariant(0),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Hidden_ScreenX",
                           1,
                           0,
                           "",
                           "int",
                           QVariantList(),
                           QVariant(0),
                           QVariant(0),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Hidden_ScreenW",
                           2,
                           1920,
                           "",
                           "int",
                           QVariantList(),
                           QVariant(800),
                           QVariant(1920),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Hidden_ScreenH",
                           3,
                           1200,
                           "",
                           "int",
                           QVariantList(),
                           QVariant(600),
                           QVariant(1200),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Hidden_HcuIp",
                           4,
                           "192.168.11.1",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(0),
                           QVariant(0),
                           QVariant(0));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Hidden_HcuPort",
                           5,
                           4501,
                           "",
                           "int",
                           QVariantList(),
                           QVariant(1),
                           QVariant(9999),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Hidden_CruIpWired",
                           6,
                           "192.168.11.2",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(0),
                           QVariant(0),
                           QVariant(0));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Hidden_CruIpWireless",
                           7,
                           "192.168.11.3",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(0),
                           QVariant(0),
                           QVariant(0));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Hidden_CruPort",
                           8,
                           8901,
                           "",
                           "int",
                           QVariantList(),
                           QVariant(1),
                           QVariant(9999),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Hidden_CruRouterIp",
                           9,
                           "192.168.137.1",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(0),
                           QVariant(0),
                           QVariant(0));
    dbNew->set(cfgItem);

    QVariantList pressureCfgOptions;
    QVariantMap optionMap;
    optionMap.insert("Unit", "psi");
    optionMap.insert("Values", QVariantList() << "50" << "100" << "150" << "200" << "250" << "300");
    pressureCfgOptions.append(optionMap);
    optionMap.insert("Unit", "kpa");
    optionMap.insert("Values", QVariantList() << "345" << "689" << "1034" << "1379" << "1724" << "2068");
    pressureCfgOptions.append(optionMap);
    optionMap.insert("Unit", "kg/cm2");
    optionMap.insert("Values", QVariantList() << "3.5" << "7.0" << "10.5" << "14.1" << "17.6" << "21.1");
    pressureCfgOptions.append(optionMap);

    cfgItem = db->initItem("Hidden_PressureOptionValues",
                           10,
                           pressureCfgOptions,
                           "",
                           "list",
                           QVariantList(),
                           QVariant(0),
                           QVariant(0),
                           QVariant(0));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Hidden_CurrentUtcOffsetMinutes",
                           11,
                           0,
                           "",
                           "int",
                           QVariantList(),
                           QVariant(-3600),
                           QVariant(3600),
                           QVariant(30));
    dbNew->set(cfgItem);

    DS_WorkflowDef::MudsSodStatus mudsSodStatus;

    cfgItem = db->initItem("Hidden_MudsSodStatus",
                           12,
                           ImrParser::ToImr_MudsSodStatus(mudsSodStatus),
                           "",
                           "object",
                           QVariantList(),
                           QVariant(0),
                           QVariant(0),
                           QVariant(0));
    dbNew->set(cfgItem);

    DS_ExamDef::ExamAdvanceInfo examAdvanceInfo;
    cfgItem = db->initItem("Hidden_LastExamAdvanceInfo",
                           13,
                           ImrParser::ToImr_ExamAdvanceInfo(examAdvanceInfo),
                           "",
                           "object",
                           QVariantList(),
                           QVariant(0),
                           QVariant(0),
                           QVariant(0));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Hidden_BMSLastBatteryIdA",
                           14,
                           "0000-0000",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(0),
                           QVariant(10),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Hidden_BMSLastBatteryIdB",
                           15,
                           "0000-0000",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(0),
                           QVariant(10),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Hidden_BMSLastPFResetCycleCountA",
                           16,
                           0,
                           "",
                           "int",
                           QVariantList(),
                           QVariant(0),
                           QVariant(65535),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Hidden_BMSPFResetCountA",
                           17,
                           0,
                           "",
                           "int",
                           QVariantList(),
                           QVariant(0),
                           QVariant(65535),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Hidden_BMSLastPFResetCycleCountB",
                           18,
                           0,
                           "",
                           "int",
                           QVariantList(),
                           QVariant(0),
                           QVariant(65535),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Hidden_BMSPFResetCountB",
                           19,
                           0,
                           "",
                           "int",
                           QVariantList(),
                           QVariant(0),
                           QVariant(65535),
                           QVariant(1));
    dbNew->set(cfgItem);


    // Init CRU Fluid DB
    QVariant fluidDb;
    QFile fileFluidDb(QString(PATH_RESOURCES_DEFAULT_CONFIG) + "/DefaultFluidOptions.json");
    if (fileFluidDb.open(QFile::ReadOnly | QFile::Text))
    {
        QString jsonStr = fileFluidDb.readAll();
        fileFluidDb.close();

        QJsonParseError parseErr;
        QJsonDocument document = QJsonDocument::fromJson(jsonStr.toUtf8(), &parseErr);

        if (parseErr.error == QJsonParseError::NoError)
        {
            fluidDb = document.toVariant();
        }
        else
        {
            LOG_ERROR("Failed to load %s. ParseErr=%s\n", fileFluidDb.fileName().CSTR(), parseErr.errorString().CSTR());
        }
    }
    else
    {
        LOG_ERROR("Failed to open %s. Err=%s\n", fileFluidDb.fileName().CSTR(), fileFluidDb.errorString().CSTR());
    }

    cfgItem = db->initItem("Hidden_FluidOptions",
                           20,
                           fluidDb,
                           "",
                           "map",
                           QVariantList(),
                           QVariant(0),
                           QVariant(0),
                           QVariant(0));
    dbNew->set(cfgItem);


    // Init SRU Default Injection Plan Template
    QVariant defaultInjectionPlanTemplate;
    QFile fileInjectionPlanTemplate(QString(PATH_RESOURCES_DEFAULT_CONFIG) + "/DefaultInjectionPlanTemplate.json");

    if (fileInjectionPlanTemplate.open(QFile::ReadOnly | QFile::Text))
    {
        QString jsonStr = fileInjectionPlanTemplate.readAll();
        fileInjectionPlanTemplate.close();

        QJsonParseError parseErr;
        QJsonDocument document = QJsonDocument::fromJson(jsonStr.toUtf8(), &parseErr);
        if (parseErr.error == QJsonParseError::NoError)
        {
            defaultInjectionPlanTemplate = document.toVariant();
        }
        else
        {
            LOG_ERROR("Failed to load %s. ParseErr=%s\n", fileInjectionPlanTemplate.fileName().CSTR(), parseErr.errorString().CSTR());
        }
    }
    else
    {
        LOG_ERROR("Failed to open %s. Err=%s\n", fileInjectionPlanTemplate.fileName().CSTR(), fileInjectionPlanTemplate.errorString().CSTR());
    }

    cfgItem = db->initItem("Hidden_DefaultInjectionPlanTemplate",
                           21,
                           defaultInjectionPlanTemplate,
                           "",
                           "map",
                           QVariantList(),
                           QVariant(),
                           QVariant(),
                           QVariant());
    dbNew->set(cfgItem);



    // Init Injection Plan TemplateGroups
    cfgItem = db->initItem("Hidden_InjectionPlanTemplateGroups",
                           22,
                           QVariant(),
                           "",
                           "list",
                           QVariantList(),
                           QVariant(),
                           QVariant(),
                           QVariant());
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Hidden_PMLastPerformedAt",
                           23,
                           0,
                           "",
                           "int",
                           QVariantList(),
                           QVariant(0),
                           QVariant(INT_MAX),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Hidden_PMLastReminderAt",
                           24,
                           0,
                           "",
                           "int",
                           QVariantList(),
                           QVariant(0),
                           QVariant(INT_MAX),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Hidden_SyringesUsedInLastExam",
                           25,
                           QVariantList() << "false" << "false" << "false",
                           "",
                           "list",
                           QVariantList(),
                           QVariant(0),
                           QVariant(0),
                           QVariant(0));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Hidden_BottleFillCount",
                           0,
                           0,
                           "",
                           "int",
                           QVariantList(),
                           QVariant(0),
                           QVariant(INT_MAX),
                           QVariant(1));
    dbNew->set(cfgItem);

    // Set new db
    dbNew->setPathConfig(db->getPathConfig());
    dbNew->saveCfgFile();
    delete db;
    db = dbNew;
}

void DS_CfgLocal::slotAppStarted()
{
    // Register Config Set Callbacks
    connect(env->ds.examData, &DS_ExamData::signalDataChanged_InjectionPlanTemplateGroups, this, [=](const DS_ExamDef::InjectionPlanTemplateGroups &groups, const DS_ExamDef::InjectionPlanTemplateGroups &prevGroups) {
        // Check if Group Data is ready
        for (int groupIdx = 0; groupIdx < groups.length(); groupIdx++)
        {
            if (!groups[groupIdx].isReady())
            {
                // Group Data is still initialising.. Save later
                return;
            }
        }

        QString err;
        auto value = ImrParser::ToImr_InjectionPlanTemplateGroups(groups, &err);
        if (err == "")
        {
            env->ds.cfgLocal->set_Hidden_InjectionPlanTemplateGroups(value);
        }
        else
        {
            LOG_ERROR("Failed to parse InjectionPlanTemplateGroups. Err=%s\n", err.CSTR());
        }

       // Save Default Injection Plan Template
        for (int groupIdx = 0; groupIdx < groups.length(); groupIdx++)
        {
            DS_ExamDef::InjectionPlanTemplateGroup group = groups[groupIdx];
            const DS_ExamDef::InjectionPlanDigest *planDigest = group.getPlanDigestFromTemplateGuid(DEFAULT_INJECT_PLAN_TEMPLATE_GUID);

            if (planDigest != NULL)
            {
                QVariantMap value = ImrParser::ToImr_InjectionPlan(planDigest->plan, &err);
                if (err == "")
                {
                    env->ds.cfgLocal->set_Hidden_DefaultInjectionPlanTemplate(value);
                }
                else
                {
                    LOG_ERROR("Failed to parse DefaultInjectionPlanTemplate. Err=%s\n", err.CSTR());
                }
                break;
            }
        }
    });

    connect(env->ds.deviceData, &DS_DeviceData::signalDataChanged_FluidOptions, this, [=](const DS_DeviceDef::FluidOptions &fluidOptions) {
        QString err;
        DS_DeviceDef::FluidOptions fluidOptionsBuf = fluidOptions;

        if (fluidOptionsBuf.salinePackages.length() <= 0)
        {
            Config::Item cfg = env->ds.cfgLocal->getItem_Hidden_FluidOptions();
            // Check for any error with the Saline package and correct it
            DS_DeviceDef::FluidOptions defaultOptions = ImrParser::ToCpp_FluidOptions(cfg.defaultValue.toMap());
            fluidOptionsBuf.salinePackages = defaultOptions.salinePackages;
        }

        QVariantMap value = ImrParser::ToImr_FluidOptions(fluidOptionsBuf, &err);

        if (err == "")
        {
            env->ds.cfgLocal->set_Hidden_FluidOptions(value);

            DS_CruDef::CruLinkStatus linkStatus = env->ds.cruData->getCruLinkStatus();
            if (linkStatus.state == DS_CruDef::CRU_LINK_STATE_ACTIVE)
            {
                env->ds.cruAction->actPutFluids(fluidOptionsBuf);
            }
            else
            {
                // When link becomes active, CRU shall get the fluid option from HCU
            }
        }
        else
        {
            LOG_ERROR("Failed to parse FluidOptions. Err=%s\n", err.CSTR());
        }
    });

    connect(env->ds.workflowData, &DS_WorkflowData::signalDataChanged_MudsSodStatus, this, [=](const DS_WorkflowDef::MudsSodStatus &mudsSodStatus) {
        QString err;
        QVariantMap value = ImrParser::ToImr_MudsSodStatus(mudsSodStatus, &err);

        if (err == "")
        {
            env->ds.cfgLocal->set_Hidden_MudsSodStatus(value);
        }
        else
        {
            LOG_ERROR("Failed to parse MudsSodStatus. Err=%s\n", err.CSTR());
        }
    });

    connect(env->ds.examData, &DS_ExamData::signalDataChanged_ExamAdvanceInfo, this, [=](const DS_ExamDef::ExamAdvanceInfo &examAdvanceInfo) {
        QString err;
        QVariantMap value = ImrParser::ToImr_ExamAdvanceInfo(examAdvanceInfo, &err);

        if (err == "")
        {
            env->ds.cfgLocal->set_Hidden_LastExamAdvanceInfo(value);
        }
        else
        {
            LOG_ERROR("Failed to parse ExamAdvanceInfo. Err=%s\n", err.CSTR());
        }
    });

    connect(env->ds.workflowData, &DS_WorkflowData::signalDataChanged_SyringesUsedInLastExam, this, [=](const QList<bool> &syringesUsedInLastExam) {
        QList<QVariant> cfgList;
        for (int i = 0; i < syringesUsedInLastExam.length(); i++)
        {
            cfgList.append(syringesUsedInLastExam[i]);
        }
        env->ds.cfgLocal->set_Hidden_SyringesUsedInLastExam(cfgList);
    });

    emitAllConfigChanged();
}

void DS_CfgLocal::emitAllConfigChanged()
{
    EMIT_CONFIG_CHANGED_SIGNAL(Hidden_ScreenY)
    EMIT_CONFIG_CHANGED_SIGNAL(Hidden_ScreenX)
    EMIT_CONFIG_CHANGED_SIGNAL(Hidden_ScreenW)
    EMIT_CONFIG_CHANGED_SIGNAL(Hidden_ScreenH)
    EMIT_CONFIG_CHANGED_SIGNAL(Hidden_HcuIp)
    EMIT_CONFIG_CHANGED_SIGNAL(Hidden_HcuPort)
    EMIT_CONFIG_CHANGED_SIGNAL(Hidden_CruIpWired)
    EMIT_CONFIG_CHANGED_SIGNAL(Hidden_CruIpWireless)
    EMIT_CONFIG_CHANGED_SIGNAL(Hidden_CruPort)
    EMIT_CONFIG_CHANGED_SIGNAL(Hidden_CruRouterIp)
    EMIT_CONFIG_CHANGED_SIGNAL(Hidden_FluidOptions)
    EMIT_CONFIG_CHANGED_SIGNAL(Hidden_InjectionPlanTemplateGroups)
    EMIT_CONFIG_CHANGED_SIGNAL(Hidden_DefaultInjectionPlanTemplate)
    EMIT_CONFIG_CHANGED_SIGNAL(Hidden_PressureOptionValues)
    EMIT_CONFIG_CHANGED_SIGNAL(Hidden_CurrentUtcOffsetMinutes)
    EMIT_CONFIG_CHANGED_SIGNAL(Hidden_MudsSodStatus)
    EMIT_CONFIG_CHANGED_SIGNAL(Hidden_LastExamAdvanceInfo)
    EMIT_CONFIG_CHANGED_SIGNAL(Hidden_BMSLastBatteryIdA)
    EMIT_CONFIG_CHANGED_SIGNAL(Hidden_BMSLastBatteryIdB)
    EMIT_CONFIG_CHANGED_SIGNAL(Hidden_BMSLastPFResetCycleCountA)
    EMIT_CONFIG_CHANGED_SIGNAL(Hidden_BMSPFResetCountA)
    EMIT_CONFIG_CHANGED_SIGNAL(Hidden_BMSLastPFResetCycleCountB)
    EMIT_CONFIG_CHANGED_SIGNAL(Hidden_BMSPFResetCountB)
    EMIT_CONFIG_CHANGED_SIGNAL(Hidden_PMLastPerformedAt)
    EMIT_CONFIG_CHANGED_SIGNAL(Hidden_PMLastReminderAt)
    EMIT_CONFIG_CHANGED_SIGNAL(Hidden_SyringesUsedInLastExam)
    EMIT_CONFIG_CHANGED_SIGNAL(Hidden_BottleFillCount)
    EMIT_CONFIG_CHANGED_SIGNAL(Settings_SruLink_ConnectionType)
    EMIT_CONFIG_CHANGED_SIGNAL(Settings_Sound_NormalAudioLevel)
    EMIT_CONFIG_CHANGED_SIGNAL(Settings_Sound_InjectionAudioLevel)
    EMIT_CONFIG_CHANGED_SIGNAL(Settings_Sound_SudsPrimedAudioLevel)
    EMIT_CONFIG_CHANGED_SIGNAL(Settings_Sound_NotificationAudioLevel)
    EMIT_CONFIG_CHANGED_SIGNAL(Settings_Sound_KeyClicksEnabled)
    EMIT_CONFIG_CHANGED_SIGNAL(Settings_Display_ScreenBrightness)
    EMIT_CONFIG_CHANGED_SIGNAL(Settings_Display_ScreenOffTimeoutMinutes)
    EMIT_CONFIG_CHANGED_SIGNAL(Settings_Display_Theme)
    EMIT_CONFIG_CHANGED_SIGNAL(Settings_Injector_MoodLightColor)
    EMIT_CONFIG_CHANGED_SIGNAL(Settings_Injector_PMReminderFrequency)
    EMIT_CONFIG_CHANGED_SIGNAL(Settings_Injector_EmptyContrastFirst)
    EMIT_CONFIG_CHANGED_SIGNAL(ServiceCalibration_IADCalibrationMethod)

    emit signalConfigChanged();
}

QVariantMap DS_CfgLocal::getConfigs(QString *err)
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

void DS_CfgLocal::setConfigs(QVariantMap configs, bool setChangedAt, QString *errOut)
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

        SET_CONFIG_BY_NAME(item, Hidden_ScreenY, setChangedAt)
        SET_CONFIG_BY_NAME(item, Hidden_ScreenX, setChangedAt)
        SET_CONFIG_BY_NAME(item, Hidden_ScreenW, setChangedAt)
        SET_CONFIG_BY_NAME(item, Hidden_ScreenH, setChangedAt)
        SET_CONFIG_BY_NAME(item, Hidden_HcuIp, setChangedAt)
        SET_CONFIG_BY_NAME(item, Hidden_HcuPort, setChangedAt)
        SET_CONFIG_BY_NAME(item, Hidden_CruIpWired, setChangedAt)
        SET_CONFIG_BY_NAME(item, Hidden_CruIpWireless, setChangedAt)
        SET_CONFIG_BY_NAME(item, Hidden_CruPort, setChangedAt)
        SET_CONFIG_BY_NAME(item, Hidden_CruRouterIp, setChangedAt)
        SET_CONFIG_BY_NAME(item, Hidden_FluidOptions, setChangedAt)
        SET_CONFIG_BY_NAME(item, Hidden_InjectionPlanTemplateGroups, setChangedAt)
        SET_CONFIG_BY_NAME(item, Hidden_DefaultInjectionPlanTemplate, setChangedAt)
        SET_CONFIG_BY_NAME(item, Hidden_PressureOptionValues, setChangedAt)
        SET_CONFIG_BY_NAME(item, Hidden_CurrentUtcOffsetMinutes, setChangedAt)
        SET_CONFIG_BY_NAME(item, Hidden_MudsSodStatus, setChangedAt)
        SET_CONFIG_BY_NAME(item, Hidden_LastExamAdvanceInfo, setChangedAt)
        SET_CONFIG_BY_NAME(item, Hidden_BMSLastBatteryIdA, setChangedAt)
        SET_CONFIG_BY_NAME(item, Hidden_BMSLastBatteryIdB, setChangedAt)
        SET_CONFIG_BY_NAME(item, Hidden_BMSLastPFResetCycleCountA, setChangedAt)
        SET_CONFIG_BY_NAME(item, Hidden_BMSPFResetCountA, setChangedAt)
        SET_CONFIG_BY_NAME(item, Hidden_BMSLastPFResetCycleCountB, setChangedAt)
        SET_CONFIG_BY_NAME(item, Hidden_BMSPFResetCountB, setChangedAt)
        SET_CONFIG_BY_NAME(item, Hidden_PMLastPerformedAt, setChangedAt)
        SET_CONFIG_BY_NAME(item, Hidden_PMLastReminderAt, setChangedAt)
        SET_CONFIG_BY_NAME(item, Hidden_SyringesUsedInLastExam, setChangedAt)
        SET_CONFIG_BY_NAME(item, Hidden_BottleFillCount, setChangedAt)
        SET_CONFIG_BY_NAME(item, Settings_SruLink_ConnectionType, setChangedAt)
        SET_CONFIG_BY_NAME(item, Settings_Sound_NormalAudioLevel, setChangedAt)
        SET_CONFIG_BY_NAME(item, Settings_Sound_InjectionAudioLevel, setChangedAt)
        SET_CONFIG_BY_NAME(item, Settings_Sound_SudsPrimedAudioLevel, setChangedAt)
        SET_CONFIG_BY_NAME(item, Settings_Sound_NotificationAudioLevel, setChangedAt)
        SET_CONFIG_BY_NAME(item, Settings_Sound_KeyClicksEnabled, setChangedAt)
        SET_CONFIG_BY_NAME(item, Settings_Display_ScreenBrightness, setChangedAt)
        SET_CONFIG_BY_NAME(item, Settings_Display_ScreenOffTimeoutMinutes, setChangedAt)
        SET_CONFIG_BY_NAME(item, Settings_Display_Theme, setChangedAt)
        SET_CONFIG_BY_NAME(item, Settings_Injector_MoodLightColor, setChangedAt)
        SET_CONFIG_BY_NAME(item, Settings_Injector_PMReminderFrequency, setChangedAt)
        SET_CONFIG_BY_NAME(item, Settings_Injector_EmptyContrastFirst, setChangedAt)
        SET_CONFIG_BY_NAME(item, ServiceCalibration_IADCalibrationMethod, setChangedAt)
    }

    if (oldCfgMap != db->getCfgMap())
    {
        emit signalConfigChanged();
    }

    *errOut = err;
}

