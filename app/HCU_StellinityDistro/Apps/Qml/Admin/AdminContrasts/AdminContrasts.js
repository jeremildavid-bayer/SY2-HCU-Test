function loadContrastFamiliesFromFluidOptionCfg()
{
    contrastsPage.contrastFamilies = getContrastFamiliesFromFluidOptionsCfg();
    contrastsPage.contrastFamiliesLoaded = true;
}

// Get Contrast Family model: includes <NEW> items
function getContrastFamiliesFromFluidOptionsCfg()
{
    var contrastFamiliesBuf = [];
    if ( (fluidOptions === undefined) ||
         (fluidOptions.ContrastFamilies === undefined) )
    {
        return;
    }

    // Convert CFG structure to UI friendly structure
    for (var familyIdx = 0; familyIdx < fluidOptions.ContrastFamilies.length; familyIdx++)
    {
        var fluidPackages = fluidOptions.ContrastFamilies[familyIdx].FluidPackages;
        for (var fluidPackageIdx = 0; fluidPackageIdx < fluidPackages.length; fluidPackageIdx++)
        {
            var fluidPackage = fluidPackages[fluidPackageIdx];

            // Find ContrastFamily entry
            var contrastFamilyFound = false;

            var group = { Brand: fluidPackage.Brand,
                          Concentration: fluidPackage.Concentration,
                          FluidPackages: [ Util.copyObject(fluidPackage) ] };

            // Insert group to compatible family group
            for (var insertFamilyIdx = 0; insertFamilyIdx < contrastFamiliesBuf.length; insertFamilyIdx++)
            {
                if (contrastFamiliesBuf[insertFamilyIdx].Name === fluidPackage.CompatibilityFamily)
                {
                    var groups = contrastFamiliesBuf[insertFamilyIdx].Groups;
                    for (var groupIdx = 0; groupIdx< groups.length; groupIdx++)
                    {
                        if ( (groups[groupIdx].Brand.toLowerCase() === fluidPackage.Brand.toLowerCase()) &&
                             (groups[groupIdx].Concentration === fluidPackage.Concentration) )
                        {
                            //logInfo("getContrastFamiliesFromFluidOptionsCfg: Adding package to Family[" + insertFamilyIdx + "].Groups[" + groupIdx + "]..");
                            contrastFamiliesBuf[insertFamilyIdx].Groups[groupIdx].FluidPackages.push(fluidPackage);
                            contrastFamilyFound = true;
                            break;
                        }
                    }

                    if (!contrastFamilyFound)
                    {
                        //logInfo("getContrastFamiliesFromFluidOptionsCfg: Adding package to Family[" + insertFamilyIdx + "].Groups[NEW(" + contrastFamiliesBuf[insertFamilyIdx].Groups.length + ")]..");
                        contrastFamiliesBuf[insertFamilyIdx].Groups.push(group);
                        contrastFamilyFound = true;
                    }

                    //logDebug("getContrastFamiliesFromFluidOptionsCfg: contrastFamiliesBuf[" + insertFamilyIdx + "].Groups=" + JSON.stringify(contrastFamiliesBuf[insertFamilyIdx].Groups));
                    break;
                }
            }

            if (!contrastFamilyFound)
            {
                var contrastFamily = { Groups: [ group ], Name: Util.copyObject(fluidPackage.CompatibilityFamily) };
                contrastFamiliesBuf.push(contrastFamily);
            }
        }
    }

    return contrastFamiliesBuf;
}

