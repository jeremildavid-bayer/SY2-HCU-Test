import QtQuick 2.12
import "../Widgets"
import "../Widgets/Popup"

PopupMessage {
    property string sodErrorState: dsWorkflow.sodErrorState
    property string statePath: dsSystem.statePath
    property string lastStatePath: dsSystem.lastStatePath
    property string screenState: appMain.screenState
    property string screenStatePrev: appMain.screenStatePrev

    type: "WARNING"

    onScreenStateChanged: {
        if (screenState.indexOf("Admin-Service-") >= 0)
        {
            close();
        }
    }

    onSodErrorStateChanged: {
        reload();
    }

    onBtnOkClicked: {
        if ( (contentText.indexOf("T_MudsPrimeUserAborted_Message") >= 0) ||
             (contentText.indexOf("T_MudsPrimeFailed_UserDirection") >= 0) ||
             (contentText.indexOf("T_MudsPrimeFailedAirDetected_Message") >= 0) )
        {
            if (okBtnText == "T_Retry")
            {
                dsWorkflow.slotSodResume();
            }
        }
        else if ( (contentText.indexOf("T_FindPlungersUserAborted_Message") >= 0) ||
                  (contentText.indexOf("T_PurgeStopped_Message") >= 0) ||
                  (contentText.indexOf("T_ReservoirAirCheckCalFailed_UserDirection") >= 0) )
        {
            dsWorkflow.slotSodResume();
        }
        close();
    }

    onBtnCancelClicked: {
        if ( (contentText.indexOf("T_UsedMUDSDetected_UserDirection") >= 0) ||
             (contentText.indexOf("T_FindPlungersUserAborted_Message") >= 0) ||
             (contentText.indexOf("T_PurgeStopped_Message") >= 0) ||
             (contentText.indexOf("T_MudsPrimeUserAborted_Message") >= 0) ||
             (contentText.indexOf("T_MudsPrimeFailed_UserDirection") >= 0) ||
             (contentText.indexOf("T_MudsPrimeFailedAirDetected_Message") >= 0) ||
             (contentText.indexOf("T_ReservoirAirCheckCalFailed_UserDirection") >= 0) )
        {
            dsWorkflow.slotEjectMuds();
        }
        close();
    }

    function reload()
    {
        handleSodError();
    }

    function handleSodError()
    {
        logDebug("PopupSODError: SodErrorState=" + sodErrorState);

        if (sodErrorState === "SOD_ERROR_STATE_NONE")
        {
            close();
        }
        else if (sodErrorState === "SOD_ERROR_STATE_USER_ABORT")
        {
            close();
        }
        else if (sodErrorState === "SOD_ERROR_STATE_MUDS_LATCH_LIFTED")
        {
            close();
        }
        else if (sodErrorState === "SOD_ERROR_STATE_SERVICE_ABORT")
        {
            close();
        }
        else if (sodErrorState === "SOD_ERROR_STATE_MUDS_EJECTED")
        {
            close();
        }
        else if (sodErrorState === "SOD_ERROR_STATE_USED_MUDS_DETECTED")
        {
            titleText = "T_UsedMUDSDetected_Name";
            contentText = "T_UsedMUDSDetected_UserDirection";
            cancelBtnText = "T_Eject";
            showOkBtn = false;
            showCancelBtn = true;
            showServiceBtn = true;
            open();
        }
        else if (sodErrorState === "SOD_ERROR_STATE_FIND_PLUNGERS_USER_ABORT")
        {
            titleText = "T_FindPlungersUserAborted_Title";
            contentText = "T_FindPlungersUserAborted_Message";
            okBtnText = "T_Continue";
            showOkBtn = true;
            cancelBtnText = "T_Eject";
            showCancelBtn = true;
            showServiceBtn = true;
            open();
        }
        else if (sodErrorState === "SOD_ERROR_STATE_PURGE_FAILED")
        {
            titleText = "T_PurgeStopped_Title";
            contentText = "T_PurgeStopped_Message";
            okBtnText = "T_Continue";
            showOkBtn = true;
            showCancelBtn = true;
            cancelBtnText = "T_Eject";
            showServiceBtn = true;
            open();
        }
        else if (sodErrorState === "SOD_ERROR_STATE_PURGE_FAILED_SUDS_INSERTED")
        {
            titleText = "T_PurgeStopped_Title";
            contentText = "T_PurgeStoppedRemoveSuds_Message";
            showCancelBtn = false;
            showOkBtn = false;
            showServiceBtn = false;
            open();
        }
        else if (sodErrorState === "SOD_ERROR_STATE_PURGE_FAILED_MUDS_REMOVED")
        {
            // SOD cannot complete, should be handled by eject sequence
            close();
        }
        else if (sodErrorState === "SOD_ERROR_STATE_ENGAGE_FAILED")
        {
            // SOD cannot complete, should be handled by alert
            close();
        }
        else if (sodErrorState === "SOD_ERROR_STATE_PURGE_USER_ABORT")
        {
            titleText = "T_PurgeStopped_Title";
            contentText = "T_PurgeStopped_Message";
            okBtnText = "T_Continue";
            showOkBtn = true;
            showCancelBtn = true;
            cancelBtnText = "T_Eject";
            showServiceBtn = true;
            open();
        }
        else if (sodErrorState === "SOD_ERROR_STATE_REUSE_MUDS_WAITING_SUDS_REMOVAL")
        {
            titleText = "T_ReuseMudsWaitingSudsRemoval_Title";
            contentText = "T_ReuseMudsWaitingSudsRemoval_Message";
            showCancelBtn = false;
            showOkBtn = false;
            showServiceBtn = false;
            open();
        }
        else if ( (sodErrorState === "SOD_ERROR_STATE_SYRINGE_PRIME_ABORT") ||
                  (sodErrorState === "SOD_ERROR_STATE_SYRINGE_PRIME_FAILED_HW_FAULT") ||
                  (sodErrorState === "SOD_ERROR_STATE_SYRINGE_PRIME_FAILED_AIR_DETECTED") ||
                  (sodErrorState === "SOD_ERROR_STATE_SYRINGE_PRIME_FAILED_SUDS_REMOVED") )
        {
            var syringeIdxStr = dsAlert.slotGetAlertFromCodeName("MudsPrimeFailed").Data;

            titleText = "T_MudsPrimeFailed_Name";

            logDebug("PopupSODError: SodErrorState=" + sodErrorState + ", Syringe=" + syringeIdxStr);

            if (sodErrorState === "SOD_ERROR_STATE_SYRINGE_PRIME_ABORT")
            {
                contentText = "T_MudsPrimeUserAborted_Message";
            }
            else if (sodErrorState === "SOD_ERROR_STATE_SYRINGE_PRIME_FAILED_HW_FAULT")
            {
                contentText = "T_MudsPrimeFailed_UserDirection;" + syringeIdxStr;
            }
            else if (sodErrorState === "SOD_ERROR_STATE_SYRINGE_PRIME_FAILED_AIR_DETECTED")
            {
                contentText = "T_MudsPrimeFailedAirDetected_Message";
            }
            else if (sodErrorState === "SOD_ERROR_STATE_SYRINGE_PRIME_FAILED_SUDS_REMOVED")
            {
                contentText = "T_MudsPrimeFailedSudsRemoved_Message";
            }

            if ( (sodErrorState === "SOD_ERROR_STATE_SYRINGE_PRIME_ABORT") ||
                 (sodErrorState === "SOD_ERROR_STATE_SYRINGE_PRIME_FAILED_HW_FAULT") ||
                 (sodErrorState === "SOD_ERROR_STATE_SYRINGE_PRIME_FAILED_AIR_DETECTED") )
            {
                // Allow user to retry if possible
                showCancelBtn = true;
                showOkBtn = true;
                okBtnText = dsDevice.slotGetSyringeSodStartReady(syringeIdxStr) ? "T_Retry" : "T_OK";
                cancelBtnText = "T_Eject";
                showServiceBtn = (sodErrorState !== "SOD_ERROR_STATE_SYRINGE_PRIME_ABORT");
            }
            else
            {
                showCancelBtn = false;
                showOkBtn = true;
                okBtnText = "T_OK";
                showServiceBtn = false;
            }
            open();
        }
        else if (sodErrorState === "SOD_ERROR_STATE_CAL_SYRINGE_AIR_CHECK_FAILED_BAD_DATA")
        {
            titleText = "T_ReservoirAirCheckCalFailed_Name";
            contentText = "T_ReservoirAirCheckCalFailed_UserDirection;" + dsAlert.slotGetActiveAlertFromCodeName("ReservoirAirCheckCalFailed").Data;
            showCancelBtn = true;
            showOkBtn = true;
            okBtnText = "T_Retry";
            cancelBtnText = "T_Eject";
            showServiceBtn = true;
            open();
        }
        else if (sodErrorState === "SOD_ERROR_STATE_CAL_SYRINGE_AIR_CHECK_FAILED_HW_FAULT")
        {
            titleText = "T_ReservoirAirCheckCalFailed_Name";
            contentText = "T_ReservoirAirCheckCalFailed_UserDirection;" + dsAlert.slotGetActiveAlertFromCodeName("ReservoirAirCheckCalFailed").Data;
            showCancelBtn = true;
            showOkBtn = false;
            cancelBtnText = "T_Eject";
            showServiceBtn = true;
            open();
        }
        else
        {
            // TODO: Needs proper translation
            titleText = "Unexpected SOD Error Occurred";
            contentText = sodErrorState;
            showCancelBtn = false;
            showOkBtn = true;
            showServiceBtn = false;
            logWarning("PopupSODError: Unexpected sodErrorState=" + sodErrorState);
        }
    }
}
