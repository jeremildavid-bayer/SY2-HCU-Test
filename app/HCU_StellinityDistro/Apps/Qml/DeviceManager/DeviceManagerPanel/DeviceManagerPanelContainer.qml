import QtQuick 2.12

import "DeviceManagerPanelBottle"
import "DeviceManagerPanelMuds"
import "DeviceManagerPanelSuds"
import "DeviceManagerPanelWC"
import "../../PopupManager"
import "../../Widgets/Popup"

Item {
    DeviceManagerPanelMuds {}

    DeviceManagerPanelBottle {
        anchors.fill: parent
        panelName: "BOTTLE1"
        titleText: "T_SalineSupply"
        titleIconText: "\ue930"
        fluidColor: colorMap.saline
        fluidSourceBottle: fluidSourceBottle1
        fluidSourceSyringe: fluidSourceSyringe1
        panelActiveAlerts: activeAlertsBottle1
        fluidSelectItems: salineSelectItems
        syringeIndex: 0
        syringeVolume: syringeVolume1
    }

    DeviceManagerPanelBottle {
        panelName: "BOTTLE2"
        titleText: "T_ContrastSupply1"
        titleIconText: "\ue92f"
        fluidColor: colorMap.contrast1
        fluidSourceBottle: fluidSourceBottle2
        fluidSourceSyringe: fluidSourceSyringe2
        panelActiveAlerts: activeAlertsBottle2
        fluidSelectItems: contrastSelectItems
        syringeIndex: 1
        syringeVolume: syringeVolume2
    }

    DeviceManagerPanelBottle {
        panelName: "BOTTLE3"
        titleText: "T_ContrastSupply2"
        titleIconText: "\ue92f"
        fluidColor: fluidC2Color
        fluidSourceBottle: fluidSourceBottle3
        fluidSourceSyringe: fluidSourceSyringe3
        panelActiveAlerts: activeAlertsBottle3
        fluidSelectItems: contrastSelectItems
        syringeIndex: 2
        syringeVolume: syringeVolume3
    }

    DeviceManagerPanelWC {}
    DeviceManagerPanelSuds {}
}
