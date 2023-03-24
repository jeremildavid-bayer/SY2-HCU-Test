import QtQuick 2.5

Item {
    property double plotX: 0
    property string lineColor: "white"
    property string transitionType: "NORMAL" // "NORMAL", "START", "END"
    property int lineWidth: 2
    property int markerWidth: 4

    id: root
    x: getXAxisPosFromVal(plotX);

    Item {
        visible: transitionType == "NORMAL"
        Rectangle {
            width: lineWidth;
            height: root.height;
            y: 0;
            color: lineColor
            x: -(width / 2)
        }
    }

    Item {
        visible: transitionType == "END"
        Rectangle {
            width: lineWidth;
            height: root.height;
            y: 0;
            color: lineColor
            x: (markerWidth * -0.1) - (width / 2)
        }

        Rectangle {
            width: lineWidth;
            height: root.height;
            y: 0;
            color: lineColor
            x: (markerWidth * 0.1) - (width / 2)
        }
    }

    Rectangle {
        color: lineColor
        width: markerWidth
        height: markerWidth
        y: 0 + root.height - markerWidth
        x: -(markerWidth / 2)
    }
}
