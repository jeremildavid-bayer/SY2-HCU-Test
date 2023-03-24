import QtQuick 2.12
import ".."

Item {
    property string iconType: "NORMAL" // "NORMAL", "INFO", "WARNING", "PLAIN"

    Item {
        id: normalFrame
        anchors.fill: parent
        visible: iconType == "NORMAL"
        Text {
            anchors.fill: parent
            color: colorMap.blk01
            font.family: fontIcon.name
            font.pixelSize: height
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            text: ""
        }
    }

    Item {
        // all icons other than "WARNING"
        id: iconFrame
        anchors.fill: parent
        visible: ((iconType == "INFO") || (iconType == "SPECIAL_WARNING"))

        Text {
            anchors.fill: parent
            color: colorMap.blk01
            font.family: fontIcon.name
            font.pixelSize: height
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            text: {
                if (!visible) return "";

                if (iconType === "INFO") {
                    return "\ue93a";
                } else if (iconType === "SPECIAL_WARNING") {
                    return "\uf071";
                }
            }
        }
    }

    Item {
        id: warningFrame
        anchors.fill: parent
        visible: iconType == "WARNING"

        WarningIcon {
            anchors.fill: parent
        }
    }
}
