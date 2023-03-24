import QtQuick 2.12
import "../Widgets"
import "../Widgets/Popup"

Rectangle {
    property var activeAlerts: dsAlert.activeAlerts
    property string hcuBuildType: dsSystem.hcuBuildType
    property string uiTheme: dsCfgLocal.uiTheme

    width: parent.width
    height: parent.height
    color: colorMap.homeBackground

    GenericIconButton {
        id: btnChangeTheme
        color: "transparent"
        width: parent.width * 0.12
        height: parent.height * 0.7
        x: parent.width * 0.7
        iconText: (colorMap === themes.colorMapPurity) ? "\ue905" : "\ue904"
        iconFontPixelSize: height * 0.4
        captionText: translate("T_DisplayTheme")
        onBtnClicked: {
            btnChangeTheme.interactive = false;
            if (uiTheme == "purity")
            {
                dsCfgLocal.slotUiThemeChanged("twilight");
            }
            else
            {
                dsCfgLocal.slotUiThemeChanged("purity");
            }

            // Enable the btnChangeTheme later. Reason: Changing theme take a lot of CPU processings, prevent rapid clicking.
            timerSingleShot(1000, function() {
                btnChangeTheme.interactive = true;
            });
        }
    }

    HomeActionBtnScreenLock {
        id: btnScreenLock
        width: parent.width * 0.12
        height: parent.height * 0.7
        x: parent.width * 0.845

        onSignalLocked: {
            btnScreenUnlock.reset();
            popupScreenLocked.open();
        }
    }

    GenericIconButton {
        id: btnPower
        color: "transparent"
        width: parent.width * 0.12
        height: parent.height * 0.7
        x: parent.width * 0.037
        iconText: "\ue901"
        iconFontPixelSize: height * 0.4
        captionText: translate("T_Power")
        onBtnClicked: {
            appMain.setScreenState("Shutdown");
        }
    }

    Popup {
        id: popupScreenLocked
        widthMin: dsCfgLocal.screenW * 0.5
        heightMin: dsCfgLocal.screenH * 0.5
        showCancelBtn: false
        showOkBtn: false
        backgroundWidget: entireBlurBackground
        type: "NORMAL"
        btnHeight: 0
        titleText: "T_ScreenLocked"
        content: [
            Item {
                width: parent.width * 0.9
                height: parent.height * 0.9
                anchors.centerIn: parent

                Text {
                    id: captionText
                    width: parent.width
                    anchors.top: parent.top
                    anchors.bottom: btnScreenUnlock.top
                    anchors.bottomMargin: popupScreenLocked.height * 0.04
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    text: translate("T_PressAndHoldToUnlockScreen")
                    font.pixelSize: popupScreenLocked.height * 0.06
                    font.family: fontRobotoLight.name
                    color: colorMap.blk01
                    wrapMode: Text.Wrap
                    fontSizeMode: Text.Fit
                    minimumPixelSize: font.pixelSize * 0.7
                }

                HomeActionBtnScreenLock {
                    id: btnScreenUnlock
                    anchors.bottom: parent.bottom
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: popupScreenLocked.width * 0.25
                    height: popupScreenLocked.height * 0.3
                    screenLockCaption: ""

                    onSignalLocked: {
                        btnScreenLock.reset();
                        popupScreenLocked.close();
                    }
                }
            }
        ]
    }

    Row {
        id: serviceWarningMessage
        spacing: parent.width * 0.01
        anchors.left: btnPower.right
        anchors.leftMargin: parent.width * 0.015
        anchors.verticalCenter: btnPower.verticalCenter
        height: parent.height * 0.4
        visible: false

        WarningIcon {
            height: parent.height * 0.6
            width: height
            anchors.verticalCenter: parent.verticalCenter
        }

        Column {
            anchors.verticalCenter: parent.verticalCenter
            Text {
                text: translate("T_ServiceModeRestartRequired_Name")
                font.family: fontRobotoBold.name
                font.pixelSize: serviceWarningMessage.height * 0.4
                color: colorMap.text01
                width: dsCfgLocal.screenW * 0.4
            }

            Text {
                text: translate("T_ServiceModeRestartRequired_UserDirection")
                font.family: fontRobotoLight.name
                font.pixelSize: serviceWarningMessage.height * 0.3
                color: colorMap.text01
                width: dsCfgLocal.screenW * 0.4
                wrapMode: Text.Wrap
            }
        }
    }

    Row {
        id: tradeshowWarningMessage
        spacing: parent.width * 0.01
        anchors.left: btnPower.right
        anchors.leftMargin: parent.width * 0.015
        anchors.verticalCenter: btnPower.verticalCenter
        height: parent.height * 0.4
        visible: false

        WarningIcon {
            height: parent.height * 0.6
            width: height
            anchors.verticalCenter: parent.verticalCenter
        }

        Column {
            anchors.verticalCenter: parent.verticalCenter
            Text {
                text: translate("T_TradeshowModeActive_Name")
                font.family: fontRobotoBold.name
                font.pixelSize: tradeshowWarningMessage.height * 0.4
                color: colorMap.text01
                width: dsCfgLocal.screenW * 0.4
            }

            Text {
                text: translate("T_TradeshowModeActive_UserDirection")
                font.family: fontRobotoLight.name
                font.pixelSize: tradeshowWarningMessage.height * 0.3
                color: colorMap.text01
                width: dsCfgLocal.screenW * 0.4
                wrapMode: Text.Wrap
            }
        }
    }

    onActiveAlertsChanged: {
        if (!visible)
        {
            return;
        }

        reloadWarningMessages();
    }

    onVisibleChanged: {
        if (!visible)
        {
            return;
        }

        reloadWarningMessages();
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
        visible = (appMain.screenState === "Home");
    }

    function reloadWarningMessages()
    {
        if (activeAlerts === undefined)
        {
            return;
        }

        if (hcuBuildType == "PROD")
        {
            return;
        }

        if (activeAlerts.filter(x => x.CodeName === "ServiceModeRestartRequired").length > 0)
        {
            serviceWarningMessage.visible = true;
            tradeshowWarningMessage.visible = false;
            return;
        }
        else if (activeAlerts.filter(x => x.CodeName === "TradeshowModeActive").length > 0) //If there is a service mode alert, we don't need to show the tradeshow alert...
        {
            serviceWarningMessage.visible = false;
            tradeshowWarningMessage.visible = true;
            return;
        }

        serviceWarningMessage.visible = false;
        tradeshowWarningMessage.visible = false;
    }
}


