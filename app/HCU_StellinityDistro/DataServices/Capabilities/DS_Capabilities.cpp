#include "Apps/AppManager.h"
#include "DS_Capabilities.h"
#include "DataServices/Exam/DS_ExamData.h"
#include "DataServices/System/DS_SystemData.h"
#include "Common/ImrParser.h"

DS_Capabilities::DS_Capabilities(QObject *parent, EnvGlobal *env_) :
    QObject(parent),
    env(env_)
{
    envLocal = new EnvLocal("DS_Capabilities", "CAPABILITIES");

    QString dbFileName = QString(PATH_CONFIG_CAPABILITIES) + ((env->ds.systemData->getBuildType() == BUILD_TYPE_PROD) ? ".PROD" : "");
    db = new Config(dbFileName, env, envLocal);

    if (!db->isCfgOk())
    {
        db->initCfgFile();
    }

    connect(env->appManager, &AppManager::signalAppStarted, this, [=]() {
        slotAppStarted();
    });

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });

    initConfig();
}

DS_Capabilities::~DS_Capabilities()
{
    delete db;
    delete envLocal;
}

void DS_Capabilities::initConfig()
{
    Config::Item cfgItem;
    Config *dbNew = new Config("", env, envLocal);


    cfgItem = db->initItem("General_ScreenMode",
                           0,
                           "Splash",
                           "",
                           "list",
                           QVariantList() << "Splash" << "X11Bypass" << "Dialog",
                           QVariant(0),
                           QVariant(0),
                           QVariant(0));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("General_StartVideoEnabled",
                           1,
                           true,
                           "",
                           "bool",
                           QVariantList(),
                           QVariant(),
                           QVariant(),
                           QVariant());
    dbNew->set(cfgItem);

    cfgItem = db->initItem("General_DisplayDeviceManagerDuringInjection",
                           2,
                           false,
                           "",
                           "bool",
                           QVariantList(),
                           QVariant(),
                           QVariant(),
                           QVariant());
    dbNew->set(cfgItem);

    cfgItem = db->initItem("General_WebApplicationEnabled",
                           3,
                           true,
                           "",
                           "bool",
                           QVariantList(),
                           QVariant(),
                           QVariant(),
                           QVariant());
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Developer_McuSimulationEnabled",
                           0,
                           false,
                           "",
                           "bool",
                           QVariantList(),
                           QVariant(),
                           QVariant(),
                           QVariant());
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Developer_AdvanceModeDevModeEnabled",
                           1,
                           false,
                           "",
                           "bool",
                           QVariantList(),
                           QVariant(),
                           QVariant(),
                           QVariant());
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Developer_ContinuousExamsTestEnabled",
                           2,
                           false,
                           "",
                           "bool",
                           QVariantList(),
                           QVariant(),
                           QVariant(),
                           QVariant());
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Developer_ContinuousExamsTestLimit",
                           3,
                           0,
                           "exams",
                           "int",
                           QVariantList(),
                           QVariant(0),
                           QVariant(1000),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Developer_ContinuousExamsTestExamStartDelaySec",
                           4,
                           5,
                           "sec",
                           "int",
                           QVariantList(),
                           QVariant(1),
                           QVariant(60 * 10),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Developer_ContinuousExamsTestSelectedPlans",
                           5,
                           "[\"" DEFAULT_INJECT_PLAN_TEMPLATE_GUID "\"]",
                           "",
                           "object",
                           QVariantList(),
                           QVariant(0),
                           QVariant(0),
                           QVariant(0));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Developer_ContinuousExamsTestActiveContrastLocation",
                           6,
                           "C1",
                           "",
                           "list",
                           QVariantList() << "C1" << "C2",
                           QVariant(0),
                           QVariant(0),
                           QVariant(0));
    dbNew->set(cfgItem);
    
    cfgItem = db->initItem("Developer_HCUMonitorEnabled",
                           7,
                           false,
                           "",
                           "bool",
                           QVariantList(),
                           QVariant(),
                           QVariant(),
                           QVariant());
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Developer_DebugMode",
                           8,
                           "off",
                           "",
                           "list",
                           QVariantList() << "off" << "on" << "vm_as_hcu" << "vm_as_host",
                           QVariant(0),
                           QVariant(0),
                           QVariant(0));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Developer_WifiDriver",
                           9,
                           "OpenSource",
                           "",
                           "list",
                           QVariantList() << "OpenSource" << "Silex",
                           QVariant(0),
                           QVariant(0),
                           QVariant(0));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("DateTime_SystemDateTime",
                           0,
                           QDateTime::currentDateTimeUtc().toString("yyyy/MM/dd hh:mm"),
                           "",
                           "curDateTime",
                           QVariantList(),
                           QVariant(),
                           QVariant(),
                           QVariant());
    dbNew->set(cfgItem);

    cfgItem = db->initItem("FluidControl_NewMudsVolOffset",
                           0,
                           SYRINGE_VOLUME_NEW_MUDS_MIN,
                           "ml",
                           "double",
                           QVariantList(),
                           QVariant(SYRINGE_VOLUME_NEW_MUDS_MIN),
                           QVariant(SYRINGE_VOLUME_NEW_MUDS_MAX),
                           QVariant(0.1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("FluidControl_FillFlowRate",
                           1,
                           9.0,
                           "ml/s",
                           "double",
                           QVariantList(),
                           QVariant(0.1),
                           QVariant(10.0),
                           QVariant(0.1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("FluidControl_CalSyringeSlackRequiredVol",
                           2,
                           30.0,
                           "ml",
                           "double",
                           QVariantList(),
                           QVariant(CAL_SLACK_REQUIRED_VOLUME_MIN),
                           QVariant(CAL_SLACK_REQUIRED_VOLUME_MAX),
                           QVariant(0.1));
    dbNew->set(cfgItem);
	
    cfgItem = db->initItem("FluidControl_CalSyringeAirCheckFlow",
                           3,
                           2.0,
                           "ml/s",
                           "double",
                           QVariantList(),
                           QVariant(CAL_SYRINGE_AIR_CHECK_FLOW_MIN),
                           QVariant(CAL_SYRINGE_AIR_CHECK_FLOW_MAX),
                           QVariant(0.1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("FluidControl_CalSyringeAirCheckTargetPressure",
                           4,
                           1500,
                           "kPa",
                           "int",
                           QVariantList(),
                           QVariant(CAL_SYRINGE_AIR_CHECK_TARGET_PRESSURE_KPA_MIN),
                           QVariant(CAL_SYRINGE_AIR_CHECK_TARGET_PRESSURE_KPA_MAX),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("FluidControl_ReusedMudsValidationOffset",
                           5,
                           5.0,
                           "ml",
                           "double",
                           QVariantList(),
                           QVariant(0),
                           QVariant(300),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("FluidControl_SyringeVolumeAdjustmentLimitMin",
                           6,
                           2.0,
                           "ml",
                           "double",
                           QVariantList(),
                           QVariant(0),
                           QVariant(20),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("FluidControl_SyringeVolumeAdjustmentLimitMax",
                           7,
                           198.0,
                           "ml",
                           "double",
                           QVariantList(),
                           QVariant(180),
                           QVariant(200),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("FluidControl_SyringeVolumeSwitchOverAdjustmentLimitMin",
                           8,
                           5.0,
                           "ml",
                           "double",
                           QVariantList(),
                           QVariant(0),
                           QVariant(20),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("FluidControl_FluidRemovalFlow",
                           9,
                           6.0,
                           "ml/s",
                           "double",
                           QVariantList(),
                           QVariant(0.1),
                           QVariant(10.0),
                           QVariant(0.1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("FluidControl_PatientLineVolume",
                           10,
                           8.0,
                           "ml",
                           "double",
                           QVariantList(),
                           QVariant(0),
                           QVariant(15),
                           QVariant(0.1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("FluidControl_PatientLineVolumeExtended",
                           11,
                           11.3,
                           "ml",
                           "double",
                           QVariantList(),
                           QVariant(0),
                           QVariant(18.3),
                           QVariant(0.1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Prime_AutoPrimeVol",
                           0,
                           15.0,
                           "ml",
                           "double",
                           QVariantList(),
                           QVariant(1),
                           QVariant(SYRINGE_VOLUME_MAX),
                           QVariant(0.1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Prime_AutoPrimeFlow",
                           1,
                           6.0,
                           "ml/s",
                           "double",
                           QVariantList(),
                           QVariant(0.1),
                           QVariant(10.0),
                           QVariant(0.1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Prime_ExtendedAutoPrimeVol",
                           2,
                           30.0,
                           "ml",
                           "double",
                           QVariantList(),
                           QVariant(1),
                           QVariant(SYRINGE_VOLUME_MAX),
                           QVariant(0.1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Prime_ManualPrimeFlow",
                           3,
                           2.0,
                           "ml/s",
                           "double",
                           QVariantList(),
                           QVariant(0.1),
                           QVariant(10.0),
                           QVariant(0.1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Prime_SyringePrimeVol",
                           4,
                           5.0,
                           "ml",
                           "double",
                           QVariantList(),
                           QVariant(1),
                           QVariant(SYRINGE_VOLUME_MAX),
                           QVariant(0.1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Prime_SyringePrimeFlow",
                           5,
                           5.0,
                           "ml/s",
                           "double",
                           QVariantList(),
                           QVariant(0.1),
                           QVariant(10.0),
                           QVariant(0.1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Prime_PrimeSalinePressureDecayWait",
                           6,
                           1000,
                           "ms",
                           "int",
                           QVariantList(),
                           QVariant(SUDS_PRIME_PRESSURE_DECAY_MS_MIN),
                           QVariant(SUDS_PRIME_PRESSURE_DECAY_MS_MAX),
                           QVariant(0.1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Prime_PrimeContrastPressureDecayWait",
                           7,
                           1500,
                           "ms",
                           "int",
                           QVariantList(),
                           QVariant(SUDS_PRIME_PRESSURE_DECAY_MS_MIN),
                           QVariant(SUDS_PRIME_PRESSURE_DECAY_MS_MAX),
                           QVariant(0.1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("AirCheck_SyringeAirCheckRequiredVol",
                           0,
                           20,
                           "ml",
                           "double",
                           QVariantList(),
                           QVariant(CAL_SYRINGE_AIR_CHECK_REQUIRED_VOL_MIN),
                           QVariant(CAL_SYRINGE_AIR_CHECK_REQUIRED_VOL_MAX),
                           QVariant(0.1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("AirCheck_SyringeAirCheckFlow",
                           1,
                           2.0,
                           "ml/s",
                           "double",
                           QVariantList(),
                           QVariant(SYRINGE_AIR_CHECK_TARGET_FLOW_MIN),
                           QVariant(SYRINGE_AIR_CHECK_TARGET_FLOW_MAX),
                           QVariant(0.1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("AirCheck_SyringeAirCheckTargetPressure",
                           2,
                           1500,
                           "kPa",
                           "int",
                           QVariantList(),
                           QVariant(SYRINGE_AIR_CHECK_TARGET_PRESSURE_KPA_MIN),
                           QVariant(SYRINGE_AIR_CHECK_TARGET_PRESSURE_KPA_MAX),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("AirCheck_SyringeAirRecoveryPrimeExtraVol",
                           3,
                           5.0,
                           "ml",
                           "double",
                           QVariantList(),
                           QVariant(SYRINGE_AIR_RECOVERY_STATE_PRIME_EXTRA_VOL_MIN),
                           QVariant(SYRINGE_AIR_RECOVERY_STATE_PRIME_EXTRA_VOL_MAX),
                           QVariant(0.1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("AirCheck_SyringeAirRecoveryPrimeFlow",
                           4,
                           10.0,
                           "ml/s",
                           "double",
                           QVariantList(),
                           QVariant(SYRINGE_AIR_RECOVERY_STATE_PRIME_FLOW_MIN),
                           QVariant(SYRINGE_AIR_RECOVERY_STATE_PRIME_FLOW_MAX),
                           QVariant(0.1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("AirCheck_SyringeAirRecoveryMaxAirAmount",
                           5,
                           10.0,
                           "ml",
                           "double",
                           QVariantList(),
                           QVariant(SYRINGE_AIR_RECOVERY_STATE_MAX_AIR_AMOUNT_MIN),
                           QVariant(SYRINGE_AIR_RECOVERY_STATE_MAX_AIR_AMOUNT_MAX),
                           QVariant(0.1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Preload_SalineVolume",
                           0,
                           11.0,
                           "ml",
                           "double",
                           QVariantList(),
                           QVariant(1),
                           QVariant(SYRINGE_VOLUME_MAX),
                           QVariant(0.1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Preload_Contrast1Volume",
                           1,
                           10.5,
                           "ml",
                           "double",
                           QVariantList(),
                           QVariant(1),
                           QVariant(SYRINGE_VOLUME_MAX),
                           QVariant(0.1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Preload_Contrast2Volume",
                           2,
                           10.0,
                           "ml",
                           "double",
                           QVariantList(),
                           QVariant(1),
                           QVariant(SYRINGE_VOLUME_MAX),
                           QVariant(0.1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Preload_SalineVolumeExtended",
                           3,
                           14.3,
                           "ml",
                           "double",
                           QVariantList(),
                           QVariant(1),
                           QVariant(SYRINGE_VOLUME_MAX),
                           QVariant(0.1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Preload_Contrast1VolumeExtended",
                           4,
                           13.8,
                           "ml",
                           "double",
                           QVariantList(),
                           QVariant(1),
                           QVariant(SYRINGE_VOLUME_MAX),
                           QVariant(0.1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Preload_Contrast2VolumeExtended",
                           5,
                           13.3,
                           "ml",
                           "double",
                           QVariantList(),
                           QVariant(1),
                           QVariant(SYRINGE_VOLUME_MAX),
                           QVariant(0.1));
    dbNew->set(cfgItem);



    cfgItem = db->initItem("Preload_PreloadFlowRate",
                           6,
                           5.0,
                           "ml/s",
                           "double",
                           QVariantList(),
                           QVariant(SYRINGE_AIR_RECOVERY_STATE_PRIME_FLOW_MIN),
                           QVariant(SYRINGE_AIR_RECOVERY_STATE_PRIME_FLOW_MAX),
                           QVariant(0.1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Network_EthernetInterface",
                           0,
                           "enp3s0",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(1),
                           QVariant(10),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Network_WifiInterface",
                           1,
                           "ath0",
                           "",
                           "string",
                           QVariantList(),
                           QVariant(1),
                           QVariant(10),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Network_DosAttackMonitorEnabled",
                           2,
                           true,
                           "",
                           "bool",
                           QVariantList(),
                           QVariant(),
                           QVariant(),
                           QVariant());
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Network_SpontaneousDigestInterval",
                           3,
                           0, // 0 means feature is off
                           "ms",
                           "int",
                           QVariantList(),
                           QVariant(0),
                           QVariant(60000),
                           QVariant(0));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Network_SpontaneousDigestOnInterval",
                           4,
                           1,
                           "min",
                           "int",
                           QVariantList(),
                           QVariant(1),
                           QVariant(2000),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Network_SpontaneousDigestOffInterval",
                           5,
                           5,
                           "min",
                           "int",
                           QVariantList(),
                           QVariant(1),
                           QVariant(2000),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("BMS_DigestPollingInterval",
                           0,
                           5,
                           "min",
                           "int",
                           QVariantList(),
                           QVariant(1),
                           QVariant(60),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("BMS_MaxErrorLimitLow",
                           1,
                           10,
                           "%",
                           "int",
                           QVariantList(),
                           QVariant(1),
                           QVariant(100),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("BMS_TemperatureLimitLow",
                           2,
                           5,
                           "°C",
                           "int",
                           QVariantList(),
                           QVariant(1),
                           QVariant(60),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("BMS_TemperatureLimitHigh",
                           3,
                           50,
                           "°C",
                           "int",
                           QVariantList(),
                           QVariant(1),
                           QVariant(60),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("BMS_CycleCountLimit",
                           4,
                           3000,
                           "times",
                           "int",
                           QVariantList(),
                           QVariant(0),
                           QVariant(5000),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("BMS_StateOfHealthLowLimit",
                           5,
                           60,
                           "%",
                           "int",
                           QVariantList(),
                           QVariant(0),
                           QVariant(100),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("BMS_PFResetTrialLimit",
                           6,
                           3,
                           "times",
                           "int",
                           QVariantList(),
                           QVariant(1),
                           QVariant(20),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("BMS_ShippingModeAirTargetChargedLevel",
                           7,
                           30,
                           "%",
                           "int",
                           QVariantList(),
                           QVariant(1),
                           QVariant(100),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("BMS_ShippingModeLandTargetChargedLevel",
                           8,
                           85,
                           "%",
                           "int",
                           QVariantList(),
                           QVariant(1),
                           QVariant(100),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("BMS_ShippingModeSeaTargetChargedLevel",
                           9,
                           85,
                           "%",
                           "int",
                           QVariantList(),
                           QVariant(1),
                           QVariant(100),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("BMS_EnableBatteryTestMode",
                           10,
                           false,
                           "",
                           "bool",
                           QVariantList(),
                           QVariant(),
                           QVariant(),
                           QVariant());
    dbNew->set(cfgItem);

    cfgItem = db->initItem("BMS_AQD_BatteryPackA_MaxError_Trigger",
                           11,
                           50,
                           "%",
                           "int",
                           QVariantList(),
                           QVariant(2),
                           QVariant(100),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("BMS_AQD_BatteryPackB_MaxError_Trigger",
                           12,
                           55,
                           "%",
                           "int",
                           QVariantList(),
                           QVariant(2),
                           QVariant(100),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("BMS_AQD_OtherBatteryChargeLimit",
                           13,
                           93,
                           "%",
                           "int",
                           QVariantList(),
                           QVariant(1),
                           QVariant(95),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Alert_AlertBufferLimit",
                           0,
                           ALERTS_BUFFER_LIMIT_MAX,
                           "records",
                           "int",
                           QVariantList(),
                           QVariant(ALERTS_BUFFER_LIMIT_MIN),
                           QVariant(ALERTS_BUFFER_LIMIT_MAX),
                           QVariant(100));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Alert_SUDSPortExposedLimitSeconds",
                           1,
                           30,
                           "s",
                           "int",
                           QVariantList(),
                           QVariant(10),
                           QVariant(3600),
                           QVariant(10));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Alert_SUDSPortExposedAfterUserAckLimitSeconds",
                           2,
                           60,
                           "s",
                           "int",
                           QVariantList(),
                           QVariant(10),
                           QVariant(3600),
                           QVariant(10));
    dbNew->set(cfgItem);   

    cfgItem = db->initItem("Alert_SUDSPrimeDataSlopeStartTime",
                           3,
                           700,
                           "ms",
                           "int",
                           QVariantList(),
                           QVariant(500),
                           QVariant(1000),
                           QVariant(10));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Alert_SUDSPrimeDataSlopeEndTime",
                           4,
                           2400,
                           "ms",
                           "int",
                           QVariantList(),
                           QVariant(1500),
                           QVariant(3000),
                           QVariant(10));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Alert_SUDSPrimeDataSteadyStateStartTime",
                           5,
                           2400,
                           "ms",
                           "int",
                           QVariantList(),
                           QVariant(1500),
                           QVariant(3000),
                           QVariant(10));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Alert_SUDSPrimeDataSteadyStateEndTime",
                           6,
                           3100,
                           "ms",
                           "int",
                           QVariantList(),
                           QVariant(1500),
                           QVariant(3500),
                           QVariant(10));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Alert_SUDSPrimeDataSlopeCutoffLow",
                           7,
                           0.05,
                           "slope",
                           "double",
                           QVariantList(),
                           QVariant(0),
                           QVariant(2),
                           QVariant(0.01));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Alert_SUDSPrimeDataSlopeCutoffHigh",
                           8,
                           0.85,
                           "slope",
                           "double",
                           QVariantList(),
                           QVariant(0),
                           QVariant(2),
                           QVariant(0.01));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Alert_ContrastBottleCountReached_Limit",
                           9,
                           25,
                           "bottles",
                           "int",
                           QVariantList(),
                           QVariant(1),
                           QVariant(100),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Alert_NewBottleMinimumFillVolume",
                           10,
                           20,
                           "ml",
                           "double",
                           QVariantList(),
                           QVariant(SYRINGE_VOLUME_MIN),
                           QVariant(SYRINGE_VOLUME_MAX),
                           QVariant(0.1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Alert_McuDiagnosticEventTriggerCount",
                           11,
                           20,
                           "events",
                           "int",
                           QVariantList(),
                           QVariant(1),
                           QVariant(50),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Alert_McuDiagnosticEventTriggerPeriod",
                           12,
                           5,
                           "s",
                           "int",
                           QVariantList(),
                           QVariant(1),
                           QVariant(600),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Logging_ImrServerHcuDigest",
                           0,
                           false,
                           "",
                           "bool",
                           QVariantList(),
                           QVariant(),
                           QVariant(),
                           QVariant());
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Logging_ImrServerDumpBodySizeLimit",
                           1,
                           50,
                           "Bytes",
                           "int",
                           QVariantList(),
                           QVariant(50),
                           QVariant(20000),
                           QVariant(10));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Logging_McuInjectDigest",
                           2,
                           false,
                           "",
                           "bool",
                           QVariantList(),
                           QVariant(),
                           QVariant(),
                           QVariant());
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Calibration_PressureCalMaxTravelUntilEngaged",
                           0,
                           30.0,
                           "ml",
                           "double",
                           QVariantList(),
                           QVariant(SYRINGE_VOLUME_MIN),
                           QVariant(SYRINGE_VOLUME_MAX),
                           QVariant(0.1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Calibration_PressureCalMaxTravelUntilDataReady",
                           1,
                           15.0,
                           "ml",
                           "double",
                           QVariantList(),
                           QVariant(SYRINGE_VOLUME_MIN),
                           QVariant(SYRINGE_VOLUME_MAX),
                           QVariant(0.1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Calibration_PressureCalInjectVolume",
                           2,
                           57.0,
                           "ml",
                           "double",
                           QVariantList(),
                           QVariant(SYRINGE_VOLUME_MIN),
                           QVariant(SYRINGE_VOLUME_MAX),
                           QVariant(0.1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Calibration_PressureCalADCRangeMin",
                           3,
                           150,
                           "",
                           "int",
                           QVariantList(),
                           QVariant(1),
                           QVariant(2000),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Calibration_PressureCalADCRangeMax",
                           4,
                           3950,
                           "",
                           "int",
                           QVariantList(),
                           QVariant(2000),
                           QVariant(5000),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Calibration_PressureCalMovingMeanSampleSize",
                           5,
                           10,
                           "samples",
                           "int",
                           QVariantList(),
                           QVariant(1),
                           QVariant(20),
                           QVariant(1));
    dbNew->set(cfgItem);


    // Hidden capabilities
    cfgItem = db->initItem("Hidden_MaxSteps",
                           0,
                           10,
                           "",
                           "int",
                           QVariantList(),
                           QVariant(1),
                           QVariant(10),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Hidden_MaxPhases",
                           0,
                           6,
                           "",
                           "int",
                           QVariantList(),
                           QVariant(1),
                           QVariant(6),
                           QVariant(1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Hidden_InjectionPhaseFlowRateMin",
                           0,
                           0.1,
                           "",
                           "double",
                           QVariantList(),
                           QVariant(0),
                           QVariant(10),
                           QVariant(0.1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Hidden_InjectionPhaseFlowRateMax",
                           0,
                           10.0,
                           "",
                           "double",
                           QVariantList(),
                           QVariant(0),
                           QVariant(10),
                           QVariant(0.1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Hidden_InjectionPhaseVolMin",
                           0,
                           1,
                           "",
                           "double",
                           QVariantList(),
                           QVariant(1),
                           QVariant(200),
                           QVariant(0.1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Hidden_InjectionPhaseVolMax",
                           0,
                           SYRINGE_VOLUME_MAX,
                           "",
                           "double",
                           QVariantList(),
                           QVariant(1),
                           QVariant(200),
                           QVariant(0.1));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Hidden_InjectionPhaseDelayMsMin",
                           0,
                           1000,
                           "",
                           "int",
                           QVariantList(),
                           QVariant(1000),
                           QVariant(900000),
                           QVariant(1000));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Hidden_InjectionPhaseDelayMsMax",
                           0,
                           15000 * 60,
                           "",
                           "int",
                           QVariantList(),
                           QVariant(1000),
                           QVariant(900000),
                           QVariant(1000));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Hidden_InjectionPressureKpaMin",
                           0,
                           345,
                           "",
                           "int",
                           QVariantList(),
                           QVariant(345),
                           QVariant(1),
                           QVariant(2068));
    dbNew->set(cfgItem);

    cfgItem = db->initItem("Hidden_InjectionPressureKpaMax",
                           0,
                           2068,
                           "",
                           "int",
                           QVariantList(),
                           QVariant(345),
                           QVariant(1),
                           QVariant(2068));
    dbNew->set(cfgItem);


    // Set new db
    dbNew->setPathConfig(db->getPathConfig());
    dbNew->saveCfgFile();
    delete db;
    db = dbNew;
}

void DS_Capabilities::slotAppStarted()
{
    // Register Config Set Callbacks
    emitAllConfigChanged();
}

void DS_Capabilities::slotAppInitialised()
{
}

void DS_Capabilities::emitAllConfigChanged()
{
    EMIT_CONFIG_CHANGED_SIGNAL(General_ScreenMode)
    EMIT_CONFIG_CHANGED_SIGNAL(General_StartVideoEnabled)
    EMIT_CONFIG_CHANGED_SIGNAL(General_DisplayDeviceManagerDuringInjection)
    EMIT_CONFIG_CHANGED_SIGNAL(General_WebApplicationEnabled)
    EMIT_CONFIG_CHANGED_SIGNAL(Developer_McuSimulationEnabled)
    EMIT_CONFIG_CHANGED_SIGNAL(Developer_AdvanceModeDevModeEnabled)
    EMIT_CONFIG_CHANGED_SIGNAL(Developer_HCUMonitorEnabled)
    EMIT_CONFIG_CHANGED_SIGNAL(Developer_ContinuousExamsTestEnabled)
    EMIT_CONFIG_CHANGED_SIGNAL(Developer_ContinuousExamsTestLimit)
    EMIT_CONFIG_CHANGED_SIGNAL(Developer_ContinuousExamsTestExamStartDelaySec)
    EMIT_CONFIG_CHANGED_SIGNAL(Developer_ContinuousExamsTestSelectedPlans)
    EMIT_CONFIG_CHANGED_SIGNAL(Developer_ContinuousExamsTestActiveContrastLocation)
    EMIT_CONFIG_CHANGED_SIGNAL(Developer_DebugMode)
    EMIT_CONFIG_CHANGED_SIGNAL(Developer_DebugMode)

    //EMIT_CONFIG_CHANGED_SIGNAL(DateTime_SystemDateTime) Do not emit during startup. prevent date time set
    EMIT_CONFIG_CHANGED_SIGNAL(FluidControl_NewMudsVolOffset)
    EMIT_CONFIG_CHANGED_SIGNAL(FluidControl_FillFlowRate)
    EMIT_CONFIG_CHANGED_SIGNAL(FluidControl_CalSyringeSlackRequiredVol)
    EMIT_CONFIG_CHANGED_SIGNAL(FluidControl_CalSyringeAirCheckFlow)
    EMIT_CONFIG_CHANGED_SIGNAL(FluidControl_CalSyringeAirCheckTargetPressure)
    EMIT_CONFIG_CHANGED_SIGNAL(FluidControl_ReusedMudsValidationOffset)
    EMIT_CONFIG_CHANGED_SIGNAL(FluidControl_SyringeVolumeAdjustmentLimitMin)
    EMIT_CONFIG_CHANGED_SIGNAL(FluidControl_SyringeVolumeAdjustmentLimitMax)
    EMIT_CONFIG_CHANGED_SIGNAL(FluidControl_SyringeVolumeSwitchOverAdjustmentLimitMin)
    EMIT_CONFIG_CHANGED_SIGNAL(FluidControl_FluidRemovalFlow)
    EMIT_CONFIG_CHANGED_SIGNAL(FluidControl_PatientLineVolume)
    EMIT_CONFIG_CHANGED_SIGNAL(FluidControl_PatientLineVolumeExtended)
    EMIT_CONFIG_CHANGED_SIGNAL(Prime_AutoPrimeVol)
    EMIT_CONFIG_CHANGED_SIGNAL(Prime_AutoPrimeFlow)
    EMIT_CONFIG_CHANGED_SIGNAL(Prime_ManualPrimeFlow)
    EMIT_CONFIG_CHANGED_SIGNAL(Prime_ExtendedAutoPrimeVol)
    EMIT_CONFIG_CHANGED_SIGNAL(Prime_SyringePrimeVol)
    EMIT_CONFIG_CHANGED_SIGNAL(Prime_SyringePrimeFlow)
    EMIT_CONFIG_CHANGED_SIGNAL(Prime_PrimeSalinePressureDecayWait)
    EMIT_CONFIG_CHANGED_SIGNAL(Prime_PrimeContrastPressureDecayWait)
    EMIT_CONFIG_CHANGED_SIGNAL(AirCheck_SyringeAirCheckRequiredVol)
    EMIT_CONFIG_CHANGED_SIGNAL(AirCheck_SyringeAirCheckFlow)
    EMIT_CONFIG_CHANGED_SIGNAL(AirCheck_SyringeAirCheckTargetPressure)
    EMIT_CONFIG_CHANGED_SIGNAL(AirCheck_SyringeAirRecoveryPrimeExtraVol)
    EMIT_CONFIG_CHANGED_SIGNAL(AirCheck_SyringeAirRecoveryPrimeExtraVolMaxAir)
    EMIT_CONFIG_CHANGED_SIGNAL(AirCheck_SyringeAirRecoveryPrimeFlow)
    EMIT_CONFIG_CHANGED_SIGNAL(AirCheck_SyringeAirRecoveryMaxAirAmount)
    EMIT_CONFIG_CHANGED_SIGNAL(Preload_SalineVolume)
    EMIT_CONFIG_CHANGED_SIGNAL(Preload_Contrast1Volume)
    EMIT_CONFIG_CHANGED_SIGNAL(Preload_Contrast2Volume)
    EMIT_CONFIG_CHANGED_SIGNAL(Preload_SalineVolumeExtended)
    EMIT_CONFIG_CHANGED_SIGNAL(Preload_Contrast1VolumeExtended)
    EMIT_CONFIG_CHANGED_SIGNAL(Preload_Contrast2VolumeExtended)
    EMIT_CONFIG_CHANGED_SIGNAL(Preload_PreloadFlowRate)
    EMIT_CONFIG_CHANGED_SIGNAL(Network_EthernetInterface)
    EMIT_CONFIG_CHANGED_SIGNAL(Network_WifiInterface)
    EMIT_CONFIG_CHANGED_SIGNAL(Network_DosAttackMonitorEnabled)
    EMIT_CONFIG_CHANGED_SIGNAL(Network_SpontaneousDigestInterval)
    EMIT_CONFIG_CHANGED_SIGNAL(Network_SpontaneousDigestOnInterval)
    EMIT_CONFIG_CHANGED_SIGNAL(Network_SpontaneousDigestOffInterval)
    EMIT_CONFIG_CHANGED_SIGNAL(BMS_DigestPollingInterval)
    EMIT_CONFIG_CHANGED_SIGNAL(BMS_MaxErrorLimitLow)
    EMIT_CONFIG_CHANGED_SIGNAL(BMS_TemperatureLimitLow)
    EMIT_CONFIG_CHANGED_SIGNAL(BMS_TemperatureLimitHigh)
    EMIT_CONFIG_CHANGED_SIGNAL(BMS_CycleCountLimit)
    EMIT_CONFIG_CHANGED_SIGNAL(BMS_StateOfHealthLowLimit)
    EMIT_CONFIG_CHANGED_SIGNAL(BMS_PFResetTrialLimit)
    EMIT_CONFIG_CHANGED_SIGNAL(BMS_ShippingModeAirTargetChargedLevel)
    EMIT_CONFIG_CHANGED_SIGNAL(BMS_ShippingModeLandTargetChargedLevel)
    EMIT_CONFIG_CHANGED_SIGNAL(BMS_ShippingModeSeaTargetChargedLevel)
    EMIT_CONFIG_CHANGED_SIGNAL(BMS_EnableBatteryTestMode)
    EMIT_CONFIG_CHANGED_SIGNAL(BMS_AQD_BatteryPackA_MaxError_Trigger)
    EMIT_CONFIG_CHANGED_SIGNAL(BMS_AQD_BatteryPackB_MaxError_Trigger)
    EMIT_CONFIG_CHANGED_SIGNAL(BMS_AQD_OtherBatteryChargeLimit)
    EMIT_CONFIG_CHANGED_SIGNAL(Alert_AlertBufferLimit)
    EMIT_CONFIG_CHANGED_SIGNAL(Alert_SUDSPortExposedLimitSeconds)
    EMIT_CONFIG_CHANGED_SIGNAL(Alert_SUDSPortExposedAfterUserAckLimitSeconds)    
    EMIT_CONFIG_CHANGED_SIGNAL(Alert_SUDSPrimeDataSlopeStartTime)
    EMIT_CONFIG_CHANGED_SIGNAL(Alert_SUDSPrimeDataSlopeEndTime)
    EMIT_CONFIG_CHANGED_SIGNAL(Alert_SUDSPrimeDataSteadyStateStartTime)
    EMIT_CONFIG_CHANGED_SIGNAL(Alert_SUDSPrimeDataSteadyStateEndTime)
    EMIT_CONFIG_CHANGED_SIGNAL(Alert_SUDSPrimeDataSlopeCutoffLow)
    EMIT_CONFIG_CHANGED_SIGNAL(Alert_SUDSPrimeDataSlopeCutoffHigh)
    EMIT_CONFIG_CHANGED_SIGNAL(Alert_ContrastBottleCountReached_Limit)
    EMIT_CONFIG_CHANGED_SIGNAL(Alert_NewBottleMinimumFillVolume)
    EMIT_CONFIG_CHANGED_SIGNAL(Alert_McuDiagnosticEventTriggerCount)
    EMIT_CONFIG_CHANGED_SIGNAL(Alert_McuDiagnosticEventTriggerPeriod)
    EMIT_CONFIG_CHANGED_SIGNAL(Logging_ImrServerHcuDigest)
    EMIT_CONFIG_CHANGED_SIGNAL(Logging_ImrServerDumpBodySizeLimit)
    EMIT_CONFIG_CHANGED_SIGNAL(Logging_McuInjectDigest)
    EMIT_CONFIG_CHANGED_SIGNAL(Calibration_PressureCalMaxTravelUntilEngaged)
    EMIT_CONFIG_CHANGED_SIGNAL(Calibration_PressureCalMaxTravelUntilDataReady)
    EMIT_CONFIG_CHANGED_SIGNAL(Calibration_PressureCalInjectVolume)
    EMIT_CONFIG_CHANGED_SIGNAL(Calibration_PressureCalADCRangeMin)
    EMIT_CONFIG_CHANGED_SIGNAL(Calibration_PressureCalADCRangeMax)
    EMIT_CONFIG_CHANGED_SIGNAL(Calibration_PressureCalMovingMeanSampleSize)

    EMIT_CONFIG_CHANGED_SIGNAL(Hidden_MaxSteps)
    EMIT_CONFIG_CHANGED_SIGNAL(Hidden_MaxPhases)
    EMIT_CONFIG_CHANGED_SIGNAL(Hidden_InjectionPhaseFlowRateMin)
    EMIT_CONFIG_CHANGED_SIGNAL(Hidden_InjectionPhaseFlowRateMax)
    EMIT_CONFIG_CHANGED_SIGNAL(Hidden_InjectionPhaseVolMin)
    EMIT_CONFIG_CHANGED_SIGNAL(Hidden_InjectionPhaseVolMax)
    EMIT_CONFIG_CHANGED_SIGNAL(Hidden_InjectionPhaseDelayMsMin)
    EMIT_CONFIG_CHANGED_SIGNAL(Hidden_InjectionPhaseDelayMsMax)
    EMIT_CONFIG_CHANGED_SIGNAL(Hidden_InjectionPressureKpaMin)
    EMIT_CONFIG_CHANGED_SIGNAL(Hidden_InjectionPressureKpaMax)

    emit signalConfigChanged();
}

QVariantMap DS_Capabilities::getConfigs(QString *err)
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

void DS_Capabilities::setConfigs(QVariantMap configs, bool setChangedAt, QString *errOut)
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

        SET_CONFIG_BY_NAME(item, General_ScreenMode, setChangedAt)
        SET_CONFIG_BY_NAME(item, General_StartVideoEnabled, setChangedAt)
        SET_CONFIG_BY_NAME(item, General_DisplayDeviceManagerDuringInjection, setChangedAt)
        SET_CONFIG_BY_NAME(item, General_WebApplicationEnabled, setChangedAt)
        SET_CONFIG_BY_NAME(item, Developer_McuSimulationEnabled, setChangedAt)
        SET_CONFIG_BY_NAME(item, Developer_AdvanceModeDevModeEnabled, setChangedAt)
        SET_CONFIG_BY_NAME(item, Developer_HCUMonitorEnabled, setChangedAt)
        SET_CONFIG_BY_NAME(item, Developer_ContinuousExamsTestEnabled, setChangedAt)
        SET_CONFIG_BY_NAME(item, Developer_ContinuousExamsTestLimit, setChangedAt)
        SET_CONFIG_BY_NAME(item, Developer_ContinuousExamsTestExamStartDelaySec, setChangedAt)
        SET_CONFIG_BY_NAME(item, Developer_ContinuousExamsTestSelectedPlans, setChangedAt)
        SET_CONFIG_BY_NAME(item, Developer_ContinuousExamsTestActiveContrastLocation, setChangedAt)
        SET_CONFIG_BY_NAME(item, Developer_DebugMode, setChangedAt)
        SET_CONFIG_BY_NAME(item, Developer_WifiDriver, setChangedAt)

        SET_CONFIG_BY_NAME(item, DateTime_SystemDateTime, setChangedAt)
        SET_CONFIG_BY_NAME(item, FluidControl_NewMudsVolOffset, setChangedAt)
        SET_CONFIG_BY_NAME(item, FluidControl_FillFlowRate, setChangedAt)
        SET_CONFIG_BY_NAME(item, FluidControl_CalSyringeSlackRequiredVol, setChangedAt)
        SET_CONFIG_BY_NAME(item, FluidControl_CalSyringeAirCheckFlow, setChangedAt)
        SET_CONFIG_BY_NAME(item, FluidControl_CalSyringeAirCheckTargetPressure, setChangedAt)
        SET_CONFIG_BY_NAME(item, FluidControl_ReusedMudsValidationOffset, setChangedAt)
        SET_CONFIG_BY_NAME(item, FluidControl_SyringeVolumeAdjustmentLimitMin, setChangedAt)
        SET_CONFIG_BY_NAME(item, FluidControl_SyringeVolumeAdjustmentLimitMax, setChangedAt)
        SET_CONFIG_BY_NAME(item, FluidControl_SyringeVolumeSwitchOverAdjustmentLimitMin, setChangedAt)
        SET_CONFIG_BY_NAME(item, FluidControl_FluidRemovalFlow, setChangedAt)
        SET_CONFIG_BY_NAME(item, FluidControl_PatientLineVolume, setChangedAt)
        SET_CONFIG_BY_NAME(item, FluidControl_PatientLineVolumeExtended, setChangedAt)
        SET_CONFIG_BY_NAME(item, Prime_AutoPrimeVol, setChangedAt)
        SET_CONFIG_BY_NAME(item, Prime_AutoPrimeFlow, setChangedAt)
        SET_CONFIG_BY_NAME(item, Prime_ManualPrimeFlow, setChangedAt)
        SET_CONFIG_BY_NAME(item, Prime_ExtendedAutoPrimeVol, setChangedAt)
        SET_CONFIG_BY_NAME(item, Prime_SyringePrimeVol, setChangedAt)
        SET_CONFIG_BY_NAME(item, Prime_SyringePrimeFlow, setChangedAt)
        SET_CONFIG_BY_NAME(item, Prime_PrimeSalinePressureDecayWait, setChangedAt)
        SET_CONFIG_BY_NAME(item, Prime_PrimeContrastPressureDecayWait, setChangedAt)
        SET_CONFIG_BY_NAME(item, AirCheck_SyringeAirCheckRequiredVol, setChangedAt)
        SET_CONFIG_BY_NAME(item, AirCheck_SyringeAirCheckFlow, setChangedAt)
        SET_CONFIG_BY_NAME(item, AirCheck_SyringeAirCheckTargetPressure, setChangedAt)
        SET_CONFIG_BY_NAME(item, AirCheck_SyringeAirRecoveryPrimeExtraVol, setChangedAt)
        SET_CONFIG_BY_NAME(item, AirCheck_SyringeAirRecoveryPrimeExtraVolMaxAir, setChangedAt)
        SET_CONFIG_BY_NAME(item, AirCheck_SyringeAirRecoveryPrimeFlow, setChangedAt)
        SET_CONFIG_BY_NAME(item, AirCheck_SyringeAirRecoveryMaxAirAmount, setChangedAt)
        SET_CONFIG_BY_NAME(item, Preload_SalineVolume, setChangedAt)
        SET_CONFIG_BY_NAME(item, Preload_Contrast1Volume, setChangedAt)
        SET_CONFIG_BY_NAME(item, Preload_Contrast2Volume, setChangedAt)
        SET_CONFIG_BY_NAME(item, Preload_SalineVolumeExtended, setChangedAt)
        SET_CONFIG_BY_NAME(item, Preload_Contrast1VolumeExtended, setChangedAt)
        SET_CONFIG_BY_NAME(item, Preload_Contrast2VolumeExtended, setChangedAt)
        SET_CONFIG_BY_NAME(item, Preload_PreloadFlowRate, setChangedAt)
        SET_CONFIG_BY_NAME(item, Network_EthernetInterface, setChangedAt)
        SET_CONFIG_BY_NAME(item, Network_WifiInterface, setChangedAt)
        SET_CONFIG_BY_NAME(item, Network_DosAttackMonitorEnabled, setChangedAt)
        SET_CONFIG_BY_NAME(item, Network_SpontaneousDigestInterval, setChangedAt)
        SET_CONFIG_BY_NAME(item, Network_SpontaneousDigestOnInterval, setChangedAt)
        SET_CONFIG_BY_NAME(item, Network_SpontaneousDigestOffInterval, setChangedAt)
        SET_CONFIG_BY_NAME(item, BMS_DigestPollingInterval, setChangedAt)
        SET_CONFIG_BY_NAME(item, BMS_MaxErrorLimitLow, setChangedAt)
        SET_CONFIG_BY_NAME(item, BMS_TemperatureLimitLow, setChangedAt)
        SET_CONFIG_BY_NAME(item, BMS_TemperatureLimitHigh, setChangedAt)
        SET_CONFIG_BY_NAME(item, BMS_CycleCountLimit, setChangedAt)
        SET_CONFIG_BY_NAME(item, BMS_StateOfHealthLowLimit, setChangedAt)
        SET_CONFIG_BY_NAME(item, BMS_PFResetTrialLimit, setChangedAt)
        SET_CONFIG_BY_NAME(item, BMS_ShippingModeAirTargetChargedLevel, setChangedAt)
        SET_CONFIG_BY_NAME(item, BMS_ShippingModeLandTargetChargedLevel, setChangedAt)
        SET_CONFIG_BY_NAME(item, BMS_ShippingModeSeaTargetChargedLevel, setChangedAt)
        SET_CONFIG_BY_NAME(item, BMS_EnableBatteryTestMode, setChangedAt)
        SET_CONFIG_BY_NAME(item, BMS_AQD_BatteryPackA_MaxError_Trigger, setChangedAt)
        SET_CONFIG_BY_NAME(item, BMS_AQD_BatteryPackB_MaxError_Trigger, setChangedAt)
        SET_CONFIG_BY_NAME(item, BMS_AQD_OtherBatteryChargeLimit, setChangedAt)
        SET_CONFIG_BY_NAME(item, Alert_AlertBufferLimit, setChangedAt)
        SET_CONFIG_BY_NAME(item, Alert_SUDSPortExposedLimitSeconds, setChangedAt)
        SET_CONFIG_BY_NAME(item, Alert_SUDSPortExposedAfterUserAckLimitSeconds, setChangedAt)        
        SET_CONFIG_BY_NAME(item, Alert_SUDSPrimeDataSlopeStartTime, setChangedAt)
        SET_CONFIG_BY_NAME(item, Alert_SUDSPrimeDataSlopeEndTime, setChangedAt)
        SET_CONFIG_BY_NAME(item, Alert_SUDSPrimeDataSteadyStateStartTime, setChangedAt)
        SET_CONFIG_BY_NAME(item, Alert_SUDSPrimeDataSteadyStateEndTime, setChangedAt)
        SET_CONFIG_BY_NAME(item, Alert_SUDSPrimeDataSlopeCutoffLow, setChangedAt)
        SET_CONFIG_BY_NAME(item, Alert_SUDSPrimeDataSlopeCutoffHigh, setChangedAt)
        SET_CONFIG_BY_NAME(item, Alert_ContrastBottleCountReached_Limit, setChangedAt)
        SET_CONFIG_BY_NAME(item, Alert_NewBottleMinimumFillVolume, setChangedAt)
        SET_CONFIG_BY_NAME(item, Alert_McuDiagnosticEventTriggerCount, setChangedAt)
        SET_CONFIG_BY_NAME(item, Alert_McuDiagnosticEventTriggerPeriod, setChangedAt)
        SET_CONFIG_BY_NAME(item, Logging_ImrServerHcuDigest, setChangedAt)
        SET_CONFIG_BY_NAME(item, Logging_ImrServerDumpBodySizeLimit, setChangedAt)
        SET_CONFIG_BY_NAME(item, Logging_McuInjectDigest, setChangedAt)
        SET_CONFIG_BY_NAME(item, Calibration_PressureCalMaxTravelUntilEngaged, setChangedAt)
        SET_CONFIG_BY_NAME(item, Calibration_PressureCalMaxTravelUntilDataReady, setChangedAt)
        SET_CONFIG_BY_NAME(item, Calibration_PressureCalInjectVolume, setChangedAt)
        SET_CONFIG_BY_NAME(item, Calibration_PressureCalADCRangeMin, setChangedAt)
        SET_CONFIG_BY_NAME(item, Calibration_PressureCalADCRangeMax, setChangedAt)
        SET_CONFIG_BY_NAME(item, Calibration_PressureCalMovingMeanSampleSize, setChangedAt)
        SET_CONFIG_BY_NAME(item, Hidden_MaxSteps, setChangedAt)
        SET_CONFIG_BY_NAME(item, Hidden_MaxPhases, setChangedAt)
        SET_CONFIG_BY_NAME(item, Hidden_InjectionPhaseFlowRateMin, setChangedAt)
        SET_CONFIG_BY_NAME(item, Hidden_InjectionPhaseFlowRateMax, setChangedAt)
        SET_CONFIG_BY_NAME(item, Hidden_InjectionPhaseVolMin, setChangedAt)
        SET_CONFIG_BY_NAME(item, Hidden_InjectionPhaseVolMax, setChangedAt)
        SET_CONFIG_BY_NAME(item, Hidden_InjectionPhaseDelayMsMin, setChangedAt)
        SET_CONFIG_BY_NAME(item, Hidden_InjectionPhaseDelayMsMax, setChangedAt)
        SET_CONFIG_BY_NAME(item, Hidden_InjectionPressureKpaMin, setChangedAt)
        SET_CONFIG_BY_NAME(item, Hidden_InjectionPressureKpaMax, setChangedAt)
    }

    if (oldCfgMap != db->getCfgMap())
    {
        emit signalConfigChanged();
    }

    *errOut = err;
}