// Convert UI structure to Cfg structure
function getContrastOptionsCfgFromContrastFamilies()
{
    //logDebug("contrastFamilyModel = " + JSON.stringify(contrastFamilyModel, null, " "));
    //logDebug("\n\ncontrastFamilies = " + JSON.stringify(contrastFamilies, null, " "));
    var contrastFamiles = contrastsPage.contrastFamilies;
    var contrastOptions = [];

    if (contrastFamiles == undefined)
    {
        return undefined;
    }

    for (var familyIdx = 0; familyIdx < contrastFamiles.length; familyIdx++)
    {
        var contrastFamilyName = "Family" + (familyIdx + 1).toString();
        var groups = contrastFamiles[familyIdx].Groups;
        var allFluidPackages = [];

        for (var groupIdx = 0; groupIdx < groups.length; groupIdx++)
        {
            var groupData = groups[groupIdx];
            var groupDataErr = getErrorFromGroupData(groupData, familyIdx, groupIdx);
            if (groupDataErr !== "")
            {
                logWarning("Ignoring bad Fluid Group Data. familyIdx=" + familyIdx + ", groupIdx=" + groupIdx + ", Err=" + groupDataErr);
                return undefined;
            }

            var fluidPackages = groupData.FluidPackages;

            if (fluidPackages.length === 0)
            {
                allFluidPackages.push( { BarcodePrefixes: [],
                                         ConcentrationUnits: "MG_I_PER_ML",
                                         FluidKind: "Contrast",
                                         CompatibilityFamily: contrastFamilyName,
                                         Brand: groupData.Brand,
                                         Concentration: groupData.Concentration,
                                         Volume: 0 });
            }
            else
            {
                for (var fluidPackageIdx = 0; fluidPackageIdx < fluidPackages.length; fluidPackageIdx++)
                {
                    //logDebug(groupData.Brand + ": groups.length=" + groups.length + ", fluidPackages.length=" + fluidPackages.length);

                    var fluidPackage = fluidPackages[fluidPackageIdx];
                    var barcodePrefixes = [];

                    //logDebug("fluidPackages[" + fluidPackageIdx + "] = " + JSON.stringify(fluidPackage));

                    // Set barcodePrefixes
                    for (var barcodeIdx = 0; barcodeIdx < fluidPackage.BarcodePrefixes.length; barcodeIdx++)
                    {
                        barcodePrefixes.push(fluidPackage.BarcodePrefixes[barcodeIdx]);
                    }

                    // Set FluidPackage
                    fluidPackage.BarcodePrefixes = barcodePrefixes;
                    fluidPackage.CompatibilityFamily = contrastFamilyName;
                    fluidPackage.Brand = groupData.Brand;
                    fluidPackage.Concentration = groupData.Concentration;
                    fluidPackage.ConcentrationUnits = "MG_I_PER_ML";
                    fluidPackage.FluidKind = "Contrast";
                    allFluidPackages.push(fluidPackage);
                }
            }
        }

        if (allFluidPackages.length > 0)
        {
            var contrastFamily = { Name: contrastFamilyName, FluidPackages: allFluidPackages };
            contrastOptions.push(contrastFamily);
        }
    }
    logDebug("contrastOptions = " + JSON.stringify(contrastOptions));
    return contrastOptions;
}

function reorderSelectedGroupFluidPackages()
{
    //logDebug("reorderSelectedGroupFluidPackages()");
    var groupDataBuf = Util.copyObject(getGroupData(selectedFamilyIdx, selectedGroupIdx));
    if (groupDataBuf === undefined)
    {
        // No contrasts exist
        return;
    }

    var fluidPackages = groupDataBuf.FluidPackages;

    // Apply Bubble Sort
    for (var packageIdx1 = 0; packageIdx1 < fluidPackages.length - 1; packageIdx1++)
    {
        var swapped = false;
        for (var packageIdx2 = 0; packageIdx2 < fluidPackages.length - 1 - packageIdx1; packageIdx2++)
        {
            var idx0 = packageIdx2;
            var idx1 = packageIdx2 + 1;

            if (fluidPackages[idx0].Volume < fluidPackages[idx1].Volume)
            {
                // swap
                var fluidPackageBuf = Util.copyObject(fluidPackages[idx0]);
                fluidPackages[idx0] = Util.copyObject(fluidPackages[idx1]);
                fluidPackages[idx1] = fluidPackageBuf;
                swapped = true;
            }
        }

        if (!swapped)
        {
            break;
        }
    }
    setGroupData(selectedFamilyIdx, selectedGroupIdx, groupDataBuf);
}

