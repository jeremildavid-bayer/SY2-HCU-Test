import QtQuick 2.12

Item {
    // Data
    property bool simulatorEnabled: false
    property var ledControlStatus
    property double syringesAirVolume

    // Function from QML to CPP
    function slotStopButtonPressed(pressed) { return dsMcuSimCpp.slotStopButtonPressed(pressed); }
    function slotManualPrimeButtonPressed(pressed) { return dsMcuSimCpp.slotManualPrimeButtonPressed(pressed); }
    function slotMudsPresent(present) { return dsMcuSimCpp.slotMudsPresent(present); }
    function slotMudsLatched(latched) { return dsMcuSimCpp.slotMudsLatched(latched); }
    function slotSudsInserted(inserted) { return dsMcuSimCpp.slotSudsInserted(inserted); }
    function slotAdaptiveFlowStateChanged(state) { return dsMcuSimCpp.slotAdaptiveFlowStateChanged(state); }
    function slotWasteBinStateChanged(state) { return dsMcuSimCpp.slotWasteBinStateChanged(state); }
    function slotDoorStateChanged(state) { return dsMcuSimCpp.slotDoorStateChanged(state); }
    function slotBatteryStateChanged(state) { return dsMcuSimCpp.slotBatteryStateChanged(state); }
    function slotAcConnectedStateChanged(state) { return dsMcuSimCpp.slotAcConnectedStateChanged(state); }
    function slotOutletDoorStateChanged(state) { return dsMcuSimCpp.slotOutletDoorStateChanged(state); }
    function slotTemperature1Changed(temperature) { return dsMcuSimCpp.slotTemperature1Changed(temperature); }
    function slotTemperature2Changed(temperature) { return dsMcuSimCpp.slotTemperature2Changed(temperature); }
    function slotSyringesAir() { return dsMcuSimCpp.slotSyringesAir(); }
    function slotBubblePatientLine() { return dsMcuSimCpp.slotBubblePatientLine(); }
    function slotBubbleSaline() { return dsMcuSimCpp.slotBubbleSaline(); }
    function slotBubbleContrast1() { return dsMcuSimCpp.slotBubbleContrast1(); }
    function slotBubbleContrast2() { return dsMcuSimCpp.slotBubbleContrast2(); }
    function slotAlarmGetNext(alarmName)  { return dsMcuSimCpp.slotAlarmGetNext(alarmName); }
    function slotAlarmActivate(alarmName)  { return dsMcuSimCpp.slotAlarmActivate(alarmName); }
}
