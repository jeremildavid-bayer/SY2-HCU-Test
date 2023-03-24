import QtQuick 2.12
import "../"

InputPadGeneric {
    property int minLength: 0
    property int maxLength: 10

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
                    currentValue = "";
                }
            }
            else
            {
                currentValue = "";
            }
        }
        else
        {
            if (modified)
            {
                if (currentValue.length < maxLength)
                {
                    currentValue += btnText;
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
        var valStr = currentValue.toString();
        if (valStr.length < minLength)
        {
            return false;
        }
        else if (valStr.length > maxLength)
        {
            return false;
        }
        return true;
    }
}
