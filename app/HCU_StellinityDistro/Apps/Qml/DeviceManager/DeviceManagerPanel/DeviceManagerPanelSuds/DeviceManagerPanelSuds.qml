import QtQuick 2.12
import ".."
import "../../../Util.js" as Util

DeviceManagerPanel {
    property var fluidSourceSuds: dsDevice.fluidSourceSuds
    property var activeAlerts: dsAlert.activeAlerts

    titleText: "T_PatientLine"
    panelName: "SUDS"
    panelActiveAlerts: activeAlertsSuds

    textDisposableState2: getTextDisposableState2()

    onFluidSourceSudsChanged: {
        if (visible)
        {
            reload();
        }
    }

    onVisibleChanged: {
        if (visible)
        {
            reload();
        }
    }

    function reload()
    {
        if ( (fluidSourceSuds === undefined) ||
             (fluidSourceSuds.InstalledAt === undefined) )
        {
            textDisposableState = "T_NotPresent";
            stopElapsedTimer();
        }
        else
        {
            initElapsedTimer(Util.utcDateTimeToMillisec(fluidSourceSuds.InstalledAt), -1);
            startElapsedTimer();
        }
    }

    function getTextDisposableState2()
    {
        for (var alertIdx = 0; alertIdx < activeAlerts.length; alertIdx++)
        {
            if (activeAlerts[alertIdx].CodeName === "SRUPurgingFluid")
            {
                return "T_Purging...";
            }
            else if (activeAlerts[alertIdx].CodeName === "SRUPrimingSUDS")
            {
                return "T_Priming...";
            }
        }

        if ( (fluidSourceSuds === undefined) ||
             (fluidSourceSuds.InstalledAt === undefined) )
        {
            return "";
        }
        else if ( (fluidSourceMuds !== undefined) &&
                  (fluidSourceMuds.IsReady) &&
                  (fluidSourceSuds.IsReady) )
        {
            return "T_PatientLinePrimed";
        }
        return "T_PatientLineNotPrimed";
    }
}
