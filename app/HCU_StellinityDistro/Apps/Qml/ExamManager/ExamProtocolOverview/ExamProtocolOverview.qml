import QtQuick 2.12
import "../../Widgets"
import "../../Util.js" as Util

Item {
    readonly property string pressureLimitIcon:"\ue947"
    readonly property string contrastTypeIcon: "\ue92f"
    readonly property string sudsLengthTranslationPrefix: "T_SUDSLength_"
    // SUDSLength Icon is SVG
    readonly property string sudsLengthIcon: imageMap.sudsLength

    property string pressureUnit: dsCfgGlobal.pressureUnit
    property var pressureCfgOptions: dsCfgLocal.pressureCfgOptions
    property var availableSUDSLengthOptions: dsCfgGlobal.availableSUDSLengthOptions
    property var fluidSourceSyringePackages2: dsDevice.fluidSourceSyringePackages2
    property var fluidSourceSyringePackages3: dsDevice.fluidSourceSyringePackages3
    property var fluidSourceSyringe2: dsDevice.fluidSourceSyringe2
    property var fluidSourceSyringe3: dsDevice.fluidSourceSyringe3
    property var injectionRequestProcessStatus: dsExam.injectionRequestProcessStatus
    property var pressureCfgOptionKpaValues: []
    property var plan: dsExam.plan
    property var selectedContrast: dsExam.selectedContrast
    property var executedSteps: dsExam.executedSteps
    property var executingStep: dsExam.executingStep
    property bool isContrastSelectAllowed: dsExam.isContrastSelectAllowed
    property string selectedSUDSLength: dsExam.selectedSUDSLength
    property int comboBoxHeight: parent.height * 0.13
    property int comboBoxSpacing: parent.height * 0.02
    property string cultureCode: dsCfgGlobal.cultureCode
    property bool sudsInserted: dsMcu.sudsInserted

    property var dropDowns: [ cmbPressureLimit, cmbContrastType, cmbSUDSLengthOptions ]

    visible: false

    GenericComboBox {
        id: cmbPressureLimit
        anchors.top: parent.top
        anchors.topMargin: parent.height * 0.1
        width: parent.width * 0.85
        rowHeight: comboBoxHeight
        height: rowHeight
        itemValueTextFontPixelSize: comboBoxHeight * 0.35
        itemUnitTextFontPixelSize: comboBoxHeight * 0.32
        onSignalCurrentIndexChanged: (index) => {
            if (appMain.screenState !== "ExamManager-ProtocolModification")
            {
                // combo box closed in other screen. Don't apply value changed
                return;
            }

            // Find pressureLimitKpa from pressureCfgOptions
            var pressureLimit = localeFromFloatStr(optionList[index].value);
            var pressureLimitKpa = -1;

            if (pressureUnit == "kpa")
            {
                pressureLimitKpa = pressureLimit;
            }
            else
            {
                var curValueIdx = -1;

                // Get pressure option values for current type
                for (var optionIdx = 0; optionIdx < pressureCfgOptions.length; optionIdx++)
                {
                    if (pressureCfgOptions[optionIdx].Unit === pressureUnit)
                    {
                        var optionValues = pressureCfgOptions[optionIdx].Values;
                        for (var valueIdx = 0; valueIdx < optionValues.length; valueIdx++)
                        {
                            if (optionValues[valueIdx] === pressureLimit)
                            {
                                curValueIdx = valueIdx;
                                break;
                            }
                        }
                        break;
                    }
                }

                if (curValueIdx == -1)
                {
                    //logDebug("PresureLimitOptionChanged: Unexpected option value selected. Convert to kPa using formula.");
                    pressureLimitKpa = Util.getPressureKpa(pressureUnit, pressureLimit).toFixed(0);
                }
                else
                {
                    //logDebug("PresureLimitOptionChanged: " + pressureLimit + pressureUnit + " -> PressureCfgOptionKpaValues[" + curValueIdx +"] -> " + pressureCfgOptionKpaValues[curValueIdx] + "kPa");
                    pressureLimitKpa = pressureCfgOptionKpaValues[curValueIdx];
                }
            }

            logInfo("ExamProtocolOverView: PresureLimitOptionChanged: " + pressureLimit + pressureUnit + "(" + pressureLimitKpa + "kPa)");
            dsExam.slotPressureLimitChanged(pressureLimitKpa);
        }
        onSignalOpened: {
            closeAll(this.id);
        }
    }

    GenericComboBox {
        id: cmbContrastType
        anchors.top: cmbPressureLimit.bottom
        anchors.topMargin: parent.height * 0.04
        width: parent.width * 0.85
        rowHeight: comboBoxHeight
        height: rowHeight
        itemIconTextFontPixelSize: comboBoxHeight * 0.38
        itemValueTextFontPixelSize: comboBoxHeight * 0.32
        itemUnitTextFontPixelSize: comboBoxHeight * 0.35
        itemValueColor: colorMap.text02
        itemUnitColor: colorMap.text01
        itemValueFontFamily: fontRobotoLight.name
        itemUnitFontFamily: fontRobotoBold.name
        onSignalCurrentIndexChanged: {
            if ( (appMain.screenState !== "ExamManager-ProtocolModification") ||
                 (injectionRequestProcessStatus.State === "T_ARMING") )
            {
                // combo box closed in other screen. Don't apply value changed
                return;
            }
            logInfo("ExamProtocolOverView: Contrast Selected - " + optionList[index].value + " " + optionList[index].unit);
            dsExam.slotContrastTypeChanged(optionList[index].value, optionList[index].unit);
        }
        onSignalOpened: {
            closeAll(this.id);
        }
    }

    GenericComboBox {
        id: cmbSUDSLengthOptions
        anchors.top: cmbContrastType.bottom
        anchors.topMargin: parent.height * 0.04
        width: parent.width * 0.85
        rowHeight: comboBoxHeight
        height: rowHeight

        visible: (btnPreload.visible && dsCfgGlobal.extendedSUDSAvailable)

        readOnly: (visible && (executingStep !== undefined) && executingStep.IsPreloaded)

        // This is not used since it's svg. Keep it anyway
        itemIconTextFontPixelSize: comboBoxHeight * 0.38

        itemValueTextFontPixelSize: comboBoxHeight * 0.32
        itemUnitTextFontPixelSize: comboBoxHeight * 0.35

        translateValueText: true

        useSvgIcon: true
        svgIconSource: sudsLengthIcon
        svgIconWidth: height * 0.4
        svgIconHeight: svgIconWidth

        onSignalCurrentIndexChanged: {
            if ( (appMain.screenState !== "ExamManager-ProtocolModification") ||
                 (injectionRequestProcessStatus.State === "T_ARMING") )
            {
                // combo box closed in other screen. Don't apply value changed
                return;
            }
            var value = optionList[index].value.replace(sudsLengthTranslationPrefix, "");
            logInfo("ExamProtocolOverView: SUDS Length Selected - " + value);
            dsExam.slotSUDSLengthChanged(value);
        }
        onSignalOpened: {
            closeAll(this.id);
        }
    }

    GenericButton {
        id: btnPreload
        anchors.top: cmbSUDSLengthOptions.bottom
        anchors.topMargin: parent.height * 0.04
        width: parent.width * 0.85
        height: comboBoxHeight
        color: colorMap.keypadButton
        visible: (plan !== undefined) && (plan.IsPreloadable)
        enabled: {
            if (sudsInserted !== true)
            {
                return false;
            }

            if (executingStep === undefined)
            {
                return false;
            }

            if (executingStep.IsTestInjection)
            {
                return false;
            }

            if (!executingStep.IsPreloaded)
            {
                // Button state = preload. Make sure the step contains fluid phase.
                var fluidPhaseExist = false;
                for (var phaseIdx = 0; phaseIdx < executingStep.Phases.length; phaseIdx++)
                {
                    if (executingStep.Phases[phaseIdx].Type === "Fluid")
                    {
                        fluidPhaseExist = true;
                        break;
                    }
                }

                if (!fluidPhaseExist)
                {
                    return false;
                }
            }

            return true;
        }

        content: [
            Image {
                id: iconPreload
                anchors.left: parent.left
                anchors.leftMargin: parent.width * 0.06
                anchors.verticalCenter: parent.verticalCenter
                width: height * 0.85
                height: parent.height * 0.5
                source: btnPreloadText.text === translate("T_Reprime") ? imageMap.reprimeProtocol : imageMap.preloadProtocol
                sourceSize.width: width
                sourceSize.height: height
            },
            Text {
                id: btnPreloadText
                font.family: fontRobotoLight.name
                text: {
                    var preloadText = "T_Preload";
                    if (executingStep === undefined)
                    {
                    }
                    else if (!executingStep.IsPreloaded)
                    {
                    }
                    else
                    {
                        var fluidPhaseExist = false;
                        for (var phaseIdx = 0; phaseIdx < executingStep.Phases.length; phaseIdx++)
                        {
                            if (executingStep.Phases[phaseIdx].Type === "Fluid")
                            {
                                fluidPhaseExist = true;
                                break;
                            }
                        }
                        if (fluidPhaseExist)
                        {
                            preloadText = "T_Reprime";
                        }
                    }
                    return translate(preloadText);
                }
                anchors.left: iconPreload.right
                anchors.leftMargin: parent.width * 0.048
                anchors.right: parent.right
                height: parent.height
                font.pixelSize: height * 0.35
                color: colorMap.text01
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.Wrap
            }
        ]

        onBtnClicked: {
            if (executingStep === undefined)
            {
                return;
            }

            if (executingStep.IsPreloaded)
            {
                popupManager.popupReprimeFromInjectionPreloaded.open();
            }
            else
            {
                dsWorkflow.slotPreloadProtocolStart(true);
            }
        }
    }

    ExamProtocolOverviewInner {
        id: rectProtocolOverview
        height: parent.height * 0.21
        anchors.bottom: parent.bottom
        anchors.bottomMargin: parent.height * 0.01
        anchors.left: parent.left
        anchors.right: parent.right
        // there is flyout item (injectionStepReview) on top that's not easily accesible from here. Providing margin rather than anchoring to flyout to avoid overlapping
        anchors.rightMargin: frameMargin / 2
        selectedPlan: plan
        planExecutedSteps: executedSteps
        planExecutingStep: executingStep
    }

    onFluidSourceSyringe2Changed: {
        reloadContrastOptionValues();
    }

    onFluidSourceSyringe3Changed: {
        reloadContrastOptionValues();
    }

    onFluidSourceSyringePackages2Changed: {
        reloadContrastOptionValues();
    }

    onFluidSourceSyringePackages3Changed: {
        reloadContrastOptionValues();
    }

    onIsContrastSelectAllowedChanged: {
        reloadContrastOptionValues();
    }

    onSelectedContrastChanged: {
        reloadContrastValue();
    }

    onExecutingStepChanged: {
        reloadPressureLimit();
    }

    onPressureUnitChanged: {
        reloadPressureOptionValues();
        reloadPressureLimit();
    }

    onPressureCfgOptionsChanged: {
        reloadPressureOptionKpaValues();
        reloadPressureOptionValues();
        reloadPressureLimit();
    }

    onPlanChanged: {
        reloadPressureOptionValues();
    }

    onAvailableSUDSLengthOptionsChanged: {
        reloadAvailableSUDSLengthOptionValues();
    }

    onSelectedSUDSLengthChanged: {
        reloadSelectedSUDSLengthValue();
    }

    onCultureCodeChanged: {
        reload();
    }

    Component.onCompleted: {
        appMain.screenStateChanged.connect(slotScreenStateChanged);
        slotScreenStateChanged();
    }

    Component.onDestruction: {
        appMain.screenStateChanged.disconnect(slotScreenStateChanged);
    }

    function slotScreenStateChanged()
    {
        visible = (appMain.screenState === "ExamManager-ProtocolModification");
        reload();
    }

    function reloadContrastValue()
    {
        if (!visible)
        {
            return;
        }

        if (selectedContrast === undefined)
        {
            return;
        }

        var contrastAvailable = (selectedContrast.FluidPackage.LoadedAt !== undefined);
        cmbContrastType.enableSignalCurrentIndexChanged = false;

        cmbContrastType.setCurrentValue({   unit: contrastAvailable ? selectedContrast.FluidPackage.Concentration : "",
                                            icon: contrastTypeIcon,
                                            value: contrastAvailable ? selectedContrast.FluidPackage.Brand : translate("T_Contrast"),
                                            iconColor: (selectedContrast.ColorCode === "GREEN") ? colorMap.contrast1 : colorMap.contrast2
                                        });
        cmbContrastType.enableSignalCurrentIndexChanged = true;
    }

    function reloadPressureLimit()
    {
        if (!visible)
        {
            return;
        }

        var pressureLimit;
        if (executingStep !== undefined)
        {
            pressureLimit = Util.getPressure(pressureUnit, executingStep.PressureLimit);
        }
        else
        {
            // current executing step is not defined. set last one
            var lastExecutingStep = plan.Steps[executedSteps.length - 1];
            pressureLimit = Util.getPressure(pressureUnit, lastExecutingStep.PressureLimit);
        }

        var pressureLimitStr;
        if (pressureUnit == "kg/cm2")
        {
            pressureLimitStr = localeToFloatStr(pressureLimit, 1);
        }
        else
        {
            pressureLimitStr = localeToFloatStr(pressureLimit, 0);
        }

        cmbPressureLimit.enableSignalCurrentIndexChanged = false;
        cmbPressureLimit.setCurrentValue({ unit: pressureUnit, icon: pressureLimitIcon, value: pressureLimitStr });
        cmbPressureLimit.enableSignalCurrentIndexChanged = true;
    }

    function reloadPressureOptionKpaValues()
    {
        if (!visible)
        {
            return;
        }

        if (pressureCfgOptions === undefined)
        {
            return;
        }

        for (var i = 0; i < pressureCfgOptions.length; i++)
        {
            if (pressureCfgOptions[i].Unit === "kpa")
            {
                pressureCfgOptionKpaValues = pressureCfgOptions[i].Values;
                break;
            }
        }
    }

    function reloadPressureOptionValues()
    {
        if (!visible)
        {
            return;
        }

        if (pressureCfgOptions === undefined)
        {
            return;
        }

        // Update pressure options
        for (var i = 0; i < pressureCfgOptions.length; i++)
        {
            if (pressureCfgOptions[i].Unit === pressureUnit)
            {
                var optionList = [];
                for (var optionIdx = 0; optionIdx < pressureCfgOptions[i].Values.length; optionIdx++)
                {
                    var optionValue = pressureCfgOptions[i].Values[optionIdx];
                    var optionValueStr;
                    if (pressureUnit == "kg/cm2")
                    {
                        optionValueStr = localeToFloatStr(optionValue, 1);
                    }
                    else
                    {
                        optionValueStr = localeToFloatStr(optionValue, 0);
                    }

                    var optionItem = { unit: pressureUnit, icon: pressureLimitIcon, value: optionValueStr };
                    optionList.push(optionItem);
                }
                cmbPressureLimit.setOptionList(optionList);
                break;
            }
        }


        if (executedSteps.length < plan.Steps.length)
        {
            cmbPressureLimit.readOnly = false;
        }
        else
        {
            cmbPressureLimit.readOnly = true;
        }
    }

    function reloadContrastOptionValues()
    {
        if (!visible)
        {
            return;
        }

        var contrastOptionList = [];
        var contrastItem1 = null;
        if (fluidSourceSyringePackages2.length > 0)
        {
            contrastItem1 = {
                unit: fluidSourceSyringePackages2[0].Concentration,
                icon: contrastTypeIcon,
                value: fluidSourceSyringePackages2[0].Brand,
                iconColor: colorMap.contrast1
            };

            if (fluidSourceSyringe2.IsReady && dsWorkflow.contrastAvailable1)
            {
                contrastOptionList.push(contrastItem1);
            }
        }

        if (fluidSourceSyringePackages3.length > 0)
        {
            var contrastItem2 = {
                unit: fluidSourceSyringePackages3[0].Concentration,
                icon: contrastTypeIcon,
                value: fluidSourceSyringePackages3[0].Brand,
                iconColor: colorMap.contrast2
            };

            if (fluidSourceSyringe3.IsReady && dsWorkflow.contrastAvailable2)
            {
                if ( (contrastItem1 != null) &&
                     (contrastItem1.unit === contrastItem2.unit) &&
                     (contrastItem1.value === contrastItem2.value) )
                {
                    // Same  contrast type
                    if (fluidSourceSyringe3.IsReady)
                    {
                        // Already Added
                    }
                    else
                    {
                        contrastItem2.iconColor = colorMap.contrast1;
                        contrastOptionList.push(contrastItem2);
                    }
                }
                else
                {
                    contrastOptionList.push(contrastItem2);
                }
            }
        }

        //logDebug("reloadContrastOptionValues(): contrastOptionList=" + JSON.stringify(contrastOptionList));

        // Update contrast options
        if (!Util.compareObjects(cmbContrastType.optionList, contrastOptionList))
        {
            cmbContrastType.setOptionList(contrastOptionList);
        }
        reloadContrastValue();

        cmbContrastType.readOnly = ( (!isContrastSelectAllowed) || (contrastOptionList.length <= 1) );
    }

    function reloadAvailableSUDSLengthOptionValues()
    {
        if ((!visible) || (availableSUDSLengthOptions === undefined))
        {
            return;
        }

        var optionList = [];
        for (var i = 0; i < availableSUDSLengthOptions.length; i++)
        {
            // icon is not needed since SUDSLength is using SVG icon. However it requires string value so we set it to some dummy string
            var optionItem = { unit: "", icon: "", value: (sudsLengthTranslationPrefix + availableSUDSLengthOptions[i]) };
            optionList.push(optionItem);
        }
        cmbSUDSLengthOptions.setOptionList(optionList);

        reloadSelectedSUDSLengthValue();
    }

    function reloadSelectedSUDSLengthValue()
    {
        cmbSUDSLengthOptions.enableSignalCurrentIndexChanged = false;

        // icon is not needed since SUDSLength is using SVG icon. However it requires string value so we set it to some dummy string
        cmbSUDSLengthOptions.setCurrentValue( { unit: "", icon: "", value: (sudsLengthTranslationPrefix + selectedSUDSLength) } );
        cmbSUDSLengthOptions.enableSignalCurrentIndexChanged = true;
    }

    function closeAll(exception)
    {
        for (var i = 0; i < dropDowns.length; i++) {
            if ((exception === undefined) || (dropDowns[i] !== exception)) {
                dropDowns[i].close();
            }
        }
    }

    function reload()
    {
        if (!visible)
        {
            closeAll();
            return;
        }

        reloadPressureOptionKpaValues();
        reloadPressureOptionValues();
        reloadContrastOptionValues();
        reloadPressureLimit();
        reloadAvailableSUDSLengthOptionValues();
    }
}
