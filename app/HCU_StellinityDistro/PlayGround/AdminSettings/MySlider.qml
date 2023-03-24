import QtQuick 2.9

Item {
    property var labelData: []
    property int labelItemWidth: rectLabels.width * 0.1
    property int handleWidth: width * 0.1
    property int valueIndex: 0
    property int labelSpacing: 1
    signal signalValueChanged(int val)

    id: root

    Canvas {
        // Draw Background Line
        y: rectSlider.y + (rectSlider.height / 2) - (height / 2)
        width: parent.width
        height: parent.height * 0.15

        onPaint: {
            var ctx = getContext("2d");
            ctx.save();
            ctx.clearRect(0, 0, width, height);
            ctx.strokeStyle = "black";
            ctx.lineWidth = 5;
            ctx.fillStyle = "black";
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
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
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
        }
    }


    Item {
        id: rectLabels
        height: parent.height
        width: parent.width * 0.7
        anchors.horizontalCenter: parent.horizontalCenter

        Repeater {
            id: rectLabel
            width: parent.width
            height: parent.height * 0.4
            model: labelData
            delegate: Text {
                x: getLabelPosition(index)
                height: rectLabel.height
                width: labelItemWidth
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: (index == valueIndex) ? height * 0.8 : height * 0.7
                font.bold: index == valueIndex
                text: labelData[index]
                visible: (index == 0) || (index == labelData.length - 1) || (index % labelSpacing == 0)
            }
        }

        Item {
            id: rectSlider
            width: parent.width
            height: parent.height * 0.6
            anchors.bottom: parent.bottom

            /*MouseArea {
                anchors.fill: parent
                onClicked: {
                    // Increment/decrement value by one
                    var newValueIndex;

                    if (mouse.x > handle.x)
                    {
                        newValueIndex = Math.min(valueIndex + 1, labelData.length - 1);
                    }
                    else
                    {
                        newValueIndex = Math.max(valueIndex - 1, 0);
                    }
                    setValueIndex(newValueIndex);
                }
            }*/

            Rectangle {
                id: handle
                width: handleWidth
                height: parent.height
                radius: width * 0.1
                color: "gray"

                Text {
                    anchors.fill: parent
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    font.pixelSize: height * 0.6
                    font.family: fontIcon.name
                    text: "\ue94a"
                    color: "white"
                }

                onXChanged: {
                    x = Math.max(x, getHandlePosition(0));
                    x = Math.min(x, getHandlePosition(labelData.length - 1));
                    console.log("sliderPosX=" + x);
                }

                onYChanged: {
                    y = 0;
                }

                MouseArea {
                    anchors.fill: parent
                    drag.target: handle

                    onPressed: {
                        handle.color = "lightgray";
                    }

                    onReleased: {
                        handle.color = "gray";
                        setValueFromHandlePosition();
                        setHandlePositionFromValue();
                    }

                    onClicked: {
                        handle.color = "gray";
                    }

                    onExited: {
                        handle.color = "gray";
                    }
                }
            }
        }
    }

    onLabelItemWidthChanged: {
        console.log("width=" + width);
        //labelItemWidth = width * 0.08;
        var prevLabelData = labelData;
        labelData = [];
        setLabels(prevLabelData);
    }

    function setLabels(labels)
    {
        if (JSON.stringify(labelData) != JSON.stringify(labels))
        {


            if (labels.length < ((rectLabels.width / labelItemWidth) - 10) )
            {
                console.log("1");
                labelSpacing = 1;
            }
            else
            {
                console.log("2");
                labelSpacing = (labels.length - 1) / (rectLabels.width / labelItemWidth);
            }
            console.log("label max count=" + (rectLabels.width / labelItemWidth));

            var spacing = ((labels.length - 1) / (rectLabels.width / labelItemWidth));
            spacing = Math.ceil(spacing);
            spacing = Math.max(spacing, 1);
            //spacing = spacing * 2;
            labelSpacing = spacing;
            console.log("possible spacing=" + spacing);
            //console.log("labelItemWidth=" + labelItemWidth);
            //console.log("(rectLabel.width / labelItemWidth)=" + (rectLabels.width / labelItemWidth) + ", labelSpacing=" + labelSpacing);

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
        //console.log("labelX[" + labelIdx + "]="+ labelX);
        return labelX;
    }

    function getHandlePosition(labelIdx)
    {
        var labelX = getLabelPosition(labelIdx);
        var handleX = (labelX + (labelItemWidth / 2)) - (handle.width / 2);
        //console.log("handleX[" + labelIdx + "]="+ handleX);
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
            //console.log(labelIdx + ": " + curDistanceToLabel);
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
}
