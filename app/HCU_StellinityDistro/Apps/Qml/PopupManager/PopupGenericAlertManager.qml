import QtQuick 2.12

Item {
    property var activeAlerts: dsAlert.activeAlerts
    property string statePath: dsSystem.statePath
    property string lastStatePath: dsSystem.lastStatePath
    property var popupComponent
    property var handledAlerts: []

    id: root

    onActiveAlertsChanged: {
        reload();
    }

    onStatePathChanged: {
        if ( (statePath != "StartupUnknown") &&
             (lastStatePath == "StartupUnknown") )
        {
            // reload after start up screen
            reload();
        }
    }

    Component.onCompleted: {
        popupComponent = Qt.createComponent("PopupGenericAlert.qml");

        if (popupComponent === null)
        {
            logError("PopupGenericAlertManager: Failed to create popup component");
        }
    }

    function reload()
    {
        if ( (statePath == "OffUnreachable") ||
             (statePath == "OnReachable") ||
             (statePath == "StartupUnknown") )
        {
            return;
        }

        var alertActive = false;
        for (var alertIdx = 0; alertIdx < activeAlerts.length; alertIdx++)
        {
            addAlert(activeAlerts[alertIdx]);
        }

        removeAlerts();
    }

    function addAlert(alertData)
    {
        if ( (alertData.CodeName !== "GenericNondismissablePopup") &&
             (alertData.CodeName !== "GenericAcknowledgementPopup") &&
             (alertData.CodeName !== "GenericEventOccurred") &&
             (alertData.CodeName !== "HCUClockBatteryDead") )
        {
            return;
        }

        //Ensure we haven't previously handled this alert's GUID.
        var alertHandled = false;
        for (var handledAlertIdx = 0; handledAlertIdx < handledAlerts.length; handledAlertIdx++)
        {
            if (handledAlerts[handledAlertIdx].alertData.GUID === alertData.GUID)
            {
                alertHandled = true;
                break;
            }
        }

        //If this is our first time seeing this alert, show a pop-up.
        if (!alertHandled)
        {
            var popupObject = popupComponent.createObject(root, { "alertData": alertData });
            if (popupObject === null)
            {
                logError("PopupGenericAlertManager: Failed to create Popup object. Alert=" + JSON.stringify(alertData));
            }
            else
            {
                popupObject.open();
                handledAlerts.push(popupObject);
            }
        }
    }


    function removeAlerts()
    {
        for (var handledAlertIdx = 0; handledAlertIdx < handledAlerts.length; handledAlertIdx++)
        {
            var alertActive = false;
            for (var alertIdx = 0; alertIdx < activeAlerts.length; alertIdx++)
            {
                if (handledAlerts[handledAlertIdx].alertData.GUID === activeAlerts[alertIdx].GUID)
                {
                    alertActive = true;
                    break;
                }
            }

            if (!alertActive)
            {
                // alert is inactive, delete it
                handledAlerts[handledAlertIdx].close();
                handledAlerts[handledAlertIdx].destroy();
                handledAlerts.splice(handledAlertIdx, 1);
                handledAlertIdx = 0;
            }
        }
    }
}
