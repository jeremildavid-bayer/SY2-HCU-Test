import QtQuick 2.12
import QtQuick.Controls 2.12
import "../Widgets"
import "../Widgets/Popup"

Popup {
    widthMin: dsCfgLocal.screenW * 0.75
    heightMin: dsCfgLocal.screenH * 0.7
    titleText: "Battery Workflow Test Mode"
    btnWidth: dsCfgLocal.screenW * 0.16
    okBtnText: "Start"
    cancelBtnText: "Abort"
    otherBtnText: "Close"
    translationRequired: false
    showOkBtn: false
    showCancelBtn: true
    showOtherBtn: false

    property var workflowBatteryStatus: dsWorkflow.workflowBatteryStatus
    property string workflowBatteryState:  "WORKFLOW_BATTERY_STATE_IDLE"
    property string statusText: ""
    property var bmsDigests: dsMcu.bmsDigests
    property var bmsDigest: bmsDigests[batteryIdx]
    property var powerStatus: dsMcu.powerStatus
    property string workflowMessage: ""
    property int batteryIdx: cmbBatterySelect.currentIndex

    property bool batteryWearMode: false

    content: [
        Item{
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
                    id: cmbTestAction
                    rowHeight: parent.height
                    width: cmbBatterySelect.width
                    optionList: [ { unit: "", icon: "", value: "Charge 2%"  },
                                  { unit: "", icon: "", value: "Charge 5%" },
                                  { unit: "", icon: "", value: "Full Charge" },
                                  { unit: "", icon: "", value: "Discharge 2%"  },
                                  { unit: "", icon: "", value: "Discharge 5%"  },
                                  { unit: "", icon: "", value: "Que AQD"  },
                                  { unit: "", icon: "", value: "Battery wear"  },
                                  { unit: "", icon: "", value: "Baseboard Test"  },
                                 ]
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
                border.color: "grey"
                color: "lightgrey"

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

                            return "AC: " + (powerStatus.IsAcPowered ? "Connected" : "Disconnected") +
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
        if (okBtnText == "Start")
        {
            if (workflowBatteryState === "WORKFLOW_BATTERY_STATE_IDLE")
            {
                cmbBatterySelect.readOnly = true;
                cmbTestAction.readOnly = true;
                var action = getTargetAction();

                if (action === "Que AQD")
                {
                    // not checking status since AQD process will check conditions
                    dsWorkflow.slotWorkflowBatteryAction(batteryIdx, action);
                    // close since AQD is background process
                    close();
                }
                else if (action === "Battery wear")
                {
                    // this is QML only action.
                    batteryWearMode = true;
                    dsWorkflow.slotWorkflowBatteryAction(batteryIdx, 95);
                }
                else
                {
                    var status = dsWorkflow.slotWorkflowBatteryAction(batteryIdx, action);

                    if ((status !== undefined) && (status.State !== "Waiting") && (status.State !== "Started"))
                    {
                        statusText = "Start Failed: " + status.State + "\n" + status.Err;
                        showCancelBtn = false;
                        showOkBtn = false;
                        showOtherBtn = true;
                    }
                }
            }
            else
            {
                statusText = "Start Failed: workflowBatteryState is invalid [" + workflowBatteryState + "]\n";
                showCancelBtn = false;
                showOkBtn = false;
                showOtherBtn = true;
            }
        }
        else
        {
            // code shouldn't reach here
        }
    }

    onBtnCancelClicked: {
        dsWorkflow.slotWorkflowBatteryAbort();
        batteryWearMode = false;
    }

    onBtnOtherClicked: {
        dsWorkflow.slotWorkflowBatteryAbort();
        batteryWearMode = false;
        close();
    }

    Component.onCompleted: {
        cmbBatterySelect.setCurrentIndex(0);
        cmbTestAction.setCurrentIndex(0);
    }

    onWorkflowBatteryStatusChanged: {
        if (!isOpen() || workflowBatteryStatus === undefined)
        {
            return;
        }
        if (workflowBatteryState !== workflowBatteryStatus.State)
        {
            workflowBatteryState = workflowBatteryStatus.State;
        }
        if (workflowMessage !== workflowBatteryStatus.Message)
        {
            workflowMessage = workflowBatteryStatus.Message;
        }

        reload();
    }

    function init() {
        showOkBtn = true;
        showOtherBtn = true;
        showCancelBtn = false;
        okBtnText = "Start";
        statusText = "Choose options and Click 'Start'";
        cmbBatterySelect.readOnly = false;
        cmbTestAction.readOnly = false;
        enableOkBtn = true;
    }

    function reload() {
        if (workflowBatteryState === "WORKFLOW_BATTERY_STATE_IDLE")
        {
            // due to how cpp state machine chages, qml will process IDLE state first which will cause cpp to reset internal variables after qml
            // check for workflowMessage = "" which is done inside IDLE state of cpp to ensure cpp's idle state is processed first
            // this is only for battery wear mode
            if (batteryWearMode && (workflowMessage === ""))
            {
                var curCharge = bmsDigests[batteryIdx].SbsStatus["RelativeStateOfCharge (%)"];
                if (curCharge >= 90)
                {
                    // if we go too low it may do QD!
                    dsWorkflow.slotWorkflowBatteryAction(batteryIdx, 15);
                }
                else
                {
                    dsWorkflow.slotWorkflowBatteryAction(batteryIdx, 95);
                }
            }
            else if (!batteryWearMode)
            {
                init();
            }
        }
        else if ((workflowBatteryState === "WORKFLOW_BATTERY_STATE_CHARGE_PREPARATION") ||
                 (workflowBatteryState === "WORKFLOW_BATTERY_STATE_CHARGE_PROGRESS") ||
                 (workflowBatteryState === "WORKFLOW_BATTERY_STATE_CHARGE_DONE") ||
                 (workflowBatteryState === "WORKFLOW_BATTERY_STATE_DISCHARGE_PREPARATION") ||
                 (workflowBatteryState === "WORKFLOW_BATTERY_STATE_DISCHARGE_PROGRESS") ||
                 (workflowBatteryState === "WORKFLOW_BATTERY_STATE_DISCHARGE_DONE") ||
                 (workflowBatteryState === "WORKFLOW_BATTERY_STATE_BASEBOARD_VOUTTEST_ON") ||
                 (workflowBatteryState === "WORKFLOW_BATTERY_STATE_BASEBOARD_VOUTTEST_OFF"))
        {
            showOkBtn = false;
            showOtherBtn = false;
            showCancelBtn = true;
            statusText = niceStateName(workflowBatteryState) + "\n" + workflowBatteryStatus.Message;
        }
        else
        {
            showOkBtn = false;
            showOtherBtn = false;
            showCancelBtn = true;
            statusText = niceStateName(workflowBatteryState) + " Is not handled!\nMessage : " + workflowBatteryState.Message;
        }

        if (batteryWearMode)
        {
            statusText = statusText + "\n BATTERY WEAR MODE";
        }
    }

    function getTargetAction()
    {
        var currentActionValue = cmbTestAction.getCurrentValue().value;
        if (currentActionValue === "Charge 2%")
        {
            return Math.min(100, bmsDigests[batteryIdx].SbsStatus["RelativeStateOfCharge (%)"] + 2);
        }
        else if (currentActionValue === "Charge 5%")
        {
            return Math.min(100, bmsDigests[batteryIdx].SbsStatus["RelativeStateOfCharge (%)"] + 5);
        }
        else if (currentActionValue === "Full Charge")
        {
            return 100; // TODO: 100 may not work!
        }
        else if (currentActionValue === "Discharge 2%")
        {
            return Math.max(5, (bmsDigests[batteryIdx].SbsStatus["RelativeStateOfCharge (%)"] - 2));
        }
        else if (currentActionValue === "Discharge 5%")
        {
            return Math.max(5, (bmsDigests[batteryIdx].SbsStatus["RelativeStateOfCharge (%)"] - 5));
        }
        else
        {
            return currentActionValue;
        }
    }

    function niceStateName(workflowBatteryState)
    {
        return workflowBatteryState.replace("WORKFLOW_BATTERY_STATE_", "");
    }
}
