import QtQuick 2.12
import QtGraphicalEffects 1.12
import "../Widgets"
import "../Util.js" as Util

Item {
    property string hcuBuildType: dsSystem.hcuBuildType
    property var activeSystemAlerts: dsAlert.activeSystemAlerts
    property string statePath: dsSystem.statePath
    property string lastStatePath: dsSystem.lastStatePath
    property int systemAlertsPanelHeight: frameSystemAlertsPanel.height
    property int animationMs: 250
    property var activeFatalAlerts: []
    property int numFatalErrors: activeFatalAlerts.length

    id: root
    state: "COMPRESSED"

    states: [
        State { name: "COMPRESSED" },
        State { name: "EXPANDED" }
    ]

    transitions: [
        Transition {
            to: "EXPANDED"
            SequentialAnimation {
                ParallelAnimation {
                    NumberAnimation { target: frameListView; properties: "height"; to: listView.height; duration: animationMs;  }
                    NumberAnimation { target: stateIconFrame; properties: 'rotation'; to: 180; duration: animationMs }
                }

                ScriptAction { script: {
                        frameListView.height = listView.height;
                    }
                }
            }
        },

        Transition {
            to: "COMPRESSED"
            SequentialAnimation {
                ParallelAnimation {
                    NumberAnimation { target: frameListView; properties: "height"; to: 0; duration: animationMs; }
                    NumberAnimation { target: stateIconFrame; properties: 'rotation'; to: 0; duration: animationMs }
                }
            }
        }
    ]

    GenericButton {
        id: btnVisibleCtrl
        x: frameSystemAlertsBtn.x
        y: frameSystemAlertsBtn.y
        color: root.state == "EXPANDED" ? colorMap.systemAlertsPanelBackground : "transparent"
        width: frameSystemAlertsBtn.width
        height: frameSystemAlertsBtn.height
        radius: 0

        content: [
            Text {
                width: parent.width
                height: parent.height * 0.75
                font.family: fontIcon.name
                font.pixelSize: height * 0.7
                anchors.verticalCenter: parent.verticalCenter
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                color: colorMap.text01
                text: "\ue912"

                Text {
                    height: parent.height * 0.3
                    anchors.centerIn: parent

                    font.family: fontIcon.name
                    text: "\ue93e"
                    color: (numFatalErrors > 0) ? colorMap.red : colorMap.gry01
                    font.pixelSize: height
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter

                    Text {
                        font.family: fontRobotoBold.name
                        text: (numFatalErrors > 0) ? numFatalErrors : "i"
                        color: colorMap.white01
                        anchors.fill: parent
                        font.pixelSize: parent.font.pixelSize * 0.9
                        verticalAlignment: parent.verticalAlignment
                        horizontalAlignment: parent.horizontalAlignment
                        fontSizeMode: Text.Fit
                        minimumPixelSize: font.pixelSize * 0.7
                    }
                }
            },

            Item {
                id: stateIconFrame
                height: parent.height * 0.25
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom

                Text {
                    id: iconExpandedState
                    anchors.fill: parent
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    color: colorMap.text01
                    font.pixelSize: height
                    font.family: fontIcon.name
                    text: "\ue906"
                }
            }
        ]
        onBtnClicked: {
            if (root.state == "COMPRESSED")
            {
                root.state = "EXPANDED";
            }
            else
            {
                root.state = "COMPRESSED";
            }
        }
    }

    Item {
        id: frameListView
        x: frameSystemAlertsPanel.x
        y: frameSystemAlertsPanel.y
        width: frameSystemAlertsPanel.width
        height: 0
        clip: true

        Rectangle {
            // background
            anchors.fill: parent
            opacity: 0.5
            color: colorMap.buttonShadow
        }

        MouseArea {
            // Prevent background touch
            anchors.fill: parent
        }

        ListView {
            id: listView
            height: Math.min(systemAlertsPanelHeight, contentHeight)
            width: parent.width
            clip: true
            cacheBuffer: systemAlertsPanelHeight * 10
            spacing: frameSystemAlertsPanel.height * 0.003
            delegate: TitleBarSystemAlertItem {}

            ScrollBar {}
            ListFade {}

            onHeightChanged: {
                if ( (root.state == "EXPANDED") &&
                     (frameListView.height != listView.height) )
                {
                    // ListView height changed (probably model changed), update frameListView height
                    root.state = "UNKNOWN";
                    root.state = "EXPANDED";
                }
            }
        }

        layer.enabled: true
        layer.effect: Glow {
            radius: 17
            samples: 17
            spread: 0.5
            color: colorMap.buttonShadow
            transparentBorder: true
        }
    }

    onActiveSystemAlertsChanged: {
        activeFatalAlerts = activeSystemAlerts.filter(function(alert) { return alert["Severity"] === "Fatal"; });
        reload();
    }

    onStatePathChanged: {
        if ( (statePath != "StartupUnknown") &&
             (lastStatePath == "StartupUnknown") )
        {
            reload();
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
        if ( (appMain.screenState != "Admin-Service-Upgrade") &&
             (appMain.screenStatePrev == "Admin-Service-Upgrade") )
        {
            reload();
        }
    }

    function reload()
    {
        if ( (statePath == "OffUnreachable") ||
             (statePath == "OnReachable") ||
             (statePath == "StartupUnknown") )
        {
            return;
        }

        if (JSON.stringify(listView.model) === JSON.stringify(activeSystemAlerts))
        {
            return;
        }

        listView.model = activeSystemAlerts;

        if ( (hcuBuildType != "PROD") &&
             (activeSystemAlerts.length > 0) )
        {
            frameSystemAlertsBtn.width = dsCfgLocal.screenW * 0.05;
            soundPlayer.playError();
            if (hcuBuildType == "DEV")
            {
                // DEV mode. Don't need to expand
            }
            else if (appMain.screenState == "Admin-Service-Upgrade")
            {
                // Upgrade is in progress. Don't need to expand
            }
            else
            {
                if (numFatalErrors > 0)
                {
                    root.state = "UNKNOWN";
                    root.state = "EXPANDED";
                }
            }
        }
        else
        {
            frameSystemAlertsBtn.width = 0;
            root.state = "COMPRESSED";
        }
    }
}
