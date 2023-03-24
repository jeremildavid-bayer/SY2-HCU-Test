import QtQuick 2.12

Item {
    // Data
    property string examGuid
    property bool isExamStarted: examGuid !== "00000000-0000-0000-0000-000000000000"
    property bool isPatientSelected: (examAdvanceInfo !== undefined) && (examAdvanceInfo.GUID !== "00000000-0000-0000-0000-000000000000")
    property string examProgressState: "Idle"
    property string examScreenState
    property var examAdvanceInfo
    property bool isAirCheckNeeded
    property var planTemplateGroups: []
    property bool planTemplateGroupsReady: false
    property var plan
    property var planPreview
    property var selectedContrast:  { ColorCode: "GREEN" }
    property string selectedSUDSLength
    property var stepProgressDigest
    property var executedSteps: []
    property var executingStep
    property bool isContrastSelectAllowed
    property var reminders: []
    property string defaultInjectPlanTemplateGuid
    property var scannerInterlocks
    property var injectionRequestProcessStatus
    property int injectionStartedElapsedSec: 0
    property int injectionCompletedElapsedSec: 0
    property int injectionResumedElapsedSec: 0

    // Function from QML to CPP
    function slotAirCheckDone() { return dsExamCpp.slotAirCheckDone(); }
    function slotScannerInterlocksCheckDone() { return dsExamCpp.slotScannerInterlocksCheckDone(); }
    function slotDefaultPlanTemplateSelected() { return dsExamCpp.slotDefaultPlanTemplateSelected(); }
    function slotInjectionArmed() { return dsExamCpp.slotInjectionArmed(); }
    function slotInjectionStarted() { return dsExamCpp.slotInjectionStarted(); }
    function slotInjectionPaused() { return dsExamCpp.slotInjectionPaused(); }
    function slotInjectionAborted() { return dsExamCpp.slotInjectionAborted(); }
    function slotInjectionSkipped() { return dsExamCpp.slotInjectionSkipped(); }
    function slotInjectionRepeat() { return dsExamCpp.slotInjectionRepeat(); }
    function slotInjectionFlowAdjusted(up) { return dsExamCpp.slotInjectionFlowAdjusted(up); }
    function slotExamProgressStateChanged(examProgressState) { return dsExamCpp.slotExamProgressStateChanged(examProgressState); }
    function slotAdjustInjectionVolume() { return dsExamCpp.slotAdjustInjectionVolume(); }
    function slotPressureLimitChanged(pressureLimit) { return dsExamCpp.slotPressureLimitChanged(pressureLimit); }
    function slotContrastTypeChanged(brand, concentration) { return dsExamCpp.slotContrastTypeChanged(brand, concentration); }
    function slotSUDSLengthChanged(length) { return dsExamCpp.slotSUDSLengthChanged(length); }
    function slotPlanChanged(plan) { return dsExamCpp.slotPlanChanged(plan); }
    function slotPlanPreviewChanged(planPreview) { return dsExamCpp.slotPlanPreviewChanged(planPreview); }
    function slotLoadPlanTemplateFromPlan(plan) { return dsExamCpp.slotLoadPlanTemplateFromPlan(plan); }
    function slotLoadPlanFromPlanPreview(planTemplate) { return dsExamCpp.slotLoadPlanFromPlanPreview(planTemplate); }
    function slotUpdateReviewPlot(stepIndex) { return dsExamCpp.slotUpdateReviewPlot(stepIndex); }

    // QML Modules
    property var qmlReviewPlot: null
    property var qmlInjectionMonitor: null
    property var qmlInjectionMonitorOverview: null
    property var qmlInjectionPlot: null
    property var qmlReminders: null
    property var qmlExamManager: null

    // Function from CPP to QML
    function slotPlanChangedFromUnmodifiedDefault()
    {
        qmlExamManager.slotPlanChangedFromUnmodifiedDefault();
    }

    function slotInjectionMonitorUpdate(maxPressureKpa, isPressureLimiting, pressureKpa, flowRate, volContrast, volSaline)
    {
        if (qmlInjectionMonitorOverview != null)
        {
            qmlInjectionMonitorOverview.slotUpdate(maxPressureKpa, isPressureLimiting, pressureKpa, flowRate, volContrast, volSaline);
        }
    }

    function slotInjectionPlotSetIsLoading(mode, isLoading)
    {
        var qmlPlot = (mode === "PROGRESS") ? qmlInjectionPlot : qmlReviewPlot;
        qmlPlot.slotSetIsLoading(isLoading);
    }

    function slotInjectionPlotReset(mode)
    {
        var qmlPlot = (mode === "PROGRESS") ? qmlInjectionPlot : qmlReviewPlot;
        qmlPlot.slotReset();
    }

    function slotInjectionPlotSetRange(mode, xMin, xMax)
    {
        if (mode === "PROGRESS")
        {
            qmlInjectionPlot.slotSetRange(xMin, xMax);
        }
        else
        {
            qmlReviewPlot.slotCachePlotData(qmlReviewPlot.slotSetRange, xMin, xMax);
        }
    }

    function slotInjectionPlotSetContrastColor(mode, contrastColor)
    {
        if (mode === "PROGRESS")
        {
            qmlInjectionPlot.slotSetContrastColor(contrastColor);
        }
        else
        {
            qmlReviewPlot.slotCachePlotData(qmlReviewPlot.slotSetContrastColor, contrastColor);
        }
    }

    function slotInjectionPlotSetMaxPressure(mode, maxPressure)
    {
        if (mode === "PROGRESS")
        {
            qmlInjectionPlot.slotSetMaxPressure(maxPressure);
        }
        else
        {
            qmlReviewPlot.slotCachePlotData(qmlReviewPlot.slotSetMaxPressure, maxPressure);
        }
    }

    function slotInjectionPlotSetPhaseInfo(mode, phaseIdx, x, phaseType, contrastPercentage)
    {
        if (mode === "PROGRESS")
        {
            qmlInjectionPlot.slotSetPhaseInfo(phaseIdx, x, phaseType, contrastPercentage);
        }
        else
        {
            qmlReviewPlot.slotCachePlotData(qmlReviewPlot.slotSetPhaseInfo, phaseIdx, x, phaseType, contrastPercentage);
        }
    }

    function slotInjectionPlotSetEndTime(mode, phaseCount, x)
    {
        if (mode === "PROGRESS")
        {
            qmlInjectionPlot.slotSetEndTime(phaseCount, x);
        }
        else
        {
            qmlReviewPlot.slotCachePlotData(qmlReviewPlot.slotSetEndTime, phaseCount, x);
        }
    }

    function slotInjectionPlotSetTerminatedTime(mode, x, terminatedReason)
    {
        if (mode === "PROGRESS")
        {
            qmlInjectionPlot.slotSetTerminatedTime(x, terminatedReason);
        }
        else
        {
            qmlReviewPlot.slotCachePlotData(qmlReviewPlot.slotSetTerminatedTime, x, terminatedReason);
        }
    }

    function slotInjectionPlotAddReminder(mode, x)
    {
        if (mode === "PROGRESS")
        {
            qmlInjectionPlot.slotAddReminder(x);
        }
        else
        {
            qmlReviewPlot.slotCachePlotData(qmlReviewPlot.slotAddReminder, x);
        }
    }

    function slotInjectionPlotAdjustPhaseDuration(mode, phaseIdx, adjustedTimeMs)
    {
        if (mode === "PROGRESS")
        {
            qmlInjectionPlot.slotAdjustPhaseDuration(phaseIdx, adjustedTimeMs);
        }
        else
        {
            qmlReviewPlot.slotCachePlotData(qmlReviewPlot.slotAdjustPhaseDuration, phaseIdx, adjustedTimeMs);
        }
    }

    function slotInjectionPlotAddUserPausedData(mode, phaseIdx, x, y, pausedMs)
    {
        if (mode === "PROGRESS")
        {
            qmlInjectionPlot.slotAddUserPausedData(phaseIdx, x, y, pausedMs);
        }
        else
        {
            qmlReviewPlot.slotCachePlotData(qmlReviewPlot.slotAddUserPausedData, phaseIdx, x, y, pausedMs);
        }

        qmlReminders.slotAddUserPausedData(pausedMs);
    }

    function slotInjectionPlotAddData(mode, phaseIdx, x, y)
    {
        if (mode === "PROGRESS")
        {
            qmlInjectionPlot.slotAddData(phaseIdx, x, y);
        }
        else
        {
            qmlReviewPlot.slotCachePlotData(qmlReviewPlot.slotAddData, phaseIdx, x, y);
        }
    }

    function slotInjectionPlotAddPhaseSeries(mode, phaseType, contrastPercentage)
    {
        if (mode === "PROGRESS")
        {
            qmlInjectionPlot.slotAddPhaseSeries(phaseType, contrastPercentage);
        }
        else
        {
            qmlReviewPlot.slotCachePlotData(qmlReviewPlot.slotAddPhaseSeries, phaseType, contrastPercentage);
        }
    }

    function slotInjectionPlotAddUserResumeSeries(mode, phaseIdx, x)
    {
        if (mode === "PROGRESS")
        {
            qmlInjectionPlot.slotAddUserResumeSeries(phaseIdx, x);
        }
        else
        {
            qmlReviewPlot.slotCachePlotData(qmlReviewPlot.slotAddUserResumeSeries, phaseIdx, x);
        }
    }

    function slotInjectionPlotAddUserPauseSeries(mode, x)
    {
        if (mode === "PROGRESS")
        {
            qmlInjectionPlot.slotAddUserPauseSeries(x);
        }
        else
        {
            qmlReviewPlot.slotCachePlotData(qmlReviewPlot.slotAddUserPauseSeries, x);
        }
    }

    function slotInjectionPlotSetPhaseSkipped(mode, phaseIdx)
    {
        if (mode === "PROGRESS")
        {
            qmlInjectionPlot.slotSetPhaseSkipped(phaseIdx);
        }
        else
        {
            qmlReviewPlot.slotCachePlotData(qmlReviewPlot.slotSetPhaseSkipped, phaseIdx);
        }
    }

    function slotInjectionPlotAddPressureLimitStartSeries(mode, phaseIdx, x, userPaused)
    {
        if (mode === "PROGRESS")
        {
            qmlInjectionPlot.slotAddPressureLimitStartSeries(phaseIdx, x, userPaused);
        }
        else
        {
            qmlReviewPlot.slotCachePlotData(qmlReviewPlot.slotAddPressureLimitStartSeries, phaseIdx, x, userPaused);
        }
    }

    function slotInjectionPlotAddPressureLimitEndSeries(mode, phaseIdx, x, userPaused)
    {
        if (mode === "PROGRESS")
        {
            qmlInjectionPlot.slotAddPressureLimitEndSeries(phaseIdx, x, userPaused);
        }
        else
        {
            qmlReviewPlot.slotCachePlotData(qmlReviewPlot.slotAddPressureLimitEndSeries, phaseIdx, x, userPaused);
        }
    }

    Timer {
        id: pressureLimitSoundPlayTimer
        interval: 500
        repeat: true
        onTriggered: {
            soundPlayer.playInjPulse();
        }
    }

    function slotInjectionPlotPressureLimitingActive(mode, active)
    {
        if (mode === "PROGRESS")
        {
            if (active)
            {
                if (!pressureLimitSoundPlayTimer.running)
                {
                    pressureLimitSoundPlayTimer.start();
                }
            }
            else
            {
                pressureLimitSoundPlayTimer.stop();
            }
        }
    }
}
