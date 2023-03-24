#ifndef DS_WORKFLOW_DEF_H
#define DS_WORKFLOW_DEF_H

#include "Common/Common.h"
#include "Common/HwCapabilities.h"

class DS_WorkflowDef
{
public:
    enum WorkflowState
    {
        STATE_INACTIVE = 0,
        STATE_INIT,
        STATE_INIT_STOPCOCK_SETTING,
        STATE_INIT_STOPCOCK_PROGRESS,
        STATE_INIT_STOPCOCK_DONE,
        STATE_INIT_STOPCOCK_FAILED,
        STATE_INIT_FAILED,

        STATE_MUDS_INSERT_WAITING,
        STATE_MUDS_INSERTED,
        STATE_DOOR_CLOSE_WAITING,
        STATE_DOOR_CLOSED,

        STATE_SOD_STARTED,
        STATE_SOD_STARTED_BY_NEW_FLUID_LOADED,
        STATE_SOD_PROGRESS,
        STATE_SOD_FAILED,
        STATE_SOD_SUSPENDED,
        STATE_SOD_RESUMED,
        STATE_SOD_DONE,

        STATE_NORMAL_WORKFLOW_READY,
        STATE_NORMAL_WORKFLOW_SUDS_INSERT_WAITING,
        STATE_NORMAL_WORKFLOW_AUTO_PRIME_SALINE_WAITING,

        STATE_AUTO_PRIME_STARTED,
        STATE_AUTO_PRIME_PROGRESS,
        STATE_AUTO_PRIME_FAILED,
        STATE_AUTO_PRIME_DONE,

        STATE_AUTO_EMPTY_STARTED,
        STATE_AUTO_EMPTY_PROGRESS,
        STATE_AUTO_EMPTY_FAILED,
        STATE_AUTO_EMPTY_DONE,

        STATE_FLUID_REMOVAL_STARTED,
        STATE_FLUID_REMOVAL_PROGRESS,
        STATE_FLUID_REMOVAL_FAILED,
        STATE_FLUID_REMOVAL_DONE,

        STATE_SYRINGE_AIR_RECOVERY_STARTED,
        STATE_SYRINGE_AIR_RECOVERY_PROGRESS,
        STATE_SYRINGE_AIR_RECOVERY_FAILED,
        STATE_SYRINGE_AIR_RECOVERY_DONE,

        STATE_SUDS_AIR_RECOVERY_STARTED,
        STATE_SUDS_AIR_RECOVERY_PROGRESS,
        STATE_SUDS_AIR_RECOVERY_FAILED,
        STATE_SUDS_AIR_RECOVERY_DONE,

        STATE_END_OF_DAY_PURGE_STARTED,
        STATE_END_OF_DAY_PURGE_PROGRESS,
        STATE_END_OF_DAY_PURGE_FAILED,
        STATE_END_OF_DAY_PURGE_DONE,

        STATE_MUDS_EJECT_STARTED,
        STATE_MUDS_EJECT_PROGRESS,
        STATE_MUDS_EJECT_FAILED,
        STATE_MUDS_EJECT_DONE,
    };

    enum SodErrorState
    {
        SOD_ERROR_STATE_UNKNOWN = 0,
        SOD_ERROR_STATE_NONE,
        SOD_ERROR_STATE_SERVICE_ABORT,
        SOD_ERROR_STATE_USER_ABORT,

        SOD_ERROR_STATE_FIND_PLUNGERS_USER_ABORT,
        SOD_ERROR_STATE_FIND_PLUNGERS_FAILED,

        SOD_ERROR_STATE_USED_MUDS_DETECTED,

        SOD_ERROR_STATE_PURGE_USER_ABORT,
        SOD_ERROR_STATE_PURGE_FAILED_STOPCOCK_FAILED,
        SOD_ERROR_STATE_PURGE_FAILED_MUDS_REMOVED,
        SOD_ERROR_STATE_PURGE_FAILED_SUDS_INSERTED,
        SOD_ERROR_STATE_PURGE_FAILED,

        SOD_ERROR_STATE_REUSE_MUDS_WAITING_SUDS_REMOVAL,

        SOD_ERROR_STATE_ENGAGE_USER_ABORT,
        SOD_ERROR_STATE_ENGAGE_FAILED,

