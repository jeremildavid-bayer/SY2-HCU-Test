#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "HwCapabilities.h"

// Action BaseExt
#define PROCESS_STATE_TRANSITION_DELAY_MS                       1

// Power
#define POWER_OFF_WAIT_TIME_MS                                  1000
#define POWER_OFF_BATTERY_CRITICAL_WAIT_TIME_MS                 5000
#define POWER_BATTERY_INDEX_MAX                                 2
#define POWER_BMS_INIT_DURATION_MS                              5000
#define POWER_BMS_DIGEST_UPDATE_DURING_SERVICE_INTERVAL_MS      20000
#define POWER_BMS_FULLY_DISCHARGED_LEVEL_MAX                    2

// Build Type definitions
#define BUILD_TYPE_PROD "PROD"
#define BUILD_TYPE_REL  "REL"
#define BUILD_TYPE_VNV  "VNV"
#define BUILD_TYPE_SQA  "SQA"
#define BUILD_TYPE_DEV  "DEV"

// Heat Maintainer
#define HEAT_MAINTAINER_TEMPERATURE_MONITOR_INTERVAL_MS         (5000 * 60)

// Spike Sensor
#define SPIKE_STATE_MONITOR_DEBOUNCE_INTERVAL_MS                600

// MCU Link
#define MCU_FTDI_DESCRIPTION                                    "STELLINITY2"
#define MCU_TX_POLL_MS                                          5
#define MCU_COMM_TIMEOUT_MS                                     500
#define MCU_ACTION_TIMEOUT_MS                                   (MCU_COMM_TIMEOUT_MS * 6) // Changed to 3 seconds. 2 wasn't enough
#define MCU_COMM_RECONNECT_TIMEOUT_MS                           3000
#define MCU_LINK_HEART_BEAT_INTERVAL_MS                         200
#define MCU_SERVICE_MONITOR_INTERVAL_MS                         500
#define MCU_COMM_CONNECT_TIMEOUT_MS                             (45 * 1000)
#define MCU_STOP_COMMAND_ALIVE_MS                               1000

// MCU Simulator
#define MCU_SIM_PROCESSING_TIME_MS                              4
#define MCU_SIM_SYRINGE_ACTION_DELAY_MS                         500

#define EMPTY_GUID                                              "00000000-0000-0000-0000-000000000000"
#define DEFAULT_INJECT_PLAN_TEMPLATE_GUID                       "6c17d678-e52a-4570-ab48-0910bb60db92"
#define DEFAULT_INJECT_STEP_TEMPLATE_GUID                       "90c89398-457e-417f-bba9-35b4a8b1f77d"
#define ANONYMIZED_STR                                          "*****"

#define INJECTOR_ARM_TIMEOUT_MS                                 (20 * 60 * 1000) // 20 minutes expressed in ms

// CRU Link
#define CRU_SSID_PREFIX                                         "BAYER_CENTARGO_SN"
#define CRU_RECONNECT_TIMEOUT_MS                                2000
#define CRU_LINK_HEART_BEAT_INTERVAL_MS                         1500
#define CRU_REPLY_TIMEOUT_MS                                    1500
#define CRU_START_EXAM_REPLY_TIMEOUT_MS                         3000
#define CRU_RX_RETRY_LIMIT                                      2
#define CRU_WAIT_FOR_NEXT_RETRY_HEART_BEAT                      1000
#define CRU_INJECTION_PLAN_DATA_POLL_MS                         200
#define CRU_SSID_PASSWORD_MD5_SALT                              "75514561-FC44-4E41-986B-045603042221"
#define CRU_TIME_SYNC_TOLERANCE_SEC                             2
#define CRU_HCU_ACTION_PROCESS_TIMEOUT_MS                       5000


// LED
#define LED_FLASH_INTERVAL_MS                                   500
#define LED_MOOD_LIGHT_BACKGROUND_CTRL_INTERVAL_MS              100
#define LED_MOOD_LIGHT_CONFIG_IMAXEON                           "[ { \"From\": \"ffffff\", \"To\": \"00ff00\", \"Duration\": 2000 },"\
                                                                "  { \"From\": \"00ff00\", \"To\": \"0000ff\", \"Duration\": 2000 },"\
                                                                "  { \"From\": \"0000ff\", \"To\": \"ffffff\", \"Duration\": 2000 } ]"

