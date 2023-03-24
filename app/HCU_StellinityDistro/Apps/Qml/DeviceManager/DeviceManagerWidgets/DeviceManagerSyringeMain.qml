import QtQuick 2.12
import "../../CommonElements"

Item {
    property var fluidSourceSyringe
    property double syringeVolume: 0
    property string syringeColor: "transparent"
    property double volumeMax: dsCapabilities.volumeMax
    property bool autoEmptyEnabled: false

    id: root
    antialiasing: true

    Item {
        clip: true
        width: syringeEmptyImage.width - 2
        height: parent.height
        anchors.centerIn: parent

        Text {
            id: syringeBackground
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            font.family: fontDeviceIcon.name
            font.pixelSize: parent.height
            visible: true
            text: "\ue919"
            color: {
                if ( (fluidSourceSyringe === undefined) ||
                     (fluidSourceSyringe.InstalledAt === undefined) )
                {
                    return colorMap.deviceIconMissing;
                }
                else if (fluidSourceSyringe.NeedsReplaced)
                {
                    return colorMap.deviceIconWarningState;
                }
                else if ( (fluidSourceSyringe.SourcePackages !== undefined) && (fluidSourceSyringe.SourcePackages.length > 0) )
                {
                    return syringeColor;
                }
                return "transparent";
            }
        }
    }

    Item {
        clip: true
        width: syringeBackground.width - 7
        height: parent.height
        anchors.centerIn: parent

        Text {
            id: syringePlungerTop
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: syringePlungerFill.top
            anchors.bottomMargin: -7 - (parent.height * 0.091)
            font.family: fontDeviceIcon.name
            font.pixelSize: syringeBackground.height
            visible: {
                if ( (fluidSourceSyringe === undefined) ||
                     (fluidSourceSyringe.InstalledAt === undefined) )
                {
                    return false;
                }
                return true;
            }
            text: "\ue917"
            color: colorMap.devicePlunger
        }

        Rectangle {
            id: syringePlungerFill
            anchors.bottom: syringePlungerBot.bottom
            anchors.bottomMargin: parent.height * 0.091
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            height: {
                var syringeVolBuf = Math.max(syringeVolume, 0);
                syringeVolBuf = Math.min(syringeVolBuf, volumeMax);
                return ((root.height - (syringePlungerTop.font.pixelSize * 0.091) - (syringePlungerBot.font.pixelSize * 0.091) ) / volumeMax) * (volumeMax - syringeVolBuf);
            }
            color: colorMap.devicePlunger
            visible: syringePlungerTop.visible
        }

        Text {
            id: syringePlungerBot
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            font.family: fontDeviceIcon.name
            font.pixelSize: syringeBackground.font.pixelSize
            visible: syringePlungerFill.visible
            text: "\ue916"
            color: colorMap.devicePlunger
        }

        Item {
            id: volumeRow
            anchors.bottom: parent.bottom
            anchors.bottomMargin: parent.height * 0.018
            anchors.horizontalCenter: parent.horizontalCenter
            width: textSyringeVolume.contentWidth + textSyringeVolumeUnit.contentWidth
            height: parent.height * 0.1
            visible: (fluidSourceSyringe !== undefined) && (fluidSourceSyringe.SourcePackages !== undefined) && (fluidSourceSyringe.SourcePackages.length > 0) && (!fluidSourceSyringe.NeedsReplaced)

            Text {
                id: textSyringeVolume
                anchors.bottom: parent.bottom
                text: syringeVolume.toFixed(0)
                verticalAlignment: Text.AlignBottom
                width: contentWidth
                height: parent.height * 0.7
                color: colorMap.text02
                font.pixelSize: height
                font.family: fontRobotoBold.name
            }
            Text {
                id: textSyringeVolumeUnit
                text: translate("T_Units_ml")
                anchors.bottom: parent.bottom
                anchors.left: textSyringeVolume.right
                width: contentWidth * 1.1
                height: parent.height * 0.7
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignBottom
                color: colorMap.text02
                font.pixelSize: height
                font.family: fontRobotoLight.name
            }
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

    Image {
        id: syringeEmptyImage
        width: parent.width * 0.87
        height: parent.height
        anchors.centerIn: parent
        visible: {
            if ( (fluidSourceSyringe === undefined) ||
                 (fluidSourceSyringe.InstalledAt === undefined) )
            {
                return false;
            }
            else if (fluidSourceSyringe.NeedsReplaced)
            {
                return false;
            }
            return true;
        }
        sourceSize.height: height
        sourceSize.width: width
        source: {
            if (syringeColor === colorMap.contrast1)
            {
                return imageMap.deviceReservoirEmptyC1;
            }
            else if (syringeColor === colorMap.contrast2)
            {
                return imageMap.deviceReservoirEmptyC2;
            }
            return imageMap.deviceReservoirEmptySaline;
        }
    }
}
