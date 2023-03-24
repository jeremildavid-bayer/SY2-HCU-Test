import QtQuick 2.12

DeviceManagerGenericButton {
    antialiasing: true
    panelName: "SUDS"
    isBusy: (fluidSourceSuds !== undefined) && (fluidSourceSuds.IsBusy !== undefined) && (fluidSourceSuds.IsBusy)

    content: [
        Item {
            anchors.centerIn: parent
            width: parent.width * 0.75
            height: parent.height * 0.7
            // Visible SUDS Icon
            Text {
                id: txtSudsIcon
                anchors.centerIn: parent
                font.family: fontDeviceIcon.name
                font.pixelSize: parent.height
                text: "\ue915"
                color: {
                    if ( (fluidSourceSuds === undefined) ||
                         (fluidSourceSuds.InstalledAt === undefined) )
                    {
                        return colorMap.deviceIconMissing;
                    }
                    else if (fluidSourceSuds.IsBusy)
                    {
                        return colorMap.red;
                    }
                    else if (fluidSourceSuds.NeedsReplaced)
                    {
                        return colorMap.deviceIconWarningState;
                    }
                    else if (fluidSourceSuds.IsReady)
                    {
                        return sudsLineFluidColor;
                    }
                    return colorMap.red;
                }
            }
        }
    ]
}
