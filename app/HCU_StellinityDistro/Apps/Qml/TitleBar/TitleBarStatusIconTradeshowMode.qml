import QtQuick 2.12
import "../Widgets"
import "../Util.js" as Util

Item {
    property bool tradeshowModeEnabled: dsCfgGlobal.tradeshowModeEnabled
    property var activeAlerts: dsAlert.activeAlerts

    Text {
        id: tradeshowModeSymbol
        width: parent.width
        height: parent.height
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.pixelSize: height * 0.4
        font.family: fontIcon.name
        color: colorMap.text01
        text: "\ue978"
    }


    onActiveAlertsChanged: {
        reloadTradeshowModeIcon();
    }


    function reloadTradeshowModeIcon()
    {
        if (activeAlerts === undefined)
        {
            return;
        }

        if (hcuBuildType == "PROD")
        {
            return;
        }

        if (activeAlerts.filter(x => x.CodeName === "TradeshowModeActive").length > 0)
        {
            visible = true;
            return;
        }

        visible = false;
    }
}
