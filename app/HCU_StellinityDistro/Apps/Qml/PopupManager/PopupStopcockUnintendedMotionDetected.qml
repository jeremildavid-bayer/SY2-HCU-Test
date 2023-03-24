import QtQuick 2.12
import "../Widgets/Popup"

PopupMessage {
    property bool primeBtnPressed: dsMcu.primeBtnPressed
    property string statePath: dsSystem.statePath
    property var activeAlerts: dsAlert.activeAlerts

    type: "WARNING"
    titleText: "T_StopcockUnintendedMotionDetected_Name"

    cancelBtnText: "T_Eject"
    okBtnText: "T_Retry"

    onBtnOkClicked: {
        for (var alertIdx = 0; alertIdx < activeAlerts.length; alertIdx++)
        {
            if (activeAlerts[alertIdx].CodeName === "StopcockUnintendedMotionDetected")
            {
                dsDevice.slotStopcockFillPosition(activeAlerts[alertIdx].Data);
            }
        }
    }

    onBtnCancelClicked: {
        dsWorkflow.slotEjectMuds();
        close();
    }

    onActiveAlertsChanged: {
        var alertActive = false;

        for (var alertIdx = 0; alertIdx < activeAlerts.length; alertIdx++)
        {
            if (activeAlerts[alertIdx].CodeName === "StopcockUnintendedMotionDetected")
            {
                alertActive = true;
                setContentTextFromActiveAlert();
                break;
            }
        }

        if (alertActive)
        {
            if (!isOpen())
            {
                open();
            }
        }
        else
        {
            if (isOpen())
            {
                close();
            }
        }
    }

    function setContentTextFromActiveAlert()
    {
        // Get merged alerts to combine all data values for StopcockUnintendedMotionDetected alert
        var mergedActiveAlerts = (activeAlerts.length === 0) ? activeAlerts : dsAlert.slotGetMergedAlerts(activeAlerts);
        for (var alertIdx = 0; alertIdx < mergedActiveAlerts.length; alertIdx++)
        {
            if (mergedActiveAlerts[alertIdx].CodeName === "StopcockUnintendedMotionDetected")
            {
                contentText = "T_StopcockUnintendedMotionDetected_UserDirection;" + translate(mergedActiveAlerts[alertIdx].Data);
                break;
            }
        }
    }
}