function addDummyGroups()
{
    logDebug("addDummyGroups()");
    if (contrastsPage.contrastFamilies === undefined)
    {
        return;
    }

    // Add Dummy Group if required
    var contrastFamiliesBuf = Util.copyObject(contrastsPage.contrastFamilies);

    var dummyContrastFluidPackage = { Brand: "<NEW>",
                                      BarcodePrefixes: [],
                                      Volume: 0 }; // Row with '+' button

    var emptyContrastFluidPackage = { Brand: "<NEW*>",
                                      BarcodePrefixes: [],
                                      Volume: 0 };

    var dummyContrastGroup =  { Brand: "<NEW>",
                                Concentration: 0,
                                FluidPackages: [ emptyContrastFluidPackage ] };

    var dummyContrastFamily = { Groups: [ dummyContrastGroup ],
                                Name: "Family" + (contrastFamiliesBuf.length + 1).toString() };

    // Add dummy group to each family
    for (var familyIdx = 0; familyIdx < contrastFamiliesBuf.length; familyIdx++)
    {
        var groups = contrastFamiliesBuf[familyIdx].Groups;
        if ( (groups.length === 0) ||
             (groups[groups.length - 1].Brand !== "<NEW>") )
        {
            // Add dummy group after the last group, <NEW>
            //logDebug("Adding dummy group to familyIdx=" + familyIdx + ", groupIdx=" + groups.length);
            var dummyGroup = Util.copyObject(dummyContrastGroup);
            contrastFamiliesBuf[familyIdx].Groups.push(dummyGroup);
        }
    }

    // Add dummy group to last family
    if ( (contrastFamiliesBuf.length === 0) ||
         (contrastFamiliesBuf[contrastFamiliesBuf.length - 1].Groups.length > 1) )
    {
        //logDebug("Adding last dummy group to familyIdx=" + contrastFamiliesBuf.length + ", groupIdx=" + 0);
        contrastFamiliesBuf.push(dummyContrastFamily);
    }

    // Add dummy fluid package to each group
    for (familyIdx = 0; familyIdx < contrastFamiliesBuf.length; familyIdx++)
    {
        groups = contrastFamiliesBuf[familyIdx].Groups;
        for (var groupIdx = 0; groupIdx < groups.length; groupIdx++)
        {
            if (contrastFamiliesBuf[familyIdx].Groups[groupIdx].FluidPackages.length === 0)
            {
                var emptyPackage = Util.copyObject(emptyContrastFluidPackage);
                contrastFamiliesBuf[familyIdx].Groups[groupIdx].FluidPackages.push(emptyPackage);
            }

            //logDebug("Adding dummy fluidPackage to familyIdx=" + familyIdx + ", groupIdx=" + groupIdx);
            //logDebug("groups[groupIdx]=" + JSON.stringify(groups[groupIdx]));
            var dummyPackage = Util.copyObject(dummyContrastFluidPackage);
            contrastFamiliesBuf[familyIdx].Groups[groupIdx].FluidPackages.push(dummyPackage);

            var group = contrastFamiliesBuf[familyIdx].Groups[groupIdx];
            for (var packageIdx = 0; packageIdx < group.FluidPackages.length; packageIdx++)
            {
                // Add dummy barcode to each fluid package
                contrastFamiliesBuf[familyIdx].Groups[groupIdx].FluidPackages[packageIdx].BarcodePrefixes.push("<NEW>");
            }
        }
    }
    contrastsPage.contrastFamilies = contrastFamiliesBuf;
}

function removeDummyGroups()
{
    logDebug("AdminContrasts: removeDummyGroups()");
    if (contrastsPage.contrastFamilies === undefined)
    {
        return;
    }

    var contrastFamiliesBuf = Util.copyObject(contrastsPage.contrastFamilies);

    for (var familyIdx = 0; familyIdx < contrastFamiliesBuf.length; familyIdx++)
    {
        var groups = contrastFamiliesBuf[familyIdx].Groups;
        for (var groupIdx = 0; groupIdx < groups.length; groupIdx++)
        {
            var group = groups[groupIdx];
            var removeGroupOk = false;

            if (group.Brand === "<NEW>")
            {
                // New group (to be added)
                removeGroupOk = true;
            }
            else if ( (group.Brand === "") &&
                      (group.Concentration === 0) &&
                      (group.FluidPackages.length <= 2) &&
                      (group.FluidPackages[0].Volume === 0) &&
                      (group.FluidPackages[0].BarcodePrefixes.length === 1) )
            {
                // Empty group
                removeGroupOk = true;
            }

            if (removeGroupOk)
            {
                // Remove Dummy Group
                //logDebug("AdminContrasts: removeDummyGroups(): Removing Group[" + groupIdx + "]: " + JSON.stringify(group));
                contrastFamiliesBuf[familyIdx].Groups.splice(groupIdx, 1);
                groupIdx--;
                continue;
            }

            // Remove Dummy Package
            for (var packageIdx = 0; packageIdx < group.FluidPackages.length; packageIdx++)
            {
                var package = group.FluidPackages[packageIdx];
                if (package.Brand === "<NEW>")
                {
                    contrastFamiliesBuf[familyIdx].Groups[groupIdx].FluidPackages.splice(packageIdx, 1);
                    packageIdx--;
                    continue;
                }

                // Remove Dummy Barcodes
                for (var barcodeIdx = 0; barcodeIdx < package.BarcodePrefixes.length; barcodeIdx++)
                {
                    var removeBarcodePrefix = false;
                    if ( (package.BarcodePrefixes[barcodeIdx] === "<NEW>") ||
                         (package.BarcodePrefixes[barcodeIdx] === "") )
                    {
                        contrastFamiliesBuf[familyIdx].Groups[groupIdx].FluidPackages[packageIdx].BarcodePrefixes.splice(barcodeIdx, 1);
                        barcodeIdx--;
                        continue;
                    }
                }
            }
        }

        if (contrastFamiliesBuf[familyIdx].Groups.length === 0)
        {
            // Remove empty family
            contrastFamiliesBuf.splice(familyIdx, 1);
            familyIdx--;
        }
    }

    contrastsPage.contrastFamilies = contrastFamiliesBuf;
}

