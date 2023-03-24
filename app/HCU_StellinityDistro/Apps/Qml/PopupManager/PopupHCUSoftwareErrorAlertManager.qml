import QtQuick 2.12

Item {
    property var inactiveAlerts: dsAlert.inactiveAlerts
    property string hcuBuildType: dsSystem.hcuBuildType
    property bool tradeshowModeEnabled: dsCfgGlobal.tradeshowModeEnabled
    property var popupComponent
    property var handledAlerts: []

    id: root

    onInactiveAlertsChanged: {
        reload();
    }

    Component.onCompleted: {
        popupComponent = Qt.createComponent("PopupHCUSoftwareErrorAlert.qml");

        if (popupComponent === null)
        {
            logError("PopupHCUSoftwareErrorAlertManager: Failed to create popup component");
        }
    }

    function reload()
    {
        if (inactiveAlerts.length > 0) {
            // Only process the latest one
            addAlert(inactiveAlerts[inactiveAlerts.length - 1]);
        }
    }

    function addAlert(alertData)
    {
        if ( (alertData.CodeName !== "HCUSoftwareError") &&
             (alertData.CodeName !== "HCUInternalSoftwareError") )
        {
            return;
        }

        if (alertData.CodeName === "HCUInternalSoftwareError")
        {
            // Internal bad conditions inside HCU for DEV and SQA build
            if ( (hcuBuildType != "DEV") &&
                 (hcuBuildType != "SQA") &&
                 (!tradeshowModeEnabled) )
            {
                // Donot show popup for VNV/REL
                return;
            }
        }

        var alertHandled = false;
        for (var handledAlertIdx = 0; handledAlertIdx < handledAlerts.length; handledAlertIdx++)
        {
            if (handledAlerts[handledAlertIdx].alertData.GUID === alertData.GUID)
            {
                alertHandled = true;
                break;
            }
        }

        if (!alertHandled)
        {
            var popupObject = popupComponent.createObject(root, { "alertData": alertData });
            if (popupObject === null)
            {
                logError("PopupHCUSoftwareErrorAlertManager: Failed to create Popup object. Alert=" + JSON.stringify(alertData));
            }
            else
            {
                logWarning("PopupHCUSoftwareErrorAlertManager: Alert Added: " + JSON.stringify(alertData.Data));
                popupObject.open();
                handledAlerts.push(popupObject);
            }
        }
    }


    function removeAlert(alertData)
    {
        for (var handledAlertIdx = 0; handledAlertIdx < handledAlerts.length; handledAlertIdx++)
        {
            if (handledAlerts[handledAlertIdx].alertData.GUID === alertData.GUID)
            {
                // alert is inactive, delete it
                handledAlerts[handledAlertIdx].close();
                handledAlerts[handledAlertIdx].destroy();
                handledAlerts.splice(handledAlertIdx, 1);
                logDebug("PopupHCUSoftwareErrorAlertManager: Alert Removed: " + JSON.stringify(alertData.Data));
                break;
            }
        }
    }
}
