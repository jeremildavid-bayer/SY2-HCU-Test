import QtQuick 2.12

Item {
    id: root
    property int horizontalAlignment: Text.AlignHCenter
    property int verticalAlignment: Text.AlignVCenter

    Text {
        font.family: fontIcon.name
        width: parent.width
        height: parent.height * 0.35
        verticalAlignment: parent.verticalAlignment
        horizontalAlignment: parent.horizontalAlignment
        font.pixelSize: height * 0.8
        text: "\ue930"
        color: colorMap.text01
    }

    Text {
        font.family: fontAwesome.name
        width: parent.width
        y: parent.height * 0.4
        height: parent.height * 0.4
        verticalAlignment: parent.verticalAlignment
        horizontalAlignment: parent.horizontalAlignment
        font.pixelSize: height
        text: "\uf0e2"
        color: colorMap.text01
    }

}
