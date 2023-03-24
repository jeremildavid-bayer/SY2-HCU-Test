import QtQuick 2.12
import "../Widgets"
import "../Widgets/Popup"
import "../Util.js" as Util

PopupMessage {
    property var activeAlerts: dsAlert.activeAlerts
    property var lastActiveAlert

    type: "WARNING"
    titleText: "T_ReservoirAirCheckFailed_Name"
    showOkBtn: true;
    okBtnText: "T_Retry";
    cancelBtnText: "T_Eject";

    onBtnOkClicked: {
        if (contentText.indexOf("T_ReservoirAirCheckFailed_UserDirection") >= 0)
        {
            var alertData = dsAlert.slotGetActiveAlertFromCodeName("ReservoirAirCheckFailed").Data;
            var syringeIdxStr = alertData.split(";")[0];
            logDebug("PopupSyringeAirCheckFailed: Aircheck retried: slotSyringeAirCheck(" + syringeIdxStr + ")");
            dsDevice.slotSyringeAirCheck(syringeIdxStr);
        }
        close();
    }

    onBtnCancelClicked: {
        dsWorkflow.slotEjectMuds();
    }

    onActiveAlertsChanged: {
        var alertActive = false;

        for (var alertIdx = 0; alertIdx < activeAlerts.length; alertIdx++)
        {
            var codeName = activeAlerts[alertIdx].CodeName;
            var data = activeAlerts[alertIdx].Data;

            if ( (codeName === "InsufficientVolumeForReservoirAirCheck") ||
                 (codeName === "ReservoirAirCheckFailed") )
            {
                if ( (lastActiveAlert !== undefined) &&
                     (lastActiveAlert.GUID === activeAlerts[alertIdx].GUID) )
                {
                    // Same active alert occurred, no need further process
                    return;
                }

                lastActiveAlert = Util.copyObject(activeAlerts[alertIdx]);
                alertActive = true;

                if (codeName === "InsufficientVolumeForReservoirAirCheck")
                {
                    contentText = "T_InsufficientVolumeForReservoirAirCheck_UserDirection;" + data;
                    showCancelBtn = false;
                    okBtnText = "T_OK";
                    showServiceBtn = false;
                    break;
                }
                else if (codeName === "ReservoirAirCheckFailed")
                {
                    contentText = "T_ReservoirAirCheckFailed_UserDirection;" + data;
                    showCancelBtn = true;
                    okBtnText = "T_Retry";
                    cancelBtnText = "T_Eject";
                    showServiceBtn = true;
                    break;
                }
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
}

