#include <QTimer>
#include "QML_Capabilities.h"
#include "DataServices/Capabilities/DS_Capabilities.h"
#include "Common/Util.h"

QML_Capabilities::QML_Capabilities(QObject *parent, EnvGlobal *env_) :
    QObject(parent),
    env(env_)
{
    envLocal = new EnvLocal("QML_Capabilities", "QML_CAPABILITIES");
    qmlSrc = env->qml.object->findChild<QObject*>("dsCapabilities");
    env->qml.engine->rootContext()->setContextProperty("dsCapabilitiesCpp", this);

    if (qmlSrc == NULL)
    {
        LOG_ERROR("Failed to assign qmlSrc.\n");
    }

    // Register Data Callbacks
    connect(env->ds.capabilities, &DS_Capabilities::signalConfigChanged_DateTime_SystemDateTime, this, [=](const Config::Item &cfg){
        qmlSrc->setProperty("dateTimeModifiedAt", cfg.value);
    });

    connect(env->ds.capabilities, &DS_Capabilities::signalConfigChanged_General_ScreenMode, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("screenMode", cfg.value);
    });

    connect(env->ds.capabilities, &DS_Capabilities::signalConfigChanged_General_DisplayDeviceManagerDuringInjection, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("displayDeviceManagerDuringInjection", cfg.value);
    });

    connect(env->ds.capabilities, &DS_Capabilities::signalConfigChanged, this, [=](){
        QString err;
        QVariantMap map = env->ds.capabilities->getConfigs(&err);

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

    connect(env->ds.capabilities, &DS_Capabilities::signalConfigChanged_Developer_AdvanceModeDevModeEnabled, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("advanceModeDevModeEnabled", cfg.value.toBool());
    });

    connect(env->ds.capabilities, &DS_Capabilities::signalConfigChanged_Developer_HCUMonitorEnabled, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("hcuMonitorEnabled", cfg.value.toBool());
    });
    
    connect(env->ds.capabilities, &DS_Capabilities::signalConfigChanged_Developer_ContinuousExamsTestEnabled, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("continuousExamsTestEnabled", cfg.value.toBool());
    });

    connect(env->ds.capabilities, &DS_Capabilities::signalConfigChanged_Developer_ContinuousExamsTestLimit, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("continuousExamsTestLimit", cfg.value.toInt());
    });

    connect(env->ds.capabilities, &DS_Capabilities::signalConfigChanged_Developer_DebugMode, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("debugMode", cfg.value);
    });

    connect(env->ds.capabilities, &DS_Capabilities::signalConfigChanged_Developer_WifiDriver, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("wifiDriver", cfg.value);
    });

    connect(env->ds.capabilities, &DS_Capabilities::signalConfigChanged_Network_EthernetInterface, this, [=](const Config::Item &cfg){
        qmlSrc->setProperty("ethernetInterface", cfg.value);
    });

    connect(env->ds.capabilities, &DS_Capabilities::signalConfigChanged_BMS_MaxErrorLimitLow, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("bmsMaxErrorLimitLow", cfg.value);
    });

    connect(env->ds.capabilities, &DS_Capabilities::signalConfigChanged_BMS_TemperatureLimitLow, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("bmsTemperatureLimitLow", cfg.value.toInt());
    });

    connect(env->ds.capabilities, &DS_Capabilities::signalConfigChanged_BMS_TemperatureLimitHigh, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("bmsTemperatureLimitHigh", cfg.value.toInt());
    });

    connect(env->ds.capabilities, &DS_Capabilities::signalConfigChanged_BMS_CycleCountLimit, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("bmsCycleCountLimit", cfg.value.toInt());
    });

    connect(env->ds.capabilities, &DS_Capabilities::signalConfigChanged_BMS_StateOfHealthLowLimit, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("bmsStateOfHealthLowLimit", cfg.value.toInt());
    });

    connect(env->ds.capabilities, &DS_Capabilities::signalConfigChanged_BMS_ShippingModeAirTargetChargedLevel, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("bmsShippingModeAirTargetChargedLevel", cfg.value.toInt());
    });

    connect(env->ds.capabilities, &DS_Capabilities::signalConfigChanged_BMS_ShippingModeLandTargetChargedLevel, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("bmsShippingModeLandTargetChargedLevel", cfg.value.toInt());
    });

    connect(env->ds.capabilities, &DS_Capabilities::signalConfigChanged_BMS_ShippingModeSeaTargetChargedLevel, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("bmsShippingModeSeaTargetChargedLevel", cfg.value.toInt());
    });

    connect(env->ds.capabilities, &DS_Capabilities::signalConfigChanged_BMS_EnableBatteryTestMode, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("bmsEnableBatteryTestMode", cfg.value.toBool());
    });

    connect(env->ds.capabilities, &DS_Capabilities::signalConfigChanged_BMS_AQD_BatteryPackA_MaxError_Trigger, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("bmsAQDBatteryPackAMaxErrorTrigger", cfg.value.toInt());
    });

    connect(env->ds.capabilities, &DS_Capabilities::signalConfigChanged_BMS_AQD_BatteryPackB_MaxError_Trigger, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("bmsAQDBatteryPackBMaxErrorTrigger", cfg.value.toInt());
    });

    connect(env->ds.capabilities, &DS_Capabilities::signalConfigChanged_BMS_AQD_OtherBatteryChargeLimit, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("bmsAQDOtherBatteryChargeLimit", cfg.value.toInt());
    });

    connect(env->ds.capabilities, &DS_Capabilities::signalConfigChanged_Hidden_MaxSteps, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("stepCountMax", cfg.value.toInt());
    });

    connect(env->ds.capabilities, &DS_Capabilities::signalConfigChanged_Hidden_MaxPhases, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("phaseCountMax", cfg.value.toInt());
    });

    connect(env->ds.capabilities, &DS_Capabilities::signalConfigChanged_Hidden_InjectionPhaseFlowRateMin, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("flowRateMin", cfg.value.toDouble());
    });

    connect(env->ds.capabilities, &DS_Capabilities::signalConfigChanged_Hidden_InjectionPhaseFlowRateMax, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("flowRateMax", cfg.value.toDouble());
    });

    connect(env->ds.capabilities, &DS_Capabilities::signalConfigChanged_Hidden_InjectionPhaseVolMin, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("volumeMin", cfg.value.toDouble());
    });

    connect(env->ds.capabilities, &DS_Capabilities::signalConfigChanged_Hidden_InjectionPhaseVolMax, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("volumeMax", cfg.value.toDouble());
    });

    connect(env->ds.capabilities, &DS_Capabilities::signalConfigChanged_Hidden_InjectionPhaseDelayMsMin, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("delayMsMin", cfg.value.toULongLong());
    });

    connect(env->ds.capabilities, &DS_Capabilities::signalConfigChanged_Hidden_InjectionPhaseDelayMsMax, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("delayMsMax", cfg.value.toULongLong());
    });

    qmlSrc->setProperty("fluidOptionContrastBrandLenMax", FLUID_OPTION_CONTRAST_BRAND_LEN_MAX);
    qmlSrc->setProperty("fluidOptionContrastBarcodeLenMin", FLUID_OPTION_CONTRAST_BARCODE_LEN_MIN);
    qmlSrc->setProperty("fluidOptionContrastBarcodeLenMax", FLUID_OPTION_CONTRAST_BARCODE_LEN_MAX);
    qmlSrc->setProperty("fluidOptionContrastConcentrationMin", FLUID_OPTION_CONTRAST_CONCENTRATION_MIN);
    qmlSrc->setProperty("fluidOptionContrastConcentrationMax", FLUID_OPTION_CONTRAST_CONCENTRATION_MAX);
    qmlSrc->setProperty("fluidOptionContrastVolumeMin", FLUID_OPTION_CONTRAST_VOLUME_MIN);
    qmlSrc->setProperty("fluidOptionContrastVolumeMax", FLUID_OPTION_CONTRAST_VOLUME_MAX);
    qmlSrc->setProperty("fluidOptionContrastMaxUseLifeHourMin", FLUID_OPTION_CONTRAST_MAX_USE_LIFE_HOUR_MIN);
    qmlSrc->setProperty("fluidOptionContrastMaxUseLifeHourMax", FLUID_OPTION_CONTRAST_MAX_USE_LIFE_HOUR_MAX);
    qmlSrc->setProperty("fluidBottleLotBatchTextLenMin", FLUID_BOTTLE_LOT_BATCH_TEXT_LEN_MIN);
    qmlSrc->setProperty("fluidBottleLotBatchTextLenMax", FLUID_BOTTLE_LOT_BATCH_TEXT_LEN_MAX);
    qmlSrc->setProperty("injectionSkipPhaseEnableWaitingMs", INJECTION_SKIP_PHASE_ENABLE_WAITING_MS);
    qmlSrc->setProperty("haspKeyEnforcementServiceKey", HASP_KEY_ENFORCEMENT_SERVICE_KEY);
}

QML_Capabilities::~QML_Capabilities()
{
    delete envLocal;
}

void QML_Capabilities::slotConfigChanged(QVariant configItem)
{
    QString err;
    QVariantMap configItemMap = configItem.toMap();
    QVariantMap map = env->ds.capabilities->getConfigs(&err);

    if (err == "")
    {
        map.insert(configItemMap["KeyName"].toString(), configItemMap);
        env->ds.capabilities->setConfigs(map, true, &err);
    }

    if (err != "")
    {
        LOG_ERROR("Failed to set config (err=%s)\n", err.CSTR());
    }
}
