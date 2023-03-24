import QtQuick 2.12
import QtQml 2.15
import "../Widgets"

Rectangle {
    property int buttonWidth: width * 0.2
    property int buttonHeight: buttonWidth
    property bool aqdInProgress: false

    property var activeAlerts: []
    Binding on activeAlerts {
        when: visible
        value: dsAlert.activeAlerts;
        restoreMode: Binding.RestoreBinding
    }

    onActiveAlertsChanged:
    {
        for (var i = 0; i < activeAlerts.length; i++)
        {
            if (activeAlerts[i].CodeName === "AutomaticQualifiedDischargeInProgress")
            {
                aqdInProgress = true;
                break;
            }
        }
    }

    visible: false
    color: colorMap.mainBackground

    Row {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: -(actionBar.height / 2)
        spacing: parent.width * 0.05

        GenericIconButton {
            height: buttonHeight
            width: buttonWidth
            iconColor: colorMap.text01
            iconText: "\ue901"
            iconFontPixelSize: height * 0.5
            iconFontFamily: fontIcon.name
            captionText: translate("T_Shutdown")
            captionFontPixelSize: height * 0.08
            onBtnClicked: {
                if (aqdInProgress)
                {
                    var titleText = translate("T_AQDInProgress_Title");
                    var context = "T_AQDInProgress_Message";
                    var argList = [true];
                    var delayedOK = true;

                    openConfirmationPopup(titleText, context, shutdown, argList, delayedOK);
                }
                else
                {
                    shutdown(true);
                }
            }
        }

        GenericIconButton {
            height: buttonHeight
            width: buttonWidth
            iconColor: colorMap.text01
            iconText: "\ue943"
            iconFontPixelSize: height * 0.5
            iconFontFamily: fontIcon.name
            captionText: translate("T_Restart")
            captionFontPixelSize: height * 0.08
            onBtnClicked: {
                if (aqdInProgress)
                {
                    var titleText = translate("T_AQDInProgress_Title");
                    var context = "T_AQDInProgress_Message";
                    var argList = [false];
                    var delayedOK = true;

                    openConfirmationPopup(titleText, context, shutdown, argList, delayedOK);
                }
                else
                {
                    shutdown(false);
                }
            }
        }
    }

    ShutdownActionBar {
        id: actionBar
        Component.onCompleted: {
            this.parent = frameActionBar;
        }
    }

    Component.onCompleted: {
        appMain.screenStateChanged.connect(slotScreenStateChanged);
        slotScreenStateChanged();
    }

    Component.onDestruction: {
        appMain.screenStateChanged.disconnect(slotScreenStateChanged);
    }

    function shutdown(poweroff)
    {
        if (poweroff)
        {
            // shutdown
            dsSystem.slotShutdown(true);
            appMain.setInteractiveState(false, "Shutdown");
        }
        else
        {
            // reboot
            dsSystem.slotShutdown(false);
            appMain.setInteractiveState(false, "Reboot");
        }
    }

    function slotScreenStateChanged()
    {
        visible = (appMain.screenState === "Shutdown");
        reload();
    }

    function reload()
    {
        if (!visible)
        {
            return;
        }
    }

    function openConfirmationPopup(titleText, context, confirmFunction, confirmFunctionArgumentsList, delayedOK)
    {
        popupManager.popupActionConfirmation.titleText = titleText;
        popupManager.popupActionConfirmation.contentText = context;
        popupManager.popupActionConfirmation.okFunction = confirmFunction;
        popupManager.popupActionConfirmation.okFunctionArgumentsList = confirmFunctionArgumentsList;
        if (delayedOK)
        {
            popupManager.popupActionConfirmation.enableOkBtn = false;
            timerSingleShot(2000, function() {popupManager.popupActionConfirmation.enableOkBtn = true;});
        }
        else
        {
            popupManager.popupActionConfirmation.enableOkBtn = true;
        }

        popupManager.popupActionConfirmation.open();
    }
}