#define LED_MOOD_LIGHT_CONFIG_KIPPER                            "[ { \"From\": \"000000\", \"To\": \"ff0000\", \"Duration\": 3000 },"\
                                                                "  { \"From\": \"ff0000\", \"To\": \"0000ff\", \"Duration\": 3000 },"\
                                                                "  { \"From\": \"0000ff\", \"To\": \"00ff00\", \"Duration\": 3000 },"\
                                                                "  { \"From\": \"00ff00\", \"To\": \"ffffff\", \"Duration\": 3000 },"\
                                                                "  { \"From\": \"ffffff\", \"To\": \"000000\", \"Duration\": 3000 } ]"\

#define LED_MOOD_LIGHT_CONFIG_HEARTBEAT                         "[ { \"From\": \"000000\", \"To\": \"ff0000\", \"Duration\": 100 },"\
                                                                "  { \"From\": \"ff0000\", \"To\": \"000000\", \"Duration\": 3000 },"\
                                                                "  { \"From\": \"000000\", \"To\": \"00ff00\", \"Duration\": 100 },"\
                                                                "  { \"From\": \"00ff00\", \"To\": \"000000\", \"Duration\": 3000 },"\
                                                                "  { \"From\": \"000000\", \"To\": \"0000ff\", \"Duration\": 100 }, "\
                                                                "  { \"From\": \"0000ff\", \"To\": \"000000\", \"Duration\": 3000 } ]"\

// IMR
#define IMR_VERSION                                             "v11"
#define IMR_MAX_DIGESTS                                         3000
#define IMR_MIN_DIGEST_INTERVAL_MS                              50
#define IMR_SRU_URL_PREFIX                                      "/sru"
#define IMR_CRU_URL_TEST_MESSAGE                                "v0/test/message"
#define IMR_CRU_URL_CONFIGS                                     IMR_VERSION "/configs"                   // put params="body:ConfigurationItems"
#define IMR_CRU_URL_DIGEST                                      IMR_VERSION "/digest"
#define IMR_CRU_URL_PROFILE                                     IMR_VERSION "/profile"
#define IMR_CRU_URL_FLUIDS                                      IMR_VERSION "/fluidoptions"
#define IMR_CRU_URL_INJECT_PLANS                                IMR_VERSION "/plans"
#define IMR_CRU_URL_INJECT_PLAN                                 IMR_VERSION "/plan"                      // post params="?guid={PlanTemplate}"
#define IMR_CRU_URL_BARCODE                                     IMR_VERSION "/update/barcode"            // post params="?read={Value}"
#define IMR_CRU_URL_WORKLIST                                    IMR_VERSION "/worklist"
#define IMR_CRU_URL_QUERY_WORKLIST                              IMR_VERSION "/commands/queryWorklist"
#define IMR_CRU_URL_SELECT_WORKLIST_ENTRY                       IMR_VERSION "/commands/selectWorklist"   // post params="?studyInstanceUid={UID}"
#define IMR_CRU_URL_START_EXAM                                  IMR_VERSION "/commands/startExam"
#define IMR_CRU_URL_END_EXAM                                    IMR_VERSION "/commands/endExam"
#define IMR_CRU_URL_UPDATE_INJECTION_PARAMTER                   IMR_VERSION "/commands/updateParameter"
#define IMR_CRU_URL_UPDATE_EXAM_FIELD                           IMR_VERSION "/commands/updateExamField" // post params="?examGuid={guid}&name={fieldName}&value={fieldValue}"
#define IMR_CRU_URL_UPDATE_EXAM_FIELD_PARAMETER                 IMR_VERSION "/commands/updateExamFieldParameter" // post params="?examGuid={guid}&name={fieldName}&value={fieldValue}"
#define IMR_CRU_URL_UPDATE_LINKED_ACCESSION                     IMR_VERSION "/commands/updateLinkedAccession" // post params="?examGuid={guid}&studyInstanceUid={UID}&linked={linked}"
#define IMR_CRU_URL_APPLY_LIMITS                                IMR_VERSION "/commands/applyLimits"
#define IMR_LAST_REQUEST_LIMIT_MS                               10
#define IMR_HCU_ACTION_PROCESS_TIME_WARNING_MS                  500

