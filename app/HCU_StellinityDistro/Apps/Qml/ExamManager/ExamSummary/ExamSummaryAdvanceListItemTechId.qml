import QtQuick 2.12
import "../../Util.js" as Util
import "../../Widgets"

ExamSummaryAdvanceListItem {
    id: root

    labelText: translate("T_ExamField_TechnologistId_Name")
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

        var validList = Util.copyObject(paramData.ValidList);

        if (validList.length === 0)
        {
            widgetKeyboard.open(valueTextObject.parent, valueTextObject, paramData.ValidRange.LowerLimitInclusive, paramData.ValidRange.UpperLimitInclusive);
            widgetKeyboard.signalValueChanged.connect(slotKeyboardValChanged);
            widgetKeyboard.signalClosed.connect(slotKeyboardClosed);
        }
        else
        {
            // Add Keyboard Input option
            validList.push("\ue957");
            widgetInputPad.openSelectListPad(valueTextObject.parent, valueTextObject, validList, "");
            widgetInputPad.signalClosed.connect(slotInputPadClosed);
        }
    }

    Component.onCompleted: {
        setParamData();
    }

    function slotInputPadClosedInner(modified, newValue)
    {
        widgetKeyboard.signalValueChanged.disconnect(slotKeyboardValChanged);
        widgetKeyboard.signalClosed.disconnect(slotKeyboardClosed);
        widgetInputPad.signalClosed.disconnect(slotInputPadClosed);

        if (modified)
        {
            if (newValue === "\ue957")
            {
                valueTextObject.text = Qt.binding(function() { return valueText; });
                widgetKeyboard.open(valueTextObject.parent, valueTextObject, paramData.ValidRange.LowerLimitInclusive, paramData.ValidRange.UpperLimitInclusive);
                widgetKeyboard.signalValueChanged.connect(slotKeyboardValChanged);
                widgetKeyboard.signalClosed.connect(slotKeyboardClosed);
                return;
            }

            if (paramData.Value !== newValue)
            {
                if (newValue === "--")
                {
                    newValue = "";
                }
                paramData.Value = newValue;

                // Mask private data(TechID) in VNV and REL logs
                if ((hcuBuildType === "REL") || (hcuBuildType === "VNV"))
                {
                    var logData = Util.copyObject(paramData);
                    // Same as ANONYMIZED_STR on cpp side
                    logData.ValidList = "*****";
                    logData.Value = "*****";
                    logDebug("ExamSummaryAdvanceListItemTechId: Submitting New Data: " + JSON.stringify(logData));
                }
                else
                {
                    logDebug("ExamSummaryAdvanceListItemTechId: Submitting New Data: " + JSON.stringify(paramData));
                }
                dsCru.slotPostUpdateExamField(paramData);
            }

            // Invalidate the paramData. This is to force-update paramDataValue
            setParamData();
        }
        setDataBindings();
    }

    function slotInputPadClosed(modified)
    {
        slotInputPadClosedInner(modified, widgetInputPad.currentValue);
    }

    function slotKeyboardClosed(modified)
    {
        slotInputPadClosedInner(modified, widgetKeyboard.currentValue);
    }

    function slotKeyboardValChanged(newValue)
    {
    }

    function setParamData()
    {
        paramData = undefined;

        paramData = Qt.binding(function() {
            if  ( (examAdvanceInfo !== undefined) &&
                  (examAdvanceInfo.ExamResults !== undefined) &&
                  (examAdvanceInfo.ExamResults.TechnologistId !== undefined) )
            {
                return examAdvanceInfo.ExamResults.TechnologistId;
            }
            return undefined;
        });
    }
}
