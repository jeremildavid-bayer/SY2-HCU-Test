#ifndef DS_MCU_DEF_H
#define DS_MCU_DEF_H

#include "Common/Common.h"
#include "Internal/McuAlarm.h"

#define MSG_START_CHAR          '>'
#define MSG_BODY_START_CHAR     '@'
#define MSG_END_CHAR            '\r'
#define MSG_REPLY_START         '#'

class DS_McuDef
{
public:

    // ==============================================
    // Data Enumerations
    enum LinkState
    {
        LINK_STATE_UNKNOWN = 0,
        LINK_STATE_DISCONNECTED,
        LINK_STATE_CONNECTING, // Connect start is initiated
        LINK_STATE_RECOVERING, // Connected, hand shaking..
        LINK_STATE_CONNECTED, // Hearbeat monitor is active (Digest is monitored)
    };

    enum BatteryLevel
    {
        BATTERY_LEVEL_UNKNOWN = 0,
        BATTERY_LEVEL_NO_BATTERY,
        BATTERY_LEVEL_FULL, // Not charging from AC
        BATTERY_LEVEL_HIGH,
        BATTERY_LEVEL_MEDIUM,
        BATTERY_LEVEL_LOW, //
        BATTERY_LEVEL_FLAT, // Arming and injection is still allowed. Notify to user
        BATTERY_LEVEL_DEAD, // Arming is not allowed. Injection continues. Warning to user
        BATTERY_LEVEL_CRITICAL, // Injection is aborted, shutdown sequence performed
    };

    enum DoorState
    {
        DOOR_UNKNOWN = 0,
        DOOR_CLOSED,
        DOOR_OPEN
    };

    enum WasteBinState
    {
        WASTE_BIN_STATE_UNKNOWN = 0,
        WASTE_BIN_STATE_MISSING,
        WASTE_BIN_STATE_LOW_FILLED,
        WASTE_BIN_STATE_HIGH_FILLED,
        WASTE_BIN_STATE_FULLY_FILLED,
        WASTE_BIN_STATE_COMM_DOWN
    };

    enum StopcockPos
    {
        STOPCOCK_POS_UNKNOWN = 0,
        STOPCOCK_POS_NONE,
        STOPCOCK_POS_FILL,
        STOPCOCK_POS_INJECT,
        STOPCOCK_POS_CLOSED,
        STOPCOCK_POS_MOVING,
        STOPCOCK_POS_DISENGAGED,
    };

    typedef QList<StopcockPos> StopcockPosAll;

    enum PlungerState
    {
        PLUNGER_STATE_UNKNOWN = 0,
        PLUNGER_STATE_ENGAGED,
        PLUNGER_STATE_DISENGAGED,
        PLUNGER_STATE_LOCKFAILED
    };

    typedef QList<PlungerState> PlungerStates;

    enum SyringeState
    {
        SYRINGE_STATE_UNKNOWN = 0,
        SYRINGE_STATE_PROCESSING,
        SYRINGE_STATE_COMPLETED,
        SYRINGE_STATE_USER_ABORT,
        SYRINGE_STATE_AIR_DETECTED,
        SYRINGE_STATE_LOST_PLUNGER,
        SYRINGE_STATE_PLUNGER_ENGAGE_FAULT,
        SYRINGE_STATE_PLUNGER_DISENGAGE_FAULT,
        SYRINGE_STATE_MOTOR_FAIL,
        SYRINGE_STATE_OVER_PRESSURE,
        SYRINGE_STATE_OVER_CURRENT,
        SYRINGE_STATE_SUDS_REMOVED,
        SYRINGE_STATE_SUDS_INSERTED,
        SYRINGE_STATE_MUDS_REMOVED,
        SYRINGE_STATE_INSUFFICIENT_FLUID,
        SYRINGE_STATE_TIMEOUT,
        SYRINGE_STATE_HOME_SENSOR_MISSING,
        SYRINGE_STATE_INVALID_STATE,
        SYRINGE_STATE_BAD_DATA,
        SYRINGE_STATE_FRAM_FAULT,
        SYRINGE_STATE_STOP_PENDING,
        SYRINGE_STATE_SPIKE_MISSING,
        SYRINGE_STATE_BAD_STOPCOCK_POSITION
    };

    typedef QList<SyringeState> SyringeStates;

    enum InjectorState
    {
        INJECTOR_STATE_IDLE = 0,
        INJECTOR_STATE_READY_START,
        INJECTOR_STATE_DELIVERING,
        INJECTOR_STATE_HOLDING,
        INJECTOR_STATE_PHASE_PAUSED,
        INJECTOR_STATE_COMPLETING,
        INJECTOR_STATE_COMPLETED
    };

    enum InjectionCompleteReason
    {
        INJECTION_COMPLETE_REASON_UNKNOWN,
        INJECTION_COMPLETE_REASON_COMPLETED_NORMAL,
        INJECTION_COMPLETE_REASON_COMPLETED_ALLSTOP_ABORT,
        INJECTION_COMPLETE_REASON_COMPLETED_USER_ABORT,
        INJECTION_COMPLETE_REASON_COMPLETED_HOLD_TIMEOUT,
        INJECTION_COMPLETE_REASON_COMPLETED_ALARM_ABORT,
        INJECTION_COMPLETE_REASON_COMPLETED_OVER_PRESSURE,
        INJECTION_COMPLETE_REASON_COMPLETED_OVER_CURRENT,
        INJECTION_COMPLETE_REASON_COMPLETED_AIR_DETECTED,
        INJECTION_COMPLETE_REASON_COMPLETED_SUDS_MISSING,
        INJECTION_COMPLETE_REASON_COMPLETED_MOTOR_STALL_1,
        INJECTION_COMPLETE_REASON_COMPLETED_MOTOR_STALL_2,
        INJECTION_COMPLETE_REASON_COMPLETED_MOTOR_STALL_3,
        INJECTION_COMPLETE_REASON_COMPLETED_MOTOR_SLIP_1,
        INJECTION_COMPLETE_REASON_COMPLETED_MOTOR_SLIP_2,
        INJECTION_COMPLETE_REASON_COMPLETED_MOTOR_SLIP_3,
        INJECTION_COMPLETE_REASON_COMPLETED_HCU_COMM_DOWN,
        INJECTION_COMPLETE_REASON_COMPLETED_OVER_TEMPERATURE,
        INJECTION_COMPLETE_REASON_COMPLETED_BATTERY_CRITICAL,
        INJECTION_COMPLETE_REASON_COMPLETED_STOPCOCK_COMM_ERROR,
        INJECTION_COMPLETE_REASON_COMPLETED_MUDS_UNLATCHED,

        INJECTION_COMPLETE_REASON_DISARMED_SUDS_MISSING,
        INJECTION_COMPLETE_REASON_DISARMED_HCU_ABORT,
        INJECTION_COMPLETE_REASON_DISARMED_STOP_BUTTON_ABORT,
        INJECTION_COMPLETE_REASON_DISARMED_ARM_TIMEOUT,
        INJECTION_COMPLETE_REASON_DISARMED_MUDS_UNLATCHED
    };

    enum InjectionPhaseType
    {
        INJECTION_PHASE_TYPE_NONE = 0,
        INJECTION_PHASE_TYPE_PAUSE,
        INJECTION_PHASE_TYPE_CONTRAST1,
        INJECTION_PHASE_TYPE_CONTRAST2,
        INJECTION_PHASE_TYPE_SALINE,
        INJECTION_PHASE_TYPE_DUAL1,
        INJECTION_PHASE_TYPE_DUAL2
    };

    enum InjectControlType
    {
        INJECT_CONTROL_START = 0,
        INJECT_CONTROL_HOLD,
        INJECT_CONTROL_ABORT,
        INJECT_CONTROL_JUMP,
    };

    enum PowerControlType
    {
        POWER_CONTROL_TYPE_OFF = 0,
        POWER_CONTROL_TYPE_REBOOT,
        POWER_CONTROL_TYPE_NONE
    };

    enum SyringeActionType
    {
        SYRINGE_ACTION_TYPE_UNKNOWN = 0,
        SYRINGE_ACTION_TYPE_STOP,
        SYRINGE_ACTION_TYPE_ENGAGE,
        SYRINGE_ACTION_TYPE_DISENGAGE,
        SYRINGE_ACTION_TYPE_FILL_SOD,
        SYRINGE_ACTION_TYPE_FILL_BOTTLE_SHUFFLE,
        SYRINGE_ACTION_TYPE_FILL_WITH_NO_AIR,
        SYRINGE_ACTION_TYPE_INJECT,
        SYRINGE_ACTION_TYPE_PRIME_AUTO,
        SYRINGE_ACTION_TYPE_PRIME_SOD,
        SYRINGE_ACTION_TYPE_PURGE
    };

