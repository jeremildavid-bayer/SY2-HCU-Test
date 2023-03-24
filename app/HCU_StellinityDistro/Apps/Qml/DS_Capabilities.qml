import QtQuick 2.12

Item {
    // Data
    property string pathResources:  "file:///home/user/Imaxeon/bin/resources"
    property string pathFonts: pathResources + "/Fonts"
    property string pathSound: pathResources + "/Sound"
    property string pathImages: pathResources + "/Images"

    property string ethernetInterface

    property string dateTimeModifiedAt
    property string screenMode: "Splash"
    property string debugMode: "off"
    property string keyboardStyle: "imax"
    property bool displayDeviceManagerDuringInjection: false
    property string modelNumber
    property int phaseCountMax
    property int stepCountMax
    property double flowRateMin
    property double flowRateMax
    property double volumeMin
    property double volumeMax
    property int delayMsMax
    property int delayMsMin
    property int fluidOptionContrastBrandLenMax
    property int fluidOptionContrastBarcodeLenMin
    property int fluidOptionContrastBarcodeLenMax
    property int fluidOptionContrastConcentrationMin
    property int fluidOptionContrastConcentrationMax
    property int fluidOptionContrastVolumeMin
    property int fluidOptionContrastVolumeMax
    property int fluidOptionContrastMaxUseLifeHourMin
    property int fluidOptionContrastMaxUseLifeHourMax
    property int fluidBottleLotBatchTextLenMin
    property int fluidBottleLotBatchTextLenMax
    property int injectionSkipPhaseEnableWaitingMs: 2000
    property string haspKeyEnforcementServiceKey: "0000"
    property var configTable

    property bool advanceModeDevModeEnabled: false
    property bool hcuMonitorEnabled: false

    property bool continuousExamsTestEnabled: false
    property int continuousExamsTestLimit: 0

    property int bmsMaxErrorLimitLow: 0
    property int bmsTemperatureLimitLow: 0
    property int bmsTemperatureLimitHigh: 0
    property int bmsCycleCountLimit: 0
    property int bmsStateOfHealthLowLimit: 0
    property int bmsShippingModeAirTargetChargedLevel: 0
    property int bmsShippingModeLandTargetChargedLevel: 0
    property int bmsShippingModeSeaTargetChargedLevel: 0
    property bool bmsEnableBatteryTestMode: false
    property int bmsAQDBatteryPackAMaxErrorTrigger: 0
    property int bmsAQDBatteryPackBMaxErrorTrigger: 0
    property int bmsAQDOtherBatteryChargeLimit: 0

    property int defaultButtonDebounceTimerMs: 500


    // Function from QML to CPP
    function slotConfigChanged(configItem)  { return dsCapabilitiesCpp.slotConfigChanged(configItem); }
}
