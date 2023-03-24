import QtQuick 2.12
import "../Widgets"
import "../Widgets/Popup"

PopupMessage {
    property var activeAlerts: dsAlert.activeAlerts
    property bool airRecoveryAutoPriming: false

    id: root
    type: "PLAIN"
    contentText: "T_AirRecovery_Complete"
    showCancelBtn: false

    onBtnOkClicked: {
        close();
    }

    onActiveAlertsChanged: {
        var sudsPriming = false;
        var outletAirDetected = false;

        for (var alertIdx = 0; alertIdx < activeAlerts.length; alertIdx++)
        {
            if (activeAlerts[alertIdx].CodeName === "SRUPrimingSUDS")
            {
                // Service mode already visited. Unlocked
                sudsPriming = true;
            }
            if (activeAlerts[alertIdx].CodeName === "OutletAirDetected")
            {
                // Service mode already visited. Unlocked
                outletAirDetected = true;
            }
        }

        if ( (sudsPriming) && (outletAirDetected) )
        {
            // AirRecovery Auto-priming is occuring..
            airRecoveryAutoPriming = true;
        }
        else if ( (airRecoveryAutoPriming) && (!sudsPriming) && (!outletAirDetected) )
        {
            open();
            airRecoveryAutoPriming = false;
        }
    }
}