        SOD_ERROR_STATE_MUDS_SOD_START_FAILED,

		SOD_ERROR_STATE_CAL_SLACK_FAILED,
		
        SOD_ERROR_STATE_SYRINGE_PRIME_ABORT,
        SOD_ERROR_STATE_SYRINGE_PRIME_FAILED_SUDS_REMOVED,
        SOD_ERROR_STATE_SYRINGE_PRIME_FAILED_INSUFFICIENT_VOLUME,
        SOD_ERROR_STATE_SYRINGE_PRIME_FAILED_HW_FAULT,
        SOD_ERROR_STATE_SYRINGE_PRIME_FAILED_AIR_DETECTED,

        SOD_ERROR_STATE_CAL_SYRINGE_AIR_CHECK_FAILED_INSUFFICIENT_VOLUME,
        SOD_ERROR_STATE_CAL_SYRINGE_AIR_CHECK_FAILED_BAD_DATA,
        SOD_ERROR_STATE_CAL_SYRINGE_AIR_CHECK_FAILED_HW_FAULT,

        SOD_ERROR_STATE_MUDS_LATCH_LIFTED,
        SOD_ERROR_STATE_MUDS_EJECTED,
    };

    enum WorkflowErrorState
    {
        WORKFLOW_ERROR_STATE_UNKNOWN = 0,
        WORKFLOW_ERROR_STATE_NONE,
        WORKFLOW_ERROR_STATE_MUDS_EJECT_FAILED,
        WORKFLOW_ERROR_STATE_SOD_FILL_START_WITH_PATIENT_CONNECTED,
        WORKFLOW_ERROR_STATE_NORMAL_SUDS_REMOVED_AFTER_FIRST_PRIMED, // First auto prime done
        WORKFLOW_ERROR_STATE_NORMAL_SUDS_REMOVED_AFTER_FIRST_PRIMED_AND_NEXT_PRIME_FAILED, // First auto prime done but next prime failed
        WORKFLOW_ERROR_STATE_NORMAL_SUDS_REMOVED_BEFORE_FIRST_PRIME // Never been primed
    };

    enum MudsSodState
    {
        MUDS_SOD_STATE_INACTIVE = 0,
        MUDS_SOD_STATE_READY,
        MUDS_SOD_STATE_STARTED,

        MUDS_SOD_STATE_SYRINGES_IDLE_WAITING,
        MUDS_SOD_STATE_SALINE_VOLUME_WAITING,
        MUDS_SOD_STATE_SUDS_READY_WAITING,
        MUDS_SOD_STATE_OUTLET_AIR_DOOR_CLOSE_WAITING,
        MUDS_SOD_STATE_WASTE_CONTAINER_WAITING,

        MUDS_SOD_STATE_SYRINGE_SOD_PREPARING,

        MUDS_SOD_STATE_SYRINGE_SOD_STARTED,
        MUDS_SOD_STATE_SYRINGE_SOD_PROGRESS,
        MUDS_SOD_STATE_SYRINGE_SOD_FAILED,
        MUDS_SOD_STATE_SYRINGE_SOD_DONE,

        MUDS_SOD_STATE_ABORT,
        MUDS_SOD_STATE_DONE
    };

    enum FluidRemovalState
    {
        FLUID_REMOVAL_STATE_INACTIVE = 0,
        FLUID_REMOVAL_STATE_READY,

        FLUID_REMOVAL_STATE_STARTED,
        FLUID_REMOVAL_STATE_WAIT_USER_START,
        FLUID_REMOVAL_STATE_WAIT_REMOVE_CURRENT_SUPPLY,
        FLUID_REMOVAL_STATE_WAIT_SUDS_TO_WASTE_CONTAINER,

        FLUID_REMOVAL_STATE_SUDS_INSERT_WAITING,

        FLUID_REMOVAL_STATE_PURGE_STARTED,
        FLUID_REMOVAL_STATE_PURGE_PROGRESS,
        FLUID_REMOVAL_STATE_PURGE_DONE,
        FLUID_REMOVAL_STATE_PURGE_FAILED,

