import QtQuick 2.12
import "../../Widgets"
import "../../Widgets/Popup"
import "../../Util.js" as Util
import "AdminContrasts.js" as ContrastsUtil

Rectangle {
    property int fluidOptionContrastBrandLenMax: dsCapabilities.fluidOptionContrastBrandLenMax
    property int fluidOptionContrastConcentrationMin: dsCapabilities.fluidOptionContrastConcentrationMin
    property int fluidOptionContrastConcentrationMax: dsCapabilities.fluidOptionContrastConcentrationMax
    property int fluidOptionContrastVolumeMin: dsCapabilities.fluidOptionContrastVolumeMin
    property int fluidOptionContrastVolumeMax: dsCapabilities.fluidOptionContrastVolumeMax
    property int fluidOptionContrastMaxUseLifeHourMin: dsCapabilities.fluidOptionContrastMaxUseLifeHourMin
    property int fluidOptionContrastMaxUseLifeHourMax: dsCapabilities.fluidOptionContrastMaxUseLifeHourMax
    property int fluidOptionContrastBarcodeLenMin: dsCapabilities.fluidOptionContrastBarcodeLenMin
    property int fluidOptionContrastBarcodeLenMax: dsCapabilities.fluidOptionContrastBarcodeLenMax
    property var barcodeInfo: dsDevice.barcodeInfo

    property int selectedFamilyIdx: -1
    property int selectedGroupIdx: -1

    property int leftFrameWidth: (dsCfgLocal.screenW * 0.35) + (frameMargin * 2)
    property var fluidOptions: dsCfgLocal.fluidOptions
    property var contrastFamilies: [] // model data converted from fluidOptions
    property string reloadReason: "NONE" // "INIT", "REMOVED", "SELECTED", "ADDED", "MOVED", "MODIFIED", "NONE"
    property bool contrastFamiliesLoaded: false

    signal signalReload()

    id: contrastsPage
    visible: false
    anchors.fill: parent
    color: colorMap.mainBackground

    AdminContrastsList {
        id: contrastsList
        anchors.left: parent.left
        anchors.leftMargin: frameMargin
        width: leftFrameWidth - (frameMargin * 2)
        height: parent.height
    }

    Rectangle {
        // separator line
        anchors.left: contrastsList.right
        anchors.leftMargin: (frameMargin / 2)
        width: frameMargin / 6
        height: parent.height
        color: colorMap.titleBarBackground
    }

    AdminContrastsMain {
        id: contrastsMain
        anchors.right: parent.right
        anchors.rightMargin: frameMargin
        width: parent.width - leftFrameWidth - (frameMargin * 2)
        height: parent.height
    }

    AdminContrastsActionBar {
        id: contrastsActionBar
        Component.onCompleted: {
            this.parent = actionBar;
        }
    }

    onFluidOptionsChanged: {
        contrastFamiliesLoaded = false;
        ContrastsUtil.loadContrastFamiliesFromFluidOptionCfg();

        if (visible)
        {
            logWarning("AdminContrasts: Fluid options changed by other. Reloading the page..");
            reload();
        }
        else
        {
            logDebug("AdminContrasts: Fluid options changed by other");
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

        var fluidPackageAdded = false;

        if (barcodeInfo.Err !== "")
        {
            popupManager.popupBarcodeReadErr.contentText = "T_InvalidBarcodeScanned_Message";

            if (!popupManager.popupBarcodeReadErr.isOpen())
            {
                popupManager.popupBarcodeReadErr.open();
            }
        }
        else if (ContrastsUtil.addFluidPackageFromBarcodeInfo(barcodeInfo))
        {
            logDebug("Package added - barcodeInfo=" + JSON.stringify(barcodeInfo));
            fluidPackageAdded = true;
        }
        else
        {
            if (widgetInputPad.isOpen())
            {
                widgetInputPad.setCurrentValue(barcodeInfo.BarcodePrefix);
                widgetInputPad.close(true);
            }
            else
            {
                logDebug("Package barcode is added manually. barcodeInfo=" + JSON.stringify(barcodeInfo));
                ContrastsUtil.addBarcodePrefixFromBarcodeInfo();
            }
        }

        if (widgetInputPad.isOpen())
        {
            widgetInputPad.close(false);
        }

        reload();

        if (fluidPackageAdded)
        {
            // Select new fluid package
            for (var familyIdx = 0; familyIdx < contrastFamilies.length; familyIdx++)
            {
                var groups = contrastFamilies[familyIdx].Groups;
                for (var groupIdx = 0; groupIdx < groups.length; groupIdx++)
                {
                    var group = groups[groupIdx];
                    if ( (group.Brand.toLowerCase() === barcodeInfo.FluidPackage.Brand.toLowerCase()) &&
                         (group.Concentration === barcodeInfo.FluidPackage.Concentration) )
                    {
                        // Screen snap to new fluid package
                        selectedFamilyIdx = familyIdx;
                        selectedGroupIdx = groupIdx;
                        break;
                    }
                }
            }

            soundPlayer.playModalExpected();
        }
    }

    Component.onCompleted: {
        appMain.screenStateChanged.connect(slotScreenStateChanged);
        slotScreenStateChanged();
        this.signalReload.connect(reload);
    }

    Component.onDestruction: {
        appMain.screenStateChanged.disconnect(slotScreenStateChanged);
    }

    function slotScreenStateChanged()
    {
        var prevVisible = (appMain.screenStatePrev.indexOf("Admin-Contrasts") >= 0);
        visible = (appMain.screenState.indexOf("Admin-Contrasts") >= 0);

        if ( (visible) && (!prevVisible) )
        {
            ContrastsUtil.setReloadReason("INIT");
            if (!contrastFamiliesLoaded)
            {
                ContrastsUtil.loadContrastFamiliesFromFluidOptionCfg();
            }
        }
        else if ( (!visible) && (prevVisible))
        {
            // Exit from Admin/contrast, save cfg
            logDebug("AdminContrasts: Saving FluidOption Cfg");
            if (fluidOptions !== undefined)
            {
                var cfgErr = ContrastsUtil.getErrorFromFamiliesData();
                if (cfgErr !== "")
                {
                    logWarning("AdminContrasts: Config Err=" + cfgErr);
                }
                saveCfg();
            }
        }

        reload();
    }

    function reload()
    {
        if (!visible)
        {
            return;
        }

        //logDebug("AdminContrasts: reload(): Reload Reason=" + reloadReason);

        // Reorganise dummy groups
        ContrastsUtil.removeDummyGroups();
        ContrastsUtil.addDummyGroups();

        contrastsList.reload();
        contrastsMain.reload();

        ContrastsUtil.setReloadReason("NONE");
    }

    function saveCfg()
    {
        ContrastsUtil.removeDummyGroups();
        ContrastsUtil.reorderSelectedGroupFluidPackages();
        var contrastFamilies = ContrastsUtil.getContrastOptionsCfgFromContrastFamilies();
        if (contrastFamilies !== undefined)
        {
            fluidOptions.ContrastFamilies = contrastFamilies;
            dsCfgLocal.slotFluidOptionsChanged(fluidOptions);
        }
    }

    function saveFamilies(familiesData)
    {
        contrastFamilies = familiesData;
        reload();
    }
}
