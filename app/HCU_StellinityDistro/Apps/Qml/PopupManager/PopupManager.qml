import QtQuick 2.12
import "../Widgets/Popup"

Item {
    property alias popupBarcodeReadErr: popupBarcodeReadErr
    property alias popupBarcodeScannedKnownBarcodeAdded: popupBarcodeScannedKnownBarcodeAdded
    property alias popupFluidExpirationDatePastDue: popupFluidExpirationDatePastDue
    property alias popupFluidRemoval: popupFluidRemoval
    property alias popupShutdownRestartIgnored: popupShutdownRestartIgnored
    property alias popupEjectMudsRequired: popupEjectMudsRequired
    property alias popupBatteryDead: popupBatteryDead
    property alias popupEnterPassCode: popupEnterPassCode
    property alias popupInvalidInjectionSelected: popupInvalidInjectionSelected
    property alias popupPersonalizedProtocolUnavailable: popupPersonalizedProtocolUnavailable
    property alias popupManualQualifiedDischarge: popupManualQualifiedDischarge
    property alias popupShippingMode: popupShippingMode
    property alias popupBatteryTestMode: popupBatteryTestMode
    property alias popupSafeRenewMuds: popupSafeRenewMuds
    property alias popupReprimeFromInjectionPreloaded: popupReprimeFromInjectionPreloaded
    property alias popupActionConfirmation: popupActionConfirmation
    property alias popupSharingInformation: popupSharingInformation
    property alias popupHCUSoftwareErrorAlertManager: popupHCUSoftwareErrorAlertManager
    property alias popupEndOfDayPurge: popupEndOfDayPurge
    property alias popupContrastChangeDisabled : popupContrastChangeDisabled
    property var openPopups: []

    QtObject {
        id: p
        function disableScreenSleep() {
            dsSystemCpp.slotScreenWakeup();
            dsSystemCpp.slotSetScreenSleepTime(0);
            console.debug("Disable Screen Sleep");
        }

        function setScreenSleepDefault() {
            dsSystemCpp.slotSetScreenSleepTimeToDefault();
            console.debug("ScreenSleepDefault");
        }
    }

    // NOTE: Popup Declaration is located from LOW priority to HIGH priority (So that high prioirty popup appears on top of other)

    // ==================================== Popup: Info

    PopupSafeRenewMuds {
        topLevel: 1
        id: popupSafeRenewMuds
    }

    PopupManualQualifiedDischarge {
        topLevel: 1
        id: popupManualQualifiedDischarge
    }

    PopupShippingMode {
        topLevel: 1
        id: popupShippingMode
    }

    PopupBatteryTestMode {
        topLevel: 1
        id: popupBatteryTestMode
    }

    PopupAlertBase {
        topLevel: 1
        alertCodeName: "InjectionDisarmedByTimeout"
        titleText: "T_InjectionDisarmedByTimeout_Name"
        userDirectionText: "T_InjectionDisarmedByTimeout_UserDirection"
        showCancelBtn: false
    }

    PopupAlertBase {
        topLevel: 1
        alertCodeName: "InjectionDisarmedByMUDSUnlatched"
        titleText: "T_InjectionDisarmedByMUDSUnlatched_Name"
        userDirectionText: "T_InjectionDisarmedByMUDSUnlatched_UserDirection"
        showCancelBtn: false
    }

    PopupAlertBase {
        topLevel: 1
        alertCodeName: "OutletAirDoorLeftOpen"
        titleText: "T_OutletAirDoorLeftOpen_Name"
        userDirectionText: "T_OutletAirDoorLeftOpen_UserDirection"
        showCancelBtn: false
        clearAlertWhenAck: false
        onBtnOkClicked: {
            close();
        }
    }

    PopupAlertBase {
        topLevel: 1
        alertCodeName: "ISI2InjectionDisarmedByCommLoss"
        titleText: "T_ISI2InjectionDisarmedByCommLoss_Name"
        userDirectionText: "T_ISI2InjectionDisarmedByCommLoss_UserDirection"
        showCancelBtn: false
    }

    PopupAlertBase {
        topLevel: 1
        alertCodeName: "InjectionDisarmedByCommLoss"
        titleText: "T_InjectionDisarmedByCommLoss_Name"
        userDirectionText: "T_InjectionDisarmedByCommLoss_UserDirection"
        showCancelBtn: false
    }

    PopupAlertBase {
        topLevel: 1
        alertCodeName: "SI2009CannotUpdateConcentration"
        titleText: "T_SI2009CannotUpdateConcentration_Name"
        userDirectionText: "T_SI2009CannotUpdateConcentration_UserDirection"
        showCancelBtn: false
    }

    PopupAlertBase {
        topLevel: 1
        alertCodeName: "SI2009HoldPhaseEncountered"
        titleText: "T_SI2009HoldPhaseEncountered_Name"
        userDirectionText: "T_SI2009HoldPhaseEncountered_UserDirection"
        showCancelBtn: false
    }

    PopupAlertBase {
        topLevel: 1
        alertCodeName: "RemoteServicingEnabled"
        titleText: "T_RemoteServicingEnabled_Name"
        userDirectionText: "T_RemoteServicingEnabled_UserDirection"
        showDuringServicing: true
        okBtnText: "T_Cancel"
        showCancelBtn: false
    }

    PopupArmCatheterLimitCheck { topLevel: 1 }
    PopupArmFluidVolumeAdjust { topLevel: 1 }
    PopupGenericAlertManager {}
    PopupBottleFillAbortWaiting { topLevel: 1 }

    PopupShutdownRestartIgnored {
        topLevel: 1
        id: popupShutdownRestartIgnored
    }

    PopupFluidRemoval {
        topLevel: 1
        id: popupFluidRemoval

        onWorkflowFluidRemovalStateChanged: {
            if (workflowFluidRemovalState === "FLUID_REMOVAL_STATE_WAIT_SUDS_TO_WASTE_CONTAINER"){
                // open Air Embolism Risk popup on top
                popupEnsureNoPatient.add(this);
            }
        }
    }

    PopupMessage {
        id: popupEjectMudsRequired
        topLevel: 1
        type: "WARNING"
        titleText: "Day Set Inserted"
        contentText: "Please eject the Day Set and try again."
        showCancelBtn: false
        translationRequired: false
        onBtnOkClicked: {
            close();
        }
    }

    PopupMessage {
        id: popupBatteryDead
        topLevel: 1
        type: "WARNING"
        titleText: "Battery Dead"
        contentText: "Please connect AC power and try again."
        showCancelBtn: false
        translationRequired: false
        onBtnOkClicked: {
            close();
        }
    }

    PopupPreloadProtocol {
        property string examProgressState: dsExam.examProgressState
        topLevel: 1

        onWorkflowPreloadProtocolStateChanged: {
            if ((workflowPreloadProtocolState === "PRELOAD_PROTOCOL_STATE_USER_CONFIRM_WAITING")) {
                // open Air Embolism Risk popup on top
                popupEnsureNoPatient.add(this);
            }
        }
    }

    PopupMessage {
        property string examProgressState: dsExam.examProgressState
        id: popupReprimeFromInjectionPreloaded
        topLevel: 1
        type: "INFO"
        titleText: "T_Reprime_Title"
        contentText: "T_Reprime_Message"
        onBtnOkClicked: {
            dsWorkflow.slotSudsAutoPrimeForceStart();
            close();
        }
        onBtnCancelClicked: {
            close();
        }
        onExamProgressStateChanged: {
            if ( (isOpen()) && (examProgressState === "Completing") )
            {
                close();
            }
        }

        onOpened: {
            // open Air Embolism Risk popup on top
            popupEnsureNoPatient.add(this);
        }
    }

    PopupRepreloadReprimeRequired {
        topLevel: 1
        onOpened: {
            // open Air Embolism Risk popup on top
            popupEnsureNoPatient.add(this);
        }
    }

    PopupMessage {
        id: popupActionConfirmation
        type: "INFO"
        topLevel: 1
        showOkBtn: true
        showCancelBtn: true
        titleText: "Confirmation"
        contentText: ""

        property var okFunction
        property var okFunctionArgumentsList

        onBtnCancelClicked: {
            close();
            clear();
        }

        onBtnOkClicked: {
            if ((okFunction !== undefined) && (okFunctionArgumentsList !== undefined))
            {
                okFunction.apply(null, okFunctionArgumentsList);
            }

            close();
            clear();
        }

        function clear()
        {
            okFunction = undefined;
            okFunctionArgumentsList = undefined;
            titleText = "Confirmation";
            contentText = ""
        }
    }

    PopupSharingInformation {
        id: popupSharingInformation
        topLevel: 1
    }

    PopupAlertBase {
        topLevel: 1
        alertCodeName: "HCUClockBatteryDead"
        titleText: "T_HCUClockBatteryDead_Name"
        userDirectionText: "T_HCUClockBatteryDead_UserDirection"
        showCancelBtn: false
        showFromStartupScreen: true
        onBtnOkClicked: {
            ackAlert();
        }
    }

    PopupAlertBase {
        topLevel: 1
        alertCodeName: "PreventativeMaintenanceReminder"
        titleText: "T_PreventativeMaintenanceReminder_Name"
        userDirectionText: "T_PreventativeMaintenanceReminder_UserDirection"
        showCancelBtn: false
        showFromStartupScreen: true
        onBtnOkClicked: {
            ackAlert();
            dsCfgLocal.slotSetLastPMReminderAtToNow();
        }
    }

    PopupEndOfDayPurge {
        topLevel: 1
        id: popupEndOfDayPurge

        // this popup doesn't call open on workflowEndOfDayPurgeState changes...
        onWorkflowEndOfDayPurgeStateChanged: {
            if (visible && (workflowEndOfDayPurgeState === "END_OF_DAY_PURGE_STATE_WAIT_SUDS_TO_WASTE_CONTAINER")) {
                // open Air Embolism Risk popup on top
                popupEnsureNoPatient.add(this);
            }
        }
    }

    PopupMessage {
        id: popupContrastChangeDisabled
        topLevel: 1
        type: "PLAIN"
        titleText: "T_ChangeContrastDisabled_Name"
        contentText: "T_ChangeContrastDisabled_Message"
        showCancelBtn: false
        onBtnOkClicked: {
            close();
        }
    }


    // ==================================== Popup: Warning
    PopupAlertBase {
        topLevel: 2
        alertCodeName: "SUDSPortExposedNotifyUser"
        titleText: "T_SUDSPortExposedNotifyUser_Name"
        userDirectionText: "T_SUDSPortExposedNotifyUser_UserDirection"
        showCancelBtn: false
    }

    PopupAlertBase {
        topLevel: 2
        alertCodeName: "TimeoutExamEnded"
        titleText: "T_TimeoutExamEnded_Name"
        userDirectionText: "T_TimeoutExamEnded_UserDirection"
        showCancelBtn: false
    }

    PopupAlertBase {
        topLevel: 2
        alertCodeName: "SI2009InvalidConcentration"
        titleText: "T_SI2009InvalidConcentration_Name"
        userDirectionText: "T_SI2009InvalidConcentration_UserDirection"
        showOkBtn: false
        showCancelBtn: false
    }

    PopupAlertBase {
        topLevel: 2
        alertCodeName: "SI2009CannotSwitchConcentrations"
        titleText: "T_SI2009CannotSwitchConcentrations_Name"
        userDirectionText: "T_SI2009CannotSwitchConcentrations_UserDirection"
        showOkBtn: false
        showCancelBtn: false
    }

    PopupAlertBase {
        topLevel: 2
        alertCodeName: "RemoteServicingRestartRequired"
        titleText: "T_RemoteServicingRestartRequired_Name"
        userDirectionText: "T_RemoteServicingRestartRequired_UserDirection"
        okBtnText: "T_Restart"
        showCancelBtn: false
        clearAlertWhenAck: false
        showDuringServicing: true
        onBtnOkClicked: {
            dsSystem.slotShutdown(false);
            appMain.setInteractiveState(false, "RemoteServicingRestartRequired: Shutdown");
        }
    }

    PopupAlertBase {
        topLevel: 2
        alertCodeName: "SalineReservoirContrastPossible"
        titleText: "T_SalineReservoirContrastPossible_Name"
        userDirectionText: "T_SalineReservoirContrastPossible_UserDirection"
        showCancelBtn: false
    }

    PopupMessage {
        id: popupBarcodeReadErr
        topLevel: 2
        type: "PLAIN"
        contentText: "T_InvalidBarcodeScanned_Message"
        showCancelBtn: false
        onBtnOkClicked: {
            close();
            dsDevice.slotBarcodeReaderStart(0);
        }
    }

    PopupMessage {
        id: popupBarcodeScannedKnownBarcodeAdded
        topLevel: 2
        type: "INFO"
        titleText: "T_KnownBarcodeAdded_Title"
        contentText: "T_KnownBarcodeAdded_Message"
        showCancelBtn: false
        onBtnOkClicked: {
            close();
        }
    }

    PopupAlertBase {
        id: popupFluidExpirationDatePastDue
        topLevel: 2
        alertCodeName: "FluidExpirationDatePastDue"
        titleText: "T_FluidExpirationDatePastDue_Name"
        userDirectionText: "T_FluidExpirationDatePastDue_UserDirection"
        contentText: {
            if (activeAlert === undefined) {
                return "";
            } else {
                var alertData = JSON.parse(activeAlert.Data);
                var expirationStr= alertData["Expiration"];
                var supplyStr = alertData["Supply"];
                return (userDirectionText + ";" + expirationStr + ";" + supplyStr);
            }
        }
    }

    PopupStopcockUnintendedMotionDetected { topLevel: 2 }
    PopupManualPrimeFailed { topLevel: 2 }
    PopupDoorOpenFailed { topLevel: 2 }
    PopupSudsReinsertedPrimeRequired  {
        topLevel: 2
        onOpened: {
            // open Air Embolism Risk popup on top
            popupEnsureNoPatient.add(this);
        }
    }
    PopupPrimeNeededAfterSodFill {
        topLevel: 2
        onOpened: {
            if (workflowErrorStatus === undefined) return;

            if (workflowErrorStatus.State === "WORKFLOW_ERROR_STATE_SOD_FILL_START_WITH_PATIENT_CONNECTED") {
                // open Air Embolism Risk popup on top
                popupEnsureNoPatient.add(this);
            }
        }
    }
    PopupBatteryCriticalShutdown { topLevel: 2 }
    PopupSODError {
        topLevel: 2
        onOpened: {
            if (sodErrorState === "SOD_ERROR_STATE_REUSE_MUDS_WAITING_SUDS_REMOVAL") {
                // open Air Embolism Risk popup on top
                popupEnsureNoPatient.add(this);
            }
        }
    }
    PopupSudsAirRecovery {
        topLevel: 2
        onWorkflowSudsAirRecoveryStateChanged: {
            if (visible && (workflowSudsAirRecoveryState === "SUDS_AIR_RECOVERY_STATE_USER_START_WAITING")) {
                // open Air Embolism Risk popup on top
                popupEnsureNoPatient.add(this);
            }
        }
    }
    PopupSudsAirRecoveryAutoPrime { topLevel: 2 }
    PopupSyringeAirRecovery {
        topLevel: 2
        onWorkflowSyringeAirRecoveryStateChanged: {
            if (visible && (workflowSyringeAirRecoveryState === "SYRINGE_AIR_RECOVERY_STATE_USER_START_WAITING_2")) {
                // open Air Embolism Risk popup on top
                popupEnsureNoPatient.add(this);
            }
        }
    }
    PopupInjectionRequestFailed { topLevel: 2 }
    PopupArmInsufficientVolumeForSteps { topLevel: 2 }
    PopupAutoPrimeFailed { topLevel: 2 }
    PopupAlertBase {
        topLevel: 2
        alertCodeName: "FillFailed"
        titleText: "T_FillFailed_Name"
        userDirectionText: "T_FillFailed_UserDirection"
    }

    PopupMessage {
        id: popupEnterPassCode
        topLevel: 2
        widthMin: dsCfgLocal.screenW * 0.38
        heightMin: dsCfgLocal.screenH * 0.2
        contentHeight: heightMin - titleHeight - contentMarginBottom
        contentfontPixelSize: contentHeight * 0.5
        btnHeight: 0
        type: "NORMAL"
        showCancelBtn: false
        showOkBtn: false
        showFromStartupScreen: true

        function setValue(newValue)
        {
            var passcode = "";
            for (var i = 0; i < newValue.length; i++)
            {
                passcode += "*";
            }
            textWidget.text = passcode;
            textWidget.color = colorMap.blk01;
        }
    }

    PopupAlertBase {
        topLevel: 2
        alertCodeName: "HaspKeyEnforcement"
        titleText: "T_HaspKeyEnforcement_Name"
        userDirectionText: "T_HaspKeyEnforcement_UserDirection"
        showCancelBtn: false
        showFromStartupScreen: true
    }

    PopupMessage {
        id: popupInvalidInjectionSelected
        topLevel: 2
        contentText: "T_ProtocolIsInvalid"
        type: "PLAIN"
        showCancelBtn: false
        onBtnOkClicked: {
            close();
        }
    }

    PopupMessage {
        id: popupPersonalizedProtocolUnavailable
        topLevel: 2
        contentText: "T_PersonalizedProtocolUnavailable"
        type: "PLAIN"
        showCancelBtn: false
        onBtnOkClicked: {
            close();
        }
    }

    PopupSlide {
        type: "WARNING"
        id: popupEnsureNoPatient
        // Toplevel is 1 higher than other warnings to ensure this displays on top
        topLevel: 3
        titleText: ""
        contentText: "T_AirEmbolismHazard_Message"

        property var parentPopup;
        property bool sudsInserted: dsMcu.sudsInserted;

        // add it with parent popup
        function add(parentPopup) {
            this.parentPopup = parentPopup;
            titleText = parentPopup.titleText;
            parentPopup.closed.connect(close);
            open();
        }

        onClosed: {
            parentPopup = undefined;
            titleText = "";
        }

        onSudsInsertedChanged: {
            if (!sudsInserted) {
                close();
            }
        }
    }

    // ==================================== Popup: Error
    PopupSyringeAirCheckFailed { topLevel: 3 }

    PopupAlertBase {
        topLevel: 3
        alertCodeName: "RemoteServicingActive"
        titleText: "T_RemoteServicingActive_Name"
        userDirectionText: "T_RemoteServicingActive_UserDirection"
        showDuringServicing: true
        showOkBtn: false
        showCancelBtn: false
    }

    PopupAlertBase {
        topLevel: 3
        alertCodeName: "CalSlackFailed"
        titleText: "T_CalSlackFailed_Name"
        userDirectionText: "T_CalSlackFailed_UserDirection"
        okBtnText: "T_Eject"
        showCancelBtn: false
        showServiceBtn: true
        clearAlertWhenAck: false
        onBtnOkClicked: {
            dsWorkflow.slotEjectMuds();
            close();
        }
    }

    PopupAlertBase {
        topLevel: 3
        alertCodeName: "ReservoirAirRecoveryFailed"
        titleText: "T_ReservoirAirRecoveryFailed_Name"
        userDirectionText: "T_ReservoirAirRecoveryFailed_UserDirection"
        okBtnText: "T_Retry"
        cancelBtnText: "T_Eject"
        showCancelBtn: true
        showServiceBtn: true
        clearAlertWhenAck: false
        onBtnOkClicked: {
            dsWorkflow.slotSyringeAirRecoveryResume();
            close();
        }
        onBtnCancelClicked: {
            dsWorkflow.slotEjectMuds();
            close();
        }
    }

    PopupAlertBase {
        topLevel: 3
        alertCodeName: "HCUApplicationCrashed"
        titleText: "T_HCUApplicationCrashed_Name"
        userDirectionText: "T_HCUApplicationCrashed_UserDirection"
        showCancelBtn: false
        showFromStartupScreen: true
    }

    PopupAlertBase {
        topLevel: 3
        alertCodeName: "ConfigsReset"
        titleText: "T_ConfigsReset_Name"
        userDirectionText: "T_ConfigsReset_UserDirection"
        showCancelBtn: false
    }

    // ==================================== Popup: Critical
    PopupAlertBase {
        topLevel: 4
        alertCodeName: "OverCurrentFault"
        titleText: "T_OverCurrentFault_Name"
        userDirectionText: "T_OverCurrentFault_UserDirection"
        showCancelBtn: false
        showDuringServicing: true
    }

    PopupAlertBase {
        topLevel: 4
        alertCodeName: "OverPressureFault"
        titleText: "T_OverPressureFault_Name"
        userDirectionText: "T_OverPressureFault_UserDirection"
        showCancelBtn: false
        showDuringServicing: true
    }

    PopupAlertBase {
        topLevel: 4
        alertCodeName: "StopcockEngagementFault"
        titleText: "T_StopcockEngagementFault_Name"
        userDirectionText: "T_StopcockEngagementFault_UserDirection"
        okBtnText: "T_Eject"
        showCancelBtn: false
        showServiceBtn: true
        clearAlertWhenAck: false
        onBtnOkClicked: {
            dsWorkflow.slotEjectMuds();
            close();
        }
    }

    PopupAlertBase {
        topLevel: 4
        alertCodeName: "MotorPositionFault"
        titleText: "T_MotorPositionFault_Name"
        userDirectionText: "T_MotorPositionFault_UserDirection"
        okBtnText: "T_Eject"
        showCancelBtn: false
        showServiceBtn: true
        clearAlertWhenAck: false
        onBtnOkClicked: {
            dsWorkflow.slotEjectMuds();
            close();
        }
    }

    PopupAlertBase {
        // TODO: Should support one popup with multiple syringes
        topLevel: 4
        alertCodeName: "PlungerNotDetected"
        titleText: "T_PlungerNotDetected_Name"
        userDirectionText: "T_PlungerNotDetected_UserDirection"
        okBtnText: "T_Eject"
        showCancelBtn: false
        showServiceBtn: true
        clearAlertWhenAck: false
        onBtnOkClicked: {
            dsWorkflow.slotEjectMuds();
            close();
        }
    }

    PopupAlertBase {
        // TODO: Should support one popup with multiple syringes
        topLevel: 4
        alertCodeName: "PlungerEngagementFault"
        titleText: "T_PlungerEngagementFault_Name"
        userDirectionText: "T_PlungerEngagementFault_UserDirection"
        okBtnText: "T_Eject"
        showCancelBtn: false
        showServiceBtn: true
        clearAlertWhenAck: false
        onBtnOkClicked: {
            dsWorkflow.slotEjectMuds();
            close();
        }
    }

    PopupHCUSoftwareErrorAlertManager {
        id: popupHCUSoftwareErrorAlertManager
    }

    function addPopup(popup)
    {
        //logDebug("PopupManager: addPopup(): openPopups.length=" + openPopups.length);

        // Add popup with given topLevel
        var added = false;

        for (var popupIdx = 0; popupIdx < openPopups.length; popupIdx++)
        {
            if (popup.topLevel < openPopups[popupIdx].topLevel)
            {
                logDebug("PopupManager: add popup to " + popupIdx);
                openPopups.splice(popupIdx, 0, popup);
                added = true;
                break;
            }
        }

        if (!added)
        {
            //logDebug("PopupManager: add popup to " + openPopups.length);
            openPopups.push(popup);
            added = true;
        }

        if (added) {
            p.disableScreenSleep();
        }

        // Update popup layout
        for (popupIdx = 0; popupIdx < openPopups.length; popupIdx++)
        {
            //logDebug("PopupManager: addPopup(): openPopups[" + popupIdx + "].titleText=" + openPopups[popupIdx].titleText + ", popupId=" + openPopups[popupIdx].popupId + " -> " + popupIdx);
            openPopups[popupIdx].popupId = popupIdx;
        }
    }

    function removePopup(popup)
    {
        //logDebug("PopupManager: removePopup()");
        for (var popupIdx = 0; popupIdx < openPopups.length; popupIdx++)
        {
            if (openPopups[popupIdx] === popup)
            {
                //logDebug("PopupManager: Remove popup from " + popupIdx);
                openPopups.splice(popupIdx, 1);
                break;
            }
        }

        // Update popup layout
        for (popupIdx = 0; popupIdx < openPopups.length; popupIdx++)
        {
            //logDebug("PopupManager: removePopup(): openPopups[" + popupIdx + "].titleText=" + openPopups[popupIdx].titleText + ", popupId=" + openPopups[popupIdx].popupId + " -> " + popupIdx);
            openPopups[popupIdx].popupId = popupIdx;
        }

        if ((openPopups === undefined) || (openPopups.length === 0))
        {
            p.setScreenSleepDefault();
        }
    }
}
