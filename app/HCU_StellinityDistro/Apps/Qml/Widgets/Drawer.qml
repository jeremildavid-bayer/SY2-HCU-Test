import QtQuick 2.12

Item {
    property string type: "RIGHT" // "LEFT", "RIGHT"
    property bool edgeDragEnabled: true
    property int animationMs: 250
    property int handleRectWidth: dsCfgLocal.screenH * 0.032
    property int xScreenEdge: parent.mapFromItem(appMainView, 0, 0).x + appMainView.width
    property int xOpen: getOpenX()
    property int xClosed: getClosedX()
    property alias content: mainRect.children

    property int xLimitLeft: getLimitLeft()
    property int xLimitRight: getLimitRight()

    property bool handlePressed

    signal signalOpen()
    signal signalClosed()

    property bool interactive: true

    id: root
    state: "CLOSED"
    x: xClosed
    width: 0
    clip: true

    states: [
        State { name: "OPEN" },
        State { name: "CLOSED" }
    ]

    transitions: [
        Transition {
            to: "OPEN"
            SequentialAnimation {
                ParallelAnimation {
                    NumberAnimation { target: root; properties: 'x'; to: xOpen; duration: animationMs; }
                }

                ScriptAction { script: {
                        root.x = xOpen;
                        signalOpen();
                    }
                }
            }

            onRunningChanged: {
                if (root.state === "OPEN" && running) {
                    mainRect.visible = true;
                }
            }
        },
        Transition {
            to: "CLOSED"
            SequentialAnimation {
                ParallelAnimation {
                    NumberAnimation { target: root; properties: 'x'; to: xClosed; duration: animationMs; }
                }
                ScriptAction { script: {
                        root.x = xClosed;
                        signalClosed();
                    }
                }
            }

            onRunningChanged: {
                if (root.state === "CLOSED" && !running) {
                    mainRect.visible = false;
                }
            }
        }
    ]

    MouseArea {
        property int yStart
        property int xLast
        property int flickOffset: 20

        id: mouseArea
        anchors.fill: parent
        drag.target: root
        enabled: interactive && ((edgeDragEnabled) || (root.state === "OPEN"))

        Timer {
            id: flickResetTimer
            interval: 500
            onTriggered: {
                mouseArea.xLast = root.x;
            }
        }

        onPressed: {
            soundPlayer.playPressGood();
            handlePressed = true;
            mainRect.visible = true;
            yStart = root.y;
            xLast = root.x;
        }

        onReleased: {
            handlePressed = false;
            var diffX = root.x - xLast;

            if ( (type == "LEFT") && (diffX > flickOffset) )
            {
                root.state = "UNKNOWN";
                root.state = "OPEN";
            }
            else if ( (type == "LEFT") && (diffX < -flickOffset) )
            {
                root.state = "UNKNOWN";
                root.state = "CLOSED";
            }
            else if ( (type == "RIGHT") && (diffX > flickOffset) )
            {
                root.state = "UNKNOWN";
                root.state = "CLOSED";
            }
            else if ( (type == "RIGHT") && (diffX < -flickOffset) )
            {
                root.state = "UNKNOWN";
                root.state = "OPEN";
            }
            else if ( (type == "LEFT") && (root.x >= xLimitRight) )
            {
                root.state = "UNKNOWN";
                root.state = "OPEN";
            }
            else if ( (type == "LEFT") && (root.x <= xLimitLeft) )
            {
                root.state = "UNKNOWN";
                root.state = "CLOSED";
            }
            else if ( (type == "RIGHT") && (root.x >= xLimitRight) )
            {
                root.state = "UNKNOWN";
                root.state = "CLOSED";
            }
            else if ( (type == "RIGHT") && (root.x <= xLimitLeft) )
            {
                root.state = "UNKNOWN";
                root.state = "OPEN";
            }
            else
            {
                var prevState = root.state;
                root.state = "UNKNOWN";
                root.state = prevState;
            }
        }

        onExited: {
            handlePressed = false;
        }

        onEntered: {
            handlePressed = true;
        }

        onClicked: {
            if (root.state == "OPEN")
            {
                root.state = "CLOSED";
            }
            else
            {
                root.state = "OPEN";
            }
        }

        onPositionChanged: {
            root.y = yStart;

            if (root.x < xLimitLeft)
            {
                root.x = xLimitLeft;
            }
            else if (root.x > xLimitRight)
            {
                root.x = xLimitRight;
            }

            var diffX = root.x - xLast;

            if ( (diffX > flickOffset) || (diffX < -flickOffset) )
            {
                if (!flickResetTimer.running)
                {
                    flickResetTimer.start();
                }
            }
            else
            {
                xLast = root.x;
            }

            if ( (type == "LEFT") && (diffX > 0) )
            {
                mainRect.visible = true;
            }
            else if ( (type == "RIGHT") && (diffX < 0) )
            {
                mainRect.visible = true;
            }
        }
    }

    Rectangle {
        id: mainRect
        x: getMainRectX()
        width: root.width - handleRectWidth
        height: root.height
        color: colorMap.subPanelBackground
        clip: true
        visible: (root.state === "OPEN")
    }

    Item {
        id: handleRect
        x: getHandleX()
        width: handleRectWidth
        height: root.height
        visible: ((edgeDragEnabled) || (root.state === "OPEN")) && (handleRectWidth !== 0)

        Rectangle {
            id: handle
            x: (type == "LEFT") ? handleBar.width - radius : 0
            width: (parent.width * 0.5) + radius
            height: parent.height * 0.12
            anchors.verticalCenter: parent.verticalCenter
            color: handlePressed ? colorMap.drawerHandleBackground : mainRect.color
            radius: 8
        }

        Rectangle {
            id: handleBar
            width: parent.width * 0.5
            x: (type == "LEFT") ? 0 : width
            height: parent.height
            color: handle.color
        }

        Item {
            id: stateIconFrame
            x: (type == "LEFT") ? 0 : handle.x
            width: parent.width
            height: handle.height
            rotation: getHandleAngle()
            anchors.verticalCenter: parent.verticalCenter

            Text {
                anchors.fill: parent
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                color: colorMap.text01
                font.pixelSize: height * 0.32
                font.family: fontIcon.name
                text: "\ue908"
            }
        }
    }

    function open()
    {
        state = "OPEN";
    }

    function close()
    {
        state = "CLOSED";
    }

    function isOpened()
    {
        return state === "OPEN";
    }

    function getClosedX()
    {
        if (type == "LEFT")
        {
            return -width + handleRectWidth;
        }
        else if (type == "RIGHT")
        {
            return xScreenEdge - handleRectWidth;
        }

        return 0;
    }

    function getOpenX()
    {
        if (type == "LEFT")
        {
            return 0;
        }
        else if (type == "RIGHT")
        {
            return xScreenEdge - width;
        }

        return 0;
    }

    function getLimitLeft()
    {
        if (type == "LEFT")
        {
            return xClosed;
        }
        else if (type == "RIGHT")
        {
            return xOpen;
        }
    }

    function getLimitRight()
    {
        if (type == "LEFT")
        {
            return xOpen;
        }
        else if (type == "RIGHT")
        {
            return xClosed;
        }
    }

    function getMainRectX()
    {
        if (type == "LEFT")
        {
            return 0;
        }
        else if (type == "RIGHT")
        {
            return handleRectWidth;
        }
    }

    function getHandleX()
    {
        if (type == "LEFT")
        {
            return width - handleRect.width;
        }
        else if (type == "RIGHT")
        {
            return 0;
        }
    }

    function getHandleAngle()
    {
        var xTotal = xLimitRight - xLimitLeft;
        var m = 180 / xTotal;
        var xMoved = root.x - xLimitLeft;
        var angle = xMoved * m;
        return angle;
    }
}
