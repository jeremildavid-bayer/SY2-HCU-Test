#ifndef HW_CAPABILITIES_H
#define HW_CAPABILITIES_H

//========================================================
// Data Declaration - Injector Specific
#define MAX_MCU_PHASES                                      12

// Injector Device
#define SYRINGE_VOLUME_NEW_MUDS_MIN                         209.0
#define SYRINGE_VOLUME_NEW_MUDS_MAX                         220.0
#define SYRINGE_VOLUME_MAX                                  200.0
#define SYRINGE_VOLUME_MIN                                  0.1
#define SYRINGE_VOLUME_FILL_ALL                             -1.0
#define SYRINGE_VOLUME_PRIME_ALL                            -1.0

#define SYRINGE_FLOW_PURGE_FLUID                            3
#define SYRINGE_FLOW_PURGE_AIR                              10.0

#define SUDS_PRIME_PRESSURE_DECAY_MS_MIN                    100
#define SUDS_PRIME_PRESSURE_DECAY_MS_MAX                    2000

#define SUDS_LENGTH_NORMAL                                  "250"
#define SUDS_LENGTH_EXTENDED                                "350"

#define SOD_SYRINGE_PRIME_TRIALS_LIMIT                      2

#define CAL_SLACK_REQUIRED_VOLUME_MIN                       30
#define CAL_SLACK_REQUIRED_VOLUME_MAX                       50
#define CAL_SYRINGE_AIR_CHECK_REQUIRED_VOL_MIN              10.0
#define CAL_SYRINGE_AIR_CHECK_REQUIRED_VOL_MAX              60.0
#define CAL_SYRINGE_AIR_CHECK_FLOW_MIN                      1.0
#define CAL_SYRINGE_AIR_CHECK_FLOW_MAX                      4.0
#define CAL_SYRINGE_AIR_CHECK_TARGET_PRESSURE_KPA_MIN       500
#define CAL_SYRINGE_AIR_CHECK_TARGET_PRESSURE_KPA_MAX       2000
#define CAL_SYRINGE_AIR_ALLOWED_VOLUME_MIN                  0.1
#define CAL_SYRINGE_AIR_ALLOWED_VOLUME_MAX                  0.5
#define CAL_SYRINGE_AIR_CHECK_DATA_DEVISION_FACTOR          100

#define SYRINGE_AIR_CHECK_TARGET_FLOW_MIN                   1.0
#define SYRINGE_AIR_CHECK_TARGET_FLOW_MAX                   4.0
#define SYRINGE_AIR_CHECK_TARGET_PRESSURE_KPA_MIN           500
#define SYRINGE_AIR_CHECK_TARGET_PRESSURE_KPA_MAX           2000
#define SYRINGE_AIR_ALLOWED_VOLUME_MIN                      0.1
#define SYRINGE_AIR_ALLOWED_VOLUME_MAX                      4.0
#define SYRINGE_AIR_RECOVERY_STATE_PRIME_EXTRA_VOL_MIN      0.5
#define SYRINGE_AIR_RECOVERY_STATE_PRIME_EXTRA_VOL_MAX      30.0
#define SYRINGE_AIR_RECOVERY_STATE_PRIME_FLOW_MIN           1.0
#define SYRINGE_AIR_RECOVERY_STATE_PRIME_FLOW_MAX           10.0
#define SYRINGE_AIR_RECOVERY_STATE_MAX_AIR_AMOUNT_MIN       1.0
#define SYRINGE_AIR_RECOVERY_STATE_MAX_AIR_AMOUNT_MAX       20.0
#define SYRINGE_AIR_RECOVERY_STATE_PRIME_VOL_MAX_AIR_MIN    0.5
#define SYRINGE_AIR_RECOVERY_STATE_PRIME_VOL_MAX_AIR_MAX    40.0
#define SYRINGE_AIR_RECOVERY_SYRINGE_VACUUM_VOL_MAX         150.0
#define SYRINGE_AIR_RECOVERY_EXTRA_PRIME_TRIALS_LIMIT       2

#define SUDS_AIR_RECOVERY_SYRINGE_PRIME_TRIALS_LIMIT        3

#define WASTE_BIN_VOLUME_MAX_ML                             750
#define CAL_SYRINGE_AIR_CHECK_COEFF_MAX                     3

#define FLUID_REMOVAL_PURGE_FLOW                            6.0
#define FLUID_REMOVAL_INLET_LINE_WITHDRAW_VOL               20.0
#define FLUID_REMOVAL_INLET_LINE_WITHDRAW_FLOW              -8.0

