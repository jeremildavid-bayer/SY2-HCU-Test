#include "Apps/AppManager.h"
#include "McuAlarmMonitor.h"
#include "Common/Translator.h"
#include "DataServices/Mcu/DS_McuData.h"
#include "DataServices/Mcu/DS_McuAction.h"
#include "DataServices/Alert/DS_AlertAction.h"

McuAlarmMonitor::McuAlarmMonitor(QObject *parent, EnvGlobal *env_) :
    QObject(parent),
    env(env_)
{
    envLocal = new EnvLocal("DS_Mcu-AlarmMonitor", "MCU_ALARM_MONITOR");

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });
}

McuAlarmMonitor::~McuAlarmMonitor()
{
    delete envLocal;
}

void McuAlarmMonitor::slotAppInitialised()
{
    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_AlarmCodes, this, [=]() {
        slotHandleMcuAlarms();
    });
}

QVariantMap McuAlarmMonitor::getHcuAlertFromMcuAlarm(McuAlarm::AlarmId alarmId)
{
    QVariantMap ret;
    QString mcuAlarmName = McuAlarm::getAlarmName(alarmId);

    switch (alarmId)
    {
    case McuAlarm::ALARM_POST_BAD_SYSTEM_CRC:
        ret = env->ds.alertAction->prepareAlert("ChecksumError", mcuAlarmName);
        break;
    case McuAlarm::ALARM_I2C_SC_FAULT:
        ret = env->ds.alertAction->prepareAlert("MCULostCommWithSCB", mcuAlarmName);
        break;
    case McuAlarm::ALARM_STOP_BTN_FAULT:
    case McuAlarm::ALARM_POST_STOP_BTN_PRESSED:
        ret = env->ds.alertAction->prepareAlert("AllStopButtonFault", mcuAlarmName);
        break;
    case McuAlarm::ALARM_HEAT_MAINTAINER_POWER_FAULT_CORE:
    case McuAlarm::ALARM_HEAT_MAINTAINER_POWER_FAULT_DOOR:
    case McuAlarm::ALARM_HEAT_MAINTAINER_DIGITAL_POWER_FAULT_CORE:
    case McuAlarm::ALARM_HEAT_MAINTAINER_DIGITAL_POWER_FAULT_DOOR:
        ret = env->ds.alertAction->prepareAlert("HeatMaintainerPowerFault", mcuAlarmName);
        break;
    case McuAlarm::ALARM_SC_WATCH_DOG_RESET:
    case McuAlarm::ALARM_SC_HBRIDGE_FAULT_S0:
    case McuAlarm::ALARM_SC_HBRIDGE_FAULT_C1:
    case McuAlarm::ALARM_SC_HBRIDGE_FAULT_C2:
    case McuAlarm::ALARM_SC_ENCODER_FAULT_S0:
    case McuAlarm::ALARM_SC_ENCODER_FAULT_C1:
    case McuAlarm::ALARM_SC_ENCODER_FAULT_C2:
    case McuAlarm::ALARM_SC_TIMEOUT_FAULT_S0:
    case McuAlarm::ALARM_SC_TIMEOUT_FAULT_C1:
    case McuAlarm::ALARM_SC_TIMEOUT_FAULT_C2:
        ret = env->ds.alertAction->prepareAlert("StopcockHardwareFault", mcuAlarmName);
        break;
    case McuAlarm::ALARM_SC_UNINTENDED_MOTION_DETECTED_S0:
        ret = env->ds.alertAction->prepareAlert("StopcockUnintendedMotionDetected", "RS0");
        break;
    case McuAlarm::ALARM_SC_UNINTENDED_MOTION_DETECTED_C1:
        ret = env->ds.alertAction->prepareAlert("StopcockUnintendedMotionDetected", "RC1");
        break;
    case McuAlarm::ALARM_SC_UNINTENDED_MOTION_DETECTED_C2:
        ret = env->ds.alertAction->prepareAlert("StopcockUnintendedMotionDetected", "RC2");
        break;
    case McuAlarm::ALARM_INLET_AIR_CAL_NEEDED_S0:
    case McuAlarm::ALARM_INLET_AIR_SENSOR_FAULT_S0:
        ret = env->ds.alertAction->prepareAlert("RS0InletAirDetectorFault", mcuAlarmName);
        break;
    case McuAlarm::ALARM_INLET_AIR_CAL_NEEDED_C1:
    case McuAlarm::ALARM_INLET_AIR_SENSOR_FAULT_C1:
        ret = env->ds.alertAction->prepareAlert("RC1InletAirDetectorFault", mcuAlarmName);
        break;
    case McuAlarm::ALARM_INLET_AIR_CAL_NEEDED_C2:
    case McuAlarm::ALARM_INLET_AIR_SENSOR_FAULT_C2:
        ret = env->ds.alertAction->prepareAlert("RC2InletAirDetectorFault", mcuAlarmName);
        break;
    case McuAlarm::ALARM_OUTLET_AIR_SENSOR_FAULT:
        ret = env->ds.alertAction->prepareAlert("OutletAirDetectorFault", mcuAlarmName);
        break;
    case McuAlarm::ALARM_POST_LOW_POWER_RESET:
    case McuAlarm::ALARM_POST_5V_LEVEL_CHECK_FAILED:
    case McuAlarm::ALARM_POST_12V_LEVEL_CHECK_FAILED:
    case McuAlarm::ALARM_POST_36V_LEVEL_CHECK_FAILED:
    case McuAlarm::ALARM_POST_LED_VOLTAGE_CHECK_FAILED:
        ret = env->ds.alertAction->prepareAlert("POSTPowerFault", mcuAlarmName);
        break;
    case McuAlarm::ALARM_VMT_OUT_RANGE:
        ret = env->ds.alertAction->prepareAlert("MotorPowerFault", mcuAlarmName);
        break;
    case McuAlarm::ALARM_BASE_OVER_TEMPERATURE:
        ret = env->ds.alertAction->prepareAlert("BaseboardTemperatureFault", mcuAlarmName);
        break;
    case McuAlarm::ALARM_PRESSURE_CAL_NEEDED_S0:
    case McuAlarm::ALARM_PRESSURE_CAL_NEEDED_C1:
    case McuAlarm::ALARM_PRESSURE_CAL_NEEDED_C2:
        ret = env->ds.alertAction->prepareAlert("PressureCalibrationNeeded", mcuAlarmName);
        break;
    case McuAlarm::ALARM_MOTOR_PLUNGER_CAL_NEEDED_S0:
    case McuAlarm::ALARM_MOTOR_CAL_NEEDED_S0:
    case McuAlarm::ALARM_MOTOR_HALL_SENSOR_OUT_RANGE_S0:
    case McuAlarm::ALARM_MOTOR_ELECTRICAL_FAULT_S0:
    case McuAlarm::ALARM_MOTOR_FINAL_DRIVE_ENCODER_FAULT_S0:
    case McuAlarm::ALARM_MOTOR_HOME_SENSOR_FAULT_S0:
        ret = env->ds.alertAction->prepareAlert("RS0MotorModuleFault", mcuAlarmName);
        break;
    case McuAlarm::ALARM_MOTOR_PLUNGER_CAL_NEEDED_C1:
    case McuAlarm::ALARM_MOTOR_CAL_NEEDED_C1:
    case McuAlarm::ALARM_MOTOR_HALL_SENSOR_OUT_RANGE_C1:
    case McuAlarm::ALARM_MOTOR_ELECTRICAL_FAULT_C1:
    case McuAlarm::ALARM_MOTOR_FINAL_DRIVE_ENCODER_FAULT_C1:
    case McuAlarm::ALARM_MOTOR_HOME_SENSOR_FAULT_C1:
        ret = env->ds.alertAction->prepareAlert("RC1MotorModuleFault", mcuAlarmName);
        break;
    case McuAlarm::ALARM_MOTOR_PLUNGER_CAL_NEEDED_C2:
    case McuAlarm::ALARM_MOTOR_CAL_NEEDED_C2:
    case McuAlarm::ALARM_MOTOR_HALL_SENSOR_OUT_RANGE_C2:
    case McuAlarm::ALARM_MOTOR_ELECTRICAL_FAULT_C2:
    case McuAlarm::ALARM_MOTOR_FINAL_DRIVE_ENCODER_FAULT_C2:
    case McuAlarm::ALARM_MOTOR_HOME_SENSOR_FAULT_C2:
        ret = env->ds.alertAction->prepareAlert("RC2MotorModuleFault", mcuAlarmName);
        break;
    case McuAlarm::ALARM_MOTOR_ZERO_VOLUME_NOT_SET_S0:
    case McuAlarm::ALARM_MOTOR_ZERO_VOLUME_NOT_SET_C1:
    case McuAlarm::ALARM_MOTOR_ZERO_VOLUME_NOT_SET_C2:
        ret = env->ds.alertAction->prepareAlert("MotorModuleZeroVolumeNotSet", mcuAlarmName);
        break;
    case McuAlarm::ALARM_MOTOR_UNEXPECTED_PLUGNER_TRANSITION_S0:
    case McuAlarm::ALARM_MOTOR_UNEXPECTED_PLUGNER_TRANSITION_C1:
    case McuAlarm::ALARM_MOTOR_UNEXPECTED_PLUGNER_TRANSITION_C2:
        ret = env->ds.alertAction->prepareAlert("MotorModuleUnexpectedPlungerTransition", mcuAlarmName);
        break;
    case McuAlarm::ALARM_I2C_MOTOR_MUX_FAULT:
        ret = env->ds.alertAction->prepareAlert("MotorI2CMuxFault", mcuAlarmName);
        break;
    case McuAlarm::ALARM_I2C_MOTOR_ADC_FAULT_S0:
    case McuAlarm::ALARM_I2C_MOTOR_BAD_ADDRESS_S0:
    case McuAlarm::ALARM_I2C_MOTOR_FRAM_FAULT_S0:
    case McuAlarm::ALARM_I2C_MOTOR_IO_FAULT_S0:
    case McuAlarm::ALARM_I2C_MOTOR_PLUNGER_POT_FAULT_S0:
    case McuAlarm::ALARM_I2C_MOTOR_LOCK_UINLOCK_POT_FAULT_S0:
        ret = env->ds.alertAction->prepareAlert("RS0MotorModuleI2CFault", mcuAlarmName);
        break;
    case McuAlarm::ALARM_I2C_MOTOR_ADC_FAULT_C1:
    case McuAlarm::ALARM_I2C_MOTOR_BAD_ADDRESS_C1:
    case McuAlarm::ALARM_I2C_MOTOR_FRAM_FAULT_C1:
    case McuAlarm::ALARM_I2C_MOTOR_IO_FAULT_C1:
    case McuAlarm::ALARM_I2C_MOTOR_PLUNGER_POT_FAULT_C1:
    case McuAlarm::ALARM_I2C_MOTOR_LOCK_UINLOCK_POT_FAULT_C1:
        ret = env->ds.alertAction->prepareAlert("RC1MotorModuleI2CFault", mcuAlarmName);
        break;
    case McuAlarm::ALARM_I2C_MOTOR_ADC_FAULT_C2:
    case McuAlarm::ALARM_I2C_MOTOR_BAD_ADDRESS_C2:
    case McuAlarm::ALARM_I2C_MOTOR_FRAM_FAULT_C2:
    case McuAlarm::ALARM_I2C_MOTOR_IO_FAULT_C2:
    case McuAlarm::ALARM_I2C_MOTOR_PLUNGER_POT_FAULT_C2:
    case McuAlarm::ALARM_I2C_MOTOR_LOCK_UINLOCK_POT_FAULT_C2:
        ret = env->ds.alertAction->prepareAlert("RC2MotorModuleI2CFault", mcuAlarmName);
        break;
    case McuAlarm::ALARM_I2C_GENERAL_BASE_ADC_FAULT:
    case McuAlarm::ALARM_I2C_GENERAL_BASE_IO_FAULT:
    case McuAlarm::ALARM_I2C_GENERAL_INLET_AIR_DETECT_COMM_FAULT_S0:
    case McuAlarm::ALARM_I2C_GENERAL_INLET_AIR_DETECT_COMM_FAULT_C1:
    case McuAlarm::ALARM_I2C_GENERAL_INLET_AIR_DETECT_COMM_FAULT_C2:
    case McuAlarm::ALARM_I2C_GENERAL_SUDS_COMM_FAULT:
    case McuAlarm::ALARM_I2C_GENERAL_LED_SUDS_COMM_FAULT:
    case McuAlarm::ALARM_I2C_GENERAL_MUX_FAULT:
    case McuAlarm::ALARM_I2C_GENERAL_BAD_ADDRESS:
        ret = env->ds.alertAction->prepareAlert("CriticalI2CFault", mcuAlarmName);
        break;
    case McuAlarm::ALARM_I2C_GENERAL_HEAT_MAINTAINER_COMM_FAULT_CORE:
    case McuAlarm::ALARM_I2C_GENERAL_HEAT_MAINTAINER_COMM_FAULT_DOOR:
    case McuAlarm::ALARM_I2C_GENERAL_WASTE_CONTAINER_DETECT_COMM_FAULT:
    case McuAlarm::ALARM_I2C_GENERAL_WASTE_CONTAINER_LEVEL_COMM_FAULT:
    case McuAlarm::ALARM_I2C_GENERAL_DOOR_LOCK_COMM_FAULT:
    case McuAlarm::ALARM_I2C_GENERAL_LED_DOOR_COMM_FAULT:
    case McuAlarm::ALARM_I2C_GENERAL_LED_TOP_COMM_FAULT:
    case McuAlarm::ALARM_I2C_GENERAL_BASE_FAN_FAULT:
    case McuAlarm::ALARM_I2C_GENERAL_BATTERY_MANAGEMENT_SYSTEM_A:
    case McuAlarm::ALARM_I2C_GENERAL_BATTERY_MANAGEMENT_SYSTEM_B:
        ret = env->ds.alertAction->prepareAlert("GeneralI2CFault", mcuAlarmName);
        break;
    case McuAlarm::ALARM_POST_SC_VERSION_MISMATCH:
        ret = env->ds.alertAction->prepareAlert("IncompatibleSCBConnected", mcuAlarmName);
        break;
    case McuAlarm::ALARM_HCU_SHUTDOWN_FAILED:
        ret = env->ds.alertAction->prepareAlert("HCUShutdownFailed", mcuAlarmName);
        break;
    case McuAlarm::ALARM_SUDS_CAL_NEEDED:
    case McuAlarm::ALARM_SUDS_SENSOR_FAULT:
        ret = env->ds.alertAction->prepareAlert("SUDSSensorFault", mcuAlarmName);
        break;
    case McuAlarm::ALARM_MOTOR_LOST_POSITION_S0:
        ret = env->ds.alertAction->prepareAlert("MotorPositionFault", "RS0");
        break;
    case McuAlarm::ALARM_MOTOR_LOST_POSITION_C1:
        ret = env->ds.alertAction->prepareAlert("MotorPositionFault", "RC1");
        break;
    case McuAlarm::ALARM_MOTOR_LOST_POSITION_C2:
        ret = env->ds.alertAction->prepareAlert("MotorPositionFault", "RC2");
        break;
    case McuAlarm::ALARM_SC_ENGAGEMENT_FAULT_S0:
        ret = env->ds.alertAction->prepareAlert("StopcockEngagementFault", "RS0");
        break;
    case McuAlarm::ALARM_SC_ENGAGEMENT_FAULT_C1:
        ret = env->ds.alertAction->prepareAlert("StopcockEngagementFault", "RC1");
        break;
    case McuAlarm::ALARM_SC_ENGAGEMENT_FAULT_C2:
        ret = env->ds.alertAction->prepareAlert("StopcockEngagementFault", "RC2");
        break;
    case McuAlarm::ALARM_POST_ADVANCE_BTN_PRESSED:
        ret = env->ds.alertAction->prepareAlert("AdvanceSalineButtonFault", mcuAlarmName);
        break;
    case McuAlarm::ALARM_POST_MCU_SC_CLOCK_CHK_FAILED:
        ret = env->ds.alertAction->prepareAlert("MCUClockFaultWithSCB", mcuAlarmName);
        break;
    case McuAlarm::ALARM_SC_CRC_FAULT:
        ret = env->ds.alertAction->prepareAlert("StopcockChecksumError", mcuAlarmName);
        break;
    case McuAlarm::ALARM_DOOR_MECHANISM_FAILED:
    case McuAlarm::ALARM_DOOR_OPEN_FAULT:
        ret = env->ds.alertAction->prepareAlert("DoorOpenFault", mcuAlarmName);
        break;
    case McuAlarm::ALARM_POST_WATCH_DOG_RESET:
        ret = env->ds.alertAction->prepareAlert("WatchdogResetFault", mcuAlarmName);
        break;
    default:
        LOG_WARNING("Unexpected MCU Alarm raised. Alarm=%s\n", mcuAlarmName.CSTR());
        ret = env->ds.alertAction->prepareAlert("UnexpectedMCUAlarm", mcuAlarmName);
        break;
    }

    ret.insert("Reporter", "MCU");
    return ret;
}

void McuAlarmMonitor::slotHandleMcuAlarms()
{
    QByteArray alarmBits = env->ds.mcuData->getAlarmCodes();

    // Add MCU alerts
    QList<McuAlarm::AlarmId> activeAlarms = McuAlarm::getAlarmListFromAlarmBits(alarmBits);
    foreach (McuAlarm::AlarmId alarmId, activeAlarms)
    {
        QVariantMap hcuAlert = getHcuAlertFromMcuAlarm(alarmId);
        if (!env->ds.alertAction->isActivated(hcuAlert))
        {
            LOG_ERROR("MCU Alarm(%s) is activated. Raising HCU Alert(%s)\n", McuAlarm::getAlarmName(alarmId).CSTR(), hcuAlert[_L("CodeName")].toString().CSTR());
            env->ds.alertAction->activate(hcuAlert);
        }
    }

    // Active alarms are all recorded: Clear all alarms. Note: MCU never clears the alarm bit itself
    if (activeAlarms.length() > 0)
    {
        env->ds.mcuAction->actAlarmClear();
    }
}