        FLUID_REMOVAL_STATE_FILL_STOPCOCK_FILL_SETTING,
        FLUID_REMOVAL_STATE_FILL_STOPCOCK_FILL_PROGRESS,
        FLUID_REMOVAL_STATE_FILL_STOPCOCK_FILL_DONE,
        FLUID_REMOVAL_STATE_FILL_STOPCOCK_FILL_FAILED,

        FLUID_REMOVAL_STATE_FILL_STARTED,
        FLUID_REMOVAL_STATE_FILL_PROGRESS,
        FLUID_REMOVAL_STATE_FILL_DONE,
        FLUID_REMOVAL_STATE_FILL_FAILED,

        FLUID_REMOVAL_STATE_CHECK_SPIKE_STATE_STARTED,
        FLUID_REMOVAL_STATE_CHECK_SPIKE_STATE_DONE,
        FLUID_REMOVAL_STATE_CHECK_SPIKE_STATE_FAILED,

        FLUID_REMOVAL_STATE_FINAL_PURGE_STARTED,
        FLUID_REMOVAL_STATE_FINAL_PURGE_PROGRESS,
        FLUID_REMOVAL_STATE_FINAL_PURGE_DONE,
        FLUID_REMOVAL_STATE_FINAL_PURGE_FAILED,

        FLUID_REMOVAL_STATE_REMOVING_TRAPPED_FLUID_ABORTED,
        FLUID_REMOVAL_STATE_REMOVING_TRAPPED_FLUID_ABORTED_BY_USER_STOP,
        FLUID_REMOVAL_STATE_REMOVING_TRAPPED_FLUID_ABORTED_BY_SUPPLY_STILL_LOADED,
        FLUID_REMOVAL_STATE_REMOVING_TRAPPED_FLUID_ABORTED_BY_HW_FAILED,

        FLUID_REMOVAL_STATE_FAILED_INVALID_STATE,
        FLUID_REMOVAL_STATE_FAILED_OTHER_SYRINGE_BUSY,
        FLUID_REMOVAL_STATE_FAILED_SUDS_REMOVED,
        FLUID_REMOVAL_STATE_FAILED_WASTE_CONTAINER_NOT_READY,
        FLUID_REMOVAL_STATE_FAILED_USER_ABORT,
        FLUID_REMOVAL_STATE_FAILED_HW_FAILED,

        FLUID_REMOVAL_STATE_FAILED,
        FLUID_REMOVAL_STATE_FAILED_DURING_REMOVING_TRAPPED_FLUID,
        FLUID_REMOVAL_STATE_DONE
    };

    enum EndOfDayPurgeState
    {
        END_OF_DAY_PURGE_STATE_INACTIVE = 0,
        END_OF_DAY_PURGE_STATE_READY,

        END_OF_DAY_PURGE_STATE_STARTED,
        END_OF_DAY_PURGE_STATE_WAIT_USER_START,
        END_OF_DAY_PURGE_STATE_WAIT_INSERT_SUDS,
        END_OF_DAY_PURGE_STATE_WAIT_SUDS_TO_WASTE_CONTAINER,

        END_OF_DAY_PURGE_STATE_STOPCOCK_INJECT_SETTING,
        END_OF_DAY_PURGE_STATE_STOPCOCK_INJECT_PROGRESS,
        END_OF_DAY_PURGE_STATE_STOPCOCK_INJECT_DONE,
        END_OF_DAY_PURGE_STATE_STOPCOCK_INJECT_FAILED,

        END_OF_DAY_PURGE_STATE_PUSH_PLUNGERS_STARTED,
        END_OF_DAY_PURGE_STATE_PUSH_PLUNGERS_PROGRESS,
        END_OF_DAY_PURGE_STATE_PUSH_PLUNGERS_DONE,
        END_OF_DAY_PURGE_STATE_PUSH_PLUNGERS_FAILED,

        END_OF_DAY_PURGE_STATE_CHECK_SALINE_LEVEL,
        END_OF_DAY_PURGE_STATE_WAIT_USER_CONFIRM_EMPTY_SALINE,

        END_OF_DAY_PURGE_STATE_FAILED_SUDS_REMOVED,
        END_OF_DAY_PURGE_STATE_FAILED_WASTE_CONTAINER_NOT_READY,
        END_OF_DAY_PURGE_STATE_FAILED_USER_ABORT,
        END_OF_DAY_PURGE_STATE_FAILED,
        END_OF_DAY_PURGE_STATE_DONE
    };

