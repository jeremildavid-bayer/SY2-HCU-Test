import QtQuick 2.12
import "../Widgets"
import "../Widgets/Popup"
import "../Util.js" as Util

PopupMessage {
    property int syringeIndex
    property string workflowAutoEmptyState: dsWorkflow.workflowAutoEmptyState
    property string workflowFluidRemovalState: dsWorkflow.workflowFluidRemovalState
    property var fluidSourceBottle1: dsDevice.fluidSourceBottle1
    property var fluidSourceBottle2: dsDevice.fluidSourceBottle2
    property var fluidSourceBottle3: dsDevice.fluidSourceBottle3
    property var fluidSourceSyringe1: dsDevice.fluidSourceSyringe1
    property var fluidSourceSyringe2: dsDevice.fluidSourceSyringe2
    property var fluidSourceSyringe3: dsDevice.fluidSourceSyringe3
    property string statePath: dsSystem.statePath
    property bool sudsInserted: dsMcu.sudsInserted

    type: "INFO"
    titleText: {
        if ( (workflowAutoEmptyState === "AUTO_EMPTY_STATE_FLUID_REMOVAL_STARTED") ||
             (workflowAutoEmptyState === "AUTO_EMPTY_STATE_FLUID_REMOVAL_PROGRESS") ||
             (workflowAutoEmptyState === "AUTO_EMPTY_STATE_FLUID_REMOVAL_DONE") ||
             (workflowAutoEmptyState === "AUTO_EMPTY_STATE_FLUID_REMOVAL_FAILED") ) {

            // Fluid removal is initiated as part of Auto Empty
            return (type == "WARNING") ? "T_AutoEmpty_FailedTitle" : "T_AutoEmpty_Name";
        }

        return (type == "WARNING") ? "T_FluidRemoval_FailedTitle" : "T_FluidRemoval_Name";
    }

    onBtnOkClicked: {
        if ( (contentText.indexOf("T_FluidRemoval_Confirmation") >= 0) ||
             (contentText.indexOf("T_FluidRemoval_RemoveCurrentSupply") >= 0) ||
             (contentText.indexOf("T_FluidRemoval_HoldPatientLineOverWasteBin") >= 0) ||
             (contentText.indexOf("T_FluidRemoval_Interrupted") >= 0) ||
             (contentText.indexOf("T_FluidRemoval_SupplyStillLoaded") >= 0) ||
             (contentText.indexOf("T_FluidRemoval_HwFailed") >= 0) )
        {
            logDebug("PopupFluidRemoval: Resuming Fluid Removal..");
            dsWorkflow.slotFluidRemovalResume();
        }
        else if ( (contentText.indexOf("T_FluidRemovalFailed_InvalidState") >= 0) ||
                  (contentText.indexOf("T_FluidRemovalFailed_OtherSyringeBusy") >= 0) )
        {
            logDebug("PopupFluidRemoval: Aborting Fluid Removal..");
            dsWorkflow.slotFluidRemovalAbort();
            close();
        }
        else
        {
            close();
        }
    }

    onBtnCancelClicked: {
        dsWorkflow.slotFluidRemovalAbort();
        close();
    }

    onStatePathChanged: {
        if ( (statePath === "Ready/Armed") ||
             (statePath === "Executing") ||
             (statePath === "Error") )
        {
            if (isOpen())
            {
                // Abort handled in arming action
                dsWorkflow.slotFluidRemovalAbort();
                close();
            }
        }
    }

    onSudsInsertedChanged: {
        if ( (sudsInserted) &&
             (contentText == "T_FluidRemoval_SudsRemoved") )
        {
            close();
        }
    }

    onWorkflowFluidRemovalStateChanged: {
        logDebug("PopupFluidRemoval: workflowFluidRemovalState=" + workflowFluidRemovalState);
        reload();
    }

    onWorkflowAutoEmptyStateChanged: {
        logDebug("PopupFluidRemoval: workflowAutoEmptyState=" + workflowAutoEmptyState);
        reload();
    }

    function reload()
    {
        if (workflowFluidRemovalState.indexOf("FLUID_REMOVAL_STATE_REMOVING_TRAPPED_FLUID_ABORTED_BY") >= 0)
        {
            // Need to process regardless of the AutoEmptyMode
        }
        else if ( (workflowAutoEmptyState === "AUTO_EMPTY_STATE_FLUID_REMOVAL_PROGRESS") ||
                  (workflowAutoEmptyState === "AUTO_EMPTY_STATE_FLUID_REMOVAL_DONE") ||
                  (workflowAutoEmptyState === "AUTO_EMPTY_STATE_DONE") )
        {
            // AutoEmpty mode: Don't show popup
            close();
            return;
        }

        if (workflowFluidRemovalState === "FLUID_REMOVAL_STATE_FAILED_DURING_REMOVING_TRAPPED_FLUID")
        {
            // FluidRemoval failed, no confirmation required
            close();
            return;
        }

        if ( (workflowFluidRemovalState === "FLUID_REMOVAL_STATE_FAILED") ||
             (workflowFluidRemovalState === "FLUID_REMOVAL_STATE_FILL_STOPCOCK_FILL_SETTING") ||
             (workflowFluidRemovalState === "FLUID_REMOVAL_STATE_FILL_STOPCOCK_FILL_PROGRESS") ||
             (workflowFluidRemovalState === "FLUID_REMOVAL_STATE_FILL_STOPCOCK_FILL_DONE") ||
             (workflowFluidRemovalState === "FLUID_REMOVAL_STATE_FILL_STOPCOCK_FILL_FAILED") ||
             (workflowFluidRemovalState === "FLUID_REMOVAL_STATE_FILL_DONE") ||
             (workflowFluidRemovalState === "FLUID_REMOVAL_STATE_FILL_FAILED") ||
             (workflowFluidRemovalState === "FLUID_REMOVAL_STATE_PURGE_DONE") )
        {
            // do nothing
            return;
        }

        var popupShouldVisible = true;

        if (workflowFluidRemovalState === "FLUID_REMOVAL_STATE_FAILED_INVALID_STATE")
        {
            type = "WARNING";
            contentText = "T_FluidRemovalFailed_InvalidState";
            showOkBtn = true;
            showCancelBtn = false;
        }
        else if (workflowFluidRemovalState === "FLUID_REMOVAL_STATE_FAILED_OTHER_SYRINGE_BUSY")
        {
            type = "WARNING";
            contentText = "T_FluidRemovalFailed_OtherSyringeBusy";
            showOkBtn = true;
            showCancelBtn = false;
        }
        else if (workflowFluidRemovalState === "FLUID_REMOVAL_STATE_WAIT_USER_START")
        {
            var volume;
            var fluidType;
            var fluidSourceBottle;
            var fluidSourceSyringe;

            if (syringeIndex === 0)
            {
                fluidSourceBottle = fluidSourceBottle1;
                fluidSourceSyringe = fluidSourceSyringe1;
            }
            else if (syringeIndex === 1)
            {
                fluidSourceBottle = fluidSourceBottle2;
                fluidSourceSyringe = fluidSourceSyringe2;
            }
            else
            {
                fluidSourceBottle = fluidSourceBottle3;
                fluidSourceSyringe = fluidSourceSyringe3;
            }

            volume = Math.floor(fluidSourceSyringe.CurrentVolumes[0]);
            fluidType = fluidSourceBottle.SourcePackages[0].Brand + " " + fluidSourceBottle.SourcePackages[0].Concentration;

            type = "INFO";

            contentText = "T_FluidRemoval_Confirmation" + ";" + volume + ";" + fluidType;
            showOkBtn = true;
            showCancelBtn = true;
            okBtnText = "T_OK";
            cancelBtnText = "T_Cancel";
        }
        else if (workflowFluidRemovalState === "FLUID_REMOVAL_STATE_WAIT_REMOVE_CURRENT_SUPPLY")
        {
            type = "INFO";
            contentText = "T_FluidRemoval_RemoveCurrentSupply";
            showOkBtn = true;
            showCancelBtn = true;
            cancelBtnText = "T_Abort";
        }
        else if (workflowFluidRemovalState === "FLUID_REMOVAL_STATE_WAIT_SUDS_TO_WASTE_CONTAINER")
        {
            type = "INFO";
            contentText = "T_FluidRemoval_HoldPatientLineOverWasteBin";
            showOkBtn = true;
            showCancelBtn = true;
            cancelBtnText = "T_Abort";
        }
        else if (workflowFluidRemovalState === "FLUID_REMOVAL_STATE_SUDS_INSERT_WAITING")
        {
            type = "INFO";
            contentText = "T_FluidRemoval_SudsInsertWaiting";
            showOkBtn = false;
            showCancelBtn = true;
            cancelBtnText = "T_Abort";
        }
        else if (workflowFluidRemovalState === "FLUID_REMOVAL_STATE_FAILED_SUDS_REMOVED")
        {
            type = "WARNING";
            contentText = "T_FluidRemoval_SudsRemoved";
            showOkBtn = true;
            showCancelBtn = false;
            okBtnText = "T_OK";
        }
        else if (workflowFluidRemovalState === "FLUID_REMOVAL_STATE_FAILED_WASTE_CONTAINER_NOT_READY")
        {
            type = "WARNING";
            contentText = "T_FluidRemoval_WasteContainerNotReady";
            showOkBtn = true;
            showCancelBtn = false;
            okBtnText = "T_OK";
        }
        else if (workflowFluidRemovalState === "FLUID_REMOVAL_STATE_FAILED_USER_ABORT")
        {
            type = "WARNING";
            contentText = "T_FluidRemoval_UserAborted";
            showOkBtn = true;
            showCancelBtn = false;
            okBtnText = "T_OK";
        }
        else if (workflowFluidRemovalState === "FLUID_REMOVAL_STATE_FAILED_HW_FAILED")
        {
            type = "WARNING";
            contentText = "T_FluidRemoval_Failed;" + "Hardware Failed";
            showOkBtn = true;
            showCancelBtn = false;
            okBtnText = "T_OK";
        }
        else if (workflowFluidRemovalState === "FLUID_REMOVAL_STATE_REMOVING_TRAPPED_FLUID_ABORTED_BY_USER_STOP")
        {
            type = "WARNING";
            contentText = "T_FluidRemoval_Interrupted";
            showOkBtn = true;
            showCancelBtn = false;
            okBtnText = "T_OK";
        }
        else if (workflowFluidRemovalState === "FLUID_REMOVAL_STATE_REMOVING_TRAPPED_FLUID_ABORTED_BY_SUPPLY_STILL_LOADED")
        {
            type = "WARNING";
            contentText = "T_FluidRemoval_SupplyStillLoaded";
            showOkBtn = true;
            showCancelBtn = false;
            okBtnText = "T_OK";
        }
        else if (workflowFluidRemovalState === "FLUID_REMOVAL_STATE_REMOVING_TRAPPED_FLUID_ABORTED_BY_HW_FAILED")
        {
            contentText = "T_FluidRemoval_HwFailed;" + "Hardware Failed";
            showOkBtn = true;
            showCancelBtn = true;
            okBtnText = "T_Retry";
            cancelBtnText = "T_Abort";
        }
        else if ( (workflowFluidRemovalState === "FLUID_REMOVAL_STATE_PURGE_STARTED") ||
                  (workflowFluidRemovalState === "FLUID_REMOVAL_STATE_PURGE_PROGRESS") ||
                  (workflowFluidRemovalState === "FLUID_REMOVAL_STATE_FILL_STARTED") ||
                  (workflowFluidRemovalState === "FLUID_REMOVAL_STATE_FILL_PROGRESS") ||
                  (workflowFluidRemovalState === "FLUID_REMOVAL_STATE_FINAL_PURGE_STARTED") ||
                  (workflowFluidRemovalState === "FLUID_REMOVAL_STATE_FINAL_PURGE_PROGRESS") )
        {
            contentText = "T_FluidRemoval_Purging";
            showOkBtn = false;
            showCancelBtn = false;
        }
        else if (workflowFluidRemovalState === "FLUID_REMOVAL_STATE_DONE")
        {
            type = "INFO";
            contentText = "T_FluidRemoval_Complete";
            okBtnText = "T_OK";
            showCancelBtn = false;
            showOkBtn = true;
        }
        else if ( (workflowFluidRemovalState === "FLUID_REMOVAL_STATE_READY") ||
                  (workflowFluidRemovalState === "FLUID_REMOVAL_STATE_STARTED") ||
                  (workflowFluidRemovalState === "FLUID_REMOVAL_STATE_INACTIVE") )
        {
            popupShouldVisible = false;
        }
        else
        {
            logWarning("PopupFluidRemoval: Unexpected workflowFluidRemovalState=" + workflowFluidRemovalState);
            popupShouldVisible = false;
        }

        if (popupShouldVisible)
        {
            if (!isOpen())
            {
                open();
            }
        }
    }

    function init(newSyringeIdx)
    {
        syringeIndex = newSyringeIdx;

        var status = dsWorkflow.slotFluidRemovalStart(syringeIndex);
        if ( (status.State !== "Waiting") &&
             (status.State !== "Started") )
        {
            type = "WARNING";
            contentText = "T_FluidRemoval_Failed;" + status.State + ": " + status.Err;
            showOkBtn = true;
            showCancelBtn = false;

            if (!isOpen())
            {
                open();
            }
        }
    }
}

