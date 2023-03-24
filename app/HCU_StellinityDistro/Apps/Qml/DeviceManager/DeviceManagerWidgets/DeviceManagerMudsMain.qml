import QtQuick 2.12
import "../../Widgets"

DeviceManagerGenericButton {
    panelName: "MUDS"
    isBusy: {
        if ( (fluidSourceMuds === undefined) ||
             (fluidSourceSyringe1 === undefined) ||
             (fluidSourceSyringe2 === undefined) ||
             (fluidSourceSyringe3 === undefined) )
        {
            return false;
        }

        if ( (fluidSourceMuds.IsBusy) ||
             (fluidSourceSyringe1.IsBusy) ||
             (fluidSourceSyringe2.IsBusy) ||
             (fluidSourceSyringe3.IsBusy) )
        {
            return true;
        }
        return false;
    }

    content: [
        Row {
            anchors.bottom: parent.bottom
            anchors.bottomMargin: parent.height * 0.03
            anchors.top: parent.top
            anchors.topMargin: parent.height * 0.03
            width: bottleGroup.width
            anchors.left: parent.left
            anchors.leftMargin: bottleGroup.anchors.leftMargin
            spacing: bottleGroup.spacing

            DeviceManagerSyringeMain {
                anchors.bottom: parent.bottom
                anchors.top: parent.top
                width: bottleBtn1.width
                syringeColor: colorMap.saline
                syringeVolume: syringeVolume1
                fluidSourceSyringe: fluidSourceSyringe1
                autoEmptyEnabled: dsCfgGlobal.autoEmptySalineEnabled
            }

            DeviceManagerSyringeMain {
                anchors.bottom: parent.bottom
                anchors.top: parent.top
                width: bottleBtn2.width
                syringeColor: colorMap.contrast1
                syringeVolume: syringeVolume2
                fluidSourceSyringe: fluidSourceSyringe2
                autoEmptyEnabled: dsCfgGlobal.autoEmptyContrast1Enabled
            }

            DeviceManagerSyringeMain {
                anchors.bottom: parent.bottom
                anchors.top: parent.top
                width: bottleBtn3.width
                syringeColor: sameContrasts ? colorMap.contrast1 : colorMap.contrast2
                syringeVolume: syringeVolume3
                fluidSourceSyringe: fluidSourceSyringe3
                autoEmptyEnabled: dsCfgGlobal.autoEmptyContrast2Enabled
            }
        }
    ]
}
