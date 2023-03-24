import QtQuick 2.12

Item {
    // Data
    property string productSoftwareVersion: ""
    property string pressureUnit: "kpa"
    property string temperatureUnit: "degreesC"
    property string servicePasscode
    property string serviceContactTelephone
    property string accessCode
    property string quickNotes: "[]"
    property string cultureCode: ""
    property var accessProtected
    property var mandatoryFields
    property string dateFormat: "yyyy/MM/dd"
    property string timeFormat: "HH:mm"
    property var configTable
    property int mudsUseLifeLimitHours
    property bool tradeshowModeEnabled: false
    property bool extendedSUDSAvailable: false
    property var availableSUDSLengthOptions: []
    property bool autoEmptySalineEnabled: false
    property bool autoEmptyContrast1Enabled: false
    property bool autoEmptyContrast2Enabled: false
    property bool changeContrastEnabled: false

    // Function from QML to CPP
    function slotConfigChanged(configItem)  { return dsCfgGlobalCpp.slotConfigChanged(configItem); }
}
