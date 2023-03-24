import QtQuick 2.12

Item {
    property var fluidSourceBottle
    property string bottleColor: "transparent"
    property bool isAlertActive: false
    property int syringeIndex

    width: outer.contentWidth

    Text {
        id: outer
        anchors.centerIn: parent
        font.family: fontDeviceIcon.name
        font.pixelSize: parent.height
        text: "\ue91b"
        color: {
            if ((fluidSourceBottle !== undefined) && fluidSourceBottle.NeedsReplaced) {
                return colorMap.deviceIconWarningState;
            }
            else if (isAlertActive) {
                return colorMap.errText;
            }
            else if ( (fluidSourceBottle === undefined) ||
                      (fluidSourceBottle.InstalledAt === undefined) ||
                      (fluidSourceBottle.SourcePackages.length === 0) ) {
                return colorMap.deviceButtonBackground;
            }
            return bottleColor;
        }
    }

    Text {
        id: backInner
        anchors.centerIn: parent
        font.family: fontDeviceIcon.name
        font.pixelSize: parent.height
        text: "\ue91a"
        color: {
            if (isAlertActive) {
                return rootBackgroundColor;
            }
            else if ( (fluidSourceBottle === undefined) ||
                      (fluidSourceBottle.InstalledAt === undefined) ||
                      (fluidSourceBottle.SourcePackages.length === 0) ) {
                return colorMap.deviceIconMissing;
            }
            return rootBackgroundColor;
        }
    }

    Text {
        id: inner
        anchors.centerIn: parent
        font.family: fontDeviceIcon.name
        font.pixelSize: parent.height
        text: "\ue91a"
        opacity: {
            if (isAlertActive)
            {
                return 0.5;
            }
            else if ( (fluidSourceBottle === undefined) ||
                      (fluidSourceBottle.InstalledAt === undefined) ||
                      (fluidSourceBottle.SourcePackages.length === 0) )
            {
                return 1;
            }
            return 0.5;
        }
        color: {
            if ((fluidSourceBottle !== undefined) && fluidSourceBottle.NeedsReplaced) {
                return colorMap.deviceIconWarningState;
            }
            else if (isAlertActive) {
                return colorMap.errText;
            }
            else if ( (fluidSourceBottle === undefined) ||
                      (fluidSourceBottle.InstalledAt === undefined) ||
                      (fluidSourceBottle.SourcePackages.length === 0) ) {
                return colorMap.deviceIconMissing;
            }
            return bottleColor;
        }
    }
}
