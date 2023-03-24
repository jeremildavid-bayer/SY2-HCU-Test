import QtQuick 2.12

DeviceManagerGenericButton {
    antialiasing: true
    panelName: "WASTE_BIN"

    content: [
        Item {
            anchors.centerIn: parent
            width: parent.width * 0.75
            height: parent.height * 0.7

            DeviceManagerWCIcon {
                anchors.fill: parent
            }
        }
    ]
}
