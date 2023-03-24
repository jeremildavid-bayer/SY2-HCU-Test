import QtQuick 2.12
import "../"

InputPadGeneric {
    property double minValue: 1
    property double maxValue: 1000
    property string unitStr: "ml"
    property int decimalPointLimit: 2

    textDescription: minValue + " - " + maxValue + " " + unitStr

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
        else
        {
            if (modified)
            {
                if (currentValue === "0")
                {
                    currentValue = btnText;
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
        //finalValue = parseInt(currentValue);
        //logDebug("IntegerPad: currentValue=" + currentValue + ", finalValue=" + finalValue);

        textWidget.text = currentValue;
        valueOk = checkValue();
        textWidget.color = valueOk ?  colorMap.actionButtonText : colorMap.errText;

        signalValueChanged(currentValue);
    }

    function checkValue()
    {
        var valInt = parseInt(currentValue);
        if (valInt < minValue)
        {
            return false;
        }
        else if (valInt > maxValue)
        {
            return false;
        }
        return true;
    }
}
