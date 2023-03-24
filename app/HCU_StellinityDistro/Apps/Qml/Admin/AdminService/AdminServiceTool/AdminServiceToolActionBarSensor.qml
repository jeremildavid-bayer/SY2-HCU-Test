import QtQuick 2.12
import "../../../Widgets"

Rectangle {
    property string stateStr: ""
    property string title: ""
    property string value: ""
    property string valueColor: colorMap.text02
    property string titleColor: colorMap.text02

    id: root
    height: parent.height
    width: parent.width * 0.08

    color: "transparent"
    border.color: valueColor
    border.width: height * 0.01

    Text {
        width: parent.width
        y: parent.height * 0.035
        height: parent.height * 0.3
        font.pixelSize: height * 0.5
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.Wrap
        text: title
        color: titleColor
        opacity: 0.8
    }

    Text {
        id: txtValue
        width: parent.width
        height: parent.height * 0.7
        y: parent.height * 0.3
        font.bold: true
        font.pixelSize: height * 0.16
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.Wrap
        text: value
        color: valueColor
    }
}

