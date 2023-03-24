import QtQuick 2.12
import "../"

InputPadGeneric {
    property double minValue: 1
    property double maxValue: 1000
    property string unitStr: ""
    property int floatPointLimit: 0
    property int decimalPointLimit: 2
    property string decimalPoint: localeDecimalPoint()

    textDescription: localeToFloatStr(minValue, floatPointLimit) + " - " + localeToFloatStr(maxValue, floatPointLimit) + " " + unitStr

    content: [
        InputPadOneToNineButtons {
            id: keys
        },

        Item {
            anchors.top: keys.bottom
            width: parent.width
            height: btnWidth

            GenericIconButton {
                id: btnDot
                width: btnWidth
                height: btnWidth
                radius: 0
                color: colorMap.keypadButton
                iconColor: colorMap.text01
                iconText: decimalPoint
                iconFontFamily: fontRobotoLight.name
                pressedSoundCallback: keyPressedSoundCallback
                onBtnClicked: {
                    slotBtnPressed(decimalPoint);
                }
            }

            GenericIconButton {
                x: btnWidth + btnSpacing
                width: btnWidth
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

    function slotBtnPressed(btnText)
    {
        if (btnText === "<")
        {
            if (modified)
            {
                if (currentValue.length > 1)
                {
                    currentValue = currentValue.substring(0, currentValue.length - 1);
                }
                else
                {
                    currentValue = "0";
                }
            }
            else
            {
                currentValue = "0";
            }
        }
        else if (btnText === decimalPoint)
        {
            if (modified)
            {
                if (currentValue.indexOf(decimalPoint) === -1)
                {
                    currentValue += decimalPoint;
                }
            }
            else
            {
                currentValue = "0" + decimalPoint;
            }
        }
        else
        {
            if (modified)
            {
                if (currentValue === "0")
                {
                    currentValue = btnText;
                }
                else if (currentValue.indexOf(decimalPoint) >= 0)
                {
                    var curFloatPoints = currentValue.length - currentValue.indexOf(decimalPoint);
                    if (curFloatPoints <= floatPointLimit)
                    {
                        currentValue += btnText;
                    }
                }
                else
                {
                    if (currentValue.length < decimalPointLimit)
                    {
                        currentValue += btnText;
                    }
                }
            }
            else
            {
                currentValue = btnText;
            }
        }

        modified = true;

        textWidget.text = currentValue;
        valueOk = checkValue();
        textWidget.color = valueOk ?  colorMap.actionButtonText : colorMap.errText;

        signalValueChanged(currentValue);
    }

    function checkValue()
    {
        // it has to be a number
        if (!isNaN(parseFloat(currentValue)))
        {
            var valFloat = localeFromFloatStr(currentValue);
            if (valFloat < minValue)
            {
                return false;
            }
            else if (valFloat > maxValue)
            {
                return false;
            }
            return true;
        }
        return false;
    }
}
