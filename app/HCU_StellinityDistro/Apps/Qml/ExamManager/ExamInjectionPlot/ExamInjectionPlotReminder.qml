import QtQuick 2.12

Item {
    property int xValue: 0

    visible: getXAxisPosFromVal(xValue) !== -1
    width: markerWidth
    height: markerWidth
    y: plotAreaH - height
    x: getXAxisPosFromVal(xValue)

    Rectangle {
        id: marker1
        width: parent.width
        height: parent.height / 2
        color: colorMap.injectPlotMarkerBackground
        radius: 4
    }

    Rectangle {
        width: parent.width
        y: marker1.height - marker1.radius
        height: parent.height - y
        color: colorMap.injectPlotMarkerBackground
    }

    Text {
        anchors.fill: parent
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        color: colorMap.injectPlotMarkerIcon
        font.pointSize: parent.height * 0.75
        font.family: fontIcon.name
        text: "\ue917"
    }
}

