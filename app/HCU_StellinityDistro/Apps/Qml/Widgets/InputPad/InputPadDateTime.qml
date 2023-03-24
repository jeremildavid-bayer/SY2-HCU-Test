import QtQuick 2.12
import "../../Util.js" as Util
import "../"

InputPadGeneric {
    property string valueFormat: "yyyy/MM/dd hh:mm"
    property string dateTimeStr
    property int maxLength
    property var minDateInclusive
    property var maxDateInclusive

    textDescription: getTextDescription()

    content: [
        InputPadOneToNineButtons {
            id: keys
        },

        Item {
            anchors.top: keys.bottom
            width: parent.width
            height: btnWidth

            GenericIconButton {
                width: (btnWidth * 2) + btnSpacing
                height: btnWidth
                radius: 0
                color: colorMap.keypadButton
                iconColor: colorMap.text01
                iconText: "0"
                iconFontFamily: fontRobotoLight.name
                pressedSoundCallback: keyPressedSoundCallback
                onBtnClicked: {
                    slotBtnPressed("0");
                }
            }

            GenericIconButton {
                anchors.right: parent.right
                width: btnWidth
                height: btnWidth
                radius: 0
                color: colorMap.keypadButton
                iconColor: colorMap.text01
                iconText: "\ue965"
                iconFontPixelSize: height * 0.3
                iconFontFamily: fontIcon.name
                pressedSoundCallback: keyPressedSoundCallback
                onBtnClicked: {
                    slotBtnPressed("<");
                }
            }
        }
    ]

    function getTextDescription()
    {
        if ((minDateInclusive === undefined) || (maxDateInclusive === undefined))
        {
            return valueFormat;
        }
        var minString = Qt.formatDateTime(minDateInclusive, valueFormat);
        var maxString = Qt.formatDateTime(maxDateInclusive, valueFormat);

        return valueFormat + "\n" + "(" + minString + " - " + maxString + ")";
    }

    function slotBtnPressed(btnText)
    {
        if (btnText === "<")
        {
            if (modified)
            {
                if (dateTimeStr.length > 1)
                {
                    dateTimeStr = dateTimeStr.substring(0, dateTimeStr.length - 1);
                }
                else
                {
                    dateTimeStr = "";
                }
            }
            else
            {
                dateTimeStr = "";
            }
        }
        else
        {
            if (modified)
            {
                if (dateTimeStr.length < maxLength)
                {
                    dateTimeStr += btnText;
                }
            }
            else
            {
                dateTimeStr = btnText;
            }
        }

        modified = true;

        currentValue = convertToFormatValue(dateTimeStr);
        textWidget.text = currentValue;
        valueOk = checkValue();
        textWidget.color = valueOk ?  colorMap.actionButtonText : colorMap.errText;

        signalValueChanged(currentValue);
    }

    function convertToFormatValue(currentValueIn)
    {
        var formatValueStr = currentValueIn;

        for (var i = 0; i < valueFormat.length; i++)
        {
            if (formatValueStr.length <= i)
            {
                // complete remaining string
                formatValueStr += valueFormat.substring(i, valueFormat.length);
                break;
            }

            if ( (valueFormat[i] === "/") ||
                 (valueFormat[i] === ".") ||
                 (valueFormat[i] === ":") ||
                 (valueFormat[i] === " ") )
            {
                // Add format character
                formatValueStr = formatValueStr.substring(0, i) + valueFormat[i] + formatValueStr.substring(i, formatValueStr.length);
            }
        }

        if (formatValueStr.length > valueFormat.length)
        {
            formatValueStr = formatValueStr.substring(0, valueFormat.length);
        }
        return formatValueStr;
    }

    function checkValue()
    {
        var year = 0, month = 0, day = 0, hour = 0, min = 0;
        var dateTime;

        dateTime = Util.localeDateFormattoUTCDateTime(valueFormat, currentValue);

        var newDateTimeStr = Qt.formatDateTime(dateTime, valueFormat);

        // is it valid string
        if (currentValue !== newDateTimeStr)
        {
            return false;
        }

        // check if it is within range
        if ((minDateInclusive !== undefined) && (maxDateInclusive !== undefined))
        {
            if ((dateTime < minDateInclusive) || (dateTime > maxDateInclusive))
            {
                return false;
            }
        }

        if (currentValue === valueFormat)
        {
            return true;
        }

        return true;
    }

    function init(dateTimeFormat)
    {
        valueFormat = dateTimeFormat;
        var valueFormatMinimised = dateTimeFormat.replace(/ /g, "");
        valueFormatMinimised = valueFormatMinimised.replace(/\//g, "");
        valueFormatMinimised = valueFormatMinimised.replace(/:/g, "");
        valueFormatMinimised = valueFormatMinimised.replace(/-/g, "");
        valueFormatMinimised = valueFormatMinimised.replace(/\./g, "");
        maxLength = valueFormatMinimised.length;
    }
}
