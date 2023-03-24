import QtQuick 2.12
import "../../../Widgets"
import "../../../Widgets/ConfigTable"

Item {
    property string hcuBuildType: dsSystem.hcuBuildType
    property int frameMargin: dsCfgLocal.screenW * 0.02
    property int leftFrameWidth: (dsCfgLocal.screenW * 0.21) + (frameMargin * 2)
    property int listViewItemHeight: parent.height * 0.115
    property int cellMargin: width * 0.015
    property int webAppHostPort: dsSystem.webAppHostPort
    property var networkSettingParams: dsSystem.networkSettingParams
    property string defaultInjectPlanTemplateGuid: dsExam.defaultInjectPlanTemplateGuid
    property bool continuousExamsTestEnabled: dsCapabilities.continuousExamsTestEnabled
    property var screenStateIndexes: [
        "Admin-Service-Capabilities-General",
        "Admin-Service-Capabilities-Developer",
        "Admin-Service-Capabilities-FluidControl",
        "Admin-Service-Capabilities-Prime",
        "Admin-Service-Capabilities-AirCheck",
        "Admin-Service-Capabilities-Preload",
        "Admin-Service-Capabilities-Network",
        "Admin-Service-Capabilities-BMS",
        "Admin-Service-Capabilities-Alert",
        "Admin-Service-Capabilities-Logging",
        "Admin-Service-Capabilities-Calibration"
    ]

    anchors.fill: parent
    visible: false

    ListView {
        id: listView
        width: leftFrameWidth
        height: parent.height
        clip: true

        ListModel {
            id: listModelNormal
            ListElement { title: "General" }
            ListElement { title: "Developer" }
            ListElement { title: "FluidControl - General" }
            ListElement { title: "FluidControl - Prime" }
            ListElement { title: "FluidControl - AirCheck" }
            ListElement { title: "FluidControl - Preload" }
            ListElement { title: "Network" }
            ListElement { title: "BMS" }
            ListElement { title: "Alert" }
            ListElement { title: "Logging" }
            ListElement { title: "Calibration" }
        }

        ListModel {
            id: listModelProduction
            ListElement { title: "General" }
        }

        ScrollBar {}
        ListFade {}

        model: (hcuBuildType == "PROD") ? listModelProduction : listModelNormal

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
                font.pixelSize: height * 0.35
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
            allConfigList: dsCapabilities.configTable
            translationRequired: false
            configKeyName: "General_"
            visibleScreenState: "Admin-Service-Capabilities-General"
            onSignalConfigChanged: (configItem) => {
                dsCapabilities.slotConfigChanged(configItem);
            }

            Rectangle {
                id: webApplicationEnabledNote
                width: parent.width
                height: mainRect.height * 0.25
                color: colorMap.white01

                Image {
                    id: barcodeWifi
                    anchors.top: parent.top
                    anchors.topMargin: parent.height * 0.1
                    anchors.left: parent.left
                    anchors.leftMargin: parent.width * 0.1
                    sourceSize.height: parent.height * 0.7
                    sourceSize.width: parent.height * 0.7
                }

                Text {
                    id: captionBarcodeWifi
                    x: barcodeWifi.x
                    anchors.top: barcodeWifi.bottom
                    anchors.bottom: parent.bottom
                    width: barcodeWifi.sourceSize.width
                    text: "WIFI"
                    color: colorMap.blk01
                    font.pixelSize: height * 0.5
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                }


                Image {
                    id: barcodeUrl
                    y: barcodeWifi.y
                    anchors.right: parent.right
                    anchors.rightMargin: parent.width * 0.1
                    sourceSize.height: barcodeWifi.sourceSize.height
                    sourceSize.width: barcodeWifi.sourceSize.width
                }

                Text {
                    anchors.top: barcodeUrl.bottom
                    x: barcodeUrl.x
                    height: captionBarcodeWifi.height
                    width: barcodeUrl.sourceSize.width
                    text: "URL"
                    color: captionBarcodeWifi.color
                    font.pixelSize: captionBarcodeWifi.font.pixelSize
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                }
            }

            customConfigRows: {
                "General_WebApplicationEnabled": {
                    "getDescription": function() {
                        return "Centargo Web Appliocation Feature.\n" +
                               "Steps to run the application:\n" +
                               "1. Make sure CRU/AP is connected.\n" +
                               "2. Connect to CRU/AP (" + networkSettingParams.Ssid + " / " + networkSettingParams.Pwd + ") OR Scan the QR Code below\n" +
                               "3. Open URL (http://" + networkSettingParams.RouterIp + ":" + webAppHostPort.toString() +") OR Scan the QR Code below)\n\n";
                    },
                    "setNote": function(rectNote) {
                        barcodeWifi.source = Qt.binding(function() { return "image://QZXing/encode/" + "WIFI:T:WPA;S:" + networkSettingParams.Ssid + ";P:" + networkSettingParams.Pwd + ";;"; });
                        barcodeUrl.source = Qt.binding(function() { return "image://QZXing/encode/" + "URL:http://" + networkSettingParams.RouterIp + ":" + webAppHostPort.toString(); });
                        webApplicationEnabledNote.parent = rectNote;
                        rectNote.height = webApplicationEnabledNote.height
                    },
                    "getVisibleState": function() {
                        // Hide the row.
                        return false;
                    }
                }
            }
        }

        ConfigTable {
            allConfigList: dsCapabilities.configTable
            translationRequired: false
            configKeyName: "Developer_"
            visibleScreenState: "Admin-Service-Capabilities-Developer"
            onSignalConfigChanged: (configItem) => {
                dsCapabilities.slotConfigChanged(configItem);
            }
            customConfigRows: {
                "Developer_ContinuousExamsTestEnabled": {
                    "getDescription": function() {
                        return "Enable Continuous Exams Test.\n\n" +
                               "NOTE: Each exam starts with default plan (TemplateGuid=" + defaultInjectPlanTemplateGuid + ").";
                    }
                },
                "Developer_ContinuousExamsTestLimit": {
                    "getVisibleState": function() {
                        return continuousExamsTestEnabled;
                    },
                    "getDescription": function() {
                        return "Test limit number, 0 if unlimited";
                    }
                },
                "Developer_ContinuousExamsTestExamStartDelaySec": {
                    "getVisibleState": function() {
                        return continuousExamsTestEnabled;
                    },
                    "getDescription": function() {
                        return "Interval between exam creation";
                    }
                },
                "Developer_ContinuousExamsTestSelectedPlans": {
                    "getVisibleState": function() {
                        return false;
                    }
                },
                "Developer_ContinuousExamsTestActiveContrastLocation": {
                    "getVisibleState": function() {
                        return continuousExamsTestEnabled;
                    },
                    "getDescription": function() {
                        return "Active Contrast location for each exam";
                    }
                },
            }
        }

        ConfigTable {
            allConfigList: dsCapabilities.configTable
            translationRequired: false
            configKeyName: "FluidControl_"
            visibleScreenState: "Admin-Service-Capabilities-FluidControl"
            onSignalConfigChanged: (configItem) => {
                dsCapabilities.slotConfigChanged(configItem);
            }
        }

        ConfigTable {
            allConfigList: dsCapabilities.configTable
            translationRequired: false
            configKeyName: "Network_"
            visibleScreenState: "Admin-Service-Capabilities-Network"
            onSignalConfigChanged: (configItem) => {
                dsCapabilities.slotConfigChanged(configItem);
            }

            customConfigRows: {
            }
        }

        ConfigTable {
            allConfigList: dsCapabilities.configTable
            translationRequired: false
            configKeyName: "Prime_"
            visibleScreenState: "Admin-Service-Capabilities-Prime"
            onSignalConfigChanged: (configItem) => {
                dsCapabilities.slotConfigChanged(configItem);
            }
        }

        ConfigTable {
            allConfigList: dsCapabilities.configTable
            translationRequired: false
            configKeyName: "AirCheck_"
            visibleScreenState: "Admin-Service-Capabilities-AirCheck"
            onSignalConfigChanged: (configItem) => {
                dsCapabilities.slotConfigChanged(configItem);
            }
        }

        ConfigTable {
            allConfigList: dsCapabilities.configTable
            translationRequired: false
            configKeyName: "Preload_"
            visibleScreenState: "Admin-Service-Capabilities-Preload"
            onSignalConfigChanged: (configItem) => {
                dsCapabilities.slotConfigChanged(configItem);
            }
        }

        ConfigTable {
            allConfigList: dsCapabilities.configTable
            translationRequired: false
            configKeyName: "BMS_"
            visibleScreenState: "Admin-Service-Capabilities-BMS"
            onSignalConfigChanged: (configItem) => {
                dsCapabilities.slotConfigChanged(configItem);
            }
        }

        ConfigTable {
            allConfigList: dsCapabilities.configTable
            translationRequired: false
            configKeyName: "Alert_"
            visibleScreenState: "Admin-Service-Capabilities-Alert"
            onSignalConfigChanged: (configItem) => {
                dsCapabilities.slotConfigChanged(configItem);
            }
        }

        ConfigTable {
            allConfigList: dsCapabilities.configTable
            translationRequired: false
            configKeyName: "Logging_"
            visibleScreenState: "Admin-Service-Capabilities-Logging"
            onSignalConfigChanged: (configItem) => {
                dsCapabilities.slotConfigChanged(configItem);
            }
        }

        ConfigTable {
            allConfigList: dsCapabilities.configTable
            translationRequired: false
            configKeyName: "Calibration_"
            visibleScreenState: "Admin-Service-Capabilities-Calibration"
            onSignalConfigChanged: (configItem) => {
                dsCapabilities.slotConfigChanged(configItem);
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
        var prevVisible = (appMain.screenStatePrev.indexOf("Admin-Service-Capabilities") >= 0);
        visible = (appMain.screenState.indexOf("Admin-Service-Capabilities") >= 0);
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
        appMain.setScreenState(screenStateIndexes[listView.currentIndex]);
    }
}
