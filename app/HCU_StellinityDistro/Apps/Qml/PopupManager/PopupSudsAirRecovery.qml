import QtQuick 2.12
import "../Widgets"
import "../Widgets/Popup"

PopupMessage {
    property string workflowSudsAirRecoveryState: dsWorkflow.workflowSudsAirRecoveryState

    type: "INFO"
    titleText: "T_OutletAirDetected_Name"

    onBtnOkClicked: {
        if ( (contentText.indexOf("T_PatientLineAirRecovery_UserDirection") >= 0) ||
             (contentText.indexOf("T_PatientLineAirRecovery_PrimeStartConfirmation") >= 0) )
        {
            dsWorkflow.slotSudsAirRecoveryResume();
        }
    }

    onWorkflowSudsAirRecoveryStateChanged: {
        if (workflowSudsAirRecoveryState === "SUDS_AIR_RECOVERY_STATE_STARTED")
        {
            contentText = "";
            showOkBtn = false;
            showCancelBtn = false;
            open();
        }
        else if (workflowSudsAirRecoveryState === "SUDS_AIR_RECOVERY_STATE_DONE_WITH_AUTO_PRIME_OPERATION_USER_CONFIRMED")
        {
            // Let Autoprime to take care of UI
            close();
        }
        else if (workflowSudsAirRecoveryState === "SUDS_AIR_RECOVERY_STATE_DONE")
        {
            // Let Syringe Air Recovery to take care of UI
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

        if (workflowSudsAirRecoveryState === "SUDS_AIR_RECOVERY_STATE_WAIT_INJECTION_COMPLETE")
        {
            contentText = "T_PatientLineAirRecovery_InjectionAbortInProgress";
            showOkBtn = false;
            showCancelBtn = false;
            type = "WARNING";
        }
        else if (workflowSudsAirRecoveryState === "SUDS_AIR_RECOVERY_STATE_USER_START_WAITING")
        {
            contentText = "T_PatientLineAirRecovery_UserDirection";
            showCancelBtn = false;
            showOkBtn = true;
            okBtnText = "T_OK";
        }
        else if (workflowSudsAirRecoveryState === "SUDS_AIR_RECOVERY_STATE_DONE_WITH_AUTO_PRIME_OPERATION_USER_WAITING")
        {
            contentText = "T_PatientLineAirRecovery_PrimeStartConfirmation";
            showOkBtn = true;
            showCancelBtn = false;
            okBtnText = "T_OK";
            type = "INFO";
        }
        else if (workflowSudsAirRecoveryState === "SUDS_AIR_RECOVERY_STATE_SYRINGE_PRIME_STARTED")
        {
            contentText = "T_Priming...";
            showOkBtn = false;
            showCancelBtn = false;
            type = "INFO";
        }
        else if (workflowSudsAirRecoveryState === "SUDS_AIR_RECOVERY_STATE_SYRINGE_AIR_CHECK_STARTED")
        {
            contentText = "T_PatientLineAirRecovery_PerformingAirCheck";
            showOkBtn = false;
            showCancelBtn = false;
            type = "INFO";
        }
    }
}