    enum AutoEmptyState
    {
        AUTO_EMPTY_STATE_INACTIVE = 0,
        AUTO_EMPTY_STATE_READY,
        AUTO_EMPTY_STATE_STARTED,

        AUTO_EMPTY_STATE_FLUID_REMOVAL_STARTED,
        AUTO_EMPTY_STATE_FLUID_REMOVAL_PROGRESS,
        AUTO_EMPTY_STATE_FLUID_REMOVAL_DONE,
        AUTO_EMPTY_STATE_FLUID_REMOVAL_FAILED,

        AUTO_EMPTY_STATE_FAILED,
        AUTO_EMPTY_STATE_DONE
    };

    enum MudsEjectState
    {
        MUDS_EJECT_STATE_INACTIVE = 0,
        MUDS_EJECT_STATE_READY,

        // Set stopcock
        MUDS_EJECT_STATE_STARTED,
        MUDS_EJECT_STATE_WAIT_STATE_PATH_IDLE_WAITING,
        MUDS_EJECT_STATE_SYRINGES_IDLE_WAITING,

        MUDS_EJECT_STATE_PULL_PISTONS_STARTED,
        MUDS_EJECT_STATE_PULL_PISTONS_PROGRESS,
        MUDS_EJECT_STATE_PULL_PISTONS_FAILED,
        MUDS_EJECT_STATE_PULL_PISTONS_DONE,

        MUDS_EJECT_STATE_STOPCOCK_FILL_SETTING,
        MUDS_EJECT_STATE_STOPCOCK_FILL_PROGRESS,
        MUDS_EJECT_STATE_STOPCOCK_FILL_DONE,
        MUDS_EJECT_STATE_STOPCOCK_FILL_FAILED,

        // Disengage
        MUDS_EJECT_STATE_DISENGAGE_STARTED,
        MUDS_EJECT_STATE_DISENGAGE_PROGRESS,
        MUDS_EJECT_STATE_DISENGAGE_DONE,
        MUDS_EJECT_STATE_DISENGAGE_FAILED,

        // Wait for unlatch
        MUDS_EJECT_STATE_UNLATCH_WAITING,
        MUDS_EJECT_STATE_UNLATCH_DONE,

        // Wait for removed
        MUDS_EJECT_STATE_REMOVAL_WAITING,
        MUDS_EJECT_STATE_REMOVAL_DONE,

        MUDS_EJECT_STATE_FAILED_USER_ABORT,
        MUDS_EJECT_STATE_FAILED_SYSTEM_ERROR,
        MUDS_EJECT_STATE_FAILED,
        MUDS_EJECT_STATE_DONE
    };

    enum SyringeAirRecoveryState
    {
        SYRINGE_AIR_RECOVERY_STATE_INACTIVE = 0,
        SYRINGE_AIR_RECOVERY_STATE_READY,

        SYRINGE_AIR_RECOVERY_STATE_STARTED,
        SYRINGE_AIR_RECOVERY_STATE_MAX_AIR_DETECTED,
        SYRINGE_AIR_RECOVERY_STATE_USER_START_WAITING_1,
        SYRINGE_AIR_RECOVERY_STATE_VOLUME_CHECKING,

        SYRINGE_AIR_RECOVERY_STATE_USER_START_WAITING_2,

        SYRINGE_AIR_RECOVERY_STATE_SYRINGES_IDLE_WAITING,
        SYRINGE_AIR_RECOVERY_STATE_SUDS_INSERT_WAITING,

        SYRINGE_AIR_RECOVERY_STATE_VACUUM_CYCLE_STARTED,
        SYRINGE_AIR_RECOVERY_STATE_VACUUM_CYCLE_PROGRESS,
        SYRINGE_AIR_RECOVERY_STATE_VACUUM_CYCLE_FAILED,
        SYRINGE_AIR_RECOVERY_STATE_VACUUM_CYCLE_DONE,

        SYRINGE_AIR_RECOVERY_STATE_PRIME_STARTED,
        SYRINGE_AIR_RECOVERY_STATE_PRIME_PROGRESS,
        SYRINGE_AIR_RECOVERY_STATE_PRIME_FAILED,
        SYRINGE_AIR_RECOVERY_STATE_PRIME_DONE,

