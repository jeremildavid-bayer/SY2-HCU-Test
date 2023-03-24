import QtQuick 2.12
import "../Widgets"
import "../Widgets/Popup"

PopupMessage {
    property var injectionRequestProcessStatus: dsExam.injectionRequestProcessStatus
    property string examProgressState: dsExam.examProgressState

    type: "SPECIAL_WARNING"
    titleText: "T_InsufficientVolumeForSteps_Title"
    contentText: "T_InsufficientVolumeForSteps_Message"

    // OtherBtn will be used for "Yes" to swap "Yes" and "No" buttons
    showOtherBtn: true
    enableOtherBtn: true
    showOkBtn: false
    enableOkBtn: false

    otherBtnText: "T_Yes"
    cancelBtnText: "T_No"

    onBtnOtherClicked: {
        // This is "Yes" button
        dsAlert.slotActivateAlert("InsufficientVolumeForStepsAccepted", dataText);
        close();
        logInfo("PopupInjectionRequestFailed: Insufficient volume accepted by user. Arm again..");
        dsExam.slotInjectionArmed();
    }

    onBtnCancelClicked: {
        // this file was separated out from PopupInjectionRequestFailed as the behaviour was very different to rest.
        // below de-activating of the alert was part of the original. Not sure if this is needed but having this ensures original behavior
        dsAlert.slotDeactivateAlert("CatheterLimitsExceededAccepted", "");

        close();
    }

    onExamProgressStateChanged:  {
        // the popup should not show outside of exam
        if (examProgressState == "Idle" && isOpen()) {
            close();
        }
    }

    onInjectionRequestProcessStatusChanged: {
        if (injectionRequestProcessStatus === undefined) {
            return;
        }
        // Ensure that 'T_ARMFAILED_InsufficientVolumeForSteps' is evaluated prior to 'T_ARMFAILED_InsufficientVolume' due to same string contents.
        else if ( (injectionRequestProcessStatus.State.indexOf("T_ARMFAILED_InsufficientVolumeForSteps") >= 0) &&
                  (injectionRequestProcessStatus.RequestedByHcu) ) {
            // Expected state string is "T_ARMFAILED_InsufficientVolumeForSteps <stepIdx>;<planContrastVol>;<planSalineVol>;<syringeContrastVol>;<syringeSalineVol>;<planGuid>"
            dataText = injectionRequestProcessStatus.State.replace("T_ARMFAILED_InsufficientVolumeForSteps ", "");
            open();
        }
    }
}
