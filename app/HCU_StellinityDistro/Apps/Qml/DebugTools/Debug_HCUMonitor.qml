import QtQuick 2.12
import QtQml 2.15
import "../Util.js" as Util

Item {
    id: root
    visible: dsCapabilities.hcuMonitorEnabled
    anchors.left: parent.left
    anchors.bottom: parent.bottom
    property string temperatureUnit: dsCfgGlobal.temperatureUnit
    property string hcuCpuTempText: ""
    property string hcuCurrentText: ""

    property double cpuTemperatureCelcius
    Binding on cpuTemperatureCelcius {
        when: root.visible
        value: dsSystem.cpuTemperatureCelcius
        restoreMode: Binding.RestoreBinding
    }
    onCpuTemperatureCelciusChanged: {
        var unit = (temperatureUnit == "degreesF") ? "°F" : "°C";
        hcuCpuTempText = "[HCU CPU Temp]: " + Util.getTemperature(temperatureUnit, cpuTemperatureCelcius) + unit;
    }

    property var hwDigest: undefined
    Binding on hwDigest {
        when: root.visible
        value: dsMcu.hwDigest
        restoreMode: Binding.RestoreBinding
    }
    onHwDigestChanged: {
        if (hwDigest === undefined) {
            hcuCurrentText = "";
            return;
        }
        hcuCurrentText = "[Current]: HCU(" + hwDigest.HcuCurrent + "mA), AUX(" + hwDigest.AuxCurrent + "mA)";
    }

    Text {
        id: hcuDebugMonitorText
        anchors.left: root.left
        anchors.bottom: root.bottom
        anchors.bottomMargin: 5
        font.pixelSize: root.height * 0.4
        verticalAlignment: Text.AlignTop
        horizontalAlignment: Text.AlignLeft
        color: colorMap.text02
        text: root.hcuCpuTempText + " " + root.hcuCurrentText
    }
}
