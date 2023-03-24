import QtQuick 2.12

Item {

    // Data
    property string mcuVersion: ""
    property string mcuCommandVersion: ""
    property string stopcockVersion: ""
    property string mcuSerialNumber: ""
    property var motorModuleSerialNumbers: ["","",""]
    property string mcuLinkState
    property var hwDigest
    property var pressureCalibrationStatus
    property string calibrationResult: ""
    property string bmsDigestOutput: ""
    property string bmsCommandOutput: ""
    property bool isMudsPresent
    property var powerStatus
    property int pressureKpa: 0
    property var heatMaintainerStatus
    property var syringeAirCheckDigests: []
    property var syringeVols: []
    property var syringeFlows: []
    property var plungerStates: []
    property var syringeStates: []
    property var bottleBubbleStates: []
    property bool sudsBubbleDetected: false
    property var stopcockPositions: []
    property string doorState: "UNKNOWN"
    property string outletDoorState: "UNKNOWN"
    property bool mudsInserted: false
    property bool mudsLatched: false
    property bool mudsPresent: false
    property bool sudsInserted: false
    property bool primeBtnPressed: false
    property bool stopBtnPressed: false
    property bool doorBtnPressed: false
    property bool isShuttingDown: false
    property var bmsDigests: []

    // Signal from QML to CPP
    function slotResetMuds() { return dsMcuCpp.slotResetMuds(); }
    function slotResetMcu() { return dsMcuCpp.slotResetMcu(); }
    function slotResetStopcock() { return dsMcuCpp.slotResetStopcock(); }
    function slotHwDigestMonitorActive(active) { return dsMcuCpp.slotHwDigestMonitorActive(active); }
    function slotCalibrateAirDetector(idx) { return dsMcuCpp.slotCalibrateAirDetector(idx); }
    function slotCalibrateMotor(idx) { return dsMcuCpp.slotCalibrateMotor(idx); }
    function slotCalibrateSuds() { return dsMcuCpp.slotCalibrateSuds(); }
    function slotCalibratePlunger(idx) { return dsMcuCpp.slotCalibratePlunger(idx); }
    function slotCalibrateSetPressureMeter() { return dsMcuCpp.slotCalibrateSetPressureMeter(); }
    function slotCalibratePressureStart(idx) { return dsMcuCpp.slotCalibratePressureStart(idx); }
    function slotCalibratePressureStop() { return dsMcuCpp.slotCalibratePressureStop(); }
    function slotCalibrateSetPressure(idx) { return dsMcuCpp.slotCalibrateSetPressure(idx); }
    function slotGetCalibrationStatus() { return dsMcuCpp.slotGetCalibrationStatus(); }
    function slotFindPlunger(index)  { return dsMcuCpp.slotFindPlunger(index); }
    function slotPullPlungers() { return dsMcuCpp.slotPullPlungers(); }
    function slotPistonEngage(index)  { return dsMcuCpp.slotPistonEngage(index); }
    function slotPistonDisengage(index)  { return dsMcuCpp.slotPistonDisengage(index); }
    function slotPistonStop(index)  { return dsMcuCpp.slotPistonStop(index); }
    function slotPistonUp(index, flow)  { return dsMcuCpp.slotPistonUp(index, flow); }
    function slotPistonDown(index, flow)  { return dsMcuCpp.slotPistonDown(index, flow); }
    function slotLedControl(params)  { return dsMcuCpp.slotLedControl(params); }
    function slotDoorLock(lock)  { return dsMcuCpp.slotDoorLock(lock); }
    function slotMovePistonToCleaningPos(index)  { return dsMcuCpp.slotMovePistonToCleaningPos(index); }
    function slotBmsDigest() { return dsMcuCpp.slotBmsDigest(); }
    function slotBmsCommand(index, data) {  return dsMcuCpp.slotBmsCommand(index, data); }
    function slotSetBaseFanTemperature(temperature) {  return dsMcuCpp.slotSetBaseFanTemperature(temperature); }

    // MONITOR FUNCTIONS
    onStopBtnPressedChanged: {
        if (stopBtnPressed)
        {
            soundPlayer.playPressAllStop();
        }
    }

    onIsShuttingDownChanged: {
        appMain.setInteractiveState(false, "DS_Mcu: Shutdown in progress");
    }
}