    enum LedControlType
    {
        LED_CONTROL_TYPE_UNKNOWN = 0,
        LED_CONTROL_TYPE_NO_CHANGE,
        LED_CONTROL_TYPE_OFF,
        LED_CONTROL_TYPE_SET
    };

    enum BottleBubbleDetectorState
    {
        BOTTLE_BUBBLE_DETECTOR_STATE_UNKNOWN = 0,
        BOTTLE_BUBBLE_DETECTOR_STATE_MISSING,
        BOTTLE_BUBBLE_DETECTOR_STATE_AIR,
        BOTTLE_BUBBLE_DETECTOR_STATE_FLUID
    };

    typedef QList<DS_McuDef::BottleBubbleDetectorState> BottleBubbleDetectorStates;

    enum OutletDoorState
    {
        OUTLET_DOOR_STATE_UNKNOWN = 0,
        OUTLET_DOOR_STATE_OPEN,
        OUTLET_DOOR_STATE_CLOSED
    };


    enum PressureCalibrationState
    {
        PRESSURE_CAL_STATE_IDLE = 0,
        PRESSURE_CAL_STATE_STARTED,

        PRESSURE_CAL_STATE_DISENGAGE_STARTED,
        PRESSURE_CAL_STATE_DISENGAGE_PROGRESS,
        PRESSURE_CAL_STATE_DISENGAGE_FAILED,
        PRESSURE_CAL_STATE_DISENGAGE_COMPLETED,

        PRESSURE_CAL_STATE_FIND_PLUNGER_STARTED,
        PRESSURE_CAL_STATE_FIND_PLUNGER_PROGRESS,
        PRESSURE_CAL_STATE_FIND_PLUNGER_FAILED,
        PRESSURE_CAL_STATE_FIND_PLUNGER_COMPLETED,

        PRESSURE_CAL_STATE_INJECT_STAGE_STARTED,
        PRESSURE_CAL_STATE_INJECT_STAGE_PROGRESS,
        PRESSURE_CAL_STATE_INJECT_STAGE_FAILED,
        PRESSURE_CAL_STATE_INJECT_STAGE_COMPLETED,

        PRESSURE_CAL_STATE_FINAL_DISENGAGE_STARTED,
        PRESSURE_CAL_STATE_FINAL_DISENGAGE_PROGRESS,
        PRESSURE_CAL_STATE_FINAL_DISENGAGE_FAILED,
        PRESSURE_CAL_STATE_FINAL_DISENGAGE_COMPLETED,

        PRESSURE_CAL_STATE_FAILED,
        PRESSURE_CAL_STATE_DONE,
    };

    enum HeatMaintainerState
    {
        HEAT_MAINTAINER_STATE_UNKNOWN = 0,
        HEAT_MAINTAINER_STATE_ENABLED_ON,  // enabled and running
        HEAT_MAINTAINER_STATE_ENABLED_OFF,  // enabled and not running
        HEAT_MAINTAINER_STATE_DISABLED, // disabled by user
        HEAT_MAINTAINER_STATE_CUTOFF,  // turned off by MCU as temperature is too high
    };

    enum FillType
    {
        FILL_TYPE_SOD = 0,
        FILL_TYPE_NORMAL,
        FILL_TYPE_FILL_TO_PROTOCOL
    };

    enum AdaptiveFlowState
    {
        ADAPTIVE_FLOW_STATE_OFF,
        ADAPTIVE_FLOW_STATE_ACTIVE,
        ADAPTIVE_FLOW_STATE_CRITICAL
    };

    enum PressureLimitSensitivityType
    {
        PRESSURE_LIMIT_SENSITIVITY_TYPE_INVALID = 0,
        PRESSURE_LIMIT_SENSITIVITY_TYPE_UNKNOWN = 1,
        PRESSURE_LIMIT_SENSITIVITY_TYPE_LOW = 25,
        PRESSURE_LIMIT_SENSITIVITY_TYPE_MEDIUM = 50,
        PRESSURE_LIMIT_SENSITIVITY_TYPE_DEFAULT = 75,
        PRESSURE_LIMIT_SENSITIVITY_TYPE_HIGH = 100
    };

    // ==============================================
    // Data Structures

    struct InjectionCompleteStatus
    {
        InjectionCompleteReason reason;
        McuAlarm::AlarmId alarmId;
        QString message;

        InjectionCompleteStatus()
        {
            init();
        }

        void init()
        {
            reason = INJECTION_COMPLETE_REASON_UNKNOWN;
            alarmId = McuAlarm::ALARM_ID_BAD;
            message = "";
        }

        bool operator==(const InjectionCompleteStatus &arg) const
        {
            return ( (reason == arg.reason) &&
                     (alarmId == arg.alarmId) &&
                     (message == arg.message) );
        }

        bool operator!=(const InjectionCompleteStatus &arg) const
        {
            return !operator==(arg);
        }
    };

    struct TemperatureReading
    {
        QString key;
        double reading;

        TemperatureReading()
        {
            reading = 0.0;
        }

        bool operator==(const TemperatureReading &arg) const
        {
            return ( (key == arg.key) &&
                     (reading == arg.reading) );
        }

        bool operator!=(const TemperatureReading &arg) const
        {
            return !operator==(arg);
        }
    };

    typedef QList<double> TemperatureReadings;

    struct HeatMaintainerStatus
    {
        TemperatureReadings temperatureReadings;
        HeatMaintainerState state;

        bool operator==(const HeatMaintainerStatus &arg) const
        {
            return ( (temperatureReadings == arg.temperatureReadings) &&
                     (state == arg.state) );
        }

        bool operator!=(const HeatMaintainerStatus &arg) const
        {
            return !operator==(arg);
        }
    };

    struct InjectorStatus
    {
        InjectorState state;
        InjectionCompleteStatus completeStatus;

        InjectorStatus()
        {
            state = DS_McuDef::INJECTOR_STATE_IDLE;
            completeStatus.reason = DS_McuDef::INJECTION_COMPLETE_REASON_UNKNOWN;
        }

        bool operator==(const InjectorStatus &arg) const
        {
            return( (state == arg.state) &&
                    (completeStatus == arg.completeStatus) );
        }

        bool operator!=(const InjectorStatus &arg) const
        {
            return !operator==(arg);
        }
    };

    struct PowerStatus
    {
        bool isAcPowered;
        BatteryLevel batteryLevel;
        double batteryCharge;

        PowerStatus()
        {
            isAcPowered = true;
            batteryLevel = DS_McuDef::BATTERY_LEVEL_UNKNOWN;
            batteryCharge = 0;
        }

        bool operator==(const PowerStatus &arg) const
        {
            return ( (isAcPowered == arg.isAcPowered) &&
                     (batteryLevel == arg.batteryLevel) &&
                     (batteryCharge == arg.batteryCharge) );
        }

        bool operator!=(const PowerStatus &arg) const
        {
            return !operator==(arg);
        }
    };

    struct ActLedParams
    {
        LedControlType type;
        int colorR;
        int colorG;
        int colorB;
        bool isFlashing;

        ActLedParams()
        {
            setNoChange();
            isFlashing = false;
        }

        bool operator==(const ActLedParams &arg) const
        {
            return ( (type == arg.type) &&
                     (colorR == arg.colorR) &&
                     (colorG == arg.colorG) &&
                     (colorB == arg.colorB) &&
                     (isFlashing == arg.isFlashing) );
        }
        bool operator!=(const ActLedParams &arg) const
        {
            return !operator==(arg);
        }

        void setNoChange()
        {
            type = LED_CONTROL_TYPE_NO_CHANGE;
            colorR = 0;
            colorG = 0;
            colorB = 0;
        }

        void setColorOff()
        {
            type = LED_CONTROL_TYPE_OFF;
            colorR = 0;
            colorG = 0;
            colorB = 0;
            isFlashing = false;
        }

        void setColorGreen()
        {
            type = LED_CONTROL_TYPE_SET;
            colorR = 0;
            colorG = 255;
            colorB = 0;
        }

        void setColorGray()
        {
            type = LED_CONTROL_TYPE_SET;
            colorR = 100;
            colorG = 100;
            colorB = 100;
        }

        void setColorWhite()
        {
            type = LED_CONTROL_TYPE_SET;
            colorR = 255;
            colorG = 255;
            colorB = 255;
        }

