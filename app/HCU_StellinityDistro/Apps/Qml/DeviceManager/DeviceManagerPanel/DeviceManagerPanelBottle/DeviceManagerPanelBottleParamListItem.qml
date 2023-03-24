import QtQuick 2.12
import "../../../Widgets"
import "../../../Util.js" as Util

GenericButton {
    property string labelText: "Label"
    property string valueText: "Value"
    property alias valueTextContainerObj: editContainer
    property alias valueTextObj: textValue
    property bool isMandatoryField: false
    property bool isReady: {
        if ( (widgetKeyboard.isOpenFor(textValue)) ||
             (widgetInputPad.isOpenFor(textValue)) )
        {
            return false;
        }

        if (isMandatoryField)
        {
            if ( (valueText == "") ||
                 (valueText == "--") )
            {
                return false;
            }
        }

        return true;
    }

    signal editStarted();

    width: parent.width
    height: rowHeight
    radius: 0

    Text {
        id: textLabel
        height: parent.height
        width: parent.width * 0.35
        verticalAlignment: Text.AlignVCenter
        font.family: fontRobotoLight.name
        font.pixelSize: rowHeight * 0.3
        minimumPixelSize: font.pixelSize * 0.24
        fontSizeMode: Text.Fit
        color: interactive ? colorMap.text01 : colorMap.text02
        text: labelText
        wrapMode: Text.Wrap

        MandatoryFieldMark {
            visible: isMandatoryField && (valueText === "--");
        }
    }

    Rectangle {
        id: editContainer
        color: "transparent"
        anchors.left: textLabel.right
        anchors.right: parent.right
        height: textValue.height

        Text {
            id: textValue
            width: {
                if ( (widgetKeyboard.isOpenFor(this)) ||
                     (widgetInputPad.isOpenFor(this)) )
                {
                    return parent.width * 0.96;
                }
                return parent.width;
            }

            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignRight
            height: rowHeight
            text: valueText
            color: interactive ? colorMap.text01 : colorMap.text02
            wrapMode: Text.Wrap
            font.family: fontRobotoBold.name
            font.pixelSize: rowHeight * 0.32
        }
    }

    onBtnClicked: {
        if ( (widgetKeyboard.isOpenFor(textValue)) ||
             (widgetInputPad.isOpenFor(textValue)) )
        {
            // Editing
            return;
        }

        if (widgetKeyboard.isOpen())
        {
            widgetKeyboard.close(true);
        }
        if (widgetInputPad.isOpen())
        {
            widgetInputPad.close(true);
        }
        editStarted();
    }

    function setDataBindings()
    {
        textValue.text = Qt.binding(function() { return valueText; });
        textValue.color = Qt.binding(function() { return interactive ? colorMap.text01 : colorMap.text02; });
    }
}
