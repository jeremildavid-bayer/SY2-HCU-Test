import QtQuick 2.12
import "../../Widgets"
import "../../Util.js" as Util

GenericButton {
    property int currentUtcOffsetMinutes: dsCfgLocal.currentUtcOffsetMinutes
    property var globalConfigs: dsCfgGlobal.configTable
    property int heightMin: visible ? ListView.view.height * 0.15 : 0
    property int heightMax: visible ? title.height + textDescription.height + rectNote.height : 0
    property int animationMs: 250
    property var configData
    property string titleTextStr: ""
    property string descriptionTextStr: ""
    property var validListTranslations
    property string translationKeyPrefix: ""
    property int integerSliderTickMax: 15
    property string cultureCode: dsCfgGlobal.cultureCode

    id: root

    width: ListView.view.width
    height: heightMin

    state: "CLOSED"

    states: [
        State { name: "OPEN" },
        State { name: "CLOSED" }
    ]

    transitions: [
        Transition {
            to: "OPEN"
            SequentialAnimation {
                ScriptAction { script: {
                        startEdit();
                    }
                }

                ParallelAnimation {
                    NumberAnimation { target: root; properties: 'height'; to: heightMax; duration: animationMs; }
                    PropertyAnimation { target: title; properties: 'height'; to: title.contentHeight * 1.5; duration: animationMs; }
                    PropertyAnimation { target: rectValue; properties: 'color'; to: colorMap.actionButtonBackground; duration: animationMs; }
                }
            }
        },
        Transition {
            to: "CLOSED"
            SequentialAnimation {
                ParallelAnimation {
                    NumberAnimation { target: root; properties: 'height'; to: heightMin; duration: animationMs; }
                    PropertyAnimation { target: title; properties: 'height'; to: heightMin; duration: animationMs; }
                    PropertyAnimation { target: rectValue; properties: 'color'; to: "transparent"; duration: animationMs; }
                }

                ScriptAction { script: {
                        if (!root.visible)
                        {
                            root.height = Qt.binding(function() { return heightMin; });
                        }
                    }
                }
            }
        }
    ]

    content: [
        Item {
            height: parent.height
            width: parent.width
            clip: true

            Text {
                id: title
                height: heightMin
                width: parent.width * 0.5
                font.pixelSize: heightMin * 0.26
                verticalAlignment: Text.AlignVCenter
                font.family: fontRobotoLight.name
                color: colorMap.text01
                text: translationRequired ? translate(titleTextStr) : titleTextStr
                wrapMode: Text.Wrap
                elide: Text.ElideRight
            }

            Text {
                id: textDescription
                anchors.top: title.bottom
                height: contentHeight
                width: title.width
                font.pixelSize: title.font.pixelSize * 0.78
                verticalAlignment: Text.AlignTop
                font.family: fontRobotoLight.name
                color: colorMap.text02
                wrapMode: Text.Wrap
                text: translationRequired ? translate(descriptionTextStr) : descriptionTextStr
            }

            Item {
                id: rectNote
                anchors.top: textDescription.bottom
                width: title.width
                height: 0
            }

            Rectangle {
                id: rectValue
                anchors.left: title.right
                anchors.leftMargin: parent.width * 0.05
                width: parent.width - x
                height: parent.height
                color: "transparent"

                Text {
                    id: textValue
                    anchors.right: textUnit.left
                    anchors.rightMargin: (textUnit.contentWidth == 0) ? 0 : parent.width * 0.02
                    anchors.left: parent.left
                    anchors.leftMargin: parent.width * 0.06
                    height: parent.height
                    font.pixelSize: heightMin * 0.3
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignRight
                    font.family: {
                        if ( (customConfigRows !== undefined) &&
                             (customConfigRows[configData.KeyName] !== undefined) &&
                             (customConfigRows[configData.KeyName].getFontFamily !== undefined) )
                        {
                            return customConfigRows[configData.KeyName].getFontFamily();
                        }
                        return fontRobotoBold.name;
                    }
                    wrapMode: Text.Wrap
                }

                Text {
                    id: textUnit
                    anchors.right: parent.right
                    anchors.rightMargin: parent.width * 0.06
                    width: contentWidth
                    height: parent.height
                    font.family: fontRobotoLight.name
                    font.pixelSize: textValue.font.pixelSize * 0.9
                    color: colorMap.text02
                    verticalAlignment: Text.AlignVCenter
                    text: getUnitValue()
                }
            }

            GenericToggleButton {
                id: toggleBtn
                anchors.right: parent.right
                anchors.rightMargin: parent.width * 0.01
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width * 0.13
                height: heightMin * 0.7
                onToggled: (activated) => {
                    if (configData.Value !== activated)
                    {
                        var oldConfigData = Util.copyObject(configData);
                        configData.Value = activated;
                        configList[index].Value = activated;
                        logInfo("SavingConfig: " + visibleScreenState + " : " + configData.KeyName + ": value changed: " + oldConfigData.Value + " -> " + configData.Value);
                        configTable.signalConfigChanged(configData);
                    }
                }
            }

            GenericSlider {
                id: sliderCtrl
                anchors.right: parent.right
                anchors.rightMargin: parent.width * 0.01
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width * 0.35
                height: heightMin * 0.85
                onSignalValueIndexChanged: (newValueIndex) => {
                    var newValue = configData.ValidRange.LowerLimitInclusive + (configData.ValidRange.ResolutionIncrement * newValueIndex);
                    if (configData.Value !== newValue)
                    {
                        var oldConfigData = Util.copyObject(configData);
                        configData.Value = newValue;
                        configList[index].Value = newValue;
                        logInfo("SavingConfig: " + visibleScreenState + " : " + configData.KeyName + ": value changed: " + oldConfigData.Value + " -> " + configData.Value);
                        configTable.signalConfigChanged(configData);
                    }
                }
            }
        },

        Rectangle {
            id: separatorLine
            y: parent.height - height
            width: parent.width
            height: heightMin * 0.01
            color: colorMap.text02
        }
    ]

    onBtnClicked: {
        if (state == "CLOSED")
        {
            listViewSettings.currentIndex = index;

            if (widgetKeyboard.isOpen())
            {
                widgetKeyboard.close(true);
            }

            if (widgetInputPad.isOpen())
            {
                widgetInputPad.close(true);
            }
            configTable.signalRowSelected(index);
        }
    }

    onCultureCodeChanged: {
        reload();
    }

    Component.onCompleted: {
        appMain.signalUpdateDateTime.connect(reloadDateTime);
        configTable.signalRowSelected.connect(slotRowSelected);
        configTable.signalReloadRow.connect(reloadAll);
        listViewSettings.dragStarted.connect(reset);
        reloadAll();
    }

    Component.onDestruction: {
        appMain.signalUpdateDateTime.disconnect(reloadDateTime);
        configTable.signalRowSelected.disconnect(slotRowSelected);
        configTable.signalReloadRow.disconnect(reloadAll);
        listViewSettings.dragStarted.disconnect(reset);
        widgetInputPad.signalValueChanged.disconnect(slotInputPadValChanged);
        widgetInputPad.signalClosed.disconnect(slotInputPadClosed);
        widgetKeyboard.signalClosed.disconnect(slotKeyboardClosed);
    }

    function reloadAll()
    {
        if (state == "OPEN")
        {
            return;
        }

        if (JSON.stringify(configList[index]) == JSON.stringify(configData))
        {
            return;
        }

        configData = Util.copyObject(configList[index]);
        reload();
    }

    function reload()
    {
        if (configData === undefined)
        {
            return;
        }

        // Set row info text
        if (translationRequired)
        {
            // Set translationKeyPrefix
            translationKeyPrefix = "T_ConfigItem_SRU_";

            if (globalConfigs !== undefined)
            {
                for (var cfgIdx = 0; cfgIdx < globalConfigs.length; cfgIdx++)
                {
                    if (globalConfigs[cfgIdx].KeyName === configData.KeyName)
                    {
                        translationKeyPrefix = "T_ConfigItem_";
                        break;
                    }
                }
            }

            titleTextStr = translationKeyPrefix + configData.KeyName + "_Name";
            descriptionTextStr = translationKeyPrefix + configData.KeyName + "_Description";
        }
        else
        {
            titleTextStr = configData.KeyName.substring(configKeyName.length, configData.KeyName.length);
            descriptionTextStr = "";
        }

        // Set visible state
        if ( (customConfigRows !== undefined) &&
             (customConfigRows[configData.KeyName] !== undefined) &&
             (customConfigRows[configData.KeyName].getVisibleState !== undefined) )
        {
            root.visible = Qt.binding(function() { return customConfigRows[configData.KeyName].getVisibleState(); });
        }

        // Get customised description
        if ( (customConfigRows !== undefined) &&
             (customConfigRows[configData.KeyName] !== undefined) &&
             (customConfigRows[configData.KeyName].getDescription !== undefined) )
        {
            textDescription.text = Qt.binding(function() { return customConfigRows[configData.KeyName].getDescription(); });
        }

        // Set note field
        if ( (customConfigRows !== undefined) &&
             (customConfigRows[configData.KeyName] !== undefined) &&
             (customConfigRows[configData.KeyName].setNote !== undefined) )
        {
            customConfigRows[configData.KeyName].setNote(rectNote);
        }

        // Set row value
        rectValue.visible = true;
        toggleBtn.visible = false;
        sliderCtrl.visible = false;

        if (configData.ValidDataType === "bool")
        {
            rectValue.visible = false;
            toggleBtn.visible = true;
            toggleBtn.activated = configData.Value;
        }
        else if (isIntegerSliderType())
        {
            rectValue.visible = false;
            sliderCtrl.visible = true;

            var sliderCtrlLabels = [];
            for (var label = configData.ValidRange.LowerLimitInclusive; label <= configData.ValidRange.UpperLimitInclusive; label += configData.ValidRange.ResolutionIncrement)
            {
                sliderCtrlLabels.push(label);
            }

            if ( (customConfigRows !== undefined) &&
                 (customConfigRows[configData.KeyName] !== undefined) )
            {
                if (customConfigRows[configData.KeyName].sliderCtrlLabelStart !== undefined)
                {
                    sliderCtrlLabels[0] = customConfigRows[configData.KeyName].sliderCtrlLabelStart;
                }
            }

            sliderCtrl.setLabels(sliderCtrlLabels);
            sliderCtrl.setValue(configData.Value)
        }
        else if (configData.ValidDataType === "list")
        {
            validListTranslations = [];

            if ( (customConfigRows !== undefined) &&
                 (customConfigRows[configData.KeyName] !== undefined) &&
                 (customConfigRows[configData.KeyName].getValidList !== undefined) )
            {
                validListTranslations = customConfigRows[configData.KeyName].getValidList(configData.ValidList);
            }
            else if (translationRequired)
            {
                for (var listIdx = 0; listIdx < configData.ValidList.length; listIdx++)
                {
                    var translationKey = translationKeyPrefix + configData.KeyName + "_" + configData.ValidList[listIdx];
                    validListTranslations.push(translate(translationKey));
                }
            }
            else
            {
                validListTranslations = configData.ValidList;
            }
        }

        textValue.text = Qt.binding(function() { return getTextValue(configData.Value); });
        textValue.color = Qt.binding(function() { return colorMap.text01; });
    }

    function reloadDateTime()
    {
        if ( (customConfigRows !== undefined) &&
             (customConfigRows[configData.KeyName] !== undefined) &&
             (customConfigRows[configData.KeyName].updateFromRealDateTime !== undefined) &&
             (customConfigRows[configData.KeyName].updateFromRealDateTime === true) &&
             (state == "CLOSED") )
        {
            reload();
        }
    }

    function slotEditClosed(modified, newValue)
    {
        state = "CLOSED";
        textValue.text = Qt.binding(function() { return getTextValue(configData.Value); });
        textValue.color = Qt.binding(function() { return colorMap.text01; });

        if ( (!modified) &&
              (configData.ValidDataType !== "curDateTime") )
        {
            // No need to change config
            // For dateTime config, always apply cfg as the current date/time might be changed while editing.
            return;
        }

        var oldConfigData = Util.copyObject(configData);

        if ( (customConfigRows !== undefined) &&
             (customConfigRows[configData.KeyName] !== undefined) &&
             (customConfigRows[configData.KeyName].getTextFromConfigValue !== undefined) &&
             (customConfigRows[configData.KeyName].getConfigValueFromInputPad !== undefined) )
        {
            configData = customConfigRows[configData.KeyName].getConfigValueFromInputPad(configData, newValue);
        }
        else if (configData.ValidDataType === "list")
        {
            // Revert the translation and commit the cfg
            for (var i = 0; i < validListTranslations.length; i++)
            {
                if (validListTranslations[i] === newValue)
                {
                    configData.Value = configData.ValidList[i];
                    break;
                }
            }
        }
        else
        {
            configData.Value = (newValue === "--") ? "" : newValue;
        }

        if (configData.ValidDataType === "int")
        {
            configData.Value = parseInt(configData.Value);
        }
        else if (configData.ValidDataType === "double")
        {
            configData.Value = localeFromFloatStr(configData.Value);
        }
        else
        {
            configData.Value = configData.Value.toString();
        }

        if (configTable.selectedRow === index)
        {
            configTable.selectedRow = -1;
        }

        if (configData.Value !== oldConfigData.Value)
        {
            logInfo("SavingConfig: " + visibleScreenState + " : " + configData.KeyName + ": value changed: " + oldConfigData.Value + " -> " + configData.Value);
            configTable.signalConfigChanged(configData);
        }
        else
        {
            logDebug("ConfigNotChanged: " + visibleScreenState + " : " + configData.KeyName + ": value is " + configData.Value);
        }

        // Force to reload the row, just in case the row cannot be reloaded due to data not changed
        timerSingleShot(20, function() {
            reload();
        });
    }

    function slotKeyboardClosed(modified)
    {
        widgetKeyboard.signalClosed.disconnect(slotKeyboardClosed);
        slotEditClosed(modified, widgetKeyboard.currentValue);
    }

    function slotInputPadValChanged(newValue)
    {
        if (state == "CLOSED")
        {
            return;
        }

        if ( (configData.ValidDataType === "int") ||
             (configData.ValidDataType === "double") )
        {
            // Don't need to customise the editing value
        }
        else
        {
            textValue.text = getTextValue(newValue);
        }
    }

    function slotInputPadClosed(modified)
    {
        widgetInputPad.signalClosed.disconnect(slotInputPadClosed);
        slotEditClosed(modified, widgetInputPad.currentValue);
    }

    function getTextValue(curValue)
    {
        if ( (customConfigRows !== undefined) &&
             (customConfigRows[configData.KeyName] !== undefined) &&
             (customConfigRows[configData.KeyName].getTextFromConfigValue !== undefined) )
        {
            return customConfigRows[configData.KeyName].getTextFromConfigValue(curValue);
        }
        else if (curValue === "")
        {
            return "--";
        }
        else if (configData.ValidDataType === "passcode")
        {
            var passcode = "";

            for (var i = 0; i < curValue.length; i++)
            {
                passcode += "*";
            }

            if (passcode == "")
            {
                passcode = "--";
            }

            return passcode;
        }
        else if (configData.ValidDataType === "list")
        {
            if (translationRequired)
            {
                var translationKey = translationKeyPrefix + configData.KeyName + "_" + curValue;
                return translate(translationKey);
            }
        }
        else if (configData.ValidDataType === "double")
        {
            var resolution = configData.ValidRange.ResolutionIncrement;
            var digits = 0;
            if (resolution < 1)
            {
                digits = resolution.toString().length - resolution.toString().indexOf(".") - 1;
            }
            return localeToFloatStr(curValue, digits);
        }

        return curValue;
    }

    function getUnitValue()
    {
        if (configData.Units === "")
        {
            return "";
        }
        else if (!translationRequired)
        {
            return configData.Units;
        }
        return translate("T_Units_" + configData.Units);
    }

    function slotRowSelected(rowIndex)
    {
        if (rowIndex === index)
        {
            // Open edit panel
            state = "OPEN";
        }
        else
        {
            state = "CLOSED";
        }
    }

    function startEdit()
    {
        widgetInputPad.signalValueChanged.disconnect(slotInputPadValChanged);
        widgetInputPad.signalClosed.disconnect(slotInputPadClosed);
        widgetKeyboard.signalClosed.disconnect(slotKeyboardClosed);

        if ( (customConfigRows !== undefined) &&
             (customConfigRows[configData.KeyName] !== undefined) &&
             (customConfigRows[configData.KeyName].setConfigValueBeforeEdit !== undefined) )
        {
            // Set config before edit.
            customConfigRows[configData.KeyName].setConfigValueBeforeEdit(configData);
            logDebug("slotRowSelected: " + configData.KeyName + ": Value changed to " + configData.Value + " before edit");
        }

        if ( (customConfigRows !== undefined) &&
             (customConfigRows[configData.KeyName] !== undefined) &&
             (customConfigRows[configData.KeyName].onRowSelected !== undefined) )
        {
            customConfigRows[configData.KeyName].onRowSelected(rectValue, textValue, configData);
        }
        else if (configData.ValidDataType === "bool")
        {
        }
        else if (configData.ValidDataType === "int")
        {
            if (isIntegerSliderType())
            {
                // slider widget
            }
            else
            {
                widgetInputPad.setBackgroundSlide();
                widgetInputPad.openIntegerPad(rectValue, textValue, textUnit.text, configData.ValidRange.LowerLimitInclusive, configData.ValidRange.UpperLimitInclusive);
            }
        }
        else if (configData.ValidDataType === "object")
        {
            return;
        }
        else if (configData.ValidDataType === "list")
        {
            widgetInputPad.setBackgroundSlide();
            widgetInputPad.openSelectListPad(rectValue, textValue, validListTranslations, "");
        }
        else if (configData.ValidDataType === "double")
        {
            widgetInputPad.setBackgroundSlide();
            widgetInputPad.openFloatPad(rectValue, textValue, textUnit.text, 1, configData.ValidRange.LowerLimitInclusive, configData.ValidRange.UpperLimitInclusive);
        }
        else if (configData.ValidDataType === "numeric")
        {
            widgetInputPad.setBackgroundSlide();
            widgetInputPad.openTextPad(rectValue, textValue, configData.ValidRange.LowerLimitInclusive, configData.ValidRange.UpperLimitInclusive);
        }
        else if (configData.ValidDataType === "passcode")
        {
            widgetInputPad.setBackgroundSlide();
            widgetInputPad.openTextPad(rectValue, textValue, configData.ValidRange.LowerLimitInclusive, configData.ValidRange.UpperLimitInclusive);
            widgetInputPad.startValue = configData.Value;
            widgetInputPad.setCurrentValue(configData.Value);
        }
        else if (configData.ValidDataType === "curDateTime")
        {
            widgetInputPad.setBackgroundSlide();
            widgetInputPad.openDateTimePad(rectValue, textValue, "yyyy/MM/dd hh:mm");
        }
        else if (configData.ValidDataType === "string")
        {
            widgetKeyboard.open(rectValue, textValue, configData.ValidRange.LowerLimitInclusive, configData.ValidRange.UpperLimitInclusive);
        }
        else
        {
            logError("Failed to open edit pad: configData = " + JSON.stringify(configData));
        }

        if (widgetInputPad.isOpen())
        {
            widgetInputPad.signalValueChanged.connect(slotInputPadValChanged);
            widgetInputPad.signalClosed.connect(slotInputPadClosed);
        }

        if (widgetKeyboard.isOpen())
        {
            widgetKeyboard.signalClosed.connect(slotKeyboardClosed);
        }
    }

    function isIntegerSliderType()
    {
        if (configData.ValidDataType === "int")
        {
            if ( (customConfigRows !== undefined) &&
                 (customConfigRows[configData.KeyName] !== undefined) &&
                 (customConfigRows[configData.KeyName].editType !== undefined) )
            {
                if (customConfigRows[configData.KeyName].editType === "keypad")
                {
                    return false;
                }
                else if (customConfigRows[configData.KeyName].editType === "slider")
                {
                    return true;
                }
            }

            if (((configData.ValidRange.UpperLimitInclusive - configData.ValidRange.LowerLimitInclusive) / configData.ValidRange.ResolutionIncrement) < integerSliderTickMax)
            {
                return true;
            }
        }
        return false;
    }
}

