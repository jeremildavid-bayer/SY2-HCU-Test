import QtQuick 2.3
import QtQuick.Window 2.2

Window {
    id: window
    width: 1000
    height: 600
    visible: true
    objectName: "main"

    OptionList {

        anchors.top: parent.top
        anchors.topMargin: 20
        anchors.left: parent.left
        anchors.leftMargin: 20

        width: parent.width / 2
        height: parent.height
    }
}
