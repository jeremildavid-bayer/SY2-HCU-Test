import QtQuick 2.3
import QtQuick.Window 2.2

Window {
    id: topWindow
    x: 0
    width: 1900
    height: 1200
    visible: true
    objectName: "main"
    color: "lightblue"

    PlayGround {
        id: window
        anchors.fill: parent
    }
}





