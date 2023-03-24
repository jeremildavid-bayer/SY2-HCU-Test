import QtQuick 2.12
import "../Widgets"
import "../Widgets/Popup"
import "../Util.js" as Util

PopupAlertBase {
    property string workflowState: dsWorkflow.workflowState
    property bool sudsInserted: dsMcu.sudsInserted
    property bool popupVisibleOk: {
        if (activeAlert === undefined)
        {
            return false;
        }
        if ( (activeAlert.Data.indexOf("SUDS_REMOVED") >= 0) ||
             (activeAlert.Data.indexOf("INSUFFICIENT_FLUID") >= 0) )
        {
            return false;
        }
        return true;
    }

    alertCodeName: "AutoPrimeFailed"
    titleText: "T_AutoPrimeFailed_Name"
    type: popupVisibleOk ? "WARNING" : "QUIET"

    contentText: {
        if (activeAlert === undefined)
        {
            return "";
        }
        else if (activeAlert.Data.indexOf("AIR_DETECTED") >= 0)
        {
            return "T_AutoPrimeFailedByAirDetected_UserDirection";
        }
        else if (activeAlert.Data.indexOf("MOTOR_FAIL") >= 0)
        {
            return "T_AutoPrimeFailedByMotorFail_UserDirection";
        }
        else if (activeAlert.Data.indexOf("OVER_PRESSURE") >= 0)
        {
            return "T_AutoPrimeFailedByOverPressure_UserDirection";
        }
        else if (activeAlert.Data.indexOf("OVER_CURRENT") >= 0)
        {
            return "T_AutoPrimeFailedByOverCurrent_UserDirection";
        }
        else if (activeAlert.Data.indexOf("MUDS_REMOVED") >= 0)
        {
            return "T_AutoPrimeFailedByMudsRemoved_UserDirection";
        }
        else if (activeAlert.Data.indexOf("STOPCOCK_FAILED") >= 0)
        {
            return "T_AutoPrimeFailedByStopcockFailure_UserDirection";
        }
        else if (activeAlert.Data.indexOf("PISTON_MOVING") >= 0)
        {
            return "T_AutoPrimeFailedByPistonMoving_UserDirection";
        }
        else if (activeAlert.Data.indexOf("SYRINGE_NOT_PRIMED") >= 0)
        {
            return "T_AutoPrimeFailedBySyringeNotPrimed_UserDirection";
        }
        else if (activeAlert.Data.indexOf("INVALID_STATE") >= 0)
        {
            return "T_AutoPrimeFailedByInvalidState_UserDirection";
        }
        return "T_AutoPrimeFailedByUnknownReason_UserDirection;" + activeAlert.Data;
    }

    showCancelBtn: false

    onSudsInsertedChanged: {
        if ( (sudsInserted) &&
             (isOpen()) )
        {
            // SUDS is re-inserted. close popup
            close();
        }
    }

    onOpened: {
        if (!popupVisibleOk)
        {
            // Close Popup. DM Panel and LED shall indicate the problem.
            close();
        }

        // Change title for current workflowState.
        if ( (workflowState !== undefined) &&
             (workflowState == "STATE_SUDS_AIR_RECOVERY_PROGRESS") )
        {
            titleText = "T_OutletAirDetected_Name";
        }
        else
        {
            titleText = "T_AutoPrimeFailed_Name";
        }

        // Close duplicate popup
        if (contentText.indexOf("T_AutoPrimeFailedByOverPressure_UserDirection") >= 0)
        {
            dsAlert.slotDeactivateAlert("OverPressureFault");
        }
        else if (contentText.indexOf("T_AutoPrimeFailedByOverCurrent_UserDirection") >= 0)
        {
            dsAlert.slotDeactivateAlert("OverCurrentFault");
        }
    }
}
