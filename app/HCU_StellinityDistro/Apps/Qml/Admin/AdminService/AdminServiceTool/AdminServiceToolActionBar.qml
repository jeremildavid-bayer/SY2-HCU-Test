import QtQuick 2.12
import "../../../Widgets"
import "../../../Util.js" as Util

Rectangle {
    property var hwDigest: dsMcu.hwDigest
    property string mcuSerialNumber: dsMcu.mcuSerialNumber
    property var motorModuleSerialNumbers: dsMcu.motorModuleSerialNumbers
    property string hcuBuildType: dsSystem.hcuBuildType
    property string hcuVersion: dsSystem.hcuVersion
    property string mcuVersion: dsMcu.mcuVersion
    property string mcuCommandVersion: dsMcu.mcuCommandVersion
    property string stopcockVersion: dsMcu.stopcockVersion
    property string mcuLinkState: dsMcu.mcuLinkState
    property var bottleBubbleStates: dsMcu.bottleBubbleStates
    property bool sudsBubbleDetected: dsMcu.sudsBubbleDetected
    property var stopcockPositions: dsMcu.stopcockPositions
    property var syringeVols: dsMcu.syringeVols
    property var syringeFlows: dsMcu.syringeFlows
    property var plungerStates: dsMcu.plungerStates
    property string doorState: dsMcu.doorState
    property string outletDoorState: dsMcu.outletDoorState
    property bool mudsPresent: dsMcu.mudsPresent
    property bool mudsLatched: dsMcu.mudsLatched
    property bool sudsInserted: dsMcu.sudsInserted
    property bool primeBtnPressed: dsMcu.primeBtnPressed
    property bool stopBtnPressed: dsMcu.stopBtnPressed
    property var heatMaintainerStatus: dsMcu.heatMaintainerStatus
    property bool barcodeReaderConnected: dsDevice.barcodeReaderConnected
    property var powerStatus: dsMcu.powerStatus
    property int pressureKpa: dsMcu.pressureKpa
    property double cpuTemperatureCelcius: dsSystem.cpuTemperatureCelcius
    property int hcuFanSpeed: dsSystem.hcuFanSpeed
    property string temperatureUnit: dsCfgGlobal.temperatureUnit
    property string wifiSsid: dsCru.wifiSsid
    property string wifiPassword: dsCru.wifiPassword

    id: root
    height: actionBarHeight
    anchors.bottom: parent.bottom
    anchors.right: parent.right
    anchors.left: parent.left
    anchors.leftMargin: parent.width * 0.09
    color: "transparent"

    Column {
        x: parent.width * 0.0045
        y: parent.height * 0.01
        height: parent.height * 0.99

        Text {
            id: txtHcuVersion
            font.pixelSize: parent.height * 0.07
            color: colorMap.text01
        }

        Text {
            id: txtMcuVersion
            font.pixelSize: parent.height * 0.07
            color: colorMap.text01
        }

        Text {
            id: txtScVersion
            font.pixelSize: parent.height * 0.07
            color: colorMap.text01
        }

        Text {
            id: txtMcuSerialNumber
            font.pixelSize: parent.height * 0.07
            color: colorMap.text01
            text: "-"
        }

        Text {
            id: txtMmSerialNumbers
            font.pixelSize: parent.height * 0.07
            color: colorMap.text01
            text: "-,-,-"
        }

        Text {
            id: txtWifiConfig
            font.pixelSize: parent.height * 0.06
            color: colorMap.text01
        }

        Item {
            height: parent.height * 0.01
            width: parent.width
        }

        Text {
            id: txtBatteryVoltage
            font.pixelSize: parent.height * 0.06
            color: colorMap.text02
        }

        Text {
            id: txtSupplyVoltage
            font.pixelSize: parent.height * 0.06
            color: colorMap.text02
        }

        Text {
            id: txtHeatMaintainerCurrent
            font.pixelSize: parent.height * 0.06
            color: colorMap.text02
        }

        Text {
            id: txtCpuTemperature
            font.pixelSize: parent.height * 0.06
            color: colorMap.text02
        }

        Text {
            id: txtHcuFanSpeed
            font.pixelSize: parent.height * 0.06
            color: colorMap.text02
        }

        Text {
            id: txtCurrent
            font.pixelSize: parent.height * 0.06
            color: colorMap.text02
        }

        Text {
            id: txtMudsSodStatus
            font.pixelSize: parent.height * 0.06
            color: colorMap.text02
        }
    }

    Row {
        x: parent.width * 0.22
        y: parent.height * 0.03
        height: parent.height * 0.44
        width: parent.width - x
        spacing: parent.width * 0.002

        AdminServiceToolActionBarSensor {
            id: syringe1
            title: "Syringe S"
        }

        AdminServiceToolActionBarSensor {
            id: syringe2
            title: "Syringe C1"
        }

        AdminServiceToolActionBarSensor {
            id: syringe3
            title: "Syringe C2"
        }

        AdminServiceToolActionBarSensor {
            id: stopcock1
            title: "Stopcock S"
        }

        AdminServiceToolActionBarSensor {
            id: stopcock2
            title: "Stopcock C1"
        }

        AdminServiceToolActionBarSensor {
            id: stopcock3
            title: "Stopcock C2"
        }

        AdminServiceToolActionBarSensor {
            id: bubbleBottle1
            title: "Bottle S Air"
        }

        AdminServiceToolActionBarSensor {
            id: bubbleBottle2
            title: "Bottle C1 Air"
        }

        AdminServiceToolActionBarSensor {
            id: bubbleBottle3
            title: "Bottle C2 Air"
        }

        AdminServiceToolActionBarSensor {
            id: bubbleSuds
            title: "SUDS Air"
        }

        AdminServiceToolActionBarSensor {
            id: pressure
            title: "Pressure"
        }

        AdminServiceToolActionBarSensor {
            id: battery
            title: "Battery"
        }
    }

    Row {
        x: parent.width * 0.22
        y: parent.height * 0.53
        height: parent.height * 0.44
        spacing: parent.width * 0.002
        width: parent.width - x

        AdminServiceToolActionBarSensor {
            id: mudsLatchedState
            title: "MUDS Latch"
        }

        AdminServiceToolActionBarSensor {
            id: mudsPresentState
            title: "MUDS Presence"
        }

        AdminServiceToolActionBarSensor {
            id: sudsState
            title: "SUDS State"
        }

        AdminServiceToolActionBarSensor {
            id: door
            title: "Door"
        }

        AdminServiceToolActionBarSensor {
            id: doorUnlockBtn
            title: "Door Unlock Button"
        }

        AdminServiceToolActionBarSensor {
            id: outletDoor
            title: "Outlet Door"
        }

        AdminServiceToolActionBarSensor {
            id: wasteBinPresence
            title: "Waste Bin Presence"
        }

        AdminServiceToolActionBarSensor {
            id: wasteBinLevel
            title: "Waste Bin Level"
        }

        AdminServiceToolActionBarSensor {
            id: advanceBtn
            title: "Advance Btn"
        }

        AdminServiceToolActionBarSensor {
            id: stopBtn
            title: "Stop Btn"
        }

        AdminServiceToolActionBarSensor {
            id: heatMaintainer
            title: "Heat Maintainer"
        }

        AdminServiceToolActionBarSensor {
            id: barcodeReader
            title: "Barcode Reader"
        }
    }

    onMotorModuleSerialNumbersChanged: {
        reloadMotorModuleSerialNumbers();
    }

    onMcuSerialNumberChanged: {
        reloadMcuSerialNumber();
    }

    onHcuVersionChanged: {
        reloadHcuVersion();
    }

    onHcuBuildTypeChanged: {
        reloadHcuVersion();
    }

    onMcuVersionChanged: {
        reloadMcuVersion();
    }

    onMcuCommandVersionChanged: {
        reloadMcuVersion();
    }

    onStopcockVersionChanged: {
        reloadStopcockVersion();
    }

    onWifiSsidChanged: {
        reloadWifiConfig();
    }

    onWifiPasswordChanged: {
        reloadWifiConfig();
    }

    onBottleBubbleStatesChanged: {
        reloadBottleBubbleStates();
    }

    onSudsBubbleDetectedChanged: {
        reloadSudsBubbleDetected();
    }

    onStopcockPositionsChanged: {
        reloadStopcockPositions();
    }

    onSyringeVolsChanged: {
        reloadSyringeStates();
    }

    onSyringeFlowsChanged: {
        reloadSyringeStates();
    }

    onPlungerStatesChanged: {
        reloadSyringeStates();
    }

    onDoorStateChanged: {
        reloadDoorState();
    }

    onOutletDoorStateChanged: {
        reloadOutletDoorState();
    }

    onMudsPresentChanged: {
        reloadMudsPresent();
    }

    onMudsLatchedChanged: {
        reloadMudsLatched();
    }

    onSudsInsertedChanged: {
        reloadSudsInserted();
    }

    onPrimeBtnPressedChanged: {
        reloadPrimeBtnPressed();
    }

    onStopBtnPressedChanged: {
        reloadStopBtnPressed();
    }

    onHeatMaintainerStatusChanged: {
        reloadHeatMaintainerStatus();
    }

    onBarcodeReaderConnectedChanged: {
        reloadBarcodeReaderConnected();
    }

    onPowerStatusChanged: {
        reloadPowerStatus();
    }

    onHwDigestChanged: {
        reloadHwDigest();
    }

    onCpuTemperatureCelciusChanged: {
        reloadCpuTemperature();
    }

    onHcuFanSpeedChanged: {
        reloadHcuFanSpeed();
    }

    onPressureKpaChanged: {
        reloadPressureKpa();
    }

    Component.onCompleted:
    {
        appMain.screenStateChanged.connect(slotScreenStateChanged);
        slotScreenStateChanged();
    }

    function slotScreenStateChanged()
    {
        visible = (appMain.screenState.indexOf("Admin-Service-Tool") >= 0);
        reload();
    }

    function reload()
    {
        if (!visible)
        {
            return;
        }

        reloadMcuSerialNumber();
        reloadMotorModuleSerialNumbers();
        reloadHcuVersion();
        reloadMcuVersion();
        reloadStopcockVersion();
        reloadWifiConfig();
        reloadBottleBubbleStates();
        reloadSudsBubbleDetected();
        reloadStopcockPositions();
        reloadSyringeStates();
        reloadDoorState();
        reloadOutletDoorState();
        reloadMudsPresent();
        reloadMudsLatched();
        reloadSudsInserted();
        reloadPrimeBtnPressed();
        reloadStopBtnPressed();
        reloadHeatMaintainerStatus();
        reloadBarcodeReaderConnected();
        reloadPowerStatus();
        reloadHwDigest();
        reloadCpuTemperature();
        reloadHcuFanSpeed();
        reloadPressureKpa();
    }

    function reloadMcuSerialNumber()
    {
        if (!visible)
        {
            return;
        }
        txtMcuSerialNumber.text = "MCU Serial: " + mcuSerialNumber;
    }

    function reloadMotorModuleSerialNumbers()
    {
        if (!visible)
        {
            return;
        }

        txtMmSerialNumbers.text = "MM Serials: ";
        for (var i = 0; i < motorModuleSerialNumbers.length; i++)
        {
            txtMmSerialNumbers.text += (motorModuleSerialNumbers[i] === "NOT_CONFIGURED") ? "-" : motorModuleSerialNumbers[i];
            txtMmSerialNumbers.text += (i < motorModuleSerialNumbers.length - 1) ? "," : "";
        }
    }

    function reloadHcuVersion()
    {
        if (!visible)
        {
            return;
        }

        txtHcuVersion.text = "HCU Version : " + hcuVersion + " - " + hcuBuildType;
    }


    function reloadMcuVersion()
    {
        if (!visible)
        {
            return;
        }

        txtMcuVersion.text = "MCU Version : " + mcuVersion + " (CMD=v" + mcuCommandVersion + ")";
    }

    function reloadStopcockVersion()
    {
        if (!visible)
        {
            return;
        }

        txtScVersion.text =  "SC Version : " + stopcockVersion;
    }

    function reloadWifiConfig()
    {
        if (!visible)
        {
            return;
        }

        txtWifiConfig.text =  "Wifi : " + wifiSsid + " /\n            " + wifiPassword;
    }

    function reloadBottleBubbleStates()
    {
        if (!visible)
        {
            return;
        }

        if ( (bottleBubbleStates !== undefined) &&
             (hwDigest !== undefined) )
        {
            bubbleBottle1.value = bottleBubbleStates[0] + "\n\n" + hwDigest.InletAirSensorValue1;
            bubbleBottle2.value = bottleBubbleStates[1] + "\n\n" + hwDigest.InletAirSensorValue2;
            bubbleBottle3.value = bottleBubbleStates[2] + "\n\n" + hwDigest.InletAirSensorValue3;

            if (bottleBubbleStates[0] === "AIR")
            {
                bubbleBottle1.valueColor = "red";
            }
            else
            {
                bubbleBottle1.valueColor = colorMap.text02;
            }

            if (bottleBubbleStates[1] === "AIR")
            {
                bubbleBottle2.valueColor = "red";
            }
            else
            {
                bubbleBottle2.valueColor = colorMap.text02;
            }

            if (bottleBubbleStates[2] === "AIR")
            {
                bubbleBottle3.valueColor = "red";
            }
            else
            {
                bubbleBottle3.valueColor = colorMap.text02;
            }
        }
    }

    function reloadSudsBubbleDetected()
    {
        if (!visible)
        {
            return;
        }

        if (sudsBubbleDetected !== undefined)
        {
            bubbleSuds.value = sudsBubbleDetected ? "AIR" : "FLUID";
            bubbleSuds.valueColor = sudsBubbleDetected ? "red" : colorMap.text02;
        }
    }

    function reloadStopcockPositions()
    {
        if (!visible)
        {
            return;
        }

        if ( (stopcockPositions !== undefined) &&
             (hwDigest !== undefined) )
        {
            stopcock1.value = (hwDigest.SCEngagedSaline ? "ENGAGED" : "DISENGAGED") + "\n" + stopcockPositions[0];

            if (hwDigest.SCEngagedSaline === "DISENGAGED")
            {
                stopcock1.valueColor = "red";
            }
            else if (stopcockPositions[0] === "FILL")
            {
                stopcock1.valueColor = "blue";
            }
            else if (stopcockPositions[0] === "INJECT")
            {
                stopcock1.valueColor = "green";
            }
            else if (stopcockPositions[0] === "CLOSED")
            {
                stopcock1.valueColor = "orange";
            }

            stopcock2.value = (hwDigest.SCEngagedContrast1 ? "ENGAGED" : "DISENGAGED") + "\n" + stopcockPositions[1];

            if (hwDigest.SCEngagedContrast1 === "DISENGAGED")
            {
                stopcock2.valueColor = "red";
            }
            else if (stopcockPositions[1] === "FILL")
            {
                stopcock2.valueColor = "blue";
            }
            else if (stopcockPositions[1] === "INJECT")
            {
                stopcock2.valueColor = "green";
            }
            else if (stopcockPositions[1] === "CLOSED")
            {
                stopcock2.valueColor = "orange";
            }

            stopcock3.value = (hwDigest.SCEngagedContrast2 ? "ENGAGED" : "DISENGAGED") + "\n" + stopcockPositions[2];

            if (hwDigest.SCEngagedContrast2 === "DISENGAGED")
            {
                stopcock3.valueColor = "red";
            }
            else if (stopcockPositions[2] === "FILL")
            {
                stopcock3.valueColor = "blue";
            }
            else if (stopcockPositions[2] === "INJECT")
            {
                stopcock3.valueColor = "green";
            }
            else if (stopcockPositions[2] === "CLOSED")
            {
                stopcock3.valueColor = "orange";
            }
        }
    }

    function reloadSyringeStates()
    {
        if (!visible)
        {
            return;
        }

        if ( (plungerStates !== undefined) &&
             (plungerStates.length == 3) &&
             (syringeFlows !== undefined) &&
             (syringeFlows.length == 3) &&
             (syringeVols !== undefined)  &&
             (syringeVols.length == 3) )
        {
            syringe1.value = plungerStates[0] + "\n" + syringeFlows[0].toFixed(1) + "ml/s\n" + Math.floor(syringeVols[0]) + "ml";
            syringe1.valueColor = (plungerStates[0] === "ENGAGED") ? colorMap.text02 : "red";

            syringe2.value = plungerStates[1] + "\n" + syringeFlows[1].toFixed(1) + "ml/s\n" + Math.floor(syringeVols[1]) + "ml";
            syringe2.valueColor = (plungerStates[1] === "ENGAGED") ? colorMap.text02 : "red";

            syringe3.value = plungerStates[2] + "\n" + syringeFlows[2].toFixed(1) + "ml/s\n" + Math.floor(syringeVols[2]) + "ml";
            syringe3.valueColor = (plungerStates[2] === "ENGAGED") ? colorMap.text02 : "red";
        }
    }

    function reloadDoorState()
    {
        if (!visible)
        {
            return;
        }

        door.value = doorState;
        door.valueColor = (doorState == "CLOSED" ? colorMap.text02 : "red");
    }

    function reloadOutletDoorState()
    {
        if (!visible)
        {
            return;
        }

        outletDoor.value = outletDoorState;
        outletDoor.valueColor = (outletDoorState == "CLOSED" ? colorMap.text02 : "red");
    }

    function reloadMudsPresent()
    {
        if (!visible)
        {
            return;
        }

        mudsPresentState.value = (mudsPresent ? "PRESENT" : "MISSING");
        mudsPresentState.valueColor = mudsPresent ? colorMap.text02 : "red";
    }

    function reloadMudsLatched()
    {
        if (!visible)
        {
            return;
        }

        mudsLatchedState.value = mudsLatched ? "LATCHED" : "UNLATCHED";
        mudsLatchedState.valueColor = (mudsLatched ? colorMap.text02 : "red");
    }

    function reloadSudsInserted()
    {
        if (!visible)
        {
            return;
        }

        sudsState.value = sudsInserted ? "INSERTED" : "MISSING";
        sudsState.valueColor = sudsInserted ? colorMap.text02 : "red";
    }

    function reloadPrimeBtnPressed()
    {
        if (!visible)
        {
            return;
        }

        advanceBtn.value = primeBtnPressed ? "PRESSED" : "RELEASED";
        advanceBtn.valueColor = primeBtnPressed ? "blue" : colorMap.text02;
    }

    function reloadStopBtnPressed()
    {
        if (!visible)
        {
            return;
        }

        stopBtn.value = stopBtnPressed ? "PRESSED" : "RELEASED";
        stopBtn.valueColor = stopBtnPressed ? "blue" : colorMap.text02;
    }

    function reloadHeatMaintainerStatus()
    {
        if (!visible)
        {
            return;
        }

        if (heatMaintainerStatus !== undefined)
        {
            var reading1 = (heatMaintainerStatus.TemperatureReadings.MUDS === undefined) ? "--" : heatMaintainerStatus.TemperatureReadings.MUDS.toFixed(1);
            var reading2 = (heatMaintainerStatus.TemperatureReadings.DOOR === undefined) ? "--" : heatMaintainerStatus.TemperatureReadings.DOOR.toFixed(1);
            heatMaintainer.value = reading1 + "\n" + reading2 + "\n" + heatMaintainerStatus.State;
        }
    }

    function reloadBarcodeReaderConnected()
    {
        if (!visible)
        {
            return;
        }

        barcodeReader.value = barcodeReaderConnected ? "CONNECTED" : "DISCONNECTED";
        barcodeReader.valueColor = barcodeReaderConnected ? "blue" : "red";
    }

    function reloadPowerStatus()
    {
        if (!visible)
        {
            return;
        }

        if (powerStatus !== undefined)
        {
            battery.value = (powerStatus.IsAcPowered ? "AC" : "BATTERY") + "\n" +
                            powerStatus.BatteryLevel;
        }
    }

    function reloadHwDigest()
    {
        if (!visible)
        {
            return;
        }

        if (hwDigest === undefined)
        {
            return;
        }

        txtBatteryVoltage.text = "Power : Main(" + (hwDigest.MainConnected ? "O" : "X") + ", Battery(" + hwDigest.BatteryVoltage + "V), USB(" + hwDigest.UsbHostVoltage + "V)";
        txtSupplyVoltage.text = "Supply (V5/12/28.5) : " + hwDigest.V5Voltage + " / " + hwDigest.V12AuxVoltage + " / " + hwDigest.V285Voltage;
        txtHeatMaintainerCurrent.text = "Heat Maintainer Current : " + hwDigest.HeatMaintainerCurrent + "mA";
        txtCurrent.text = "Current : HCU(" + hwDigest.HcuCurrent + "mA), AUX(" + hwDigest.AuxCurrent + "mA)"
        doorUnlockBtn.value = hwDigest.DoorButtonPressed ? "PRESSED" : "RELEASED";
        doorUnlockBtn.valueColor = hwDigest.DoorButtonPressed ? "blue" : colorMap.text02;

        wasteBinLevel.value = hwDigest.WasteLevelSensor1 + "\n" + hwDigest.WasteLevelSensor2;

        wasteBinPresence.value = hwDigest.WastePresence ? "PRESENT" : "MISSING";
        wasteBinPresence.valueColor = hwDigest.WastePresence ? colorMap.text02 : "red";

        reloadBottleBubbleStates();
        reloadStopcockPositions();
    }

    function reloadCpuTemperature()
    {
        if (!visible)
        {
            return;
        }

        var unit = (temperatureUnit == "degreesF") ? "°F" : "°C";
        txtCpuTemperature.text = "HCU CPU Temperature: " + Util.getTemperature(temperatureUnit, cpuTemperatureCelcius) + unit;

    }

    function reloadHcuFanSpeed()
    {
        if (!visible)
        {
            return;
        }

        if (hcuFanSpeed === -1)
        {
            txtHcuFanSpeed.text = "HCU Fan Speed: N/A";
        }
        else
        {

            txtHcuFanSpeed.text = "HCU Fan Speed: " + hcuFanSpeed + "rpm";
        }

    }

    function reloadPressureKpa()
    {
        if (!visible)
        {
            return;
        }

        pressure.value = pressureKpa + " kPa";
    }
}

