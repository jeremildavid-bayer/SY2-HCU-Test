import QtQuick 2.12

Row {
    DeviceManagerSyringeIcon {
        height: parent.height
        syringeColor: colorMap.saline
        syringeVolume: syringeVolume1
        isAlertActive: activeAlertsSyringe1.length > 0
        fluidSourceSyringe: fluidSourceSyringe1
        autoEmptyEnabled: dsCfgGlobal.autoEmptySalineEnabled
    }

    DeviceManagerSyringeIcon {
        height: parent.height
        syringeColor: colorMap.contrast1
        syringeVolume: syringeVolume2
        isAlertActive: activeAlertsSyringe2.length > 0
        fluidSourceSyringe: fluidSourceSyringe2
        autoEmptyEnabled: dsCfgGlobal.autoEmptyContrast1Enabled
    }

    DeviceManagerSyringeIcon {
        height: parent.height
        syringeIndex: 2
        syringeColor: fluidC2Color
        syringeVolume: syringeVolume3
        isAlertActive: activeAlertsSyringe3.length > 0
        fluidSourceSyringe: fluidSourceSyringe3
        autoEmptyEnabled: dsCfgGlobal.autoEmptyContrast2Enabled
    }
}
