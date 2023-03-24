import QtQuick 2.12
import "../Widgets"
import "../Widgets/Popup"

PopupMessage {
    property var injectionRequestProcessStatus: dsExam.injectionRequestProcessStatus
    property bool isAirCheckNeeded: dsExam.isAirCheckNeeded
    property bool sudsInserted: dsMcu.sudsInserted
    property string workflowPreloadProtocolState: dsWorkflow.workflowPreloadProtocolState
    property bool licenseEnabledWorklistSelection: dsCru.licenseEnabledWorklistSelection
    property string examProgressState: dsExam.examProgressState

    type: "PLAIN"
    showCancelBtn: false

    onBtnOkClicked: {
        if (contentText == "T_HaveYouCheckedForAirInThePatientLine")
        {
            dsExam.slotAirCheckDone();
            dsExam.slotInjectionArmed();
        }
        else if (contentText == "T_ScannerInterlocksBypassPrompt_Message")
        {
            dsExam.slotScannerInterlocksCheckDone();
            dsExam.slotInjectionArmed();
        }
        else if (contentText == "T_ARMFAILED_ExamNotStarted")
        {
            // Select anonymous. Note, the exam shall be started when selection completed
            dsMwl.slotSelectWorklistEntry("00000000-0000-0000-0000-000000000000");
        }
        else if (contentText.indexOf("T_StartExamFor_XXX") >= 0)
        {
            // patient is selected (without starting exam). Quick step to start exam with chosen patient
            dsExam.slotExamProgressStateChanged("Started");
            close();
        }
        else
        {
            close();
        }
    }

    onBtnCancelClicked: {
        if (contentText == "T_ARMFAILED_ExamNotStarted")
        {
            if (licenseEnabledWorklistSelection)
            {
                logDebug("PopupInjectionRequestFailed: Arm failed (" + contentText + "): WorkList option selected: Disarm and Navigate to Patient Selection");
                // Disarm to clear InjectionRequestProcessStatus
                dsExam.slotInjectionAborted();

                // Select worklist
                examManager.slotNavPatient();
            }
            else
            {
                logDebug("PopupInjectionRequestFailed: Arm failed (" + contentText + "): Cancel option selected: Doing nothing.");
                close();
            }

        }
        else if (contentText.indexOf("T_StartExamFor_XXX") >= 0)
        {
            // fall back to T_ARMFAILED_ExamNotStarted only popup
            // close and re-open with properties changed
            close();

            contentText = "T_ARMFAILED_ExamNotStarted";
            okBtnText = "T_Anonymous";
            cancelBtnText = licenseEnabledWorklistSelection ? "T_Worklist" : "T_Cancel";
            showCancelBtn = true;
            open();
        }
        else
        {
            dsAlert.slotDeactivateAlert("CatheterLimitsExceededAccepted", "");
            close();
        }
    }

    onSudsInsertedChanged: {
        if ( (sudsInserted) &&
             (isOpen()) &&
             (contentText == "T_HaveYouCheckedForAirInThePatientLine") )
        {
            // SUDS is re-inserted. close popup
            close();
        }
    }

    onIsAirCheckNeededChanged: {
        if (contentText == "T_HaveYouCheckedForAirInThePatientLine")
        {
            if (!isAirCheckNeeded && isOpen())
            {
                close();
            }
        }
    }

    onExamProgressStateChanged: {
        // if exam is started while exam not started popup is shown, close it (CRU started exam)
        if ((examProgressState == "Started") && ((contentText.indexOf("T_ARMFAILED_ExamNotStarted") >= 0) || (contentText.indexOf("T_StartExamFor_XXX") >= 0)) && isOpen())
        {
            close();
        }
        // the popup should not show outside of exam
        else if (examProgressState == "Idle" && isOpen())
        {
            close();
        }
    }

    onInjectionRequestProcessStatusChanged: {
        if (injectionRequestProcessStatus === undefined)
        {
            return;
        }

        if (workflowPreloadProtocolState !== "PRELOAD_PROTOCOL_STATE_READY")
        {
            // No popup if current step is preloading
            close();
        }
        else if ( (injectionRequestProcessStatus.State.indexOf("T_ARM") < 0) &&
                  (injectionRequestProcessStatus.State.indexOf("T_INJECTSTART") < 0) )
        {
            // Request type is not ARM nor INJECTSTART
            close();
        }
        else if ( (injectionRequestProcessStatus.State === "T_ARMING") ||
                  (injectionRequestProcessStatus.State === "T_ARMED") ||
                  (injectionRequestProcessStatus.State === "T_INJECTSTARTING") ||
                  (injectionRequestProcessStatus.State === "T_INJECTSTARTED") )
        {
            // Request is processing
            close();
        }       
        else if (!injectionRequestProcessStatus.RequestedByHcu)
        {
            // Request is sent from other(e.g. CRU) and failed.
            close();
        }
        else if (injectionRequestProcessStatus.State.indexOf("T_ARMFAILED_CatheterCheckNeeded") >= 0)
        {
            // Will be handled in PopupArmCatheterLimitCheck
            close();
        }
        // Ensure that 'T_ARMFAILED_InsufficientVolumeForSteps' is evaluated prior to 'T_ARMFAILED_InsufficientVolume' due to same string contents.
        else if (injectionRequestProcessStatus.State.indexOf("T_ARMFAILED_InsufficientVolumeForSteps") >= 0)
        {
            // Expected state string is "T_ARMFAILED_InsufficientVolumeForSteps <stepIdx>;<planContrastVol>;<planSalineVol>;<syringeContrastVol>;<syringeSalineVol>;<planGuid>"
            // Will be handled in PopupArmInsufficientVolumesForSteps
            close();
        }
        else if (injectionRequestProcessStatus.State.indexOf("T_ARMFAILED_InsufficientVolume") >= 0)
        {
            // Will be handled in PopupArmFluidVolumeAdjust
            close();
        }
        else if (injectionRequestProcessStatus.State.indexOf("T_ARMFAILED_ExamNotStarted") >= 0)
        {
            // there are two T_ARMFAILED_ExamNotStarted scenarios
            // just "T_ARMFAILED_ExamNotStarted" case - no patient is selected
            // "T_ARMFAILED_ExamNotStarted T_StartExamFor_XXX" case - (note space inbetween) patient is selected
            // T_StartExamFor_XXX was added later.. apologies for messy logic
            type = "PLAIN";
            var errorStrings = injectionRequestProcessStatus.State.split(" ");

            if (errorStrings.length > 1)
            {
                contentText = errorStrings[1] + ";" + dsExam.examAdvanceInfo.WorklistDetails.Entry.DicomFields.patientName.Value;
                okBtnText = "T_Yes";
                cancelBtnText = "T_No";
                showCancelBtn = true;
            }
            else
            {
                contentText = errorStrings[0];
                okBtnText = "T_Anonymous";
                cancelBtnText = licenseEnabledWorklistSelection ? "T_Worklist" : "T_Cancel";
                showCancelBtn = true;
            }
            open();
        }
        else if (injectionRequestProcessStatus.State.indexOf("T_ARMFAILED_AirCheckNeeded") >= 0)
        {
            type = "PLAIN";
            contentText = "T_HaveYouCheckedForAirInThePatientLine";
            okBtnText = "T_Yes";
            cancelBtnText = "T_No";
            showCancelBtn = true;
            open();
        }
        else if (injectionRequestProcessStatus.State.indexOf("T_ARMFAILED_ScannerInterlocksBypassed") >= 0)
        {
            type = "WARNING";
            titleText = "T_ScannerInterlocksBypassPrompt_Title";
            contentText = "T_ScannerInterlocksBypassPrompt_Message";
            okBtnText = "T_Yes";
            cancelBtnText = "T_No";
            showCancelBtn = true;
            open();
        }
        else
        {
            type = "WARNING";

            if (injectionRequestProcessStatus.State.indexOf("T_ARM") >= 0)
            {
                titleText = "T_ARMFAILED";
            }
            else if (injectionRequestProcessStatus.State.indexOf("T_INJECTSTART") >= 0)
            {
                titleText = "T_INJECTSTARTFAILED";
            }
            else
            {
                titleText = "";
            }

            contentText = injectionRequestProcessStatus.State;
            okBtnText = "T_OK";
            showCancelBtn = false;
            open();
        }
    }
}

