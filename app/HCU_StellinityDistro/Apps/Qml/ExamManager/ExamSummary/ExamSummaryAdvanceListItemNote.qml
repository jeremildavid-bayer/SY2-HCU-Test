import QtQuick 2.12
import "../../Util.js" as Util
import "../../Widgets"

ExamSummaryAdvanceListItem {
    id: root

    property string quickNotes: dsCfgGlobal.quickNotes

    property var currentSelectedOptions

    property real quickNoteContentHeight

    labelText: translate("T_ExamField_Notes_Name")
    valueText: {
        if ( (paramData !== undefined) &&
             (paramData.Value !== "") )
        {
            return paramData.Value;
        }
        return "--";
    }

    onBtnClicked: {
        if (widgetInputPad.isOpen())
        {
            widgetInputPad.close(true);
        }

        if (widgetKeyboard.isOpen())
        {
            widgetKeyboard.close(true);
        }

        widgetInputPad.signalClosed.disconnect(slotInputPadClosed);
        widgetKeyboard.signalValueChanged.disconnect(slotKeyboardValChanged);
        widgetKeyboard.signalClosed.disconnect(slotKeyboardClosed);


        var validList = JSON.parse(quickNotes);

        if (validList.length === 0)
        {
            openKeyboard();
        }
        else
        {
            // Prepare Note Text Options
            var options = [];
            var valueTextBuf = valueText;
            for (var itemIdx = 0; itemIdx < validList.length; itemIdx++)
            {
                var itemTitle = validList[itemIdx];
                var strFound = (valueTextBuf.indexOf(itemTitle) >= 0);
                options.push({ "title": itemTitle, "value": strFound });
            }

            // deep copying array
            currentSelectedOptions = JSON.parse(JSON.stringify(options));

            // add keyboard option at the end (last "true")
            widgetInputPad.openMultiSelectListPad(valueTextObject.parent, valueTextObject, options, true);
            widgetInputPad.signalClosed.connect(slotInputPadClosed);
            widgetInputPad.signalValueChanged.connect(slotInputValChanged);
        }
    }

    Component.onCompleted: {
        setParamData();
    }

    function evaluateCurrentSelectedOptions(options, noteString)
    {
        // Prepare Note Text Options
        currentSelectedOptions = [];
        var valueTextBuf = noteString;
        for (var itemIdx = 0; itemIdx < options.length; itemIdx++)
        {
            var itemTitle = options[itemIdx].title;
            var strFound = (valueTextBuf.indexOf(itemTitle) >= 0);
            currentSelectedOptions.push({ "title": itemTitle, "value": strFound });
        }
    }

    function slotInputPadClosedInner(modified, newValue)
    {
        widgetKeyboard.signalValueChanged.disconnect(slotKeyboardValChanged);
        widgetKeyboard.signalClosed.disconnect(slotKeyboardClosed);
        widgetInputPad.signalValueChanged.disconnect(slotInputValChanged);
        widgetInputPad.signalClosed.disconnect(slotInputPadClosed);

        if (modified)
        {
            if (paramData.Value === newValue)
            {
                setDataBindings();
                return;
            }

            if ( (newValue.length > 0) &&
                 (newValue.charAt(newValue.length - 1) === ' ') )
            {
                newValue = newValue.substring(0, newValue.length - 1);
            }

            if ( (newValue.length > 0) &&
                 (newValue.charAt(0) === ' ') )
            {
                newValue = newValue.substring(1, newValue.length);
            }

            if (paramData.Value !== newValue)
            {
                if (newValue === "--")
                {
                    newValue = "";
                }

                paramData.Value = newValue;

                logDebug("ExamSummaryAdvanceListItemNotes: Submitting New Data: [" + JSON.stringify(paramData) + "]");
                dsCru.slotPostUpdateExamField(paramData);
            }

            // Invalidate the paramData. This is to force-update paramDataValue
            setParamData();
        }

        setDataBindings();
    }

    function slotInputPadClosed(modified)
    {
        var newValue = getStringFromInputPadVal(widgetInputPad.currentValue);
        slotInputPadClosedInner(modified, newValue);
    }

    function slotKeyboardClosed(modified)
    {
        slotInputPadClosedInner(modified, widgetKeyboard.currentValue);
        flickAdvanceFields.returnToBounds();
    }

    function slotInputValChanged(newValue)
    {
        if (typeof newValue === "string")
        {
            if (newValue === "\ue957")
            {
                // save what's toggled already before opening keybaord
                if (widgetInputPad.isOpen())
                {
                    widgetInputPad.close(true);
                }

                // Keyboard Input Option selected
                widgetInputPad.signalValueChanged.disconnect(slotInputValChanged);
                widgetInputPad.signalClosed.disconnect(slotInputPadClosed);
                openKeyboard();

                widgetKeyboard.textSetWrapMode(TextEdit.Wrap);
                if (widgetKeyboard.currentValue !== "--")
                {
                    // Open keyboard and prepare for append to end if there is room
                    if ((widgetKeyboard.currentValue.length + 1) < paramData.ValidRange.UpperLimitInclusive)
                    {
                        widgetKeyboard.setCurrentValue(widgetKeyboard.currentValue + " ");
                    }

                    widgetKeyboard.textDeselect();
                    widgetKeyboard.textCursorPosition(widgetKeyboard.currentValue.length);
                }
            }
        }
        else
        {
            // newValue is Seleted Input Option List
            valueText = getStringFromInputPadVal(newValue);

            // refresh the options list with correct valid options
            var newOptions = JSON.parse(JSON.stringify(currentSelectedOptions));
            widgetInputPad.resetMultiSelectListPadOptions(newOptions);
        }
    }

    function openKeyboard()
    {
        widgetKeyboard.open(valueTextObject.parent, valueTextObject, paramData.ValidRange.LowerLimitInclusive, paramData.ValidRange.UpperLimitInclusive);
        widgetKeyboard.signalValueChanged.connect(slotKeyboardValChanged);
        widgetKeyboard.signalClosed.connect(slotKeyboardClosed);

        quickNoteContentHeight = valueTextObject.contentHeight;
    }

    function slotKeyboardValChanged(newValue)
    {
        valueTextObject.text = newValue;

        if (quickNoteContentHeight !== valueTextObject.contentHeight)
        {
            widgetKeyboard.shiftMainWindow(valueTextObject.parent);

            flickAdvanceFields.contentY += (valueTextObject.contentHeight - quickNoteContentHeight);
            quickNoteContentHeight = valueTextObject.contentHeight;
        }
    }

    function setParamData()
    {
        paramData = undefined;

        paramData = Qt.binding(function() {
            if  ( (examAdvanceInfo !== undefined) &&
                  (examAdvanceInfo.ExamResults !== undefined) &&
                  (examAdvanceInfo.ExamResults.Notes !== undefined) )
            {
                return examAdvanceInfo.ExamResults.Notes;
            }
            return undefined;
        });
    }

    function getStringFromInputPadVal(selectedOpts)
    {
        var strBuf = valueText;

        if (strBuf == "--")
        {
            strBuf = "";
        }

        for (var optIdx = 0; optIdx < selectedOpts.length; optIdx++)
        {
            var optItem = selectedOpts[optIdx];

            // only deal with what's changed
            if ((optItem.value === undefined) || (currentSelectedOptions[optIdx].value === optItem.value))
            {
                continue;
            }

            //var optFoundPos = strBuf.indexOf(" " + optItem.title + " ");
            var optFoundPos = strBuf.indexOf(optItem.title);
            if (optItem.value)
            {
                if (optFoundPos == -1)
                {
                    if ((strBuf.length + optItem.title.length + 1) > paramData.ValidRange.UpperLimitInclusive)
                    {
                        // don't add new string if it exceeds length limit
                        optItem.value = false;
                    }
                    else
                    {
                        // Opt selected but value not exist. Append it
                        if (strBuf.length !== 0)
                        {
                            strBuf += " ";
                        }

                        strBuf += optItem.title;
                    }
                }
            }
            else
            {
                // value should be removed
                if (optFoundPos >= 0)
                {
                    strBuf = strBuf.substring(0, optFoundPos) + strBuf.substring(optFoundPos + optItem.title.length, strBuf.length);
                }
            }
        }

        strBuf = strBuf.trim();

        if ((strBuf === "") || (strBuf === " "))
        {
            strBuf = "--";
        }

        evaluateCurrentSelectedOptions(selectedOpts, strBuf);

        return strBuf;
    }
}
