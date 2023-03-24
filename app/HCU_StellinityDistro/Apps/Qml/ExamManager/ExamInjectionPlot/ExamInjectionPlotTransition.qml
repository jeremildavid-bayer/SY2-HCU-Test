import QtQuick 2.12

Item {
    property double plotX: 0
    property double plotY: 0
    property string markerColor1: "white"
    property string markerColor2: markerColor1
    property string transitionType: "NORMAL" // "NORMAL", "START", "STARTED", "PAUSE", "RESUME", "SKIPPED", "PRESSURE_LIMIT_START", "PRESSURE_LIMIT_END", "END", "ABORTED"
    property int lineWidth: getLineWidth()

    id: root
    height: plotAreaH - y + 1
    y: getY()
    x: getX()
    visible: (enabled) && (x >= 0)

    Item {
        // Normal transition line
        visible: getNormalTransitionLineVisible()
        Rectangle {
            id: normalTransitionLine
            x: -(width / 2)
            y: (noticeList.length > 0) ? (root.height - height) : 0
            width: lineWidth + (((transitionType == "START") || (transitionType == "STARTED") || (transitionType == "ABORTED")) ? 1 : 0)
            height: (noticeList.length > 0) ? root.height * 0.8 : root.height // If Catheter Limits Exceeded noticeList is not empty, avoid the notice display area by shorten the marks
            color: markerColor1
        }
    }

    Item {
        // Double transition lines
        visible: (transitionType == "END")
        Rectangle {
            width: lineWidth
            height: normalTransitionLine.height
            y: normalTransitionLine.y
            color: colorMap.injectPlotMarkerBackground
            x: (markerWidth * -0.1) - (width / 2)
        }

        Rectangle {
            width: lineWidth;
            height: normalTransitionLine.height
            y: normalTransitionLine.y
            color: colorMap.injectPlotMarkerBackground
            x: (markerWidth * 0.1) - (width / 2)
        }
    }

    Item {
        width: markerWidth
        height: markerWidth
        x: ( (transitionType == "START") || (transitionType == "STARTED") ) ? 0 : -(markerWidth / 2)
        y: 0 + root.height - markerWidth

        Rectangle {
            id: marker1
            width: parent.width
            height: (parent.height / 2) + radius
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
            font.family: fontIcon.name
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

        if (x >= 0)
        {
            // HACK: Transition line needs to be shifted little bit right
            x += 1;
        }
        return x;
    }

    function getY()
    {
        return -plotAreaY;
    }

    function getLineWidth()
    {
        if ( (transitionType == "PRESSURE_LIMIT_START") ||
             (transitionType == "PRESSURE_LIMIT_END") )
        {
            return 4;
        }
        return 2;
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
                 (transitionType == "ABORTED") )
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
        else if (transitionType == "ABORTED")
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
        return colorMap.white01;
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
