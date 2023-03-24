import QtQuick 2.12
import "../"
import "../../Util.js" as Util

InputPadGeneric {
    property int minMs: 1
    property int maxMs: 900
    property string curDurationStr

    textDescription: Util.getMinimisedDurationStr(Util.millisecToDurationStr(minMs)) + " - " + Util.getMinimisedDurationStr(Util.millisecToDurationStr(maxMs));

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
                if (curDurationStr.length > 1)
                {
                    curDurationStr = curDurationStr.substring(0, curDurationStr.length - 1);
                }
                else
                {
                    curDurationStr = "";
                }
            }
            else
            {
                curDurationStr = "";
            }
        }
        else
        {
            if (modified)
            {
                if (curDurationStr.length < 4)
                {
                    curDurationStr += btnText;
                }
            }
            else
            {
                curDurationStr = btnText;
            }
        }

        modified = true;


        // Format the output text
        //var textValue = currentValue;
        currentValue = curDurationStr;
        if (currentValue === "")
        {
            currentValue = "--:--";
        }
        else
        {
            while (currentValue.length < 4)
            {
                currentValue = " " + currentValue;
            }

            var minutes = currentValue.substr(0, 2);
            var seconds = currentValue.substr(2, 2);
            currentValue = minutes + ":" + seconds;
        }

        textWidget.text = currentValue;

        //textWidget.text = currentValue;
        valueOk = checkValue();
        textWidget.color = valueOk ?  colorMap.actionButtonText : colorMap.errText;
        signalValueChanged(currentValue);
    }

    function checkValue()
    {
        var durationMs = Util.durationStrToMillisec(textWidget.text);

        if (durationMs < minMs)
        {
            return false;
        }
        else if (durationMs > maxMs)
        {
            return false;
        }
        return true;
    }
}
