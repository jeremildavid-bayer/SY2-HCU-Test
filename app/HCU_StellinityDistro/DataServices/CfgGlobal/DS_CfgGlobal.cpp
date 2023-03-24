#include "Apps/AppManager.h"
#include "DS_CfgGlobal.h"
#include "Common/ImrParser.h"
#include "DataServices/Alert/DS_AlertAction.h"
#include "DataServices/System/DS_SystemData.h"

DS_CfgGlobal::DS_CfgGlobal(QObject *parent, EnvGlobal *env_) :
    QObject(parent),
    env(env_)
{
    envLocal = new EnvLocal("DS_CfgGlobal", "CFG_GLOBAL", LOG_MID_SIZE_BYTES);

    QString dbFileName = QString(PATH_CONFIG_GLOBAL) + ((env->ds.systemData->getBuildType() == BUILD_TYPE_PROD) ? ".PROD" : "");
    db = new Config(dbFileName, env, envLocal);

    dbLoadFailed = true;

    if (!db->isCfgOk())
    {
        db->initCfgFile();
        dbLoadFailed = false;
    }

    connect(env->appManager, &AppManager::signalAppStarted, this, [=]() {
        slotAppStarted();
    });

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });

    initConfig();
}

DS_CfgGlobal::~DS_CfgGlobal()
{
    delete db;
    delete envLocal;
}

