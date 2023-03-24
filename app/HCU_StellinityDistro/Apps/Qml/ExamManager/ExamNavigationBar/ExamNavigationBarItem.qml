import QtQuick 2.12
import QtGraphicalEffects 1.12
import "../../Widgets"
import "../../Util.js" as Util

Item {
    property string examScreenState: dsExam.examScreenState
    property var plan: dsExam.plan
    property var executedSteps: dsExam.executedSteps
    property string examProgressState: dsExam.examProgressState
    property bool selected: false
    property string label: ""
    property string btnState: "ENABLED" // "ENABLED", "SELECTED", "DISABLED"
    property double progressPercent: 0
    property bool visibleCondition: true
    property string examGuid: dsExam.examGuid
    property var examAdvanceInfo: dsExam.examAdvanceInfo
    property bool isExamStarted: dsExam.isExamStarted
    property bool licenseEnabledPatientStudyContext: dsCru.licenseEnabledPatientStudyContext
    property var cruLinkStatus: dsCru.cruLinkStatus

    signal btnClicked();

    id: root
    visible: (btnState !== "DISABLED") && (visibleCondition)

    GenericButton {
        anchors.fill: parent
        color: "transparent"
        interactive: !minimised

        Item {
            anchors.fill: parent

            Text {
                id: mandatoryFieldMark
                visible: getMandatoryMarkVisibleState();
                anchors.top: parent.top
                anchors.topMargin: contentHeight * 0.52
                anchors.right: parent.right
                anchors.rightMargin: contentWidth * 1.3
                width: contentWidth
                height: contentHeight
                font.family: fontIcon.name
                color: colorMap.errText
                font.pixelSize: dsCfgLocal.screenH * 0.018
                text: "\ue945"
                font.bold: true
            }

            Image {
                id: image
                visible: !animation.visible
                y: minimised ? ((parent.height - height) / 2) : (parent.height * 0.1)
                width: minimised ? parent.height * 0.7 : parent.height * 0.6
                height: width
                sourceSize.width: width
                sourceSize.height: height
                anchors.horizontalCenter: parent.horizontalCenter
            }

            AnimatedImage {
                id: animation
                visible: playing
                x: image.x
                y: image.y
                width: image.width
                height: image.height
                playing: false
            }

            ProgressArc {
                id: navProgress
                lineWidth: parent.width * 0.05
                x: image.x - lineWidth
                y: image.y - lineWidth
                width: image.width + (lineWidth * 2)
                height: image.height + (lineWidth * 2)
                anchors.horizontalCenter: parent.horizontalCenter
                animationMs: minimised ? 0 : 500
                lineColor: colorMap.navBtnProgressLine
                angleStart: 1.5 * Math.PI
                angleTotal: 2 * Math.PI
            }

            Text {
                visible: !minimised
                anchors.bottom: parent.bottom
                anchors.bottomMargin: parent.height * 0.07
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width * 0.95 // cater for border
                font.family: fontRobotoLight.name
                font.pixelSize: parent.height * 0.115
                fontSizeMode: Text.Fit
                horizontalAlignment: Text.AlignHCenter
                color: colorMap.text01
                text: translate(label)
            }
        }

        onBtnClicked: {
            if (btnState == "ENABLED")
            {
                root.btnClicked();
            }
        }

        pressedSoundCallback: function(){ soundPlayer.playNext(); }
    }

    onExamAdvanceInfoChanged: {
        reloadProgress();
    }

    onExamGuidChanged: {
        reloadProgress();
    }

    onPlanChanged: {
        reloadProgress();
    }

    onExecutedStepsChanged: {
        reloadProgress();
    }

    onExamProgressStateChanged: {
        reloadProgress();
    }

    onExamScreenStateChanged: {
        reloadProgress();
    }

    onVisibleChanged: {
        reloadProgress();
    }

    onCruLinkStatusChanged: {
        reloadProgress();
    }

    function getMandatoryMarkVisibleState()
    {
        if (!visible)
        {
            return false;
        }

        if (minimised)
        {
            return false;
        }

        if (label === "T_Patient")
        {
            if ( (cruLinkStatus.State === "Active") &&
                 (examAdvanceInfo !== undefined) &&
                 (examAdvanceInfo.ExamInputs !== undefined) )
            {
                var examInputs = examAdvanceInfo.ExamInputs;
                for (var key in examInputs)
                {
                    if ( (examInputs[key].IsMandatory) &&
                         (!examInputs[key].IsMandatoryEntered) )
                    {
                        return true;
                    }
                }
            }
        }
        else if (label === "T_Summary")
        {
            if ( (cruLinkStatus.State === "Active") &&
                 (isExamStarted) &&
                 (examAdvanceInfo !== undefined) &&
                 (examAdvanceInfo.ExamResults !== undefined) )
            {
                var examOutputs = examAdvanceInfo.ExamResults;
                for (var key2 in examOutputs)
                {
                    if (examOutputs[key2].IsMandatory && !examOutputs[key2].IsMandatoryEntered)
                    {
                        return true;
                    }
                }
            }
        }

        return false;
    }

    function reloadProgress()
    {
        if (!visible)
        {
            return;
        }

        if (examScreenState == "Home")
        {
            resetProgress();
            return;
        }

        // Re-evaluate whether the * is shown
        mandatoryFieldMark.visible = getMandatoryMarkVisibleState();

        var newProgressPercent = 0;

        if (label == "T_Patient")
        {
            var firstProgressPatient = 0;
            var secondProgressPatient = 0;

            // 50% of the progress is whether or not the exam is started
            if (dsExam.isExamStarted)
            {
                firstProgressPatient = 50;
                secondProgressPatient = 50;
            }

            // 50% of the progress is if all mandatory fields are entered,
            // independent of whether an exam is started
            if ( (licenseEnabledPatientStudyContext) &&
                 (cruLinkStatus.State === "Active") &&
                 (examAdvanceInfo !== undefined) )
            {
                // If there are no mandatory fields, CRU will give 100 progress.
                // We only want to show second progress if at least one field is designated as mandatory.
                var examInputs = examAdvanceInfo.ExamInputs;
                for (var key in examInputs)
                {
                    // If any value is marked as mandatory, use ExamInputProgress
                    if (examInputs[key].IsMandatory)
                    {
                        // Only use the progress data if fields are designated as mandatory
                        secondProgressPatient = examAdvanceInfo.ExamInputsProgress * 0.5;
                        break;
                    }
                }
            }

            newProgressPercent = firstProgressPatient + secondProgressPatient;

            if ( (!minimised) &&
                 (progressPercent !== newProgressPercent) )
            {
                if (newProgressPercent === 100)
                {
                    animation.source = imageMap.examStagePatientSelectedGif;
                    animation.playing = true;
                }
            }

            if (isExamStarted)
            {
                image.source = imageMap.examStagePatientComplete;
            }
            else
            {
                image.source = imageMap.examStagePatientIncomplete;
            }

        }
        else if (label == "T_Protocols")
        {
            if ( (examProgressState == "ProtocolModification") ||
                 (examProgressState == "InjectionExecution") ||
                 (examProgressState == "SummaryConfirmation") ||
                 (examProgressState == "Completing") )
            {
                newProgressPercent = 100;
            }

            image.source = imageMap.examStageLibrary;
        }
        else if (label == "T_Injection")
        {
            if ( (executedSteps != undefined) &&
                 (plan !== undefined) )
            {
                newProgressPercent = (executedSteps.length / plan.Steps.length) * 100;
            }

            image.source = imageMap.examStageInjection;
        }
        else if (label == "T_Summary")
        {
            var firstProgressSummary = 0;
            var secondProgressSummary = 0;

            // 50% of the progress is if all injections are completed
            if ( (executedSteps != undefined) &&
                 (plan !== undefined) &&
                 (executedSteps.length >= plan.Steps.length) )
            {
                firstProgressSummary = 50;
                secondProgressSummary = 50;
            }

            // 50% of the progress is if all mandatory fields are entered,
            // independent of whether all injections are completed
            if ( (licenseEnabledPatientStudyContext) &&
                 (cruLinkStatus.State === "Active") &&
                 (examAdvanceInfo !== undefined) )
            {
                // If there are no mandatory fields, CRU will give 100 progress.
                // We only want to show second progress if at least one field is designated as mandatory.
                var examResults = examAdvanceInfo.ExamResults;
                for (var key2 in examResults)
                {
                    // If any value is marked as mandatory, use ExamInputProgress
                    if (examResults[key2].IsMandatory)
                    {
                        // Only use the progress data if fields are designated as mandatory
                        secondProgressSummary = examAdvanceInfo.ExamResultsProgress * 0.5;
                        break;
                    }
                }
            }

            newProgressPercent = firstProgressSummary + secondProgressSummary;

            if (minimised)
            {
                // No animation needed for minimised
            }
            else if (progressPercent !== newProgressPercent)
            {
                if (firstProgressSummary === 50)
                {
                    if (image.source.toString() !== imageMap.examStageSummaryComplete.toString())
                    {
                        animation.source = imageMap.examStageSummaryProgressToCompleteGif;
                        animation.playing = true;
                    }
                }
                else
                {
                    if ( image.source.toString() !== imageMap.examStageSummaryProgress.toString() &&
                         image.source.toString() !== imageMap.examStageSummaryAbort.toString() )
                    {
                        animation.source = imageMap.examStageSummaryCompleteToProgressGif;
                        animation.playing = true;
                    }
                }
            }
            else if (examProgressState == "Completing")
            {
                if (newProgressPercent === 100)
                {
                    animation.source = imageMap.examStageSummaryCompleteGif;
                }
                else
                {
                    animation.source = imageMap.examStageSummaryAbortGif;
                    image.source = imageMap.examStageSummaryAbort;
                }
                animation.playing = true;
            }

            if (image.source.toString() === imageMap.examStageSummaryAbort.toString())
            {
                if (examScreenState === "ExamManager-SummaryConfirmation")
                {
                    image.source = imageMap.examStageSummaryAbort;
                }
                else
                {
                    image.source = imageMap.examStageSummaryProgress;
                }
            }
            else if (firstProgressSummary === 50)
            {
                image.source = imageMap.examStageSummaryComplete;
            }
            else
            {
                image.source = imageMap.examStageSummaryProgress;
            }

        }

        if (progressPercent !== newProgressPercent)
        {
            logDebug("ExamNavigationBarItem: " + label + ": Progress changed from " + progressPercent + " -> " + newProgressPercent);
            progressPercent = newProgressPercent;
            navProgress.setProgress(newProgressPercent);
        }
    }

    function resetProgress()
    {
        progressPercent = 0;
        navProgress.setProgress(progressPercent);
    }
}
