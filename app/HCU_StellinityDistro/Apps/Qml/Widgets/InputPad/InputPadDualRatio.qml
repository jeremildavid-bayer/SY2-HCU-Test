import QtQuick 2.12
import "../"

InputPadGeneric {
    property int stepSize: 5
    property int minRatio: 5
    property int maxRatio: 95
    property int markerCount: ((maxRatio - minRatio) / 5) + 1
    property int markerHeight: 2
    property var selectedContrast: dsExam.selectedContrast
    property bool handlePressed

    content: [
        Item {
            anchors.fill: parent

            Text {
                x: parent.width * 0.13
                y: sliderContainer.y
                width: parent.width * 0.1
                height: parent.height * 0.1
                font.family: fontIcon.name
                font.pixelSize: height
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                text: "\ue92f"
                color: ((selectedContrast === undefined) || (selectedContrast.ColorCode === "GREEN")) ? colorMap.contrast1 : colorMap.contrast2;
            }

            Text {
                x: parent.width * 0.13
                y: sliderContainer.y + sliderContainer.height - height
                width: parent.width * 0.1
                height: parent.height * 0.1
                font.family: fontIcon.name
                font.pixelSize: height
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                text: "\ue930"
                color: colorMap.saline
            }

            Item {
                id: sliderContainer
                //y: parent.height * 0.1
                height: parent.height * 0.9
                width: parent.width

                Rectangle {
                    id: sliderTop
                    anchors.top: parent.top
                    width: parent.width * 0.4
                    anchors.horizontalCenter: parent.horizontalCenter
                    height: width
                    radius: width / 2
                    color: ((selectedContrast === undefined) || (selectedContrast.ColorCode === "GREEN")) ? colorMap.contrast1 : colorMap.contrast2;

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            var newValue = Math.max(parseInt(currentValue) - stepSize, minRatio);
                            if (currentValue !== newValue)
                            {
                                currentValue = newValue;
                                percentToSlider(currentValue);
                                //signalValueChanged(currentValue);
                            }
                        }
                    }
                }

                Rectangle {
                    id: sliderBottom
                    anchors.bottom: parent.bottom
                    width: parent.width * 0.4
                    anchors.horizontalCenter: parent.horizontalCenter
                    height: width
                    radius: width / 2
                    color: colorMap.saline

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            var newValue = Math.min(parseInt(currentValue) + stepSize, maxRatio);
                            if (currentValue !== newValue)
                            {
                                currentValue = newValue;
                                percentToSlider(currentValue);
                                //signalValueChanged(currentValue);
                            }
                        }
                    }
                }

                Item {
                    id: slider
                    width: parent.width * 0.4
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    anchors.topMargin: sliderTop.height / 2
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: sliderBottom.height / 2

                    Rectangle {
                        id: rectContrast
                        anchors.top: parent.top
                        width: parent.width
                        height: handle.y + (handle.height / 2) - slider.y
                        color: ((selectedContrast === undefined) || (selectedContrast.ColorCode === "GREEN")) ? colorMap.contrast1 : colorMap.contrast2;

                        Text {
                            anchors.fill: parent
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignBottom
                            font.pixelSize: slider.height * 0.12
                            font.family: fontRobotoLight.name
                            text: "<b>" + parseInt(currentValue) + "</b> %"
                            color: colorMap.text01
                        }
                    }

                    Rectangle {
                        id: rectSaline
                        anchors.top: rectContrast.bottom
                        anchors.bottom: parent.bottom
                        width: parent.width
                        color: colorMap.saline

                        Text {
                            anchors.fill: parent
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignTop
                            font.pixelSize: slider.height * 0.12
                            font.family: fontRobotoLight.name
                            text: "<b>" + (100 - parseInt(currentValue)) + "</b> %"
                            color: colorMap.text01
                        }
                    }

                    Repeater {
                        // dot lines
                        model: markerCount
                        delegate: Rectangle {
                            property int percent: minRatio + (index * stepSize)
                            y: ((parent.height / (markerCount - 1)) * index) - (height / 2)
                            width: (percent % 25 == 0) ? parent.width * 0.1 : parent.width * 0.05
                            anchors.right: parent.right
                            color: colorMap.text01
                            height: markerHeight
                        }
                    }
                }

                Item {
                    property int yMin: slider.y - (height / 2)
                    property int yMax: (slider.y + slider.height) - (height / 2)

                    id: handle
                    width: parent.width * 0.36
                    height: parent.height * 0.15
                    anchors.left: slider.right

                    Text {
                        id: iconHandle
                        anchors.fill: parent
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                        text: "\ue900"
                        font.pixelSize: height
                        font.family: fontIcon.name
                        color: handlePressed ? Qt.tint(colorMap.drawerHandleBackground, colorMap.buttonShadow) : colorMap.drawerHandleBackground

                        Text {
                            x: parent.width * 0.17
                            width: parent.width * 0.83
                            height: parent.height
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignHCenter
                            font.pixelSize: height * 0.4
                            font.family: fontIcon.name
                            text: "\ue980"
                            color: colorMap.text01
                        }
                    }

                    onYChanged: {
                        if (y < yMin)
                        {
                            y = yMin;
                        }
                        else if (y > yMax)
                        {
                            y = yMax;
                        }
                        slotValueChanged();
                    }

                    MouseArea {
                        anchors.fill: parent
                        drag.target: handle

                        onPressed: {
                            soundPlayer.playPressGood();
                            handlePressed = true;
                        }

                        onReleased: {
                            handlePressed = false;
                            var percent = parseInt(currentValue);
                            percentToSlider(percent);
                            currentValue = percent;
                            //signalValueChanged(currentValue);
                        }

                        onEntered: {
                            handlePressed = true;
                        }

                        onExited: {
                            handlePressed = false;
                        }
                    }
                }
            }
        }
    ]

    function sliderToPercent()
    {
        var rectContrastHeight = handle.y + (handle.height / 2) - slider.y;
        var percent = (((maxRatio - minRatio) / slider.height) * rectContrastHeight) + minRatio;
        return percent;
    }

    function percentToSlider(percent)
    {
        var m = (maxRatio - minRatio) / slider.height;
        var newRectContrastHeight = (percent - minRatio) / m;
        var newHandleY = slider.y + newRectContrastHeight - (handle.height / 2);
        handle.y = newHandleY;
    }

    function slotValueChanged()
    {
        var percent = sliderToPercent();
        var percentFinal = (Math.round((percent / stepSize)) * stepSize);
        percentFinal = Math.max(percentFinal, minRatio);
        percentFinal = Math.min(percentFinal, maxRatio);

        modified = true;
        currentValue = percentFinal;

        if (textWidget !== undefined)
        {
            textWidget.text = currentValue;
        }
        signalValueChanged(currentValue);
    }

    function init(val)
    {
        var percentFinal = (Math.round((val / stepSize)) * stepSize);
        percentFinal = Math.max(percentFinal, minRatio);
        percentFinal = Math.min(percentFinal, maxRatio);
        percentToSlider(percentFinal);
    }
}
