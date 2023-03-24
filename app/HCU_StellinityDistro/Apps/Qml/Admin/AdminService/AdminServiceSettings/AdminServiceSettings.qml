import QtQuick 2.12
import "../../../Widgets"
import "../../../Widgets/ConfigTable"
import "../../../Util.js" as Util

Item {
    property int frameMargin: dsCfgLocal.screenW * 0.02
    property int leftFrameWidth: (dsCfgLocal.screenW * 0.21) + (frameMargin * 2)
    property int listViewItemHeight: parent.height * 0.12
    property int cellMargin: width * 0.015
    property int currentUtcOffsetMinutes: dsCfgLocal.currentUtcOffsetMinutes
    property string hcuBuildType: dsSystem.hcuBuildType
    property string haspKeyEnforcementServiceKey: dsCapabilities.haspKeyEnforcementServiceKey

    anchors.fill: parent
    visible: false

    ListView {
        id: listView
        width: leftFrameWidth
        height: parent.height
        clip: true

        ScrollBar {}
        ListFade {}

        model: ListModel {
            ListElement { title: "Admin" }
            ListElement { title: "Date & Time" }
            ListElement { title: "Calibration" }
        }

        delegate: GenericButton {
            width: ListView.view.width
            height: listViewItemHeight
            radius: 0
            color: (listView.currentIndex === index) ? colorMap.text02 : "transparent"

            Text {
                text: title
                anchors.fill: parent
                anchors.leftMargin: cellMargin
                anchors.rightMargin: cellMargin
                font.family: (listView.currentIndex === index) ? fontRobotoBold.name : fontRobotoLight.name
                font.pixelSize: height * 0.4
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
                color: (listView.currentIndex === index) ? colorMap.white01 : colorMap.text01
                wrapMode: Text.Wrap
            }

            onBtnClicked: {
                selectPage(index);
            }

            Component.onCompleted: {
                listView.dragStarted.connect(reset);
            }

            Component.onDestruction: {
                listView.dragStarted.disconnect(reset);
            }
        }
    }

    Rectangle {
        // separator line
        id: rectSeparator
        anchors.left: listView.right
        width: frameMargin / 6
        height: parent.height
        color: colorMap.titleBarBackground
    }


    Item {
        id: mainRect
        anchors.left: rectSeparator.right
        anchors.leftMargin: frameMargin
        width: parent.width - x - frameMargin
        height: parent.height

        ConfigTable {
            allConfigList: dsCfgGlobal.configTable
            translationRequired: false
            configKeyName: "Service_"
            visibleScreenState: "Admin-Service-Settings-Admin"
            onSignalConfigChanged: (configItem) => {
                dsCfgGlobal.slotConfigChanged(configItem);
            }
            customConfigRows: {
                "Service_ServicePasscode": {
                    "getVisibleState": function() {
                        if (hcuBuildType == "REL")
                        {
                            for (var cfgIdx = 0; cfgIdx < allConfigList.length; cfgIdx++)
                            {
                                if (allConfigList[cfgIdx].KeyName === "Service_ServicePasscode")
                                {
                                    var servicePassCode = allConfigList[cfgIdx].Value;
                                    // Hide 'ServicePasscode' when HaspKeyEnabled and BuildType is REL
                                    return (servicePassCode !== haspKeyEnforcementServiceKey);
                                }
                             }
                        }
                        return true;
                    }
                }
            }
        }

        ConfigTable {
            allConfigList: dsCapabilities.configTable
            translationRequired: false
            configKeyName: "DateTime_"
            visibleScreenState: "Admin-Service-Settings-DateTime"
            onSignalConfigChanged: (configItem) => {
                dsCapabilities.slotConfigChanged(configItem);
            }
            customConfigRows: {
                "DateTime_SystemDateTime": {
                    "getTextFromConfigValue": function(configValue) {
                        if (widgetInputPad.isOpen())
                        {
                            // Editing is in progress. Don't update to current time
                            return configValue;
                        }

                        var newConfigVal = Util.getCurrentDateTimeStr("yyyy/MM/dd", "hh:mm", currentUtcOffsetMinutes);
                        return newConfigVal;
                    },
                    "setConfigValueBeforeEdit": function(configData) {
                        configData.Value = Util.getCurrentDateTimeStr("yyyy/MM/dd", "hh:mm", currentUtcOffsetMinutes);
                    },
                    "updateFromRealDateTime": true
                }
            }
        }

        ConfigTable {
            allConfigList: dsCfgLocal.configTable
            translationRequired: false
            configKeyName: "ServiceCalibration_"
            visibleScreenState: "Admin-Service-Settings-Calibration"
            onSignalConfigChanged: (configItem) => {
                dsCfgLocal.slotConfigChanged(configItem);
            }
        }
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
        var prevVisible = (appMain.screenStatePrev.indexOf("Admin-Service-Settings") >= 0);
        visible = (appMain.screenState.indexOf("Admin-Service-Settings") >= 0);
        if ( (visible) && (!prevVisible))
        {
            selectPage(0);
        }
        reload();
    }

    function reload()
    {
        if (!visible)
        {
            widgetInputPad.close();
            return;
        }
    }

    function selectPage(pageIdx)
    {
        listView.currentIndex = pageIdx;

        if (listView.currentIndex == 0)
        {
            appMain.setScreenState("Admin-Service-Settings-Admin");
        }
        else if (listView.currentIndex == 1)
        {
            appMain.setScreenState("Admin-Service-Settings-DateTime");
        }
        else if (listView.currentIndex == 2)
        {
            appMain.setScreenState("Admin-Service-Settings-Calibration");
        }
    }
}
