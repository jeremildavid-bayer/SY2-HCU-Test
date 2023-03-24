import QtQuick 2.12
import QtQuick.Controls 2.12
import "../../../Widgets"
import "../../../Util.js" as Util

Row {
    property string val1: ""
    property string val2: ""
    property string color1: colorMap.text01
    property string color2: colorMap.text01

    anchors.fill: parent
    Rectangle {
        width: parent.width / 2
        height: parent.height
        color: (color1 === colorMap.text01) ? "transparent" : color1
        Text {
            width: parent.width * 0.85
            height: parent.height
            anchors.centerIn: parent
            text: val1
            verticalAlignment: Text.AlignVCenter
            color: (color1 === colorMap.text01) ? color1 : colorMap.white01
            font.pixelSize: parent.height * 0.35
            wrapMode: Text.Wrap
        }
    }

    Rectangle {
        width: 1
        height: parent.height
        color: colorMap.text02
    }

    Rectangle {
        width: (parent.width / 2) - 1
        height: parent.height
        color: (color2 === colorMap.text01) ? "transparent" : color2
        Text {
            width: parent.width * 0.85
            height: parent.height
            anchors.centerIn: parent
            text: val2
            verticalAlignment: Text.AlignVCenter
            color: (color2 === colorMap.text01) ? color2 : colorMap.white01
            font.pixelSize: parent.height * 0.35
            wrapMode: Text.Wrap
        }
    }
}