function saveFamilies(familiesData)
{
    //logDebug("AdminContrasts: SaveFamilies()");
    contrastsPage.saveFamilies(familiesData);
}

function saveGroup(groupData, familyIdx, groupIdx)
{
    logDebug("AdminContrasts: SaveGroup()");
    var contrastFamiliesBuf = Util.copyObject(contrastsPage.contrastFamilies);
    contrastFamiliesBuf[familyIdx].Groups[groupIdx] = groupData;
    contrastsPage.saveFamilies(contrastFamiliesBuf);
}

function removeGroup(familyIdx, groupIdx)
{
    logDebug("AdminContrasts: RemoveGroup()");
    var contrastFamiliesBuf = Util.copyObject(contrastsPage.contrastFamilies);
    contrastFamiliesBuf[familyIdx].Groups.splice(groupIdx, 1);
    contrastsPage.saveFamilies(contrastFamiliesBuf);
}

function setReloadReason(newReason)
{
    // Reason can be "INIT", "REMOVED", "SELECTED", "ADDED", "MOVED", "MODIFIED", "NONE"
    //logDebug("AdminContrasts: Reload Reason=" + newReason);
    contrastsPage.reloadReason = newReason;
}

function getGroupData(familyIdx, groupIdx)
{
    if (contrastsPage.contrastFamilies[familyIdx] === undefined)
    {
        return undefined;
    }

    return contrastsPage.contrastFamilies[familyIdx].Groups[groupIdx];
}

function setGroupData(familyIdx, groupIdx, groupData)
{
    if (contrastsPage.contrastFamilies[familyIdx] === undefined)
    {
        return;
    }

    return contrastsPage.contrastFamilies[familyIdx].Groups[groupIdx] = Util.copyObject(groupData);
}

function setFluidPackage(selectedFamilyIdx, selectedGroupIdx, packageIndex, packageData)
{
    contrastsPage.contrastFamilies[selectedFamilyIdx].Groups[selectedGroupIdx].FluidPackages[packageIndex] = Util.copyObject(packageData);
}

function getErrorFromGroupDataBrand(brand)
{
    if ( (brand.length === 0) ||
         (brand.length > fluidOptionContrastBrandLenMax) )
    {
        return "T_PUTFLUIDOPTIONSFAILED_InvalidBrand";
    }

    return "";
}

function getErrorFromGroupDataConcentration(concentration)
{
    concentration = parseInt(concentration);
    if ( (concentration < fluidOptionContrastConcentrationMin) ||
         (concentration > fluidOptionContrastConcentrationMax) )
    {
        var concentrationStr = (concentration === 0) ? "--" : concentration.toString();
        return "T_PUTFLUIDOPTIONSFAILED_InvalidConcentration;" + concentrationStr;
    }
    return "";
}