        void setColorOrange()
        {
            type = LED_CONTROL_TYPE_SET;
            colorR = 255;
            colorG = 38;
            colorB = 0;
        }

        void setColorYellow()
        {
            type = LED_CONTROL_TYPE_SET;
            colorR = 255;
            colorG = 216;
            colorB = 0;
        }

        void setColorRed()
        {
            type = LED_CONTROL_TYPE_SET;
            colorR = 255;
            colorG = 0;
            colorB = 0;
        }

        void setColorSaline()
        {
            type = LED_CONTROL_TYPE_SET;
            colorR = 0;
            colorG = 0;
            colorB = 255;
        }

        void setColorContrast1()
        {
            type = LED_CONTROL_TYPE_SET;
            colorR = 0;
            colorG = 255;
            colorB = 0;
        }

        void setColorContrast2()
        {
            type = LED_CONTROL_TYPE_SET;
            colorR = 128;
            colorG = 0;
            colorB = 128;
        }

        void setColorFluid(SyringeIdx syringeIdx)
        {
            switch (syringeIdx)
            {
            case SYRINGE_IDX_SALINE:
                setColorSaline();
                break;
            case SYRINGE_IDX_CONTRAST1:
                setColorContrast1();
                break;
            case SYRINGE_IDX_CONTRAST2:
                setColorContrast2();
                break;
            default:
                break;
            }
        }
    };

    // Remove to ActLedsParams
    typedef QList<ActLedParams> ActLedParamsList;

    struct LedControlStatus
    {
        ActLedParamsList paramsList;

        LedControlStatus()
        {
            for (int ledIdx = 0; ledIdx < LED_IDX_MAX; ledIdx++)
            {
                ActLedParams params;
                paramsList.append(params);
            }
        }

        bool operator==(const LedControlStatus &arg) const
        {
            return (paramsList == arg.paramsList);
        }

        bool operator!=(const LedControlStatus &arg) const
        {
            return !operator==(arg);
        }
    };

    struct InjectionPhase
    {
        InjectionPhaseType type;
        int mix; // percent, 0-100
        double vol; // ml
        double flow; // ml/s
        int duration; // ms

        bool operator==(const InjectionPhase &arg) const
        {
            return ( (type == arg.type) &&
                     (mix == arg.mix) &&
                     (vol == arg.vol) &&
                     (flow == arg.flow) &&
                     (duration == arg.duration) );
        }

        bool operator!=(const InjectionPhase &arg) const
        {
            return !operator==(arg);
        }

        QList<double> getSyringeVols()
        {
            QList<double> syringeVols;

            for (int syringeIdx = 0; syringeIdx < SYRINGE_IDX_MAX; syringeIdx++)
            {
                syringeVols.append(0);
            }

            for (int syringeIdx = 0; syringeIdx < SYRINGE_IDX_MAX; syringeIdx++)
            {
                switch (type)
                {
                case DS_McuDef::INJECTION_PHASE_TYPE_CONTRAST1:
                case DS_McuDef::INJECTION_PHASE_TYPE_DUAL1:
                    syringeVols[SYRINGE_IDX_SALINE] += vol * (100 - mix) * 0.01;
                    syringeVols[SYRINGE_IDX_CONTRAST1] += vol * mix * 0.01;
                    break;
                case DS_McuDef::INJECTION_PHASE_TYPE_CONTRAST2:
                case DS_McuDef::INJECTION_PHASE_TYPE_DUAL2:
                    syringeVols[SYRINGE_IDX_SALINE] += vol * (100 - mix) * 0.01;
                    syringeVols[SYRINGE_IDX_CONTRAST2] += vol * mix * 0.01;
                    break;
                case DS_McuDef::INJECTION_PHASE_TYPE_SALINE:
                    syringeVols[SYRINGE_IDX_SALINE] += vol * 0.01;
                    break;
                default:
                    break;
                }
            }

            return syringeVols;
        }

        void setDeliveryDuration()
        {
            duration = (vol * 1000) / flow;
        }
    };

    typedef QList<InjectionPhase> InjectionPhases;

    struct InjectionProtocol
    {
        int pressureLimit; // kpa
        InjectionPhases phases;
        int twoContrastInjectPhaseIndex; // phase index that contrast injection cross over occurs. -1 if none.
        int maximumFlowRateReduction;
        int pressureLimitSensitivity;


        bool operator==(const InjectionProtocol &arg) const
        {
            return ( (pressureLimit == arg.pressureLimit) &&
                     (phases == arg.phases) &&
                     (twoContrastInjectPhaseIndex == arg.twoContrastInjectPhaseIndex) &&
                     (maximumFlowRateReduction == arg.maximumFlowRateReduction) &&
                     (pressureLimitSensitivity == arg.pressureLimitSensitivity));
        }
        bool operator!=(const InjectionProtocol &arg) const
        {
            return !operator==(arg);
        }

        void init()
        {
            pressureLimit = 0;
            twoContrastInjectPhaseIndex = -1;
            maximumFlowRateReduction = 0;
            pressureLimitSensitivity = DS_McuDef::PRESSURE_LIMIT_SENSITIVITY_TYPE_INVALID;
        }

        QList<double> getSyringeVols()
        {
            QList<double> syringeVols;
            for (int syringeIdx = 0; syringeIdx < SYRINGE_IDX_MAX; syringeIdx++)
            {
                syringeVols.append(0);
            }


            for (int phaseIdx = 0; phaseIdx < phases.length(); phaseIdx++)
            {
                QList<double> syringeVolsBuf = phases[phaseIdx].getSyringeVols();
                for (int syringeIdx = 0; syringeIdx < SYRINGE_IDX_MAX; syringeIdx++)
                {
                    syringeVols[syringeIdx] += syringeVolsBuf[syringeIdx];
                }
            }

            return syringeVols;
        }

        double getTotalVolume()
        {
            double totalVol = 0;
            QList<double> syringeVols = getSyringeVols();
            for (int syringeIdx = 0; syringeIdx < syringeVols.length(); syringeIdx++)
            {
                totalVol += syringeVols[syringeIdx];
            }
            return totalVol;
        }

        InjectionPhase *getFirstDeliveryPhase()
        {
            for (int phaseIdx = 0; phaseIdx < phases.length(); phaseIdx++)
            {
                InjectionPhaseType phaseType = phases[phaseIdx].type;
                if ( (phaseType == INJECTION_PHASE_TYPE_CONTRAST1) ||
                     (phaseType == INJECTION_PHASE_TYPE_CONTRAST1) ||
                     (phaseType == INJECTION_PHASE_TYPE_CONTRAST2) ||
                     (phaseType == INJECTION_PHASE_TYPE_SALINE) ||
                     (phaseType == INJECTION_PHASE_TYPE_DUAL1) ||
                     (phaseType == INJECTION_PHASE_TYPE_DUAL2) )
                {
                    return &phases[phaseIdx];
                }
            }
            return NULL;
        }
    };

    struct PressureCalibrationStageParams
    {
        double flowRate;
        double dataCaptureIntervalMs;

        bool operator==(const PressureCalibrationStageParams &arg) const
        {
            return ( (flowRate == arg.flowRate) &&
                     (dataCaptureIntervalMs == arg.dataCaptureIntervalMs) );
        }

        bool operator !=(const PressureCalibrationStageParams &arg) const
        {
            return !operator==(arg);
        }
    };

    typedef QList<PressureCalibrationStageParams> PressureCalibrationStagesParams;

    struct PressureCalibrationDataCheckInfo
    {
        int sampleIdx;
        double adcValuesSum;
        double lastMovingAverage;

        PressureCalibrationDataCheckInfo()
        {
            init();
        }

        void init()
        {
            sampleIdx = 0;
            adcValuesSum = 0;
            lastMovingAverage = -1;
        }

        bool operator==(const PressureCalibrationDataCheckInfo &arg) const
        {
            return ( (sampleIdx == arg.sampleIdx) &&
                     (adcValuesSum == arg.adcValuesSum) &&
                     (lastMovingAverage == arg.lastMovingAverage) );
        }

        bool operator !=(const PressureCalibrationDataCheckInfo &arg) const
        {
            return !operator==(arg);
        }
    };

    struct PressureCalibrationStatus
    {
        PressureCalibrationState state;
        QString err;
        int injectStageIdx;
        double adcReadValue;
        double homePosition; // ml
        double engagedPosition; // ml
        double firstGoodAdcValuePosition; // ml

        PressureCalibrationStagesParams stagesParams;
        PressureCalibrationDataCheckInfo dataCheckInfo;