// Filesystem Path
#define PATH_HOME                                               "/home/user/Imaxeon"
#define PATH_USER                                               "/IMAX_USER"
#define PATH_USB                                                "/media/user"
#define PATH_RESOURCES                                          PATH_HOME "/bin/resources"
#define PATH_RESOURCES_DOC                                      PATH_RESOURCES "/Doc"
#define PATH_RESOURCES_DEFAULT_CONFIG                           PATH_RESOURCES "/DefaultConfig"
#define PATH_RESOURCES_FONTS                                    PATH_RESOURCES "/Fonts"
#define PATH_RESOURCES_SOUND                                    PATH_RESOURCES "/Sound"
#define PATH_RESOURCES_IMAGES                                   PATH_RESOURCES "/Images"
#define PATH_RESOURCES_PHRASES                                  PATH_RESOURCES "/Phrases"
#define PATH_RESOURCES_PHRASE_I18N_BUNDLE_FILENAME              "i18n-bundle.json"

#define PATH_VERSION_INFO                                       PATH_HOME "/doc/version.inf"
#define PATH_INTRO_VIDEO                                        PATH_HOME "/media/title.mp4"
#define PATH_UPGRADE_DIR                                        PATH_HOME "/upgrade/"
#define PATH_LOG_DIR                                            PATH_USER "/log/"
#define PATH_INFO_HARDWARE                                      PATH_USER "/info/hardware.inf"
#define PATH_CONFIG_LOCAL                                       PATH_USER "/db/local.cfg"
#define PATH_CONFIG_GLOBAL                                      PATH_USER "/db/global.cfg"
#define PATH_CONFIG_CAPABILITIES                                PATH_USER "/db/capabilities.cfg"
#define PATH_LAST_DIGEST                                        PATH_USER "/db/lastDigest.json"
#define PATH_LAST_ALERTS                                        PATH_USER "/db/lastAlerts.json"
#define PATH_LAST_ACTIVE_ALERTS                                 PATH_USER "/db/lastActiveAlerts.json"
#define PATH_LAST_MCU_DIGEST                                    PATH_USER "/db/lastMcuDigest.json"
#define PATH_LAST_BMS_DIGESTS                                   PATH_USER "/db/lastBmsDigests.json"
#define PATH_SHUTDOWN_INFO_FILE                                 PATH_USER "/db/shutdown.inf"
#define PATH_LAST_STATUS_FILE                                   PATH_USER "/db/laststatus.json"
#define PATH_INJECTION_PLOT_DATA_PREFIX                         PATH_USER "/db/injection_plot"
#define PATH_INJECTION_PLOT_DATA_EXT                            ".dat"
#define PATH_TEMP_UPGRADE                                       PATH_USER "/temp/upgrade"

// Scripts
#define PATH_UPGRADE_CHECK_FILE                                 PATH_HOME "/script/upgrade_check_file.sh"
#define PATH_UPGRADE_EXTRACT                                    PATH_HOME "/script/upgrade_extract.sh"
#define PATH_UPGRADE_LOAD_IMAGE                                 PATH_HOME "/script/upgrade_load_image.sh"
#define PATH_SET_SLEEP_TIME                                     PATH_HOME "/script/set_sleep_time.sh"
#define PATH_WAKE_UP                                            PATH_HOME "/script/wake_up.sh"
#define PATH_SET_DATETIME                                       PATH_HOME "/script/set_datetime.sh"
#define PATH_SYNC_DATETIME                                      PATH_HOME "/script/sync_datetime.sh"
#define PATH_SET_BRIGHTNESS                                     PATH_HOME "/script/set_brightness.sh"
#define PATH_SET_DEV_ENV                                        PATH_HOME "/script/set_dev_env.sh"
#define PATH_SET_NETWORK                                        PATH_HOME "/script/set_network.sh"
#define PATH_SAVE_USER_DATA                                     PATH_HOME "/script/save_user_data.sh"
#define PATH_SAVE_SCREENSHOT                                    PATH_HOME "/script/save_screenshot.sh"
#define PATH_FACTORY_RESET                                      PATH_HOME "/script/factory_reset.sh"
#define PATH_PING_TEST                                          PATH_HOME "/script/ping_test.sh"
#define PATH_DELETE_ALL_USB_DIRS                                PATH_HOME "/script/delete_all_usb_dirs.sh"
#define PATH_SYSTEM_SHUTDOWN                                    PATH_HOME "/script/system_shutdown.sh"
#define PATH_SYSTEM_REBOOT                                      PATH_HOME "/script/system_reboot.sh"
#define PATH_GET_TEMPERATURE_PARAMS                             PATH_HOME "/script/get_temperature_params.sh"
#define PATH_RECONNECT_MCU_FTDI                                 PATH_HOME "/script/reconnect_Mcu_FTDI.sh"
#define PATH_IWCONFIG_TEST                                      PATH_HOME "/script/iwconfig_test.sh"
#define PATH_CHECK_ETHERNET_INTERFACE_NAME                      PATH_HOME "/script/check_ethernet_interface_name.sh"
#define PATH_CONTROL_USB                                        PATH_HOME "/script/control_usbs.sh"
#define PATH_UNBIND_USB                                         PATH_HOME "/script/unbind_usb.sh"
#define PATH_SET_TOUCHSCREEN                                    PATH_HOME "/script/set_touchscreen.sh"

