import QtQuick 2.12

Item {
    id: root
    AutoEmptyEnabledIcon {
        id: autoEmptyEnabledIcon
        width: parent.width * 0.06
        height: parent.height
        anchors.verticalCenter: parent.verticalCenter
    }

    Text {
        id: autoEmptyModeEnabledText
        anchors.left: autoEmptyEnabledIcon.right
        anchors.leftMargin: autoEmptyEnabledIcon.width * 0.3
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.top: parent.top
        text: translate("T_ConfigItem_Configuration_Behaviors_AutoEmptyModeEnabled_Name")
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignLeft
        color: colorMap.text01
        font.pixelSize: height * 0.45
        font.family: fontRobotoLight.name
        minimumPixelSize: font.pixelSize * 0.8
        fontSizeMode: Text.Fit
        elide: Text.ElideRight
    }
}
