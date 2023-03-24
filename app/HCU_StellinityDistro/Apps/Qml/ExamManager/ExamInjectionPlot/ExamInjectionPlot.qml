import QtQuick 2.12
import QtCharts 2.2

import "../../Util.js" as Util
import "../../Widgets"

Item {
    id: root
    clip: true

    property bool reviewMode: false

    property int maxPhases: dsCapabilities.phaseCountMax
    property string pressureUnit: dsCfgGlobal.pressureUnit
    property int maxPressureKpa: 2000
    property int xRangeDefaultByFactory: 30000
    property int xRangeDefault: xRangeDefaultByFactory
    property int xScrollStartOffset: xRangeDefaultByFactory * 0.8 // x-axis width when scroll starts
    property double yWarningLevel: 300
    property string contrastColor: "GREEN"

    property int yMin: 0
    property int yMax: 350
    property int xMin: 0
    property int xMinPrev: 0
    property int xMax: xRangeDefault - xMin
    property int curPhaseIdx: 0
    property int xLast: 0
    property int yLast: 0
    property int precisionCtrlConst: 20 // Unit is kPa, Increase precision to gain performance
    property bool isPressuerLimiting: false

    // Some hardcoded plot geometry parameters
    property var plotAreaOffsetX: { "kpa" : -80, "psi" : -74, "kg/cm2" : -64 }
    property int plotAreaMarginTop: 85
    property int plotAreaMarginRight: 60
    property int plotAreaY: height * 0.05
    property int plotAreaH: plot.height - 52

    // Axis grid and label
    property int yAxisGridInterval: 100
    property int yAxisLabelInterval: 100

    property int xAxisGridInterval: xRangeDefault / 15
    property int xAxisLabelInterval: xAxisGridInterval * 5

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
    property double plotLineAlpha: 0.2
    property int majorGridWidth: 1
    property int minorGridWidth: 1
    property int axisLabelWidth: plotArea.width * 0.1
    property int axisLabelHeight: plotAreaH * 0.05
    property int markerWidth: plotArea.width * 0.022
    property var phaseTransitions: []
    property var phaseTransitionAborted
    property var curLineSeries: null

    property var groupDataCache: ({})
    property int groupDataCacheIdx: 0
    property int groupDataCacheSize: 0

    property int reviewMaxX: 0
    property int reviewMaxXPrev: 0
    property int reviewMinX: 0
    property int reviewMinXPrev: 0
    property int reviewXRangeMax: 0
    property int reviewPlotMultiplier: 15

    property var activeAlerts: dsAlert.activeAlerts
    property var noticeList: getCatheterLimitsNotices()
    property int animationNoticesMs: 500

    Item {
        id: plotBackground
        x: plotAxisYInfo.width
        y: plotAreaY

        // -------------------------------------
        // Y Axis Grids
        Repeater {
            id: yAxisGrids
            model: ((yMax - yMin) / yAxisGridInterval) + 1

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
            opacity: 0.3
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
        interactive: reviewMode && !updateReviewPlot
        clip: true

        property bool updateReviewPlot: false
        property bool forward

        onContentXChanged:
        {
            if (!updateReviewPlot)
            {
                if (((contentX + (width * 1.2)) > getXAxisPosFromVal(reviewMaxX)) && (reviewMaxX < reviewXRangeMax))
                {
                    reviewMaxXPrev = reviewMaxX;
                    reviewMinXPrev = reviewMinX;

                    reviewMinX = reviewMaxX - (xRangeDefault * 2);
                    reviewMaxX = Math.min(reviewMinX + (xRangeDefault * reviewPlotMultiplier), reviewXRangeMax);
                    updateReviewPlot = true;
                    forward = true;
                }
                else if ((contentX < getXAxisPosFromVal(reviewMinX)) && (reviewMinX > 0))
                {
                    reviewMaxXPrev = reviewMaxX;
                    reviewMinXPrev = reviewMinX;

                    reviewMinX = Math.max(reviewMinX - (xRangeDefault * reviewPlotMultiplier), 0);
                    reviewMaxX = Math.min(reviewMinX + (xRangeDefault * (reviewPlotMultiplier + 2)), reviewXRangeMax);
                    updateReviewPlot = true;
                    forward = false;
                }

                if (updateReviewPlot)
                {
                    cancelFlick();
                    slotSetIsLoading(true);

                    // run at next cycle so we can show loading gif (can't animate with current implementation)
                    timerSingleShot(10, function ()
                    {
                        slotClearPlot();

                        groupDataCacheIdx = 0;

                        plotReviewData(reviewMinX, reviewMaxX);

                        // updating is done
                        if (forward)
                        {
                            contentX = getXAxisPosFromVal(reviewMaxXPrev) - (width * 1.1);
                        }
                        else
                        {
                            contentX = getXAxisPosFromVal(reviewMinXPrev);
                        }

                        returnToBounds();

                        updateReviewPlot = false;

                        // there is slight jump after the new plot has been plotted. Keep the loading gif running so user is less confused about the jump
                        timerSingleShot(1000, function() {
                            slotSetIsLoading(false);
                        });
                    } );
                }
            }
        }

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
                        font.family: fontRobotoLight.name
                    }
                }
            }

            // -------------------------------------
            // Chart
            ChartView {
                id: chartView
                x: plotAreaOffsetX[pressureUnit]
                width: parent.width - x + plotAreaMarginRight
                height: parent.height
                antialiasing: true
                backgroundColor: "transparent"
                legend.visible: false

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
                    useOpenGL: true
                }
            }
        }
    }

    Item {
        id: catheterLimitsNotices
        x: plotAxisYInfo.width + 30
        y: plotAreaY - 10
        width: parent.width - x - 30
        visible: noticeList.length > 0

        Item {
            id: pressureLimitedNotice
            width: parent.width
            height: textNoticeIconPressure.height

            Text {
                id: textNoticeIconPressure
                width: 28
                height: width
                color: colorMap.text01
                font.family: fontAwesome.name
                font.pixelSize: height
                text: (visible && noticeList.length > 0) ? "\uf071" : ""
            }

            Text {
                height: font.pixelSize
                anchors.left: textNoticeIconPressure.right
                anchors.leftMargin: 10
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignLeft
                color: colorMap.text01
                font.pixelSize: textNoticeIconPressure.width
                font.family: fontRobotoLight.name
                wrapMode: Text.Wrap
                text: (textNoticeIconPressure.visible) ? translate(noticeList[0]) : ""
            }
        }

        Item {
            id: flowRateLimitedNotice
            anchors.top: pressureLimitedNotice.bottom
            anchors.topMargin: 10
            width: parent.width
            height: textNoticeIconFlowRate.height

            Text {
                id: textNoticeIconFlowRate
                width: 28
                height: width
                color: colorMap.text01
                font.family: fontAwesome.name
                font.pixelSize: height
                text: (visible && noticeList.length > 1) ? "\uf071" : ""
            }

            Text {
                height: font.pixelSize
                anchors.left: textNoticeIconFlowRate.right
                anchors.leftMargin: 10
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignLeft
                color: colorMap.text01
                font.pixelSize: textNoticeIconFlowRate.width
                font.family: fontRobotoLight.name
                wrapMode: Text.Wrap
                text: (textNoticeIconFlowRate.visible) ? translate(noticeList[1]) : ""
            }
        }

        SequentialAnimation {
            id: animationNotices
            NumberAnimation { target: catheterLimitsNotices; properties: 'opacity'; from: 1; to: 0; duration: animationNoticesMs }
            NumberAnimation { target: catheterLimitsNotices; properties: 'opacity'; from: 0; to: 1; duration: animationNoticesMs }
            NumberAnimation { target: catheterLimitsNotices; properties: 'opacity'; from: 1; to: 0; duration: animationNoticesMs }
            NumberAnimation { target: catheterLimitsNotices; properties: 'opacity'; from: 0; to: 1; duration: animationNoticesMs }
        }

        onVisibleChanged: {
            animationNotices.start()
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

        // -------------------------------------
        // Y Axis Info
        Text {
            id: yAxisInfo
            y: parent.height * 0.03
            x: plotAxisYInfo.width - (width * 1.15)
            width: axisLabelWidth
            height: axisLabelHeight
            color: fontColor
            font.family: fontRobotoBold.name
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: height * 0.9
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
            font.family: fontRobotoBold.name
            font.pixelSize: height * 1.1
            text: (pressureUnit == "kg/cm2") ? localeToFloatStr(yWarningLevel, 1) : localeToFloatStr(yWarningLevel, 0)
        }

        // -------------------------------------
        // Y Axis Grids and Labels
        Repeater {
            id: yAxisLabels
            model: ((yMax - yMin) / yAxisGridInterval) + 1
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
                    font.family: fontRobotoLight.name
                    text: {
                        var yAxisVal = getYAxisLabel(index);
                        if (yAxisVal !== "")
                        {
                            return localeToFloatStr(yAxisVal, 0);
                        }
                        return "";
                    }
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

    GenericComboBox {
        id: cmbZoom
        visible: reviewMode
        anchors.top: parent.top
        anchors.topMargin: parent.height * 0.01
        anchors.horizontalCenter: parent.horizontalCenter
        itemValueTextFontPixelSize: rowHeight * 0.4
        itemIconTextFontPixelSize: rowHeight * 0.3
        opacity: 0.6
        width: parent.width * 0.15
        rowHeight: parent.height * 0.1
        currentIndex: 0
        optionList: [
            { unit: "", icon: "\uf07e", value: "100%" },
            { unit: "", icon: "\uf07e", value: "150%" },
            { unit: "", icon: "\uf07e", value: "200%" }
        ]

        onSignalCurrentIndexChanged: {
            var curValue = getCurrentValue().value;

            if (curValue === "150%")
            {
                xRangeDefault = xRangeDefaultByFactory / 1.5;
            }
            else if (curValue === "200%")
            {
                xRangeDefault = xRangeDefaultByFactory / 2;
            }
            else
            {
                xRangeDefault = xRangeDefaultByFactory;
            }
            slotSetRange(xMin, xMax);
        }
    }

    LoadingGif {
        id: plotLoadingGif
    }

    onPressureUnitChanged: {
        if (pressureUnit === "kpa")
        {
            yAxisGridInterval = 200;
            yAxisLabelInterval = 400;
        }
        else if (pressureUnit === "psi")
        {
            yAxisGridInterval = 50;
            yAxisLabelInterval = 100;
        }
        else if (pressureUnit === "kg/cm2")
        {
            yAxisGridInterval = 2.5;
            yAxisLabelInterval = 5;
        }
        yMax = Util.getPressure(pressureUnit, 2500);
        slotSetMaxPressure(maxPressureKpa);
        yAxisInfo.text = pressureUnit;
    }

    function initializeReviewPlot()
    {
        // load initial portion of graph
        reviewMaxX = xRangeDefault * reviewPlotMultiplier;
        reviewMinX = 0;
        plotReviewData(reviewMinX, reviewMaxX);
    }

    function plotReviewData(startX, endX)
    {
        //logDebug("##plotReviewData : Plotting review data From: " + startX + " To: " + endX);
        while ((groupDataCacheIdx < groupDataCacheSize) &&
               (((groupDataCache[groupDataCacheIdx]["method"] === slotAddData) && (groupDataCache[groupDataCacheIdx]["args"][1] < endX)) || (groupDataCache[groupDataCacheIdx]["method"] !== slotAddData)))
        {
            if ((groupDataCache[groupDataCacheIdx]["method"] === slotAddData) && (groupDataCache[groupDataCacheIdx]["args"][1]) <= startX)
            {
                groupDataCacheIdx++;
            }

            if (groupDataCacheIdx >= groupDataCacheSize)
            {
                break;
            }

            // just save maximum value from set range.
            // DO NOT CALL slotSetRange as review data's slotSetRange contains full range. Only portion of the graph should be populated due to cut off issues
            if (groupDataCache[groupDataCacheIdx]["method"] === slotSetRange)
            {
                reviewXRangeMax = groupDataCache[groupDataCacheIdx]["args"][1];
                reviewMaxX = Math.min(reviewXRangeMax, reviewMaxX);
                /*if (reviewMaxX !== endX)
                {
                    logDebug("##plotReviewData : End position adjusted as it exceeds max range. From: " + endX + " To: " + reviewMaxX);
                }*/
                endX = reviewMaxX;
            }
            else
            {
                groupDataCache[groupDataCacheIdx]["method"].apply(this, groupDataCache[groupDataCacheIdx]["args"]);
            }

            groupDataCacheIdx++;
        }

        slotSetRange(startX, endX);
    }

    function slotSetIsLoading(isLoading)
    {
        plotLoadingGif.show(isLoading);
    }

    function slotSetRange(xMin_, xMax_)
    {
        xMin = xMin_;
        xMax = xMax_;
        valueAxisX.min = xMin;
        xMax = Math.max(xMax, xRangeDefault);
        valueAxisX.max = xMax;
        plot.width = (plotArea.width * (xMax - xMin)) / xRangeDefault;

        if (reviewMode)
        {
            xAxisGrids.model = (xMax - xMin) / xAxisGridInterval;
        }
        else
        {
            reloadAxis();
        }
    }

    function slotClearPlot()
    {
        isPressuerLimiting = false;
        chartView.removeAllSeries();
        curLineSeries = null;
        xMin = 0;
        xMax = xMin + xRangeDefault;
        xLast = 0;
        slotSetRange(xMin, xMax);

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
        phaseTransitionAborted.enabled = false;
    }

    function slotResetData()
    {
        groupDataCacheSize = 0;
        groupDataCacheIdx = 0;
        groupDataCache = {}
        reviewMaxX = 0;
        reviewMinX = 0;
        reviewXRangeMax = 0;
    }

    function slotReset()
    {
        slotResetData();
        slotClearPlot();
    }

    function slotSetContrastColor(newContrastColor)
    {
        contrastColor = newContrastColor;
    }

    function slotSetMaxPressure(newMaxPressureKpa)
    {
        newMaxPressureKpa = parseInt(newMaxPressureKpa);
        maxPressureKpa = newMaxPressureKpa;
        yWarningLevel = Util.getPressure(pressureUnit, maxPressureKpa);
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
        //logDebug("index" + (index * yAxisGridInterval) + ": startPos=" + startPos);
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

        return Math.round(pos);
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
            phaseTransitionAborted.enabled = false;
        }
        else
        {
            phaseTransitionAborted.markerColor1 = colorMap.errText;
            phaseTransitionAborted.markerColor2 = colorMap.errText;

            phaseTransitionAborted.enabled = true;
            phaseTransitionAborted.plotX = x;
        }
    }

    function slotSetPhaseSkipped(phaseIdx)
    {
        // Set icon
        phaseIdx = parseInt(phaseIdx);
        logDebug("ExamInjectionPlot: slotSetPhaseSkipped(): phase" + phaseIdx + ": skipped");
        phaseTransitions[phaseIdx].transitionType = "SKIPPED";
        curPhaseIdx = phaseIdx;
        rescale();
    }

    function slotAddUserPauseSeries(x)
    {
        x = parseInt(x);

        //logDebug("Adding UserPauseSeries: color=" + colorMap.paused);
        addUserPauseResumePhaseTransition("PAUSE", x, colorMap.paused, colorMap.paused);

        curLineSeries = chartView.createSeries(ChartView.SeriesTypeArea, "UserPaused", chartView.axisX, chartView.axisY);
        curLineSeries.useOpenGL = true;
        curLineSeries.borderWidth = plotLineWidth;
        curLineSeries.borderColor = colorMap.paused;
        curLineSeries.color = isPressuerLimiting ? colorMap.red : colorMap.paused;
        curLineSeries.color.a = plotLineAlpha;
    }

    function slotAddUserResumeSeries(phaseIdx, x)
    {
        phaseIdx = parseInt(phaseIdx);
        x = parseInt(x);

        //logDebug("Adding UserResumeSeries: color=" + phaseTransitions[phaseIdx].markerColor1);
        addUserPauseResumePhaseTransition("RESUME", x, phaseTransitions[phaseIdx].markerColor1, phaseTransitions[phaseIdx].markerColor2);

        curLineSeries = chartView.createSeries(ChartView.SeriesTypeArea, "Phase", chartView.axisX, chartView.axisY);
        curLineSeries.useOpenGL = true;
        curLineSeries.borderWidth = plotLineWidth;
        curLineSeries.borderColor = phaseTransitions[phaseIdx].markerColor1;
        curLineSeries.color = isPressuerLimiting ? colorMap.red : phaseTransitions[phaseIdx].markerColor1;
        curLineSeries.color.a = plotLineAlpha;
        slotAddData(phaseIdx, x, yLast);
    }

    function slotAddPhaseSeries(phaseType, contrastPercentage)
    {
        contrastPercentage = parseInt(contrastPercentage);

        var lineColors = getLineColors(phaseType, contrastPercentage);

        //logDebug("Adding PhaseSeries: color=" + lineColors);
        curLineSeries = chartView.createSeries(ChartView.SeriesTypeArea, "Phase", chartView.axisX, chartView.axisY);
        curLineSeries.useOpenGL = true;
        curLineSeries.borderWidth = plotLineWidth;
        curLineSeries.borderColor = lineColors[0];
        curLineSeries.color = isPressuerLimiting ? colorMap.red : lineColors[0];
        curLineSeries.color.a = plotLineAlpha;
    }

    function slotAddPressureLimitStartSeries(phaseIdx, x, userPaused)
    {
        phaseIdx = parseInt(phaseIdx);
        x = parseInt(x);
        userPaused = (String(userPaused) === "true");

        //logDebug("Adding PressureLimitStart: phaseIdx=" + phaseIdx + ", x=" + x + ", userPaused=" + userPaused + ", yLast=" + yLast);
        addPressureLimitTransition("PRESSURE_LIMIT_START", x);
        isPressuerLimiting = true;

        // Resume current phase / paused
        curLineSeries = chartView.createSeries(ChartView.SeriesTypeArea, "PressureLimitStart", chartView.axisX, chartView.axisY);
        curLineSeries.useOpenGL = true;
        curLineSeries.borderWidth = plotLineWidth;
        curLineSeries.borderColor = userPaused ? colorMap.paused : phaseTransitions[phaseIdx].markerColor1;
        curLineSeries.color = colorMap.red;
        curLineSeries.color.a = plotLineAlpha;
        slotAddData(phaseIdx, x, yLast);
    }

    function slotAddPressureLimitEndSeries(phaseIdx, x, userPaused)
    {
        phaseIdx = parseInt(phaseIdx);
        x = parseInt(x);
        userPaused = (String(userPaused) === "true");

        //logDebug("Adding PressureLimitEnd: phaseIdx=" + phaseIdx + ", x=" + x + ", userPaused=" + userPaused + ", yLast=" + yLast);
        addPressureLimitTransition("PRESSURE_LIMIT_END", x);
        isPressuerLimiting = false;

        curLineSeries = chartView.createSeries(ChartView.SeriesTypeArea, userPaused ? "UserPaused" : "Phase", chartView.axisX, chartView.axisY);
        curLineSeries.useOpenGL = true;
        curLineSeries.borderWidth = plotLineWidth;
        curLineSeries.borderColor = userPaused ? colorMap.paused : phaseTransitions[phaseIdx].markerColor1;
        curLineSeries.color = userPaused ? colorMap.paused : phaseTransitions[phaseIdx].markerColor1;
        curLineSeries.color.a = plotLineAlpha;
        slotAddData(phaseIdx, x, yLast);
    }

    function slotCachePlotData(func)
    {
        groupDataCache[groupDataCacheSize] = {};

        groupDataCache[groupDataCacheSize]["method"] = func
        groupDataCache[groupDataCacheSize]["args"] = [];

        for (var i = 1; i < arguments.length; i++)
        {
            groupDataCache[groupDataCacheSize]["args"].push(arguments[i]);
        }

        // All data received
        if (func === slotSetTerminatedTime)
        {
            initializeReviewPlot();
        }

        groupDataCacheSize++;
    }

    function slotAddData(phaseIdx, x, y)
    {
        phaseIdx = parseInt(phaseIdx);
        x = parseInt(x);
        y = parseInt(y);

        //logDebug("Adding Data: phaseIdx=" + phaseIdx + ", x=" + x + ", y=" + y);

        if (phaseTransitions[0].transitionType === "START")
        {
            phaseTransitions[0].transitionType = "STARTED";
        }

        if (curLineSeries === null)
        {
            return;
        }

        curPhaseIdx = phaseIdx;

        // Ensure the new x is greater than xLast (no Back To The Future problem)
        x = Math.max(x, xLast);

        // Save current x, y values
        xLast = x;
        yLast = y;

        var xNew = x - xMin;

        // Apply precision control
        var y1 = Math.round(y / precisionCtrlConst);
        var yNew = Math.round(y / precisionCtrlConst) * precisionCtrlConst;

        yNew = Util.getPressure(pressureUnit, yNew);

        // HACK: Set minimum plot value: make sure line is visible above x-axis (this is hack to resolve QML bug)
        var yMin = Util.getPressure(pressureUnit, 2);
        yNew = Math.max(yNew, yMin);

        // Two ways to add data:
        // (1)append data to series
        // (2)if previous two data points are same, replace previous one to current one
        var series = curLineSeries.upperSeries;
        if ( (series.count > 2) &&
             (yNew === series.at(series.count - 1).y) &&
             (yNew === series.at(series.count - 2).y) )
        {
            //logDebug("ADD DATA: replacing previous point with current one");
            series.replace(series.count - 1, xNew, yNew);
        }
        else
        {
            series.append(xNew, yNew);
        }

        rescale();
    }

    function slotAdjustPhaseDuration(phaseIdx, adjustedTimeMs)
    {
        if (reviewMode)
        {
            return;
        }

        phaseIdx = parseInt(phaseIdx);
        adjustedTimeMs = parseInt(adjustedTimeMs);

        //logDebug("slotAdjustPhaseDuration: phaseIdx=" + phaseIdx + ", adjustedTimeMs=" + adjustedTimeMs);

        for (var transitionIdx = phaseIdx + 1; transitionIdx < phaseTransitions.length; transitionIdx++)
        {
            phaseTransitions[transitionIdx].plotX += adjustedTimeMs;
        }
    }

    function slotAddUserPausedData(phaseIdx, x, y, pausedMs)
    {
        phaseIdx = parseInt(phaseIdx);
        x = parseInt(x);
        y = parseInt(y);
        pausedMs = parseInt(pausedMs);

        //logDebug("addUserPausedData: pausedMs=" + pausedMs);

        // Update phase transition positions
        if (reviewMode)
        {
            // Don't need to update new phase positions - They are set during init
        }
        else
        {
            // Update position of phase transitions
            for (var transitionIdx = phaseIdx + 1; transitionIdx < phaseTransitions.length; transitionIdx++)
            {
                phaseTransitions[transitionIdx].plotX += pausedMs;
            }

            // Update reminder positions
            var xPosX = getXAxisPosFromVal(x);
            for (var reminderIdx = 0; reminderIdx < reminderIconObjects.length; reminderIdx++)
            {
                var reminderPosX = reminderIconObjects[reminderIdx].x + reminderIconObjects[reminderIdx].width;

                //logDebug("slotAddUserPausedData: [" + reminderIdx + "]: xPosX=" + xPosX + ", reminderPosX=" + reminderPosX + ", xValue=" + reminderIconObjects[reminderIdx].xValue);

                if (reminderPosX >= xPosX)
                {
                    // Reminder is not expired yet. Move the reminder later.
                    reminderIconObjects[reminderIdx].xValue += pausedMs;
                }
                else
                {
                    // Reminder is already expired. Dont' move.
                }
            }
        }

        slotAddData(phaseIdx, x, y);
    }

    function slotAddReminder(x)
    {
        //logDebug("AddReminder()");
        if (reminderIconComponent === null)
        {
            logError("Reminder icon component is not ready");
            return;
        }

        var reminderIconObject = reminderIconComponent.createObject(plot, { "xValue": x });
        if (reminderIconObject === null)
        {
            logError("Failed to create ReminderIcon object");
        }
        else
        {
            reminderIconObjects.push(reminderIconObject);
        }
    }

    function getPhaseTransition(type, x, color1, color2)
    {
        //logDebug("GetPhaseTransition()");
        if (phaseTransitionComponent === null)
        {
            logWarning("phaseTransitionComponent component is not ready");
            return;
        }

        var phaseTransitionObject = phaseTransitionComponent.createObject(plot, { "transitionType": type, "plotX": x, "plotY": yLast, "markerColor1": color1, "markerColor2": color2 });

        if (phaseTransitionObject === null)
        {
            logError("Failed to create PhaseTransition object");
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
        var object = getPhaseTransition(type, x, colorMap.red, colorMap.red);  //todo check
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

        var phaseIdx;
        var curPhaseStartX = phaseTransitions[curPhaseIdx].plotX;
        var nextPhaseStartX = phaseTransitions[curPhaseIdx + 1].plotX;

        if (xLast < curPhaseStartX)
        {
            // data entered too early - move current phase transition line to left
            var lagX = curPhaseStartX - xLast;
            //logDebug("PLOT: RESCALE: Lag Phase" + curPhaseIdx + ": xLast=" + xLast + ", curPhaseStartX=" + curPhaseStartX + ", reduce x-axis by " + lagX);

            for (phaseIdx = curPhaseIdx; phaseIdx < phaseTransitions.length; phaseIdx++)
            {
                phaseTransitions[phaseIdx].plotX -= lagX;
                //logDebug("lagX=" + lagX + ", phaseTransitions[" + phaseIdx + "].plotX=" + phaseTransitions[phaseIdx].plotX)
            }
        }
        else if (xLast > nextPhaseStartX)
        {
            // data entered too early - move next phase transition lines to right
            var ledX = xLast - nextPhaseStartX;
            //logDebug("PLOT: RESCALE: Led Phase" + curPhaseIdx + ": xLast=" + xLast + ", nextPhaseStartX=" + nextPhaseStartX + ", expand x-axis by " + ledX);

            for (phaseIdx = curPhaseIdx + 1; phaseIdx < phaseTransitions.length; phaseIdx++)
            {
                phaseTransitions[phaseIdx].plotX += ledX;
                //logDebug("ledX=" + ledX + ", phaseTransitions[" + phaseIdx + "].plotX=" + phaseTransitions[phaseIdx].plotX)
            }
        }

        var xMaxNew = xLast + (xRangeDefaultByFactory - xScrollStartOffset);
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
                //logDebug("SERIES[" + seriesIdx + "]: xMinPrev=" + xMinPrev + ", xMin= " + xMin + ", DATA[" + pt + "]: x=" + data.x + " -> " + newDataX + ", y=" + data.y);
                series.replace(pt, newDataX, data.y);
            }

            // Remove data points if required: next data point is before x=0
            var removeCount = 0;
            for (pt = 1; pt < series.count; pt++)
            {
                if (series.at(pt).x < 0)
                {
                    //logDebug("SERIES[" + seriesIdx + "]: DATA[" + (pt - 1) + "]: x=" + series.at(pt - 1).x + ", DATA[" + pt + "]: x=" + series.at(pt).x + " -> DELETE");
                    removeCount++;
                }
            }

            if (removeCount > 0)
            {
                //logDebug("SERIES[" + seriesIdx + "]: deleting " + removeCount + " points " + " COUNT=" + series.count);
                chartView.series(seriesIdx).upperSeries.removePoints(0, removeCount);
            }
        }

        // Remove empty series
        for (seriesIdx = 0; seriesIdx < chartView.count; seriesIdx++)
        {
            if (chartView.series(seriesIdx).upperSeries.count === 0)
            {
                //logDebug("SERIES[" + seriesIdx + "]: Empty -> REMOVED");
                chartView.removeSeries(chartView.series(seriesIdx));
                seriesIdx--;
            }
        }

        // Remove unused Reminder objects
        for (var reminderIconIdx = 0; reminderIconIdx < reminderIconObjects.length; reminderIconIdx++)
        {
            if (reminderIconObjects[reminderIconIdx].plotX < xMin)
            {
                //logDebug("REMINDER_ICON[" + reminderIconIdx + "].x < xMin -> REMOVED");
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
                //logDebug("USER_PAUSE_RESUME[" + userPauseResumeTransitionIdx + "].x < xMin -> REMOVED");
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
                //logDebug("PRESSURE_LIMIT[" + pressureLimitTransitionIdx + "].x < xMin -> REMOVED");
                pressureLimitTransitionObjects[pressureLimitTransitionIdx].destroy();
                pressureLimitTransitionObjects.splice(pressureLimitTransitionIdx, 1);
                pressureLimitTransitionIdx--;
            }
        }
        xAxisGrids.model = (xMax - xMin) / xAxisGridInterval;
    }

    function getCatheterLimitsNotices()
    {
        var newNoticeList = [];
        var hasCatheterLimits = false;
        var flowRateLimitedByCatheterType = false;
        var pressureLimitedByCatheterType = false;
        var flowRateLimitedByInjectionSite = false;

        for (var alertIdx = 0; alertIdx < activeAlerts.length; alertIdx++)
        {
            if (activeAlerts[alertIdx].CodeName === "ArmedInjectionAboveCatheterLimits")
            {
                hasCatheterLimits = true;
                break;
            }
        }

        if ( (executingStep !== undefined) && (hasCatheterLimits) )
        {
            for ( var noticeIdx = 0 ; noticeIdx < executingStep.PersonalizationNotices.length ; noticeIdx++ )
            {
                if (executingStep.PersonalizationNotices[noticeIdx].Name === "FlowRateExceedsCatheterPlacementLimit")
                {
                    flowRateLimitedByInjectionSite = true;
                }
                else if (executingStep.PersonalizationNotices[noticeIdx].Name === "FlowRateExceedsCatheterTypeLimit")
                {
                    flowRateLimitedByCatheterType = true;
                }
                else if (executingStep.PersonalizationNotices[noticeIdx].Name === "PressureLimitExceedsCatheterTypeLimit")
                {
                    pressureLimitedByCatheterType = true;
                }
            }

            if (flowRateLimitedByCatheterType || pressureLimitedByCatheterType)
            {
                // T_PressureGraphNotice_PressureLimitExceedsCatheterTypeLimit or
                // T_PressureGraphNotice_FlowRateExceedsCatheterTypeLimit or
                // T_PressureGraphNotice_PressureLimitExceedsCatheterTypeLimit_FlowRateExceedsCatheterTypeLimit
                newNoticeList.push( "T_PressureGraphNotice" + (pressureLimitedByCatheterType ? "_PressureLimitExceedsCatheterTypeLimit" : "") + (flowRateLimitedByCatheterType ? "_FlowRateExceedsCatheterTypeLimit" : "") );
            }
            if (flowRateLimitedByInjectionSite)
            {
                newNoticeList.push("T_PressureGraphNotice_FlowRateExceedsCatheterPlacementLimit");
            }
        }
        return newNoticeList;
    }

    Component.onCompleted: {
        phaseTransitionComponent = Qt.createComponent("ExamInjectionPlotTransition.qml");
        userPauseResumeTransitionObjects = [];
        pressureLimitTransitionObjects = [];

        // Initialise ReminderIcon variables
        reminderIconComponent = Qt.createComponent("ExamInjectionPlotReminder.qml");
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

        phaseTransitionAborted = getPhaseTransition("ABORTED", 0, colorMap.errText, colorMap.errText);

        slotSetMaxPressure(maxPressureKpa);
    }
}
