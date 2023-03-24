import QtQuick 2.12
import "../Widgets"
import "../Widgets/Popup"

PopupMessage {
    property string workflowPreloadProtocolState: dsWorkflow.workflowPreloadProtocolState
    property var preloadStatus: dsWorkflow.preloadStatus
    property var injectionRequestProcessStatus: dsExam.injectionRequestProcessStatus
    property string examProgressState: dsExam.examProgressState

    type: "INFO"
    titleText: "T_PreloadProtocol_Title"

    onBtnOkClicked: {
        if (contentText == "T_PreloadProtocol_Message")
        {
            dsWorkflow.slotPreloadProtocolResume();
        }
        else if (contentText.indexOf("T_PreloadProtocol_Failed") >= 0)
        {
            close();
        }
    }

    onBtnCancelClicked: {
        if (contentText == "T_PreloadProtocol_Message")
        {
            dsWorkflow.slotPreloadProtocolAbort();
            close();
        }
    }

    onWorkflowPreloadProtocolStateChanged: {
        logDebug("PopupPreloadProtocol: workflowPreloadProtocolState=" + workflowPreloadProtocolState);

        if (workflowPreloadProtocolState == "PRELOAD_PROTOCOL_STATE_INACTIVE")
        {
            close();

        }
        else if (workflowPreloadProtocolState == "PRELOAD_PROTOCOL_STATE_USER_CONFIRM_WAITING")
        {
            contentText = "T_PreloadProtocol_Message";
            showOkBtn = true;
            showCancelBtn = true;
            okBtnText = "T_Preload";
            cancelBtnText = "T_Cancel";
            open();
        }
        else if (workflowPreloadProtocolState == "PRELOAD_PROTOCOL_STATE_PRELOAD_ARM_STARTED")
        {
            contentText = "T_PreloadProtocol_InProgress";
            showOkBtn = false;
            showCancelBtn = false;
            open();
        }
        else if ( (workflowPreloadProtocolState == "PRELOAD_PROTOCOL_STATE_PRELOAD_ARM_FAILED") ||
                  (workflowPreloadProtocolState == "PRELOAD_PROTOCOL_STATE_PRELOAD_INJECTION_FAILED") )
        {
            contentText = Qt.binding(function() {
                var reason = preloadStatus.Err;
                if (reason === "USER_ABORT")
                    reason = "T_PreloadProtocol_UserAborted";
                else if (reason === "SUDS_REMOVED")
                    reason = "T_PreloadProtocol_SudsRemoved";
                else if (reason === "PISTON_MOVING")
                    reason = "T_PreloadProtocol_InjectorBusy";
                return "T_PreloadProtocol_Failed;" + reason;
            });
            okBtnText = "T_OK";
            showOkBtn = true;
            showCancelBtn = false;
        }
        else if (workflowPreloadProtocolState == "PRELOAD_PROTOCOL_STATE_DONE")
        {
            close();
        }
    }

    onInjectionRequestProcessStatusChanged: {
        // When arm failed, in some cases, close the current popup as a separate popup shall appear.
        if (!isOpen())
        {
            return;
        }

        if (injectionRequestProcessStatus === undefined)
        {
            return;
        }

        if (injectionRequestProcessStatus.State.indexOf("T_ARMFAILED") < 0)
        {
            // arm not failed
            return;
        }

        if (injectionRequestProcessStatus.State.indexOf("T_ARMFAILED_InsufficientVolume") >= 0)
        {
            // Will be handled in PopupArmFluidVolumeAdjust
            close();
        }
    }

    onExamProgressStateChanged: {
        if (isOpen()) {
            if (examProgressState === "Completed") {
                dsWorkflow.slotPreloadProtocolAbort();
                close();
            } else if (examProgressState === "Completing") {
                close();
            }
        }
    }
}