        SYRINGE_AIR_RECOVERY_STATE_OUTLET_AIR_CHECK_STARTED,
        SYRINGE_AIR_RECOVERY_STATE_OUTLET_AIR_CHECK_FAILED,
        SYRINGE_AIR_RECOVERY_STATE_OUTLET_AIR_CHECK_DONE,

        SYRINGE_AIR_RECOVERY_STATE_EXTRA_PRIME_STARTED,
        SYRINGE_AIR_RECOVERY_STATE_EXTRA_PRIME_PROGRESS,
        SYRINGE_AIR_RECOVERY_STATE_EXTRA_PRIME_FAILED,
        SYRINGE_AIR_RECOVERY_STATE_EXTRA_PRIME_DONE,

        SYRINGE_AIR_RECOVERY_STATE_FAILED_SUDS_REMOVED,
        SYRINGE_AIR_RECOVERY_STATE_FAILED_USER_ABORT,
        SYRINGE_AIR_RECOVERY_STATE_FAILED_INSUFFICIENT_VOLUME,
        SYRINGE_AIR_RECOVERY_STATE_FAILED_HW_FAULT,
        SYRINGE_AIR_RECOVERY_STATE_FAILED_OPERATION_FAILED,

        SYRINGE_AIR_RECOVERY_STATE_USER_END_WAITING,

        SYRINGE_AIR_RECOVERY_STATE_SUSPENDED,
        SYRINGE_AIR_RECOVERY_STATE_FAILED,
        SYRINGE_AIR_RECOVERY_STATE_DONE,
    };

    enum SudsAirRecoveryState
    {
        SUDS_AIR_RECOVERY_STATE_INACTIVE = 0,
        SUDS_AIR_RECOVERY_STATE_READY,

        SUDS_AIR_RECOVERY_STATE_STARTED,
        SUDS_AIR_RECOVERY_STATE_WAIT_INJECTION_COMPLETE,

        SUDS_AIR_RECOVERY_STATE_USER_START_WAITING,
        SUDS_AIR_RECOVERY_STATE_USER_START_CONFIRMED,

        SUDS_AIR_RECOVERY_STATE_SYRINGE_PRIME_STARTED,
        SUDS_AIR_RECOVERY_STATE_SYRINGE_PRIME_PROGRESS,
        SUDS_AIR_RECOVERY_STATE_SYRINGE_PRIME_FAILED,
        SUDS_AIR_RECOVERY_STATE_SYRINGE_PRIME_DONE,

        SUDS_AIR_RECOVERY_STATE_OUTLET_AIR_CHECK_STARTED,
        SUDS_AIR_RECOVERY_STATE_OUTLET_AIR_CHECK_FAILED,
        SUDS_AIR_RECOVERY_STATE_OUTLET_AIR_CHECK_DONE,

        SUDS_AIR_RECOVERY_STATE_SYRINGE_AIR_CHECK_STARTED,
        SUDS_AIR_RECOVERY_STATE_SYRINGE_AIR_CHECK_PROGRESS,
        SUDS_AIR_RECOVERY_STATE_SYRINGE_AIR_CHECK_FAILED,
        SUDS_AIR_RECOVERY_STATE_SYRINGE_AIR_CHECK_DONE,

        SUDS_AIR_RECOVERY_STATE_DONE_WITH_AUTO_PRIME_OPERATION_USER_WAITING,
        SUDS_AIR_RECOVERY_STATE_DONE_WITH_AUTO_PRIME_OPERATION_USER_CONFIRMED,

        SUDS_AIR_RECOVERY_STATE_DONE
    };

    enum WorkflowBatteryState
    {
        WORKFLOW_BATTERY_STATE_INVALID = 0,
        WORKFLOW_BATTERY_STATE_INACTIVE,
        WORKFLOW_BATTERY_STATE_IDLE,

        WORKFLOW_BATTERY_STATE_DISCHARGE_PREPARATION,
        WORKFLOW_BATTERY_STATE_DISCHARGE_PROGRESS,
        WORKFLOW_BATTERY_STATE_DISCHARGE_DONE,
        WORKFLOW_BATTERY_STATE_DISCHARGE_ABORT,
        WORKFLOW_BATTERY_STATE_DISCHARGE_FAIL,

