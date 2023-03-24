import QtQuick 2.12

Item {
    // Data
    property string hcuVersion: ""
    property string hcuBuildType: ""
    property string statePath: "OnReachable"
    property string lastStatePath: "OffUnreachable"
    property bool usbStickInserted: false
    property int webAppHostPort: 0
    property var networkSettingParams
    property double cpuTemperatureCelcius: 0
    property int hcuFanSpeed: -1


    // Signal from QML to CPP
    function slotStartupScreenExit() { return dsSystemCpp.slotStartupScreenExit(); }
    function slotServiceModeActive(serviceModeActive) { return dsSystemCpp.slotServiceModeActive(serviceModeActive); }
    function slotShutdown(isShutdownType) { return dsSystemCpp.slotShutdown(isShutdownType); }
    function slotFactoryReset() { return dsSystemCpp.slotFactoryReset(); }
    function slotSaveUserData() { return dsSystemCpp.slotSaveUserData(); }

    // Function from CPP to QML
    function slotShutdownRestartIgnored()
    {
        popupManager.popupShutdownRestartIgnored.open();
    }
}
