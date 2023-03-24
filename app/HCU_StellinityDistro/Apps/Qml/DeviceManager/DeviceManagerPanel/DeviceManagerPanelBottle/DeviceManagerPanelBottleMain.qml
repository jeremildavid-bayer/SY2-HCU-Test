import QtQuick 2.12
import "../../../Widgets"
import "../../../Util.js" as Util
import "../../../CommonElements"

Item {
    property var activeAlerts: dsAlert.activeAlerts
    property int bottleSelectedIndex: -1
    property int volumeSelectedIndex: 0
    property int newBottleSelectedIndex: 0
    property var newBottleSelected
    property bool mudsInserted: dsMcu.mudsInserted
    property int rowSpacing: parent.height * 0.04
    property int fluidBottleLotBatchTextLenMin: dsCapabilities.fluidBottleLotBatchTextLenMin
    property int fluidBottleLotBatchTextLenMax: dsCapabilities.fluidBottleLotBatchTextLenMax
    property double volumeMax: dsCapabilities.volumeMax
    property var bottleVolumeSelectList: ((fluidSelectItems === undefined) || (fluidSelectItems[bottleSelectedIndex] === undefined)) ? [] : fluidSelectItems[bottleSelectedIndex].Volumes
    property bool plungerEngaged: (dsMcu.plungerStates[syringeIndex] === "ENGAGED")
    property var fluidSourceSyringe: panelBottle.fluidSourceSyringe
    property var fluidSourceBottle: panelBottle.fluidSourceBottle
    property var prevFluidSourceBottle
    property var fluidSelectItems: panelBottle.fluidSelectItems
    property var fluidOptions: dsCfgLocal.fluidOptions
    property string workflowMudsSodState: dsWorkflow.workflowMudsSodState
    property bool fillNewEnabled: (fillNewDisabledReason == "")
    property bool selectOtherSourcePackageEnabled: (selectOtherSourcePackageDisabledReason == "")
    property bool editSourcePackageEnabled: (editSourcePackageDisabledReason == "")
    property string fillDisabledReason: getFillDisabledReason()
    property string fillNewDisabledReason: getFillNewDisabledReason()
    property string editSourcePackageDisabledReason: getEditSourcePackageDisabledReason()
    property string selectOtherSourcePackageDisabledReason: getSelectOtherSourcePackageDisabledReason()
    property var plan: dsExam.plan

    id: panelMain

    DeviceManagerPanelBottleBarcodeScan {}

    DeviceManagerPanelBottleGroupList {
        id: groupListView
        height: actionButtonHeight *  1.1
        width: parent.width
    }

    DeviceManagerPanelBottleVolumeList {
        id: volumeListView
        anchors.top: groupListView.bottom
        anchors.topMargin: rowSpacing
        height: groupListView.height
        width: groupListView.width
    }

    DeviceManagerPanelBottleParamList {
        id: paramList
        anchors.top: volumeListView.bottom
        anchors.topMargin: rowSpacing * 1.2
        width: actionButtonWidth
    }

    AutoEmptyModeEnabledMsg {
        id: autoEmptyEnabledIndicator
        visible: {
            if (parent.visible) {
                if ((syringeIndex === 0) && fillNewEnabled) return dsCfgGlobal.autoEmptySalineEnabled;
                if ((syringeIndex === 1) && fillNewEnabled) return dsCfgGlobal.autoEmptyContrast1Enabled;
                if ((syringeIndex === 2) && fillNewEnabled) return dsCfgGlobal.autoEmptyContrast2Enabled;
                return false;
            }
            return parent.visible;
        }
        anchors.bottom: frameBtnActions.top
        anchors.top: paramList.bottom
        anchors.bottomMargin: rowSpacing * 0.7
        width: parent.width
    }

    Item {
        id: frameBtnActions
        anchors.bottom: parent.bottom
        width: parent.width
        height: actionButtonHeight * 1.2

        GenericButton {
            id: btnFillNew
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            height: actionButtonHeight * 1.2
            color: colorMap.actionButtonBackground
            enabled: fillNewEnabled
            content: [
                Item {
                    anchors.horizontalCenter: parent.horizontalCenter
                    height: parent.height
                    width: iconBottle2.x + iconBottle2.width

                    Text {
                        id: iconBottle
                        width: contentWidth + (btnFillNew.width * 0.02)
                        height: parent.height
                        font.family: fontIcon.name
                        text: titleIconText
                        font.pixelSize: height * 0.5
                        color: getFluidSelectItemColor(fluidSelectItems[bottleSelectedIndex])
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    Text {
                        width: iconBottle.width
                        height: iconBottle.height
                        text: "+"
                        font.pixelSize: height * 0.4
                        font.family: fontRobotoBold.name
                        color: colorMap.actionButtonText
                        horizontalAlignment: Text.AlignRight
                        verticalAlignment: Text.AlignVCenter
                    }
                    Text {
                        id: iconBottle2
                        anchors.left: iconBottle.right
                        anchors.leftMargin: btnFillNew.width * 0.03
                        width: contentWidth + (btnFillNew.width * 0.02)
                        height: parent.height
                        font.pixelSize: parent.height * 0.3
                        font.family: fontRobotoLight.name
                        text: translate("T_Fill")
                        verticalAlignment: Text.AlignVCenter
                        color: colorMap.actionButtonText
                    }
                }
            ]

            onBtnClicked: {
                logDebug("DeviceManagerPanelBottleMain[" + syringeIndex + "]: btnFillNew clicked");
                if ( (fluidSelectItems.length == 0) ||
                     (isFillCurrentSupplyOk(fluidSelectItems[bottleSelectedIndex])) )
                {
                    loadBottle();
                    dsWorkflow.slotFill(syringeIndex);
                }
                else if ( (syringeIndex == 1) ||
                          (syringeIndex == 2) )
                {
                    startChangeContrastBottle(bottleSelectedIndex);
                }
                else
                {
                    logError("DeviceManagerPanelBottleMain[" + syringeIndex + "]: Failed to start new fill");
                }
            }
        }
    }


    Timer {
        // To prevent screen flickering (due to fluid source state change), use sw debouncer
        id: tmrDebounceScreenState
        interval: 400 // Debounce time
        repeat: false
    }

    onVisibleChanged: {
        if (visible)
        {
            groupListView.selectItem(bottleSelectedIndex);
            volumeListView.selectItem(volumeSelectedIndex);
        }
    }


    onBottleSelectedIndexChanged: {
        updateVolumeSelectedIndex();
        groupListView.selectItem(bottleSelectedIndex);
        paramList.updateLotBatchAndExpiration();
    }

    onVolumeSelectedIndexChanged: {
        volumeListView.selectItem(volumeSelectedIndex);
        paramList.updateLotBatchAndExpiration();
    }

    onFluidSourceSyringeChanged: {
        if (visible)
        {
            tmrDebounceScreenState.restart();
        }
    }

    onFluidSourceBottleChanged: {
        if (!popupManager.popupFluidRemoval.isOpen())
        {
            reload();

            if (prevFluidSourceBottle === undefined)
            {
                // Previous Fluid Source is never set.
                paramList.updateLotBatchAndExpiration();
            }
            else if ( (prevFluidSourceBottle.IsReady) &&
                      (!fluidSourceBottle.IsReady) )
            {
                // Ready state changed to Not Ready.
                // This will reset Lot/Batch and Expiration. Only reset when spike is un-installed (IsReady == false)
                paramList.updateLotBatchAndExpiration();
            }
            else if (!Util.compareObjects(prevFluidSourceBottle.SourcePackages, fluidSourceBottle.SourcePackages))
            {
                paramList.updateLotBatchAndExpiration();
            }
        }

        if (visible)
        {
            tmrDebounceScreenState.restart();
        }

        prevFluidSourceBottle = Util.copyObject(fluidSourceBottle);
    }

    onFluidSelectItemsChanged: {
        if ( (fluidSourceSyringe !== undefined) &&
             (fluidSourceBottle !== undefined) &&
             (fluidSourceSyringe.SourcePackages !== undefined) &&
             (fluidSourceBottle.SourcePackages !== undefined) &&
             (fluidSourceSyringe.SourcePackages.length > 0) &&
             (fluidSourceBottle.SourcePackages.length > 0) &&
             (fluidSourceSyringe.SourcePackages[0].Brand === fluidSourceBottle.SourcePackages[0].Brand) &&
             (fluidSourceSyringe.SourcePackages[0].Concentration === fluidSourceBottle.SourcePackages[0].Concentration) &&
             (fluidSourceSyringe.SourcePackages[0].ExpirationDate === fluidSourceBottle.SourcePackages[0].ExpirationDate) &&
             (fluidSourceSyringe.SourcePackages[0].LotBatch === fluidSourceBottle.SourcePackages[0].LotBatch) &&
             (fluidSourceSyringe.SourcePackages[0].Volume === fluidSourceBottle.SourcePackages[0].Volume) )
        {
            // Syringe is filled from current bottle, volume index needs to evaluated - The fluid options might be changed from admin/config
            if ( (fluidSelectItems[bottleSelectedIndex] !== undefined) &&
                 (fluidSelectItems[bottleSelectedIndex].Volumes.length === 1) &&
                 (fluidSelectItems[bottleSelectedIndex].Volumes[0] === 0) )
            {
                volumeSelectedIndex = 0;
            }
            else
            {
                volumeSelectedIndex = -1;
            }
        }

        updateBottleSelectedIndex();
    }

    onMudsInsertedChanged: {
        bottleSelectedIndex = -1;
        volumeSelectedIndex = 0;
        groupListView.selectItem(bottleSelectedIndex);
        volumeListView.selectItem(volumeSelectedIndex);
    }



    onFillNewDisabledReasonChanged: {
        if (fillNewDisabledReason == "")
        {
            logInfo("DeviceManagerPanelBottleMain[" + syringeIndex + "]: FillNew Enabled");
        }
        else
        {
            logInfo("DeviceManagerPanelBottleMain[" + syringeIndex + "]: FillNew Disabled: " + fillNewDisabledReason);
        }
    }

    onEditSourcePackageDisabledReasonChanged: {
        if (editSourcePackageDisabledReason == "")
        {
            logInfo("DeviceManagerPanelBottleMain[" + syringeIndex + "]: EditSourcePackage Enabled");
        }
        else
        {
            logInfo("DeviceManagerPanelBottleMain[" + syringeIndex + "]: EditSourcePackage Disabled: " + editSourcePackageDisabledReason);
        }
    }

    onSelectOtherSourcePackageDisabledReasonChanged: {
        if (selectOtherSourcePackageDisabledReason == "")
        {
            logInfo("DeviceManagerPanelBottleMain[" + syringeIndex + "]: SelectOtherSourcePackage Enabled");
        }
        else
        {
            logInfo("DeviceManagerPanelBottleMain[" + syringeIndex + "]: SelectOtherSourcePackage Disabled: " + selectOtherSourcePackageDisabledReason);
        }
    }

    function reload()
    {
        updateBottleSelectedIndex();
    }

    function startChangeContrastBottle(newBottleSelectedIdx)
    {
        newBottleSelectedIndex = newBottleSelectedIdx;
        newBottleSelected = Util.copyObject(fluidSelectItems[newBottleSelectedIdx]);
        popupManager.popupFluidRemoval.closed.connect(slotPopupFluidRemovalClosed);
        popupManager.popupFluidRemoval.init(syringeIndex);
    }

    function updateBottleSelectedIndex()
    {
        var newBottleSelectedIndex = bottleSelectedIndex;

        if (syringeIndex == 0)
        {
            // For BS0, always select the only item.
            newBottleSelectedIndex = 0;
        }
        else if ( (fluidSourceBottle !== undefined) &&
                  (fluidSourceBottle.SourcePackages !== undefined) &&
                  (fluidSourceBottle.SourcePackages.length > 0) )
        {
            // Update bottleSelectedIndex
            var sourcePackageLoaded = fluidSourceBottle.SourcePackages[0];
            for (var itemIdx = 0; itemIdx < fluidSelectItems.length; itemIdx++)
            {
                var fluidSelectItem = fluidSelectItems[itemIdx];
                if ( (fluidSelectItem.Brand === sourcePackageLoaded.Brand) &&
                     (fluidSelectItem.Concentration === sourcePackageLoaded.Concentration) )
                {
                    newBottleSelectedIndex = itemIdx;
                    break;
                }
            }
        }

        if (bottleSelectedIndex != newBottleSelectedIndex)
        {
            if (newBottleSelectedIndex == -1)
            {
                logInfo("DeviceManagerPanelBottleMain[" + syringeIndex + "]: No selected package found. Bottle Index=-1");
            }
            bottleSelectedIndex = newBottleSelectedIndex;
            groupListView.selectItem(bottleSelectedIndex);
        }
        else
        {
            updateVolumeSelectedIndex();
        }
    }

    function updateVolumeSelectedIndex()
    {
        if (bottleSelectedIndex == -1)
        {
            return;
        }

        // Update volumeSelectedIndex
        var fluidSelectItem = fluidSelectItems[bottleSelectedIndex];
        if (fluidSelectItem === undefined)
        {
            return;
        }

        if (volumeSelectedIndex >= fluidSelectItem.Volumes.length)
        {
            // volume select out of range, set to first volume for now.
            volumeSelectedIndex = 0;
        }

        if ( (fluidSourceBottle === undefined) ||
             (fluidSourceBottle.SourcePackages === undefined) )
        {
            return;
        }

        var sourcePackageLoaded = fluidSourceBottle.SourcePackages[0];
        if (sourcePackageLoaded === undefined)
        {
            return;
        }

        // Find and set to currently loaded volume
        for (var volumeIdx = 0; volumeIdx < fluidSelectItem.Volumes.length; volumeIdx++)
        {
            if (fluidSelectItem.Volumes[volumeIdx] === sourcePackageLoaded.Volume)
            {
                volumeSelectedIndex = volumeIdx;
                break;
            }
        }
    }

    function checkBottleChanged()
    {
        var newSourcePackage = fluidSelectItems[bottleSelectedIndex];
        var curSourcePackage = fluidSourceBottle.SourcePackages[0];

        if ( (curSourcePackage === undefined) &&
             (newSourcePackage !== undefined) )
        {
            return false;
        }

        var newLotBatch = (paramList.lotBatchText == "--") ? "" : paramList.lotBatchText;
        var newExpirationDate = (paramList.expirationText == "--") ? undefined : paramList.expirationText;
        var curExpirationDate = (curSourcePackage.ExpirationDate === undefined) ? undefined :  Util.utcDateTimeToExpiryFormat(curSourcePackage.ExpirationDate);

        if ( (newSourcePackage.Brand === curSourcePackage.Brand) &&
             (newSourcePackage.Concentration === curSourcePackage.Concentration) &&
             (newSourcePackage.ConcentrationUnits === curSourcePackage.ConcentrationUnits) &&
             (newSourcePackage.Volumes[volumeSelectedIndex] === curSourcePackage.Volume) &&
             (newLotBatch === curSourcePackage.LotBatch) &&
             (newExpirationDate === curExpirationDate) )
        {
            return false;
        }
        return true;
    }

    function loadBottle()
    {
        if (fluidSelectItems[bottleSelectedIndex] !== undefined)
        {
            dsWorkflow.slotLoadNewSource(syringeIndex,
                                         fluidSelectItems[bottleSelectedIndex].Brand,
                                         parseFloat(fluidSelectItems[bottleSelectedIndex].Concentration),
                                         parseFloat(fluidSelectItems[bottleSelectedIndex].Volumes[volumeSelectedIndex]),
                                         paramList.lotBatchText,
                                         paramList.expirationText);
        }
        else if (fluidSourceBottle.SourcePackages.length > 0)
        {
            logInfo("DeviceManagerPanelBottleMain[" + syringeIndex + "]: Bottle information is not available. Loading with previous information");
            var prevPackage = fluidSourceBottle.SourcePackages[0];
            dsWorkflow.slotLoadNewSource(syringeIndex,
                                         prevPackage.Brand,
                                         parseFloat(prevPackage.Concentration),
                                         parseFloat(prevPackage.Volume),
                                         paramList.lotBatchText,
                                         paramList.expirationText);

        }
        else
        {
            logError("Bottle[" + syringeIndex + "]: Bottle information is not available from anywhere. Loading failed");
        }
    }

    function slotPopupFluidRemovalClosed()
    {
        if (popupManager.popupFluidRemoval.contentText === "T_FluidRemoval_Complete")
        {
            logDebug("DeviceManagerPanelBottleMain[" + syringeIndex + "]: bottle selected from " + bottleSelectedIndex + " -> " + newBottleSelectedIndex + " (" + JSON.stringify(newBottleSelected) + ")");
            bottleSelectedIndex = newBottleSelectedIndex;
            groupListView.selectItem(bottleSelectedIndex);

            if ( (newBottleSelected.LotBatch !== undefined) &&
                 (newBottleSelected.LotBatch !== "") &&
                 (newBottleSelected.LotBatch.length >= fluidBottleLotBatchTextLenMin) )
            {
                paramList.lotBatchText = newBottleSelected.LotBatch;
            }

            if ( (newBottleSelected.ExpirationDate !== undefined) &&
                 (newBottleSelected.ExpirationDate !== "") )
            {
                paramList.expirationText = Util.utcDateTimeToExpiryFormat(newBottleSelected.ExpirationDate);
            }
        }
        else
        {
            reload();
            paramList.updateLotBatchAndExpiration();
        }

        popupManager.popupFluidRemoval.closed.disconnect(slotPopupFluidRemovalClosed);
    }

    function isFillCurrentSupplyOk(newFluidPackage)
    {
        if ( (fluidSourceBottle === undefined) ||
             (fluidSourceBottle.SourcePackages === undefined) ||
             (fluidSourceBottle.SourcePackages.length === 0) )
        {
            // Supply never loaded.
            return true;
        }

        if (newFluidPackage !== undefined)
        {
            // Do not check compatibility for saline as the translation of Brand changes between locales
            if (syringeIndex === 0)
            {
                return true;
            }

            if ( (fluidSourceBottle.SourcePackages[0].Brand === newFluidPackage.Brand) &&
                 (fluidSourceBottle.SourcePackages[0].Concentration === newFluidPackage.Concentration) )
            {
                // New fluid package is compatible to filled in reservoir
                return true;
            }
        }

        return false;
    }

    function getFillDisabledReason()
    {
        var disabledReason = "";

        if (!visible)
        {
            disabledReason = "Not Visible";
        }
        else if (popupManager.popupFluidRemoval.isOpen())
        {
            disabledReason = "Replace contrast in progress";
        }
        else if ( (fluidSourceMuds === undefined) ||
                  (fluidSourceMuds.InstalledAt === undefined) )
        {
            disabledReason = "MUDS is not inserted";
        }
        else if (fluidSourceMuds.IsBusy)
        {
            disabledReason = "MUDS is busy";
        }
        else if ( (fluidSourceBottle === undefined) ||
                  (fluidSourceBottle.InstalledAt === undefined) )
        {
            disabledReason = "Bottle is not loaded";
        }
        else if ( (fluidSourceSyringe === undefined) ||
                  (fluidSourceSyringe.InstalledAt === undefined) )
        {
            disabledReason = "Syringe is not engaged";
        }
        else if (fluidSourceSyringe.IsBusy)
        {
            disabledReason = "Syringe is busy";
        }
        else if (fluidSourceSyringe.NeedsReplaced)
        {
            disabledReason = "Syringe needs replaced";
        }
        else if (workflowMudsSodState == "MUDS_SOD_STATE_SYRINGE_SOD_PROGRESS")
        {
            disabledReason = "MUDS SOD State is " + workflowMudsSodState;
        }
        else if ( (executedSteps.length > 0) &&
                  (!isFillCurrentSupplyOk(fluidSelectItems[bottleSelectedIndex])) )
        {
            disabledReason = "Injection is progress and currectly selected bottle is incompatible";
        }
        else if (tmrDebounceScreenState.running)
        {
            disabledReason = "ScreenState Debouncer Running";
        }

        return disabledReason;
    }

    function getFillNewDisabledReason()
    {
        var disabledReason = "";

        if (fillDisabledReason !== "")
        {
            disabledReason = fillDisabledReason;
        }
        else if (!fluidSourceBottle.IsReady)
        {
            disabledReason = "Bottle is not ready";
        }
        else if ( (fluidSourceBottle.SourcePackages.length === 0) &&
                  (fluidSelectItems[bottleSelectedIndex] === undefined) )
        {
            disabledReason = "No fluid package selected";
        }
        else if ( (fluidSelectItems.length > 0) &&
                  (fluidSelectItems[bottleSelectedIndex] === undefined) )
        {
            disabledReason = "Invalid fluid package selected";
        }
        else if ( (fluidSelectItems.length > 0) &&
                  (fluidSelectItems[bottleSelectedIndex].Volumes.length > 0) &&
                  (fluidSelectItems[bottleSelectedIndex].Volumes[volumeSelectedIndex] === undefined) )
        {
            disabledReason = "Invalid fluid package volume selected";
        }
        else if (!paramList.mandatoryFieldsFilled)
        {
            disabledReason = "Mandatory Fields not filled";
        }
        else
        {
            for (var alertIdx = 0; alertIdx < activeAlerts.length; alertIdx++)
            {
                if (activeAlerts[alertIdx].CodeName === "MUDSUseLifeLimitEnforced")
                {
                    disabledReason = "Muds use life limit enforced";
                    break;
                }
            }
        }

        return disabledReason;
    }

    function getEditSourcePackageDisabledReason()
    {
        var disabledReason = "";

        if (fillDisabledReason !== "")
        {
            disabledReason = fillDisabledReason;
        }
        else if ( (fluidSourceBottle.SourcePackages.length === 0) &&
                  (fluidSelectItems[bottleSelectedIndex] === undefined) )
        {
            disabledReason = "No fluid package selected";
        }
        else if ( (fluidSelectItems.length > 0) &&
                  (fluidSelectItems[bottleSelectedIndex] === undefined) )
        {
            disabledReason = "Invalid fluid package selected";
        }
        return disabledReason;
    }

    function getSelectOtherSourcePackageDisabledReason()
    {
        var disabledReason = "";

        if (fillDisabledReason !== "")
        {
            disabledReason = fillDisabledReason;
        }
        else if ( ( (syringeIndex == 1) || (syringeIndex == 2) ) &&
                  (fluidSourceBottle !== undefined) &&
                  (fluidSourceBottle.SourcePackages !== undefined) &&
                  (fluidSourceBottle.SourcePackages.length > 0) &&
                  (executedSteps.length > 0) )
        {
            // Contrast type AND
            // Supply Loaded AND
            // Injection in progress
            disabledReason = "Injection in progress";
        }
        return disabledReason;
    }


    function getFluidSelectItemColor(selectItem)
    {
        if (syringeIndex == 1)
        {
            if ( (selectItem !== undefined) &&
                 (selectItem.Brand === fluidSourceBrandC2) &&
                 (selectItem.Concentration === fluidSourceConcentrationC2) )
            {
                return fluidC2Color;
            }
            else
            {
                return colorMap.contrast1;
            }
        }
        else if (syringeIndex == 2)
        {
            if ( (selectItem !== undefined) &&
                 (selectItem.Brand === fluidSourceBrandC1) &&
                 (selectItem.Concentration === fluidSourceConcentrationC1) )
            {
                return colorMap.contrast1;
            }
            else
            {
                return colorMap.contrast2;
            }
        }
        return fluidColor;
    }
}


