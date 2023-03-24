import QtQuick 2.12
import QtQuick.Controls 2.12
import "../Widgets"
import "../Widgets/Popup"

Popup {
    property var workflowManualQualifiedDischargeStatus: dsWorkflow.workflowManualQualifiedDischargeStatus
    property string workflowManualQualifiedDischargeState:  "MANUAL_QUALIFIED_DISCHARGE_STATE_READY"
    property string workflowMessage: ""
    property string statusText: ""
    property bool isErrorState: false
    property var bmsDigests: dsMcu.bmsDigests
    property var powerStatus: dsMcu.powerStatus
    property int maxErrorBeforeStarted: 0
    property int batteryIdx: cmbBatterySelect.currentIndex
    property var bmsDigest: bmsDigests[batteryIdx]
    property var otherBatteryDigest: bmsDigests[(batteryIdx === 0) ? 1 : 0]
    property int dischargeTypeIdx: cmbDischargeType.currentIndex
    property string dischargeTypeMethod: cmbDischargeType.optionList[dischargeTypeIdx].value
    property int bmsMaxErrorLimitLow: dsCapabilities.bmsMaxErrorLimitLow

    widthMin: dsCfgLocal.screenW * 0.75
    heightMin: dsCfgLocal.screenH * 0.7
    titleText: "Manual Qualified Discharge"
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
                    id: cmbDischargeType
                    rowHeight: parent.height
                    width: cmbBatterySelect.width
                    optionList: [ { unit: "", icon: "", value: "Self Discharge" },
                                  { unit: "", icon: "", value: "External Load"  }]
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

                Text {
                    anchors.top: parent.top
                    anchors.topMargin: parent.height * 0.02
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.pixelSize: parent.height * 0.12
                    color: colorMap.warnText
                    horizontalAlignment: Text.AlignHCenter
                    text: (dischargeTypeMethod === "External Load") ? "Connect/Disconnect external load only when prompted" : ""
                }

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
                    id: rectBatteryDetails
                    anchors.top: rectStatus.bottom
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: parent.height * 0.18
                    width: parent.width * 0.9
                    anchors.horizontalCenter: parent.horizontalCenter
                    Text {
                        anchors.fill: parent
                        textFormat: Text.RichText
                        text: {
                            if (bmsDigest === undefined)
                            {
                                return "";
                            }

                            return "AC: " + (powerStatus.IsAcPowered ? "Connected" : "Disconnected") +
                                   "<br/>Current: " + bmsDigest.SbsStatus["Current (mA)"] + "mA" +
                                   ",  Charged: " + bmsDigest.SbsStatus["RelativeStateOfCharge (%)"] + "%" +
                                   "<br/>MaxError: " + bmsDigest.SbsStatus["MaxError (%)"] + "%" +
                                    (((workflowManualQualifiedDischargeState === "MANUAL_QUALIFIED_DISCHARGE_STATE_READY") && (bmsDigest.SbsStatus["MaxError (%)"] <= bmsMaxErrorLimitLow)) ? " : <font color=\"#FF0000\">Manual Qualified Discharge not needed</font>" : "") +
                                   "<br/>VDQ: " + bmsDigest.GaugingStatus.DischargeQualifiedForLearning +
                                   ",  EDV2: " + bmsDigest.GaugingStatus.EndOfDischargeVoltageLevel2 +
                                   ", CellOverVoltage: " + bmsDigest.SafetyStatus.CellOverVoltage;
                        }
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        wrapMode: Text.Wrap
                        font.pixelSize: parent.height * 0.5
                        color: "grey"
                    }
                }
            }

            Text {
                id: otherBatteryStatus
                anchors.top: statusBox.bottom
                anchors.right: statusBox.right
                height: parent.height * 0.1
                font.pixelSize: 20
                horizontalAlignment: Text.AlignBottom
                text: {
                    if (otherBatteryDigest === undefined)
                    {
                        return "";
                    }

                    return "Other Battery Current: " + otherBatteryDigest.SbsStatus["Current (mA)"] + "mA"
                }
            }
        }
    ]

    onOpened: {
        init();
    }

    onBtnOkClicked: {
        if (okBtnText == "Complete")
        {
            dsWorkflow.slotManualQualifiedDischargeResume();
        }
        else if (okBtnText == "Start")
        {
            if (workflowManualQualifiedDischargeState === "MANUAL_QUALIFIED_DISCHARGE_STATE_READY")
            {
                cmbBatterySelect.readOnly = true;
                cmbDischargeType.readOnly = true;
                maxErrorBeforeStarted = bmsDigest.SbsStatus["MaxError (%)"];

                logDebug("PopupManualQualifiedDischarge: Started with Battery[" + batteryIdx + "]: maxError=" + maxErrorBeforeStarted);
                var status = dsWorkflow.slotManualQualifiedDischargeStart(batteryIdx, dischargeTypeMethod);
                if ( (status.State !== "Waiting") &&
                        (status.State !== "Started") )
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
                // Start is requested but the MQD state is invalid.. what do we do here?
                statusText = "Start Failed: workflowManualQualifiedDischargeState is in invlaid state [" + workflowManualQualifiedDischargeState + "]\n";
                showCancelBtn = false;
                showOkBtn = false;
                showOtherBtn = true;
                isErrorState = true;
            }
        }
        else if (okBtnText == "Continue")
        {
            dsWorkflow.slotManualQualifiedDischargeResume();
        }
        else
        {
            // code shouldn't reach here
        }
    }

    onBtnCancelClicked: {
        dsWorkflow.slotManualQualifiedDischargeAbort();
    }

    onBtnOtherClicked: {
        dsWorkflow.slotManualQualifiedDischargeAbort();
        close();
    }

    Component.onCompleted: {
        cmbBatterySelect.setCurrentIndex(0);
        cmbDischargeType.setCurrentIndex(0);
    }

    onWorkflowManualQualifiedDischargeStatusChanged: {
        if (workflowManualQualifiedDischargeStatus === undefined)
        {
            return;
        }
        if (workflowManualQualifiedDischargeState !== workflowManualQualifiedDischargeStatus.State)
        {
            workflowManualQualifiedDischargeState = workflowManualQualifiedDischargeStatus.State;
        }
        if (workflowMessage !== workflowManualQualifiedDischargeStatus.Message)
        {
            workflowMessage = workflowManualQualifiedDischargeStatus.Message;
        }

        reload();
    }

    function reload() {
        if (workflowManualQualifiedDischargeState === "MANUAL_QUALIFIED_DISCHARGE_STATE_READY")
        {
            init();
        }
        else if (workflowManualQualifiedDischargeState === "MANUAL_QUALIFIED_DISCHARGE_STATE_USER_POWER_CONNECT_WAITING")
        {
            showOkBtn = true;
            enableOkBtn = Qt.binding(function() { return powerStatus.IsAcPowered; });
            showOtherBtn = false;
            showCancelBtn = true;
            okBtnText = "Continue";
            statusText = niceStateName(workflowManualQualifiedDischargeState) + "\nPlease Connect Power and Click 'Continue'";
        }
        else if ((workflowManualQualifiedDischargeState === "MANUAL_QUALIFIED_DISCHARGE_STATE_CHARGING_FULL_PROGRESS") ||
                 (workflowManualQualifiedDischargeState === "MANUAL_QUALIFIED_DISCHARGE_STATE_CHARGING_FULL_FAIL") ||
                 (workflowManualQualifiedDischargeState === "MANUAL_QUALIFIED_DISCHARGE_STATE_CHARGING_FULL_DONE") ||
                 (workflowManualQualifiedDischargeState === "MANUAL_QUALIFIED_DISCHARGE_STATE_DISCHARGING_PROGRESS") ||
                 (workflowManualQualifiedDischargeState === "MANUAL_QUALIFIED_DISCHARGE_STATE_DISCHARGING_FAIL") ||
                 (workflowManualQualifiedDischargeState === "MANUAL_QUALIFIED_DISCHARGE_STATE_DISCHARGING_DONE") ||
                 (workflowManualQualifiedDischargeState === "MANUAL_QUALIFIED_DISCHARGE_STATE_LOAD_DISCHARGING_PROGRESS") ||
                 (workflowManualQualifiedDischargeState === "MANUAL_QUALIFIED_DISCHARGE_STATE_LOAD_DISCHARGING_FAIL") ||
                 (workflowManualQualifiedDischargeState === "MANUAL_QUALIFIED_DISCHARGE_STATE_LOAD_DISCHARGING_DONE") )
        {
            showOkBtn = false;
            showOtherBtn = false;
            showCancelBtn = true;
            statusText = niceStateName(workflowManualQualifiedDischargeState) + "\n" + workflowMessage;
        }

        else if (workflowManualQualifiedDischargeState === "MANUAL_QUALIFIED_DISCHARGE_STATE_CONNECT_EXTERNAL_LOAD")
        {
            showOkBtn = true;
            enableOkBtn = true;
            showOtherBtn = false;
            showCancelBtn = true;
            okBtnText = "Continue";
            statusText = niceStateName(workflowManualQualifiedDischargeState) + "\n" + workflowMessage;
        }
        else if (workflowManualQualifiedDischargeState === "MANUAL_QUALIFIED_DISCHARGE_STATE_RECONNECT_BATTERY")
        {
            showOkBtn = true;
            enableOkBtn = true;
            okBtnText = "Continue"
            showOtherBtn = false;
            showCancelBtn = true;
            statusText = niceStateName(workflowManualQualifiedDischargeState) + "\n" + workflowMessage;
        }
        else if (workflowManualQualifiedDischargeState === "MANUAL_QUALIFIED_DISCHARGE_STATE_RECONNECT_BATTERY_CONFIRMED")
        {
            showOkBtn = false;
            enableOkBtn = false;
            showOtherBtn = false;
            showCancelBtn = true;
            statusText = niceStateName(workflowManualQualifiedDischargeState) + "\n" + workflowMessage;
        }
        else if (workflowManualQualifiedDischargeState === "MANUAL_QUALIFIED_DISCHARGE_STATE_DONE")
        {
            showOkBtn = true;
            enableOkBtn = true;
            showOtherBtn = false;
            showCancelBtn = false;
            okBtnText = "Complete";
            statusText = niceStateName(workflowManualQualifiedDischargeState) + "\n" + workflowMessage;
        }
        else if (workflowManualQualifiedDischargeState === "MANUAL_QUALIFIED_DISCHARGE_STATE_FAILED")
        {
            showCancelBtn = false;
            showOkBtn = false;
            showOtherBtn = true;
            isErrorState = true;
            statusText = niceStateName(workflowManualQualifiedDischargeState) + "\n" + workflowMessage;
        }
    }

    function init()
    {
        showOkBtn = true;
        showOtherBtn = true;
        showCancelBtn = false;
        okBtnText = "Start";
        statusText = "Select Battery Type and Click 'Start'";
        cmbBatterySelect.readOnly = false;
        cmbDischargeType.readOnly = false;
        isErrorState = false;
        enableOkBtn = true;
    }

    function niceStateName(state)
    {
        return state.replace("MANUAL_QUALIFIED_DISCHARGE_STATE_", "");
    }
}
