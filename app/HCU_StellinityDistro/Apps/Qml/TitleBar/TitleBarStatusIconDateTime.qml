import QtQuick 2.12
import "../Widgets"
import "../Util.js" as Util

Item {
    property string cultureCode: dsCfgGlobal.cultureCode
    property string dateFormat: dsCfgGlobal.dateFormat
    property string timeFormat: dsCfgGlobal.timeFormat
    property string dateFormatLocale: translate("T_ConfigItem_Settings_General_DateFormat_" + dateFormat)
    property string timeFormatLocale: translate("T_ConfigItem_Settings_General_TimeFormat_" + timeFormat)

    Text {
        id: textDate
        width: parent.width
        height: parent.height * 0.45
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignBottom
        font.pixelSize: height * 0.6
        font.family: fontRobotoLight.name
        color: colorMap.text01
        text: "XXXX/XX/XX"
    }

    Text {
        id: textTime
        y: parent.height * 0.5
        width: parent.width
        height: parent.height * 0.5
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignTop
        font.pixelSize: height * 0.68
        minimumPixelSize: font.pixelSize * 0.7
        font.family: fontRobotoBold.name
        fontSizeMode: Text.Fit
        color: colorMap.text01
        text: "XX:XX:XX"
    }

    onCultureCodeChanged: {
        reloadDateTime();
    }

    onDateFormatChanged: {
        reloadDateTime();
    }

    onTimeFormatChanged: {
        reloadDateTime();
    }

    Component.onCompleted: {
        appMain.signalUpdateDateTime.connect(reloadDateTime);
    }

    Component.onDestruction: {
        appMain.signalUpdateDateTime.disconnect(reloadDateTime);
    }

    function reloadDateTime()
    {
        var dateTimeStr = Util.getCurrentDateTimeStr(dateFormatLocale, timeFormatLocale, currentUtcOffsetMinutes);
        var dateTimeStrList = dateTimeStr.split(" ");
        if (dateTimeStrList.length > 0)
        {
            textDate.text = dateTimeStrList[0];
            textTime.text = "";
            for (var listIdx = 1; listIdx < dateTimeStrList.length; listIdx++)
            {
                if (listIdx > 1)
                {
                    textTime.text += " ";
                }
                textTime.text += dateTimeStrList[listIdx];
            }
        }
    }

}
