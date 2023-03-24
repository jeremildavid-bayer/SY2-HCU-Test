import QtQuick 2.12
import "../Widgets"
import "../Util.js" as Util

Rectangle {
    property string hcuBuildType: dsSystem.hcuBuildType
    property string statePath: dsSystem.statePath
    property bool mcuSimulatorEnabled: dsMcuSim.simulatorEnabled
    property string suiteName: dsMwl.suiteName
    property string patientName: dsMwl.patientName
    property string studyDescription: dsMwl.studyDescription
    property var upgradeDigest: dsUpgrade.upgradeDigest
    property var activeAlerts: dsAlert.activeAlerts
    property bool suiteNameAssigned: false

    width: dsCfgLocal.screenW
    height: titleBarHeight
    visible: true
    color: colorMap.homeBackground

    GenericIconButton {
        id: btnHome
        color: colorMap.titleBarBackground
        height: parent.height
        width: dsCfgLocal.screenW * 0.07
        visible: appMain.screenState !== "Home"
        iconText: "\uf015"
        iconFontPixelSize: parent.height * 0.7
        iconColor: colorMap.text01
        disabledColor: "transparent"
        radius: 0
        enabled: getHomeButtonEnableState()
        onBtnClicked: {
            appMain.setScreenState("Home");
        }
    }

    Rectangle {
        id: btnHomeSpacing
        anchors.left: btnHome.right
        width: parent.width * 0.004
        height: parent.height
        color: (appMain.screenState === "Home") ? "transparent" : colorMap.mainBackground
    }

    Item {
        id: frameSystemAlertsBtn
        anchors.left: btnHomeSpacing.right
        height: parent.height
    }

    Item {
        id: frameSystemAlertsPanel
        width: dsCfgLocal.screenW * 0.35
        y: titleBarHeight + (dsCfgLocal.screenH * 0.005)
        height: dsCfgLocal.screenH - y
    }

    TitleBarSystemAlertsPanel {}

    Rectangle {
        id: btnSystemAlertsSpacing
        anchors.left: frameSystemAlertsBtn.right
        width: (frameSystemAlertsBtn.width > 0) ? parent.width * 0.004 : 0
        height: parent.height
        color: colorMap.mainBackground
    }

    GenericIconButton {
        id: btnSimulatorView
        color: colorMap.mainBackground
        opacity: 0.4
        height: parent.height
        visible: mcuSimulatorEnabled
        width: parent.width * 0.025
        iconText: "\uf1b6"
        iconFontPixelSize: parent.height * 0.2
        iconColor: colorMap.text02
        radius: 0
        onBtnClicked: {
            appMcuSimulator.toggleWidgetsVisible();
        }
    }

    Text {
        id: productionTitle
        visible: (hcuBuildType === "PROD")
        width: parent.width * 0.4
        height: parent.height * 0.3
        anchors.horizontalCenter: parent.horizontalCenter
        text: "-------------------- PRODUCTION MODE --------------------"
        font.pixelSize: height
        font.family: fontRobotoBold.name
        color: colorMap.actionButtonBackground
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    WarningIcon {
        id: iconSuiteNameError
        anchors.left: btnSystemAlertsSpacing.right
        anchors.leftMargin: parent.width * 0.02
        width: height
        height: suiteNameAssigned ? 0 : parent.height * 0.4
        anchors.verticalCenter: parent.verticalCenter
        visible: isTextSuitNameVisible() && (!suiteNameAssigned)
    }

    Text {
        id: textSuiteName
        visible: isTextSuitNameVisible()
        anchors.top: parent.top
        anchors.topMargin: parent.height * 0.1
        anchors.bottom: parent.bottom
        anchors.bottomMargin: parent.height * 0.1
        anchors.left: iconSuiteNameError.right
        anchors.leftMargin: parent.width * 0.01
        anchors.right: statusIconFrame.left
        anchors.rightMargin: parent.width * 0.015
        color: colorMap.text01
        font.pixelSize: parent.height * 0.35
        font.family: fontRobotoBold.name
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        text: suiteNameAssigned ? suiteName : translate("T_SuiteNameNotAssigned_Name")
    }

    Item {
        id: adminPathFrame
        visible: isAdminPathVisible()
        anchors.top: parent.top
        anchors.topMargin: parent.height * 0.1
        anchors.bottom: parent.bottom
        anchors.bottomMargin: parent.height * 0.1
        anchors.left: btnSystemAlertsSpacing.right
        anchors.leftMargin: parent.width * 0.02
        anchors.right: statusIconFrame.left
        anchors.rightMargin: parent.width * 0.02
        clip: true

        Text {
            id: textAdminPathPrefix
            width: contentWidth
            height: parent.height
            color: colorMap.text01
            font.pixelSize: parent.height * 0.35
            font.family: fontRobotoBold.name
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            text: getAdminPathPrefix()
        }

        Text {
            id: textAdminPath
            anchors.left: textAdminPathPrefix.right
            anchors.leftMargin: parent.width * 0.01
            anchors.right: parent.right
            height: parent.height
            color: colorMap.text01
            font.pixelSize: parent.height * 0.35
            font.family: fontRobotoLight.name
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            text: getAdminPath()
            elide: Text.ElideRight
        }
    }

    Item {
        id: examInfoFrame
        visible: isExamInfoFrameVisible()
        anchors.top: parent.top
        anchors.topMargin: parent.height * 0.1
        anchors.bottom: parent.bottom
        anchors.bottomMargin: parent.height * 0.1
        anchors.left: btnSystemAlertsSpacing.right
        anchors.leftMargin: parent.width * 0.02
        anchors.right: statusIconFrame.left
        anchors.rightMargin: parent.width * 0.02

        Text {
            id: textPatientName
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.rightMargin: 400
            height: parent.height
            color: colorMap.text01
            font.pixelSize: parent.height * 0.5
            font.family: fontRobotoBold.name
            fontSizeMode: Text.Fit
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.Wrap
            text: patientName
        }

        Text {
            id: textStudyDescription
            height: parent.height
            anchors.left: textPatientName.right
            anchors.leftMargin: parent.width * 0.04
            anchors.right: parent.right
            color: colorMap.text01
            font.pixelSize: parent.height * 0.3
            font.family: fontRobotoMedium.name
            fontSizeMode: Text.Fit
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.Wrap
            text: studyDescription
        }
    }

    TitleBarStatusIcons {
        id: statusIconFrame
        height: parent.height
        anchors.right: parent.right
        anchors.rightMargin: dsCfgLocal.screenW * 0.015
        iconSpacing: dsCfgLocal.screenW * 0.005
        clip: true
    }

    TitleBarStatusIconInfoPanel {
        id: statusIconInfoPanel
        y: parent.height
        anchors.right: parent.right
    }

    onActiveAlertsChanged: {
        var curSuiteNameAssigned = true;

        for (var alertIdx = 0; alertIdx < activeAlerts.length; alertIdx++)
        {
            if (activeAlerts[alertIdx].CodeName === "SuiteNameNotAssigned")
            {
                curSuiteNameAssigned = false;
                break;
            }
        }
        suiteNameAssigned = curSuiteNameAssigned;
    }

    function addCustomWidget(customWidget)
    {
        customWidget.parent = this;
        customWidget.anchors.left = Qt.binding(function() { return btnSystemAlertsSpacing.right; });
        customWidget.height = height;
    }

    function getHomeButtonEnableState()
    {
        if (!appMain.homeButtonEnabled)
        {
            return false;
        }
        else if ( (statePath === "Ready/Armed") ||
                  (statePath === "Executing") ||
                  (statePath === "Busy/Finishing") ||
                  (statePath === "Busy/Holding") ||
                  (statePath === "Servicing") )
        {
            return false;
        }
        else if (upgradeDigest.State !== "Ready")
        {
            return false;
        }
        else if (appMain.screenState === "Startup")
        {
            return false;
        }

        return true;
    }

    function isTextSuitNameVisible()
    {
        return (appMain.screenState === "Home") ||
               (appMain.screenState.indexOf("DeviceManager-") >= 0);
    }

    function isExamInfoFrameVisible()
    {
        return (appMain.screenState.indexOf("ExamManager-") >= 0);
    }

    function isAdminPathVisible()
    {
        return (appMain.screenState.indexOf("Admin-") >= 0);
    }

    function getAdminPathPrefix()
    {
        if (appMain.screenState === "Admin-Select")
        {
            return translate("T_Admin");
        }
        else if (appMain.screenState.indexOf("Admin-Service-Settings-") === 0)
        {
            return translate("T_Admin") + " / " + translate("T_AdminTileDefinition_Service") + " / " + "Settings" + " / ";
        }
        else if (appMain.screenState.indexOf("Admin-Service-Tool-") === 0)
        {
            return translate("T_Admin") + " / " + translate("T_AdminTileDefinition_Service") + " / " + "Service Tool" + " / ";
        }
        else if (appMain.screenState.indexOf("Admin-Service-Capabilities-") === 0)
        {
            return translate("T_Admin") + " / " + translate("T_AdminTileDefinition_Service") + " / " + "Capabilities" + " / ";
        }
        else if (appMain.screenState.indexOf("Admin-Service-HardwareInfo-") === 0)
        {
            return translate("T_Admin") + " / " + translate("T_AdminTileDefinition_Service") + " / " + "Hardware Info" + " / ";
        }
        else if (appMain.screenState.indexOf("Admin-Settings-") === 0)
        {
            return translate("T_Admin") + " / " + translate("T_AdminTileDefinition_Settings") + " / ";
        }
        else if (appMain.screenState.indexOf("Admin-Service-") === 0)
        {
            return translate("T_Admin") + " / " + translate("T_AdminTileDefinition_Service") + " / ";
        }
        else if (appMain.screenState.indexOf("Admin-") === 0)
        {
            return translate("T_Admin") + " / ";
        }

        return "";
    }

    function getAdminPath()
    {
        var path = appMain.screenState;

        if (path === "Admin-Service-Select")
        {
            return translate("T_AdminTileDefinition_Service");
        }
        else if (path === "Admin-Service-Tool-PistonsStopcocks")
        {
            return "Pistons & Stopcocks";
        }
        else if (path === "Admin-Service-Tool-LedControl")
        {
            return "LED Control";
        }
        else if (path === "Admin-Service-Tool-SpecialActions")
        {
            return "Special Actions";
        }
        else if (path === "Admin-Service-Tool-Calibration")
        {
            return "Calibration";
        }
        else if (path === "Admin-Service-Tool-PressureCalibration")
        {
            return "Pressure Calibration";
        }
        else if (path === "Admin-Service-Tool-PistonCycleTest")
        {
            return "Piston Cycle Test";
        }
        else if (path === "Admin-Service-Tool-StopcockCycleTest")
        {
            return "Stopcock Cycle Test";
        }
        else if (path.indexOf("Admin-Service-Capabilities-") === 0)
        {
            path = path.replace("Admin-Service-Capabilities-", "");
            return path;
        }
        else if (path.indexOf("Admin-Service-HardwareInfo-") === 0)
        {
            path = path.replace("Admin-Service-HardwareInfo-", "");
            return path;
        }
        else if (path.indexOf("Admin-Service-Settings-") === 0)
        {
            if (path === "Admin-Service-Settings-DateTime")
            {
                return "Date & Time";
            }
            path = path.replace("Admin-Service-Settings-", "");
            return path;
        }
        else if (path.indexOf("Admin-Service-") === 0)
        {
            if (path === "Admin-Service-Tool")
            {
                return "Service Tool";
            }
            path = path.replace("Admin-Service-", "");
            return path;
        }
        else if (path.indexOf("Admin-Settings-") === 0)
        {
            path = path.replace("Admin-Settings-", "");
            return translate("T_SettingsMenuItem_" + path);
        }
        else if (path.indexOf("Admin-") === 0)
        {
            if (path === "Admin-Select")
            {
                return "";
            }
            path = path.replace("Admin-", "");
            return translate("T_AdminTileDefinition_" + path);
        }

        return "";
    }
}
