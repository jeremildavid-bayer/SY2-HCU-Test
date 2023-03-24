import QtQuick 2.12
import ".."
import "../../../Util.js" as Util

DeviceManagerPanel {
    property int syringeIndex
    property var fluidSourceBottle
    property var fluidSourceSyringe
    property double syringeVolume: 0
    property var fluidSelectItems: []
    property bool isBusy: (fluidSourceBottle !== undefined) && (fluidSourceBottle.IsBusy !== undefined) && (fluidSourceBottle.IsBusy)
    property var executedSteps: dsExam.executedSteps
    property var fluidOptions: dsCfgLocal.fluidOptions

    id: panelBottle

    content: [
        DeviceManagerPanelBottleMain {
            width: actionButtonWidth
            anchors.top: parent.top
            anchors.bottom: parent.bottom
        }
    ]

    textDisposableState2: getTextDisposableState2()

    onFluidSourceBottleChanged: {
        if (visible)
        {
            reload();
        }
    }

    onVisibleChanged: {
        if (visible)
        {
            reload();
        }
    }

    function reload()
    {
        if ( (fluidSourceBottle === undefined) ||
             (fluidSourceBottle.InstalledAt === undefined) ||
             (fluidSourceBottle.SourcePackages.length === 0) )
        {
            textDisposableState = "T_NotLoaded";
            stopElapsedTimer();
        }
        else if (fluidSourceBottle.NeedsReplaced)
        {
            textDisposableState = "T_EmptySupplyDetected";
            stopElapsedTimer();
            disposableNeedsReplaced = true;
        }
        else
        {
            var curDisposableInsertedEpochMs = Util.utcDateTimeToMillisec(fluidSourceBottle.SourcePackages[0].LoadedAt);
            var curMaxUseDurationMs = -1;

            if (fluidSourceBottle.SourcePackages[0].MaximumUseDuration !== undefined)
            {
                curMaxUseDurationMs = Util.durationStrToMillisec(fluidSourceBottle.SourcePackages[0].MaximumUseDuration);
            }
            initElapsedTimer(curDisposableInsertedEpochMs, curMaxUseDurationMs);
            startElapsedTimer();
        }
    }

    function getTextDisposableState2()
    {
        if (fluidSourceBottle === undefined)
        {
            return "";
        }
        else if ( (fluidSourceMuds === undefined) ||
                  (fluidSourceMuds.InstalledAt === undefined) )
        {
            return "";
        }
        else if (!fluidSourceBottle.IsReady)
        {
            return "T_SpikeNotDetected";
        }
        else if (fluidSourceBottle.IsBusy)
        {
            return "T_Filling...";
        }
        else if ( (syringeIndex == 1) ||
                  (syringeIndex == 2) )
        {
            // Check for contrast fluid package option
            if (fluidOptions.ContrastFamilies.length === 0)
            {
                return "T_ReplaceSupply_NoFluidPackageOption";
            }
            else if (fluidSelectItems.length === 0)
            {
                return "T_ReplaceSupply_FluidTypeNoLongerExists";
            }
            else if ( (fluidSourceMuds.InstalledAt !== undefined) &&
                      (executedSteps.length > 0) &&
                      (fluidSourceSyringe.SourcePackages !== undefined) &&
                      (fluidSourceSyringe.SourcePackages.length > 0) )
            {
                return "T_ReplaceSupply_PreventTypeChangeDuringExam";
            }
        }

        return "";
    }
}