#define MUDS_EJECT_WITHDRAW_VOL                             1.0
#define MUDS_EJECT_WITHDRAW_FLOW                            -1.0

// Injection
#define INJECTION_IS_FINISHING_STATE_DEBOUNCE_MAX           3
#define PRESSURE_LIMIT_KPA_MIN                              345
#define PRESSURE_LIMIT_KPA_MAX                              2068
#define INJECTION_SKIP_PHASE_ENABLE_WAITING_MS              2000

// Injection Phase Default Parameters
#define DEFAULT_PHASE_FLOW                                  1.0
#define DEFAULT_PHASE_VOL                                   10
#define DEFAULT_PHASE_DELAY_MS                              10000
#define FLOW_ADJUST_DELTA                                   0.1 // ml/s

// Chosen Device Limitations
#define DEFAULT_HEATER_TEMPERATURE                          38.0
#define HEATER_CUTOFF_TEMPERATURE                           42.0
#define HCU_TEMPERATURE_WARNING_ACTIVATE_CELCIUS            85
#define HCU_TEMPERATURE_WARNING_DEACTIVATE_CELCIUS          60
#define LOW_DISK_SPACE_THRESHOLD_MB                         1024

#define PISTON_CLEANING_POSITION_ML                         140.0
#define PISTON_CLEANING_POSITION_FLOW_RATE                  9.0 //ml/s

#define STOPCOCK_ACTION_TRIALS_LIMIT                        3
#define DISENGAGE_ACTION_TRIALS_LIMIT                       3

#define STOPCOCK_ACTION_TIMEOUT_MS                          1500
#define MCU_ARM_PROCESSING_TIMEOUT_MS                       10000

// Fluid Option Parameters
#define FLUID_SOURCE_USE_LIFE_CHECK_INTERVAL_MS             (1000 * 20) // ms

#define FLUID_OPTION_CONTRAST_BRAND_LEN_MAX                 50
#define FLUID_OPTION_CONTRAST_BARCODE_LEN_MIN               1
#define FLUID_OPTION_CONTRAST_BARCODE_LEN_MAX               20
#define FLUID_OPTION_CONTRAST_CONCENTRATION_MIN             200
#define FLUID_OPTION_CONTRAST_CONCENTRATION_MAX             450
#define FLUID_OPTION_CONTRAST_VOLUME_MIN                    50
#define FLUID_OPTION_CONTRAST_VOLUME_MAX                    500
#define FLUID_OPTION_CONTRAST_MAX_USE_LIFE_HOUR_MIN         1
#define FLUID_OPTION_CONTRAST_MAX_USE_LIFE_HOUR_MAX         168
#define FLUID_BOTTLE_LOT_BATCH_TEXT_LEN_MIN                 0
#define FLUID_BOTTLE_LOT_BATCH_TEXT_LEN_MAX                 16

// Alerts
#define ALERTS_BUFFER_LIMIT_MAX                             5000
#define ALERTS_BUFFER_LIMIT_MIN                             1000

// Calibration
#define CALMOTOR_TIMEOUT_MS                                 120000
#define CALSUDS_TIMEOUT_MS                                  60000
#define CALPLUNGER_TIMEOUT_MS                               60000
#define CALBUB_TIMEOUT_MS                                   60000

// Service
#define HASP_KEY_ENFORCEMENT_SERVICE_KEY                    "0000"

enum SyringeIdx
{
    SYRINGE_IDX_START = 0,
    SYRINGE_IDX_SALINE = SYRINGE_IDX_START,
    SYRINGE_IDX_CONTRAST1,
    SYRINGE_IDX_CONTRAST2,
    SYRINGE_IDX_MAX,
    SYRINGE_IDX_NONE = SYRINGE_IDX_MAX
};

enum HeatMaintainerIdx
{
    HEAT_MAINTAINER_IDX_DOOR = 0,
    HEAT_MAINTAINER_IDX_MUDS,
    HEAT_MAINTAINER_IDX_MAX
};

enum LedIndex
{
    LED_IDX_SALINE = 0,
    LED_IDX_CONTRAST1,
    LED_IDX_CONTRAST2,
    LED_IDX_SUDS1,
    LED_IDX_SUDS2,
    LED_IDX_SUDS3,
    LED_IDX_SUDS4,
    LED_IDX_DOOR1,
    LED_IDX_DOOR2,
    LED_IDX_DOOR3,
    LED_IDX_DOOR4,
    LED_IDX_AIR_DOOR,

    LED_IDX_MAX,
    LED_IDX_UNKNOWN = LED_IDX_MAX
};

#endif // HW_CAPABILITIES_H