        PressureCalibrationStatus()
        {
            init();
        }

        void init()
        {
            state = DS_McuDef::PRESSURE_CAL_STATE_IDLE;
            adcReadValue = 0;
            injectStageIdx = 0;
            homePosition = -1;
            engagedPosition = -1;
            firstGoodAdcValuePosition = -1;
            err = "";
            stagesParams.clear();
            dataCheckInfo.init();

        }

        bool operator==(const PressureCalibrationStatus &arg) const
        {
            return ( (state == arg.state) &&
                     (err == arg.err) &&
                     (injectStageIdx == arg.injectStageIdx) &&
                     (adcReadValue == arg.adcReadValue) &&
                     (homePosition == arg.homePosition) &&
                     (engagedPosition == arg.engagedPosition) &&
                     (firstGoodAdcValuePosition == arg.firstGoodAdcValuePosition) &&
                     (stagesParams == arg.stagesParams) &&
                     (dataCheckInfo == arg.dataCheckInfo) );
        }

        bool operator !=(const PressureCalibrationStatus &arg) const
        {
            return !operator==(arg);
        }
    };

    struct PressureCalCoeff
    {
        double coeff0;
        double coeff1;
        double coeff2;
        double coeff3;
        double coeff4;
        double coeff5;
        QString state;

        PressureCalCoeff()
        {
            coeff0 = 0;
            coeff1 = 0;
            coeff2 = 0;
            coeff3 = 0;
            coeff4 = 0;
            coeff5 = 0;
            state = "";
        }

        bool operator==(const PressureCalCoeff &arg) const
        {
            return ( (coeff0 == arg.coeff0) &&
                     (coeff1 == arg.coeff1) &&
                     (coeff2 == arg.coeff2) &&
                     (coeff3 == arg.coeff3) &&
                     (coeff4 == arg.coeff4) &&
                     (coeff5 == arg.coeff5) &&
                     (state == arg.state) );
        }

        bool operator!=(const PressureCalCoeff &arg) const
        {
            return !operator==(arg);
        }
    };

    typedef QList<PressureCalCoeff> PressureCalCoeffDigests;

    // ==============================================
    // Action Parameters
    struct ActPistonParams
    {
        SyringeIdx idx;
        QString arg;
        double vol;
        double flow;

        ActPistonParams()
        {
            init();
        }

        void init()
        {
            idx = SYRINGE_IDX_NONE;
            arg = "all";
            vol = 0;
            flow = 0;
        }
    };

    struct ActStopcockParams
    {
        StopcockPosAll posAll;

        ActStopcockParams()
        {
            posAll.append(STOPCOCK_POS_UNKNOWN);
            posAll.append(STOPCOCK_POS_UNKNOWN);
            posAll.append(STOPCOCK_POS_UNKNOWN);
        }
    };

    struct ActFillParams
    {
        SyringeIdx idx;
        double vol;
        double flow;
        FillType type;

        bool operator==(const ActFillParams &arg) const
        {
            return ( (idx == arg.idx) &&
                     (vol == arg.vol) &&
                     (flow == arg.flow) &&
                     (type == arg.type) );
        }
    };

    struct ActPrimeParams
    {
        SyringeIdx idx;
        double vol1;
        double vol2;
        double flow;
        QString operationName;

        ActPrimeParams()
        {
            init();
        }

        void init()
        {
            idx = SYRINGE_IDX_NONE;
            vol1 = 0;
            vol2 = 0;
            flow = 0;
            operationName = "Unknown";
        }
    };

    typedef QList<ActPrimeParams> ActPrimeParamsList;

    // ==============================================
    // MCU Simulator Data

    struct SimDigest
    {
        quint32 digestIdx;
        QByteArray alarmCode;
        int pressure;
        QString injectState;
        QString injectCompleteStatus;
        QString heatMaintainerState;
        bool stopBtnPressed;
        bool doorBtnPressed;
        bool startOfDayRequested;
        bool primeBtnPressed;
        double syringesAirVolume;
        bool sudsBubbleDetected;
        bool sudsInserted;
        bool mudsPresent;
        bool mudsLatched;
        QString batteryState;
        bool isAcPowered;
        QString doorState;
        QString outletDoorState;
        QString wasteBinState;
        bool isShuttingDown;
        double temperature1;
        double temperature2;
        double vol[SYRINGE_IDX_MAX];
        double flow[SYRINGE_IDX_MAX];
        QString syrAction[SYRINGE_IDX_MAX];
        DS_McuDef::BottleBubbleDetectorState bottleAirDetectedState[SYRINGE_IDX_MAX];
        QString plungerState[SYRINGE_IDX_MAX];
        QString stopcockPos[SYRINGE_IDX_MAX];
        QString ledColor[LED_IDX_MAX];
        QList<double> syringeAirCheckCoeffAll[SYRINGE_IDX_MAX];

        SimDigest()
        {
            digestIdx = 0;
            alarmCode = 0x0;
            pressure = 0;
            injectState = "IDLE";
            injectCompleteStatus = "UNKNOWN";
            heatMaintainerState = "ENABLED_ON";
            stopBtnPressed = false;
            doorBtnPressed = false;
            startOfDayRequested = false;
            primeBtnPressed = false;
            syringesAirVolume = 0;
            sudsBubbleDetected = false;
            sudsInserted = false;
            mudsPresent = false;
            mudsLatched = false;
            batteryState = "FULL";
            isAcPowered = true;
            doorState = "OPEN";
            outletDoorState = "OPEN";
            wasteBinState = "LOW";
            isShuttingDown = false;
            temperature1 = 35.0;
            temperature2 = 35.0;

            for (int i = 0; i < ARRAY_LEN(vol); i++)
            {
                vol[i] = 0;
            }

            for (int i = 0; i < ARRAY_LEN(flow); i++)
            {
                flow[i] = 0;
            }

            for (int i = 0; i < ARRAY_LEN(syrAction); i++)
            {
                syrAction[i] = "UNKNOWN";
            }

            for (int i = 0; i < ARRAY_LEN(bottleAirDetectedState); i++)
            {
                bottleAirDetectedState[i] = DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_MISSING;
            }

            for (int i = 0; i < ARRAY_LEN(plungerState); i++)
            {
                plungerState[i] = "DISENGAGED";
            }

            for (int i = 0; i < ARRAY_LEN(stopcockPos); i++)
            {
                stopcockPos[i] = "INJECT";
            }

            for (int i = 0; i < ARRAY_LEN(ledColor); i++)
            {
                ledColor[i] = "OFF";
            }

            for (int i = 0; i < ARRAY_LEN(syringeAirCheckCoeffAll); i++)
            {
                syringeAirCheckCoeffAll[i].append(1);
                syringeAirCheckCoeffAll[i].append(2);
                syringeAirCheckCoeffAll[i].append(3);
            }
        }

        bool operator==(const SimDigest &arg) const
        {
            return ( (digestIdx == arg.digestIdx) &&
                     (alarmCode == arg.alarmCode) &&
                     (pressure == arg.pressure) &&
                     (injectState == arg.injectState) &&
                     (injectCompleteStatus == arg.injectCompleteStatus) &&
                     (heatMaintainerState == arg.heatMaintainerState) &&
                     (stopBtnPressed == arg.stopBtnPressed) &&
                     (doorBtnPressed == arg.doorBtnPressed) &&
                     (startOfDayRequested == arg.startOfDayRequested) &&
                     (primeBtnPressed == arg.primeBtnPressed) &&
                     (syringesAirVolume == arg.syringesAirVolume) &&
                     (sudsBubbleDetected == arg.sudsBubbleDetected) &&
                     (sudsInserted == arg.sudsInserted) &&
                     (mudsPresent == arg.mudsPresent) &&
                     (mudsLatched == arg.mudsLatched) &&
                     (batteryState == arg.batteryState) &&
                     (isAcPowered == arg.isAcPowered) &&
                     (doorState == arg.doorState) &&
                     (outletDoorState == arg.outletDoorState) &&
                     (wasteBinState == arg.wasteBinState) &&
                     (isShuttingDown == arg.isShuttingDown) &&
                     (temperature1 == arg.temperature1) &&
                     (temperature2 == arg.temperature2) &&
                     (ARRAY_CMP(vol, arg.vol)) &&
                     (ARRAY_CMP(flow, arg.flow)) &&
                     (ARRAY_CMP(syrAction, arg.syrAction)) &&
                     (ARRAY_CMP(bottleAirDetectedState, arg.bottleAirDetectedState)) &&
                     (ARRAY_CMP(plungerState, arg.plungerState)) &&
                     (ARRAY_CMP(stopcockPos, arg.stopcockPos)) &&
                     (ARRAY_CMP(ledColor, arg.ledColor)) &&
                     (ARRAY_CMP(syringeAirCheckCoeffAll, arg.syringeAirCheckCoeffAll)) );
        }

