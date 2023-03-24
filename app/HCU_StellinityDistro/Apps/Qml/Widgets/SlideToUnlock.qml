import QtQuick 2.12

Item {
    id: root
    visible: true
    height: 80
    property bool unlock: false
    property string unlockText: "T_SlideRightToConfirm"
    property real sliderWidth: width * 0.7

    function reset() {
        visible = true;
        unlock = false;
        sliderBase.reset();
    }

    Rectangle {
        id: sliderBase
        anchors.top: parent.top
        height: Math.max(parent.height, lockedText.contentHeight * 1.1)
        anchors.horizontalCenter: parent.horizontalCenter
        width: sliderWidth
        radius: buttonRadius
        color: colorMap.keypadButton

        Text {
            id: lockedText
            anchors.leftMargin: sliderBrick.width
            anchors.fill: parent
            text: translate(unlockText)
            wrapMode: Text.Wrap
            color: colorMap.text01
            font.family: fontRobotoBold.name
            font.pixelSize: titleFontPixelSize
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            opacity: sliderBrick.progress
        }

        Rectangle {
            id: sliderBrick
            visible: !root.unlock
            anchors.top: parent.top
            anchors.topMargin: gap
            anchors.bottom: parent.bottom
            anchors.bottomMargin: gap
            anchors.left: parent.left
            anchors.leftMargin: gap
            width: btnWidth / 2
            color: colorMap.actionButtonBackground
            radius: buttonRadius

            property int gap: 2
            property var originalLeftAnchor: parent.left
            property double progress: (sliderBase.width - sliderBrick.x) / sliderBase.width;

            onXChanged: {
                if (x >= sliderMouseArea.drag.maximumX) {
                    if (!unlockTimer.running) {
                        unlockTimer.start();
                    }
                } else {
                    unlockTimer.stop();
                }
            }

            // Timer used to ensure that flicking doesn't unlock
            Timer {
                id: unlockTimer
                interval: 100
                onTriggered: {
                    root.unlock = true;
                }
            }

            MouseArea {
                id: sliderMouseArea
                anchors.fill: parent
                drag.target: sliderBrick
                drag.axis: Drag.XAxis
                drag.minimumX: sliderBrick.gap
                drag.maximumX: sliderBase.width - sliderBrick.width - sliderBrick.gap

                onPressed: {
                    // release the brink when pressed
                    sliderBrick.anchors.left = undefined;
                    sliderBrick.anchors.leftMargin = undefined;
                }

                onReleased: {
                    if (!root.unlock) {
                        // re-anchor when released without unlocking
                        sliderBrick.reset()
                    }
                }
            }

            function reset() {
                anchors.left = originalLeftAnchor;
                anchors.leftMargin = gap;
            }
        }

        function reset() {
            sliderBrick.reset();
        }
    }
}
