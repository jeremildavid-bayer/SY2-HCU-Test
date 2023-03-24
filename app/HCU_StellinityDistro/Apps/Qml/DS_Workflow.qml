import QtQuick 2.12

Item {
    // Data
    property string workflowState: "STATE_INACTIVE"
    property string sodErrorState: "SOD_ERROR_STATE_NONE"
    property var workflowErrorStatus
    property string workflowSudsAirRecoveryState: "SUDS_AIR_RECOVERY_STATE_INACTIVE"
    property string workflowMudsSodState: "MUDS_SOD_STATE_INACTIVE"
    property string workflowFluidRemovalState: "FLUID_REMOVAL_STATE_INACTIVE"
    property string workflowEndOfDayPurgeState: "END_OF_DAY_PURGE_STATE_INACTIVE"
    property string workflowAutoEmptyState: "AUTO_EMPTY_STATE_INACTIVE"
    property string workflowMudsEjectState: "MUDS_EJECT_STATE_INACTIVE"
    property string workflowSyringeAirRecoveryState: "SYRINGE_AIR_RECOVERY_STATE_INACTIVE"
    property string workflowPreloadProtocolState: "PRELOAD_PROTOCOL_STATE_INACTIVE"
    property var mudsSodStatus
    property var preloadStatus
    property var workflowManualQualifiedDischargeStatus
    property var workflowBatteryStatus
    property var workflowShippingModeStatus
    property var workflowAutomaticQualifiedDischargeStatus
    property bool contrastAvailable1
    property bool contrastAvailable2

    // Function from QML to CPP
    function slotSodResume() { return dsWorkflowCpp.slotSodResume(); }
    function slotFill(index)  { return dsWorkflowCpp.slotFill(index); }
    function slotEjectMuds()  { return dsWorkflowCpp.slotEjectMuds(); }
    function slotSudsAutoPrimeForceStart()  { return dsWorkflowCpp.slotSudsAutoPrimeForceStart(); }
    function slotLoadNewSource(index, brand, concentration, volume, lotBatch, expirationDate)  { return dsWorkflowCpp.slotLoadNewSource(index, brand, concentration, volume, lotBatch, expirationDate); }
    function slotUnloadSource(index)  { return dsWorkflowCpp.slotUnloadSource(index); }
    function slotSudsAirRecoveryResume()  { return dsWorkflowCpp.slotSudsAirRecoveryResume(); }
    function slotFluidRemovalStart(index)  { return dsWorkflowCpp.slotFluidRemovalStart(index); }
    function slotFluidRemovalResume()  { return dsWorkflowCpp.slotFluidRemovalResume(); }
    function slotFluidRemovalAbort()  { return dsWorkflowCpp.slotFluidRemovalAbort(); }
    function slotEndOfDayPurgeStart()  { return dsWorkflowCpp.slotEndOfDayPurgeStart(); }
    function slotEndOfDayPurgeResume()  { return dsWorkflowCpp.slotEndOfDayPurgeResume(); }
    function slotEndOfDayPurgeAbort()  { return dsWorkflowCpp.slotEndOfDayPurgeAbort(); }
    function slotWorkflowBatteryAction(batteryIndex, action) { return dsWorkflowCpp.slotWorkflowBatteryAction(batteryIndex, action); }
    function slotWorkflowBatteryAbort() { return dsWorkflowCpp.slotWorkflowBatteryAbort(); }
    function slotQueAutomaticQualifiedDischarge(index) { return dsWorkflowCpp.slotQueAutomaticQualifiedDischarge(index); }
    function slotCancelAutomaticQualifiedDischarge() { return dsWorkflowCpp.slotCancelAutomaticQualifiedDischarge(); }
    function slotManualQualifiedDischargeStart(index, method)  { return dsWorkflowCpp.slotManualQualifiedDischargeStart(index, method); }
    function slotManualQualifiedDischargeResume()  { return dsWorkflowCpp.slotManualQualifiedDischargeResume(); }
    function slotManualQualifiedDischargeAbort()  { return dsWorkflowCpp.slotManualQualifiedDischargeAbort(); }
    function slotShippingModeStart(index, targetCharge)  { return dsWorkflowCpp.slotShippingModeStart(index, targetCharge); }
    function slotShippingModeResume()  { return dsWorkflowCpp.slotShippingModeResume(); }
    function slotShippingModeAbort()  { return dsWorkflowCpp.slotShippingModeAbort(); }
    function slotPreloadProtocolStart(userConfirmRequired)  { return dsWorkflowCpp.slotPreloadProtocolStart(userConfirmRequired); }
    function slotPreloadProtocolResume()  { return dsWorkflowCpp.slotPreloadProtocolResume(); }
    function slotPreloadProtocolAbort()  { return dsWorkflowCpp.slotPreloadProtocolAbort(); }
    function slotClearSodError() { return dsWorkflowCpp.slotClearSodError(); }
    function slotSyringeAirRecoveryResume() { return dsWorkflowCpp.slotSyringeAirRecoveryResume(); }
}
