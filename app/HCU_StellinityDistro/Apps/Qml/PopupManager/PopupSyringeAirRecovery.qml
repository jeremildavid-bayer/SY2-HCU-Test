import QtQuick 2.12
import "../Widgets"
import "../Widgets/Popup"

PopupMessage {
    property var activeAlerts: dsAlert.activeAlerts
    property string workflowSyringeAirRecoveryState: dsWorkflow.workflowSyringeAirRecoveryState
    property string workflowState: dsWorkflow.workflowState

    type: (workflowSyringeAirRecoveryState === "SYRINGE_AIR_RECOVERY_STATE_USER_START_WAITING_1") ? "WARNING" : "INFO"
    titleText: (workflowSyringeAirRecoveryState === "SYRINGE_AIR_RECOVERY_STATE_USER_START_WAITING_1") ? "T_ReservoirAirDetected_Name" : "T_ReservoirAirRecovery_Name"

    onBtnOkClicked: {
        if (contentText.indexOf("T_ReservoirAirRecovery_MaxAirDetected") >= 0 )
        {
            dsWorkflow.slotEjectMuds();
            close();
        }
        else if ( (contentText.indexOf("T_ReservoirAirDetected_UserDirection") >= 0) ||
                  (contentText.indexOf("T_ReservoirAirRecovery_Confirmation") >= 0) ||
                  (contentText.indexOf("T_ReservoirAirRecovery_UserAbort") >= 0) ||
                  (contentText.indexOf("T_ReservoirAirRecovery_SudsRemoved") >= 0) ||
                  (contentText.indexOf("T_ReservoirAirRecovery_HwFault") >= 0) )
        {
            dsWorkflow.slotSyringeAirRecoveryResume();
        }
        else if (contentText.indexOf("T_ReservoirAirRecovery_Complete") >= 0)
        {
            dsWorkflow.slotSyringeAirRecoveryResume();
            close();
        }
        else
        {
            close();
        }
    }

    onBtnCancelClicked: {
        if (contentText.indexOf("T_ReservoirAirRecovery_HwFault") >= 0)
        {
            dsWorkflow.slotEjectMuds();
        }

        close();
    }

    onWorkflowStateChanged: {
        if (workflowState == "STATE_MUDS_EJECT_STARTED")
        {
            if (isOpen())
            {
                close();
            }
        }
    }

    onActiveAlertsChanged: {
        for (var alertIdx = 0; alertIdx < activeAlerts.length; alertIdx++)
        {
            if (activeAlerts[alertIdx].CodeName === "InsufficientVolumeForReservoirAirRecovery")
            {
                reload();
                break;
            }
        }
    }

    onWorkflowSyringeAirRecoveryStateChanged: {
        logDebug("PopupSyringeAirRecovery: workflowSyringeAirRecoveryState=" + workflowSyringeAirRecoveryState);
        if ( (workflowSyringeAirRecoveryState === "SYRINGE_AIR_RECOVERY_STATE_STARTED") ||
             (workflowSyringeAirRecoveryState === "SYRINGE_AIR_RECOVERY_STATE_VOLUME_CHECKING") )
        {
            open();
        }
        else if ( (workflowSyringeAirRecoveryState === "SYRINGE_AIR_RECOVERY_STATE_INACTIVE") ||
                  (workflowSyringeAirRecoveryState === "SYRINGE_AIR_RECOVERY_STATE_FAILED_OPERATION_FAILED") )
        {
            close();
        }

        reload();
    }

    function reload()
    {
        if (!visible)
        {
            return;
        }

        if (workflowSyringeAirRecoveryState === "SYRINGE_AIR_RECOVERY_STATE_MAX_AIR_DETECTED")
        {
            contentText = "T_ReservoirAirRecovery_MaxAirDetected";
            showCancelBtn = false;
            showOkBtn = true;
            okBtnText = "T_Eject";
            showServiceBtn = true;
        }
        else if (workflowSyringeAirRecoveryState === "SYRINGE_AIR_RECOVERY_STATE_USER_START_WAITING_1")
        {
            contentText = "T_ReservoirAirDetected_UserDirection;" + dsAlert.slotGetActiveAlertFromCodeName("ReservoirAirDetected").Data;
            showCancelBtn = false;
            showOkBtn = true;
            okBtnText = "T_OK";
            showServiceBtn = false;
        }
        else if (workflowSyringeAirRecoveryState === "SYRINGE_AIR_RECOVERY_STATE_USER_START_WAITING_2")
        {
            contentText = "T_ReservoirAirRecovery_Confirmation";
            showCancelBtn = false;
            showOkBtn = true;
            okBtnText = "T_OK";
            showServiceBtn = false;
        }
        else if (workflowSyringeAirRecoveryState === "SYRINGE_AIR_RECOVERY_STATE_SYRINGES_IDLE_WAITING")
        {
            contentText = "T_ReservoirAirRecovery_SyringesIdleWaiting";
            showCancelBtn = false;
            showOkBtn = false;
            showServiceBtn = false;
        }
        else if (workflowSyringeAirRecoveryState === "SYRINGE_AIR_RECOVERY_STATE_SUDS_INSERT_WAITING")
        {
            contentText = "T_ReservoirAirRecovery_SudsInsertWaiting";
            showCancelBtn = false;
            showOkBtn = false;
            showServiceBtn = false;
        }
        else if (workflowSyringeAirRecoveryState === "SYRINGE_AIR_RECOVERY_STATE_FAILED_INSUFFICIENT_VOLUME")
        {
            contentText = "T_InsufficientVolumeForReservoirAirRecovery_UserDirection;" + dsAlert.slotGetActiveAlertFromCodeName("InsufficientVolumeForReservoirAirRecovery").Data;
            showCancelBtn = false;
            showOkBtn = true;
            okBtnText = "T_OK";
            showServiceBtn = false;
        }
        else if (workflowSyringeAirRecoveryState === "SYRINGE_AIR_RECOVERY_STATE_VACUUM_CYCLE_STARTED")
        {
            contentText = "T_ReservoirAirRecovery_Progress";
            showCancelBtn = false;
            showOkBtn = false;
            showServiceBtn = false;
        }
        else if (workflowSyringeAirRecoveryState === "SYRINGE_AIR_RECOVERY_STATE_FAILED_USER_ABORT")
        {
            contentText = "T_ReservoirAirRecovery_UserAbort";
            showCancelBtn = false;
            showOkBtn = true;
            okBtnText = "T_Retry";
            showServiceBtn = false;
        }
        else if (workflowSyringeAirRecoveryState === "SYRINGE_AIR_RECOVERY_STATE_FAILED_SUDS_REMOVED")
        {
            contentText = "T_ReservoirAirRecovery_SudsRemoved";
            showCancelBtn = false;
            showOkBtn = true;
            okBtnText = "T_Retry";
            showServiceBtn = false;
        }
        else if (workflowSyringeAirRecoveryState === "SYRINGE_AIR_RECOVERY_STATE_FAILED_HW_FAULT")
        {
            contentText = "T_ReservoirAirRecovery_HwFault";
            showCancelBtn = true;
            showOkBtn = true;
            okBtnText = "T_Retry";
            cancelBtnText = "T_Eject";
            showServiceBtn = true;
        }
        else if (workflowSyringeAirRecoveryState === "SYRINGE_AIR_RECOVERY_STATE_USER_END_WAITING")
        {
            contentText = "T_ReservoirAirRecovery_Complete";
            showCancelBtn = false;
            showOkBtn = true;
            okBtnText = "T_OK";
            showServiceBtn = false;
        }
    }
}

