import QtQuick 2.12
import "../../Util.js" as Util

QtObject {
    property var examAdvanceInfo: dsExam.examAdvanceInfo
    property string dateFormat: dsCfgGlobal.dateFormat
    property string timeFormat: dsCfgGlobal.timeFormat
    property string dateFormatLocale: translate("T_ConfigItem_Settings_General_DateFormat_" + dateFormat)
    property string timeFormatLocale: translate("T_ConfigItem_Settings_General_TimeFormat_" + timeFormat)
    property int currentUtcOffsetMinutes: dsCfgLocal.currentUtcOffsetMinutes;

    function getDicomFieldDateTimeValue(dicomField)
    {
        // invalid value. display --
        // Below can be used but they are either inefficient or not flexible. Choosing to only compare the date (01-01-0001) and ignore timestamp.
        // (new Date(dicomField.Value).toString()) === (new Date(Util.DATE_TIME_MIN_VALUE).toString()))
        // (dicomField.Value === Util.DATE_TIME_MIN_VALUE)
        if ( (dicomField.Value === "") || (dicomField.Value.split("T")[0] === Util.DATE_TIME_MIN_VALUE.split("T")[0]))
        {
            return "--";
        }

        if (dicomField.DicomValueType === "TM") // time only
        {
            return Util.utcDateTimeToDateTimeStr("", timeFormatLocale, currentUtcOffsetMinutes, dicomField.Value);
        }
        else if (dicomField.DicomValueType === "DA") // date only
        {
            return Util.utcDateTimeToDateTimeStr(dateFormatLocale, "", currentUtcOffsetMinutes, dicomField.Value);
        }
        else
        {
            return Util.utcDateTimeToDateTimeStr(dateFormatLocale, timeFormatLocale, currentUtcOffsetMinutes, dicomField.Value);
        }
    }

    function getDicomFieldValue(dicomField)
    {
        if (dicomField === undefined)
        {
            return "";
        }
        else if (dicomField.Value === "")
        {
            return "--"
        }
        else if (dicomField.TranslateValue)
        {
            return translate("T_DicomTag_" + dicomField.Name + "_" + dicomField.Value);
        }
        else if (dicomField.ValueType === "datetime")
        {
            return getDicomFieldDateTimeValue(dicomField);
        }
        else if (dicomField.ValueType === "double")
        {
            var decimalDigits = Util.getDecimalDigits(dicomField.Value.toString());
            return localeToFloatStr(dicomField.Value, decimalDigits).toString();
        }

        return dicomField.Value;
    }

    function getDicomTagsData(type)
    {
        if ( (examAdvanceInfo !== undefined) &&
                (examAdvanceInfo.WorklistDetails !== undefined) )
        {
            return (type === "All") ? examAdvanceInfo.WorklistDetails.All : examAdvanceInfo.WorklistDetails.Panel;
        }
        return [];
    }
}