        WORKFLOW_BATTERY_STATE_CHARGE_PREPARATION,
        WORKFLOW_BATTERY_STATE_CHARGE_PROGRESS,
        WORKFLOW_BATTERY_STATE_CHARGE_DONE,
        WORKFLOW_BATTERY_STATE_CHARGE_ABORT,
        WORKFLOW_BATTERY_STATE_CHARGE_FAIL,

        // baseboard vout FET test
        WORKFLOW_BATTERY_STATE_BASEBOARD_VOUTTEST_ON,
        WORKFLOW_BATTERY_STATE_BASEBOARD_VOUTTEST_OFF,

        WORKFLOW_BATTERY_STATE_ABORT,
        WORKFLOW_BATTERY_STATE_FAIL,
        WORKFLOW_BATTERY_STATE_DONE,
    };

    enum AutomaticQualifiedDischargeState
    {
        AQD_STATE_INACTIVE = 0,
        AQD_STATE_IDLE,
        AQD_STATE_AQD_QUEUED,
        AQD_STATE_START,
        AQD_STATE_DISCHARGE_PROGRESS,
        AQD_STATE_DISCHARGE_DONE,
        AQD_STATE_DISCHARGE_FAIL,
        AQD_STATE_DONE,
    };

    enum ManualQualifiedDischargeState
    {
        MANUAL_QUALIFIED_DISCHARGE_STATE_INACTIVE = 0,
        MANUAL_QUALIFIED_DISCHARGE_STATE_READY,

        MANUAL_QUALIFIED_DISCHARGE_STATE_PREPARATION,

        MANUAL_QUALIFIED_DISCHARGE_STATE_USER_POWER_CONNECT_WAITING,
        MANUAL_QUALIFIED_DISCHARGE_STATE_USER_POWER_CONNECT_CONFIRMED,

        MANUAL_QUALIFIED_DISCHARGE_STATE_START,

        MANUAL_QUALIFIED_DISCHARGE_STATE_CHARGING_FULL_PROGRESS,
        MANUAL_QUALIFIED_DISCHARGE_STATE_CHARGING_FULL_DONE,
        MANUAL_QUALIFIED_DISCHARGE_STATE_CHARGING_FULL_FAIL,

        MANUAL_QUALIFIED_DISCHARGE_STATE_DISCHARGING_START,
        MANUAL_QUALIFIED_DISCHARGE_STATE_DISCHARGING_PROGRESS,
        MANUAL_QUALIFIED_DISCHARGE_STATE_DISCHARGING_DONE,
        MANUAL_QUALIFIED_DISCHARGE_STATE_DISCHARGING_FAIL,

        MANUAL_QUALIFIED_DISCHARGE_STATE_CONNECT_EXTERNAL_LOAD,
        MANUAL_QUALIFIED_DISCHARGE_STATE_LOAD_DISCHARGING_PROGRESS,
        MANUAL_QUALIFIED_DISCHARGE_STATE_LOAD_DISCHARGING_DONE,
        MANUAL_QUALIFIED_DISCHARGE_STATE_LOAD_DISCHARGING_FAIL,

        MANUAL_QUALIFIED_DISCHARGE_STATE_RECONNECT_BATTERY,
        MANUAL_QUALIFIED_DISCHARGE_STATE_RECONNECT_BATTERY_CONFIRMED,

        MANUAL_QUALIFIED_DISCHARGE_STATE_FAILED,
        MANUAL_QUALIFIED_DISCHARGE_STATE_ABORTED,
        MANUAL_QUALIFIED_DISCHARGE_STATE_DONE
    };

    enum ManualQualifiedDischargeMethod
    {
        MANUAL_QUALIFIED_DISCHARGE_METHOD_SELF = 0,
        MANUAL_QUALIFIED_DISCHARGE_METHOD_EXTERNAL_LOAD,
    };

    enum ShippingModeState
    {
        SHIPPING_MODE_STATE_INACTIVE = 0,
        SHIPPING_MODE_STATE_READY,

        SHIPPING_MODE_STATE_PREPARING,

        SHIPPING_MODE_STATE_USER_POWER_CONNECT_WAITING,
        SHIPPING_MODE_STATE_USER_POWER_CONNECT_CONFIRMED,

