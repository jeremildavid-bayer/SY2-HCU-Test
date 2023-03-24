import QtQuick 2.12
import ".."

DeviceManagerPanel {
    property var fluidSourceWasteContainer: dsDevice.fluidSourceWasteContainer

    titleText: "T_WasteContainer"
    panelName: "WASTE_BIN"
    panelActiveAlerts: activeAlertsWasteContainer

    onVisibleChanged: {
        if (visible)
        {
            reload();
        }
    }

    onFluidSourceWasteContainerChanged: {
        if (visible)
        {
            reload();
        }
    }

    function reload()
    {
        if ( (fluidSourceWasteContainer === undefined) ||
             (fluidSourceWasteContainer.InstalledAt === undefined) )
        {
            textDisposableState = "T_NotPresent";
        }
        else
        {
            textDisposableState = "T_Present";
        }
    }
}
