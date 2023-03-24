import QtQuick 2.12

Item {
    // Data
    property int screenW: 1920
    property int screenH: 1200
    property int screenX: 0
    property int screenY: 0
    property double audioLevelNormal: 0
    property double audioLevelInjection: 0
    property double audioLevelNotification: 0
    property double audioLevelSudsPrimed: 0
    property bool audioKeyClicksEnabled: false
    property int currentUtcOffsetMinutes: 0
    property string uiTheme: ""
    property string cruLinkType: ""
    property var pressureCfgOptions
    property var fluidOptions
    property var cfgNetworkConnectionType
    property var configTable
    property int soundVolumeMax: 5
    property int soundVolumeMin: 0
    property string lastPMPerformedDateString: ""

    // Function from QML to CPP
    function slotUiThemeChanged(uiTheme) { return dsCfgLocalCpp.slotUiThemeChanged(uiTheme); }
    function slotConfigChanged(configItem) { return dsCfgLocalCpp.slotConfigChanged(configItem); }
    function slotFluidOptionsChanged(fluidOptions) { return dsCfgLocalCpp.slotFluidOptionsChanged(fluidOptions); }
    function slotHeaterActive(active)  { return dsCfgLocalCpp.slotHeaterActive(active); }
    function slotSetLastPMReminderAtToNow() { return dsCfgLocalCpp.slotSetLastPMReminderAtToNow(); }
    function slotSetLastPMPerformedAtToNow() { return dsCfgLocalCpp.slotSetLastPMPerformedAtToNow(); }
    function slotLoadSampleFluidOptions() { return dsCfgLocalCpp.slotLoadSampleFluidOptions(); }
    function slotLoadSampleInjectionPlans() { return dsCfgLocalCpp.slotLoadSampleInjectionPlans(); }
}
