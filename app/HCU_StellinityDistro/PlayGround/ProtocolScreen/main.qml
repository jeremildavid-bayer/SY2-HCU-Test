import QtQuick 2.3
import QtQuick.Window 2.2

Window {
    id: window
    width: 1200
    height: 700
    visible: true
    objectName: "main"

    ProtocolScreen {
        width: parent.width
        height: parent.height
    }
}
