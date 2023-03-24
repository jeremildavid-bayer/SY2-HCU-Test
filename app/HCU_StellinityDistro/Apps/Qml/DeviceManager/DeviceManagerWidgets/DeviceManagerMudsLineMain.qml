import QtQuick 2.12

Item {
    //antialiasing: true

    Text {
        id: outerIcon
        anchors.fill: parent
        font.family: fontDeviceIcon.name
        text: "\ue912"
        font.pixelSize: parent.height * 0.99
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
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
        id: innerIcon
        anchors.fill: parent
        font.family: fontDeviceIcon.name
        text: "\ue913"
        font.pixelSize: outerIcon.font.pixelSize
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        color: {
            if ( (fluidSourceMuds === undefined) ||
                 (fluidSourceMuds.InstalledAt === undefined) )
            {
                return colorMap.deviceIconMissing;
            }
            else if (fluidSourceMuds.SourcePackages.length === 0)
            {
                return colorMap.mainBackground;
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

    AnimatedImage {
        id: animation
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        width: outerIcon.contentWidth
        visible: (fluidSourceMuds !== undefined) && (fluidSourceMuds.IsBusy !== undefined) && (fluidSourceMuds.IsBusy) && (fluidSourceMuds.SourcePackages.length > 0)
        source: getAnimationGif()
    }

    function getAnimationGif()
    {
        if (!visible)
        {
            return imageMap.primingSGif;
        }
        else if (mudsLineFluidColor === colorMap.contrast1)
        {
            return imageMap.primingC1Gif;
        }
        else if (mudsLineFluidColor === colorMap.contrast2)
        {
            return imageMap.primingC2Gif;
        }
        return imageMap.primingSGif;
    }
}
