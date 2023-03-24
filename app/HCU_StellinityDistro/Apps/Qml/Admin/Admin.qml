import QtQuick 2.12

import "AdminContrasts"
import "AdminSettings"
import "AdminService"
import "AdminAbout"
import "../Widgets"
import "../Widgets/Popup"

Rectangle {
    property string hcuBuildType: dsSystem.hcuBuildType
    property string servicePasscode: dsCfgGlobal.servicePasscode
    property string haspKeyEnforcementServiceKey: dsCapabilities.haspKeyEnforcementServiceKey
    property var accessProtected: dsCfgGlobal.accessProtected
    property string accessCode: dsCfgGlobal.accessCode
    property string nextPageSelected: ""
    property int frameMargin: dsCfgLocal.screenW * 0.02
    property int rightFrameWidth: (dsCfgLocal.screenW * 0.22) + (frameMargin * 2)
    property int buttonWidth: width * 0.15
    property int buttonHeight: height * 0.2
    property var activeAlerts: dsAlert.activeAlerts

    visible: false
    color: colorMap.mainBackground

    ListModel {
        id: listModelNormal
        ListElement { name: "Contrasts" }
        ListElement { name: "Settings" }
        ListElement { name: "Service" }
        ListElement { name: "About" }
    }

    ListModel {
        id: listModelProduction
        ListElement { name: "Service" }
        ListElement { name: "About" }
    }

    GridView {
        id: optionSelectList
        y: parent.height * 0.05
        width: parent.width * 0.8
        height: parent.height - y - actionBar.height
        anchors.horizontalCenter: parent.horizontalCenter

        cellWidth: parent.width * 0.2
        cellHeight: cellWidth

        ScrollBar {}
        ListFade {}

        model: (hcuBuildType == "PROD") ? listModelProduction : listModelNormal
        delegate: GenericButton {
            width: optionSelectList.cellWidth * 0.9
            height: width
            radius: 0
            color: colorMap.titleBarBackground
            smooth: true

            content: [
                Image {
                    y: getIconY(name, parent.height)
                    source: getIcon(name)
                    sourceSize.width: getIconWidth(name, parent.width)
                    sourceSize.height: getIconHeight(name, parent.height)
                    width: sourceSize.width
                    height: sourceSize.height
                    anchors.horizontalCenter: parent.horizontalCenter
                },
                Text {
                    y: parent.height * 0.023
                    x: parent.width * 0.82
                    height: parent.height * 0.18
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    font.pixelSize: height * 0.9
                    font.family: fontIcon.name
                    text: getLockedState(name) ? "\ue902" : ""
                    color: colorMap.text01
                },
                Text {
                    y: parent.height * 0.7
                    width: parent.width
                    height: parent.height - y
                    text: getTitle(name)
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    font.pixelSize: height * 0.33
                    font.family: fontRobotoLight.name
                    color: colorMap.text01
                }
            ]

            onBtnClicked: {
                adminBtnClicked(name);
            }

            Component.onCompleted: {
                optionSelectList.dragStarted.connect(reset);
            }

            Component.onDestruction: {
                optionSelectList.dragStarted.disconnect(reset);
            }
        }
    }

    Rectangle {
        id: adminPages
        color: "transparent"
        width: parent.width
        height: parent.height - actionBar.height

        AdminContrasts {}
        AdminSettings {}
        AdminService {}
        AdminAbout {}
    }

    AdminActionBar {
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

    function slotScreenStateChanged()
    {
        visible = (appMain.screenState.indexOf("Admin-") >= 0);
        reload();
    }

    function reload()
    {
        if (!visible)
        {
            return;
        }

        optionSelectList.visible = (appMain.screenState === "Admin-Select");
        adminPages.visible = (appMain.screenState !== "Admin-Select");
    }

    function getTitle(name)
    {
        var title;

        if (name === "Contrasts")
        {
            title = "T_AdminTileDefinition_Contrasts";
        }
        else if (name === "Settings")
        {
            title = "T_AdminTileDefinition_Settings";
        }
        else if (name === "Service")
        {
            title = "T_AdminTileDefinition_Service";
        }
        else if (name === "About")
        {
            title = "T_AdminTileDefinition_About";
        }
        else
        {
            title = name;
        }
        return translate(title);
    }

    function getIconY(name, widthTotal)
    {
        if (name === "Contrasts")
        {
            return widthTotal * 0.22;
        }
        else
        {
            return widthTotal * 0.2;
        }
    }

    function getIconWidth(name, widthTotal)
    {
        if (name === "Contrasts")
        {
            return widthTotal * 0.3;
        }
        else
        {
            return widthTotal * 0.45;
        }
    }

    function getIconHeight(name, heightTotal)
    {
        if (name === "Contrasts")
        {
            return heightTotal * 0.4;
        }
        else
        {
            return heightTotal * 0.45;
        }
    }


    function getLockedState(name)
    {
        if (accessProtected === undefined)
        {
            return false;
        }
        else if (hcuBuildType === "PROD")
        {
            return false;
        }

        // if "ServiceModeRestartRequired" alert is active, remove all locks
        for (var alertIdx = 0; alertIdx < activeAlerts.length; alertIdx++)
        {
            if (activeAlerts[alertIdx].CodeName === "ServiceModeRestartRequired")
            {
                // Service mode already visited. Unlocked
                return false;
            }
        }

        if ( (name === "Contrasts") ||
                  (name === "Settings") )
        {
            return (accessCode != "") && (accessProtected[name]);
        }
        else if (name === "Service")
        {
            if (hcuBuildType === "DEV")
            {
                return false;
            }

            return true;
        }

        return false;
    }

    function getIcon(name)
    {
        if (name === "Contrasts")
        {
            return imageMap.adminMenuContrasts;
        }
        else if (name === "Settings")
        {
            return imageMap.adminMenuSettings;
        }
        else if (name === "Service")
        {
            return imageMap.adminMenuService;
        }
        else if (name === "About")
        {
            return imageMap.adminMenuAbout;
        }

        return "";
    }

    function adminBtnClicked(name)
    {
        if (name === "Contrasts")
        {
            if (getLockedState(name))
            {
                nextPageSelected = "Admin-Contrasts";
                popupManager.popupEnterPassCode.titleText = "T_EnterAdminAccessCode";
                openInputPad();
            }
            else
            {
                appMain.setScreenState("Admin-Contrasts");
            }
        }
        else if (name === "Settings")
        {
            if (getLockedState(name))
            {
                nextPageSelected = "Admin-Settings";
                popupManager.popupEnterPassCode.titleText = "T_EnterAdminAccessCode";
                openInputPad();
            }
            else
            {
                appMain.setScreenState("Admin-Settings");
            }
        }
        else if (name === "Service")
        {
            if ( (getLockedState("Service")) &&
                 (servicePasscode != "") )
            {
                // Service Passcode required
                nextPageSelected = "Admin-Service-Select";
                popupManager.popupEnterPassCode.titleText = "T_EnterServiceAccessCode";
                openInputPad();
            }
            else
            {
                appMain.setScreenState("Admin-Service-Select");
            }
        }
        else if (name === "About")
        {
            appMain.setScreenState("Admin-About");
        }
    }

    function openInputPad()
    {
        popupManager.popupEnterPassCode.open();
        widgetInputPad.signalValueChanged.disconnect(slotInputPadValChanged);
        widgetInputPad.signalClosed.disconnect(slotInputPadClosed);
        widgetInputPad.blurBackgroundVisible = true;
        widgetInputPad.openTextPad(undefined, popupManager.popupEnterPassCode.textWidget, 0, 10);
        widgetInputPad.signalValueChanged.connect(slotInputPadValChanged);
        widgetInputPad.signalClosed.connect(slotInputPadClosed);
    }

    function slotInputPadValChanged(newValue)
    {
        popupManager.popupEnterPassCode.setValue(newValue);
    }

    function slotInputPadClosed(modified)
    {
        widgetInputPad.signalValueChanged.disconnect(slotInputPadValChanged);
        widgetInputPad.signalClosed.disconnect(slotInputPadClosed);

        popupManager.popupEnterPassCode.textWidget.text = "";
        popupManager.popupEnterPassCode.close();

        if (modified)
        {
            var codeEnteredOk = false;

            if (nextPageSelected == "Admin-Service-Select")
            {
                if (servicePasscode === haspKeyEnforcementServiceKey)
                {
                    // Raise alert when hasp key is being enforced and the user
                    // is trying to enter a passcode
                    var data = "Service Passcode is disabled, HASP key required.";
                    dsAlert.slotActivateAlert("HaspKeyEnforcement", data);
                    return;
                }
                // Requires service passCode
                else if (widgetInputPad.currentValue === servicePasscode)
                {
                    codeEnteredOk = true;
                }
            }
            else
            {
                // Requires access code
                if ( widgetInputPad.currentValue === accessCode)
                {
                    codeEnteredOk = true;
                }
            }

            if (codeEnteredOk)
            {
                if (nextPageSelected.indexOf("Admin-Service-") >= 0)
                {
                    data = "Passcode;Unknown;Unknown";
                    dsAlert.slotActivateAlert("ServiceModeRestartRequired", data);
                    appMain.setShortcutEnabled(true);

                }
                appMain.setScreenState(nextPageSelected);
            }
            else
            {
                logError("Bad Service passcode is entered");
                soundPlayer.playError();
            }
        }
    }
}

