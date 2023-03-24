import QtQuick 2.12
import "../Widgets"
import "../Widgets/Popup"

PopupMessage {
    property string workflowEndOfDayPurgeState: dsWorkflow.workflowEndOfDayPurgeState
    property string statePath: dsSystem.statePath
    property bool sudsInserted: dsMcu.sudsInserted

    type: "INFO"
    titleText: "T_EmptyDaySet_Title";

    onBtnOkClicked: {
        if (contentText === "T_EmptyDaySet_Complete")
        {
            dsWorkflow.slotEjectMuds();
            close();
        }
        else if ( (contentText.indexOf("T_EmptyDaySet_Failed") >= 0) ||
                  (contentText.indexOf("T_EmptyDaySet_SudsRemoved") >= 0) ||
                  (contentText.indexOf("T_EmptyDaySet_WasteContainerNotReady") >= 0) ||
                  (contentText.indexOf("T_EmptyDaySet_UserAborted") >= 0) )
        {
            close();
        }
        else
        {
            dsWorkflow.slotEndOfDayPurgeResume();
        }
    }

    onBtnCancelClicked: {
        if (contentText === "T_EmptyDaySet_Complete")
        {
            close();
        }
        else
        {
            dsWorkflow.slotEndOfDayPurgeAbort();
        }
    }

    onOpened: {
        // Clear previous popup information
        type = "INFO";
        contentText = "";
        showCancelBtn = false;
        showOkBtn = false;

        var status = dsWorkflow.slotEndOfDayPurgeStart();
        if ( (status.State !== "Waiting") &&
             (status.State !== "Started") )
        {
            contentText = "T_EmptyDaySet_Failed;" + status.State + ": " + status.Err;
            showCancelBtn = false;
            showOkBtn = true;
        }
    }

    onStatePathChanged: {
        if ( (statePath === "Ready/Armed") ||
             (statePath === "Executing") ||
             (statePath === "Error") )
        {
            if (isOpen())
            {
                // Abort handled in arming action
                dsWorkflow.slotEndOfDayPurgeAbort();
                close();
            }
        }
    }

    onSudsInsertedChanged: {
        if ( (sudsInserted) &&
             (contentText == "T_EmptyDaySet_SudsRemoved") )
        {
            close();
        }
    }

    onWorkflowEndOfDayPurgeStateChanged: {
        reload();
    }

    function reload()
    {
        if (!visible)
        {
            return;
        }

        if (workflowEndOfDayPurgeState === "END_OF_DAY_PURGE_STATE_WAIT_USER_START")
        {
            type = "INFO";
            contentText = "T_EmptyDaySet_Message";
            showOkBtn = true;
            showCancelBtn = true;
            okBtnText = "T_OK";
            cancelBtnText = "T_Cancel";
        }
        else if (workflowEndOfDayPurgeState === "END_OF_DAY_PURGE_STATE_WAIT_USER_CONFIRM_EMPTY_SALINE")
        {
            type = "INFO";
            contentText = "T_EmptyDaySet_ContinueEmptySaline";
            showCancelBtn = true;
            showOkBtn = true;
            cancelBtnText = "T_Abort";
        }
        else if (workflowEndOfDayPurgeState === "END_OF_DAY_PURGE_STATE_WAIT_SUDS_TO_WASTE_CONTAINER")
        {
            type = "INFO";
            contentText = "T_EmptyDaySet_HoldPatientLineOverWasteBin";
            showCancelBtn = true;
            showOkBtn = true;
            cancelBtnText = "T_Abort";
        }
        else if (workflowEndOfDayPurgeState === "END_OF_DAY_PURGE_STATE_WAIT_INSERT_SUDS")
        {
            type = "INFO";
            contentText = "T_EmptyDaySet_InsertSuds";
            showOkBtn = false;
            showCancelBtn = true;
            cancelBtnText = "T_Abort";
        }
        else if (workflowEndOfDayPurgeState === "END_OF_DAY_PURGE_STATE_FAILED_SUDS_REMOVED")
        {
            type = "WARNING";
            contentText = "T_EmptyDaySet_SudsRemoved";
            showOkBtn = true;
            showCancelBtn = false;
            okBtnText = "T_OK";
        }
        else if (workflowEndOfDayPurgeState === "END_OF_DAY_PURGE_STATE_FAILED_WASTE_CONTAINER_NOT_READY")
        {
            type = "WARNING";
            contentText = "T_EmptyDaySet_WasteContainerNotReady";
            showOkBtn = true;
            showCancelBtn = false;
            okBtnText = "T_OK";
        }
        else if (workflowEndOfDayPurgeState === "END_OF_DAY_PURGE_STATE_FAILED_USER_ABORT")
        {
            type = "WARNING";
            contentText = "T_EmptyDaySet_UserAborted";
            showOkBtn = true;
            showCancelBtn = false;
            okBtnText = "T_OK";
        }
        else if ( (workflowEndOfDayPurgeState === "END_OF_DAY_PURGE_STATE_STOPCOCK_INJECT_SETTING") ||
                  (workflowEndOfDayPurgeState === "END_OF_DAY_PURGE_STATE_PUSH_PLUNGERS_PROGRESS") )
        {
            type = "INFO";
            contentText = "T_EmptyDaySet_Emptying";
            showCancelBtn = false;
            showOkBtn = false;
        }
        else if (workflowEndOfDayPurgeState === "END_OF_DAY_PURGE_STATE_PUSH_PLUNGERS_FAILED")
        {
            type = "WARNING";
            contentText = "T_EmptyDaySet_Failed;" + "PUSH_PLUNGERS_FAILED";
            showCancelBtn = false;
            showOkBtn = true;
        }
        else if (workflowEndOfDayPurgeState === "END_OF_DAY_PURGE_STATE_DONE")
        {
            type = "INFO";
            contentText = "T_EmptyDaySet_Complete";
            showCancelBtn = true;
            showOkBtn = true;
            okBtnText = "T_Yes";
            cancelBtnText = "T_No";
        }
    }
}
