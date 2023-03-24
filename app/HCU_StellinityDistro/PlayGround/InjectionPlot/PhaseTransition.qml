import QtQuick 2.9

Item {
    property double plotX: 0
    property double plotY: 0
    property string markerColor1: "white"
    property string markerColor2: markerColor1
    property string transitionType: "NORMAL" // "NORMAL", "START", "STARTED", "PAUSE", "RESUME", "SKIPPED", "PRESSURE_LIMIT_START", "PRESSURE_LIMIT_END", "END", "TERMINATED"
    property int lineWidth: (transitionType == "PRESSURE_LIMIT_START") || (transitionType == "PRESSURE_LIMIT_END") ? 4 : 2

    id: root
    height: plotAreaH - y
    y: getY()
    x: getX()
    visible: (enabled) && (x >= 0)

    Item {
        // Normal transition line
        visible: getNormalTransitionLineVisible()
        Rectangle {
            width: lineWidth + (((transitionType == "START") || (transitionType == "STARTED") || (transitionType == "TERMINATED")) ? 2 : 0)
            height: root.height
            y: 0;
            color: markerColor1
            x: -(width / 2)
        }
    }

    Item {
        // Double transition lines
        visible: (transitionType == "END")
        Rectangle {
            width: lineWidth
            height: root.height
            y: 0;
            color: colorMap.injectPlotMarkerBackground
            x: (markerWidth * -0.1) - (width / 2)
        }

        Rectangle {
            width: lineWidth;
            height: root.height;
            y: 0;
            color: colorMap.injectPlotMarkerBackground
            x: (markerWidth * 0.1) - (width / 2)
        }
    }

    Item {
        width: markerWidth
        height: markerWidth
        y: 0 + root.height - markerWidth
        x: ( (transitionType == "START") || (transitionType == "STARTED") ) ? 0 : -(markerWidth / 2)

        Rectangle {
            id: marker1
            width: parent.width
            height: parent.height / 2
            color: getMarkerColor(0)
            radius: 4
        }

        Rectangle {
            width: parent.width
            y: marker1.height - marker1.radius
            height: parent.height - y
            color: getMarkerColor(1)
        }

        Text {
            id: markerIcon
            anchors.fill: parent
            //font.family: fontIcon.name
            font.pixelSize: height * 0.75
            text: getIcon()
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            color: getIconColor()
        }
    }

    function getX()
    {
        var x = getXAxisPosFromVal(plotX);
        if ( (transitionType == "PRESSURE_LIMIT_START") ||
             (transitionType == "PRESSURE_LIMIT_END") )
        {
            // HACK: pressure limit boundary needs to be shifted little bit right
            x += markerWidth * 0.1;
        }
        return x;
    }

    function getY()
    {
        if ( (transitionType == "PRESSURE_LIMIT_START") ||
             (transitionType == "PRESSURE_LIMIT_END") )
        {
            return getYAxisPosFromVal(plotY);
        }
        else
        {
            return -plotAreaY;
        }
    }

    function getNormalTransitionLineVisible()
    {
        return ( (transitionType == "START") ||
                 (transitionType == "STARTED") ||
                 (transitionType == "NORMAL") ||
                 (transitionType == "PAUSE") ||
                 (transitionType == "RESUME") ||
                 (transitionType == "SKIPPED") ||
                 (transitionType == "PRESSURE_LIMIT_START") ||
                 (transitionType == "PRESSURE_LIMIT_END") ||
                 (transitionType == "TERMINATED") )
    }

    function getIcon()
    {
        if (transitionType == "PAUSE")
        {
            return "\ue94f";
        }
        else if (transitionType == "RESUME")
        {
            return "\ue94e";
        }
        else if (transitionType == "STARTED")
        {
            return "\ue94e";
        }
        else if (transitionType == "SKIPPED")
        {
            return "\ue950";
        }
        else if (transitionType == "PRESSURE_LIMIT_START")
        {
            return "\ue93f";
        }
        else if (transitionType == "END")
        {
            return "\ue915";
        }
        else if (transitionType == "TERMINATED")
        {
            return "\ue915";
        }
        else
        {
            return "";
        }
    }

    function getIconColor()
    {
        if (transitionType == "END")
        {
            return colorMap.injectPlotMarkerIcon;
        }
        else if (transitionType == "TERMINATED")
        {
            return colorMap.white01;
        }
        else if (transitionType == "PRESSURE_LIMIT_START")
        {
            return colorMap.white01;
        }
        else
        {
            return colorMap.text01;
        }
    }

    function getMarkerColor(markderIdx)
    {
        if (transitionType == "END")
        {
            return colorMap.injectPlotMarkerBackground;
        }
        else if (transitionType == "PRESSURE_LIMIT_START")
        {
            return colorMap.red;
        }
        else if (transitionType == "PRESSURE_LIMIT_END")
        {
            return "transparent";
        }
        else if (markderIdx === 1)
        {
            return markerColor2;
        }
        else
        {
            return markerColor1;
        }
    }
}
