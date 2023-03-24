import QtQuick 2.12

Item {
    property bool activated: false
    property string activatedColor: root.enabled ? "lightgreen" : Qt.tint("lightgreen", colorMap.buttonDisabled)
    property string deactivatedColor: root.enabled ? "lightgray" : Qt.tint("lightgray", colorMap.buttonDisabled)
    property string handleColor: root.enabled ? "white" : Qt.tint("white", colorMap.buttonDisabled)
    property int borderWidth: 2
    property int handlePosXOff: borderWidth
    property int handlePosXOn: Math.max(width - borderWidth - handle.width, handlePosXOff)
    property int animationMs: 100
    property bool isReadOnly: false

    signal toggled(bool activated)

    id: root
    state: "UNKNOWN"

    Rectangle {
        id: backgroundOff
        height: parent.height
        width: parent.width
        radius: height / 2
        color: deactivatedColor;
    }

    Rectangle {
        id: backgroundOn
        height: parent.height
        width: parent.width
        radius: height / 2
        color: activatedColor;
    }

    Rectangle {
        id: handle
        anchors.verticalCenter: parent.verticalCenter
        height: parent.height - (borderWidth * 2)
        width: height
        radius: height / 2
        color: handleColor

        onXChanged: {
            if (root.state == "UNKNOWN")
            {
                return;
            }

            //logDebug("handle.onXChanged, x=" + x + ", state=" + root.state + ", handlePosXOff=" + handlePosXOff + ", handlePosXOn=" + handlePosXOn);
            if (x < handlePosXOff)
            {
                x = handlePosXOff;
            }
            else if (x > handlePosXOn)
            {
                x = handlePosXOn;
            }

            backgroundOn.opacity = (handle.x / (handlePosXOn - handlePosXOff));
        }
    }

    states: [
        State { name: "ON" },
        State { name: "OFF" }
    ]

    transitions: [
        Transition {
            to: "OFF"
            SequentialAnimation {
                ParallelAnimation {
                    NumberAnimation { target: handle; properties: 'x'; to: handlePosXOff; duration: animationMs }
                }

                ScriptAction {
                    script: {
                        handle.x = handlePosXOff;
                        activated = false;
                        toggled(activated)
                    }
                }
            }
        },

        Transition {
            to: "ON"
            SequentialAnimation {
                ParallelAnimation {
                    NumberAnimation { target: handle; properties: 'x'; to: handlePosXOn; duration: animationMs }
                }

                ScriptAction {
                    script: {
                        handle.x = handlePosXOn;
                        activated = true;
                        toggled(activated);
                    }
                }
            }
        }
    ]

    MouseArea {
        anchors.fill: parent
        drag.target: handle
        enabled: !isReadOnly

        onClicked: {
            handle.opacity = 1;
            root.state = (root.state === "ON") ? "OFF" : "ON";
        }

        onPressed: {
            soundPlayer.playPressGood();
            handle.opacity = 0.5;
        }

        onReleased: {
            handle.opacity = 1;
            if (handle.x == handlePosXOn)
            {
                if (!activated)
                {
                    activated = true;
                    toggled(activated);
                }
            }
            else if (handle.x == handlePosXOff)
            {
                if (activated)
                {
                    activated = false;
                    toggled(activated);
                }
            }
            else
            {
                if (handle.x < ((handlePosXOn - handlePosXOff) / 2))
                {
                    root.state = "UNKNOWN";
                    root.state = "OFF";
                }
                else
                {
                    root.state = "UNKNOWN";
                    root.state = "ON";
                }
            }
        }
    }

    onHandlePosXOnChanged: {
        reload();
    }

    onActivatedChanged: {
        root.state = (activated) ? "ON" : "OFF";
    }

    Component.onCompleted: {
        reload();
    }

    function toggle()
    {
        activated = !activated;
    }

    function reload()
    {
        handle.x = (activated) ? handlePosXOn : handlePosXOff;
        backgroundOn.opacity = (handle.x / (handlePosXOn - handlePosXOff));
    }
}
