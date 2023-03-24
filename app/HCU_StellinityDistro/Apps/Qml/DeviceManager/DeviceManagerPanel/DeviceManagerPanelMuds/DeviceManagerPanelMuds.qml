import QtQuick 2.12
import ".."
import "../../DeviceManagerWidgets"
import "../../../Widgets"
import "../../../Widgets/Popup"
import "../../../Util.js" as Util
import "../../../PopupManager"
import "../../../CommonElements"

DeviceManagerPanel {
    property bool mudsPresent: dsMcu.mudsPresent
    property var activeAlerts: dsAlert.activeAlerts
    property var fluidSourceMuds: dsDevice.fluidSourceMuds
    property string workflowState: dsWorkflow.workflowState
    property int mudsUseLifeLimitHours: dsCfgGlobal.mudsUseLifeLimitHours
    property string statePath: dsSystem.statePath
    property string workflowMudsSodState: dsWorkflow.workflowMudsSodState
    property string mudsEjectDisabledReason: getMudsEjectDisabledReason()
    property bool mudsEjectEnabled: getMudsEjectEnabled()
    property bool mudsPurgeEnabled: getMudsPurgeEnabled()
    property bool prevMudsEjectEnabled: false
    property bool prevMudsPurgeEnabled: false

    titleText: "T_DaySet"
    panelName: "MUDS"
    panelActiveAlerts: activeAlertsMuds

    PopupMudsEject {
        id: popUpMudsEject
    }

    content: [
        Column {
            anchors.fill: parent
            anchors.topMargin: parent.height * 0.04
            spacing: actionButtonSpacing

            Rectangle {
                id: containerHeatMaintainer

                width: actionButtonWidth
                height: actionButtonHeight
                radius: buttonRadius

                color: "transparent"
                border.color: colorMap.keypadButton
                border.width: readOnlyBoxBorderWidth

                LabelAndText {
                    anchors.fill: parent
                    anchors.leftMargin: actionButtonTextMargin
                    anchors.rightMargin: actionButtonTextMargin
                    maximumLabelWidth: parent.width * 0.6
                    spacing: parent.width * 0.03

                    widgetLabel.text: translate("T_HeatMaintainer") + translate("T_:")
                    widgetLabel.color: colorMap.text02
                    widgetLabel.font.pixelSize: height * 0.3
                    widgetLabel.font.family: fontRobotoLight.name

                    widgetText.text: {
                        var state = "T_On";
                        for (var alertIdx = 0; alertIdx < activeAlerts.length; alertIdx++)
                        {
                            if (activeAlerts[alertIdx].CodeName === "HeatMaintainerIsOff")
                            {
                                state = "T_Off";
                                break;
                            }
                        }
                        return translate(state);
                    }
                    widgetText.color: colorMap.text01
                    widgetText.font.pixelSize: height * 0.33
                    widgetText.font.family: fontRobotoBold.name
                    widgetText.wrapMode: Text.Wrap
                }
            }

            GenericButton {
                id: btnMudsEject
                visible: mudsPresent
                width: actionButtonWidth
                height: visible ? actionButtonHeight : 0
                color: colorMap.keypadButton
                enabled: mudsEjectEnabled

                content: [
                    Text {
                        id: iconEjectMuds
                        font.family: fontIcon.name
                        text: "\ue935"
                        width: actionButtonIconWidth
                        height: parent.height
                        font.pixelSize: height * 0.4
                        color: colorMap.text01
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                    },

                    Text {
                        font.family: fontRobotoLight.name
                        text: translate("T_EjectDaySet")
                        anchors.left: iconEjectMuds.right
                        anchors.right: parent.right
                        anchors.rightMargin: actionButtonTextMargin
                        height: parent.height
                        font.pixelSize: height * 0.35
                        color: colorMap.text01
                        verticalAlignment: Text.AlignVCenter
                        wrapMode: Text.Wrap
                    }
                ]

                onBtnClicked: {
                    popUpMudsEject.init();
                    popUpMudsEject.open();
                }
            }

            GenericButton {
                id: btnCleaningMode
                visible: !mudsPresent
                width: actionButtonWidth
                height: visible ? actionButtonHeight : 0
                color: colorMap.keypadButton

                content: [
                    Text {
                        id: iconMovePistons
                        font.family: fontIcon.name
                        text: "\ue984"
                        width: actionButtonIconWidth
                        height: parent.height
                        font.pixelSize: height * 0.46
                        color: colorMap.text01
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                    },
                    Text {
                        font.family: fontRobotoLight.name
                        text: translate("T_CleaningMode")
                        anchors.left: iconMovePistons.right
                        anchors.right: parent.right
                        anchors.rightMargin: actionButtonTextMargin
                        height: parent.height
                        font.pixelSize: height * 0.35
                        color: colorMap.text01
                        verticalAlignment: Text.AlignVCenter
                        wrapMode: Text.Wrap
                    }
                ]

                onBtnClicked: {
                    pageMovePistons.open();
                }

                onVisibleChanged: {
                    if (!visible)
                    {
                        if (pageMovePistons.isOpened())
                        {
                            pageMovePistons.close();
                        }
                    }
                }
            }

            GenericButton {
                id: btnMudsPurge
                visible: mudsPresent
                width: actionButtonWidth
                height: visible ? actionButtonHeight : 0
                color: colorMap.keypadButton
                enabled: mudsPurgeEnabled

                content: [
                    Item {
                        id: iconPurgeMuds
                        width: actionButtonIconWidth
                        height: parent.height * 0.65
                        anchors.verticalCenter: parent.verticalCenter

                        Text {
                            font.family: fontIcon.name
                            width: parent.width
                            height: parent.height * 0.35
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignHCenter
                            font.pixelSize: height * 0.8
                            text: "\ue930"
                            color: colorMap.text01
                        }

                        Text {
                            font.family: fontAwesome.name
                            width: parent.width
                            y: parent.height * 0.4
                            height: parent.height * 0.6
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignHCenter
                            font.pixelSize: height
                            text: "\uf014"
                            color: colorMap.text01
                        }
                    },
                    Text {
                        font.family: fontRobotoLight.name
                        text: translate("T_EmptyDaySet")
                        anchors.left: iconPurgeMuds.right
                        anchors.right: parent.right
                        anchors.rightMargin: actionButtonTextMargin
                        height: parent.height
                        font.pixelSize: height * 0.35
                        color: colorMap.text01
                        verticalAlignment: Text.AlignVCenter
                        wrapMode: Text.Wrap
                    }
                ]

                onBtnClicked: {
                    popupManager.popupEndOfDayPurge.open();
                }
            }

            AutoEmptyModeEnabledMsg {
                id: autoEmptyEnabledIndicator
                visible:  {
                    parent.visible && (dsCfgGlobal.autoEmptySalineEnabled || dsCfgGlobal.autoEmptyContrast1Enabled || dsCfgGlobal.autoEmptyContrast2Enabled);
                }
                height: visible ? (actionButtonHeight * 0.7) : 0
                width: parent.width
            }
        }
    ]

    DeviceManagerPanelMudsMovePistons {
        id: pageMovePistons
        height: parent.height
        width: parent.width
    }

    Timer {
        // To prevent screen flickering (due to fluid source state change), use sw debouncer
        id: tmrDebounceScreenState
        interval: 100 // Debounce time
        onTriggered: {
            updateScreen();
        }
    }

    textDisposableState2: getTextDisposableState2()

    onFluidSourceMudsChanged: {
        reload();
    }

    onVisibleChanged: {
        reload();
    }

    onMudsUseLifeLimitHoursChanged: {
        reload();
    }

    onMudsEjectEnabledChanged: {
        tmrDebounceScreenState.stop();
        tmrDebounceScreenState.start();
        timerSingleShot(1, function() {
            prevMudsEjectEnabled = mudsEjectEnabled;
        });
    }

    onMudsPurgeEnabledChanged: {
        tmrDebounceScreenState.stop();
        tmrDebounceScreenState.start();
        timerSingleShot(1, function() {
            prevMudsPurgeEnabled = mudsPurgeEnabled;
        });
    }

    function updateScreen()
    {
        btnMudsEject.enabled = mudsEjectEnabled;
        btnMudsPurge.enabled = mudsPurgeEnabled;
    }

    function reload()
    {
        if (!visible)
        {
            return;
        }

        if ( (fluidSourceMuds === undefined) ||
             (fluidSourceMuds.InstalledAt === undefined) )
        {
            textDisposableState = "T_NotPresent";
            stopElapsedTimer();
        }
        else if (fluidSourceMuds.NeedsReplaced)
        {
            textDisposableState = "";
            stopElapsedTimer();
            disposableNeedsReplaced = true;
        }
        else
        {
            var disposableInsertedEpochMsNew = Util.utcDateTimeToMillisec(fluidSourceMuds.InstalledAt);
            var maxUseDurationMsNew = mudsUseLifeLimitHours * 60 * 60 * 1000;
            initElapsedTimer(disposableInsertedEpochMsNew, maxUseDurationMsNew);
            startElapsedTimer();
        }
    }

    function getMudsEjectDisabledReason()
    {
        var disabledReason = "";

        if ( (fluidSourceMuds === undefined) ||
             (fluidSourceMuds.InstalledAt === undefined) )
        {
            disabledReason = "MUDS is not inserted";
        }
        else if (fluidSourceMuds.IsBusy)
        {
            disabledReason = "MUDS is busy";
        }
        else if ( (fluidSourceBottle1.IsBusy) ||
                  (fluidSourceBottle2.IsBusy) ||
                  (fluidSourceBottle3.IsBusy) )
        {
            disabledReason = "Bottle is busy";
        }
        else if ( (fluidSourceSyringe1.IsBusy) ||
                  (fluidSourceSyringe2.IsBusy) ||
                  (fluidSourceSyringe3.IsBusy) )
        {
            disabledReason = "Syringe is busy";
        }
        else if (workflowState === "STATE_MUDS_EJECT_PROGRESS")
        {
            disabledReason = "Workflow State is " + workflowState;
        }
        else if (workflowMudsSodState == "MUDS_SOD_STATE_SYRINGE_SOD_PROGRESS")
        {
            disabledReason = "MUDS SOD State is " + workflowMudsSodState;
        }

        return disabledReason;
    }

    function getMudsEjectEnabled()
    {
        var disabledReason = "";

        if (mudsEjectDisabledReason !== "")
        {
            disabledReason = mudsEjectDisabledReason;
        }

        var newMudsEjectEnabled = (disabledReason === "");

        if (newMudsEjectEnabled != prevMudsEjectEnabled)
        {
            if (newMudsEjectEnabled)
            {
                logInfo("DeviceManagerPanelMuds: MUDS Eject Enabled");
            }
            else
            {
                logInfo("DeviceManagerPanelMuds: MUDS Eject Disabled: " + disabledReason);
            }
        }

        return newMudsEjectEnabled;
    }

    function getMudsPurgeEnabled()
    {
        var disabledReason = "";

        if (mudsEjectDisabledReason !== "")
        {
            disabledReason = mudsEjectDisabledReason;
        }
        else if ( (syringeVolume1 <= 0) &&
                  (syringeVolume2 <= 0) &&
                  (syringeVolume3 <= 0) )
        {
            disabledReason = "Syringe Volumes are 0";
        }

        var newMudsPurgeEnabled = (disabledReason === "");

        if (newMudsPurgeEnabled != prevMudsPurgeEnabled)
        {
            if (newMudsPurgeEnabled)
            {
                logInfo("DeviceManagerPanelMuds: DeviceManagerPanelMuds: MUDS Purge Enabled");
            }
            else
            {
                logInfo("DeviceManagerPanelMuds: DeviceManagerPanelMuds: MUDS Purge Disabled: " + disabledReason);
            }
        }

        return newMudsPurgeEnabled;
    }

    function getTextDisposableState2()
    {
        for (var alertIdx = 0; alertIdx < activeAlerts.length; alertIdx++)
        {
            if (activeAlerts[alertIdx].CodeName === "SRUFillingMUDS")
            {
                return "T_SRUFillingMUDS_UserDirection";
            }
            else if (activeAlerts[alertIdx].CodeName === "SRUPurgingFluid")
            {
                return "T_Purging...";
            }
            else if (activeAlerts[alertIdx].CodeName === "SRUPurgingFactoryMUDS")
            {
                return "T_SRUPurgingFactoryMUDS_UserDirection";
            }
            else if ( (activeAlerts[alertIdx].CodeName === "SRUPrimingSUDS") ||
                      (activeAlerts[alertIdx].CodeName === "SRUPrimingMUDS") )
            {
                return "T_Priming...";
            }
			else if (activeAlerts[alertIdx].CodeName === "SRUCalibratingMUDS")
            {
                return "T_SRUCalibratingMUDS_UserDirection";
            }
            else if (activeAlerts[alertIdx].CodeName === "SRUAirCheckingMUDS")
            {
                return "T_SRUAirCheckingMUDS_UserDirection";
            }
        }

        return "";
    }
}
