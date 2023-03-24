import QtQuick.Window 2.12
import QtQuick 2.12
import QtQuick.Controls 2.12
import "Util.js" as Util

import "TitleBar"
import "Startup"
import "Home"
import "Shutdown"
import "ExamManager"
import "Admin"
import "McuSimulator"
import "DeviceManager"
import "Widgets"
import "PopupManager"
import "InjectionElapsedTime"
import "Widgets/InputPad"
import "Widgets/Keyboard"
import "Widgets/Popup"
import "Themes"
import "ContinuousExamsTest"

// debug tools
import "DebugTools"

Window {
    // UI Properties
    property int readOnlyBoxBorderWidth: Util.getPixelH(4)
    property int buttonRadius: Util.getPixelH(10)
    property int buttonSelectedBorderWidth: Util.getPixelH(6)
    property int titleBarHeight: Util.getPixelV(120)
    property int elapsedTimeFrameHeight: titleBarHeight * 0.95
    property int actionBarHeight: Util.getPixelV(255)
    property QtObject colorMap: themes.colorMapPurity
    property QtObject imageMap: themes.imageMap
    property string screenState: "OFF"
    property string screenStatePrev: "OFF"
    property string screenMode: dsCapabilities.screenMode
    property string pathFonts: dsCapabilities.pathFonts
    property bool interactive: true
    property bool homeButtonEnabled: !blurBackground.isOpen()
    property bool shortcutEnabled: false
    property string hcuBuildType: dsSystem.hcuBuildType
    property string cultureCode: dsCfgGlobal.cultureCode
    property var locale: Qt.locale(cultureCode)
    property string dateTimeModifiedAt: dsCapabilities.dateTimeModifiedAt
    property int currentUtcOffsetMinutes: dsCfgLocal.currentUtcOffsetMinutes

    //------------------------------------------
    // Signals
    signal signalUpdateDateTime()
    signal signalUIStarted()

    //------------------------------------------
    // Set Default Theme
    Themes { id: themes }

    //------------------------------------------
    // Set Sound Player
    SoundPlayer {
        id: soundPlayer
    }

    //------------------------------------------
    // Fonts
    FontLoader {
        id: fontIcon
        source: pathFonts + "/Stellinity2-UI.ttf"
    }

    FontLoader {
        id: fontDeviceIcon
        source: pathFonts + "/Stellinity2-DM.ttf"
    }

    FontLoader {
        id: fontRobotoLight
        source: pathFonts + "/Roboto-Light.ttf"
    }

    FontLoader {
        id: fontRobotoBold
        source: pathFonts + "/Roboto-Bold.ttf"
    }

    FontLoader {
        id: fontRobotoMedium
        source: pathFonts + "/Roboto-Medium.ttf"
    }

    FontLoader {
        id: fontAwesome
        source: pathFonts + "/FontAwesome-Webfont.ttf"
    }

    //------------------------------------------
    // Data & Action Services

    DS_Device       { id: dsDevice;         objectName: "dsDevice"; }
    DS_Exam         { id: dsExam;           objectName: "dsExam"; }
    DS_Alert        { id: dsAlert;          objectName: "dsAlert"; }
    DS_Cru          { id: dsCru;            objectName: "dsCru"; }
    DS_ImrServer    { id: dsImrServer;      objectName: "dsImrServer"; }
    DS_Mwl          { id: dsMwl;            objectName: "dsMwl"; }
    DS_Mcu          { id: dsMcu;            objectName: "dsMcu"; }
    DS_McuSim       { id: dsMcuSim;         objectName: "dsMcuSim"; }
    DS_System       { id: dsSystem;         objectName: "dsSystem"; }
    DS_Upgrade      { id: dsUpgrade;        objectName: "dsUpgrade"; }
    DS_Test         { id: dsTest;           objectName: "dsTest"; }
    DS_HardwareInfo { id: dsHardwareInfo;   objectName: "dsHardwareInfo"; }
    DS_CfgLocal     { id: dsCfgLocal;       objectName: "dsCfgLocal"; }
    DS_CfgGlobal    { id: dsCfgGlobal;      objectName: "dsCfgGlobal"; }
    DS_Capabilities { id: dsCapabilities;   objectName: "dsCapabilities"; }
    DS_Workflow     { id: dsWorkflow;       objectName: "dsWorkflow"; }
    DS_DevTools     { id: dsDevTools;       objectName: "dsDevTools"; }

    //------------------------------------------
    // App Main Properties
    id: appMain
    x: dsCfgLocal.screenX
    y: dsCfgLocal.screenY

    width: dsCfgLocal.screenW
    height: dsCfgLocal.screenH

    title: "HCU_StellinityDistro"
    visibility: Window.Hidden

    flags: {
        logInfo("AppManager: Set screen mode: " + screenMode);
        if (screenMode === "Splash")
        {
            return (Qt.SplashScreen | Qt.WindowStaysOnTopHint | Qt.WA_NoSystemBackground);
        }
        else if (screenMode === "X11Bypass")
        {
            return Qt.X11BypassWindowManagerHint;
        }
        return (Qt.Dialog | Qt.WindowStaysOnBottomHint);
    }

    Shortcut {
        sequence: "Alt+4"
        enabled: shortcutEnabled
        onActivated: {
            logDebug("AppManager: Application Exit requested");
            dsApp.slotExit();
        }
    }

    Shortcut {
        sequence: "Ctrl+F1"
        enabled: shortcutEnabled
        onActivated: {
            logDebug("AppManager: Screenshot requested");
            dsApp.slotScreenShot();
        }
    }

    Shortcut {
        sequence: "Ctrl+F2"
        enabled: shortcutEnabled
        onActivated: {
            logDebug("AppManager: Multiple Screenshots requested");
            dsApp.slotMultipleScreenShots();
        }
    }

    MouseArea {
        // MouseArea to get the focus for the application (so Alt+4 can work)
        anchors.fill: parent
        onClicked: {
            appMain.requestActivate();
        }
    }

    Item {
        id: appMainWindow
        width: parent.width
        height: parent.height

        Item {
            id: appMainView
            width: parent.width
            height: parent.height

            //------------------------------------------
            // MAIN VIEW APPS
            Home {
                y: titleBar.y + titleBar.height
                width: dsCfgLocal.screenW
                height: dsCfgLocal.screenH - y
            }

            Shutdown {
                y: titleBar.y + titleBar.height
                width: dsCfgLocal.screenW
                height: dsCfgLocal.screenH - y
            }

            Admin {
                y: titleBar.y + titleBar.height
                width: dsCfgLocal.screenW
                height: dsCfgLocal.screenH - y
            }

            DeviceManager {
                id: deviceManager
                y: titleBar.y + titleBar.height
                width: dsCfgLocal.screenW
                height: dsCfgLocal.screenH - y
            }

            ExamManager {
                id: examManager
                y: titleBar.y + titleBar.height
                width: dsCfgLocal.screenW
                height: dsCfgLocal.screenH - y
            }
        }

        Item {
            id: frameActionBar
            anchors.bottom: parent.bottom
            width: parent.width
            height: actionBarHeight
        }

    }

    BlurBackground {
        id: blurBackground
        anchors.fill: parent
    }

    InputPad {
        id: widgetInputPad
    }

    Keyboard {
        id: widgetKeyboard
    }

    InjectionElapsedTime {
        id: injectionElapsedTime
        x: parent.width - width + radius
        y: titleBar.height - (height * 0.1)
        width: parent.width * 0.14
        containerHeight: dsCfgLocal.screenH * 0.08
    }

    DeviceManagerIcon {
        id: deviceManagerIcon
        visible: false
    }

    TitleBar {
        id: titleBar
        height: titleBarHeight
    }

    McuSimulator {
        id: appMcuSimulator
    }

    Startup {
        id: startupPage
    }

    BlurBackground {
        id: startupPageBlurBackground
        anchors.fill: parent
        backgroundWidget: startupPage
    }

    BlurBackground {
        id: entireBlurBackground
        anchors.fill: parent
    }

    ContinuousExamsTest {
    }

    //------------------------------------------
    // Popups
    PopupManager {
        id: popupManager
    }

    //------------------------------------------
    // Prevent Unwanted User's Interaction
    Rectangle {
        anchors.fill: parent
        visible: !interactive
        color: "transparent"
        LoadingGif {
            visible: !interactive
        }
        MouseArea {
            anchors.fill: parent
            onClicked: {
                appMain.requestActivate();
            }
        }
    }

    //------------------------------------------
    // Date Time Monitor
    Timer {
        id: dateTimeUpdateTimer
        interval: 10000
        repeat: true
        triggeredOnStart: true
        onTriggered: {
            signalUpdateDateTime();
        }
    }

    Component {
        id: delayCallerComponent
        Timer {}
    }

    onDateTimeModifiedAtChanged: {
        dateTimeUpdateTimer.stop();
        dateTimeUpdateTimer.start();
    }

    onCurrentUtcOffsetMinutesChanged: {
        dateTimeUpdateTimer.stop();
        dateTimeUpdateTimer.start();
    }

    //------------------------------------------
    // Debug monitor
    Debug_HCUMonitor {
        Component.onCompleted: {
            parent = appMainWindow
        }
    }

    // Unprimed Suds Warning
    UnprimedSudsWarning {
        id: unprimedSudsWarning
        anchors.fill: parent
    }

    //------------------------------------------
    // AppManager Start
    Component.onCompleted: {
        appMain.requestActivate();
        dateTimeUpdateTimer.stop();
        dateTimeUpdateTimer.start();
    }

    //------------------------------------------
    // Public Util Functions
    function timerSingleShot(intervalMs, cb) {
        var dealyCaller = delayCallerComponent.createObject(null, { "interval": intervalMs } );
        dealyCaller.triggered.connect( function() {
            cb();
            dealyCaller.destroy();
        });
        dealyCaller.start();
    }

    //------------------------------------------
    // Public Interface Functions - Translation
    function translate(inStr)
    {
        return dsTranslator.translate(inStr, cultureCode);
    }

    function getLocale()
    {
        var curLocale = appMain.locale;
        return curLocale;
    }

    function localeDecimalPoint()
    {
        return getLocale().decimalPoint;
    }

    function localeFloatStrToFloat(floatStr)
    {
        var curLocale = getLocale();
        var nonLocaleFloatStr = floatStr.replace(curLocale.decimalPoint, ".");
        return parseFloat(nonLocaleFloatStr);
    }

    function localeToFloatStr(inFloat, digits)
    {
        var curLocale = getLocale();
        var ret;
        inFloat = parseFloat(inFloat);
        if (digits === 0)
        {
            // Round to whole number
            ret = inFloat.toFixed(0);
        }
        else
        {
            // Remove thousand separator from local conversion
            var wholeNumber = Math.floor(inFloat);
            var fraction = inFloat - wholeNumber;
            var fractionStr = Number(fraction).toLocaleString(curLocale, "f", digits);
            if (parseFloat(fractionStr) >= 1)
            {
                // fraction is rounded to 1 (e.g. 9.9999..)
                wholeNumber += 1;
                fractionStr = curLocale.decimalPoint + "0";
            }
            else
            {
                // E.g. fraction=0.2, fractionStr=".2"
                fractionStr = fractionStr.substring(1, fractionStr.length);
            }

            ret = wholeNumber.toString() + fractionStr;
        }
        return ret;
    }

    function localeFromFloatStr(inStr)
    {
        return Number.fromLocaleString(getLocale(), inStr);
    }

    function setScreenState(newScreenState)
    {
        if (screenState !== newScreenState)
        {
            logDebug("AppManager: ScreenState = " + screenState + " -> " + newScreenState);
            screenStatePrev = screenState;
            screenState = newScreenState;
        }
    }

    function setInteractiveState(newState, reason)
    {
        logDebug("AppManager: Interactive = " + newState + ", Reason=" + reason);
        interactive = newState;
    }

    function setShortcutEnabled(serviceUnlock)
    {
        if ( (hcuBuildType === "REL") || (hcuBuildType === "VNV") )
        {
            shortcutEnabled = serviceUnlock;
        }
        else
            shortcutEnabled = true;

        logDebug("AppManager: ShortcutEnabled = " + shortcutEnabled);
    }

    //------------------------------------------
    // Public Interface Functions - Logs
    function logDebug(msg)
    {
        console.log("DEBUG: " + msg);
    }

    function logInfo(msg)
    {
        console.info("INFO : " + msg);
    }

    function logWarning(msg)
    {
        console.warn("WARN : " + msg);
    }

    function logError(msg)
    {
        console.error("ERROR: " + msg);
    }

    //------------------------------------------
    // INITIALISATION
    function slotAppStarted()
    {
        //appMain.visible = true;
	//if the screen mode is splash, then use the full screen (normal HCU operation on the field). 
	//otherwise use the windowed mode for developement.
        if (screenMode === "Splash")
            appMain.visibility = Window.FullScreen;
        else
            appMain.visibility = Window.Windowed;
        appMain.setScreenState("Startup");
        appMcuSimulator.toggleWidgetsVisible();
        logDebug("AppManager: UI Started");
        signalUIStarted();
        setShortcutEnabled(false);
    }
}
