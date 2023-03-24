import QtQuick 2.5
import QtCharts 2.2

//import "../../../Util.js" as Util
//import "../../../Widgets"
import "Util.js" as Util

Rectangle {

    // #########################################################################################################
    QtObject {
        id: dsEnvironment
        property int phaseCountMax: 6
    }

    QtObject {
        id: colorMap

        property string mainBackground: "#f8f8f8"
        property string titleBarBackground: "#e6e7e8"
        property string actionBarBackground: "#e6e7e8"
        property string subPanelBackground: "#aaafb4"
        property string comboBoxBackground: "#d1d3d4"
        property string comboBoxActive: "#e4e5e6"
        property string drawerHandleBackground: "#d1d3d4"
        property string elapsedTimeBackground: "#aaafb4"
        property string remindersBackground: "#d1d3d4"
        property string fatalAlertsBackground: "#d1d3d4"

        property string scrollBar: "#a6a7a8"
        property string actionButtonBackground: "#f0d415"
        property string actionButtonText: "#1e272a"
        property string text01: "#1e272a"
        property string text02: "#777d7f"
        property string grid: "#b2b6b7"
        property string warnText: "#ff7f00"
        property string errText: "#ff0000"

        property string consoleBackground: "#ffffff"
        property string keypadButton: "#d1d3d4"
        property string keypadCancelButton: "#f1f2f2"

        property string popupBackground: "#d0ffffff"

        property string startupBackground: "#e6e7e8"
        property string startupWarningPanel: "#d1d3d4"

        property string injectPhaseProgressBackground: "#66ffffff"

        property string injectPlotGrid: "#d2d2d2"
        property string injectPlotMarkerBackground: "#1e272a"
        property string injectPlotMarkerIcon: "#f8f8f8"

        property string injectControlBarBackground: "#f1f2f2"

        property string statusIconText1: "#1e272a"
        property string statusIconText2: "#a0a4a6"
        property string statusIconRed: "#ef4237"
        property string statusIconBatteryOrange: "#f36f2a"
        property string statusIconBatteryRed: "#ed1c24"
        property string statusIconBatteryGray: "#828789"

        property string navBtnSelectedBorder: "#1e272a"
        property string navBtnProgressLine: "#8f9395"

        property string homeBackground: "#e6e7e8"
        property string homeMenuBackground: "#f8f8f8"

        property string deviceButtonBackground: "#d1d3d4"
        property string deviceIconMissing: "#b2b6b7"
        property string deviceIconSelected: "#1e272a"
        property string deviceIconWasteLevel: "#8aa4b1"
        property string deviceIconWarningState: "#f36f2a"
        property string deviceIconSudsError: "#ed1c24"
        property string devicePlunger: "#ffffff"
        property string deviceToggleGrabber: "#ffffff"
        property string deviceMovePistonBackground: "#d6d7d8"
        property string deviceMovePistonButton: "#f1f2f2"
        property string deviceButtonRed: "#ed1c24"

        property string saline: "#0090c5"
        property string contrast1: "#6bc200"
        property string contrast2: "#9f7ad1"
        property string paused: "#90969f"
        property string white01: "#FEFEFE"
        property string blk01: "#1E272A"
        property string gry01: "#90969F"
        property string blu01: "#8AA4B1"
        property string yellow01: "#FFFF00"
        property string yellow02: "#FFF3B2"
        property string red: "#ed1c24"
        property string orange: "#F3702B"
        property string buttonShadow: "#c0e6e7e8"
        property string buttonDisabled: "#e0e6e7e8"
    }


    // #########################################################################################################

    property bool reviewMode: true

    property int maxPhases: 6
    property string pressureUnit: "kpa"
    property int maxPressureKpa: 2000
    property int xRangeDefault: 80000
    property int yWarningLevel: 300
    property string contrastColor: "GREEN"

    property int yMin: 0
    property int yMax: 350
    property int xMin: 0
    property int xMinPrev: 0
    property int xMax: xRangeDefault - xMin
    property int xSpace: 20000 // x-axis space after last x value
    property int curPhaseIdx: 0
    property int xLast: 0
    property int yLast: 0
    property int precisionCtrlConst: 10 // increase precision to gain performace
    property bool isPressuerLimiting: false

    // Some hardcoded plot geometry parameters
    property var plotAreaOffsetX: { "kpa" : -80, "psi" : -74, "kg/cm2" : -64 }
    property int plotAreaMarginTop: 85
    property int plotAreaMarginRight: 60
    property int plotAreaY: height * 0.05
    property int plotAreaH: plot.height - 51

    // Axis grid and label
    property int yAxisGridInterval: 100
    property int yAxisLabelInterval: 100

    property int xAxisGridInterval: 10000
    property int xAxisLabelInterval: 30000

    property var phaseTransitionComponent: null
    property var userPauseResumeTransitionObjects: []
    property var pressureLimitTransitionObjects: []

    property var reminderIconComponent: null
    property var reminderIconObjects: []

    property string fontColor: colorMap.text01
    property string limitLineColor: colorMap.red
    property string gridColor: colorMap.injectPlotGrid
    property string endTimeGridColor: colorMap.injectPlotMarkerBackground

    property int plotLineWidth: 2
    property double plotLineAlpha: 0.25
    property int majorGridWidth: 1
    property int minorGridWidth: 1
    property int axisLabelWidth: plotArea.width * 0.1
    property int axisLabelHeight: plotAreaH * 0.05
    property int markerWidth: plotArea.width * 0.022
    property var phaseTransitions: []
    property var phaseTransitionTerminated
    property var curLineSeries: null
    property var dataPoints: []

    id: root
    clip: true

    Item {
        id: plotBackground
        x: plotAxisYInfo.width
        y: plotAreaY

        // -------------------------------------
        // Y Axis Grids
        Repeater {
            id: yAxisGrids
            model: (yMax - yMin) / yAxisGridInterval

            delegate: Item {
                Rectangle {
                    height: majorGridWidth
                    width: plot.width + plotAreaMarginRight
                    y: getYAxisPos(index) - (height / 2)
                    color: gridColor
                }
            }
        }

        // -------------------------------------
        // X Axis Base
        Rectangle
        {
            height: majorGridWidth
            width: plot.width + plotAreaMarginRight
            y: plotAreaH
            color: fontColor
            //opacity: 0.2
        }

        // -------------------------------------
        // Y-Warning Level
        Rectangle
        {
            id: yLimitLine
            visible: getYAxisPosFromVal(yWarningLevel) !== -1
            height: majorGridWidth
            width: plot.width + plotAreaMarginRight
            y: getYAxisPosFromVal(yWarningLevel) - (height / 2)
            color: limitLineColor
        }

        Rectangle
        {
            visible: yLimitLine.visible
            width: plot.width + plotAreaMarginRight
            y: -plotAreaY
            height: plotAreaY + yLimitLine.y
            color: limitLineColor
            opacity: 0.15
        }
    }

    Flickable {
        id: plotArea
        x: plotAxisYInfo.width
        width: parent.width - x
        height: parent.height
        contentWidth: plot.width
        contentHeight: plot.height
        flickableDirection: Flickable.HorizontalFlick
        interactive: reviewMode
        clip: true

        /*onContentXChanged: {
            console.log("Visible.x=" + contentX);
        }*/

        Item {
            id: plot
            y: plotAreaY
            width: plotArea.width
            height: (plotArea.height * 0.95) - plotAreaY

            // -------------------------------------
            // X Axis Grids and Labels
            Repeater {
                id: xAxisGrids
                //model: (xMax - xMin) / xAxisGridInterval
                delegate: Item {
                    Rectangle {
                        width: minorGridWidth
                        height: plotAreaH + plotAreaY
                        x: getXAxisPos(index) - (width / 2)
                        y: -plotAreaY
                        color: gridColor
                    }
                    Text {
                        width: axisLabelWidth
                        height: axisLabelHeight
                        y: plotAreaH + (height * 0.1)
                        x: getXAxisPos(index) - (width / 2)
                        color: fontColor
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pixelSize: height * 0.9
                        text: getXAxisLabel(index)
                        //font.family: fontRobotoLight.name
                    }
                }
            }

            /*Image {
                id: image
                x: chartView.x
                y: chartView.y
                width: chartView.width
                height: chartView.height
            }*/

            // -------------------------------------
            // Chart
            ChartView {
                id: chartView
                x: plotAreaOffsetX[pressureUnit]
                width: parent.width - x + plotAreaMarginRight
                height: parent.height
                antialiasing: true
                legend.visible: false
                backgroundColor: "transparent"



                ValueAxis {
                    id: valueAxisY
                    labelsVisible: false
                    gridVisible: false
                    min: yMin
                    max: yMax
                    color: "transparent"
                    gridLineColor: "transparent"
                }

                ValueAxis {
                    id: valueAxisX
                    labelsVisible: false
                    gridVisible: false
                    min: 0
                    max: xRangeDefault
                    color: "transparent"
                    gridLineColor: "transparent"
                }

                AreaSeries {
                    axisX: valueAxisX;
                    axisY: valueAxisY;
                    borderWidth: plotLineWidth;
                    //useOpenGL: true
                }
            }
        }
    }

    Item {
        visible: reviewMode
        x: plotArea.x
        y: plotArea.height * 0.95
        width: plotArea.width
        height: parent.height - y
        ScrollBar {
            y: 0
            flickable: plotArea
        }
    }

    Item {
        x: plotArea.x
        y: plotArea.y
        width: plotArea.width
        height: plotArea.height
        ListFade {
            flickable: plotArea
            forceShowTop: (!reviewMode) && (xMin > 0)
        }
    }

    Item {
        id: plotAxisYInfo
        width: parent.width * 0.1
        height: parent.height
        //color: root.color

        // -------------------------------------
        // Y Axis Info
        Text {
            id: yAxisInfo
            y: parent.height * 0.06
            x: plotAxisYInfo.width - (width * 1.04)
            width: axisLabelWidth
            height: axisLabelHeight
            color: fontColor
            //font.family: fontRobotoBold.name
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: height * 1
        }

        // -------------------------------------
        // Y Max Value
        Text {
            x: plotAxisYInfo.width * 1.15
            y: yLimitLine.y
            width: axisLabelWidth
            height: axisLabelHeight
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignBottom
            color: limitLineColor
            //font.family: fontRobotoBold.name
            font.pixelSize: height * 1.1
            text: yWarningLevel
        }

        // -------------------------------------
        // Y Axis Grids and Labels
        Repeater {
            id: yAxisLabels
            model: (yMax - yMin) / yAxisGridInterval
            delegate: Item {
                Text {
                    width: axisLabelWidth
                    height: axisLabelHeight
                    x: plotAxisYInfo.width - (width * 1.2)
                    y: plotAreaY + getYAxisPos(index) - (height / 2)
                    color: fontColor
                    horizontalAlignment: Text.AlignRight
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: height * 0.9
                    //font.family: fontRobotoLight.name
                    text: getYAxisLabel(index)
                }
            }
        }

        // -------------------------------------
        // Y Axis Boundary
        Rectangle {
            height: plotAreaH + plotAreaY
            width: majorGridWidth
            x: plotAxisYInfo.width
            y: 0
            color: fontColor
            opacity: 0.4
        }
    }

    /*LoadingGif {
        id: plotLoadingGif
    }*/

    onPressureUnitChanged: {
        if (pressureUnit === "kpa")
        {
            yAxisGridInterval = 200;
            yAxisLabelInterval = 400;
            yMax = 2451.665;
        }
        else if (pressureUnit === "psi")
        {
            yAxisGridInterval = 50;
            yAxisLabelInterval = 100;
            yMax = 355.584;
        }
        else if (pressureUnit === "kg/cm2")
        {
            yAxisGridInterval = 2.5;
            yAxisLabelInterval = 5;
            yMax = 25;
        }
        slotSetMaxPressure(maxPressureKpa);
        yAxisInfo.text = pressureUnit;
    }

    function slotSetIsLoading(isLoading)
    {
        plotLoadingGif.show(isLoading);
    }

    function slotSetRange(xMin_, xMax_)
    {
        xMin = xMin_;
        xMax = xMax_;
        console.log("Total Range=" + xMin + " to " + Util.getMinimisedDurationStr(Util.millisecToDurationStr(xMax)));

        valueAxisX.min = xMin;
        xMax = Math.max(xMax, xRangeDefault);
        valueAxisX.max = xMax;
        plot.width = (plotArea.width * (xMax - xMin)) / xRangeDefault;
        //plot.width = plot.width / 2;

        if (reviewMode)
        {
            xAxisGrids.model = (xMax - xMin) / xAxisGridInterval;
        }
        else
        {
            reloadAxis();
        }
    }

    function slotReset()
    {
        isPressuerLimiting = false;
        chartView.removeAllSeries();
        curLineSeries = null;
        xMin = 0;
        xMax = xMin + xRangeDefault;
        reloadAxis();

        var i;
        for (i = 0; i < reminderIconObjects.length; i++)
        {
            reminderIconObjects[i].destroy();
        }
        reminderIconObjects = [];

        for (i = 0; i < userPauseResumeTransitionObjects.length; i++)
        {
            userPauseResumeTransitionObjects[i].destroy();
        }
        userPauseResumeTransitionObjects = [];

        for (i = 0; i < pressureLimitTransitionObjects.length; i++)
        {
            pressureLimitTransitionObjects[i].destroy();
        }
        pressureLimitTransitionObjects = [];

        for (i = 0; i < phaseTransitions.length; i++)
        {
            phaseTransitions[i].enabled = false;
        }
        phaseTransitionTerminated.enabled = false;

        dataPoints = [];
    }

    function slotSetContrastColor(newContrastColor)
    {
        contrastColor = newContrastColor;
    }

    function slotSetMaxPressure(newMaxPressureKpa)
    {
        newMaxPressureKpa = parseInt(newMaxPressureKpa);
        maxPressureKpa = newMaxPressureKpa;
        yWarningLevel = Util.getPressure(pressureUnit, maxPressureKpa).toFixed(0);
    }

    function slotSetPhaseInfo(phaseIdx, x, phaseType, contrastPercentage)
    {
        phaseIdx = parseInt(phaseIdx);
        x = parseInt(x);
        contrastPercentage = parseInt(contrastPercentage);

        var lineColors = getLineColors(phaseType, contrastPercentage);
        phaseTransitions[phaseIdx].enabled = true;
        phaseTransitions[phaseIdx].plotX = x;
        phaseTransitions[phaseIdx].markerColor1 = lineColors[0];
        phaseTransitions[phaseIdx].markerColor2 = lineColors[1];
        phaseTransitions[phaseIdx].transitionType = (phaseIdx === 0) ? "START" : "NORMAL";
    }

    function getXAxisPos(index)
    {
        var pixelPerX = (plot.width) / (xMax - xMin);
        var startPos = -(xMin % xAxisGridInterval) * pixelPerX;
        if (startPos != 0)
        {
            startPos += (pixelPerX * xAxisGridInterval);
        }

        var pos = startPos + (index * pixelPerX * xAxisGridInterval);
        return pos;
    }

    function getYAxisPos(index)
    {
        var pixelPerY = (plot.height - plotAreaMarginTop) / (yMax - yMin);
        var startPos = -(yMin % yAxisGridInterval) * pixelPerY;
        if (startPos != 0)
        {
            startPos += (pixelPerY * yAxisGridInterval);
        }
        startPos += plotAreaH;
        var pos = startPos - (index * pixelPerY * yAxisGridInterval);
        //console.log("index" + (index * yAxisGridInterval) + ": startPos=" + startPos);
        return pos;
    }

    function getXAxisLabel(index)
    {
        var startVal = Math.floor(xMin / xAxisGridInterval) * xAxisGridInterval;
        if (xMin % xAxisGridInterval != 0)
        {
            startVal += xAxisGridInterval;
        }

        var ms = startVal + (index * xAxisGridInterval);
        if ( (ms % xAxisLabelInterval == 0) && (ms > 0) )
        {
            var sec = Math.round(ms / 1000);
            var min = Math.floor(sec / 60);
            sec = sec % 60;

            var minStr = min;
            if (min < 10)
            {
                minStr = "0" + min;
            }

            var secStr = sec;
            if (sec < 10)
            {
                secStr = "0" + sec;
            }

            var strLabel = minStr + ":" + secStr;
            return strLabel;
        }
        return "";
    }

    function getYAxisLabel(index)
    {
        var startVal = Math.floor(yMin / yAxisGridInterval) * yAxisGridInterval;
        if (yMin % yAxisGridInterval != 0)
        {
            startVal += yAxisGridInterval;
        }

        var yVal = startVal + (index * yAxisGridInterval);
        if ( (yVal % yAxisLabelInterval == 0) && (yVal > 0) )
        {
            return yVal;
        }
        return "";
    }

    function getXAxisPosFromVal(val)
    {
        var pixelPerX = (plot.width) / (xMax - xMin);
        var pos = pixelPerX * (val - xMin);

        if ( (pos >= 0) && (pos <= plot.width) )
        {
            return pos;
        }
        return -1;
    }

    function getYAxisPosFromVal(val)
    {
        var pixelPerY = (plot.height - plotAreaMarginTop) / (yMax - yMin);
        var pos = pixelPerY * (val - yMin);

        if ( (pos >= 0) && (pos <= plotAreaH) )
        {
            pos = plotAreaH - pos;
            return pos;
        }
        return -1;
    }

    function getLineColors(phaseType, contrastPercentage)
    {
        var contrastLineColor = (contrastColor === "GREEN") ? colorMap.contrast1 : colorMap.contrast2;
        var fluidLineColor1, fluidLineColor2;

        if (phaseType === "Fluid")
        {
            if (contrastPercentage === 100)
            {
                fluidLineColor1 = contrastLineColor;
                fluidLineColor2 = contrastLineColor;
            }
            else if (contrastPercentage === 0)
            {
                fluidLineColor1 = colorMap.saline;
                fluidLineColor2 = colorMap.saline;
            }
            else
            {
                fluidLineColor1 = contrastLineColor;
                fluidLineColor2 = colorMap.saline;
            }
        }
        else
        {
            fluidLineColor1 = colorMap.paused;
            fluidLineColor2 = colorMap.paused;
        }
        return [ fluidLineColor1, fluidLineColor2 ];
    }



    function slotSetEndTime(phaseCount, x)
    {
        for (var phaseIdx = phaseCount; phaseIdx < maxPhases; phaseIdx++)
        {
            // Hide all non-visible phases
            phaseTransitions[phaseIdx].enabled = false;
            phaseTransitions[phaseIdx].plotX = x;
        }

        phaseTransitions[maxPhases].enabled = true;
        phaseTransitions[maxPhases].plotX = x;
        phaseTransitions[phaseIdx].transitionType = "END";
    }

    function slotSetTerminatedTime(x, terminatedReason)
    {
        if (terminatedReason === "Normal")
        {
            phaseTransitionTerminated.enabled = false;
        }
        else
        {
            if (terminatedReason === "AbortedByRequest")
            {
                phaseTransitionTerminated.markerColor1 = colorMap.warnText;
                phaseTransitionTerminated.markerColor2 = colorMap.warnText;
            }
            else
            {
                phaseTransitionTerminated.markerColor1 = colorMap.errText;
                phaseTransitionTerminated.markerColor2 = colorMap.errText;
            }

            phaseTransitionTerminated.enabled = true;
            phaseTransitionTerminated.plotX = x;
        }
    }

    function slotSetPhaseSkipped(phaseIdx)
    {
        // Set icon
        phaseIdx = parseInt(phaseIdx);
        console.log("phase" + phaseIdx + ": skipped");
        phaseTransitions[phaseIdx].transitionType = "SKIPPED";
        curPhaseIdx = phaseIdx;
        rescale();
    }

    function slotAddUserPauseSeries(x)
    {
        x = parseInt(x);

        //console.log("Adding UserPauseSeries: color=" + colorMap.paused);
        addUserPauseResumePhaseTransition("PAUSE", x, colorMap.paused, colorMap.paused);

        curLineSeries = chartView.createSeries(ChartView.SeriesTypeArea, "UserPaused", chartView.axisX, chartView.axisY);
        //curLineSeries.useOpenGL = true;
        curLineSeries.borderWidth = plotLineWidth;
        curLineSeries.borderColor = colorMap.paused;
        curLineSeries.color = isPressuerLimiting ? colorMap.red : colorMap.paused;
        curLineSeries.color.a = plotLineAlpha;
    }

    function slotAddUserResumeSeries(phaseIdx, x)
    {
        phaseIdx = parseInt(phaseIdx);
        x = parseInt(x);

        //console.log("Adding UserResumeSeries: color=" + phaseTransitions[phaseIdx].markerColor1);
        addUserPauseResumePhaseTransition("RESUME", x, phaseTransitions[phaseIdx].markerColor1, phaseTransitions[phaseIdx].markerColor2);

        curLineSeries = chartView.createSeries(ChartView.SeriesTypeArea, "Phase", chartView.axisX, chartView.axisY);
        //curLineSeries.useOpenGL = true;
        curLineSeries.borderWidth = plotLineWidth;
        curLineSeries.borderColor = phaseTransitions[phaseIdx].markerColor1;
        curLineSeries.color = isPressuerLimiting ? colorMap.red : phaseTransitions[phaseIdx].markerColor1;
        curLineSeries.color.a = plotLineAlpha;
    }

    function slotAddPhaseSeries(lineColor)
    {
        //console.log("Adding PhaseSeries: color=" + lineColors);
        curLineSeries = chartView.createSeries(ChartView.SeriesTypeArea, "Phase", chartView.axisX, chartView.axisY);
        //curLineSeries.useOpenGL = true;
        curLineSeries.borderWidth = plotLineWidth;
        curLineSeries.borderColor = lineColor;
        curLineSeries.color = isPressuerLimiting ? colorMap.red : lineColor;
        curLineSeries.color.a = plotLineAlpha;
    }

    function slotAddPressureLimitStartSeries(phaseIdx, x)
    {
        phaseIdx = parseInt(phaseIdx);
        x = parseInt(x);

        //console.log("Adding PressureLimit: color=" + colorMap.paused);
        addPressureLimitTransition("PRESSURE_LIMIT_START", x);
        isPressuerLimiting = true;

        curLineSeries = chartView.createSeries(ChartView.SeriesTypeArea, "PressureLimitStart", chartView.axisX, chartView.axisY);
        //curLineSeries.useOpenGL = true;
        curLineSeries.borderWidth = plotLineWidth;
        curLineSeries.borderColor = phaseTransitions[phaseIdx].markerColor1;
        curLineSeries.color = colorMap.red;
        curLineSeries.color.a = plotLineAlpha;
    }

    function slotAddPressureLimitEndSeries(phaseIdx, x)
    {
        phaseIdx = parseInt(phaseIdx);
        x = parseInt(x);

        //console.log("Adding PressureLimit: color=" + colorMap.paused);
        addPressureLimitTransition("PRESSURE_LIMIT_END", x);

        curLineSeries = chartView.createSeries(ChartView.SeriesTypeArea, "Phase", chartView.axisX, chartView.axisY);
        //curLineSeries.useOpenGL = true;
        curLineSeries.borderWidth = plotLineWidth;
        curLineSeries.borderColor = phaseTransitions[phaseIdx].markerColor1;
        curLineSeries.color = phaseTransitions[phaseIdx].markerColor1;
        curLineSeries.color.a = plotLineAlpha;

        isPressuerLimiting = false;
    }

    function slotTest()
    {
        /*chartView.grabToImage(function(result) {
            image.source = result.url;
            chartView.opacity = 0.1;
        });*/
    }

    function slotAddData(phaseIdx, x, y)
    {
        phaseIdx = parseInt(phaseIdx);
        x = parseInt(x);
        y = parseInt(y);

        console.log("AddData: x=" + x + ", time=" + Util.getMinimisedDurationStr(Util.millisecToDurationStr(x)) + ", visible.x=" + plotArea.contentX + ", visible.w=" + plotArea.contentWidth);

        if (phaseTransitions[0].transitionType === "START")
        {
            phaseTransitions[0].transitionType = "STARTED";
        }

        if (curLineSeries === null)
        {
            return;
        }

        curPhaseIdx = phaseIdx;
        xLast = x;

        var xNew = x - xMin;

        // Apply precision control
        var y1 = Math.round(y / precisionCtrlConst);
        var yNew = Math.round(y / precisionCtrlConst) * precisionCtrlConst;
        yNew = Util.getPressure(pressureUnit, yNew);

        // Set minimum plot value: make sure line is visible above x-axis (this is hack to resolve QML bug)
        yNew = Math.max(yNew, 1);

        dataPoints.push({"x": xNew, "y": yNew});

        var drawRangeBuffer = 1000 * 60; //ms
        var drawRangeStart = plotArea.contentX - drawRangeBuffer;
        var drawRangeEnd = plotArea.contentX + plotArea.contentWidth + drawRangeBuffer;

        var drawOk = false;

        if ( (drawRangeStart <= xNew) &&
             (xNew <= drawRangeEnd) )
        {
            drawOk = true;
        }

        /*if (!drawOk)
        {
            return;
        }*/

        // Add data point to series
        yLast = yNew;

        // Two ways to add data:
        // (1)append data to series
        // (2)if previous two data points are same, replace previous one to current one
        var series = curLineSeries.upperSeries;
        if ( (series.count > 2) &&
             (yNew === series.at(series.count - 1).y) &&
             (yNew === series.at(series.count - 2).y) )
        {
            //console.log("ADD DATA: replacing previous point with current one");
            series.replace(series.count - 1, xNew, yNew);
        }
        else
        {
            series.append(xNew, yNew);
        }
        rescale();
    }

    function slotAddUserPausedData(phaseIdx, x, y, pausedMs)
    {
        phaseIdx = parseInt(phaseIdx);
        x = parseInt(x);
        y = parseInt(y);
        pausedMs = parseInt(pausedMs);

        //console.log("addUserPausedData: pausedMs=" + pausedMs);

        // Update phase transition positions
        if (reviewMode)
        {
            // Don't need to update new phase positions - They are set during init
        }
        else
        {
            for (var transitionIdx = phaseIdx + 1; transitionIdx < phaseTransitions.length; transitionIdx++)
            {
                phaseTransitions[transitionIdx].plotX += pausedMs;
            }
        }

        // Update reminder positions
        for (var reminderIdx = 0; reminderIdx < reminderIconObjects.length; reminderIdx++)
        {
            if (reminderIconObjects[reminderIdx].xValue > x)
            {
                reminderIconObjects[reminderIdx].xValue += pausedMs;
            }
        }

        slotAddData(phaseIdx, x, y);
    }

    function slotAddReminder(x)
    {
        //console.log("AddReminder()");
        if (reminderIconComponent === null)
        {
            console.error("Reminder icon component is not ready");
            return;
        }

        var reminderIconObject = reminderIconComponent.createObject(plot, { "xValue": x });
        if (reminderIconObject === null)
        {
            console.error("Failed to create ReminderIcon object");
        }
        else
        {
            reminderIconObjects.push(reminderIconObject);
        }
    }

    function getPhaseTransition(type, x, color1, color2)
    {
        //console.log("GetPhaseTransition()");
        if (phaseTransitionComponent === null)
        {
            console.warn("phaseTransitionComponent component is not ready");
            return;
        }

        var phaseTransitionObject = phaseTransitionComponent.createObject(plot, { "transitionType": type, "plotX": x, "plotY": yLast, "markerColor1": color1, "markerColor2": color2 });

        if (phaseTransitionObject === null)
        {
            console.error("Failed to create phaseTransitionComponent object");
        }

        return phaseTransitionObject;
    }


    function addUserPauseResumePhaseTransition(type, x, color1, color2)
    {
        var object = getPhaseTransition(type, x, color1, color2);
        if (object !== null)
        {
            userPauseResumeTransitionObjects.push(object);
        }
    }

    function addPressureLimitTransition(type, x)
    {
        var object = getPhaseTransition(type, x, colorMap.red);
        if (object !== null)
        {
            pressureLimitTransitionObjects.push(object);
        }
    }

    function rescale()
    {
        if (reviewMode)
        {
            return;
        }

        var i;
        var curPhaseStartX = phaseTransitions[curPhaseIdx].plotX;
        var nextPhaseStartX = phaseTransitions[curPhaseIdx + 1].plotX;

        if (xLast < curPhaseStartX)
        {
            // data entered too early - move current phase transition line to left
            var lagX = curPhaseStartX - xLast;
            //console.log("PLOT: RESCALE: Lag Phase" + curPhaseIdx + ": xLast=" + xLast + ", curPhaseStartX=" + curPhaseStartX + ", reduce x-axis by " + lagX);

            for (i = curPhaseIdx; i < phaseTransitions.length; i++)
            {
                phaseTransitions[i].plotX -= lagX;
            }
        }
        else if (xLast > nextPhaseStartX)
        {
            // data entered too early - move next phase transition lines to right
            var ledX = xLast - nextPhaseStartX;
            //console.log("PLOT: RESCALE: Led Phase" + curPhaseIdx + ": xLast=" + xLast + ", nextPhaseStartX=" + nextPhaseStartX + ", expand x-axis by " + ledX);

            for (i = curPhaseIdx + 1; i < phaseTransitions.length; i++)
            {
                phaseTransitions[i].plotX += ledX;
            }
        }

        if (reviewMode)
        {
            return;
        }

        var xMaxNew = xLast + xSpace;
        var range = xMax - xMin;

        if (xMaxNew > xMax)
        {
            xMax = xMaxNew;
            xMinPrev = xMin;
            xMin = xMax - xRangeDefault;
            reloadAxis();
        }
    }

    function reloadAxis()
    {
        for (var seriesIdx = 0; seriesIdx < chartView.count; seriesIdx++)
        {
            var series = chartView.series(seriesIdx).upperSeries;

            // Relocate each data points - scroll effect
            for (var pt = 0; pt < series.count; pt++)
            {
                var data = series.at(pt);
                var newDataX = data.x + xMinPrev - xMin;
                //console.log("SERIES[" + seriesIdx + "]: xMinPrev=" + xMinPrev + ", xMin= " + xMin + ", DATA[" + pt + "]: x=" + data.x + " -> " + newDataX + ", y=" + data.y);
                series.replace(pt, newDataX, data.y);
            }

            // Remove data points if required: next data point is before x=0
            var removeCount = 0;
            for (pt = 1; pt < series.count; pt++)
            {
                if (series.at(pt).x < 0)
                {
                    //console.log("SERIES[" + seriesIdx + "]: DATA[" + (pt - 1) + "]: x=" + series.at(pt - 1).x + ", DATA[" + pt + "]: x=" + series.at(pt).x + " -> DELETE");
                    removeCount++;
                }
            }

            if (removeCount > 0)
            {
                //console.log("SERIES[" + seriesIdx + "]: deleting " + removeCount + " points " + " COUNT=" + series.count);
                chartView.series(seriesIdx).upperSeries.removePoints(0, removeCount);
            }
        }

        // Remove empty series
        for (seriesIdx = 0; seriesIdx < chartView.count; seriesIdx++)
        {
            if (chartView.series(seriesIdx).upperSeries.count === 0)
            {
                //console.log("SERIES[" + seriesIdx + "]: Empty -> REMOVED");
                chartView.removeSeries(chartView.series(seriesIdx));
                seriesIdx--;
            }
        }

        // Remove unused Reminder objects
        for (var reminderIconIdx = 0; reminderIconIdx < reminderIconObjects.length; reminderIconIdx++)
        {
            if (reminderIconObjects[reminderIconIdx].plotX < xMin)
            {
                //console.log("REMINDER_ICON[" + reminderIconIdx + "].x < xMin -> REMOVED");
                reminderIconObjects[reminderIconIdx].destroy();
                reminderIconObjects.splice(reminderIconIdx, 1);
                reminderIconIdx--;
            }
        }

        // Remove unused PauseResumeTransition objects
        for (var userPauseResumeTransitionIdx = 0; userPauseResumeTransitionIdx < userPauseResumeTransitionObjects.length; userPauseResumeTransitionIdx++)
        {
            if (userPauseResumeTransitionObjects[userPauseResumeTransitionIdx].plotX < xMin)
            {
                //console.log("USER_PAUSE_RESUME[" + userPauseResumeTransitionIdx + "].x < xMin -> REMOVED");
                userPauseResumeTransitionObjects[userPauseResumeTransitionIdx].destroy();
                userPauseResumeTransitionObjects.splice(userPauseResumeTransitionIdx, 1);
                userPauseResumeTransitionIdx--;
            }
        }

        // Remove unused PressureLimitTransition objects
        for (var pressureLimitTransitionIdx = 0; pressureLimitTransitionIdx < pressureLimitTransitionObjects.length; pressureLimitTransitionIdx++)
        {
            if (pressureLimitTransitionObjects[pressureLimitTransitionIdx].plotX < xMin)
            {
                //console.log("PRESSURE_LIMIT[" + pressureLimitTransitionIdx + "].x < xMin -> REMOVED");
                pressureLimitTransitionObjects[pressureLimitTransitionIdx].destroy();
                pressureLimitTransitionObjects.splice(pressureLimitTransitionIdx, 1);
                pressureLimitTransitionIdx--;
            }
        }


        xAxisGrids.model = (xMax - xMin) / xAxisGridInterval;
    }

    Component.onCompleted: {
        phaseTransitionComponent = Qt.createComponent("PhaseTransition.qml");
        userPauseResumeTransitionObjects = [];
        pressureLimitTransitionObjects = [];

        // Initialise ReminderIcon variables
        reminderIconComponent = Qt.createComponent("ReminderIcon.qml");
        reminderIconObjects = [];

        // Initiliase phase transitions.
        phaseTransitions = [];
        phaseTransitions.push(getPhaseTransition("START", 0, "white", "white"));
        phaseTransitions.push(getPhaseTransition("NORMAL", 0, "white", "white"));
        phaseTransitions.push(getPhaseTransition("NORMAL", 0, "white", "white"));
        phaseTransitions.push(getPhaseTransition("NORMAL", 0, "white", "white"));
        phaseTransitions.push(getPhaseTransition("NORMAL", 0, "white", "white"));
        phaseTransitions.push(getPhaseTransition("NORMAL", 0, "white", "white"));
        phaseTransitions.push(getPhaseTransition("END", 0, endTimeGridColor, endTimeGridColor));

        phaseTransitionTerminated = getPhaseTransition("TERMINATED", 0, colorMap.errText, colorMap.errText);

        slotSetMaxPressure(maxPressureKpa);
    }
}
