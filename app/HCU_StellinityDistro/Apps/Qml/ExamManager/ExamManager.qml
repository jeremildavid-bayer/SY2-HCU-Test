import QtQuick 2.12
import "ExamPatientSelect"
import "ExamProtocolSelect"
import "ExamProtocolEdit"
import "ExamInjectionMonitor"
import "ExamSummary"
import "ExamProtocolOverview"
import "ExamNavigationBar"
import "ExamInjectionControlBar"
import "ExamReminders"
import "../Widgets"
import "../Widgets/Popup"
import "../Util.js" as Util

Rectangle {
    property var plan: dsExam.plan
    property string statePath: dsSystem.statePath
    property string lastStatePath: dsSystem.lastStatePath
    property string examProgressState: dsExam.examProgressState
    property string examScreenState: dsExam.examScreenState
    property string defaultInjectPlanTemplateGuid: dsExam.defaultInjectPlanTemplateGuid
    property var planPreview: dsExam.planPreview
    property var executedSteps: dsExam.executedSteps
    property var executingStep: dsExam.executingStep
    property var injectionRequestProcessStatus: dsExam.injectionRequestProcessStatus
    property int frameMargin: dsCfgLocal.screenW * 0.02
    property int rightFrameWidth: (dsCfgLocal.screenW * 0.23) + (frameMargin * 2)
    property int leftFrameWidth: (dsCfgLocal.screenW * 0.23) + (frameMargin * 2)
    property int leftFrameX: frameMargin
    property int middleFrameWidth: dsCfgLocal.screenW - rightFrameWidth - leftFrameWidth
    property int middleFrameX: leftFrameWidth
    property int examCompleteAnimationDurationMs: 800
    property var cruLinkStatus: dsCru.cruLinkStatus
    property string workflowPreloadProtocolState: dsWorkflow.workflowPreloadProtocolState
    property bool licenseEnabledPatientStudyContext: dsCru.licenseEnabledPatientStudyContext
    property bool isExamStarted: dsExam.isExamStarted
    property string examGuid: dsExam.examGuid
    property var examAdvanceInfo: dsExam.examAdvanceInfo

    id: examManager
    color: colorMap.mainBackground

    Item {
        id: rectMainContent
        width: parent.width
        height: parent.height - actionBarHeight

        ExamProtocolOverview {
            id: protocolOverview
            anchors.right: parent.right
            width: rightFrameWidth - frameMargin
            height: parent.height
        }

        ExamPatientSelect {
            id: patientSelect
        }
        ExamProtocolSelect {}
        ExamProtocolEdit {
            id: protocolEdit
        }
        ExamInjectionMonitor {}
        ExamSummary {}
    }

    ExamReminders {}

    ExamNavigationBar {
        id: navigationBar
        Component.onCompleted: {
            this.parent = frameActionBar;
        }
    }

    ExamInjectionControlBar {
        Component.onCompleted: {
            this.parent = frameActionBar;
        }
    }

    Timer {
        // Give few time to play animation for end exam
        id: examCompleteTimer
        interval: examCompleteAnimationDurationMs
        onTriggered: {
            dsExam.slotExamProgressStateChanged("Completed");
        }
    }

    Timer {
        id: injectionElapsedTimer
        interval: 1000
        repeat: true
        onTriggered: {
            dsExam.injectionStartedElapsedSec++;
            if ( (statePath != "Executing") &&
                 (statePath != "Busy/Finishing") &&
                 (statePath != "Busy/Holding") )
            {
                dsExam.injectionCompletedElapsedSec++;
            }

            if ( (statePath == "Executing") ||
                 (statePath == "Busy/Finishing") ) {
                dsExam.injectionResumedElapsedSec++;
            }
        }
    }
    onStatePathChanged: {
        if ( (lastStatePath == "Busy/Finishing") && (statePath == "Idle") )
        {
            if (executedSteps[executedSteps.length - 1].TerminationReason === "Normal")
            {
                // Normal injection complete
                soundPlayer.playInjComplete();
            }
            else
            {
                // Unexpected injection terminated
                soundPlayer.playDisarmStop();
            }

            dsExam.examScreenState = "ExamManager-ProtocolModification";
            appMain.setScreenState(dsExam.examScreenState);
            protocolEdit.openLastStepReview();
        }
        else if ( (lastStatePath == "Ready/Armed") && (statePath == "Idle") )
        {
            // Disarmed
            soundPlayer.playDisarmStop();
            dsExam.examScreenState = "ExamManager-ProtocolModification";
            appMain.setScreenState(dsExam.examScreenState);
        }
        else if ( (lastStatePath != "Ready/Armed") && (statePath == "Ready/Armed") )
        {
            injectionElapsedTimer.stop();
            soundPlayer.playArmed();
            dsExam.examScreenState = "ExamManager-InjectionExecution";
            dsExam.injectionStartedElapsedSec = 0;
            dsExam.injectionCompletedElapsedSec = 0;
            dsExam.injectionResumedElapsedSec = 0;
            appMain.setScreenState(dsExam.examScreenState);
        }
        else if ( (lastStatePath == "Ready/Armed") && (statePath == "Executing") )
        {
            injectionElapsedTimer.start();
        }
        else if ( (lastStatePath == "Busy/Holding") && (statePath == "Executing") )
        {
            dsExam.injectionResumedElapsedSec = 0;
        }

        // Update Loading Gif
        if ( (lastStatePath != "Idle") && (statePath == "Idle") )
        {
            appMain.setInteractiveState(true, "ExamManager: StatePath=" + lastStatePath + " to " + statePath);
        }
        else if ( (lastStatePath != "Busy/Finishing") && (statePath == "Busy/Finishing") )
        {
            appMain.setInteractiveState(false, "ExamManager: StatePath=" + lastStatePath + " to " + statePath);
        }

        // Screen snap for Error State
        if (statePath == "Error")
        {
            if ( (appMain.screenState.indexOf("DeviceManager-") >= 0) ||
                 (appMain.screenState.indexOf("ExamManager-") >= 0) )
            {
                appMain.setScreenState("Home");
            }
        }
    }

    onExamScreenStateChanged: {
        logDebug("ExamManager: ExamScreenState = " + examScreenState);
    }

    onExamProgressStateChanged: {
        logDebug("ExamManager: examProgress=" + examProgressState);

        if (examProgressState == "Idle")
        {
            logDebug("ExamManager: examProgressState=" + examProgressState + ": examScreenState=" + dsExam.examScreenState + " to Home");
            dsExam.examScreenState = "Home";
            if (appMain.screenState.indexOf("ExamManager-") >= 0)
            {
                logDebug("ExamManager: examProgressState=" + examProgressState + ": Setting screen state to " + dsExam.examScreenState);
                appMain.setScreenState(dsExam.examScreenState);
            }
        }
        else if (examProgressState == "Completing")
        {
            examCompleteTimer.start();
            injectionElapsedTimer.stop();
        }
        else if (examProgressState == "Started")
        {
            // HCU GUI doesn't allow starting exam while input pad / keyboard is open, however CRU can start exam.
            // If CRU starts exam while the inputpad is open, force close them
            if (widgetInputPad.isOpen())
            {
                widgetInputPad.close(false);
            }
            if (widgetKeyboard.isOpen())
            {
                widgetKeyboard.close(false);
            }
        }
        else
        {
            // ExamProgressState is not IDLE nor Completing but ExamScreenState is still HOME.
            if (examScreenState == "Home")
            {
                logDebug("ExamManager: examProgress=" + examProgressState + " while examScreenState = " + examScreenState + ": Seemed CRU started exam by selecting patient");
                dsExam.examScreenState = "ExamManager-ProtocolSelection";
            }
        }
    }

    onInjectionRequestProcessStatusChanged: {
        if (injectionRequestProcessStatus === undefined)
        {
            return;
        }

        if (workflowPreloadProtocolState !== "PRELOAD_PROTOCOL_STATE_READY")
        {
            return;
        }

        if ( (injectionRequestProcessStatus.State === "T_ARMING") ||
             (injectionRequestProcessStatus.State === "T_DISARMING") ||
             (injectionRequestProcessStatus.State === "T_INJECTSTARTING") )
        {
            appMain.setInteractiveState(false, "ExamManager: injectionRequestProcessStatus.State=" + injectionRequestProcessStatus.State);
        }
        else
        {
            appMain.setInteractiveState(true, "ExamManager: injectionRequestProcessStatus.State=" + injectionRequestProcessStatus.State);
        }
    }

    onLicenseEnabledPatientStudyContextChanged: {
        reload();
    }

    onVisibleChanged: {
        reload();
    }

    Component.onCompleted: {
        dsExam.qmlExamManager = this;
        navigationBar.signalNavNext.connect(slotNavNext);
        navigationBar.signalNavPatient.connect(slotNavPatient);
        navigationBar.signalNavProtocol.connect(slotNavProtocol);
        navigationBar.signalNavInjection.connect(slotNavInjection);
        navigationBar.signalNavSummary.connect(slotNavSummary);

        dsExam.examScreenState = screenState;

        appMain.screenStateChanged.connect(slotScreenStateChanged);
        slotScreenStateChanged();
    }

    Component.onDestruction: {
        navigationBar.signalNavNext.disconnect(slotNavNext);
        navigationBar.signalNavPatient.disconnect(slotNavPatient);
        navigationBar.signalNavProtocol.disconnect(slotNavProtocol);
        navigationBar.signalNavInjection.disconnect(slotNavInjection);
        navigationBar.signalNavSummary.disconnect(slotNavSummary);

        appMain.screenStateChanged.disconnect(slotScreenStateChanged);
    }

    function slotScreenStateChanged()
    {
        visible = (screenState.indexOf("ExamManager-") >= 0);
        reload();
    }

    function reload()
    {
        if (screenStatePrev == "Startup")
        {
            logDebug("ExamManager: Last ExamProgressState = " + examProgressState);
            var newExamScreenState;

            if ( (examProgressState == "InjectionExecution") ||
                 (examProgressState == "ProtocolModification") )
            {
                if (executedSteps.length >= plan.Steps.length)
                {
                    newExamScreenState = "ExamManager-SummaryConfirmation";
                }
                else
                {
                    newExamScreenState = "ExamManager-ProtocolModification";
                }
            }
            else if (examProgressState == "Idle")
            {
                newExamScreenState = "Home";
            }
            else
            {
                newExamScreenState = "ExamManager-" + examProgressState;
            }

            logDebug("ExamManager: Updated ExamScreenState = " + newExamScreenState);
            dsExam.examScreenState = newExamScreenState;
            return;
        }

        if ( (screenStatePrev !== "Home") &&
             (screenState === "Home") )
        {
            enterHomeScreen();
        }

        if (!visible)
        {
            return;
        }

        enterExamScreen();
    }

    function enterExamScreen()
    {
        if (examProgressState === "Idle")
        {
            // Exam is never started. Starting now.
            dsExam.slotExamProgressStateChanged("Prepared");
            dsExam.examScreenState = "ExamManager-PatientSelection";
        }

        if (examScreenState == "ExamManager-PatientSelection")
        {
            if (!licenseEnabledPatientStudyContext)
            {
                dsExam.examScreenState = "ExamManager-ProtocolSelection";
            }
        }

        appMain.setScreenState(examScreenState);
    }

    function enterHomeScreen()
    {
        if (examProgressState === "Idle")
        {
            dsExam.examScreenState = "Home";
        }
        // if PatientSelection state is incomplete, set screen state to patient selection state
        else if (licenseEnabledPatientStudyContext && ((examAdvanceInfo !== undefined && examAdvanceInfo.ExamInputsProgress !== 100) || !isExamStarted))
        {
            dsExam.examScreenState = "ExamManager-PatientSelection";
        }
        else if (executedSteps.length === plan.Steps.length)
        {
            // All injections are executed
            dsExam.examScreenState = "ExamManager-SummaryConfirmation";
        }
        else if (executedSteps.length > 0)
        {
            // At least injection is executed
            dsExam.examScreenState = "ExamManager-ProtocolModification";
        }
        else if (plan.Template !== defaultInjectPlanTemplateGuid)
        {
            // Plan is not default
            dsExam.examScreenState = "ExamManager-ProtocolModification";
        }
        else if ( (plan.Template === defaultInjectPlanTemplateGuid) && (plan.IsModifiedFromTemplate) )
        {
            // Plan is modified default
            dsExam.examScreenState = "ExamManager-ProtocolModification";
        }
    }

    function isSharingInformation(curPlan)
    {
        if (curPlan === undefined)
        {
            return false;
        }
        else if ( (curPlan.SharingInformation.Credits !== "") ||
             (curPlan.SharingInformation.Version !== "") ||
             (curPlan.SharingInformation.Url !== "") )
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    function sharingInformationPopup(curPlan)
    {
        popupManager.popupSharingInformation.titleText = curPlan.Name;
        popupManager.popupSharingInformation.credits = curPlan.SharingInformation.Credits;
        popupManager.popupSharingInformation.version = curPlan.SharingInformation.Version;
        popupManager.popupSharingInformation.url = curPlan.SharingInformation.Url;
        popupManager.popupSharingInformation.logo = getSharingInformationIcon(curPlan);
        popupManager.popupSharingInformation.open();
    }

    function getSharingInformationIcon(curPlan)
    {
        var imgSource = imageMap.examProtocolGlobe.replace(/Twilight/g, "Purity");
        if ((curPlan !== undefined) && curPlan.SharingInformation.Logo !== "")
        {
            var logoInfo = curPlan.SharingInformation.Logo.split(";");
            imgSource = "data:image/" + logoInfo[0] + ";base64," + logoInfo[1];
        }
        return imgSource;
    }

    function slotNavPatient()
    {
        if (examProgressState == "PatientSelection")
        {
            //dsExam.examScreenState = "ExamManager-PatientSelection";
        }
        else if ( (examProgressState === "Started") ||
                  (examProgressState === "ProtocolSelection") ||
                  (examProgressState === "ProtocolModification") ||
                  (examProgressState === "InjectionExecution") ||
                  (examProgressState === "SummaryConfirmation") )
        {
            dsExam.examScreenState = "ExamManager-PatientSelection";
        }
        appMain.setScreenState(dsExam.examScreenState);
    }

    function slotNavProtocol()
    {
        if ( (examProgressState === "PatientSelection") ||
             (examProgressState === "Started") )
        {
            dsExam.slotExamProgressStateChanged("ProtocolSelection");
            dsExam.examScreenState = "ExamManager-ProtocolSelection";
        }
        else if ( (examProgressState === "ProtocolSelection") ||
                  (examProgressState === "ProtocolModification") ||
                  (examProgressState === "InjectionExecution") ||
                  (examProgressState === "SummaryConfirmation") )
        {
            dsExam.examScreenState = "ExamManager-ProtocolSelection";
        }
        appMain.setScreenState(dsExam.examScreenState);
    }

    function slotNavInjection()
    {
        if ( (examProgressState === "PatientSelection") ||
             (examProgressState === "Started") ||
             (examProgressState === "ProtocolSelection") )
        {
            dsExam.slotExamProgressStateChanged("ProtocolModification");
            dsExam.examScreenState = "ExamManager-ProtocolModification";
        }
        else if ( (examProgressState === "ProtocolModification") ||
                  (examProgressState === "InjectionExecution") ||
                  (examProgressState === "SummaryConfirmation") )
        {
            dsExam.examScreenState = "ExamManager-ProtocolModification";
        }
        appMain.setScreenState(dsExam.examScreenState);
    }

    function slotNavSummary()
    {
        if ( (examProgressState === "PatientSelection") ||
             (examProgressState === "Started") ||
             (examProgressState === "ProtocolSelection") ||
             (examProgressState === "ProtocolModification") ||
             (examProgressState === "InjectionExecution") )
        {
            dsExam.slotExamProgressStateChanged("SummaryConfirmation");
            dsExam.examScreenState = "ExamManager-SummaryConfirmation";
        }
        else if (examProgressState === "SummaryConfirmation")
        {
            dsExam.examScreenState = "ExamManager-SummaryConfirmation";
        }
        appMain.setScreenState(dsExam.examScreenState);
    }

    function slotNavNext()
    {
        if (screenState === "ExamManager-PatientSelection")
        {
            if ( (cruLinkStatus.State === "Active") &&
                 (!patientSelect.isEntrySelected()) )
            {
                // Reload worklist
                dsMwl.slotPatientsReload();
            }
            else
            {
                if (examProgressState == "PatientSelection" || examProgressState == "ProtocolSelection")
                {
                    dsExam.examScreenState = "ExamManager-ProtocolSelection";
                }
                else
                {
                    dsExam.examScreenState = "ExamManager-ProtocolModification";
                }

                if (!isExamStarted)
                {
                    // Exam is not started yet. Start Exam.
                    // NOTE: examProgressState will be changed from slotExamProgressStateChanged().
                    dsExam.slotExamProgressStateChanged("Started");
                }
            }
        }
        else if (screenState === "ExamManager-ProtocolSelection")
        {
            if ( (planPreview.IsPersonalized) &&
                 (cruLinkStatus.State !== "Active") )
            {
                logWarning("CRU Link is " + cruLinkStatus.State + " and personalized protocol selected: " + JSON.stringify(planPreview));
                popupManager.popupPersonalizedProtocolUnavailable.open();
            }
            else
            {
                dsExam.slotLoadPlanFromPlanPreview(planPreview);

                if ( (examProgressState === "PatientSelection") ||
                     (examProgressState === "ProtocolSelection") )
                {
                    dsExam.slotExamProgressStateChanged("ProtocolModification");
                }
                dsExam.examScreenState = "ExamManager-ProtocolModification";
            }
        }
        else if (screenState === "ExamManager-ProtocolModification")
        {
            if (executedSteps.length < plan.Steps.length)
            {
                dsExam.slotInjectionArmed();
            }
            else
            {
                if ( (examProgressState === "PatientSelection") ||
                     (examProgressState === "ProtocolSelection") ||
                     (examProgressState === "ProtocolModification") ||
                     (examProgressState === "Started") ||
                     (examProgressState === "InjectionExecution") )
                {
                    dsExam.slotExamProgressStateChanged("SummaryConfirmation");
                }
                dsExam.examScreenState = "ExamManager-SummaryConfirmation";
            }
        }
        else if (screenState === "ExamManager-SummaryConfirmation")
        {
            if ( (examProgressState === "PatientSelection") ||
                 (examProgressState === "ProtocolSelection") ||
                 (examProgressState === "ProtocolModification") ||
                 (examProgressState === "Started") ||
                 (examProgressState === "InjectionExecution") ||
                 (examProgressState === "SummaryConfirmation") )
            {
                // End exam
                dsExam.slotInjectionAborted();
                dsExam.examProgressState = "Completing";
            }
        }
        appMain.setScreenState(dsExam.examScreenState);
    }

    function slotPlanChangedFromUnmodifiedDefault()
    {
        // Current plan (based from default template) is changed
        dsExam.slotExamProgressStateChanged("ProtocolModification");

        if (appMain.screenState === "Home")
        {
            // Snap to Protocol Modification screen from Home screen
            dsExam.examScreenState = "ExamManager-ProtocolModification";
            logDebug("ExamManager: slotPlanChangedFromUnmodifiedDefault(): examScreenState=" + dsExam.examScreenState + ": ScreenState=" + appMain.screenState + " to " + dsExam.examScreenState);
            appMain.setScreenState(dsExam.examScreenState);
        }
    }
}