void DS_CfgGlobal::initConfig()
{
    Config::Item cfgItem;
    Config *dbNew = new Config("", env, envLocal);

    QString buildType = env->ds.systemData->getBuildType();

    cfgItem = db->initItem("Hidden_ProductSoftwareVersion",
                           2,
                           "",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(0),
                           QVariant(50),
                           QVariant(1));
    dbNew->set(cfgItem);

    //This list will show the language list. sort by country code is better than random thing

    QVariantList phraseOptions = QVariantList() << "cd-XX" << "cs-CZ" << "da-DK" << "db-XX" << "de-CH" << "de-DE" << "el-GR" << "en-AU" << "en-GB"
                                                << "en-US" << "es-ES" << "es-MX" << "et-EE" << "fi-FI" << "fr-BE" << "fr-CA" << "fr-CH" << "fr-FR" << "hu-HU"
                                                << "id-ID" << "it-CH" << "it-IT"
                                                << "ja-JP" << "kk-KZ"<< "ko-KR"
                                                << "lp-XX" << "nb-NO" << "nl-BE" << "nl-NL" << "pl-PL" << "pt-BR" << "pt-PT"
                                                << "ru-RU" << "sv-SE" << "sl-SI" << "sk-SK"
                                                << "th-TH" << "tr-TR"
                                                << "vi-VN"
                                                << "zh-CN" << "zh-TW";


    if ( (buildType == BUILD_TYPE_VNV) ||
         (buildType == BUILD_TYPE_REL) )
    {
        phraseOptions.removeOne("cd-XX");
        phraseOptions.removeOne("db-XX");
        phraseOptions.removeOne("lp-XX");
    }

    cfgItem = db->initItem("Settings_General_CultureCode",
                           1,
                           "en-US",
                           "",
                           "list",
                           phraseOptions,
                           QVariant(0),
                           QVariant(0),
                           QVariant(0),
                           Config::LegacyKeyNames() << "General_CultureCode");
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Settings_General_DateFormat",
                           2,
                           "yyyy/MM/dd",
                           "",
                           "list",
                           QVariantList() << "MM/dd/yyyy" << "dd/MM/yyyy" << "yyyy/MM/dd",
                           QVariant(0),
                           QVariant(0),
                           QVariant(0),
                           Config::LegacyKeyNames() << "General_DateFormat");
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Settings_General_TimeFormat",
                           3,
                           "HH:mm",
                           "",
                           "list",
                           QVariantList() << "HH:mm" << "hh:mm tt",
                           QVariant(0),
                           QVariant(0),
                           QVariant(0),
                           Config::LegacyKeyNames() << "General_TimeFormat");
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Settings_General_PressureUnit",
                           5,
                           "kpa",
                           "",
                           "list",
                           QVariantList() << "psi" << "kpa" << "kg/cm2",
                           QVariant(0),
                           QVariant(0),
                           QVariant(0),
                           Config::LegacyKeyNames() << "General_PressureUnit");
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Settings_General_WeightUnit",
                           6,
                           "kg",
                           "",
                           "list",
                           QVariantList() << "kg" << "lb",
                           QVariant(0),
                           QVariant(0),
                           QVariant(0),
                           Config::LegacyKeyNames() << "General_WeightUnit");
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Settings_General_HeightUnit",
                           7,
                           "m",
                           "",
                           "list",
                           QVariantList() << "m" << "cm" << "in",
                           QVariant(0),
                           QVariant(0),
                           QVariant(0),
                           Config::LegacyKeyNames() << "General_HeightUnit");
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Settings_General_TemperatureUnit",
                           8,
                           "degreesC",
                           "",
                           "list",
                           QVariantList() << "degreesC" << "degreesF",
                           QVariant(0),
                           QVariant(0),
                           QVariant(0),
                           Config::LegacyKeyNames() << "General_TemperatureUnit");
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Settings_SruLink_WirelessCountryCode",
                           2,
                           "DE-276",
                           "",
                           "list",
                           QVariantList() << "DE-276" << "US-841" << "JP-4015",
                           QVariant(0),
                           QVariant(0),
                           QVariant(0),
                           Config::LegacyKeyNames() << "Networking_WirelessCountryCode");
    dbNew->set(cfgItem);

    cfgItem = db->initItem("AccessControl_Settings_AccessPasscode",
                           0,
                           "",
                           "",
                           "passcode",
                           QVariantList(),
                           QVariant(0),
                           QVariant(8),
                           QVariant(1),
                           Config::LegacyKeyNames() << "Admin_AccessPasscode");
    dbNew->set(cfgItem);

    cfgItem = db->initItem("AccessControl_Settings_AccessProtected",
                           1,
                           "{\"AccessControl\":false"
                           ",\"Protocols\":false"
                           ",\"Logbook\":false"
                           ",\"Reports\":false"
                           ",\"Settings\":false"
                           ",\"Setup\":false"
                           ",\"Networking\":false"
                           ",\"System\":false"
                           ",\"Contrasts\":false"
                           ",\"Catheters\":false"
                           ",\"InjectionSites\":false"
                           ",\"EgfrSetup\":false"
                           ",\"KvpRuleSets\":false"
                           ",\"MwlSetup\":false"
                           ",\"PACS\":false"
                           ",\"MIRTH\":false"
                           ",\"LDAP\":false"
                           ",\"SMTP\":false"
                           ",\"Lan1Ris\":false"
                           ",\"ProxySettings\":false"
                           ",\"Configuration\":false"
                           ",\"ImportExport\":false"
                           ",\"BackupRestore\":false"
                           ",\"DateTime\":false"
                           "}",
                           "",
                           "object",
                           QVariantList(),
                           QVariant(0),
                           QVariant(0),
                           QVariant(0),
                           Config::LegacyKeyNames() << "Admin_AccessProtected");
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Configuration_Exam_MandatoryFields",
                           2,
                           "{\"PatientHeight\":false"
                           ",\"PatientWeight\":false"
                           ",\"CatheterType\":false"
                           ",\"CatheterPlacement\":false"
                           ",\"CatheterPlacedBy\":false"
                           ",\"EgfrValue\":false"
                           ",\"ContrastLotBatch\":false"
                           ",\"ContrastExpiration\":false"
                           ",\"SalineLotBatch\":false"
                           ",\"SalineExpiration\":false"
                           ",\"ExamNotes\":false"
                           ",\"TechnologistId\":false"
                           "}",
                           "",
                           "object",
                           QVariantList(),
                           QVariant(0),
                           QVariant(0),
                           QVariant(0),
                           Config::LegacyKeyNames() << "Admin_MandatoryFields");
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Configuration_Exam_QuickNotes",
                           3,
                           "[]",
                           "",
                           "object",
                           QVariantList(),
                           QVariant(0),
                           QVariant(0),
                           QVariant(0),
                           Config::LegacyKeyNames() << "Advanced_QuickNotes");
    dbNew->set(cfgItem);


    cfgItem = db->initItem("Configuration_Exam_ExamTimeoutPeriod",
                           5,
                           3,
                           "hours",
                           "int",
                           QVariantList(),
                           QVariant(0),
                           QVariant(10),
                           QVariant(1),
                           Config::LegacyKeyNames() << "Exam_ExamTimeoutPeriod");
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Settings_Injector_HeatMaintainerEnabled",
                           100,
                           true,
                           "",
                           "bool",
                           QVariantList(),
                           QVariant(0),
                           QVariant(0),
                           QVariant(0),
                           Config::LegacyKeyNames() << "Device_HeatMaintainerEnabled");
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Configuration_UseLife_MUDSUseLifeLimitEnforced",
                           101,
                           false,
                           "",
                           "bool",
                           QVariantList(),
                           QVariant(0),
                           QVariant(0),
                           QVariant(0),
                           Config::LegacyKeyNames() << "Admin_MUDSUseLifeLimitEnforced");
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Configuration_UseLife_MUDSUseLifeLimitHours",
                           102,
                           24,
                           "hours",
                           "int",
                           QVariantList(),
                           QVariant(1),
                           QVariant(24),
                           QVariant(1),
                           Config::LegacyKeyNames() << "Admin_MUDSUseLifeLimitHours");
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Configuration_UseLife_MUDSUseLifeGraceHours",
                           103,
                           1,
                           "hours",
                           "int",
                           QVariantList(),
                           QVariant(1),
                           QVariant(24),
                           QVariant(1),
                           Config::LegacyKeyNames() << "Admin_MUDSUseLifeGraceHours");
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Configuration_UseLife_SalineMaximumUseDuration",
                           104,
                           0,
                           "hours",
                           "int",
                           QVariantList(),
                           QVariant(0),
                           QVariant(168),
                           QVariant(1),
                           Config::LegacyKeyNames() << "Admin_SalineMaximumUseDuration");
    dbNew->set(cfgItem);   

    // This is intentionally empty since all data should be received from CRU side and filled
    cfgItem = db->initItem("Configuration_Behaviors_Country",
                           6,
                           "",
                           "",
                           "list",
                           QVariantList(),
                           QVariant(0),
                           QVariant(0),
                           QVariant(0));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Configuration_Behaviors_AutoEmptySalineEnabled",
                           105,
                           false,
                           "",
                           "bool",
                           QVariantList(),
                           QVariant(0),
                           QVariant(0),
                           QVariant(0));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Configuration_Behaviors_AutoEmptyContrast1Enabled",
                           106,
                           false,
                           "",
                           "bool",
                           QVariantList(),
                           QVariant(0),
                           QVariant(0),
                           QVariant(0));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Configuration_Behaviors_AutoEmptyContrast2Enabled",
                           107,
                           false,
                           "",
                           "bool",
                           QVariantList(),
                           QVariant(0),
                           QVariant(0),
                           QVariant(0));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Configuration_Behaviors_MaximumFlowRateReduction",
                           108,
                           40,
                           "%",
                           "int",
                           QVariantList(),
                           QVariant(0),
                           QVariant(50),
                           QVariant(1),
                           Config::LegacyKeyNames() << "Device_MaximumFlowRateReduction");
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Configuration_Behaviors_PressureLimitSensitivity",
                           109,
                           "75",
                           "",
                           "list",
                           QVariantList() << "100" << "75" << "50" << "25",
                           QVariant(0),
                           QVariant(0),
                           QVariant(0),
                           Config::LegacyKeyNames() << "Device_PressureLimitSensitivity");
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Configuration_Behaviors_ExtendedSUDSAvailable",
                           110,
                           false,
                           "",
                           "bool",
                           QVariantList(),
                           QVariant(0),
                           QVariant(0),
                           QVariant(0));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Configuration_Behaviors_DefaultSUDSLength",
                           111,
                           SUDS_LENGTH_NORMAL,
                           "",
                           "list",
                           QVariantList() << SUDS_LENGTH_NORMAL << SUDS_LENGTH_EXTENDED,
                           QVariant(0),
                           QVariant(0),
                           QVariant(0));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Configuration_Behaviors_ChangeContrastEnabled",
                           112,
                           false,
                           "",
                           "bool",
                           QVariantList(),
                           QVariant(0),
                           QVariant(0),
                           QVariant(0));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Service_ServicePasscode",
                           0,
                           "8845",
                           "",
                           "passcode",
                           QVariantList(),
                           QVariant(4),
                           QVariant(8),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Service_ContactServiceTelephone",
                           1,
                           "",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(0),
                           QVariant(16),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Service_TradeshowModeEnabled",
                           2,
                           false,
                           "",
                           "bool",
                           QVariantList(),
                           QVariant(0),
                           QVariant(0),
                           QVariant(0));
    dbNew->set(cfgItem);

    // Set new db
    dbNew->setPathConfig(db->getPathConfig());
    dbNew->saveCfgFile();
    delete db;
    db = dbNew;
}

