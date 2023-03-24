import QtQuick 2.12
import "../../../Widgets"
import "../../../Util.js" as Util
import "../"

GenericButton {
    property var egfrParamData: egfrItem.Value.Parameters[index]
    property var paramDataName: {
        if (egfrParamData === undefined)
        {
            return "";
        }

        return translate("T_P3T_" + egfrParamData.Name + "_Name");
    }

    property var paramDataValue: {
        if ((egfrParamData === undefined) || (egfrParamData.IsValueVisible === false) || (egfrParamData.Value === "") || (egfrParamData.Value === undefined))
        {
            return "--";
        }
        //e.g. T_ParameterInput_Gender_Female
        return egfrParamData.NeedsTranslated? translate("T_ParameterInput_" + egfrParamData.Name + "_" + egfrParamData.Value) : egfrParamData.Value;
    }

    property var paramDataValidList: {
        var newValidList = [];
        if (egfrParamData === undefined) {
            return newValidList;
        }
        else
        {
            if (egfrParamData.NeedsTranslated){
                for (var listIdx = 0; listIdx < egfrParamData.ValidList.length; listIdx++)
                {
                    newValidList.push(translate("T_ParameterInput_" + egfrParamData.Name + "_" + egfrParamData.ValidList[listIdx]));
                }
                return newValidList;
            }
            return egfrParamData.ValidList;
        }
    }

    property var paramDataUnit: {
        if ((egfrParamData === undefined) || (egfrParamData.Units === undefined) || (egfrParamData.Units === null) || (egfrParamData.Units === ""))
        {
            return "";
        }

        return translate("T_Units_" + egfrParamData.Units);
    }

    property var dateFormatString : translate("T_ConfigItem_Settings_General_DateFormat_" + dsCfgGlobal.dateFormat)

    id: root
    radius: 0

    interactive: ((egfrParamData !== undefined) && egfrParamData.IsEnabled)

    Text {
        id: dataName
        height: parent.height * 0.9
        width: parent.width * 0.55
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignLeft
        wrapMode: Text.Wrap
        font.pixelSize: height * 0.34
        font.family: fontRobotoLight.name
        text: paramDataName
        color: interactive ? colorMap.text01 : colorMap.text02
        fontSizeMode: Text.Fit
        minimumPixelSize: font.pixelSize * 0.7
    }

    Rectangle {
        id: rectValue
        height: parent.height
        width: parent.width * 0.45
        anchors.right: parent.right
        anchors.leftMargin: parent.width * 0.023
        color: "transparent"

        ExamPatientSelectEditAdvanceParamListItemValue {
            id: rectEgfrSubItem
            paramData: egfrParamData
            visible: ((paramData !== undefined) && (paramData.ValidDataType !== "bool"))

            function getTextValue()
            {
                if (paramData.ValidDataType === "double" && paramDataValue !== "--")
                {
                    var decimalDigits = Util.getDecimalDigits(paramDataValue);
                    return localeToFloatStr(paramDataValue, decimalDigits);
                }
                else if (paramData.ValidDataType === "date" && paramDataValue !== "--")
                {
                    return Util.utcDateTimeToDateTimeStr(dateFormatString, "", 0, paramDataValue);
                }

                return paramDataValue;
            }

            function getUnitValue()
            {
                return paramDataUnit;
            }

            // override default color for text
            function getTextValueColor()
            {
                if (textValue.text !== "--")
                {
                    return (root.interactive ? colorMap.text01 : colorMap.text02);
                }
                else
                {
                    return "red";
                }
            }

            function getTextUnitColor()
            {
                return textValue.color;
            }
        }

        GenericToggleButton {
            id: toggleButton
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            width: parent.width * 0.4
            height: parent.height * 0.6
            isReadOnly: false
            visible: {
                if (egfrParamData === undefined)
                {
                    return false;
                }

                return (egfrParamData.ValidDataType === "bool");
            }
            activated: (paramDataValue === "True")

            onToggled: {
                egfrParamData.Value = (activated ? "True" : "False");
                dsCru.slotPostUpdateExamFieldParameter(egfrParamData);
            }
        }
    }

    Rectangle {
        id: separatorLine
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        width: parent.width
        height: (index < root.count - 1) ? 1 : 3
        color: colorMap.text02
    }

    Component.onCompleted: {
        egfrPanelList.dragStarted.connect(reset);
        egfrPanel.signalResumeEditEgfrParam.connect(resumeEdit);
    }

    Component.onDestruction: {
        egfrPanelList.dragStarted.disconnect(reset);
        egfrPanel.signalResumeEditEgfrParam.disconnect(resumeEdit);
    }

    onBtnClicked: {
        if (!toggleButton.visible)
        {
            startEdit();
        }
    }

    function slotInputPadValChanged(newValue)
    {
        rectEgfrSubItem.textValue.text = newValue;
    }

    function startEdit()
    {
        if (widgetInputPad.isOpen)
        {
            widgetInputPad.close(true);
        }

        egfrPanel.setCurEditParamInfo({"State": "EDITING_EGFR_PARAM", "Index": index});

        widgetInputPad.signalValueChanged.disconnect(slotInputPadValChanged);
        widgetInputPad.signalClosed.disconnect(slotInputPadClosed);

        if (egfrParamData.ValidDataType === "double")
        {
            var decimalDigits = Util.getDecimalDigits(egfrParamData.ValidRange.ResolutionIncrement.toString());
            widgetInputPad.openFloatPad(rectValue, rectEgfrSubItem.textValue, egfrParamData.Units, decimalDigits, egfrParamData.ValidRange.LowerLimitInclusive, egfrParamData.ValidRange.UpperLimitInclusive);
        }
        else if (egfrParamData.ValidDataType === "int")
        {
            widgetInputPad.openIntegerPad(rectValue, rectEgfrSubItem.textValue, egfrParamData.Units, egfrParamData.ValidRange.LowerLimitInclusive, egfrParamData.ValidRange.UpperLimitInclusive);
        }
        else if (egfrParamData.ValidDataType === "list")
        {
            widgetInputPad.openSelectListPad(rectValue, rectEgfrSubItem.textValue, paramDataValidList, egfrParamData.Units);
        }
        else if (egfrParamData.ValidDataType === "date")
        {

            // egfrParamDate.ValidRange.Lower/UpperLimits are number of days from current day
            var now = Date.now();
            var minDateInclusive = new Date(now + (egfrParamData.ValidRange.LowerLimitInclusive * 24 * 60 * 60 * 1000));
            var maxDateInclusive = new Date(now + (egfrParamData.ValidRange.UpperLimitInclusive * 24 * 60 * 60 * 1000));

            // set minDate to absolute minimum down to milliseconds level
            minDateInclusive.setHours(0,0,0);
            minDateInclusive.setMilliseconds(000);

            // set maxDate to absolute maximum down to milliseonds level
            maxDateInclusive.setHours(23,59,59);
            maxDateInclusive.setMilliseconds(999);

            widgetInputPad.openDateTimePad(rectValue, rectEgfrSubItem.textValue, dateFormatString, minDateInclusive, maxDateInclusive );
        }

        widgetInputPad.signalValueChanged.connect(slotInputPadValChanged);
        widgetInputPad.signalClosed.connect(slotInputPadClosed);
    }

    function resumeEdit()
    {
        var editParamInfo = egfrPanel.getCurEditParamInfo();
        if ( (editParamInfo.State === "EDITING_EGFR_PARAM") &&
             (editParamInfo.Index === index) )
        {
            startEdit();
        }
    }

    function doneEdit()
    {
        egfrPanel.setCurEditParamInfo({ "State": "IDLE" });
    }


    function slotInputPadClosed(modified)
    {
        widgetInputPad.signalValueChanged.disconnect(slotInputPadValChanged);
        widgetInputPad.signalClosed.disconnect(slotInputPadClosed);

        if (modified)
        {
            if (!(egfrParamData.IsValueVisible) || (egfrParamData.Value !== widgetInputPad.currentValue.toString()))
            {
                if (egfrParamData.ValidDataType === "double")
                {
                    var decimalDigits = Util.getDecimalDigits(egfrParamData.ValidRange.ResolutionIncrement.toString());
                    egfrParamData.Value = localeFromFloatStr(widgetInputPad.currentValue, decimalDigits).toString();
                }
                else if (egfrParamData.ValidDataType === "date")
                {
                    // need to do some manual offset work.. let's convert all to Millisec and convert at the end so we don't have to deal with date format string
                    // some places do correct date format conversion according to Config, many other places use "yyyy/mm/dd" hardcoded format
                    // converting to milliseconds should fix mismatching formats.. as long as Date.parse accepts the string

                    var birthDayInMillisec = Util.utcDateTimeToMillisec(Util.localeDateFormattoUTCDateTime(dateFormatString, widgetInputPad.currentValue));

                    // CRU requires mid day time to allow for correct date calculations
                    var midDayBirthdayInMillisec = birthDayInMillisec + (12 * 60 * 60 * 1000);

                    // need to apply reverse UTC Offset - HCU doesn't know what the timezone is and forces local time with 0 offset and call it UTC time which is wrong
                    var manualUTCOffsetInMillisec = (dsCfgLocal.currentUtcOffsetMinutes * 60 * 1000) * (-1);

                    var finalBirthDayInUTCMillisec = midDayBirthdayInMillisec + manualUTCOffsetInMillisec;
                    var utcDateTime = new Date(finalBirthDayInUTCMillisec);

                    // convert to ISO format before sending
                    egfrParamData.Value = utcDateTime.toISOString();
                }
                else if (egfrParamData.ValidDataType === "list")
                {
                    // Revert the translation and commit the selection
                    for (var i = 0; i < paramDataValidList.length; i++)
                    {
                        if (paramDataValidList[i] === widgetInputPad.currentValue)
                        {
                            egfrParamData.Value = egfrParamData.ValidList[i];
                            break;
                        }
                    }
                }
                else
                {
                    egfrParamData.Value = widgetInputPad.currentValue;
                }

                dsCru.slotPostUpdateExamFieldParameter(egfrParamData);

                // Invalidate the egfrParamData. This is to force-update paramDataValue
                egfrParamData = undefined;
                egfrParamData = Qt.binding(function() { return egfrItem.Value.Parameters[index]});
            }
        }

        rectEgfrSubItem.init();

        doneEdit();
    }
}
