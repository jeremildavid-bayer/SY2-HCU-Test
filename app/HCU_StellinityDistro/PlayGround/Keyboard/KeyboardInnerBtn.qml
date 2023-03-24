import QtQuick 2.5

Rectangle {
    property var keys: []
    property string backgroundColor: "#f0f0f0"
    property string fontColor: "#000000"
    property int btnLongPressStartMs: 800
    property int btnLongPressProcessMs: 100
    property int btnTextFontPixelSize: height * 0.5

    id: root
    color: backgroundColor
    width: buttonWidth
    height: buttonHeight
    Text {
        id: textKey
        color: fontColor
        anchors.fill: parent
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        text: getBtnText()
        font.pixelSize: btnTextFontPixelSize
    }

    MouseArea {
        anchors.fill: parent
        onPressed: {
            root.opacity = 0.5;
            pressedTimer.interval = btnLongPressStartMs;
            pressedTimer.start();
        }

        onReleased: {
            root.opacity = 1;
            pressedTimer.stop();
        }

        onClicked: {
            keyboard.slotKeyEntered(textKey.text);
        }
    }

    Timer {
        id: pressedTimer
        interval: btnLongPressStartMs
        onTriggered: {
            keyboard.slotKeyEntered(textKey.text);
            pressedTimer.interval = btnLongPressProcessMs;
            pressedTimer.start();
        }
    }

    function getBtnText()
    {
        if (keyboardInner.mode == "CAP")
        {
            return keys[1];
        }
        else if (keyboardInner.mode == "NUM1")
        {
            return keys[2];
        }
        else if (keyboardInner.mode == "NUM2")
        {
            return keys[3];
        }
        else
        {
            return keys[0];
        }
    }
}