// Log
#define LOG_NUM_BACKUP_FILES                                    3
#define LOG_LINE_CHAR_LIMIT                                     1000
#define LOG_MIN_SIZE_BYTES                                      50000
#define LOG_MID_SIZE_BYTES                                      1000000
#define LOG_LRG_SIZE_BYTES                                      5000000

// Barcode Reader
#define BARCODE_READER_BYTES_WRITTEN_TIMEOUT_MS                 25
#define BARCODE_READER_TEST_MSG_RX_TIMEOUT_MS                   3000
#define BARCODE_READER_SLEEP_TIMEOUT_MS                         5000
#define BARCODE_PREFIX_LEN_MAX                                  20
#define BARCODE_NUMBER_OF_FIELDS                                2
#define BARCODE_LOT_BATCH_DELIMETER                             "\035"
#define BARCODE_IDENTIFIER_LEN                                  2
#define BARCODE_PREFIX_DATA_LEN                                 14
#define BARCODE_EXPIRATION_DATE_LEN                             6

// Exam Monitor
#define EXAM_TIMEOUT_MONITOR_INTERVAL_MS                        60000

// Alert Monitor Period
#define ALERT_LOGGER_MONITOR_POLL_INTERVAL_MS                   1000

// System Monitor Period
#define SYSTEM_MONITOR_INTERVAL_MS_TIME_SHIFT                   5000
#define SYSTEM_MONITOR_INTERVAL_MS_MAIN_HDD                     30000
#define SYSTEM_MONITOR_INTERVAL_MS_USER_HDD                     30000
#define SYSTEM_MONITOR_INTERVAL_MS_USB_STICK_INSERTED           5000
#define SYSTEM_MONITOR_INTERVAL_MS_HCU_TEMPERATURE_HW           60000
#define SYSTEM_MONITOR_INTERVAL_MS_CPU_MEM_USAGE                30000
#define SYSTEM_MONITOR_INTERVAL_MS_SETUP_CRU_LINK               30000
#define SYSTEM_MONITOR_INTERVAL_MS_IWCONFIG                     5000
#define SYSTEM_MONITOR_SAMPLE_COUNT_IWCONFIG                    12
#define SYSTEM_MONITOR_ALERT_THRESHOLD_IWCONFIG                 3
#define SYSTEM_MONITOR_HISTORY_COUNT_IWCONFIG                   6

// FTDI
#define FTDI_STOPCOCK_VENDOR_ID                                 0x0403
#define FTDI_STOPCOCK_PRODUCT_ID                                0x6001
#define FTDI_MCU_VENDOR_ID                                      0x0403
#define FTDI_MCU_PRODUCT_ID                                     0x6001

#define BARCODE_READER_PORT_NAME_PART                           "ttyS0"
#define STOP_BUTTON_LONG_PRESSED_INTERVAL_MS                    4000

// Injection Plot
#define INJECTION_REVIEW_PLOT_DRAW_INTERVAL_MS                  10
#define INJECTION_REVIEW_PLOT_DRAW_SAMPLES                      50

// MCU Simulator
#define MCU_SIM_STOPCOCK_DELAY_MS                               300
#define MCU_SIM_INJECTION_PRESSURE_RANDOM_NOISE_KPA             100
#define MCU_SIM_INJECTION_PRESSURE_UPDATE_DELAY_SAMPLES         5
#define MCU_SIM_SYRINGE_STATE_UPDATE_INTERVAL_MS                100
#define MCU_SIM_SYRINGE_VOLUME_UPDATE_INTERVAL_MS               10
#define MCU_SIM_FRESH_MUDS_VOLUME                               (SYRINGE_VOLUME_MAX + 10)

// MCU STL2CLI
#define MCU_INJECT_DIGEST_PHASE_NUM_PARAMS                      4 // S0Vol, C1Vol, C2Vol, Duration

// Web Application
#define WEB_APPLICATION_HOST_PORT                               5000

//HCUClockBatteryDead alert - number of days that must have passed before raising alert
#define HCU_CLOCK_BATTERY_DEAD_MAX_DAYS                             730

#endif // ENVIRONMENT_H
