import QtQuick 2.12
import "../Widgets"
import "../Widgets/Popup"
import "../Util.js" as Util

PopupMessage {
    property string workflowMudsEjectState: dsWorkflow.workflowMudsEjectState
    property string statePath: dsSystem.statePath

    type: "INFO"
    titleText: "T_MudsEject_Title"

    onBtnOkClicked: {
        logDebug("PopupMudsEject: onBtnOkClicked()");

        if (contentText === "T_MudsEject_Confirmation")
        {
            dsWorkflow.slotEjectMuds();
        }
        else if (contentText === "T_MudsEject_AllStopPressed")
        {
            dsWorkflow.slotEjectMuds();
        }
        else if (contentText === "T_MudsEject_FailedBySystemError")
        {
            close();
        }
        else if (contentText === "T_MudsEject_Disengaged")
        {
            close();
        }
    }

    onBtnCancelClicked: {
        logDebug("PopupMudsEject: onBtnCancelClicked()");
        if (contentText === "T_MudsEject_Confirmation")
        {
            close();
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
                logDebug("PopupMudsEject: onStatePathChanged: statePath=" + statePath + ", closing..");
                close();
            }
        }
    }

    onWorkflowMudsEjectStateChanged: {
        if ( (workflowMudsEjectState === "MUDS_EJECT_STATE_STARTED") &&
            (!isOpen()) )
        {
            // Muds eject is triggered, open popup if closed
            logDebug("PopupMudsEject: onWorkflowMudsEjectStateChanged(): workflowMudsEjectState=" + workflowMudsEjectState + ", opening..");
            open();
        }
        reload();
    }

    function reload()
    {
        //logDebug("workflowMudsEjectState=" + workflowMudsEjectState);
        if (!visible)
        {
            return;
        }

        logDebug("PopupMudsEject: reload(): workflowMudsEjectState=" + workflowMudsEjectState);

        if (workflowMudsEjectState === "MUDS_EJECT_STATE_STARTED")
        {
            contentText = "T_MudsEject_Progress";
            showOkBtn = false;
            showCancelBtn = false;
            showServiceBtn = false;
        }
        else if (workflowMudsEjectState === "MUDS_EJECT_STATE_FAILED_USER_ABORT")
        {
            contentText = "T_MudsEject_AllStopPressed";
            showOkBtn = true;
            showCancelBtn = false;
            showServiceBtn = true;
            okBtnText = "T_Continue";
        }
        else if (workflowMudsEjectState === "MUDS_EJECT_STATE_FAILED_SYSTEM_ERROR")
        {
            contentText = "T_MudsEject_FailedBySystemError";
            showOkBtn = true;
            showCancelBtn = false;
            showServiceBtn = true;
            okBtnText = "T_OK";
        }
        else if (workflowMudsEjectState === "MUDS_EJECT_STATE_DISENGAGE_DONE")
        {
            contentText = "T_MudsEject_Disengaged";
            showOkBtn = true;
            showCancelBtn = false;
            showServiceBtn = false;
            okBtnText = "T_OK";
        }
        else if (workflowMudsEjectState === "MUDS_EJECT_STATE_DONE")
        {
            close();
        }
    }

    function init()
    {
        logDebug("PopupMudsEject: init(): workflowMudsEjectState=" + workflowMudsEjectState);

        if (workflowMudsEjectState == "MUDS_EJECT_STATE_READY")
        {
            // Ask user to start the MUDS Eject
            contentText = "T_MudsEject_Confirmation";
            showOkBtn = true;
            showCancelBtn = true;
            showServiceBtn = false;
            okBtnText = "T_OK";
            cancelBtnText = "T_Cancel";
        }
        else
        {
            contentText = "T_MudsEject_Disengaged";
            showOkBtn = true;
            showCancelBtn = false;
            showServiceBtn = false;
            okBtnText = "T_OK";
        }
    }
}

