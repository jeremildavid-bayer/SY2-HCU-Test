import QtQuick 2.12

Rectangle {
    property alias content: mainRect.children
    property alias pressedCoverContent: pressedCover.children
    property string disabledColor: Qt.tint(root.color, colorMap.buttonDisabled)
    property string pressedColor: Qt.tint(root.color, colorMap.buttonShadow)
    property var pressedSoundCallback: function() { soundPlayer.playPressGood(); }
    property bool clickedEventNeedsEmit: false
    property bool interactive: true

    signal btnClicked();
    signal btnPressed();
    signal btnReleased();
    signal btnExit();
    signal btnPressedLong(var mouse);
    signal btnPositionChanged(var mouse);

    id: root
    color: "transparent"
    radius: buttonRadius
    clip: true

    Rectangle {
        id: disabledCover
        visible: !root.enabled
        anchors.fill: parent
        radius: parent.radius
        color: disabledColor
        opacity: 0.8
        z: root.z + 1
    }

    Rectangle {
        id: pressedCover
        anchors.fill: parent
        radius: parent.radius
        visible: false
        color: pressedColor
        opacity: 0.6
        z: root.z + 1
    }

    MouseArea {
        anchors.fill: parent
        enabled: interactive

        onClicked: {
            pressedCover.visible = false;
            if (clickedEventNeedsEmit)
            {
                btnClicked();
                clickedEventNeedsEmit = false;
            }
        }

        onPressedChanged: {
            if (pressed)
            {
                clickedEventNeedsEmit = true;
                pressedSoundCallback();
                pressedCover.visible = true;
                btnPressed();
            }
            else
            {
                if (clickedEventNeedsEmit)
                {
                    btnClicked();
                    clickedEventNeedsEmit = false;
                }

                pressedCover.visible = false;
                btnReleased();
            }
        }

        onExited: {
            clickedEventNeedsEmit = false;
            pressedCover.visible = false;
            btnExit();
        }

        onPressAndHold: (mouse) => {
            btnPressedLong(mouse);
        }

        onPositionChanged: (mouse) => {
            btnPositionChanged(mouse);
        }
    }

    Rectangle {
        id: mainRect
        anchors.fill: parent
        radius: parent.radius
        color: "transparent"
    }

    onVisibleChanged: {
        if (visible)
        {
            // Sometimes pressedCover still shown when visible status changed
            pressedCover.visible = false;
        }
    }

    function reset()
    {
        clickedEventNeedsEmit = false;
        pressedCover.visible = false;
    }
}


