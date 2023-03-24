import QtQuick 2.12

Item {
    // Data
    property var calibrationInfo
    property string modelNumber: ""
    property string serialNumber: ""
    property string baseBoardType: "NoBattery"

    property var configTable

    // Function from QML to CPP
    function slotConfigChanged(configItem)  { return dsHardwareInfoCpp.slotConfigChanged(configItem); }
}
