import QtQuick 2.12

import "../../CommonElements"

Item {
    property int syringeIndex
    property var fluidSourceSyringe
    property double syringeVolume: 0
    property string syringeColor
    property string backgroundColor
    property bool isAlertActive: false
    property bool autoEmptyEnabled: false

    antialiasing: true
    width: syringeIconOuter.contentWidth

    Text {
        id: syringeIconOuter
        anchors.centerIn: parent
        font.family: fontDeviceIcon.name
        font.pixelSize: parent.height
        text: "\ue91f"
        color: {
            if (isAlertActive)
            {
                return colorMap.errText;
            }
            else if ( (fluidSourceSyringe === undefined) ||
                      (fluidSourceSyringe.InstalledAt === undefined) )
            {
                return colorMap.deviceIconMissing;
            }
            else if (fluidSourceSyringe.NeedsReplaced)
            {
                return colorMap.deviceIconWarningState;
            }
            return syringeColor;
        }
    }

    Text {
        id: syringeIconInnerBack
        anchors.centerIn: parent
        font.family: fontDeviceIcon.name
        font.pixelSize: parent.height
        text: "\ue91e"
        color: {
            if (isAlertActive)
            {
                return rootBackgroundColor;
            }
            else if ( (fluidSourceSyringe === undefined) ||
                      (fluidSourceSyringe.InstalledAt === undefined) )
            {
                return colorMap.deviceIconMissing;
            }
            else if (fluidSourceSyringe.NeedsReplaced)
            {
                return colorMap.mainBackground;
            }
            return rootBackgroundColor;
        }
    }

    Text {
        id: syringeIconInner
        anchors.centerIn: parent
        font.family: fontDeviceIcon.name
        font.pixelSize: parent.height
        text: "\ue91e"
        opacity: {
            if (isAlertActive)
            {
                return 0.5;
            }
            else if ( (fluidSourceSyringe === undefined) ||
                      (fluidSourceSyringe.InstalledAt === undefined) )
            {
                return 1;
            }
            return 0.5;
        }

        color: {
            if (isAlertActive)
            {
                return colorMap.errText;
            }
            else if ( (fluidSourceSyringe === undefined) ||
                      (fluidSourceSyringe.InstalledAt === undefined) )
            {
                return colorMap.deviceIconMissing;
            }
            else if (fluidSourceSyringe.NeedsReplaced)
            {
                return colorMap.deviceIconWarningState;
            }
            else if ( (fluidSourceSyringe.SourcePackages === undefined) || (fluidSourceSyringe.SourcePackages.length === 0) )
            {
                return rootBackgroundColor;
            }
            return syringeColor;
        }
    }

    Text {
        id: textSyringeVolume
        anchors.fill: parent
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        color: colorMap.text01
        font.pointSize: Math.max(parent.height * 0.08, 12)
        font.bold: true
        text:  {
            if ( (fluidSourceSyringe === undefined) ||
                 (fluidSourceSyringe.SourcePackages === undefined) ||
                 (fluidSourceSyringe.SourcePackages.length === 0) ||
                 (fluidSourceSyringe.NeedsReplaced) )
            {
                return "";
            }
            return syringeVolume.toFixed(0);
        }
    }

    AutoEmptyEnabledIcon {
        visible: autoEmptyEnabled
        anchors.top: parent.top
        anchors.topMargin: parent.height * 0.05
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width * 0.3
        height: parent.height * 0.17
    }
}
