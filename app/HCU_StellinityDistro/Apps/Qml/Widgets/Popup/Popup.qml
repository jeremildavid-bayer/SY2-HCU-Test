import QtQuick 2.12
import QtGraphicalEffects 1.12
import ".."
import "../../Util.js" as Util

Rectangle {
    property string type: "NORMAL" // "NORMAL", "INFO", "WARNING", "SPECIAL_WARNING", "PLAIN", "QUIET"
    property string titleText: ""
    property string titleBarColor: {
        if (type === "SPECIAL_WARNING") return colorMap.actionButtonBackground;
        return "transparent"
    }
    property bool showTitleBarBorder: {
        if (type === "SPECIAL_WARNING") return false;
        return true;
    }
    property string okBtnText: translationRequired ? "T_OK" : "OK"
    property string cancelBtnText: translationRequired ? "T_Cancel" : "Cancel"
    property string otherBtnText: ""
    property bool showCancelBtn: true
    property bool showOkBtn: true
    property bool showOtherBtn: false
    property bool showServiceBtn: false
    property bool enableCancelBtn: true
    property bool enableOkBtn: true
    property bool enableOtherBtn: true
    property alias content: mainRect.children
    property alias mainRect: mainRect
    property int contentSurroundingHeight: getContentSurroundingHeight();

    function getContentSurroundingHeight() {
        var ret = titleBar.y + titleBar.height + contentMarginTop + contentMarginBottom + btnMarginBottom;
        if ( (showOkBtn) ||
             (showCancelBtn) ||
             (showOtherBtn) ||
             (showServiceBtn) )
        {
            ret += btnHeight;
        }

        return ret;
    }

    property int contentHeight: height - contentSurroundingHeight
    property int contentWidth: width * 0.9
    property int widthMin: dsCfgLocal.screenW * 0.55
    property int heightMin: dsCfgLocal.screenH * 0.44
    property int dividerHeight: 4
    property int titleHeight: dsCfgLocal.screenH * 0.09
    property int btnHeight: dsCfgLocal.screenH * 0.08
    property int btnWidth: dsCfgLocal.screenW * 0.14
    property var backgroundWidget
    property var curBackgroundWidget
    property bool translationRequired: true
    property int contentfontPixelSize: dsCfgLocal.screenH * 0.03
    property int titleFontPixelSize: dsCfgLocal.screenH * 0.032
    property int contentMarginTop: dsCfgLocal.screenH * 0.02
    property int contentMarginBottom: dsCfgLocal.screenH * 0.027
    property int btnMarginBottom: dsCfgLocal.screenH * 0.04

    property string servicePasscode: dsCfgGlobal.servicePasscode
    property var activeAlerts: dsAlert.activeAlerts

    property int popupId: 0
    property int topLevel: 0 // 0-3, 0:bottom, 3:top
    property string screenState: appMain.screenState
    property bool showFromStartupScreen: false
    property bool showDuringServicing: true

    signal opened();
    signal closed();
    signal btnOkClicked()
    signal btnCancelClicked()
    signal btnOtherClicked()

    id: root
    state: "INACTIVE"
    visible: false
    z: popupId
    width: widthMin
    height: heightMin
    anchors.horizontalCenter: parent.horizontalCenter
    anchors.horizontalCenterOffset: -( (popupManager.openPopups.length - popupId) * 10)
    anchors.verticalCenter: parent.verticalCenter
    anchors.verticalCenterOffset: -( (popupManager.openPopups.length - popupId) * 10)

    color: colorMap.popupBackground;

    onHeightMinChanged: {
        root.height = Qt.binding(function() { return heightMin; });
    }

    states: [
        State { name: "ACTIVE" },
        State { name: "INACTIVE" }
    ]

    transitions: [
        Transition {
            from: "INACTIVE"
            to: "ACTIVE"
            SequentialAnimation {
                ScriptAction {
                    script: {
                        curBackgroundWidget = getBackgroundWidget();

                        if (curBackgroundWidget !== undefined)
                        {
                            curBackgroundWidget.open([root], false, null);
                        }
                        mainRect.visible = false;
                        root.visible = true;
                        root.z = Qt.binding(function() { return popupId; });
                        root.width = Qt.binding(function() { return widthMin; });
                        root.height = Qt.binding(function() { return heightMin; });
                    }
                }

                // Add Animation (optional)

                ScriptAction {
                    script: {
                        mainRect.visible = true;
                        handleOpenState();
                    }
                }
            }
        },
        Transition {
            from: "ACTIVE"
            to: "INACTIVE"
            SequentialAnimation {
                ScriptAction {
                    script: {
                        mainRect.visible = false;
                    }
                }

                // Add Animation (optional)

                ScriptAction {
                    script: {
                        if (curBackgroundWidget !== undefined)
                        {
                            curBackgroundWidget.close([root]);
                            root.visible = false;
                        }
                        popupManager.removePopup(root);
                    }
                }
            }
        }
    ]


    MouseArea {
        // MouseArea to get the focus for the application (so Alt+4 can work)
        anchors.fill: parent
        onClicked: {
            appMain.requestActivate();
        }
    }


    Rectangle {
        id: titleBar
        anchors.top: parent.top
        visible: (type !== "PLAIN")
        height: (type == "PLAIN") ? 0 : titleHeight
        width: parent.width
        color: titleBarColor

        Text {
            anchors.left: icon1.right
            anchors.leftMargin: parent.width * 0.01
            anchors.right: icon2.left
            anchors.rightMargin: parent.width * 0.01
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            text: translationRequired ? translate(titleText) : titleText
            color: colorMap.blk01
            font.family: fontRobotoBold.name
            font.pixelSize: titleFontPixelSize
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            minimumPixelSize: font.pixelSize * 0.5
            fontSizeMode: Text.Fit
        }

        Rectangle {
            id: titleBarBorder
            visible: showTitleBarBorder
            y: parent.height - height
            width: parent.width * 0.95
            height: 4
            color: colorMap.white01
            anchors.horizontalCenter: parent.horizontalCenter
        }

        PopupIcon {
            id: icon1
            x: 0
            width: parent.width * 0.15
            height: parent.height * 0.44
            y: parent.height * 0.24
            iconType: type
        }


        PopupIcon {
            id: icon2
            x: parent.width - width
            width: parent.width * 0.15
            height: parent.height * 0.44
            y: parent.height * 0.24
            iconType: type
        }
    }

    Row {
        id: btns
        anchors.horizontalCenter: parent.horizontalCenter
        height: btnHeight
        y: parent.height - height - btnMarginBottom
        spacing: parent.width * 0.04

        GenericIconButton {
            id: btnService
            visible: getBtnServiceVisible()
            height: parent.height
            width: btnWidth * 0.3
            color: colorMap.keypadButton
            iconFontFamily: fontAwesome.name
            iconText: "\uf0ad"
            iconColor: colorMap.text01

            onBtnClicked: {
                if (titleText != "")
                {
                    logDebug("Popup: [" + titleText + "]: Btn(ServiceMode) clicked");
                }
                logPopupActivities("buttonClick", "Btn(ServiceMode) clicked");
                setScreenState("Admin-Service-Select");
                close();
            }
        }

        GenericIconButton {
            id: btnOther
            enabled: enableOtherBtn
            visible: showOtherBtn
            height: parent.height
            width: btnWidth
            color: colorMap.keypadButton
            iconFontFamily: fontRobotoBold.name
            iconText: translationRequired ? translate(otherBtnText) : otherBtnText
            iconColor: colorMap.text01

            onBtnClicked: {
                if (titleText != "")
                {
                    logDebug("Popup: [" + titleText + "]: Btn(" + otherBtnText + ") clicked");
                }
                logPopupActivities("buttonClick", "Btn(" + otherBtnText + ") clicked");
                btnOtherClicked();
            }
        }

        GenericIconButton {
            id: btnCancel
            enabled: enableCancelBtn
            visible: showCancelBtn
            height: parent.height
            width: btnWidth
            color: colorMap.keypadButton
            iconFontFamily: fontRobotoBold.name
            iconText: translationRequired ? translate(cancelBtnText) : cancelBtnText
            iconColor: colorMap.text01

            onBtnClicked: {
                if (titleText != "")
                {
                    logDebug("Popup: [" + titleText + "]: Btn(" + cancelBtnText + ") clicked");
                }
                logPopupActivities("buttonClick", "Btn(" + cancelBtnText + ") clicked");
                btnCancelClicked();
            }
        }

        GenericIconButton {
            id: btnOk
            enabled: enableOkBtn
            visible: showOkBtn
            height: parent.height
            width: btnWidth
            color: {
                if ( (showOkBtn) &&
                     ( (showCancelBtn) || (showOtherBtn) ) )
                {
                    return colorMap.keypadButton;
                }
                return colorMap.actionButtonBackground;
            }
            iconFontFamily: fontRobotoBold.name
            iconText: translationRequired ? translate(okBtnText) : okBtnText
            iconColor: {
                if ( (showOkBtn) &&
                     ( (showCancelBtn) || (showOtherBtn) ) )
                {
                    return colorMap.text01;
                }
                return colorMap.actionButtonText;
            }
            onBtnClicked: {
                if (titleText != "")
                {
                    logDebug("Popup: [" + titleText + "]: Btn(" + okBtnText + ") clicked");
                }
                logPopupActivities("buttonClick", "Btn(" + okBtnText + ") clicked");
                btnOkClicked();
            }
        }
    }

    Item {
        id: mainRect
        anchors.top: titleBar.bottom
        anchors.topMargin: contentMarginTop
        height: contentHeight
        width: contentWidth
        anchors.horizontalCenter: parent.horizontalCenter
        clip: true
    }

    onScreenStateChanged: {
        if (!isOpen())
        {
            return;
        }

        // Change background widget if required
        var newBackgroundWidget = getBackgroundWidget();
        if (newBackgroundWidget !== curBackgroundWidget)
        {
            if (curBackgroundWidget !== undefined)
            {
                curBackgroundWidget.close([root]);
            }
            if (newBackgroundWidget !== undefined)
            {
                newBackgroundWidget.open([root], false, null);
            }
            curBackgroundWidget = newBackgroundWidget;
        }

        if ( (showFromStartupScreen) &&
             (appMain.screenStatePrev != "Startup") &&
             (appMain.screenState == "Startup") )
        {
            // popup was open from startup
            handleOpenState();
        }
        else if ( (!showFromStartupScreen) &&
                  (appMain.screenStatePrev == "Startup") &&
                  (appMain.screenState != "Startup") )
        {
            // popup was open but waiting for screen to be NOT Startup
            handleOpenState();
        }
        else if ( (!showDuringServicing) &&
                  (appMain.screenStatePrev.indexOf("Admin-Service-") >= 0) &&
                  (appMain.screenState.indexOf("Admin-Service-") < 0) )
        {
            // popup was open but waiting for screen to be NOT Service
            handleOpenState();
        }
    }

    function close(reason = "Other")
    {
        if (root.state == "INACTIVE")
        {
            return;
        }

        root.state = "INACTIVE";

        if (titleText != "")
        {
            logDebug("Popup: [" + titleText + "] Closed");
        }
        logPopupActivities("deactivate", reason);
        closed();
    }

    function open()
    {
        if (widgetKeyboard.isOpen())
        {
            widgetKeyboard.close(false);
        }

        if (widgetInputPad.isOpen())
        {
            widgetInputPad.close(false);
        }

        if (root.state == "ACTIVE")
        {
            return;
        }

        root.state = "ACTIVE";

        if (titleText != "")
        {
            logDebug("Popup: [" + titleText + "] Open");
        }
        logPopupActivities("activate", null);
        opened();

    }

    function isOpen()
    {
        return root.state == "ACTIVE";
    }

    function handleOpenState()
    {
        var openOk = false;

        if (screenState == "OFF")
        {
            //logDebug("handleOpenState: Popup[" + titleText + "] : Screen is not started yet");
        }
        else if (!isOpen())
        {
            //logDebug("handleOpenState: Popup[" + titleText + "] : Popup closed");
        }
        else if ( (!showFromStartupScreen) && (screenState == "Startup") )
        {
            //logDebug("handleOpenState: Popup[" + titleText + "] : Screen is not ready to show: !showFromStartupScreen");
        }
        else if ( (!showDuringServicing) && (screenState.indexOf("Admin-Service-") >= 0) )
        {
            //logDebug("handleOpenState: Popup[" + titleText + "] : Screen is not ready to show: !showDuringServicing");
        }
        else
        {
            openOk = true;
        }

        if (!openOk)
        {
            curBackgroundWidget.close([root]);
            root.visible = false;
            return;
        }

        curBackgroundWidget.open([root], false, null);
        root.visible = true;
        popupManager.addPopup(root);

        if (type == "WARNING")
        {
            soundPlayer.playModalUnexpected();
        }
        else if (type == "QUIET")
        {
            // no sound
        }
        else
        {
            soundPlayer.playModalExpected();
        }
    }

    function getBtnServiceVisible()
    {
        if (!showServiceBtn)
        {
            return false;
        }

        if (hcuBuildType == "DEV")
        {
            return true;
        }

        for (var alertIdx = 0; alertIdx < activeAlerts.length; alertIdx++)
        {
            if (activeAlerts[alertIdx].CodeName === "ServiceModeRestartRequired")
            {
                return true;
            }
        }
        return false;
    }

    function getBackgroundWidget()
    {
        if (backgroundWidget !== undefined)
        {
            return backgroundWidget;
        }
        else if ( (showFromStartupScreen) &&
                  ( (screenState == "OFF") || (screenState == "Startup") ) )
        {
            return startupPageBlurBackground;
        }
        else
        {
            return blurBackground;
        }
    }

    function logPopupActivities(type, reason)
    {
        var alertData = {};
        var buttons = [];

        if (showOkBtn)
        {
            buttons.push(okBtnText);
        }

        if (showCancelBtn)
        {
            buttons.push(cancelBtnText);
        }

        if (showOtherBtn)
        {
            buttons.push(cancelBtnText);
        }

        alertData["Title"] = titleText;
        alertData["Description"] = typeof contentText === "undefined"? "Object" : contentText;
        alertData["Buttons"] = buttons;

        switch (type)
        {
            case "buttonClick":
                // log the next popup
                timerSingleShot(1, function() {
                    if (isOpen())
                    {
                        // Use singleshot timer to make sure activate action is called after deactivate the old alert
                        timerSingleShot(1, function() {
                            logPopupActivities("activate", null);
                        });
                    }
                });
                // fall through
            case "deactivate":
                // Use singleshot timer to break binding loop with onActiveAlertsChanged in PopupAlertBase
                timerSingleShot(1, function() {
                    var oldAlertData = Util.copyObject(alertData);
                    alertData["ClosedBy"] = reason;
                    dsAlert.slotDeactivateAlertWithReason("SRUPopupActivated", JSON.stringify(alertData), JSON.stringify(oldAlertData), false);
                });
                break;
            case "activate":
                // Use singleshot timer to break binding loop with onActiveAlertsChanged in PopupAlertBase
                timerSingleShot(1, function() {
                    var alertActive = false;
                    for (var alertIdx = 0; alertIdx < activeAlerts.length; alertIdx++)
                    {
                        if (activeAlerts[alertIdx].CodeName === "SRUPopupActivated" && activeAlerts[alertIdx].Data === JSON.stringify(alertData))
                        {
                            alertActive = true;
                            break;
                        }
                    }
                    if (!alertActive)
                    {
                        dsAlert.slotActivateAlert("SRUPopupActivated", JSON.stringify(alertData));
                    }
                });
                break;
            case "contentChanged":
                // Use singleshot timer to make sure this action is called after deactivate the old alert
                timerSingleShot(1, function() {
                    for (var alertIdx = 0; alertIdx < activeAlerts.length; alertIdx++)
                    {
                        if (activeAlerts[alertIdx].CodeName === "SRUPopupActivated")
                        {
                            var newData = Util.copyObject(JSON.parse(activeAlerts[alertIdx].Data));
                            newData["ClosedBy"] = reason;
                            dsAlert.slotDeactivateAlertWithReason("SRUPopupActivated", JSON.stringify(newData), activeAlerts[alertIdx].Data, false);
                            break;
                        }
                    }
                    logPopupActivities("activate", null);
                });
                break;
        }
    }
}
