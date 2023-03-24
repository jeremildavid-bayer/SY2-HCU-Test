import QtQuick 2.12

Item {
    property bool sruPrimingSUDSAlertActive: dsAlert.activeAlerts.findIndex(alert => alert.CodeName === "SRUPrimingSUDS") !== -1
    property var inactiveAlerts: dsAlert.inactiveAlerts
    antialiasing: true

    // Visible SUDS Icon
    Text {
        id: sudsIconOuter
        anchors.centerIn: parent
        font.family: fontDeviceIcon.name
        font.pixelSize: parent.height
        text: "\ue921"
        color: sudsIconInner.color
    }

    Text {
        id: sudsIconInnerBack
        visible: (fluidSourceSuds !== undefined) && (fluidSourceSuds.InstalledAt !== undefined)
        anchors.centerIn: parent
        font.family: fontDeviceIcon.name
        font.pixelSize: parent.height
        text: "\ue920"
        color: colorMap.mainBackground
    }

    Text {
        id: sudsIconInner
        opacity: (fluidSourceSuds !== undefined) && (fluidSourceSuds.InstalledAt !== undefined) ? 0.5 : 1
        anchors.centerIn: parent
        font.family: fontDeviceIcon.name
        font.pixelSize: parent.height
        text: "\ue920"
        color: {
            if ( (fluidSourceSuds === undefined) ||
                 (fluidSourceSuds.InstalledAt === undefined) )
            {
                return colorMap.deviceIconMissing;
            }
            else if (fluidSourceSuds.IsBusy)
            {
                return colorMap.errText;
            }
            else if (fluidSourceSuds.NeedsReplaced)
            {
                return colorMap.deviceIconWarningState;
            }
            else if (fluidSourceSuds.IsReady)
            {
                return sudsLineFluidColor;
            }
            return colorMap.errText;
        }
    }

    onSruPrimingSUDSAlertActiveChanged: {
        if(!sruPrimingSUDSAlertActive)
        {
            for(var idx = inactiveAlerts.length - 1; idx >= 0; idx--){
                if (inactiveAlerts[idx].CodeName === "SRUPrimingSUDS")
                {
                    if (inactiveAlerts[idx].Data === "AutoPrimeDone")
                    {
                        soundPlayer.playSUDSPrimed();
                    }
                    break;
                }
            }
        }
    }
}
