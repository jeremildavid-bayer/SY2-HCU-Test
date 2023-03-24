import QtQuick 2.12

Item {
    property var labelData: []
    property int labelItemWidth: width * 0.2
    property int handleWidth: width * 0.1
    property int valueIndex: 0
    property int labelSpacing: 1
    property string mainColor: colorMap.text01
    property string handleColor: colorMap.grid
    signal signalValueChanged(int newValue)
    signal signalValueIndexChanged(int newValueIndex)

    id: root

    MouseArea {
        // Prevent background touch
        anchors.fill: parent
    }

    Canvas {
        // Draw Background Line
        y: rectSlider.y + (rectSlider.height / 2) - (height / 2)
        width: parent.width
        height: parent.height * 0.15

        onPaint: {
            var ctx = getContext("2d");
            ctx.save();
            ctx.clearRect(0, 0, width, height);
            ctx.strokeStyle = mainColor;
            ctx.lineWidth = 1;
            ctx.fillStyle = mainColor;
            ctx.lineJoin = "round";
            ctx.lineCap = "round";
            ctx.beginPath();
            ctx.moveTo(0, height / 2);
            ctx.lineTo(width, 0);
            ctx.lineTo(width, height);
            ctx.lineTo(0, height / 2);
            ctx.closePath();
            ctx.fill();
            ctx.stroke();
        }

        Component.onCompleted: {
            appMain.colorMapChanged.connect(requestPaint);
        }

        Component.onDestruction: {
            appMain.colorMapChanged.disconnect(requestPaint);
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: (mouse) => {
            soundPlayer.playPressGood();

            // Increment/decrement value by one
            var newValueIndex;
            var sliderPosX = rectSlider.mapFromItem(parent, mouse.x, 0).x;

            if (sliderPosX > handle.x)
            {
                newValueIndex = Math.min(valueIndex + 1, labelData.length - 1);
            }
            else
            {
                newValueIndex = Math.max(valueIndex - 1, 0);
            }
            setValueIndex(newValueIndex);
            signalValueIndexChanged(valueIndex);
            signalValueChanged(labelData[valueIndex]);
        }
    }

    Item {
        height: parent.height
        width: parent.width * 0.88
        anchors.horizontalCenter: parent.horizontalCenter

        Repeater {
            id: rectLabel
            width: parent.width
            height: parent.height * 0.35
            model: labelData
            delegate: Text {
                x: getLabelPosition(index)
                height: rectLabel.height
                width: labelItemWidth
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: (index === valueIndex) ? height * 1 : height * 0.7
                font.family: fontRobotoLight.name
                font.bold: index === valueIndex
                color: mainColor
                text: labelData[index]
                visible: (index === 0) || (index === labelData.length - 1) || (index % labelSpacing === 0)
            }
            onWidthChanged: {
                setHandlePositionFromValue();
            }

        }

        Item {
            id: rectSlider
            width: parent.width
            height: parent.height * 0.6
            anchors.bottom: parent.bottom

            Rectangle {
                id: handle
                width: handleWidth
                height: parent.height
                radius: width * 0.1
                color: handleColor

                Text {
                    anchors.fill: parent
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    font.pixelSize: height * 0.6
                    font.family: fontIcon.name
                    text: "\ue94a"
                    color: mainColor
                }

                onXChanged: {
                    x = Math.max(x, getHandlePosition(0));
                    x = Math.min(x, getHandlePosition(labelData.length - 1));
                }

                onYChanged: {
                    y = 0;
                }

                MouseArea {
                    anchors.fill: parent
                    drag.target: handle

                    onPressed: {
                        soundPlayer.playPressGood();
                        handle.color = Qt.tint(handleColor, colorMap.buttonShadow);
                    }

                    onReleased: {
                        handle.color = handleColor;
                        setValueFromHandlePosition();
                        setHandlePositionFromValue();
                        signalValueIndexChanged(valueIndex);
                        signalValueChanged(labelData[valueIndex]);
                    }

                    onClicked: {
                        handle.color = handleColor;
                    }
                }
            }
        }
    }

    function setLabels(labels)
    {
        if (JSON.stringify(labelData) != JSON.stringify(labels))
        {
            labelData = labels;
            setValueIndex(0);
        }
    }

    function getLabelPosition(labelIdx)
    {
        if (labelData.length < 2)
        {
            return -(labelItemWidth / 2);
        }

        var scaleSpace = rectLabel.width / (labelData.length - 1);
        var labelX = labelIdx * scaleSpace - (labelItemWidth / 2);
        //logDebug("labelX[" + labelIdx + "]="+ labelX);
        return labelX;
    }

    function getHandlePosition(labelIdx)
    {
        var labelX = getLabelPosition(labelIdx);
        var handleX = (labelX + (labelItemWidth / 2)) - (handle.width / 2);
        //logDebug("handleX[" + labelIdx + "]="+ handleX);
        return handleX;
    }

    function setValueFromHandlePosition()
    {
        if (labelData.length < 2)
        {
            valueIndex = 0;
            return;
        }

        //var scaleSpace = handle.parent.width / (labelData.length - 1);
        var distanceToLabel = 0;
        var closestLabelIdx = labelData.length - 1;
        for (var labelIdx = 0; labelIdx < labelData.length; labelIdx++)
        {
            var labelPosX = getLabelPosition(labelIdx) + (labelItemWidth / 2);
            var handlePosX = handle.x + (handle.width / 2);
            var curDistanceToLabel = Math.abs(handlePosX - labelPosX);
            //logDebug(labelIdx + ": " + curDistanceToLabel);
            if ( (labelIdx > 0) &&
                 (curDistanceToLabel > distanceToLabel) )
            {
                closestLabelIdx = labelIdx - 1;
                break;
            }
            distanceToLabel = curDistanceToLabel;
        }

        valueIndex = closestLabelIdx;
    }

    function setHandlePositionFromValue()
    {
        handle.x = getHandlePosition(valueIndex);
    }

    function setValueIndex(newValueIndex)
    {
        valueIndex = newValueIndex;
        setHandlePositionFromValue();
    }

    function setValue(newValue)
    {
        for (var valueIdx = 0; valueIdx < labelData.length; valueIdx++)
        {
            if (labelData[valueIdx] === newValue)
            {
                setValueIndex(valueIdx);
                break;
            }
        }
    }
}
