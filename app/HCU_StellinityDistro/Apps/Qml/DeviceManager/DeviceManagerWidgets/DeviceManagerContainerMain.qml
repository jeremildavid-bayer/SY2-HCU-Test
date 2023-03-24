import QtQuick 2.12
import "../../Widgets"
import "../../Util.js" as Util

Item {
    property int alertIconWidth: width * 0.065

    Row {
        id: bottleGroup
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.leftMargin: parent.width * 0.02
        width: parent.width * 0.82
        height: parent.height * 0.18
        spacing: parent.width * 0.034

        DeviceManagerBottleMain {
            id: bottleBtn1
            height: parent.height
            width: (parent.width - (parent.spacing * 2)) / 3
            panelName: "BOTTLE1"
            isAlertActive: activeAlertsBottle1.length > 0
            fluidSourceBottle: fluidSourceBottle1
            bottleColor: colorMap.saline
            titleIconText: "\ue930"
            syringeIndex: 0
        }

        DeviceManagerBottleMain {
            id: bottleBtn2
            height: bottleBtn1.height
            width: bottleBtn1.width
            panelName: "BOTTLE2"
            isAlertActive: activeAlertsBottle2.length > 0
            fluidSourceBottle: fluidSourceBottle2
            bottleColor: colorMap.contrast1
            titleIconText: "\ue92f"
            syringeIndex: 1
        }

        DeviceManagerBottleMain {
            id: bottleBtn3
            height: bottleBtn1.height
            width: bottleBtn1.width
            panelName: "BOTTLE3"
            isAlertActive: activeAlertsBottle3.length > 0
            fluidSourceBottle: fluidSourceBottle3
            bottleColor: fluidC2Color
            titleIconText: "\ue92f"
            syringeIndex: 2
        }
    }

    DeviceManagerMudsLineMain {
        id: mudsLine
        anchors.left: bottleGroup.left
        anchors.top: bottleGroup.bottom
        anchors.topMargin: parent.height * 0.015
        anchors.bottom: btnSuds.top
        anchors.bottomMargin: parent.height * 0.015
        anchors.right: parent.right
    }

    DeviceManagerMudsMain {
        id: btnMuds
        anchors.top: mudsLine.top
        anchors.topMargin: mudsLine.height * 0.08
        anchors.bottom: mudsLine.bottom
        anchors.bottomMargin: mudsLine.height * 0.06
        width: bottleGroup.width * 1.05
        isAlertActive: activeAlertsMuds.length > 0
        radius: buttonRadius * 2
    }

    DeviceManagerSudsMain {
        id: btnSuds
        width: parent.width * 0.22
        height: parent.height * 0.18
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: btnMuds.horizontalCenter
        isAlertActive: activeAlertsSuds.length > 0
    }

    DeviceManagerWCMain {
        id: btnWasteBin
        width: parent.width * 0.22
        height: parent.height * 0.18
        anchors.bottom: parent.bottom
        isAlertActive: activeAlertsWasteContainer.length > 0
    }

}
