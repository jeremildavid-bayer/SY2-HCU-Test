import QtQuick 2.12
import "../Widgets"

Rectangle {
    height: actionBarHeight
    anchors.bottom: parent.bottom
    anchors.right: parent.right
    anchors.left: parent.left
    color: colorMap.actionBarBackground

    Row {
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        height: parent.height * 0.6
        spacing: parent.width * 0.01

        GenericIconButton {
            id: btnBack
            color: "transparent"
            height: parent.height
            width: height
            iconColor: colorMap.text01
            iconText: "\ue909"
            iconFontPixelSize: parent.height * 0.6
            iconFontFamily: fontIcon.name
            disabledColor: "transparent"
            onBtnClicked: {
                appMain.setScreenState("Home");
            }
        }
    }

    Component.onCompleted: {
        appMain.screenStateChanged.connect(slotScreenStateChanged);
        slotScreenStateChanged();
    }

    Component.onDestruction: {
        appMain.screenStateChanged.disconnect(slotScreenStateChanged);
    }

    function slotScreenStateChanged()
    {
        visible = (appMain.screenState === "Shutdown");
    }
}
