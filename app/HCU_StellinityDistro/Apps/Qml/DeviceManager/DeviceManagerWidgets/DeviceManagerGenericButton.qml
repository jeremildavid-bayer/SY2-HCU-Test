import QtQuick 2.12
import "../../Widgets"

GenericButton {
    property string panelName: "NONE"
    property bool isAlertActive: false
    property string borderColor: "transparent"
    property int borderWidth: buttonSelectedBorderWidth
    property bool isSelected: activePanelName == panelName
    property bool isBusy: false
    property bool useDefaultFrameBackground: true

    color: isAlertActive ? colorMap.deviceButtonRed : colorMap.deviceButtonBackground
    clip: false

    Rectangle {
        id: borderBackground
        visible: useDefaultFrameBackground && isSelected
        color: "transparent"
        radius: parent.radius
        anchors.centerIn: parent
        width: parent.width
        height: parent.height
        border.width: borderWidth
        border.color: colorMap.deviceIconSelected
    }

    LoadingGif {
        visible: isBusy
    }

    WarningIcon {
        width: alertIconWidth
        height: width
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.rightMargin: -width / 2
        anchors.topMargin: -height / 2
        visible: isAlertActive
        z: pressedCoverContent.z + 1
   }

    onBtnClicked: {
        if (widgetKeyboard.isOpen())
        {
            widgetKeyboard.close(true);
        }

        if (widgetInputPad.isOpen())
        {
            widgetInputPad.close(true);
        }

        setActivePanel(panelName);
    }
}
