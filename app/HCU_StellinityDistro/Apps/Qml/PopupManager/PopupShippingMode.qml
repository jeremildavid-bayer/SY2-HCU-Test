import QtQuick 2.12
import QtQuick.Controls 2.12
import "../Widgets"
import "../Widgets/Popup"

Popup {
    property var workflowShippingModeStatus: dsWorkflow.workflowShippingModeStatus
    property string workflowShippingModeState: "SHIPPING_MODE_STATE_READY"
    property string workflowMessage: ""
    property int bmsShippingModeAirTargetChargedLevel: dsCapabilities.bmsShippingModeAirTargetChargedLevel
    property int bmsShippingModeLandTargetChargedLevel: dsCapabilities.bmsShippingModeLandTargetChargedLevel
    property int bmsShippingModeSeaTargetChargedLevel: dsCapabilities.bmsShippingModeSeaTargetChargedLevel
    property int bmsShippingModeTargetChargedLevel: {
        var methodType = cmbMethodType.getCurrentValue().value;
        if (methodType === "Land") {
            return bmsShippingModeLandTargetChargedLevel;
        }
        else if (methodType === "Sea") {
            return bmsShippingModeSeaTargetChargedLevel;
        }
        else
            return bmsShippingModeAirTargetChargedLevel;
    }

    property string statusText: ""
    property bool isErrorState: false
    property var bmsDigests: dsMcu.bmsDigests
    property var bmsDigest: bmsDigests[batteryIdx]
    property var powerStatus: dsMcu.powerStatus
    property int batteryIdx: cmbBatterySelect.currentIndex

    widthMin: dsCfgLocal.screenW * 0.75
    heightMin: dsCfgLocal.screenH * 0.7
    titleText: "Shipping Mode"
    btnWidth: dsCfgLocal.screenW * 0.16
    okBtnText: "Start"
    cancelBtnText: "Abort"
    otherBtnText: "Close"
    translationRequired: false
    showOkBtn: false
    showCancelBtn: true
    showOtherBtn: false

    content: [
        Item {
            anchors.fill: parent

            Row {
                id: rectOptions
                z: statusBox.z + 1
                anchors.top: parent.top
                anchors.topMargin: parent.height * 0.06
                anchors.horizontalCenter: parent.horizontalCenter
                height: parent.height * 0.15
                width: content.width
                spacing: parent.width * 0.05

                GenericComboBox {
                    id: cmbBatterySelect
                    rowHeight: parent.height
                    width: rectOptions.parent.width * 0.3
                    optionList: [ { unit: "", icon: "", value: "Battery A" },
                                  { unit: "", icon: "", value: "Battery B" }]
                }

                GenericComboBox {
                    id: cmbMethodType
                    rowHeight: parent.height
                    width: cmbBatterySelect.width
                    optionList: [ { unit: "", icon: "", value: "Air"  },
                                  { unit: "", icon: "", value: "Land" },
                                  { unit: "", icon: "", value: "Sea"  } ]
                }
            }

            Rectangle {
                id: statusBox
                anchors.top: rectOptions.bottom
                anchors.topMargin: parent.height * 0.1
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width * 0.8
                anchors.bottom: parent.bottom
                anchors.bottomMargin: parent.height * 0.1
                radius: 10
                border.width: 3
                border.color: isErrorState ? "red" : "grey"
                color: isErrorState ? "orange" : "lightgrey"

                Item {
                    id: rectStatus
                    anchors.top: parent.top
                    height: parent.height * 0.67
                    width: parent.width * 0.9
                    anchors.horizontalCenter: parent.horizontalCenter
                    Text {
                        anchors.fill: parent
                        text: statusText
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        wrapMode: Text.Wrap
                        font.pixelSize: parent.height * 0.16
                    }
                }

                Item {
                    id: rectStatus2
                    anchors.top: rectStatus.bottom
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: parent.height * 0.18
                    width: parent.width * 0.9
                    anchors.horizontalCenter: parent.horizontalCenter
                    Text {
                        anchors.fill: parent
                        text: {
                            if (bmsDigest === undefined)
                            {
                                return "";
                            }
                            else if (workflowShippingModeState === "SHIPPING_MODE_STATE_USER_REMOVE_BATTERY_WAITING")
                            {
                                return "AC: " + (powerStatus.IsAcPowered ? "Connected" : "Disconnected") +
                                        "\nTarget: " + bmsShippingModeTargetChargedLevel + "%" +
                                        "\n BMS information not available during this state";
                            }

                            return "AC: " + (powerStatus.IsAcPowered ? "Connected" : "Disconnected") +
                                   "\nTarget: " + bmsShippingModeTargetChargedLevel + "%" +
                                   "\nA: Current: " + bmsDigests[0].SbsStatus["Current (mA)"] + "mA" + ",  Charged: " + bmsDigests[0].SbsStatus["RelativeStateOfCharge (%)"] + "%,  FET Enabled: " + bmsDigests[0].ManufacturingStatus.AllFetAction +
                                   "\nB: Current: " + bmsDigests[1].SbsStatus["Current (mA)"] + "mA" + ",  Charged: " + bmsDigests[1].SbsStatus["RelativeStateOfCharge (%)"] + "%,  FET Enabled: " + bmsDigests[1].ManufacturingStatus.AllFetAction;
                        }
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        wrapMode: Text.Wrap
                        font.pixelSize: parent.height * 0.5
                        color: "grey"
                    }
                }
            }
        }
    ]

    onOpened: {
        init();
    }

    onBtnOkClicked: {
        if (workflowShippingModeState === "SHIPPING_MODE_STATE_READY")
        {
            cmbBatterySelect.readOnly = true;
            cmbMethodType.readOnly = true;
            var methodType = cmbMethodType.getCurrentValue().value;

            logDebug("PopupShippingMode: Started with Battery[" + batteryIdx + "]: method=" + methodType, + "(" + bmsShippingModeTargetChargedLevel  + ")");

            var status = dsWorkflow.slotShippingModeStart(batteryIdx, bmsShippingModeTargetChargedLevel);
            if ( (status.State !== "Waiting") && (status.State !== "Started") )
            {
                statusText = "Start Failed: " + status.State + "\n" + status.Err;
                showCancelBtn = false;
                showOkBtn = false;
                showOtherBtn = true;
                isErrorState = true;
            }
        }
        else
        {
            dsWorkflow.slotShippingModeResume();
        }
    }

    onBtnCancelClicked: {
        dsWorkflow.slotShippingModeAbort();
    }

    onBtnOtherClicked: {
        if (workflowShippingModeState !== "SHIPPING_MODE_STATE_READY")
        {
            dsWorkflow.slotShippingModeAbort();
        }
        close();
    }

    Component.onCompleted: {
        cmbBatterySelect.setCurrentIndex(0);
        cmbMethodType.setCurrentIndex(0);
    }

    onWorkflowShippingModeStatusChanged: {
        if (!isOpen() || workflowShippingModeStatus === undefined)
        {
            return;
        }
        if (workflowShippingModeState !== workflowShippingModeStatus.State)
        {
            workflowShippingModeState = workflowShippingModeStatus.State;
        }
        if (workflowMessage !== workflowShippingModeStatus.Message)
        {
            workflowMessage = workflowShippingModeStatus.Message;
        }

        reload();
    }

    function reload() {
        if (workflowShippingModeState === "SHIPPING_MODE_STATE_READY")
        {
            init();
        }

        else if (workflowShippingModeState === "SHIPPING_MODE_STATE_USER_POWER_CONNECT_WAITING")
        {
            showOkBtn = true;
            enableOkBtn = Qt.binding(function() { return powerStatus.IsAcPowered; });
            showOtherBtn = false;
            showCancelBtn = true;
            okBtnText = "Continue";
            statusText = niceStateName(workflowShippingModeState) + "\nPlease Connect Power and Click 'Continue'";
        }
        else if (workflowShippingModeState === "SHIPPING_MODE_STATE_CHARGING_DISCHARGING_PROGRESS")
        {
            showOkBtn = false;
            showOtherBtn = false;
            showCancelBtn = true;
            statusText = niceStateName(workflowShippingModeState) + "\n" + workflowMessage;

        }
        else if (workflowShippingModeState === "SHIPPING_MODE_STATE_CHARGING_DISCHARGING_DONE")
        {
            showOkBtn = false;
            showOtherBtn = false;
            showCancelBtn = true;
            statusText = niceStateName(workflowShippingModeState) + "\n" + workflowMessage;

        }
        else if (workflowShippingModeState === "SHIPPING_MODE_STATE_CHARGING_DISCHARGING_FAIL")
        {
            showOkBtn = false;
            showOtherBtn = false;
            showCancelBtn = true;
            statusText = niceStateName(workflowShippingModeState) + "\n" + workflowMessage;

        }
        else if (workflowShippingModeState === "SHIPPING_MODE_STATE_FAILED")
        {
            showCancelBtn = false;
            showOkBtn = false;
            showOtherBtn = true;
            isErrorState = true;
            statusText = niceStateName(workflowShippingModeState) + "\n" + workflowMessage;
        }


        else if (workflowShippingModeState === "SHIPPING_MODE_STATE_SET_SLEEP_MODE")
        {
            showOkBtn = false;
            showOtherBtn = false;
            showCancelBtn = false;
            statusText = "Sending Shutdown Mode to BMS..";
        }
        else if (workflowShippingModeState === "SHIPPING_MODE_STATE_USER_REMOVE_BATTERY_WAITING")
        {
            showOkBtn = true;
            enableOkBtn = true;
            showOtherBtn = false;
            showCancelBtn = false;
            okBtnText = "Continue";
            statusText = "Battery is ready to be shipped.\nPlease Remove Battery and Click 'Continue'";

            // play sound to notify user
            function soundLoop() {
                soundPlayer.playDisarmStop();
                timerSingleShot(2000, function() {
                    if (workflowShippingModeState === "SHIPPING_MODE_STATE_USER_REMOVE_BATTERY_WAITING")
                    {
                        soundLoop();
                    }
                });
            }
            soundLoop();
        }
        else if (workflowShippingModeState === "SHIPPING_MODE_STATE_ABORTED")
        {
            isErrorState = true;
        }
        else if (workflowShippingModeState === "SHIPPING_MODE_STATE_DONE")
        {
            showOkBtn = true;
            enableOkBtn = true;
            showOtherBtn = false;
            showCancelBtn = false;
            okBtnText = "Complete";
            statusText = niceStateName(workflowShippingModeState) + "\n" + workflowMessage;
        }
    }

    function init()
    {
        showOkBtn = true;
        showOtherBtn = true;
        showCancelBtn = false;
        okBtnText = "Start";
        statusText = "Click 'Start'";
        cmbBatterySelect.readOnly = false;
        cmbMethodType.readOnly = false;
        isErrorState = false;
        enableOkBtn = true;
    }

    function niceStateName(workflowShippingModeState)
    {
        return workflowShippingModeState.replace("SHIPPING_MODE_STATE_", "");
    }
}