        bool operator!=(const SimDigest &arg) const
        {
            return !operator==(arg);
        }
    };


    struct PhaseInjectDigest
    {
        QList<double> volumes;
        quint64 duration;
        bool pulsingActivated; // both scheduled and unscheduled

        PhaseInjectDigest()
        {
            init();
        }

        void init()
        {
            volumes.clear();
            for (int syringeIdx = 0; syringeIdx < SYRINGE_IDX_MAX; syringeIdx++)
            {
                volumes.append(0);
            }
            duration = 0;
            pulsingActivated = false;
        }

        bool operator==(const PhaseInjectDigest &arg) const
        {
            return ( (volumes == arg.volumes) &&
                     (duration == arg.duration) &&
                     (pulsingActivated == arg.pulsingActivated) );
        }

        bool operator!=(const PhaseInjectDigest &arg) const
        {
            return !operator==(arg);
        }

        double getTotalVolume()
        {
            double volume = 0;
            for (int syringeIdx = 0; syringeIdx < volumes.length(); syringeIdx++)
            {
                volume += volumes[syringeIdx];
            }
            return volume;
        }
    };

    typedef QList<PhaseInjectDigest> PhaseInjectDigests;

    struct SyringeInjectDigest
    {
        int pressure;
        int motorPid;
        StopcockPos scPos;
        int motorPos;
        double slowStartReduction;
        double storedCompliance;
        double phaseCompliance;
        double flowRate;
        double volPushed;

        SyringeInjectDigest()
        {
            init();
        }

        void init()
        {
            pressure = 0;
            motorPid = 0;
            scPos = STOPCOCK_POS_UNKNOWN;
            motorPos = 0;
            slowStartReduction = 0;
            storedCompliance = 0;
            phaseCompliance = 0;
            flowRate = 0;
            volPushed = 0;
        }

        bool operator==(const SyringeInjectDigest &arg) const
        {
            return ( (pressure == arg.pressure) &&
                     (motorPid == arg.motorPid) &&
                     (scPos == arg.scPos) &&
                     (motorPos == arg.motorPos) &&
                     (slowStartReduction == arg.slowStartReduction) &&
                     (storedCompliance == arg.storedCompliance) &&
                     (phaseCompliance == arg.phaseCompliance) &&
                     (flowRate == arg.flowRate) &&
                     (volPushed == arg.volPushed) );
        }

        bool operator!=(const SyringeInjectDigest &arg) const
        {
            return !operator==(arg);
        }
    };

    typedef QList<SyringeInjectDigest> SyringeInjectDigests;

    struct InjectDigest
    {
        int phaseIdx;
        AdaptiveFlowState adaptiveFlowState;
        bool scheduledPulsingActive;
        bool unscheduledPulsingActive;
        int injectionPressure;  //kpa
        int patientLineAirCounts;
        int adcPinReading120;
        int adcPinReading121;
        int adcPinReading122;
        int pressureMonitorPortReading;
        SyringeInjectDigests syringeInjectDigests;
        PhaseInjectDigests phaseInjectDigests;

        InjectDigest()
        {
            init();
        }

        void init()
        {
            phaseIdx = 0;
            adaptiveFlowState = ADAPTIVE_FLOW_STATE_OFF;
            scheduledPulsingActive = false;
            unscheduledPulsingActive = false;
            injectionPressure = 0;
            patientLineAirCounts = 0;
            adcPinReading120 = 0;
            adcPinReading121 = 0;
            adcPinReading122 = 0;
            pressureMonitorPortReading = 0;

            syringeInjectDigests.clear();
            SyringeInjectDigest syringeInjdexDigest;
            for (int syringeIdx = 0; syringeIdx < SYRINGE_IDX_MAX; syringeIdx++)
            {
                syringeInjectDigests.append(syringeInjdexDigest);
            }

            phaseInjectDigests.clear();
            PhaseInjectDigest injectPhaseDigest;
            for (int phaseIdx = 0; phaseIdx < MAX_MCU_PHASES; phaseIdx++)
            {
                phaseInjectDigests.append(injectPhaseDigest);
            }
        }

        bool operator==(const InjectDigest &arg) const
        {
            return ( (phaseIdx == arg.phaseIdx) &&
                     (adaptiveFlowState == arg.adaptiveFlowState) &&
                     (scheduledPulsingActive == arg.scheduledPulsingActive) &&
                     (unscheduledPulsingActive == arg.unscheduledPulsingActive) &&
                     (injectionPressure == arg.injectionPressure) &&
                     (patientLineAirCounts == arg.patientLineAirCounts) &&
                     (adcPinReading120 == arg.adcPinReading120) &&
                     (adcPinReading121 == arg.adcPinReading121) &&
                     (adcPinReading122 == arg.adcPinReading122) &&
                     (pressureMonitorPortReading == arg.pressureMonitorPortReading) &&
                     (syringeInjectDigests == arg.syringeInjectDigests) &&
                     (phaseInjectDigests.operator==(arg.phaseInjectDigests)) );
        }

        bool operator!=(const InjectDigest &arg) const
        {
            return !operator==(arg);
        }

        double getTotalInjectedVolume()
        {
            double totalVolume = 0;
            for (int phaseIdx_ = 0; phaseIdx_ <= phaseIdx; phaseIdx_++)
            {
                totalVolume += phaseInjectDigests[phaseIdx_].getTotalVolume();
            }
            return totalVolume;
        }
    };

    struct SyringeAirCheckCalData
    {
        int displacement;
        int pressureKpa;

        SyringeAirCheckCalData()
        {
            displacement = 0;
            pressureKpa = 0;
        }

        bool operator==(const SyringeAirCheckCalData &arg) const
        {
            return ( (displacement == arg.displacement) &&
                     (pressureKpa == arg.pressureKpa) );
        }

        bool operator!=(const SyringeAirCheckCalData &arg) const
        {
            return !operator==(arg);
        }
    };

    typedef QList<SyringeAirCheckCalData> SyringeAirCheckCalDigests;

    struct SyringeAirCheckCoeff
    {
        double slope;
        double intercept;
        QString state;

        SyringeAirCheckCoeff()
        {
            slope = 0;
            intercept = 0;
            state = "";
        }

        bool operator==(const SyringeAirCheckCoeff &arg) const
        {
            return ( (slope == arg.slope) &&
                     (intercept == arg.intercept) &&
                     (state == arg.state));
        }

        bool operator!=(const SyringeAirCheckCoeff &arg) const
        {
            return !operator==(arg);
        }
    };

    typedef QList<SyringeAirCheckCoeff> SyringeAirCheckCoeffDigests;

    struct SyringeAirCheckDigest
    {
        double airVolume; // ml
        double airVolume2; // ml

        SyringeAirCheckDigest()
        {
            airVolume = 0;
            airVolume2 = 0;
        }

        bool operator==(const SyringeAirCheckDigest &arg) const
        {
            return ( (airVolume == arg.airVolume) &&
                     (airVolume2 == arg.airVolume2) );
        }

        bool operator!=(const SyringeAirCheckDigest &arg) const
        {
            return !operator==(arg);
        }
    };

    typedef QList<SyringeAirCheckDigest> SyringeAirCheckDigests;

    struct BMSDigestParamsDeviceInfo
    {
        QString deviceNumber;
        QString firmwareVersion;
        QString buildNumber;
        QString cedvVersion;
        QString deviceName;

        bool operator==(const BMSDigestParamsDeviceInfo &arg) const
        {
            return ( (deviceNumber == arg.deviceNumber) &&
                     (firmwareVersion == arg.firmwareVersion) &&
                     (buildNumber == arg.buildNumber) &&
                     (cedvVersion == arg.cedvVersion) &&
                     (deviceName == arg.deviceName));
        }

        bool operator!=(const BMSDigestParamsDeviceInfo &arg) const
        {
            return !operator==(arg);
        }

        BMSDigestParamsDeviceInfo()
        {
            init();
        }

        void init()
        {
            deviceNumber = "--";
            firmwareVersion = "--";
            buildNumber = "--";
            cedvVersion = "--";
            deviceName = "--";
        }
    };

