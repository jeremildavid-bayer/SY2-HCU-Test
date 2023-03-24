import QtQuick 2.12
import QtQuick.Controls 2.12
import "../../../Widgets"
import "../../../Util.js" as Util

Row {
    signal btnClicked(int batteryIdx)

    id: root
    anchors.fill: parent
    Item {
        width: parent.width / 2
        height: parent.height
        GenericIconButton {
            width: parent.width * 0.9
            height: parent.height * 0.8
            anchors.centerIn: parent
            iconText: "Send A"
            color: colorMap.gry01
            iconColor: colorMap.white01
            iconFontPixelSize: height * 0.4
            onBtnClicked: {
                root.btnClicked(0);
            }
        }
    }

    Rectangle {
        width: 1
        height: parent.height
        color: colorMap.text02
    }

    Item {
        width: (parent.width / 2) - 1
        height: parent.height
        GenericIconButton {
            width: parent.width * 0.9
            height: parent.height * 0.8
            anchors.centerIn: parent
            iconText: "Send B"
            color: colorMap.gry01
            iconColor: colorMap.white01
            iconFontPixelSize: height * 0.4
            onBtnClicked: {
                root.btnClicked(1);
            }
        }
    }
}
