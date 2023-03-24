import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.12
import "../Widgets"
import "../Util.js" as Util

Rectangle {
    property string productSoftwareVersion: dsCfgGlobal.productSoftwareVersion
    property string serialNumber: dsHardwareInfo.serialNumber
    property string modelNumber: dsHardwareInfo.modelNumber
    property var activeAlerts: dsAlert.activeAlerts
    property string hcuVersion: dsSystem.hcuVersion
    property string hcuBuildType: dsSystem.hcuBuildType
    property string serviceContactTelephone: dsCfgGlobal.serviceContactTelephone
    property string servicePasscode: dsCfgGlobal.servicePasscode
    property string haspKeyEnforcementServiceKey: dsCapabilities.haspKeyEnforcementServiceKey
    property string infoTextCompany: translate("T_BayerDivisionName")

    id: root
    anchors.fill: parent
    visible: false
    color: colorMap.startupBackground

    Item {
        id: rectMain
        width: root.width * 0.723
        height: root.height - actionBarHeight

        Image {
            id: imageBannerTop
            anchors.right: parent.right
            anchors.rightMargin: root.height * 0.03
            anchors.top: parent.top
            anchors.topMargin: anchors.rightMargin
            source: imageMap.startUpLogo
            width: root.width * 0.386
            height: width * 0.35
            sourceSize.height: sourceSize.height
            sourceSize.width: sourceSize.width
        }

        Image {
            id: imageBannerMain
            anchors.top: imageBannerTop.bottom
            anchors.topMargin: root.height * 0.07
            width: root.width * 0.63
            height: width * 0.09
            anchors.horizontalCenter: parent.horizontalCenter
            source: imageMap.startUpBanner
            sourceSize.height: height
            sourceSize.width: width
        }

        Text {
            id: textBanner
            x: imageBannerMain.x
            anchors.top: imageBannerMain.bottom
            anchors.topMargin: root.height * 0.013
            width: imageBannerMain.width
            color: colorMap.text01
            text: translate("T_CTInjectionSystem")
            font.pixelSize: root.height * 0.045
            font.family: fontRobotoBold.name
            horizontalAlignment: Text.AlignRight
            wrapMode: Text.Wrap
            fontSizeMode: Text.Fit
            minimumPixelSize: font.pixelSize * 0.7
        }

        Text {
            id: textInfo
            anchors.left: parent.left
            anchors.leftMargin: parent.width * 0.065
            anchors.right: parent.right
            anchors.rightMargin: parent.width * 0.03
            anchors.bottom: labelNotForHumanUse.top
            anchors.bottomMargin: parent.height * 0.03
            width: parent.width
            color: colorMap.text01
            font.pixelSize: parent.height * 0.03
            font.family: fontRobotoLight.name
            wrapMode: Text.Wrap
            text: getTextInfo()
        }

        Row {
            id: labelNotForHumanUse
            visible: false
            x: textInfo.x
            anchors.bottom: parent.bottom
            spacing: parent.width * 0.01
            height: parent.height * 0.05

            WarningIcon {
                height: parent.height
                width: height
                anchors.verticalCenter: parent.verticalCenter
            }

            Text {
                text: translate("T_NotForHumanUse")
                font.family: fontRobotoBold.name
                font.pixelSize: parent.height * 0.85
                verticalAlignment: Text.AlignVCenter
                color: colorMap.text01
            }
        }
    }

    Rectangle {
        id: warningsRect
        anchors.left: rectMain.right
        anchors.right: parent.right
        width: root.width * 0.3
        height: root.height - actionBarHeight
        color: colorMap.startupWarningPanel
        clip: true

        Image {
            id: iconWarning1
            anchors.left: parent.left
            anchors.leftMargin: parent.width * 0.11
            anchors.top: parent.top
            anchors.topMargin: parent.height * 0.04
            width: height * 1.1
            height: parent.height * 0.05
            source: imageMap.startUpWarning
            sourceSize.height: height
            sourceSize.width: width
        }

        Image {
            id: iconWarning2
            anchors.right: parent.right
            anchors.rightMargin: iconWarning1.anchors.leftMargin
            y: iconWarning1.y
            width: iconWarning1.width
            height: iconWarning1.height
            source: imageMap.startUpWarning
            sourceSize.height: height
            sourceSize.width: width
        }

        Text {
            clip: true
            anchors.left: iconWarning1.right
            anchors.leftMargin: parent.width * 0.01
            anchors.right: iconWarning2.left
            anchors.rightMargin: parent.width * 0.01
            y: iconWarning1.y
            height: iconWarning1.height
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: parent.height * 0.05
            font.family: fontRobotoBold.name
            text: translate("T_Warnings")
            color: colorMap.text01
            fontSizeMode: Text.Fit
            minimumPixelSize: font.pixelSize * 0.6
        }

        Item {
            anchors.top: iconWarning1.bottom
            anchors.topMargin: parent.height * 0.04
            width: parent.width * 0.92
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            clip: true

            Flickable {
                id: flickWarnings
                anchors.fill: parent
                flickableDirection: Flickable.VerticalFlick
                contentHeight: textWarnings.height
                contentWidth: width

                Text {
                    id: textWarnings
                    color: colorMap.text01
                    font.pixelSize: root.height * 0.02
                    font.family: fontRobotoLight.name
                    width: parent.width
                    height: contentHeight
                    wrapMode: Text.Wrap
                    text: getTextWarnings()
                }
            }

            ScrollBar {
                flickable: flickWarnings
            }

            ListFade {
                flickable: flickWarnings
                fadeColor: warningsRect.color
            }
        }
    }

    Item {
        y: parent.height - height
        height: actionBarHeight
        width: parent.width

        GenericIconButton {
            id: btnService
            anchors.verticalCenter: parent.verticalCenter
            width: parent.width * 0.07
            height: parent.height * 0.6
            x: -radius
            iconText: "\uf0ad"
            color: colorMap.keypadButton
            onBtnClicked: {
                if (servicePasscode === haspKeyEnforcementServiceKey)
                {
                    // Raise alert without displaying keypad when hasp key is being enforced
                    var data = "Service Passcode is disabled, HASP key required.";
                    dsAlert.slotActivateAlert("HaspKeyEnforcement", data);
                }
                else
                {
                    popupManager.popupEnterPassCode.titleText = "T_EnterServiceAccessCode";
                    popupManager.popupEnterPassCode.open();
                    widgetInputPad.signalValueChanged.disconnect(slotInputPadValChanged);
                    widgetInputPad.signalClosed.disconnect(slotInputPadClosed);
                    widgetInputPad.blurBackgroundVisible = true;
                    widgetInputPad.openTextPad(undefined, popupManager.popupEnterPassCode.textWidget, 0, 10);
                    widgetInputPad.signalValueChanged.connect(slotInputPadValChanged);
                    widgetInputPad.signalClosed.connect(slotInputPadClosed);
                }
            }
        }

        GenericButton {
            id: btnStart
            anchors.verticalCenter: parent.verticalCenter
            width: parent.width * 0.2
            height: parent.height * 0.6
            x: parent.width - width + radius
            color: colorMap.actionButtonBackground

            Text {
                x: parent.width * 0.02
                width: parent.width * 0.8
                height: parent.height
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.family: fontRobotoBold.name
                font.pixelSize: parent.height * 0.27
                color: colorMap.actionButtonText
                text: translate("T_Continue")
            }

            Text {
                x: parent.width * 0.82
                height: parent.height
                verticalAlignment: Text.AlignVCenter
                color: colorMap.actionButtonText
                font.family: fontIcon.name
                font.pixelSize:  height * 0.3
                text: "\ue908"
            }

            pressedSoundCallback: function(){ soundPlayer.playNext(); }
            onBtnClicked: {
                logDebug("Startup: Exit()");
                dsSystem.slotStartupScreenExit();
                appMain.setScreenState("Home");
            }
        }
    }

    onActiveAlertsChanged: {
        if (!visible)
        {
            return;
        }

        reloadNotForHumanUseLabel();
    }

    Component.onCompleted:
    {
        appMain.screenStateChanged.connect(slotScreenStateChanged);
        slotScreenStateChanged();
    }

    function slotScreenStateChanged()
    {
        visible = (appMain.screenState === "Startup");
        reload();
    }

    function reload()
    {
        if (!visible)
        {
            return;
        }

        reloadNotForHumanUseLabel();
        btnStart.enabled = true;
    }

    function reloadNotForHumanUseLabel()
    {
        labelNotForHumanUse.visible = false;

        for (var alertIdx = 0; alertIdx < activeAlerts.length; alertIdx++)
        {
            if ( (activeAlerts[alertIdx].CodeName === "NotForHumanUseSRU") ||
                 (activeAlerts[alertIdx].CodeName === "NotForHumanUseCRU") )
            {
                labelNotForHumanUse.visible = true;
                break;
            }
        }
    }

    function getTextInfo()
    {
        var textInfoStr = "";

        if (hcuBuildType == "PROD")
        {
            textInfoStr += "<< PRODUCTION BUILD >>\n\n";
        }
        else
        {
            textInfoStr += "\n\n";
        }

        var versionText = hcuVersion;
        if (hcuBuildType != "REL")
        {
            versionText += " " + hcuBuildType;
        }
        textInfoStr += infoTextCompany + "\n" +
                       translate("T_ProductSoftware") + translate("T_:") + " " + (productSoftwareVersion == "" ? "-" : productSoftwareVersion) + "\n" +
                       translate("T_SRUModelNo") + translate("T_:") + " " + modelNumber + "\n" +
                       translate("T_SRUSerialNo") + translate("T_:") + " " + serialNumber + "\n" +
                       translate("T_SRUSoftwareVersion") + translate("T_:") + " " + versionText + "\n" +
                       translate("T_ServiceContact") + translate("T_:") + " " + (serviceContactTelephone == "" ? "--" : serviceContactTelephone) + "\n" +
                       translate("T_PatentInformation");

        return textInfoStr;
    }

    function getTextWarnings()
    {
        var textWarningsStr = "";
        textWarningsStr += "<b>" + translate("T_Stellinity2_StartupWarning_Training_Direction") + "</b><p><p>" +
                           "<b>" + translate("T_Stellinity2_StartupWarning_Aseptic_Direction") + "</b><p>" +
                           translate("T_Stellinity2_StartupWarning_Aseptic_Hazard") + "<p><p>" +
                           "<b>" + translate("T_Stellinity2_StartupWarning_SystemDaySet_Direction") + "</b><p>" +
                           translate("T_Stellinity2_StartupWarning_SystemDaySet_Hazard") + "<p><p>" +
                           "<b>" + translate("T_Stellinity2_StartupWarning_Prime_Direction") + "</b><p>" +
                           translate("T_Stellinity2_StartupWarning_Prime_Hazard") + "<p><p>" +
                           "<b>" + translate("T_Stellinity2_StartupWarning_PatientLine_Direction") + "</b><p>" +
                           translate("T_Stellinity2_StartupWarning_PatientLine_Hazard") + "<p><p>" +
                           "<b>" + translate("T_Stellinity2_StartupWarning_OpsManual_Direction") + "</b><p><p>" +
                           "<b>" + translate("T_Stellinity2_StartupWarning_ContrastMedia_Direction") + "</b>" +
                           "<p><p>";

        return textWarningsStr;
    }

    function slotInputPadValChanged(newValue)
    {
        popupManager.popupEnterPassCode.setValue(newValue);
    }

    function slotInputPadClosed(modified)
    {
        widgetInputPad.signalValueChanged.disconnect(slotInputPadValChanged);
        widgetInputPad.signalClosed.disconnect(slotInputPadClosed);

        if (modified)
        {
            var codeEnteredOk = false;

            // Requires service passCode
            if (widgetInputPad.currentValue === servicePasscode)
            {
                codeEnteredOk = true;
                dsSystem.slotServiceModeActive(visible);
                appMain.setScreenState("Admin-Service-Select");
            }

            if (!codeEnteredOk)
            {
                logError("Bad Service passcode is entered");
                soundPlayer.playError();
            }
        }

        popupManager.popupEnterPassCode.textWidget.text = "";
        popupManager.popupEnterPassCode.close();
    }
}