function getErrorFromGroupDataPackageVolume(volume, packageIdx, fluidPackages)
{
    var packageData = fluidPackages[packageIdx];

    if ( (!isNaN(packageData.MaximumUseDuration)) &&
         (packageData.BarcodePrefixes.length > 0) )
    {
        if (volume === 0)
        {
            return "T_PUTFLUIDOPTIONSFAILED_MissingVolume";
        }
    }

    var volumeStr = (volume === 0) ? "--" : volume.toString();

    if ( (volume !== 0) &&
         ( (volume < fluidOptionContrastVolumeMin) || (volume > fluidOptionContrastVolumeMax) ) )
    {
        return "T_PUTFLUIDOPTIONSFAILED_InvalidVolume;" + volumeStr;
    }

    for (var packageIdx2 = 0; packageIdx2 < fluidPackages.length; packageIdx2++)
    {
        if ( (packageIdx !== packageIdx2) &&
             (fluidPackages[packageIdx2].Brand !== "<NEW>") &&
             (volume === fluidPackages[packageIdx2].Volume) )
        {
            return "T_PUTFLUIDOPTIONSFAILED_DuplicateVolumeInFluidOption;" + volumeStr;
        }
    }
    return "";
}

function getErrorFromGroupDataPackageMaximumUseDuration(maximumUseDuration)
{
    if (maximumUseDuration === 0)
    {
        // maximumUseDuration is not specified
        return ""
    }
    else if ( (maximumUseDuration < fluidOptionContrastMaxUseLifeHourMin) ||
              (maximumUseDuration > fluidOptionContrastMaxUseLifeHourMax) )
    {
        var maximumUseDurationStr = (maximumUseDuration === 0) ? "--" : maximumUseDuration.toString();
        return "T_PUTFLUIDOPTIONSFAILED_InvalidMaximumUseDuration;" + maximumUseDurationStr;
    }
    return "";
}

function getErrorFromGroupDataBarcodePrefix(barcodePrefix, familyIdx, groupIdx, packageIdx, barcodeIdx)
{
    if ( (barcodePrefix === "<NEW>") ||
         (barcodePrefix === "<NEW_EDIT>") )
    {
        return "";
    }
    else if ( (barcodePrefix.length < fluidOptionContrastBarcodeLenMin) ||
              (barcodePrefix.length > fluidOptionContrastBarcodeLenMax) )
    {
        return "T_PUTFLUIDOPTIONSFAILED_InvalidBarcode;" + barcodePrefix.length;
    }
    else
    {
        // Check duplicated barcode
        if (checkBarcodeDuplicated(barcodePrefix, familyIdx, groupIdx, packageIdx, barcodeIdx))
        {
            return "T_PUTFLUIDOPTIONSFAILED_DuplicateBarcodePrefix;" + barcodePrefix;
        }
    }
    return ""
}

function getErrorFromFamiliesData()
{
    for (var familyIdx = 0; familyIdx < contrastFamilies.length; familyIdx++)
    {
        var groups = contrastFamilies[familyIdx].Groups;
        for (var groupIdx = 0; groupIdx < groups.length; groupIdx++)
        {
            var err = getErrorFromGroupData(groups[groupIdx], familyIdx, groupIdx);
            if (err !== "")
            {
                return "FamilyIndex=" + familyIdx + ", GroupIndex=" + groupIdx + ", Error=" + err;
            }
        }
    }
    return "";
}

function getErrorFromGroupData(groupData, familyIdx, groupIdx)
{
    if (groupData === undefined)
    {
        return "";
    }

    var err;

    if (groupData.Brand === "<NEW>")
    {
        return "";
    }

    if ((err = getErrorFromGroupDataBrand(groupData.Brand)) !== "")
    {
        return err;
    }

    if ((err = getErrorFromGroupDataConcentration(groupData.Concentration)) !== "")
    {
        return err;
    }

    if (checkContrastGroupExist(groupData.Brand, groupData.Concentration, familyIdx, groupIdx))
    {
        var concentrationStr = (groupData.Concentration === 0) ? "--" : groupData.Concentration.toString();
        return "T_PUTFLUIDOPTIONSFAILED_DuplicateFluidOption;" + groupData.Brand + " " + concentrationStr;
    }

    for (var packageIdx = 0; packageIdx < groupData.FluidPackages.length; packageIdx++)
    {
        var packageData = groupData.FluidPackages[packageIdx];

        if (packageData.Brand === "<NEW>")
        {
            continue;
        }

        if ((err = getErrorFromGroupDataPackageVolume(packageData.Volume, packageIdx, groupData.FluidPackages)) !== "")
        {
            return err;
        }

        if ((err = getErrorFromGroupDataPackageMaximumUseDuration(packageData.MaximumUseDuration)) !== "")
        {
            return err;
        }

        for (var barcodeIdx = 0; barcodeIdx < packageData.BarcodePrefixes.length; barcodeIdx++)
        {
            var barcodePrefix = packageData.BarcodePrefixes[barcodeIdx];

            if ((err = getErrorFromGroupDataBarcodePrefix(barcodePrefix, familyIdx, groupIdx, packageIdx, barcodeIdx)) !== "")
            {
                return err;
            }
        }
    }
    return "";
}

