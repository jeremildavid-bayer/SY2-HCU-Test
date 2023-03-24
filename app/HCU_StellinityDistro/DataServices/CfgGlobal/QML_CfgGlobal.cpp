#include "QML_CfgGlobal.h"
#include "DataServices/CfgGlobal/DS_CfgGlobal.h"
#include "Common/Util.h"
#include "Common/ImrParser.h"

QML_CfgGlobal::QML_CfgGlobal(QObject *parent, EnvGlobal *env_) :
    QObject(parent),
    env(env_)
{
    envLocal = new EnvLocal("QML_CfgGlobal", "QML_CFG_GLOBAL");
    qmlSrc = env->qml.object->findChild<QObject*>("dsCfgGlobal");
    env->qml.engine->rootContext()->setContextProperty("dsCfgGlobalCpp", this);

    if (qmlSrc == NULL)
    {
        LOG_ERROR("Failed to assign qmlSrc.\n");
    }

    // Register Data Callbacks
    connect(env->ds.cfgGlobal, &DS_CfgGlobal::signalConfigChanged_Hidden_ProductSoftwareVersion, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("productSoftwareVersion", cfg.value);
    });    

    connect(env->ds.cfgGlobal, &DS_CfgGlobal::signalConfigChanged_Settings_General_CultureCode, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("cultureCode", cfg.value);
    });

    connect(env->ds.cfgGlobal, &DS_CfgGlobal::signalConfigChanged_Settings_General_DateFormat, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("dateFormat", cfg.value);
    });

    connect(env->ds.cfgGlobal, &DS_CfgGlobal::signalConfigChanged_Settings_General_TimeFormat, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("timeFormat", cfg.value);
    });

    connect(env->ds.cfgGlobal, &DS_CfgGlobal::signalConfigChanged_Settings_General_PressureUnit, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("pressureUnit", cfg.value);
    });

    connect(env->ds.cfgGlobal, &DS_CfgGlobal::signalConfigChanged_Settings_General_TemperatureUnit, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("temperatureUnit", cfg.value);
    });

    connect(env->ds.cfgGlobal, &DS_CfgGlobal::signalConfigChanged_AccessControl_Settings_AccessPasscode, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("accessCode", cfg.value);
    });

    connect(env->ds.cfgGlobal, &DS_CfgGlobal::signalConfigChanged_AccessControl_Settings_AccessProtected, this, [=](const Config::Item &cfg) {
        QJsonDocument document = QJsonDocument::fromJson(cfg.value.toString().toUtf8());
        QVariantMap accessProtected = document.toVariant().toMap();
        qmlSrc->setProperty("accessProtected", accessProtected);
    });


    connect(env->ds.cfgGlobal, &DS_CfgGlobal::signalConfigChanged_Configuration_Exam_MandatoryFields, this, [=](const Config::Item &cfg) {
        QJsonDocument document = QJsonDocument::fromJson(cfg.value.toString().toUtf8());
        QVariantMap mandatoryFields = document.toVariant().toMap();
        qmlSrc->setProperty("mandatoryFields", mandatoryFields);
    });

    connect(env->ds.cfgGlobal, &DS_CfgGlobal::signalConfigChanged_Configuration_Exam_QuickNotes, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("quickNotes", cfg.value);
    });

    connect(env->ds.cfgGlobal, &DS_CfgGlobal::signalConfigChanged_Configuration_UseLife_MUDSUseLifeLimitHours, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("mudsUseLifeLimitHours", cfg.value);
    });

    connect(env->ds.cfgGlobal, &DS_CfgGlobal::signalConfigChanged_Service_ServicePasscode, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("servicePasscode", cfg.value);
    });

    connect(env->ds.cfgGlobal, &DS_CfgGlobal::signalConfigChanged_Service_ContactServiceTelephone, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("serviceContactTelephone", cfg.value);
    });

    connect(env->ds.cfgGlobal, &DS_CfgGlobal::signalConfigChanged_Service_TradeshowModeEnabled, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("tradeshowModeEnabled", cfg.value);
    });
    
    connect(env->ds.cfgGlobal, &DS_CfgGlobal::signalConfigChanged_Configuration_Behaviors_ExtendedSUDSAvailable, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("extendedSUDSAvailable", cfg.value);
    });

    connect(env->ds.cfgGlobal, &DS_CfgGlobal::signalConfigChanged_Configuration_Behaviors_DefaultSUDSLength, this, [=](const Config::Item &cfg) {
        // we don't need to send defaultSUDSLength to qml since it's all controlled on cpp side
        // all we are interested in is the available list
        qmlSrc->setProperty("availableSUDSLengthOptions", cfg.validList);
    });

    connect(env->ds.cfgGlobal, &DS_CfgGlobal::signalConfigChanged_Configuration_Behaviors_AutoEmptySalineEnabled, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("autoEmptySalineEnabled", cfg.value);
    });

    connect(env->ds.cfgGlobal, &DS_CfgGlobal::signalConfigChanged_Configuration_Behaviors_AutoEmptyContrast1Enabled, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("autoEmptyContrast1Enabled", cfg.value);
    });

    connect(env->ds.cfgGlobal, &DS_CfgGlobal::signalConfigChanged_Configuration_Behaviors_AutoEmptyContrast2Enabled, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("autoEmptyContrast2Enabled", cfg.value);
    });

    connect(env->ds.cfgGlobal, &DS_CfgGlobal::signalConfigChanged_Configuration_Behaviors_ChangeContrastEnabled, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("changeContrastEnabled", cfg.value);
    });


    connect(env->ds.cfgGlobal, &DS_CfgGlobal::signalConfigChanged, this, [=](){
        QString err;
        QVariantMap map = env->ds.cfgGlobal->getConfigs(&err);

        if (err == "")
        {
            QVariantList list = Util::configMapToSortedList(map, &err);
            qmlSrc->setProperty("configTable", list);
        }

        if (err != "")
        {
            LOG_ERROR("Failed to get config list (err=%s)\n", err.CSTR());
        }
    });
}

QML_CfgGlobal::~QML_CfgGlobal()
{
    delete envLocal;
}

void QML_CfgGlobal::slotConfigChanged(QVariant configItem)
{
    QString err;
    QVariantMap configItemMap = configItem.toMap();
    QVariantMap map = env->ds.cfgGlobal->getConfigs(&err);

    if (err == "")
    {
        map.insert(configItemMap["KeyName"].toString(), configItemMap);
        env->ds.cfgGlobal->setConfigs(map, true, &err);
    }

    if (err != "")
    {
        LOG_ERROR("Failed to set config (err=%s)\n", err.CSTR());
    }
}

