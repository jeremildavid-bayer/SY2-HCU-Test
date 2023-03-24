import QtQuick 2.12
import "../../../Widgets"
import "../../../Util.js" as Util

Item {
    property var barcodeInfo: dsDevice.barcodeInfo
    property bool barcodeReaderReady: (barcodeReaderDisabledReason == "")
    property string barcodeReaderDisabledReason: getBarcodeReaderDisabledReason()
    property int barcodeReaderReadyDebounceMs: 100
    property string logPrefix: "DeviceManagerPanelBottleBarcodeScan[" + syringeIndex + "]: "

    Timer {
        id: tmrStartBarcodeReader
        interval: barcodeReaderReadyDebounceMs // Debounce time
        repeat: false
        onTriggered: {
            if (barcodeReaderReady)
            {
                dsDevice.slotBarcodeReaderStart(0);
            }
        }
    }

    onBarcodeReaderReadyChanged: {
        slotStartBarcodeReader();
    }

    onBarcodeReaderDisabledReasonChanged: {
        if (barcodeReaderDisabledReason == "")
        {
            logInfo(logPrefix + "BarcodeReader Enabled");
        }
        else
        {
            logInfo(logPrefix + "BarcodeReader Disabled: " + barcodeReaderDisabledReason);
        }
    }

    onBarcodeInfoChanged: {
        if (!visible)
        {
            return;
        }

        if (widgetKeyboard.isOpen())
        {
            widgetKeyboard.close(false);
        }
        if (widgetInputPad.isOpen())
        {
            widgetInputPad.close(false);
        }

        if (barcodeInfo.Err !== "")
        {
            // Bad barcode: popup
            openBarcodeReadErrPopup("T_InvalidBarcodeScanned_Message");
            return;
        }

        // See if barcode is defined
        // NOTE: Saline barcode logic:
        // ALL saline barcodes are undefined.
        // if undefined barcorde is known bayer barcode/or configured barcode, then show popup saying it is for contrast
        // otherwies if undefined barcode is scanned for Saline, fill in Batch and Exp if available
        var barcodeDefined = isBarcodeDefined();
        if (!barcodeDefined)
        {
            if (syringeIndex === 0)
            {
                // Scanned for Saline.
                if (barcodeInfo.IsKnownBarcode)
                {
                    showContrastScannedForSalinePopup();
                }
                else
                {
                    var salineLotBatchText = "--";
                    var salineExpirationText = "--";

                    if ( (barcodeInfo.FluidPackage.LotBatch !== undefined) &&
                            (barcodeInfo.FluidPackage.LotBatch !== "") &&
                            (barcodeInfo.FluidPackage.LotBatch.length >= fluidBottleLotBatchTextLenMin) )
                    {
                        salineLotBatchText = barcodeInfo.FluidPackage.LotBatch;
                    }

                    if ( (barcodeInfo.FluidPackage.ExpirationDate !== undefined) &&
                            (barcodeInfo.FluidPackage.ExpirationDate !== "") )
                    {
                        salineExpirationText = Util.utcDateTimeToExpiryFormat(barcodeInfo.FluidPackage.ExpirationDate);
                    }

                    paramList.lotBatchText = salineLotBatchText;
                    paramList.changeExpirationText(salineExpirationText);

                    logDebug("Barcode scanned for Saline. Filling in information");
                }

                return;
            }

            if (!barcodeInfo.IsKnownBarcode)
            {
                showUndefinedBarcodeScannedPopup();
                return;
            }

            handleUndefinedBarcode();
        }
        else if (syringeIndex === 0)
        {
            // Defined barcode is scanned for Saline
            showContrastScannedForSalinePopup();
            return;
        }

        // Check if valid (compatible to current family)
        var fillNewFluidPackageState = "Incompatible";

        for (var bottleItemIdx = 0; bottleItemIdx < fluidSelectItems.length; bottleItemIdx++)
        {
            var bottleItem = fluidSelectItems[bottleItemIdx];
            if ( (bottleItem.Brand.toLowerCase() === barcodeInfo.FluidPackage.Brand.toLowerCase()) )
            {
                // compatible
                if (bottleItem.Concentration === barcodeInfo.FluidPackage.Concentration)
                {
                    // same concentration
                    for (var volumeIdx = 0; volumeIdx < bottleItem.Volumes.length; volumeIdx++)
                    {
                        if (bottleItem.Volumes[volumeIdx] === barcodeInfo.FluidPackage.Volume)
                        {
                            volumeSelectedIndex = volumeIdx;
                            break;
                        }
                    }
                }
                else
                {
                    // look for next brand
                    continue;
                }

                if (isFillCurrentSupplyOk(barcodeInfo.FluidPackage))
                {
                    // Ok to fill - update fields from barcode info
                    bottleSelectedIndex = bottleItemIdx;
                    groupListView.selectItem(bottleSelectedIndex);
                    fillNewFluidPackageState = "OK";
                }
                else
                {
                    // Different group in compatible family selected.
                    newBottleSelectedIndex = bottleItemIdx;
                    newBottleSelected = Util.copyObject(barcodeInfo.FluidPackage);
                    fillNewFluidPackageState = "Different";
                }
                break;
            }
        }

        if (fillNewFluidPackageState === "Different")
        {
           if (executedSteps.length > 0)
           {
               fillNewFluidPackageState = "ExamInProgress";
           }
        }

        // Handle fillNewFluidPackageState
        if (fillNewFluidPackageState === "Incompatible")
        {
            // Incompatible barcode: popup
            openBarcodeReadErrPopup("T_IncompatibleBarcodeScanned_Message");
        }
        else if (fillNewFluidPackageState === "OK")
        {
            var newLotBatchText = "--";
            var newExpirationText = "--";

            if ( (barcodeInfo.FluidPackage.LotBatch !== undefined) &&
                 (barcodeInfo.FluidPackage.LotBatch !== "") &&
                 (barcodeInfo.FluidPackage.LotBatch.length >= fluidBottleLotBatchTextLenMin) )
            {
                newLotBatchText = barcodeInfo.FluidPackage.LotBatch;
            }

            if ( (barcodeInfo.FluidPackage.ExpirationDate !== undefined) &&
                 (barcodeInfo.FluidPackage.ExpirationDate !== "") )
            {
                newExpirationText = Util.utcDateTimeToExpiryFormat(barcodeInfo.FluidPackage.ExpirationDate);
            }

            paramList.lotBatchText = newLotBatchText;
            paramList.changeExpirationText(newExpirationText);

            logDebug(logPrefix + "Scanned: Loading..");

            if (checkBottleChanged())
            {
                loadBottle();
            }
        }
        else if (fillNewFluidPackageState === "ExamInProgress")
        {
            logWarning(logPrefix + "Scanned: Cannot replace during exam: " + JSON.stringify(barcodeInfo.FluidPackage));
            openBarcodeReadErrPopup("T_ReplaceSupply_PreventTypeChangeDuringExam");
        }
        else if (fillNewFluidPackageState === "Different")
        {
            logWarning(logPrefix + "Scanned: Needs to replace fluid package: " + JSON.stringify(barcodeInfo.FluidPackage));
            if (dsCfgGlobal.changeContrastEnabled) {
                popupManager.popupFluidRemoval.closed.connect(slotPopupFluidRemovalClosed);
                popupManager.popupFluidRemoval.init(syringeIndex);
            } else {
                logWarning(logPrefix + "Scanned: Needs to replace fluid package: but ChangeContrast is not enabled");
                popupManager.popupContrastChangeDisabled.open();
            }
        }
    }

    Component.onCompleted: {
        deviceManager.signalStartBarcodeReader.connect(slotStartBarcodeReader);
    }

    Component.onDestruction: {
        deviceManager.signalStartBarcodeReader.disconnect(slotStartBarcodeReader);
    }

    function getBarcodeReaderDisabledReason()
    {
        var disabledReason = "";

        if (fillDisabledReason != "")
        {
            disabledReason = fillDisabledReason;
        }
        return disabledReason;
    }

    function isBarcodeDefined()
    {
        for (var familyIdx = 0; familyIdx < fluidOptions.ContrastFamilies.length; familyIdx++)
        {
            var fluidPackages = fluidOptions.ContrastFamilies[familyIdx].FluidPackages;
            for (var fluidPackageIdx = 0; fluidPackageIdx < fluidPackages.length; fluidPackageIdx++)
            {
                var fluidPackage = fluidPackages[fluidPackageIdx];
                for (var barcodePrefixIdx = 0; barcodePrefixIdx < fluidPackage.BarcodePrefixes.length; barcodePrefixIdx++)
                {
                    if (fluidPackage.BarcodePrefixes[barcodePrefixIdx] === barcodeInfo.BarcodePrefix)
                    {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    function openBarcodeReadErrPopup(contentText)
    {
        if (!popupManager.popupBarcodeReadErr.isOpen())
        {
            popupManager.popupBarcodeReadErr.contentText = contentText
            popupManager.popupBarcodeReadErr.open();
        }
    }

    function showContrastScannedForSalinePopup()
    {
        logWarning(logPrefix + "Known Contrast Scanned For Saline." + JSON.stringify(barcodeInfo.FluidPackage));
        openBarcodeReadErrPopup("T_ContrastScannedForSalineLocation_Message");
    }

    function showUndefinedBarcodeScannedPopup()
    {
        logWarning(logPrefix + "UnKnown Barcode Scanned: Needs to replace fluid package: " + JSON.stringify(barcodeInfo.FluidPackage));
        openBarcodeReadErrPopup("T_UndefinedBarcodeScanned_Message");
    }

    function handleUndefinedBarcode()
    {
        // Check if Barcode's Name & Concentration already defined
        var handledState = "FamilyNotFound";
        var fluidPackageWithSameNameConcVolFound = false;

        for (var familyIdx = 0; familyIdx < fluidOptions.ContrastFamilies.length; familyIdx++)
        {
            if (handledState !== "FamilyNotFound")
            {
                // Package or Barcode added
                break;
            }

            var fluidPackages = fluidOptions.ContrastFamilies[familyIdx].FluidPackages;
            for (var fluidPackageIdx = 0; fluidPackageIdx < fluidPackages.length; fluidPackageIdx++)
            {
                var fluidPackage = fluidPackages[fluidPackageIdx];

                if ( (barcodeInfo.FluidPackage.Brand.toLowerCase() === fluidPackage.Brand.toLowerCase()) )
                {
                    handledState = "FamilyFound";
                    barcodeInfo.FluidPackage.CompatibilityFamily = fluidPackage.CompatibilityFamily;
                    // If for some reason the brand name is same but in different cases, add it with whatever is already on the shelf
                    barcodeInfo.FluidPackage.Brand = fluidPackage.Brand;
                }

                if ( (barcodeInfo.FluidPackage.Brand.toLowerCase() === fluidPackage.Brand.toLowerCase()) &&
                     (barcodeInfo.FluidPackage.Concentration === fluidPackage.Concentration) &&
                     (barcodeInfo.FluidPackage.Volume === fluidPackage.Volume) )
                {
                    // Package with same brand, concentration and volume found
                    logDebug(logPrefix + "handleUndefinedBarcode: Barcode's Brand(" + barcodeInfo.FluidPackage.Brand + ") & Conc(" + barcodeInfo.FluidPackage.Concentration + ") & Volume(" + barcodeInfo.FluidPackage.Volume + ") already exist in Family[" + familyIdx + "].fluidPackages[" + fluidPackageIdx + "]: Adding barcode..");
                    fluidPackage.BarcodePrefixes.push(barcodeInfo.BarcodePrefix);
                    handledState = "FamilyFound-BarcodeAdded";
                    break;
                }
            }

            if (handledState === "FamilyFound")
            {
                // Package with same brand and concentration found but no same volume found
                logDebug(logPrefix + "handleUndefinedBarcode: Barcode's Brand(" + fluidPackage.Brand + ") & Conc(" + fluidPackage.Concentration + ") already exist in Family[" + familyIdx + "]: Adding fluidPackage..");
                fluidPackages.push(barcodeInfo.FluidPackage);
                handledState = "FamilyFound-PackageAdded";
                break;
            }
        }

        if (handledState === "FamilyNotFound")
        {
            // See if any family with known barcode exist
            var firstFamilyIndexWithKnownBarcode = getFirstFamilyIndexWithKnownBarcode();

            if (firstFamilyIndexWithKnownBarcode !== -1)
            {
                // Add package to the existing family
                logDebug(logPrefix + "handleUndefinedBarcode: Adding to family with known barcode: Family[" + firstFamilyIndexWithKnownBarcode + "]");
                barcodeInfo.FluidPackage.CompatibilityFamily = fluidOptions.ContrastFamilies[firstFamilyIndexWithKnownBarcode].FluidPackages[0].CompatibilityFamily;
                fluidOptions.ContrastFamilies[firstFamilyIndexWithKnownBarcode].FluidPackages.splice(0, 0, barcodeInfo.FluidPackage);
                handledState = "FamilyNotFound-PackageAdded";
            }
            else
            {
                // Add package to the new family
                var contrastFamily = { FluidPackages: [ barcodeInfo.FluidPackage ], Name: "" };
                logDebug(logPrefix + "handleUndefinedBarcode: Adding to new family[" + fluidOptions.ContrastFamilies.length + "]: family=" + JSON.stringify(contrastFamily));
                fluidOptions.ContrastFamilies.push(contrastFamily);
                handledState = "FamilyNotFound-FamilyAdded";
            }
        }

        dsCfgLocal.slotFluidOptionsChanged(fluidOptions);

        if (!popupManager.popupBarcodeScannedKnownBarcodeAdded.isOpen())
        {
            popupManager.popupBarcodeScannedKnownBarcodeAdded.contentText = "T_KnownBarcodeAdded_Message;" + barcodeInfo.FluidPackage.Brand + ", " + barcodeInfo.FluidPackage.Concentration;
            popupManager.popupBarcodeScannedKnownBarcodeAdded.open();
        }
    }

    function getFirstFamilyIndexWithKnownBarcode()
    {
        var knownBarcodesMap = fluidOptions.KnownBarcodes;

        if (Object.keys(knownBarcodesMap).length == 0)
        {
            return -1;
        }

        for (var familyIdx = 0; familyIdx < fluidOptions.ContrastFamilies.length; familyIdx++)
        {
            var fluidPackages = fluidOptions.ContrastFamilies[familyIdx].FluidPackages;
            for (var fluidPackageIdx = 0; fluidPackageIdx < fluidPackages.length; fluidPackageIdx++)
            {
                var fluidPackage = fluidPackages[fluidPackageIdx];
                for (var barcodePrefixIdx = 0; barcodePrefixIdx < fluidPackage.BarcodePrefixes.length; barcodePrefixIdx++)
                {
                    var barcodePrefix = fluidPackage.BarcodePrefixes[barcodePrefixIdx];
                    if (knownBarcodesMap[barcodePrefix] !== undefined)
                    {
                        // current barcode is known type
                        return familyIdx;
                    }
                }
            }
        }
        return -1;
    }

    function slotStartBarcodeReader()
    {
        tmrStartBarcodeReader.stop();

        if (barcodeReaderReady)
        {
            tmrStartBarcodeReader.start();
        }
        else
        {
            if (appMain.screenState !== "OFF")
            {
                dsDevice.slotBarcodeReaderStop();
            }
        }
    }

}


