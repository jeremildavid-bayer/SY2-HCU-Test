import QtQuick 2.12
import "../Widgets"
import "../Widgets/Popup"

PopupMessage {
    property var activeAlerts: dsAlert.activeAlerts
    property var workflowErrorStatus: dsWorkflow.workflowErrorStatus

    showCancelBtn: true
    showOkBtn: true

    cancelBtnText: "T_Cancel"
    okBtnText: "T_Continue"

    titleText: "T_PrimeNeededAfterSodFill_Title"
    contentText: "T_PrimeNeededAfterSodFill_Message"

    onWorkflowErrorStatusChanged: {
        reload();
    }

    onActiveAlertsChanged: {
        reload();
    }

    onBtnOkClicked: {
        dsWorkflow.slotFill(workflowErrorStatus.SyringeIndexFailed);
        close();
    }

    onBtnCancelClicked: {
        dsWorkflow.slotClearSodError();
        close();
    }

    function reload()
    {
        if (workflowErrorStatus === undefined)
        {
            return;
        }

        if (isOpen())
        {
            var alertActive = false;
            for (var alertIdx = 0; alertIdx < activeAlerts.length; alertIdx++)
            {
                if (activeAlerts[alertIdx].CodeName === "SUDSReinsertedPrimeRequired")
                {
                    // Prime required popup should be open. Close current popup for now.
                    dsWorkflow.slotClearSodError();
                    close();
                    return;
                }
            }
        }

        if (workflowErrorStatus.State === "WORKFLOW_ERROR_STATE_SOD_FILL_START_WITH_PATIENT_CONNECTED")
        {
            logDebug("PopupPrimeNeededAfterSodFill: workflowErrorStatus=" + JSON.stringify(workflowErrorStatus) + ": Open()");
            open();
        }
    }
}