    struct BMSDigestParamsSBSStatus
    {
        qint16 temperature; // °C
        quint16 maxError;
        qint16 current; // mA
        quint16 relativeStateOfCharge; // %
        quint16 runTimeToEmpty;
        quint16 aveTimeToEmpty;
        quint16 aveTimeToFull;
        quint16 batteryStatus;
        quint16 cycleCount;
        quint16 manufactureDate;
        quint16 serialNumber;
        quint16 hostFETControl;
        quint16 cellVoltages[10];
        quint8 stateOfHealth;
        // battery Status bits
        bool overchargeAlarm;
        bool terminateChargeAlarm;
        bool overTemperatureAlarm;
        bool terminateDischargeAlarm;
        bool fullyCharged;
        bool fullyDischarged;
        qint8 errorCode;

        bool operator==(const BMSDigestParamsSBSStatus &arg) const
        {
            return ( (temperature == arg.temperature) &&
                     (maxError == arg.maxError) &&
                     (current == arg.current) &&
                     (relativeStateOfCharge == arg.relativeStateOfCharge) &&
                     (runTimeToEmpty == arg.runTimeToEmpty) &&
                     (aveTimeToEmpty == arg.aveTimeToEmpty) &&
                     (aveTimeToFull == arg.aveTimeToFull) &&
                     (batteryStatus == arg.batteryStatus) &&
                     (cycleCount == arg.cycleCount) &&
                     (manufactureDate == arg.manufactureDate) &&
                     (serialNumber == arg.serialNumber) &&
                     (hostFETControl == arg.hostFETControl) &&
                     (ARRAY_CMP(cellVoltages, arg.cellVoltages)) &&
                     (stateOfHealth == arg.stateOfHealth) &&
                     // battery status
                     (overchargeAlarm == arg.overchargeAlarm) &&
                     (terminateChargeAlarm == arg.terminateChargeAlarm) &&
                     (overTemperatureAlarm == arg.overTemperatureAlarm) &&
                     (terminateDischargeAlarm == arg.terminateDischargeAlarm) &&
                     (fullyCharged == arg.fullyCharged) &&
                     (fullyDischarged == arg.fullyDischarged) &&
                     (errorCode == arg.errorCode));
        }

        bool operator!=(const BMSDigestParamsSBSStatus &arg) const
        {
            return !operator==(arg);
        }

        BMSDigestParamsSBSStatus()
        {
            init();
        }

        void init()
        {
            temperature = 0;
            current = 0;
            maxError = 0;
            relativeStateOfCharge = 0;
            runTimeToEmpty = 0;
            aveTimeToEmpty = 0;
            aveTimeToFull = 0;
            batteryStatus = 0;
            cycleCount = 0;
            manufactureDate = 0;
            serialNumber = 0;
            hostFETControl = 0;
            for (int i = 0; i < ARRAY_LEN(cellVoltages); i++)
            {
                cellVoltages[i] = 0;
            }
            stateOfHealth = 0;

            overchargeAlarm = false;
            terminateChargeAlarm = false;
            overTemperatureAlarm = false;
            terminateDischargeAlarm = false;
            fullyCharged = false;
            fullyDischarged = false;
            errorCode = 0;
        }
    };

    struct BMSDigestParamsSafetyStatus
    {
        bool overcharged;
        bool chargeTimeout;
        bool prechargeTimeout;
        bool underTemperatureDuringDischarge;
        bool underTemperatureDuringCharge;
        bool overTemperatureDuringDischarge;
        bool overTemperatureDuringCharge;
        bool shortCircuitDuringDischargeLatch;
        bool shortCircuitDuringDischarge;
        bool overloadDuringDischargeLatch;
        bool overloadDuringDischarge;
        bool overCurrentDuringDischarge;
        bool overCurrentDuringCharge;
        bool cellOverVoltage;
        bool cellUnderVoltage;

        bool operator==(const BMSDigestParamsSafetyStatus &arg) const
        {
            return ( (overcharged == arg.overcharged) &&
                     (chargeTimeout == arg.chargeTimeout) &&
                     (prechargeTimeout == arg.prechargeTimeout) &&
                     (underTemperatureDuringDischarge == arg.underTemperatureDuringDischarge) &&
                     (underTemperatureDuringCharge == arg.underTemperatureDuringCharge) &&
                     (overTemperatureDuringDischarge == arg.overTemperatureDuringDischarge) &&
                     (overTemperatureDuringCharge == arg.overTemperatureDuringCharge) &&
                     (shortCircuitDuringDischargeLatch == arg.shortCircuitDuringDischargeLatch) &&
                     (shortCircuitDuringDischarge == arg.shortCircuitDuringDischarge) &&
                     (overloadDuringDischargeLatch == arg.overloadDuringDischargeLatch) &&
                     (overloadDuringDischarge == arg.overloadDuringDischarge) &&
                     (overCurrentDuringDischarge == arg.overCurrentDuringDischarge) &&
                     (overCurrentDuringCharge == arg.overCurrentDuringCharge) &&
                     (cellOverVoltage == arg.cellOverVoltage) &&
                     (cellUnderVoltage == arg.cellUnderVoltage) );
        }

        bool operator!=(const BMSDigestParamsSafetyStatus &arg) const
        {
            return !operator==(arg);
        }

        BMSDigestParamsSafetyStatus()
        {
            init();
        }

        void init()
        {
            overcharged = false;
            chargeTimeout = false;
            prechargeTimeout = false;
            underTemperatureDuringDischarge = false;
            underTemperatureDuringCharge = false;
            overTemperatureDuringDischarge = false;
            overTemperatureDuringCharge = false;
            shortCircuitDuringDischargeLatch = false;
            shortCircuitDuringDischarge = false;
            overloadDuringDischargeLatch = false;
            overloadDuringDischarge = false;
            overCurrentDuringDischarge = false;
            overCurrentDuringCharge = false;
            cellOverVoltage = false;
            cellUnderVoltage = false;
        }
    };

    struct BMSDigestParamsPfStatus
    {
        bool dataFlashWearoutFailure;
        bool instructionFlashCehcksumFailure;
        bool safetyOvertemperatureFETFailure;
        bool openThermistorTS3Failure;
        bool openThermistorTS2Failure;
        bool openThermistorTS1Failure;
        bool companionAfeXReadyFailure;
        bool companionAfeOveride;
        bool afeCommunicationFailure;
        bool afeRegisterFailure;
        bool dischargeFETFailure;
        bool chargeFETFailure;
        bool voltageImbalanceWhilePackRestFailure;
        bool safetyOvertemperatureCellFailure;
        bool safetyOvercurrentInDischarge;
        bool safetyOvercurrentInCharge;
        bool safetyCellOvervoltageFailure;
        bool safetyCellUndervoltageFailure;

        bool operator==(const BMSDigestParamsPfStatus &arg) const
        {
            return ( (dataFlashWearoutFailure == arg.dataFlashWearoutFailure) &&
                     (instructionFlashCehcksumFailure == arg.instructionFlashCehcksumFailure) &&
                     (safetyOvertemperatureFETFailure == arg.safetyOvertemperatureFETFailure) &&
                     (openThermistorTS3Failure == arg.openThermistorTS3Failure) &&
                     (openThermistorTS2Failure == arg.openThermistorTS2Failure) &&
                     (openThermistorTS1Failure == arg.openThermistorTS1Failure) &&
                     (companionAfeXReadyFailure == arg.companionAfeXReadyFailure) &&
                     (companionAfeOveride == arg.companionAfeOveride) &&
                     (afeCommunicationFailure == arg.afeCommunicationFailure) &&
                     (afeRegisterFailure == arg.afeRegisterFailure) &&
                     (dischargeFETFailure == arg.dischargeFETFailure) &&
                     (chargeFETFailure == arg.chargeFETFailure) &&
                     (voltageImbalanceWhilePackRestFailure == arg.voltageImbalanceWhilePackRestFailure) &&
                     (safetyOvertemperatureCellFailure == arg.safetyOvertemperatureCellFailure) &&
                     (safetyOvercurrentInDischarge == arg.safetyOvercurrentInDischarge) &&
                     (safetyOvercurrentInCharge == arg.safetyOvercurrentInCharge) &&
                     (safetyCellOvervoltageFailure == arg.safetyCellOvervoltageFailure) &&
                     (safetyCellUndervoltageFailure == arg.safetyCellUndervoltageFailure) );
        }

        bool operator!=(const BMSDigestParamsPfStatus &arg) const
        {
            return !operator==(arg);
        }

        BMSDigestParamsPfStatus()
        {
            init();
        }

