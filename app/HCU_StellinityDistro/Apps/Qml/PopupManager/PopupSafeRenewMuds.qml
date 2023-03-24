import QtQuick 2.12
import QtQuick.Controls 2.12
import "../Widgets"
import "../Widgets/Popup"

PopupMessage {

    property var syringeStates: dsMcu.syringeStates
    property bool mudsInserted: dsMcu.mudsInserted
    property var bottleBubbleStates: dsMcu.bottleBubbleStates
    property var stopcockPositions: dsMcu.stopcockPositions
    property var syringeVols: dsMcu.syringeVols
    property var plungerStates: dsMcu.plungerStates
    property bool stopBtnPressed: dsMcu.stopBtnPressed

    /*
      The following states are used by this process:
      START
      ENGAGE
      USER_CHECK
      PURGE_FLUID_SC
      PURGE_FLUID
      CLEAR_INLETS_SC
      CLEAR_INLETS
      CHECK_SUPPLIES_CLEAR
      CHECK_SUPPLIES_NOT_CLEAR
      POST_CLEAR_INLETS
      PURGE_INLET_FLUID_SC
      PURGE_INLET_FLUID
      RESET_MUDS_SC
      RESET_MUDS
      DISENGAGE
     */

    titleText: "Day Set Renewal"
    showOtherBtn: false
    translationRequired: false
    // The remaining properties are set in resetPage()

    Timer {
        // Used to give time between various MCU commands
        id: delayTimer
        interval: 1000
        repeat: false
        onTriggered: {
            delayTimer.stop();

            switch (otherBtnText)
            {
                case "PURGE_FLUID":
                    engagePlungers();
                    break;
                case "CLEAR_INLETS_SC":
                    clearInlets();
                    break;
                case "CLEAR_INLETS":
                    postClearInlets();
                    break;
                case "POST_CLEAR_INLETS":
                    checkSuppliesRemoved();
                    break;
                case "PURGE_INLET_FLUID":
                    resetMudsSetStopcocks();
                    break;
                case "RESET_MUDS_SC":
                    resetMuds();
                    break;
            }
        }
    }

    onOpened: {
        resetPage();
    }

    onBtnCancelClicked: {
        closePopup();
    }

    onBtnOkClicked: {
        if (otherBtnText === "START")
        {
            sudsAndSupplyCheck();
        }
        else if ( (otherBtnText === "USER_CHECK") ||
                  (otherBtnText === "CHECK_SUPPLIES_NOT_CLEAR") )
        {
            purgeFluidSetStopcocks();
        }
    }

    onMudsInsertedChanged: {
        if (!visible)
        {
            return;
        }

        if (otherBtnText !== "START")
        {
            return;
        }

        enableOkBtn = mudsInserted;
    }


    onBottleBubbleStatesChanged: {
        processBottleBubbleStates();
    }

    onStopcockPositionsChanged: {
        processStopcockPositions();
    }

    onSyringeVolsChanged: {
        processSyringeVols();
    }

    onPlungerStatesChanged: {
        processPlungerState();
    }

    onStopBtnPressedChanged: {
        if (!visible)
        {
            return;
        }

        if (stopBtnPressed)
        {
            closePopup();
        }
    }

    onSyringeStatesChanged: {
        if (!visible)
        {
            return;
        }

        if ( !( (otherBtnText === "RESET_MUDS") ||
                (otherBtnText === "DISENGAGE") ) )
        {
            return;
        }

        var isCompleted = true;
        for (var i = 0; i < syringeStates.length; i++)
        {
            if (syringeStates[i] !== "COMPLETED")
            {
                isCompleted = false;
                break;
            }
        }
        if (isCompleted)
        {
            if (otherBtnText === "RESET_MUDS")
            {
                disengageMuds();
            }
            else
            {
                // state must be DISENGAGE
                closePopup();
            }
        }
    }

    // Used to safely stop any operations in progress and close the popup
    function closePopup()
    {
        dsMcu.slotPistonStop(0);
        dsMcu.slotPistonStop(1);
        dsMcu.slotPistonStop(2);
        close();
    }

    // Step 1 - verify no supplies are attached and SUDS is ready
    function sudsAndSupplyCheck()
    {
        otherBtnText = "USER_CHECK";

        okBtnText = "OK"
        showOkBtn = true;
        contentText = "Remove all supply bottles or bags.\n\nEnsure Patient Line is installed and hold over waste bin.";
    }

    // Step 2 - engage plungers
    function engagePlungers()
    {
        otherBtnText = "ENGAGE";

        showOkBtn = false;
        contentText = "Engaging plungers...";

        dsMcu.slotPistonEngage(0);
        dsMcu.slotPistonEngage(1);
        dsMcu.slotPistonEngage(2);

        processPlungerState();
    }    

    // Step 3 - set SCs ready for purging
    function purgeFluidSetStopcocks()
    {
        otherBtnText = "PURGE_FLUID_SC";

        showOkBtn = false;
        contentText = "Purging Day Set fluid...";

        dsDevice.slotStopcockInjectPosition("RS0");
        dsDevice.slotStopcockInjectPosition("RC1");
        dsDevice.slotStopcockInjectPosition("RC2");

        processStopcockPositions();
    }

    // Step 5 - purge fluid from MUDS
    function purgeFluid()
    {
        otherBtnText = "PURGE_FLUID";

        dsMcu.slotPistonUp(0,3);
        dsMcu.slotPistonUp(1,3);
        dsMcu.slotPistonUp(2,3);

        processSyringeVols();
    }

    // Step 6 - set SCs for inlet pulling
    function clearInletsSetStopcocks()
    {
        otherBtnText = "CLEAR_INLETS_SC";

        showOkBtn = false;
        contentText = "Clearing inlet fluid...";

        dsDevice.slotStopcockFillPosition("RS0");
        dsDevice.slotStopcockFillPosition("RC1");
        dsDevice.slotStopcockFillPosition("RC2");

        processStopcockPositions();
    }

    // Step 7 - pull fluid from inlets
    function clearInlets()
    {
        otherBtnText = "CLEAR_INLETS";

        dsMcu.slotPistonDown(0,5);
        dsMcu.slotPistonDown(1,5);
        dsMcu.slotPistonDown(2,5);

        // Pull down for 5 seconds
        delayTimer.interval = 5000;
        delayTimer.start();
    }

    // Step 8 - stop pulling fluid from inlets
    function postClearInlets()
    {
        otherBtnText = "POST_CLEAR_INLETS";

        dsMcu.slotPistonStop(0);
        dsMcu.slotPistonStop(1);
        dsMcu.slotPistonStop(2);

        delayTimer.interval = 1000;
        delayTimer.start();
    }

    // Step 9 - check bubble states to ensure fluid supplies are not present
    function checkSuppliesRemoved()
    {
        otherBtnText = "CHECK_SUPPLIES_CLEAR";
        processBottleBubbleStates();
    }

    // Step 10 - if supplies are still present, give proper instruction
    function checkSuppliesNotRemoved()
    {
        otherBtnText = "CHECK_SUPPLIES_NOT_CLEAR";

        showOkBtn = true;
        contentText = "Supplies detected.\n\nRemove bottles and click 'OK'.";
    }

    // Step 11 - set the SCs for purging the fluid from the inlets
    function purgeInletFluidSetStopcocks()
    {
        otherBtnText = "PURGE_INLET_FLUID_SC";

        contentText = "Purging inlet fluid...";

        dsDevice.slotStopcockInjectPosition("RS0");
        dsDevice.slotStopcockInjectPosition("RC1");
        dsDevice.slotStopcockInjectPosition("RC2");

        processStopcockPositions();
    }

    // Step 12 - purge the fluid pulled into the reservoir from inlets
    function purgeInletFluid()
    {
        otherBtnText = "PURGE_INLET_FLUID";

        dsMcu.slotPistonUp(0,5);
        dsMcu.slotPistonUp(1,5);
        dsMcu.slotPistonUp(2,5);

        processSyringeVols();
    }

    // Step 13 - set the SCs for resetting the MUDS
    function resetMudsSetStopcocks()
    {
        otherBtnText = "RESET_MUDS_SC";

        contentText = "Resetting plungers...";

        dsDevice.slotStopcockFillPosition("RS0");
        dsDevice.slotStopcockFillPosition("RC1");
        dsDevice.slotStopcockFillPosition("RC2");

        processStopcockPositions();
    }

    // Step 14 - reset the MUDS
    function resetMuds()
    {
        dsMcu.slotResetMuds();

        otherBtnText = "RESET_MUDS";
    }

    // Step 15 - disengage the MUDS
    function disengageMuds()
    {
        showOkBtn = false;
        contentText = "Disengaging plungers...";

        dsMcu.slotPistonDisengage(0);
        dsMcu.slotPistonDisengage(1);
        dsMcu.slotPistonDisengage(2);

        otherBtnText = "DISENGAGE";
    }

    // Called when onStopcockPositionsChanged occurs
    function processStopcockPositions()
    {
        if (!visible)
        {
            return;
        }

        if ( !( (otherBtnText === "PURGE_FLUID_SC") ||
                (otherBtnText === "PURGE_INLET_FLUID_SC") ||
                (otherBtnText === "CLEAR_INLETS_SC") ||
                (otherBtnText === "RESET_MUDS_SC") ) )
        {
            return;
        }

        if (stopcockPositions !== undefined)
        {
            if ( (stopcockPositions[0] === "INJECT") &&
                 (stopcockPositions[1] === "INJECT") &&
                 (stopcockPositions[2] === "INJECT") )
            {
                if (otherBtnText === "PURGE_FLUID_SC")
                {
                    purgeFluid();
                }
                else if (otherBtnText === "PURGE_INLET_FLUID_SC")
                {
                    purgeInletFluid();
                }
            }
            else if ( (stopcockPositions[0] === "FILL") &&
                      (stopcockPositions[1] === "FILL") &&
                      (stopcockPositions[2] === "FILL") )
            {
                if ( (otherBtnText === "CLEAR_INLETS_SC") ||
                     (otherBtnText === "RESET_MUDS_SC") )
                {
                    delayTimer.interval = 1000;
                    delayTimer.start();
                }
            }
        }
    }

    // Called when onBottleBubbleStatesChanged occurs
    function processBottleBubbleStates()
    {
        if (!visible)
        {
            return;
        }

        if (otherBtnText !== "CHECK_SUPPLIES_CLEAR")
        {
            return;
        }

        if (bottleBubbleStates !== undefined)
        {
            if ( (bottleBubbleStates[0] !== "FLUID") &&
                 (bottleBubbleStates[1] !== "FLUID") &&
                 (bottleBubbleStates[2] !== "FLUID") )
            {
                purgeInletFluidSetStopcocks();
            }
            else
            {
                checkSuppliesNotRemoved();
            }
        }
    }

    // Called when onSyringeVolsChanged occurs
    function processSyringeVols()
    {
        if (!visible)
        {
            return;
        }

        if ( !( (otherBtnText === "PURGE_FLUID") ||
                (otherBtnText === "PURGE_INLET_FLUID") ) )
        {
            return;
        }

        if ( (syringeVols[0] === 0) &&
                (syringeVols[1] === 0) &&
                (syringeVols[2] === 0) )
        {
            delayTimer.interval = 1000;
            delayTimer.start();
        }
    }

    // Called when onPlungerStatesChanged occurs
    function processPlungerState()
    {
        if (!visible)
        {
            return;
        }

        if ( !( (otherBtnText === "ENGAGE") ||
                (otherBtnText === "DISENGAGE") ) )
        {
            return;
        }

        if ( (plungerStates[0] === "ENGAGED") &&
             (plungerStates[1] === "ENGAGED") &&
             (plungerStates[2] === "ENGAGED") )
        {
            if (otherBtnText === "ENGAGE")
            {
                clearInletsSetStopcocks();
            }
        }
    }

    function resetPage()
    {
        showOtherBtn = false;
        otherBtnText = "START";

        contentText = "Ensure a Day Set is inserted and latched. It will be renewed.\n\nClick 'OK' to begin.";

        okBtnText = "OK";
        enableOkBtn = mudsInserted;
        showOkBtn = true;

        cancelBtnText = "Cancel";
        showCancelBtn = true;
    }
}