        SHIPPING_MODE_STATE_CHARGING_DISCHARGING_PROGRESS,
        SHIPPING_MODE_STATE_CHARGING_DISCHARGING_DONE,
        SHIPPING_MODE_STATE_CHARGING_DISCHARGING_FAIL,

        SHIPPING_MODE_STATE_SET_SLEEP_MODE,
        SHIPPING_MODE_STATE_USER_REMOVE_BATTERY_WAITING,
        SHIPPING_MODE_STATE_USER_REMOVE_BATTERY_DONE,

        SHIPPING_MODE_STATE_FAILED,
        SHIPPING_MODE_STATE_ABORTED,
        SHIPPING_MODE_STATE_DONE
    };

    enum PreloadProtocolState
    {
        PRELOAD_PROTOCOL_STATE_INACTIVE = 0,
        PRELOAD_PROTOCOL_STATE_READY,
        PRELOAD_PROTOCOL_STATE_STARTED,

        PRELOAD_PROTOCOL_STATE_USER_CONFIRM_WAITING,

        PRELOAD_PROTOCOL_STATE_PRELOAD_ARM_STARTED,
        PRELOAD_PROTOCOL_STATE_PRELOAD_ARM_PROGRESS,
        PRELOAD_PROTOCOL_STATE_PRELOAD_ARM_FAILED,
        PRELOAD_PROTOCOL_STATE_PRELOAD_ARM_DONE,

        PRELOAD_PROTOCOL_STATE_PRELOAD_INJECTION_STARTED,
        PRELOAD_PROTOCOL_STATE_PRELOAD_INJECTION_PROGRESS,
        PRELOAD_PROTOCOL_STATE_PRELOAD_INJECTION_FAILED,
        PRELOAD_PROTOCOL_STATE_PRELOAD_INJECTION_END_WAITING,
        PRELOAD_PROTOCOL_STATE_PRELOAD_INJECTION_DONE,

        PRELOAD_PROTOCOL_STATE_USER_ABORTED,
        PRELOAD_PROTOCOL_STATE_FAILED,
        PRELOAD_PROTOCOL_STATE_DONE
    };

    struct WorkflowErrorStatus
    {
        WorkflowErrorState state;
        SyringeIdx syringeIndexFailed;

        WorkflowErrorStatus()
        {
            state = DS_WorkflowDef::WORKFLOW_ERROR_STATE_NONE;
            syringeIndexFailed = SYRINGE_IDX_NONE;
        }

        bool operator==(const WorkflowErrorStatus &arg) const
        {
            return ( (state == arg.state) &&
                     (syringeIndexFailed == arg.syringeIndexFailed) );
        }

        bool operator!=(const WorkflowErrorStatus &arg) const
        {
            return !operator==(arg);
        }
    };

    struct SyringeSodStatus
    {
        bool plungerEngaged;
        bool calSlackDone;
        bool primed;
        bool airCheckCalibrated;
        bool actionInProgress;

        SyringeSodStatus()
        {
            init();
        }

        void init()
        {
            plungerEngaged = false;
            calSlackDone = false;
            primed = false;
            airCheckCalibrated = false;
            actionInProgress = false;
        }

        bool isCompleted()
        {
            return plungerEngaged && calSlackDone && primed && airCheckCalibrated;
        }

        bool operator==(const SyringeSodStatus &arg) const
        {
            return ( (plungerEngaged == arg.plungerEngaged) &&
                     (calSlackDone == arg.calSlackDone) &&
                     (primed == arg.primed) &&
                     (airCheckCalibrated == arg.airCheckCalibrated) &&
                     (actionInProgress == arg.actionInProgress) );
        }

        bool operator!=(const SyringeSodStatus &arg) const
        {
            return !operator==(arg);
        }
    };

    struct MudsSodStatus
    {
        bool identified; // Identified for new/used/unknown MUDS State
        QList<SyringeSodStatus> syringeSodStatusAll;

        MudsSodStatus()
        {
            SyringeSodStatus syringeSodStatus;
            for (int syringeIdx = 0; syringeIdx < SYRINGE_IDX_MAX; syringeIdx++)
            {
                syringeSodStatusAll.append(syringeSodStatus);
            }

            init();
        }