        void init()
        {
            dataFlashWearoutFailure = false;
            instructionFlashCehcksumFailure = false;
            safetyOvertemperatureFETFailure = false;
            openThermistorTS3Failure = false;
            openThermistorTS2Failure = false;
            openThermistorTS1Failure = false;
            companionAfeXReadyFailure = false;
            companionAfeOveride = false;
            afeCommunicationFailure = false;
            afeRegisterFailure = false;
            dischargeFETFailure = false;
            chargeFETFailure = false;
            voltageImbalanceWhilePackRestFailure = false;
            safetyOvertemperatureCellFailure = false;
            safetyOvercurrentInDischarge = false;
            safetyOvercurrentInCharge = false;
            safetyCellOvervoltageFailure = false;
            safetyCellUndervoltageFailure = false;
        }
    };

    struct BMSDigestParamsOperationStatus
    {
        bool cellBalancingStatus;
        bool ccMeasurementInSleepMode;
        bool adcMeasurementInSleepMode;
        bool initializationAfterFullReset;
        bool sleepMode;
        bool chargingDisabled;
        bool dischargingDisabled;
        bool permanentFailureModeActive;
        bool safetyModeActive;
        bool shutdownTriggeredViaLowPackVoltage;
        QString securityMode;
        bool safePinActive;
        bool isUnderHostFETControl;
        bool prechargeFETActive;
        bool dsgFETActive;
        bool chgFETActive;
        bool systemPresentLowActive;

        bool operator==(const BMSDigestParamsOperationStatus &arg) const
        {
            return ( (cellBalancingStatus == arg.cellBalancingStatus) &&
                     (ccMeasurementInSleepMode == arg.ccMeasurementInSleepMode) &&
                     (adcMeasurementInSleepMode == arg.adcMeasurementInSleepMode) &&
                     (initializationAfterFullReset == arg.initializationAfterFullReset) &&
                     (sleepMode == arg.sleepMode) &&
                     (chargingDisabled == arg.chargingDisabled) &&
                     (dischargingDisabled == arg.dischargingDisabled) &&
                     (permanentFailureModeActive == arg.permanentFailureModeActive) &&
                     (safetyModeActive == arg.safetyModeActive) &&
                     (shutdownTriggeredViaLowPackVoltage == arg.shutdownTriggeredViaLowPackVoltage) &&
                     (securityMode == arg.securityMode) &&
                     (safePinActive == arg.safePinActive) &&
                     (isUnderHostFETControl == arg.isUnderHostFETControl) &&
                     (prechargeFETActive == arg.prechargeFETActive) &&
                     (dsgFETActive == arg.dsgFETActive) &&
                     (chgFETActive == arg.chgFETActive) &&
                     (systemPresentLowActive == arg.systemPresentLowActive) );
        }

        bool operator!=(const BMSDigestParamsOperationStatus &arg) const
        {
            return !operator==(arg);
        }

        BMSDigestParamsOperationStatus()
        {
            init();
        }

        void init()
        {
            cellBalancingStatus = false;
            ccMeasurementInSleepMode = false;
            adcMeasurementInSleepMode = false;
            initializationAfterFullReset = false;
            sleepMode = false;
            chargingDisabled = false;
            dischargingDisabled = false;
            permanentFailureModeActive = false;
            safetyModeActive = false;
            shutdownTriggeredViaLowPackVoltage = false;
            securityMode = "RESERVED";
            safePinActive = false;
            isUnderHostFETControl = false;
            prechargeFETActive = false;
            dsgFETActive = false;
            chgFETActive = false;
            systemPresentLowActive = false;
        }
    };

    struct BMSDigestParamsChargingStatus
    {
        bool overTemperatureRegion;
        bool highTemperatureRegion;
        bool standardTemperatureRegion;
        bool lowTemperatureRegion;
        bool undertemperatureRegion;
        bool chargeTermination;
        bool chargeSuspend;
        bool chargeInhibit;

        bool operator==(const BMSDigestParamsChargingStatus &arg) const
        {
            return ( (overTemperatureRegion == arg.overTemperatureRegion) &&
                     (highTemperatureRegion == arg.highTemperatureRegion) &&
                     (standardTemperatureRegion == arg.standardTemperatureRegion) &&
                     (lowTemperatureRegion == arg.lowTemperatureRegion) &&
                     (undertemperatureRegion == arg.undertemperatureRegion) &&
                     (chargeTermination == arg.chargeTermination) &&
                     (chargeSuspend == arg.chargeSuspend) &&
                     (chargeInhibit == arg.chargeInhibit) );
        }

        bool operator!=(const BMSDigestParamsChargingStatus &arg) const
        {
            return !operator==(arg);
        }

        BMSDigestParamsChargingStatus()
        {
            init();
        }

        void init()
        {
            overTemperatureRegion = false;
            highTemperatureRegion = false;
            standardTemperatureRegion = false;
            lowTemperatureRegion = false;
            undertemperatureRegion = false;
            chargeTermination = false;
            chargeSuspend = false;
            chargeInhibit = false;
        }
    };

    struct BMSDigestParamsGaugingStatus
    {
        bool dischargeQualifiedForLearning; // VDQ
        bool endOfDischargeVoltageLevel2; // EDV2
        bool endOfDischargeVoltageLevel1;
        bool ocvReadingTaken;
        bool conditionFlag;
        bool dischargeDetected;
        bool endOfDischargeVoltageLevel0;
        bool cellBalancingPossible;
        bool terminateCharge;
        bool terminateDischarge;
        bool fullyCharged;
        bool fullyDischarged;

        bool operator==(const BMSDigestParamsGaugingStatus &arg) const
        {
            return ( (dischargeQualifiedForLearning == arg.dischargeQualifiedForLearning) &&
                     (endOfDischargeVoltageLevel2 == arg.endOfDischargeVoltageLevel2) &&
                     (endOfDischargeVoltageLevel1 == arg.endOfDischargeVoltageLevel1) &&
                     (ocvReadingTaken == arg.ocvReadingTaken) &&
                     (conditionFlag == arg.conditionFlag) &&
                     (dischargeDetected == arg.dischargeDetected) &&
                     (endOfDischargeVoltageLevel0 == arg.endOfDischargeVoltageLevel0) &&
                     (cellBalancingPossible == arg.cellBalancingPossible) &&
                     (terminateCharge == arg.terminateCharge) &&
                     (terminateDischarge == arg.terminateDischarge) &&
                     (fullyCharged == arg.fullyCharged) &&
                     (fullyDischarged == arg.fullyDischarged) );
        }

        bool operator!=(const BMSDigestParamsGaugingStatus &arg) const
        {
            return !operator==(arg);
        }

        BMSDigestParamsGaugingStatus()
        {
            init();
        }

        void init()
        {
            dischargeQualifiedForLearning = false;
            endOfDischargeVoltageLevel2 = false;
            endOfDischargeVoltageLevel1 = false;
            ocvReadingTaken = false;
            conditionFlag = false;
            dischargeDetected = false;
            endOfDischargeVoltageLevel0 = false;
            cellBalancingPossible = false;
            terminateCharge = false;
            terminateDischarge = false;
            fullyCharged = false;
            fullyDischarged = false;
        }
    };

    struct BMSDigestParamsManufacturingStatus
    {
        bool ledDisplay;
        bool safeAction;
        bool blackBoxRecorder;
        bool permanentFailure;
        bool lifetimeDataCollection;
        bool allFetAction;

        bool operator==(const BMSDigestParamsManufacturingStatus &arg) const
        {
            return ( (ledDisplay == arg.ledDisplay) &&
                     (safeAction == arg.safeAction) &&
                     (blackBoxRecorder == arg.blackBoxRecorder) &&
                     (permanentFailure == arg.permanentFailure) &&
                     (lifetimeDataCollection == arg.lifetimeDataCollection) &&
                     (allFetAction == arg.allFetAction) );
        }

        bool operator!=(const BMSDigestParamsManufacturingStatus &arg) const
        {
            return !operator==(arg);
        }

        BMSDigestParamsManufacturingStatus()
        {
            init();
        }

        void init()
        {
            ledDisplay = false;
            safeAction = false;
            blackBoxRecorder = false;
            permanentFailure = false;
            lifetimeDataCollection = false;
            allFetAction = false;
        }
    };

    struct BMSDigest
    {
        QString firmwareVersionStr;
        QString hardwareVersion;

        QString safetyStatusBytes;
        QString pfStatusBytes;
        QString operationStatusBytes;
        QString chargingStatusBytes;
        QString gaugingStatusBytes;
        QString manufacturingStatusBytes;

