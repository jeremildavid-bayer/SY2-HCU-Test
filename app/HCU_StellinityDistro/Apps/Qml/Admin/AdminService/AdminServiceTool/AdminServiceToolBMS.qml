import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQml 2.15
import "../../../Widgets"
import "../../../Util.js" as Util

AdminServiceToolTestTemplate {
    property var bmsDigests: []
    Binding on bmsDigests {
        when: visible
        value: dsMcu.bmsDigests
        restoreMode: Binding.RestoreBinding
    }

    property var powerStatus: dsMcu.powerStatus
    property int bmsMaxErrorLimitLow: dsCapabilities.bmsMaxErrorLimitLow
    property int bmsTemperatureLimitLow: dsCapabilities.bmsTemperatureLimitLow
    property int bmsTemperatureLimitHigh: dsCapabilities.bmsTemperatureLimitHigh
    property int bmsCycleCountLimit: dsCapabilities.bmsCycleCountLimit
    property int bmsStateOfHealthLowLimit: dsCapabilities.bmsStateOfHealthLowLimit
    property bool batteryTestModeEnabled: dsCapabilities.bmsEnableBatteryTestMode
    property var bmsDeviceInfoList: []
    property var bmsSbsStatusList: []
    property var bmsSafetyStatusList: []
    property var bmsPfStatusList: []
    property var bmsOperationStatusList: []
    property var bmsChargingStatusList: []
    property var bmsGaugingStatusList: []
    property var bmsManufacturingStatusList: []

    property var workflowAutomaticQualifiedDischargeStatus: dsWorkflow.workflowAutomaticQualifiedDischargeStatus
    property bool showCancelAqdButton: ((workflowAutomaticQualifiedDischargeStatus !== undefined) && (
                                        workflowAutomaticQualifiedDischargeStatus.State === "AQD_STATE_AQD_QUEUED" ||
                                        workflowAutomaticQualifiedDischargeStatus.State === "AQD_STATE_START" ||
                                        workflowAutomaticQualifiedDischargeStatus.State === "AQD_STATE_DISCHARGE_PROGRESS" ||
                                        workflowAutomaticQualifiedDischargeStatus.State === "AQD_STATE_DISCHARGE_DONE"))

    property string bmsDeviceInfoListUpdatedAt: ""
    property string bmsSbsStatusListUpdatedAt: ""
    property string bmsSafetyStatusListUpdatedAt: ""
    property string bmsPfStatusListUpdatedAt: ""
    property string bmsOperationStatusListUpdatedAt: ""
    property string bmsChargingStatusListUpdatedAt: ""
    property string bmsGaugingStatusListUpdatedAt: ""
    property string bmsManufacturingStatusListUpdatedAt: ""

    paramsTableRowWidth: width * 0.65
    paramsTableRowHeight: height * 0.08
    titleWidthPercent: 0.62
    activeScreenState: "Admin-Service-Tool-BMS"
    btnStart.visible: false
    consoleVisible: false

    paramsTable: [
        AdminServiceToolTestTableRow {
            titleText: "General"
            controlType: "HEADING"
        },

        AdminServiceToolTestTableRow {
            titleText: "AC Connected"
            valueText: {
                if (powerStatus === undefined)
                {
                    return "";
                }
                return powerStatus.IsAcPowered.toString().toUpperCase();
            }
            controlType: "TEXT"
        },

        AdminServiceToolTestTableRow {
            titleText: "Battery Level"
            valueText: {
                if (powerStatus === undefined)
                {
                    return "";
                }
                return powerStatus.BatteryLevel.toUpperCase();
            }
            controlType: "TEXT"
        },

        AdminServiceToolTestTableRow {
            titleText: "Device Info"
            controlType: "HEADING"
        },

        AdminServiceToolTestTableRow {
            titleText: "Updated At"
            controlType: "TEXT"
            valueText: bmsDeviceInfoListUpdatedAt
        },

        AdminServiceToolTestTableRow {
            titleText: "HW Version"
            controlType: "OBJECT"
            customContent: [
                AdminServiceToolBMSTableCell {
                    val1: (bmsDigests.length === 0) ? "--" : bmsDigests[0].HardwareVersion
                    val2: (bmsDigests.length === 0) ? "--" : bmsDigests[1].HardwareVersion
                    color1: colorMap.text01
                    color2: colorMap.text01

                    function getCellColor(name, val)
                    {
                        return colorMap.text01;
                    }
                }
            ]
        },

        Repeater {
            model: bmsDeviceInfoList
            delegate: AdminServiceToolTestTableRow {
                titleText: bmsDeviceInfoList[index].Name
                controlType: "OBJECT"
                customContent: [
                    AdminServiceToolBMSTableCell {
                        val1: bmsDeviceInfoList[index].Val1
                        val2: bmsDeviceInfoList[index].Val2
                        color1: getCellColor(bmsDeviceInfoList[index].Name, val1)
                        color2: getCellColor(bmsDeviceInfoList[index].Name, val2)

                        function getCellColor(name, val)
                        {
                            return colorMap.text01;
                        }
                    }
                ]
            }
        },

        AdminServiceToolTestTableRow {
            titleText: "SBS"
            controlType: "HEADING"
        },

        AdminServiceToolTestTableRow {
            titleText: "Updated At"
            controlType: "TEXT"
            valueText: bmsSbsStatusListUpdatedAt
        },

        Repeater {
            model: bmsSbsStatusList
            delegate: AdminServiceToolTestTableRow {
                titleText: bmsSbsStatusList[index].Name
                controlType: "OBJECT"
                customContent: [
                    AdminServiceToolBMSTableCell {
                        val1: bmsSbsStatusList[index].Val1
                        val2: bmsSbsStatusList[index].Val2
                        color1: getCellColor(bmsSbsStatusList[index].Name, val1)
                        color2: getCellColor(bmsSbsStatusList[index].Name, val2)

                        function getCellColor(name, val)
                        {
                            var valInt = parseInt(val);

                            if (name.indexOf("Temperature") >= 0)
                            {
                                return ((valInt > bmsTemperatureLimitLow) && (valInt < bmsTemperatureLimitHigh)) ? colorMap.text01 : "red";
                            }
                            else if (name.indexOf("MaxError") >= 0)
                            {
                                return (valInt <= bmsMaxErrorLimitLow) ? colorMap.text01 : "red";
                            }
                            else if (name.indexOf("Current") >= 0)
                            {
                                if (valInt === 0)
                                {
                                    return colorMap.text01;
                                }
                                else if (valInt > 0)
                                {
                                    return "blue";
                                }
                                else
                                {
                                    return "orange";
                                }
                            }
                            else if (name.indexOf("RelativeStateOfCharge") >= 0)
                            {
                                if (valInt < 10)
                                {
                                    return "red";
                                }
                                else if (valInt > 30)
                                {
                                    return "green";
                                }
                                else
                                {
                                    return "orange";
                                }
                            }
                            else if (name.indexOf("CellVoltage") >= 0)
                            {
                                return ((valInt > 2600) && (valInt < 3600)) ? colorMap.text01 : "red";
                            }
                            else if (name.indexOf("StateOfHealth") >= 0)
                            {
                                return valInt >= 80 ? colorMap.text01 : "red";
                            }
                            else
                            {
                                return colorMap.text01;
                            }
                        }
                    }
                ]
            }
        },

        AdminServiceToolTestTableRow {
            titleText: (bmsDigests.length === 0) ? "" : "Safety Status: A(" + bmsDigests[0].SafetyStatusBytes + "), B(" + bmsDigests[1].SafetyStatusBytes + ")"
            controlType: "HEADING"
        },

        AdminServiceToolTestTableRow {
            titleText: "Updated At"
            controlType: "TEXT"
            valueText: bmsSafetyStatusListUpdatedAt
        },

        Repeater {
            model: bmsSafetyStatusList
            delegate: AdminServiceToolTestTableRow {
                titleText: bmsSafetyStatusList[index].Name
                controlType: "OBJECT"
                customContent: [
                    AdminServiceToolBMSTableCell {
                        val1: bmsSafetyStatusList[index].Val1
                        val2: bmsSafetyStatusList[index].Val2
                        color1: getCellColor(bmsSafetyStatusList[index].Name, val1)
                        color2: getCellColor(bmsSafetyStatusList[index].Name, val2)

                        function getCellColor(name, val)
                        {
                            return (val !== "TRUE") ? colorMap.text01 : colorMap.red;
                        }
                    }
                ]
            }
        },

        AdminServiceToolTestTableRow {
            titleText: (bmsDigests.length === 0) ? "" : "Permanent Failure Status: A(" + bmsDigests[0].PfStatusBytes + "), B(" + bmsDigests[1].PfStatusBytes + ")"
            controlType: "HEADING"
        },

        AdminServiceToolTestTableRow {
            titleText: "Updated At"
            controlType: "TEXT"
            valueText: bmsPfStatusListUpdatedAt
        },

        Repeater {
            model: bmsPfStatusList
            delegate: AdminServiceToolTestTableRow {
                titleText: bmsPfStatusList[index].Name
                controlType: "OBJECT"
                customContent: [
                    AdminServiceToolBMSTableCell {
                        val1: bmsPfStatusList[index].Val1
                        val2: bmsPfStatusList[index].Val2
                        color1: getCellColor(bmsPfStatusList[index].Name, val1)
                        color2: getCellColor(bmsPfStatusList[index].Name, val2)

                        function getCellColor(name, val)
                        {
                            return (val !== "TRUE") ? colorMap.text01 : colorMap.red;
                        }
                    }
                ]
            }
        },

        AdminServiceToolTestTableRow {
            titleText: (bmsDigests.length === 0) ? "" : "Operation Status: A(" + bmsDigests[0].OperationStatusBytes + "), B(" + bmsDigests[1].OperationStatusBytes + ")"
            controlType: "HEADING"
        },

        AdminServiceToolTestTableRow {
            titleText: "Updated At"
            controlType: "TEXT"
            valueText: bmsOperationStatusListUpdatedAt
        },

        Repeater {
            model: bmsOperationStatusList
            delegate: AdminServiceToolTestTableRow {
                titleText: bmsOperationStatusList[index].Name
                controlType: "OBJECT"
                customContent: [
                    AdminServiceToolBMSTableCell {
                        val1: bmsOperationStatusList[index].Val1
                        val2: bmsOperationStatusList[index].Val2
                        color1: getCellColor(bmsOperationStatusList[index].Name, val1)
                        color2: getCellColor(bmsOperationStatusList[index].Name, val2)

                        function getCellColor(name, val)
                        {
                            var valInt = parseInt(val);

                            if ( (name.indexOf("CellBalancingStatus") >= 0) ||
                                 (name.indexOf("CcMeasurementInSleepMode") >= 0) ||
                                 (name.indexOf("AdcMeasurementInSleepMode") >= 0) ||
                                 (name.indexOf("InitializationAfterFullReset") >= 0) ||
                                 (name.indexOf("SleepMode") >= 0) ||
                                 (name.indexOf("SafePinActive") >= 0) ||
                                 (name.indexOf("IsUnderHostFETControl") >= 0) ||
                                 (name.indexOf("PrechargeFETActive") >= 0) )
                            {
                                return (val === "TRUE") ? "blue" : colorMap.text01;
                            }
                            else if ( (name.indexOf("DsgFETActive") >= 0) ||
                                      (name.indexOf("ChgFETActive") >= 0) )
                            {
                                return (val === "TRUE") ? "green" : "red";
                            }
                            else if (name.indexOf("SystemPresentLowActive") >= 0)
                            {
                                return (val === "TRUE") ? colorMap.text01 : "red";
                            }
                            else if (name.indexOf("SecurityMode") >= 0)
                            {
                                if (val === "UNSEALED")
                                {
                                    return "orange";
                                }
                                else if (val === "FULL_ACCESS")
                                {
                                    return "green";
                                }
                                return "red";
                            }

                            return (val !== "TRUE") ? colorMap.text01 : colorMap.red;
                        }
                    }
                ]
            }
        },

        AdminServiceToolTestTableRow {
            titleText: (bmsDigests.length === 0) ? "" : "Charging Status: A(" + bmsDigests[0].ChargingStatusBytes + "), B(" + bmsDigests[1].ChargingStatusBytes + ")"
            controlType: "HEADING"
        },

        AdminServiceToolTestTableRow {
            titleText: "Updated At"
            controlType: "TEXT"
            valueText: bmsChargingStatusListUpdatedAt
        },

        Repeater {
            model: bmsChargingStatusList
            delegate: AdminServiceToolTestTableRow {
                titleText: bmsChargingStatusList[index].Name
                controlType: "OBJECT"
                customContent: [
                    AdminServiceToolBMSTableCell {
                        val1: bmsChargingStatusList[index].Val1
                        val2: bmsChargingStatusList[index].Val2
                        color1: getCellColor(bmsChargingStatusList[index].Name, val1)
                        color2: getCellColor(bmsChargingStatusList[index].Name, val2)

                        function getCellColor(name, val)
                        {
                            var valInt = parseInt(val);

                            if ( (name.indexOf("StandardTemperatureRegion") >= 0) ||
                                 (name.indexOf("LowTemperatureRegion") >= 0) )
                            {
                                return (val === "TRUE") ? "green" : colorMap.text01;
                            }
                            else if (name.indexOf("ChargeTermination") >= 0)
                            {
                                return (val === "TRUE") ? "blue" : colorMap.text01;
                            }

                            return (val !== "TRUE") ? colorMap.text01 : colorMap.red;
                        }
                    }
                ]
            }
        },

        AdminServiceToolTestTableRow {
            titleText: (bmsDigests.length === 0) ? "" : "Gauging Status: A(" + bmsDigests[0].GaugingStatusBytes + "), B(" + bmsDigests[1].GaugingStatusBytes + ")"
            controlType: "HEADING"
        },

        AdminServiceToolTestTableRow {
            titleText: "Updated At"
            controlType: "TEXT"
            valueText: bmsGaugingStatusListUpdatedAt
        },

        Repeater {
            model: bmsGaugingStatusList
            delegate: AdminServiceToolTestTableRow {
                titleText: bmsGaugingStatusList[index].Name
                controlType: "OBJECT"
                customContent: [
                    AdminServiceToolBMSTableCell {
                        val1: bmsGaugingStatusList[index].Val1
                        val2: bmsGaugingStatusList[index].Val2
                        color1: getCellColor(bmsGaugingStatusList[index].Name, val1)
                        color2: getCellColor(bmsGaugingStatusList[index].Name, val2)

                        function getCellColor(name, val)
                        {
                            var valInt = parseInt(val);

                            if (name.indexOf("DischargeQualifiedForLearning") >= 0)
                            {
                                return (val === "TRUE") ? "green" : "orange";
                            }
                            else if ( (name.indexOf("OcvReadingTaken") >= 0) ||
                                      (name.indexOf("CellBalancingPossible") >= 0) ||
                                      (name.indexOf("FullyCharged") >= 0) )
                            {
                                return (val === "TRUE") ? "blue" : colorMap.text01;
                            }
                            else if (name.indexOf("DischargeDetected") >= 0)
                            {
                                return colorMap.text01;
                            }

                            return (val !== "TRUE") ? colorMap.text01 : colorMap.red;
                        }
                    }
                ]
            }
        },

        AdminServiceToolTestTableRow {
            titleText: (bmsDigests.length === 0) ? "" : "Manufacturing Status: A(" + bmsDigests[0].ManufacturingStatusBytes + "), B(" + bmsDigests[1].ManufacturingStatusBytes + ")"
            controlType: "HEADING"
        },

        AdminServiceToolTestTableRow {
            titleText: "Updated At"
            controlType: "TEXT"
            valueText: bmsManufacturingStatusListUpdatedAt
        },

        Repeater {
            model: bmsManufacturingStatusList
            delegate: AdminServiceToolTestTableRow {
                titleText: bmsManufacturingStatusList[index].Name
                controlType: "OBJECT"
                customContent: [
                    AdminServiceToolBMSTableCell {
                        val1: bmsManufacturingStatusList[index].Val1
                        val2: bmsManufacturingStatusList[index].Val2
                        color1: getCellColor(bmsManufacturingStatusList[index].Name, val1)
                        color2: getCellColor(bmsManufacturingStatusList[index].Name, val2)


                        function getCellColor(name, val)
                        {
                            var valInt = parseInt(val);

                            if (name.indexOf("LedDisplay") >= 0)
                            {
                                return colorMap.text01;
                            }
                            else if ( (name.indexOf("SafeAction") >= 0) ||
                                      (name.indexOf("BlackBoxReorder") >= 0) ||
                                      (name.indexOf("PermanentFailure") >= 0) ||
                                      (name.indexOf("LifetimeDataCollection") >= 0) ||
                                      (name.indexOf("AllFetAction") >= 0) )

                            {
                                return (val === "TRUE") ? "green" : colorMap.red;
                            }

                            return (val !== "TRUE") ? colorMap.text01 : colorMap.red;
                        }
                    }
                ]
            }
        }
    ]

    GenericIconButton {
        id: btnStartManualQualifyDischarge
        anchors.right: parent.right
        anchors.rightMargin: parent.width * 0.04
        width: parent.width * 0.25
        height: parent.height * 0.2
        iconText: "Start\nManual Qualified\nDischarge"
        iconFontPixelSize: height * 0.18
        iconColor: colorMap.white01
        color: colorMap.gry01
        onBtnClicked: {
            popupManager.popupManualQualifiedDischarge.open();
        }
    }

    GenericIconButton {
        id: btnStartShippingMode
        anchors.top: btnStartManualQualifyDischarge.bottom
        anchors.topMargin: parent.height * 0.05
        anchors.right: parent.right
        anchors.rightMargin: parent.width * 0.04
        width: parent.width * 0.25
        height: parent.height * 0.2
        iconText: "Start\nShipping Mode"
        iconFontPixelSize: height * 0.18
        iconColor: colorMap.white01
        color: colorMap.gry01
        onBtnClicked: {
            popupManager.popupShippingMode.open();
        }
    }

    GenericIconButton {
        id: btnBatteryTestMode
        anchors.top: btnStartShippingMode.bottom
        anchors.topMargin: parent.height * 0.05
        anchors.right: parent.right
        anchors.rightMargin: parent.width * 0.04
        width: parent.width * 0.25
        height: parent.height * 0.2
        iconText: "Battery Test Mode"
        iconFontPixelSize: height * 0.18
        iconColor: colorMap.white01
        color: colorMap.gry01
        visible: (showCancelAqdButton || batteryTestModeEnabled)
        onBtnClicked: {
            if (iconText === "Battery Test Mode")
            {
                popupManager.popupBatteryTestMode.open();
            }
            else
            {
                dsWorkflow.slotCancelAutomaticQualifiedDischarge();
            }
        }
    }

    onWorkflowAutomaticQualifiedDischargeStatusChanged: {
        if ((workflowAutomaticQualifiedDischargeStatus === undefined) || (workflowAutomaticQualifiedDischargeStatus.State === "AQD_STATE_IDLE"))
        {
            btnBatteryTestMode.iconText = "Battery Test Mode";
            btnStartManualQualifyDischarge.enabled = true;
            btnStartShippingMode.enabled = true;
        }
        else
        {
            btnBatteryTestMode.iconText = workflowAutomaticQualifiedDischargeStatus.Message + "\nCancel AQD";
            btnStartManualQualifyDischarge.enabled = false;
            btnStartShippingMode.enabled = false;
        }
    }

    onBmsDigestsChanged: {
        var newBmsDeviceInfoList = [], newBmsSbsStatusList = [], newBmsSafetyStatusList = [], newBmsPfStatusList = [], newBmsOperationStatusList = [], newBmsChargingStatusList = [], newBmsGaugingStatusList = [], newBmsManufacturingStatusList = [];

        if ( (bmsDigests !== undefined) &&
             (bmsDigests.length > 0) )
        {
            newBmsDeviceInfoList = getParamList(bmsDigests[0].DeviceInfo, bmsDigests[1].DeviceInfo);
            newBmsSbsStatusList = getParamList(bmsDigests[0].SbsStatus, bmsDigests[1].SbsStatus);
            newBmsSafetyStatusList = getParamList(bmsDigests[0].SafetyStatus, bmsDigests[1].SafetyStatus);
            newBmsPfStatusList = getParamList(bmsDigests[0].PfStatus, bmsDigests[1].PfStatus);
            newBmsOperationStatusList = getParamList(bmsDigests[0].OperationStatus, bmsDigests[1].OperationStatus);
            newBmsChargingStatusList = getParamList(bmsDigests[0].ChargingStatus, bmsDigests[1].ChargingStatus);
            newBmsGaugingStatusList = getParamList(bmsDigests[0].GaugingStatus, bmsDigests[1].GaugingStatus);
            newBmsManufacturingStatusList = getParamList(bmsDigests[0].ManufacturingStatus, bmsDigests[1].ManufacturingStatus);
        }

        if (!Util.compareObjects(bmsDeviceInfoList, newBmsDeviceInfoList))
        {
            bmsDeviceInfoList = newBmsDeviceInfoList;
            bmsDeviceInfoListUpdatedAt = Util.getCurrentDateTimeStr("yyyy/MM/dd", "hh:mm:ss", currentUtcOffsetMinutes);
        }

        if (!Util.compareObjects(bmsSbsStatusList, newBmsSbsStatusList))
        {
            bmsSbsStatusList = newBmsSbsStatusList;
            bmsSbsStatusListUpdatedAt = Util.getCurrentDateTimeStr("yyyy/MM/dd", "hh:mm:ss", currentUtcOffsetMinutes);
        }

        if (!Util.compareObjects(bmsSafetyStatusList, newBmsSafetyStatusList))
        {
            bmsSafetyStatusList = newBmsSafetyStatusList;
            bmsSafetyStatusListUpdatedAt = Util.getCurrentDateTimeStr("yyyy/MM/dd", "hh:mm:ss", currentUtcOffsetMinutes);
        }

        if (!Util.compareObjects(bmsPfStatusList, newBmsPfStatusList))
        {
            bmsPfStatusList = newBmsPfStatusList;
            bmsPfStatusListUpdatedAt = Util.getCurrentDateTimeStr("yyyy/MM/dd", "hh:mm:ss", currentUtcOffsetMinutes);
        }

        if (!Util.compareObjects(bmsOperationStatusList, newBmsOperationStatusList))
        {
            bmsOperationStatusList = newBmsOperationStatusList;
            bmsOperationStatusListUpdatedAt = Util.getCurrentDateTimeStr("yyyy/MM/dd", "hh:mm:ss", currentUtcOffsetMinutes);
        }

        if (!Util.compareObjects(bmsChargingStatusList, newBmsChargingStatusList))
        {
            bmsChargingStatusList = newBmsChargingStatusList;
            bmsChargingStatusListUpdatedAt = Util.getCurrentDateTimeStr("yyyy/MM/dd", "hh:mm:ss", currentUtcOffsetMinutes);
        }

        if (!Util.compareObjects(bmsGaugingStatusList, newBmsGaugingStatusList))
        {
            bmsGaugingStatusList = newBmsGaugingStatusList;
            bmsGaugingStatusListUpdatedAt = Util.getCurrentDateTimeStr("yyyy/MM/dd", "hh:mm:ss", currentUtcOffsetMinutes);
        }

        if (!Util.compareObjects(bmsManufacturingStatusList, newBmsManufacturingStatusList))
        {
            bmsManufacturingStatusList = newBmsManufacturingStatusList;
            bmsManufacturingStatusListUpdatedAt = Util.getCurrentDateTimeStr("yyyy/MM/dd", "hh:mm:ss", currentUtcOffsetMinutes);
        }
    }

    function getParamList(dataMap1, dataMap2)
    {
        var targetList = [];
        for (var key in dataMap1)
        {
            var val1 = dataMap1[key].toString().toUpperCase();
            var val2 = dataMap2[key].toString().toUpperCase();
            var newItem = { Name: key, Val1: val1, Val2: val2 };
            targetList.push(newItem);
        }
        return targetList;
    }
}