void DS_CfgGlobal::slotAppStarted()
{
    emitAllConfigChanged();

    if ( (!dbLoadFailed) &&
         (env->ds.systemData->getBuildType() != BUILD_TYPE_PROD) )
    {
        env->ds.alertAction->activate("ConfigsReset");
    }
}

void DS_CfgGlobal::slotAppInitialised()
{
}

void DS_CfgGlobal::emitAllConfigChanged()
{
    EMIT_CONFIG_CHANGED_SIGNAL(Hidden_ProductSoftwareVersion)

    EMIT_CONFIG_CHANGED_SIGNAL(Settings_General_CultureCode)
    EMIT_CONFIG_CHANGED_SIGNAL(Settings_General_DateFormat)
    EMIT_CONFIG_CHANGED_SIGNAL(Settings_General_TimeFormat)
    EMIT_CONFIG_CHANGED_SIGNAL(Settings_General_PressureUnit)
    EMIT_CONFIG_CHANGED_SIGNAL(Settings_General_WeightUnit)
    EMIT_CONFIG_CHANGED_SIGNAL(Settings_General_HeightUnit)
    EMIT_CONFIG_CHANGED_SIGNAL(Settings_General_TemperatureUnit)
    EMIT_CONFIG_CHANGED_SIGNAL(Settings_SruLink_WirelessCountryCode)
    EMIT_CONFIG_CHANGED_SIGNAL(Settings_Injector_HeatMaintainerEnabled)

    EMIT_CONFIG_CHANGED_SIGNAL(AccessControl_Settings_AccessPasscode)
    EMIT_CONFIG_CHANGED_SIGNAL(AccessControl_Settings_AccessProtected)
    EMIT_CONFIG_CHANGED_SIGNAL(Configuration_Exam_MandatoryFields)
    EMIT_CONFIG_CHANGED_SIGNAL(Configuration_Exam_QuickNotes)
    EMIT_CONFIG_CHANGED_SIGNAL(Configuration_Exam_ExamTimeoutPeriod)
    EMIT_CONFIG_CHANGED_SIGNAL(Configuration_UseLife_MUDSUseLifeLimitEnforced)
    EMIT_CONFIG_CHANGED_SIGNAL(Configuration_UseLife_MUDSUseLifeLimitHours)
    EMIT_CONFIG_CHANGED_SIGNAL(Configuration_UseLife_MUDSUseLifeGraceHours)
    EMIT_CONFIG_CHANGED_SIGNAL(Configuration_UseLife_SalineMaximumUseDuration)
    EMIT_CONFIG_CHANGED_SIGNAL(Configuration_Behaviors_Country)
    EMIT_CONFIG_CHANGED_SIGNAL(Configuration_Behaviors_AutoEmptySalineEnabled)
    EMIT_CONFIG_CHANGED_SIGNAL(Configuration_Behaviors_AutoEmptyContrast1Enabled)
    EMIT_CONFIG_CHANGED_SIGNAL(Configuration_Behaviors_AutoEmptyContrast2Enabled)
    EMIT_CONFIG_CHANGED_SIGNAL(Configuration_Behaviors_MaximumFlowRateReduction)
    EMIT_CONFIG_CHANGED_SIGNAL(Configuration_Behaviors_PressureLimitSensitivity)
    EMIT_CONFIG_CHANGED_SIGNAL(Configuration_Behaviors_ExtendedSUDSAvailable)
    EMIT_CONFIG_CHANGED_SIGNAL(Configuration_Behaviors_DefaultSUDSLength)
    EMIT_CONFIG_CHANGED_SIGNAL(Configuration_Behaviors_ChangeContrastEnabled)

    EMIT_CONFIG_CHANGED_SIGNAL(Service_ServicePasscode)
    EMIT_CONFIG_CHANGED_SIGNAL(Service_ContactServiceTelephone)
    EMIT_CONFIG_CHANGED_SIGNAL(Service_TradeshowModeEnabled)

    emit signalConfigChanged();
}

