import QtQuick 2.12

import "AdminServiceSettings"
import "AdminServiceUpgrade"
import "AdminServiceHardwareInfo"
import "AdminServiceCapabilities"
import "AdminServiceTool"
import "AdminServiceTradeshow"
import "AdminDevTools"
import "../../Widgets"

Rectangle {
    property int buttonWidth: width * 0.15
    property int buttonHeight: height * 0.2
    property string hcuBuildType: dsSystem.hcuBuildType
    property string statePath: dsSystem.statePath
    property bool mudsInserted: dsMcu.mudsInserted
    property var powerStatus: dsMcu.powerStatus
    property bool tradeshowModeEnabled: dsCfgGlobal.tradeshowModeEnabled

    visible: false
    color: colorMap.mainBackground
    anchors.fill: parent

    ListModel {
        id: listModelNormal
        ListElement { name: "Service Settings" }
        ListElement { name: "Upgrade" }
        ListElement { name: "Service Tool" }
        ListElement { name: "Hardware Info" }
    }

    ListModel {
        id: listModelDev
        ListElement { name: "Service Settings" }
        ListElement { name: "Upgrade" }
        ListElement { name: "Service Tool" }
        ListElement { name: "Capabilities" }
        ListElement { name: "Hardware Info" }
        ListElement { name: "Developer Tools" }
    }

    ListModel {
        id: listModelProduction
        ListElement { name: "Upgrade" }
        ListElement { name: "Capabilities" }
    }

    GridView {
        id: optionSelectList
        y: (parent.height + actionBar.height) * 0.05
        width: parent.width * 0.8
        height: parent.height - y
        anchors.horizontalCenter: parent.horizontalCenter

        cellWidth: parent.width * 0.2
        cellHeight: cellWidth

        ScrollBar {}
        ListFade {}

        delegate: GenericButton {
            width: optionSelectList.cellWidth * 0.9
            height: width
            radius: 0
            color: colorMap.comboBoxBackground

            content: [
                Image {
                    source: getIcon(name)
                    sourceSize.height: parent.height * 0.45
                    sourceSize.width: parent.width * 0.45
                    height: sourceSize.height
                    width: sourceSize.width
                    y: parent.height * 0.2
                    anchors.horizontalCenter: parent.horizontalCenter
                },
                Text {
                    y: parent.height * 0.7
                    width: parent.width
                    height: parent.height - y
                    text: name
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    font.pixelSize: height * 0.33
                    font.family: fontRobotoLight.name
                    color: colorMap.text01
                }
            ]
            Component.onCompleted: {
                optionSelectList.dragStarted.connect(reset);
            }

            Component.onDestruction: {
                optionSelectList.dragStarted.disconnect(reset);
            }

            onBtnClicked: {
                adminBtnClicked(name);
            }

        }
    }

    Item {
        id: servicePages
        width: parent.width
        height: parent.height

        AdminServiceSettings {}
        AdminServiceUpgrade {}
        AdminServiceTool {}
        AdminServiceCapabilities {}
        AdminServiceHardwareInfo {}
        AdminServiceTradeshow {}
        AdminDevTools {}
    }

    onVisibleChanged: {
        dsSystem.slotServiceModeActive(visible);
    }

    onStatePathChanged: {
        reload();
    }

    Component.onCompleted: {
        appMain.screenStateChanged.connect(slotScreenStateChanged);
        slotScreenStateChanged();
    }

    Component.onDestruction: {
        appMain.screenStateChanged.disconnect(slotScreenStateChanged);
    }

    function slotScreenStateChanged()
    {
        visible = (appMain.screenState.indexOf("Admin-Service-") >= 0);
        reload();
    }

    function reload()
    {
        if (!visible)
        {
            return;
        }

        if (statePath == "Idle")
        {
            appMain.setInteractiveState(false, "AdminService: Still waiting for StatePath transition to service");
        }
        else
        {
            appMain.setInteractiveState(true, "AdminService: StatePath=" + statePath + ". Service screen enabled.");
        }

        optionSelectList.model = getTilesModel();
        optionSelectList.visible = (appMain.screenState === "Admin-Service-Select");
        servicePages.visible = (appMain.screenState !== "Admin-Service-Select");
    }

    function getTilesModel()
    {
        var tiles;
        if (hcuBuildType == "PROD")
        {
            tiles = listModelProduction;
        }
        else if ( (hcuBuildType == "DEV") || (hcuBuildType == "SQA") || (tradeshowModeEnabled) )
        {
            tiles = listModelDev;
        }
        else
        {
            tiles = listModelNormal;
        }

        // Determine if the Tradeshow tile should be displayed/removed
        var tradeshowModeIndex = -1;
        for (var i = 0; i < tiles.count; i++)
        {
            if (tiles.get(i).name === "Tradeshow")
            {
                tradeshowModeIndex = i;
                break;
            }
        }
        if (tradeshowModeEnabled)
        {
            if (tradeshowModeIndex === -1)
            {
                tiles.append({"name":"Tradeshow"});
            }
        }
        else
        {
            if(tradeshowModeIndex !== -1)
            {
                tiles.remove(tradeshowModeIndex);
            }
        }

        return tiles;
    }

    function getIcon(name)
    {
        if (name === "Service Settings")
        {
            return imageMap.adminMenuServiceSettings;
        }
        else if (name === "Upgrade")
        {
            return imageMap.adminMenuUpgrade;
        }
        else if (name === "Service Tool")
        {
            return imageMap.adminMenuServiceTool;
        }
        else if (name === "Capabilities")
        {
            return imageMap.adminMenuCapabilities;
        }
        else if (name === "Hardware Info")
        {
            return imageMap.adminMenuHardwareInfo;
        }
        else if (name === "Tradeshow")
        {
            return imageMap.bayerIcon;
        }
        else if (name === "Developer Tools")
        {
            // TODO - better icon
            return imageMap.adminMenuServiceTool;
        }

        return "";
    }

    function adminBtnClicked(name)
    {
        if (name === "Service Settings")
        {
            appMain.setScreenState("Admin-Service-Settings");
        }
        else if (name === "Upgrade")
        {
            if (hcuBuildType == "DEV")
            {
                appMain.setScreenState("Admin-Service-Upgrade");
            }
            else if (mudsInserted)
            {
                popupManager.popupEjectMudsRequired.open();
            }
            else if ( (powerStatus !== undefined) &&
                      (!powerStatus.IsAcPowered) &&
                      (powerStatus.BatteryLevel === "Dead") )
            {
                popupManager.popupBatteryDead.open();
            }
            else
            {
                appMain.setScreenState("Admin-Service-Upgrade");
            }
        }
        else if (name === "Service Tool")
        {
            appMain.setScreenState("Admin-Service-Tool");
        }
        else if (name === "Capabilities")
        {
            appMain.setScreenState("Admin-Service-Capabilities");
        }
        else if (name === "Hardware Info")
        {
            appMain.setScreenState("Admin-Service-HardwareInfo");
        }
        else if (name === "Tradeshow")
        {
            appMain.setScreenState("Admin-Service-Tradeshow");
        }
        else if (name === "Developer Tools")
        {
            appMain.setScreenState("Admin-Service-DeveloperTools");
        }
    }
}
