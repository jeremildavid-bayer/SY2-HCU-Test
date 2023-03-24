import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Controls.Styles 1.4

Item {
    property string editMode: "SINGLE_LINE" // "SINGLE_LINE", "MULTI_LINE"
    property int prevMainWindowY: 0
    property int newMainWindowY: 0
    property var containerObject
    property var textObject
    property var startValue
    property var finalValue
    property var currentValue
    property int maxLen: 105

    signal signalClosed(bool modified)
    signal signalValueChanged(var val)

    id: keyboard
    width: window.width
    height: window.height

    Rectangle {
        id: textContainer
        visible: false
        x: parent.width * 0.1
        width: parent.width * 0.5
        y: parent.height * 0.3
        height: parent.height * 0.1
        clip: true

        /*TextArea {
            id: textEdit
            width: parent.width
            height: parent.height

            cursorDelegate: Rectangle {
                id: rectCursor
                color: "black"
                width: 4
                height: textEdit.height - 8
                anchors.verticalCenter: parent.verticalCenter

                SequentialAnimation {
                    id: animationCursorBlink
                    loops: Animation.Infinite
                    NumberAnimation { target: rectCursor; properties: "opacity"; to: 1; duration: 500 }
                    NumberAnimation { target: rectCursor; properties: "opacity"; to: 0; duration: 500 }
                }

                Component.onCompleted: {
                    textContainer.onVisibleChanged.connect(reload);
                    reload();
                }

                Component.onDestruction: {
                    textContainer.onVisibleChanged.disconnect(reload);
                }

                function reload()
                {
                    if (textContainer.visible)
                    {
                        animationCursorBlink.start();
                    }
                    else
                    {
                        animationCursorBlink.stop();
                    }
                }
            }
        }*/

        Flickable {
            id: flickTextArea
            width: parent.width
            height: parent.height
            //flickableDirection: (editMode == "SINGLE_LINE") ? Flickable.HorizontalFlick : Flickable.HorizontalAndVerticalFlick
            flickableDirection: Flickable.HorizontalFlick

            TextArea.flickable: TextArea {
                id: textEdit
            }
        }
    }



    KeyboardInner {
        id: keyboardInner
    }

    Component.onCompleted: {
        keyboardInner.signalActivated.connect(slotKeyboardActivated);
        keyboardInner.signalDeactivated.connect(slotKeyboardDeactivated);
    }

    Component.onDestruction: {
        keyboardInner.signalActivated.disconnect(slotKeyboardActivated);
        keyboardInner.signalDeactivated.connect(slotKeyboardDeactivated);
    }



    function open(containerObj, textObj)
    {
        // Check if main window needs to be shifted
        //textContainerParent = containerObj.parent;

        containerObject = containerObj;
        textObject = textObj;

        var containerPos = keyboard.mapFromItem(containerObj, 0, 0);
        //console.log("pos=" + containerPos.x + ", pos.y=" + containerPos.y);
        if ((containerPos.y + containerObj.height)  > keyboardInner.height)
        {
            // main window needs to be shifted
            prevMainWindowY = window.y;
            newMainWindowY = window.y - (containerPos.y - keyboardInner.height + (containerObj.height * 1.5));
            console.log("newMainWindowY=" + newMainWindowY);
        }
        else
        {
            prevMainWindowY = window.y;
            newMainWindowY = window.y;
        }

        // Display keyboard widget
        keyboardInner.state = "ACTIVE";

        // Initialise data
        startValue = textObj.text;
        finalValue = textObj.text;
        currentValue = textObj.text;
        //animationCursorBlink.start();
    }

    function close(modified)
    {
        if (modified)
        {
            textObject.text = currentValue;
            console.log("textObject.text=" + textObject.text);
        }
        signalClosed(modified);
        textContainer.visible = false;
        keyboardInner.state = "INACTIVE";
    }

    function slotKeyboardActivated()
    {
        //window.y -= mainWindowShifted;

        // Prepare keyboard edit widget
        var containerPos = keyboard.mapFromItem(containerObject, 0, 0);
        textContainer.x = containerPos.x;
        textContainer.y = containerPos.y;
        textContainer.color = containerObject.color;
        textContainer.width = containerObject.width;
        textContainer.height = containerObject.height;
        textContainer.radius = containerObject.radius;
        textContainer.opacity = containerObject.opacity;
        textContainer.visible = true;

        textEdit.text = textObject.text;
        textEdit.x = textObject.x;
        textEdit.y = textObject.y;
        textEdit.width = textObject.width;
        textEdit.height = textObject.height;
        textEdit.font.pixelSize = textObject.font.pixelSize;
        textEdit.horizontalAlignment = textObject.horizontalAlignment;
        textEdit.verticalAlignment = textObject.verticalAlignment;

        textEdit.selectAll();
        textEdit.cursorVisible = true;
    }

    function slotKeyboardDeactivated()
    {
        //window.y += mainWindowShifted;

    }

    function slotKeyEntered(textKey)
    {
        //console.log("Key " + textKey + " entered, selectedText=" + textEdit.selectedText);
        if (textEdit.readOnly)
        {
            return;
        }

        var prevVal = textEdit.text;

        if (textKey === "\ue965")
        {
            // delete key
            if (textEdit.selectedText != "")
            {
                textEdit.remove(textEdit.selectionStart, textEdit.selectionEnd);
            }
            else
            {
                textEdit.remove(textEdit.cursorPosition - 1, textEdit.cursorPosition);
            }
        }
        else if (textKey === "<-")
        {
            // newline key
            textEdit.remove(textEdit.selectionStart, textEdit.selectionEnd);
            textEdit.insert(textEdit.cursorPosition, "\n");
        }
        else if (textKey === "\ue966")
        {
            keyboardInner.mode = "CAP";
        }
        else if (textKey === "\ue911")
        {
            keyboardInner.mode = "NORMAL";
        }
        else if (textKey === "1/2")
        {
            keyboardInner.mode = "NUM2";
        }
        else if (textKey === "2/2")
        {
            keyboardInner.mode = "NUM1";
        }
        else if (textKey === "?123")
        {
            keyboardInner.mode = "NUM1";
        }
        else if (textKey === "ABC")
        {
            keyboardInner.mode = "NORMAL";
        }
        else if (textKey === "\ue909")
        {
            console.log("textEdit.selectedText=" + textEdit.selectedText);
            if (textEdit.selectedText == "")
            {
                textEdit.cursorPosition--;
            }
            else
            {
                textEdit.cursorPosition = textEdit.selectionStart;
                textEdit.deselect();
            }
        }
        else if (textKey === "\ue908")
        {
            if (textEdit.selectedText == "")
            {
                textEdit.cursorPosition++;
            }
            else
            {
                textEdit.cursorPosition = textEdit.selectionEnd;
                textEdit.deselect();
            }
        }
        else if (textKey === " ")
        {
            textEdit.remove(textEdit.selectionStart, textEdit.selectionEnd);
            textEdit.insert(textEdit.cursorPosition, " ");
        }
        else if (textKey === "\ue957\n\ue906")
        {
            // cancel key
            close(false);
        }
        else if (textKey === "OK")
        {
            close(true);
        }
        else
        {
            textEdit.remove(textEdit.selectionStart, textEdit.selectionEnd);
            textEdit.insert(textEdit.cursorPosition, textKey);
        }

        if ( (maxLen > 0) && (textEdit.text.length > maxLen) )
        {
            textEdit.remove(textEdit.text.length - (textEdit.text.length - maxLen), textEdit.text.length);
        }

        if (textEdit.text != prevVal)
        {
            signalValueChanged(textEdit.text);
            currentValue = textEdit.text;
        }
    }
}