function checkContrastGroupExist(brand, concentration, excludeFamilyIdx, excludeGroupIdx)
{
    if (contrastFamilies === undefined)
    {
        return false;
    }

    for (var familyIdx = 0; familyIdx < contrastFamilies.length; familyIdx++)
    {
        var groups = contrastFamilies[familyIdx].Groups;
        for (var groupIdx = 0; groupIdx < groups.length; groupIdx++)
        {
            if ( (familyIdx === excludeFamilyIdx) &&
                 (groupIdx === excludeGroupIdx) )
            {
            }
            else if ( (groups[groupIdx].Brand.toLowerCase() === brand.toLowerCase()) &&
                      (groups[groupIdx].Concentration === concentration) )
            {
                return true;
            }
        }
    }
    return false;
}

function checkBarcodeDuplicated(barcodePrefix, excludeFamilyIdx, excludeGroupIdx, excludePackageIdx, excludeBarcodeIdx)
{
    var contrastFamilies = contrastsPage.contrastFamilies;

    if (contrastFamilies === undefined)
    {
        return false;
    }

    for (var familyIdx = 0; familyIdx < contrastFamilies.length; familyIdx++)
    {
        var groups = contrastFamilies[familyIdx].Groups;
        for (var groupIdx = 0; groupIdx < groups.length; groupIdx++)
        {
            var fluidPackages = groups[groupIdx].FluidPackages;
            for (var fluidPackageIdx = 0; fluidPackageIdx < fluidPackages.length; fluidPackageIdx++)
            {
                var barcodePrefixes = fluidPackages[fluidPackageIdx].BarcodePrefixes;
                for (var barcodePrefixIdx = 0; barcodePrefixIdx < barcodePrefixes.length; barcodePrefixIdx++)
                {
                    if (barcodePrefix === barcodePrefixes[barcodePrefixIdx])
                    {
                        if ( (familyIdx !== excludeFamilyIdx) ||
                             (groupIdx !== excludeGroupIdx) ||
                             (fluidPackageIdx !== excludePackageIdx) ||
                             (barcodePrefixIdx !== excludeBarcodeIdx) )
                        {
                            // same barcode prefix found
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

function addNewGroup(fluidPackage, familyIdx)
{
    var newGroup =  { Brand: fluidPackage.Brand,
                      Concentration: fluidPackage.Concentration,
                      FluidPackages: [ Util.copyObject(fluidPackage) ] };

    // Add new group to the temp group
    var groupIdx = contrastFamilies[familyIdx].Groups.length - 1;

    // Replace with new
    contrastFamilies[familyIdx].Groups[groupIdx] = newGroup;
}

function addNewFluidPackage(fluidPackage)
{
    for (var familyIdx = 0; familyIdx < contrastFamilies.length; familyIdx++)
    {
        var groups = contrastFamilies[familyIdx].Groups;
        for (var groupIdx = 0; groupIdx < groups.length; groupIdx++)
        {
            var group = groups[groupIdx];

            if ( (group.Brand.toLowerCase() === fluidPackage.Brand.toLowerCase()) &&
                 (group.Concentration === fluidPackage.Concentration) )
            {
                for (var packageIdx = 0; packageIdx < group.FluidPackages.length; packageIdx++)
                {
                    var package = group.FluidPackages[packageIdx];
                    if ( (package.Volume === fluidPackage.Volume) &&
                         (package.MaximumUseDuration === fluidPackage.MaximumUseDuration) )
                    {
                        // Same fluid package found - Add barcode prefix to the existing package
                        logDebug("AdminContrasts: addNewFluidPackage(): Same fluid package found - Add barcode prefix to the existing package[" + packageIdx + "]");
                        contrastFamilies[familyIdx].Groups[groupIdx].FluidPackages[packageIdx].BarcodePrefixes.push(fluidPackage.BarcodePrefixes[0]);
                        return;
                    }
                }

                // Replace with new
                for (packageIdx = 0; packageIdx < group.FluidPackages.length; packageIdx++)
                {
                    package = group.FluidPackages[packageIdx];
                    if (package.Brand === "<NEW>")
                    {
                        logDebug("AdminContrasts: addNewFluidPackage(): Add fluid package to new package[" + packageIdx + "]");
                        contrastFamilies[familyIdx].Groups[groupIdx].FluidPackages[packageIdx] = Util.copyObject(fluidPackage);
                        return;
                    }
                }

                logError("addNewFluidPackage(): Failed to add package");
            }
        }
    }
}

function addFluidPackageFromBarcodeInfo(barcodeInfo)
{
    if (!barcodeInfo.IsFluidPackageOk)
    {
        logDebug("AdminContrasts: addFluidPackageFromBarcodeInfo(): Failed to Add: Bad FluidPackage, barcodeInfo=" + JSON.stringify(barcodeInfo));
        return false;
    }

    if (!barcodeInfo.IsKnownBarcode)
    {
        logDebug("AdminContrasts: addFluidPackageFromBarcodeInfo(): Failed to Add: Unknown Barcode, barcodeInfo=" + JSON.stringify(barcodeInfo));
        return false;
    }

    var barcodeExist = checkBarcodeDuplicated(barcodeInfo.BarcodePrefix, -1, -1, -1, -1);

    if (barcodeExist)
    {
        logDebug("AdminContrasts: addFluidPackageFromBarcodeInfo(): Failed to Add: Barcode Exist, barcodeInfo=" + JSON.stringify(barcodeInfo));
        return false;
    }

    widgetInputPad.close(false);

    // New barcode in fluid option.
    var groupExist = checkContrastGroupExist(barcodeInfo.FluidPackage.Brand, barcodeInfo.FluidPackage.Concentration, -1, -1);

    if (groupExist)
    {
        logDebug("AdminContrasts: addFluidPackageFromBarcodeInfo(): Adding to existing group");
        addNewFluidPackage(barcodeInfo.FluidPackage);
    }
    else
    {
        logDebug("AdminContrasts: addFluidPackageFromBarcodeInfo(): Adding to new group in current family");
        addNewGroup(barcodeInfo.FluidPackage, contrastsPage.selectedFamilyIdx);
    }

    // Delete temporary barcode field <WAITING_BARCODE>
    for (var familyIdx = 0; familyIdx < contrastFamilies.length; familyIdx++)
    {
        var groups = contrastFamilies[familyIdx].Groups;
        for (var groupIdx = 0; groupIdx < groups.length; groupIdx++)
        {
            var group = groups[groupIdx];
            for (var packageIdx = 0; packageIdx < group.FluidPackages.length; packageIdx++)
            {
                var package = group.FluidPackages[packageIdx];
                for (var barcodeIdx = 0; barcodeIdx < package.BarcodePrefixes.length; barcodeIdx++)
                {
                    if ( (package.BarcodePrefixes[barcodeIdx] === "<WAITING_BARCODE>") ||
                         (package.BarcodePrefixes[barcodeIdx] === "<NEW_EDIT>") )
                    {
                        contrastFamilies[familyIdx].Groups[groupIdx].FluidPackages[packageIdx].BarcodePrefixes.splice(barcodeIdx, 1);
                        barcodeIdx--;
                        continue;
                    }
                }
            }
        }
    }

    contrastsPage.reload();
    return true;
}

function addBarcodePrefixFromBarcodeInfo()
{
    // Rename <WAITING_BARCODE> to barcode value
    for (var familyIdx = 0; familyIdx < contrastFamilies.length; familyIdx++)
    {
        var groups = contrastFamilies[familyIdx].Groups;
        for (var groupIdx = 0; groupIdx < groups.length; groupIdx++)
        {
            var group = groups[groupIdx];
            for (var packageIdx = 0; packageIdx < group.FluidPackages.length; packageIdx++)
            {
                var fluidPackage = groups[groupIdx].FluidPackages[packageIdx];
                for (var barcodeIdx = 0; barcodeIdx < fluidPackage.BarcodePrefixes.length; barcodeIdx++)
                {
                    if (fluidPackage.BarcodePrefixes[barcodeIdx] === "<WAITING_BARCODE>")
                    {
                        contrastFamilies[familyIdx].Groups[groupIdx].FluidPackages[packageIdx].BarcodePrefixes[barcodeIdx] = barcodeInfo.BarcodePrefix;
                    }
                }
            }
        }
    }
}