        void init()
        {
            identified = false;
            for (int syringeIdx = 0; syringeIdx < syringeSodStatusAll.length(); syringeIdx++)
            {
                syringeSodStatusAll[syringeIdx].init();
            }
        }

        bool operator==(const MudsSodStatus &arg) const
        {
            return ( (identified == arg.identified) &&
                     (syringeSodStatusAll == arg.syringeSodStatusAll) );
        }

        bool operator!=(const MudsSodStatus &arg) const
        {
            return !operator==(arg);
        }
    };

    struct ShippingModeStatus
    {
        ShippingModeState state;
        QString message;

        ShippingModeStatus()
        {
            state = DS_WorkflowDef::SHIPPING_MODE_STATE_INACTIVE;
            message = "";
        }

        bool operator==(const DS_WorkflowDef::ShippingModeStatus &arg) const
        {
            return ( (state == arg.state) &&
                     (message == arg.message) );
        }

        bool operator!=(const DS_WorkflowDef::ShippingModeStatus &arg) const
        {
            return !operator==(arg);
        }
    };

    struct WorkflowBatteryStatus
    {
        WorkflowBatteryState state;
        QString message;

        WorkflowBatteryStatus()
        {
            state = DS_WorkflowDef::WORKFLOW_BATTERY_STATE_INACTIVE;
            message = "";
        }

        bool operator==(const DS_WorkflowDef::WorkflowBatteryStatus &arg) const
        {
            return ( (state == arg.state) &&
                     (message == arg.message) );
        }

        bool operator!=(const DS_WorkflowDef::WorkflowBatteryStatus &arg) const
        {
            return !operator==(arg);
        }
    };

    struct AutomaticQualifiedDischargeStatus
    {
        AutomaticQualifiedDischargeState state;
        QString message;

        AutomaticQualifiedDischargeStatus()
        {
            state = DS_WorkflowDef::AQD_STATE_INACTIVE;
            message = "";
        }

        bool operator==(const DS_WorkflowDef::AutomaticQualifiedDischargeStatus &arg) const
        {
            return ( (state == arg.state) &&
                     (message == arg.message) );
        }

        bool operator!=(const DS_WorkflowDef::AutomaticQualifiedDischargeStatus &arg) const
        {
            return !operator==(arg);
        }
    };

    struct ManualQualifiedDischargeStatus
    {
        ManualQualifiedDischargeState state;
        QString message;

        ManualQualifiedDischargeStatus()
        {
            state = DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_INACTIVE;
            message = "";
        }

        bool operator==(const ManualQualifiedDischargeStatus &arg) const
        {
            return ( (state == arg.state) &&
                     (message == arg.message) );
        }

        bool operator!=(const ManualQualifiedDischargeStatus &arg) const
        {
            return !operator==(arg);
        }
    };
};


Q_DECLARE_METATYPE(DS_WorkflowDef::WorkflowState);
Q_DECLARE_METATYPE(DS_WorkflowDef::WorkflowErrorStatus);
Q_DECLARE_METATYPE(DS_WorkflowDef::SodErrorState);
Q_DECLARE_METATYPE(DS_WorkflowDef::FluidRemovalState);
Q_DECLARE_METATYPE(DS_WorkflowDef::EndOfDayPurgeState);
Q_DECLARE_METATYPE(DS_WorkflowDef::AutoEmptyState);
Q_DECLARE_METATYPE(DS_WorkflowDef::MudsEjectState);
Q_DECLARE_METATYPE(DS_WorkflowDef::MudsSodStatus);
Q_DECLARE_METATYPE(DS_WorkflowDef::SyringeAirRecoveryState);
Q_DECLARE_METATYPE(DS_WorkflowDef::SudsAirRecoveryState);
Q_DECLARE_METATYPE(DS_WorkflowDef::MudsSodState);
Q_DECLARE_METATYPE(DS_WorkflowDef::ManualQualifiedDischargeStatus);
Q_DECLARE_METATYPE(DS_WorkflowDef::WorkflowBatteryStatus);
Q_DECLARE_METATYPE(DS_WorkflowDef::ShippingModeStatus);
Q_DECLARE_METATYPE(DS_WorkflowDef::AutomaticQualifiedDischargeStatus);
Q_DECLARE_METATYPE(DS_WorkflowDef::PreloadProtocolState);
#endif // DS_WORKFLOW_DEF_H