        BMSDigestParamsDeviceInfo deviceInfo;
        BMSDigestParamsSBSStatus sbsStatus;
        BMSDigestParamsSafetyStatus safetyStatus;
        BMSDigestParamsPfStatus pfStatus;
        BMSDigestParamsOperationStatus operationStatus;
        BMSDigestParamsChargingStatus chargingStatus;
        BMSDigestParamsGaugingStatus gaugingStatus;
        BMSDigestParamsManufacturingStatus manufacturingStatus;

        bool operator==(const BMSDigest &arg) const
        {
            return ( (firmwareVersionStr == arg.firmwareVersionStr) &&
                     (hardwareVersion == arg.hardwareVersion) &&
                     (safetyStatusBytes == arg.safetyStatusBytes) &&
                     (pfStatusBytes == arg.pfStatusBytes) &&
                     (operationStatusBytes == arg.operationStatusBytes) &&
                     (chargingStatusBytes == arg.chargingStatusBytes) &&
                     (gaugingStatusBytes == arg.gaugingStatusBytes) &&
                     (manufacturingStatusBytes == arg.manufacturingStatusBytes) &&
                     (deviceInfo == arg.deviceInfo) &&
                     (sbsStatus == arg.sbsStatus) &&
                     (safetyStatus == arg.safetyStatus) &&
                     (pfStatus == arg.pfStatus) &&
                     (operationStatus == arg.operationStatus) &&
                     (chargingStatus == arg.chargingStatus) &&
                     (gaugingStatus == arg.gaugingStatus) &&
                     (manufacturingStatus == arg.manufacturingStatus) );
        }

        bool operator!=(const BMSDigest &arg) const
        {
            return !operator==(arg);
        }

        BMSDigest()
        {
            init();
        }

        void init()
        {
            firmwareVersionStr = "";
            hardwareVersion = "";
            safetyStatusBytes = "--";
            pfStatusBytes = "--";
            operationStatusBytes = "--";
            chargingStatusBytes = "--";
            gaugingStatusBytes = "--";
            manufacturingStatusBytes = "--";
            deviceInfo.init();
            safetyStatus.init();
            pfStatus.init();
            operationStatus.init();
            chargingStatus.init();
            gaugingStatus.init();
            manufacturingStatus.init();
        }
    };

    typedef QList<BMSDigest> BMSDigests;

    struct HwRevCompatibility
    {
        QString revOffset; // revision offset, e.g. "Ca". If revision is equal or greater than 'Ca', the hardware type shall be "<HwType><HwNumber>"
        int hwNumber; // 1,2,3...

        HwRevCompatibility()
        {
            revOffset = "";
            hwNumber = 1;
        }

        bool operator==(const HwRevCompatibility &arg) const
        {
            return ( (revOffset == arg.revOffset) &&
                     (hwNumber == arg.hwNumber) );
        }

        bool operator!=(const HwRevCompatibility &arg) const
        {
            return !operator==(arg);
        }
    };

    struct HwRevCompatibilityGroup
    {
        QString hwType;
        QString hwDefault;
        QList<HwRevCompatibility> compatibilityList;

        void init(QString hwType_, QString hwDefault_ = "")
        {
            hwType = hwType_;
            hwDefault = hwDefault_;
            compatibilityList.clear();
        }

        bool operator==(const HwRevCompatibilityGroup &arg) const
        {
            return ( (hwType == arg.hwType) &&
                     (hwDefault == arg.hwDefault) &&
                     (compatibilityList == arg.compatibilityList) );
        }

        bool operator!=(const HwRevCompatibilityGroup &arg) const
        {
            return !operator==(arg);
        }
    };

    typedef QList<HwRevCompatibilityGroup> HwRevCompatibilityGroups;

    // ==============================================
    // MCU Message APIs

    static void parseMsg(QString msg, QString &id, QString &body, QString &reply)
    {
        id = "";
        body = "";
        reply = "";

        int idxMsgStart = msg.indexOf(MSG_START_CHAR);
        int idxReplyStart = msg.indexOf(MSG_REPLY_START);
        int idxBodyStart = msg.indexOf(MSG_BODY_START_CHAR);
        int idxMsgEnd = msg.indexOf(MSG_END_CHAR);

        if ( (idxMsgStart < 0) || (idxMsgEnd < 0) )
        {
            return;
        }

        if ( (idxBodyStart < 0) && (idxReplyStart > 0) )
        {
            // >msg#reply\r
            id = msg.mid(idxMsgStart + 1, idxReplyStart - 1);
            body = "";
            reply = msg.mid(idxReplyStart + 1, idxMsgEnd - idxReplyStart - 1);
        }
        else if (idxReplyStart < 0)
        {
            // >msg@body\r
            id = msg.mid(idxMsgStart + 1, idxBodyStart - 1);
            body = msg.mid(idxBodyStart + 1, idxMsgEnd - idxBodyStart - 1);
            reply = "";
        }
        else
        {
            // >msg@body#reply\r
            id = msg.mid(idxMsgStart + 1, idxBodyStart - 1);
            body = msg.mid(idxBodyStart + 1, idxReplyStart - idxBodyStart - 1);
            reply = msg.mid(idxReplyStart + 1, idxMsgEnd - idxReplyStart - 1);
        }
    }

    static QString newMsg(QString id)
    {
        return MSG_START_CHAR + id + MSG_END_CHAR;
    }

    static QString newMsg(QString id, QString body)
    {
        if (body == "")
        {
            return newMsg(id);
        }
        else
        {
            return MSG_START_CHAR + id + MSG_BODY_START_CHAR + body + MSG_END_CHAR;
        }
    }

    static int getMcuInjectPhaseIdxFromHcuPhaseIdx(int hcuPhaseIdx, int twoContrastInjectPhaseIndex)
    {
        int mcuPhaseIdx = hcuPhaseIdx;

        if (twoContrastInjectPhaseIndex == -1)
        {
            // MCU and HCU has same phase index
            return mcuPhaseIdx;
        }

        if (mcuPhaseIdx > twoContrastInjectPhaseIndex)
        {
            mcuPhaseIdx++;
        }
        return mcuPhaseIdx;
    }

    static int getHcuInjectPhaseIdxFromMcuPhaseIdx(int mcuPhaseIdx, int twoContrastInjectPhaseIndex)
    {
        int hcuPhaseIdx = mcuPhaseIdx;

        if (twoContrastInjectPhaseIndex == -1)
        {
            // MCU and HCU has same phase index
            return hcuPhaseIdx;
        }

        if (hcuPhaseIdx > twoContrastInjectPhaseIndex)
        {
            hcuPhaseIdx--;
        }
        return hcuPhaseIdx;
    }
};


Q_DECLARE_METATYPE(DS_McuDef::LinkState);
Q_DECLARE_METATYPE(DS_McuDef::PowerStatus);
Q_DECLARE_METATYPE(DS_McuDef::DoorState);
Q_DECLARE_METATYPE(DS_McuDef::WasteBinState);
Q_DECLARE_METATYPE(DS_McuDef::StopcockPosAll);
Q_DECLARE_METATYPE(DS_McuDef::PlungerStates);
Q_DECLARE_METATYPE(DS_McuDef::SyringeStates);
Q_DECLARE_METATYPE(DS_McuDef::InjectorStatus);
Q_DECLARE_METATYPE(DS_McuDef::InjectionProtocol);
Q_DECLARE_METATYPE(DS_McuDef::SimDigest);
Q_DECLARE_METATYPE(DS_McuDef::HeatMaintainerStatus);
Q_DECLARE_METATYPE(DS_McuDef::OutletDoorState);
Q_DECLARE_METATYPE(DS_McuDef::BottleBubbleDetectorStates);
Q_DECLARE_METATYPE(DS_McuDef::PressureCalibrationStatus);
Q_DECLARE_METATYPE(DS_McuDef::LedControlStatus);
Q_DECLARE_METATYPE(DS_McuDef::PhaseInjectDigest);
Q_DECLARE_METATYPE(DS_McuDef::InjectDigest);
Q_DECLARE_METATYPE(DS_McuDef::SyringeAirCheckCalDigests);
Q_DECLARE_METATYPE(DS_McuDef::SyringeAirCheckCoeffDigests);
Q_DECLARE_METATYPE(DS_McuDef::SyringeAirCheckDigests);
Q_DECLARE_METATYPE(DS_McuDef::PressureCalCoeffDigests);
Q_DECLARE_METATYPE(DS_McuDef::BMSDigests);
Q_DECLARE_METATYPE(DS_McuDef::HwRevCompatibilityGroups);
#endif // DS_MCU_DEF_H
