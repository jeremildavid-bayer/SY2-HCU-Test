import QtQuick 2.12
import "../Widgets"
import "../Widgets/Popup"
import "../Util.js" as Util

PopupMessage {
    property var activeAlerts: dsAlert.activeAlerts
    property bool clearAlertWhenAck: true
    property string alertCodeName
    property var activeAlert
    property string userDirectionText
    property string prevContentText: ""

    contentText: (activeAlert === undefined) ? "" : (userDirectionText + ";" + activeAlert.Data)
    showFromStartupScreen: false
    showDuringServicing: false

    onContentTextChanged: {
        if (contentText === "" && prevContentText !== "")
        {
           contentText = prevContentText;
        }
        else
        {
           prevContentText = contentText;
        }
    }

    type: "WARNING"
    showCancelBtn: false

    onActiveAlertsChanged: {
        reload();
    }

    onBtnOkClicked: {
        if (clearAlertWhenAck)
        {
            ackAlert();
        }
    }

    onClosed: {
        if (!clearAlertWhenAck)
        {
            return;
        }

        timerSingleShot(1, function() {
            // Ack alert after 1ms later to avoid Binding loop.
            ackAlert();
        });
    }

    function ackAlert()
    {
        if (activeAlert === undefined)
        {
            dsAlert.slotDeactivateAlert(alertCodeName, "");
        }
        else
        {
            dsAlert.slotDeactivateAlert(activeAlert.CodeName, activeAlert.Data);
        }
    }

    function reload()
    {
        // activeAlert is the alert that is currently shown in the popup(prev)
        // curActiveAlert is the alert matches alertCodeName(cur)
        var curActiveAlert = undefined;
        for (var alertIdx = 0; alertIdx < activeAlerts.length; alertIdx++)
        {
            if (activeAlerts[alertIdx].CodeName === alertCodeName)
            {
                curActiveAlert = Util.copyObject(activeAlerts[alertIdx]);
                break;
            }
        }

        var actionRequired = "none";

        // If there is no alert based popup shown but want to show one now
        if ( (activeAlert === undefined) &&
             (curActiveAlert !== undefined) )
        {
            actionRequired = "open";
        }
        // If there is an alert based popup shown already but want none now
        else if ( (activeAlert !== undefined) &&
                  (curActiveAlert === undefined) )
        {
            actionRequired = "close";
        }
        // If activeAlert and curActiveAlert are both defined or undefined, the popup can remain opened/closed as how it was
        else
        {
            // Do nothing
        }

        activeAlert = curActiveAlert;

        // Update popup view status after updating activeAlert
        if (actionRequired === "open")
        {
            open();
        }
        else if (actionRequired === "close")
        {
            close();
        }
    }
}

