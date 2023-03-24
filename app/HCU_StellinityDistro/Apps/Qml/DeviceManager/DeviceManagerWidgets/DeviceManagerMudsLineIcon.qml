import QtQuick 2.12

Item {
    antialiasing: true

    Text {
        id: outerIcon
        anchors.fill: parent
        font.family: fontDeviceIcon.name
        text: "\ue91d"
        font.pixelSize: parent.height
        color: {
            if ( (fluidSourceMuds === undefined) ||
                 (fluidSourceMuds.InstalledAt === undefined) ||
                 (fluidSourceMuds.SourcePackages.length === 0) )
            {
                return colorMap.deviceIconMissing;
            }
            else if (fluidSourceMuds.IsBusy)
            {
                return mudsLineFluidColor;
            }
            else if (fluidSourceMuds.IsReady)
            {
                return mudsLineFluidColor;
            }
            return outerIcon.color;
        }
    }

    Text {
        id: innerBackIcon
        anchors.fill: parent
        font.family: fontDeviceIcon.name
        text: "\ue91c"
        font.pixelSize: parent.height
        color: {
            if ( (fluidSourceMuds === undefined) ||
                 (fluidSourceMuds.InstalledAt === undefined) )
            {
                return colorMap.deviceIconMissing;
            }
            return rootBackgroundColor;
        }
    }

    Text {
        id: innerIcon
        anchors.fill: parent
        font.family: fontDeviceIcon.name
        text: "\ue91c"
        font.pixelSize: parent.height
        opacity: {
            if ( (fluidSourceMuds === undefined) ||
                 (fluidSourceMuds.InstalledAt === undefined) ||
                 (fluidSourceMuds.SourcePackages.length === 0) )
            {
                return 1;
            }
            else if (fluidSourceMuds.IsBusy)
            {
                return 0.5
            }
            else if (fluidSourceMuds.IsReady)
            {
                return 0.5
            }
            return innerIcon.opacity;
        }


        color: {
            if ( (fluidSourceMuds === undefined) ||
                 (fluidSourceMuds.InstalledAt === undefined) )
            {
                return colorMap.deviceIconMissing;
            }
            else if (fluidSourceMuds.SourcePackages.length === 0)
            {
                return rootBackgroundColor;
            }
            else if (fluidSourceMuds.IsBusy)
            {
                return mudsLineFluidColor;
            }
            else if (fluidSourceMuds.IsReady)
            {
                return mudsLineFluidColor;
            }
            return innerIcon.color;
        }
    }
}