void DS_CfgGlobal::verifyConfigs(QVariantMap configs,  QString *errOut)
{
    QString err;
    QList<QString> listName = configs.keys();
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

        if (!Config::verifyItemValue(item))
        {
            err = QString().asprintf("Config Item Verification Failed (%s)\n", Util::qVarientToJsonData(map, false).CSTR());
            break;

        }
    }

    *errOut = err;
}

QVariantMap DS_CfgGlobal::getConfigs(QString *err)
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

void DS_CfgGlobal::setConfigs(QVariantMap configs, bool setChangedAt, QString *errOut)
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

        SET_CONFIG_BY_NAME(item, Hidden_ProductSoftwareVersion, setChangedAt)

        SET_CONFIG_BY_NAME(item, Settings_General_CultureCode, setChangedAt)
        SET_CONFIG_BY_NAME(item, Settings_General_DateFormat, setChangedAt)
        SET_CONFIG_BY_NAME(item, Settings_General_TimeFormat, setChangedAt)
        SET_CONFIG_BY_NAME(item, Settings_General_PressureUnit, setChangedAt)
        SET_CONFIG_BY_NAME(item, Settings_General_WeightUnit, setChangedAt)
        SET_CONFIG_BY_NAME(item, Settings_General_HeightUnit, setChangedAt)
        SET_CONFIG_BY_NAME(item, Settings_General_TemperatureUnit, setChangedAt)

        SET_CONFIG_BY_NAME(item, Settings_SruLink_WirelessCountryCode, setChangedAt)

        SET_CONFIG_BY_NAME(item, Settings_Injector_HeatMaintainerEnabled, setChangedAt)

        SET_CONFIG_BY_NAME(item, AccessControl_Settings_AccessPasscode, setChangedAt)
        SET_CONFIG_BY_NAME(item, AccessControl_Settings_AccessProtected, setChangedAt)
        SET_CONFIG_BY_NAME(item, Configuration_Exam_MandatoryFields, setChangedAt)
        SET_CONFIG_BY_NAME(item, Configuration_Exam_QuickNotes, setChangedAt)
        SET_CONFIG_BY_NAME(item, Configuration_Exam_ExamTimeoutPeriod, setChangedAt)
        SET_CONFIG_BY_NAME(item, Configuration_UseLife_MUDSUseLifeLimitEnforced, setChangedAt)
        SET_CONFIG_BY_NAME(item, Configuration_UseLife_MUDSUseLifeLimitHours, setChangedAt)
        SET_CONFIG_BY_NAME(item, Configuration_UseLife_MUDSUseLifeGraceHours, setChangedAt)
        SET_CONFIG_BY_NAME(item, Configuration_UseLife_SalineMaximumUseDuration, setChangedAt)
        SET_CONFIG_BY_NAME(item, Configuration_Behaviors_Country, setChangedAt)
        SET_CONFIG_BY_NAME(item, Configuration_Behaviors_AutoEmptySalineEnabled, setChangedAt)
        SET_CONFIG_BY_NAME(item, Configuration_Behaviors_AutoEmptyContrast1Enabled, setChangedAt)
        SET_CONFIG_BY_NAME(item, Configuration_Behaviors_AutoEmptyContrast2Enabled, setChangedAt)
        SET_CONFIG_BY_NAME(item, Configuration_Behaviors_MaximumFlowRateReduction, setChangedAt)
        SET_CONFIG_BY_NAME(item, Configuration_Behaviors_PressureLimitSensitivity, setChangedAt)
        SET_CONFIG_BY_NAME(item, Configuration_Behaviors_ExtendedSUDSAvailable, setChangedAt)
        SET_CONFIG_BY_NAME(item, Configuration_Behaviors_DefaultSUDSLength, setChangedAt)
        SET_CONFIG_BY_NAME(item, Configuration_Behaviors_ChangeContrastEnabled, setChangedAt)

        SET_CONFIG_BY_NAME(item, Service_ServicePasscode, setChangedAt)
        SET_CONFIG_BY_NAME(item, Service_ContactServiceTelephone, setChangedAt)
        SET_CONFIG_BY_NAME(item, Service_TradeshowModeEnabled, setChangedAt)
    }

    if (oldCfgMap != db->getCfgMap())
    {
        emit signalConfigChanged();
    }

    *errOut = err;
}

bool DS_CfgGlobal::isAutoEmptyEnabled(SyringeIdx idx)
{
    // Get index of the syringes to empty
    if ((idx == SYRINGE_IDX_SALINE) && get_Configuration_Behaviors_AutoEmptySalineEnabled())
        return true;
    if ((idx == SYRINGE_IDX_CONTRAST1) && get_Configuration_Behaviors_AutoEmptyContrast1Enabled())
        return true;
    if ((idx == SYRINGE_IDX_CONTRAST2) && get_Configuration_Behaviors_AutoEmptyContrast2Enabled())
        return true;
    return false;
}
