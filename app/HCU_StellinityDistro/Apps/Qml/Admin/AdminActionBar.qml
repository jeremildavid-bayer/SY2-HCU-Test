import QtQuick 2.12
import "../Widgets"

Rectangle {
    property string statePath: dsSystem.statePath
    property var upgradeDigest: dsUpgrade.upgradeDigest

    width: parent.width
    height: parent.height
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
            enabled: getBackButtonEnableState()
            opacity: enabled ? 1 : 0.2
            onBtnClicked: {
                if (appMain.screenState == "Admin-Select")
                {
                    appMain.setScreenState("Home");
                }
                else if (appMain.screenState.indexOf("Admin-Service-Select") >= 0)
                {
                    appMain.setScreenState("Admin-Select");
                }
                else if (appMain.screenState.indexOf("Admin-Service-") >= 0)
                {
                    appMain.setScreenState("Admin-Service-Select");
                }
                else if (appMain.screenState.indexOf("Admin-") >= 0)
                {
                    appMain.setScreenState("Admin-Select");
                }
            }
        }
    }

    Item {
        id: pmDueMessage
        visible: ((appMain.screenState.indexOf("Admin-Service-Select") >= 0) && (lastPMPerformedDateString !== undefined) && (lastPMPerformedDateString !== ""))
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.topMargin: parent.height * 0.1
        width: parent.width * 0.4
        property int fontSize: height * 0.2
        property string lastPMPerformedDateString: dsCfgLocal.lastPMPerformedDateString

        WarningIcon {
            id: infoStringWarningIcon
            anchors.left: parent.left
            anchors.top: parent.top
            width: pmDueMessage.fontSize
            height: pmDueMessage.fontSize
            font.pixelSize: pmDueMessage.fontSize
        }

        Text {
            anchors.left: infoStringWarningIcon.right
            anchors.leftMargin: pmDueMessage.fontSize * 0.3
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom

            text: getServiceInfoString()
            horizontalAlignment: Text.AlignLeft
            font.pixelSize: pmDueMessage.fontSize
            fontSizeMode: Text.Fit
            wrapMode: Text.Wrap
            font.family: fontRobotoLight.name
            color: colorMap.text01

            function getServiceInfoString() {
                return "Preventative Maintenance Due\nLast maintenance performed on :\n" + pmDueMessage.lastPMPerformedDateString;
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
        visible = (appMain.screenState.indexOf("Admin-") >= 0);
    }

    function getBackButtonEnableState()
    {
        if ( (widgetInputPad.isOpen()) ||
             (widgetKeyboard.isOpen()) )
        {
            return false;
        }
        else if (statePath == "Servicing")
        {
            return false;
        }
        else if (upgradeDigest.State !== "Ready")
        {
            return false;
        }
        return true;
    }
}
