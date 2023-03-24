import QtQuick 2.12
import "../../Widgets"
import "../ExamInjectionPlot"
import "../../Util.js" as Util

Drawer {
    property var plan: dsExam.plan
    property var selectedContrast: dsExam.selectedContrast
    property var executedSteps: dsExam.executedSteps
    property int stepCountMax: dsCapabilities.stepCountMax
    property string examScreenState: dsExam.examScreenState
    property string pressureUnit: dsCfgGlobal.pressureUnit
    property int lastPlotStepIndex: -1
    property int stepIndex: -1
    property int widthMinimised: 0
    property int widthMaximised: dsCfgLocal.screenW * 0.8
    property int animationMs: 350
    property string showGraphBtnText: "T_ShowGraph"
    property var stepProgressDigest
    property double totalSalineVol: 0
    property double totalContrastVol: 0
    property int pressureKpaMax: 0
    property double flowRateMax
    property double durationMs

    id: root
    type: "LEFT"

    edgeDragEnabled: false
    width: widthMinimised

    Behavior on width {
        PropertyAnimation {
            duration: animationMs
        }
    }

    content: [
        Item {
            x: widthMinimised * 0.1
            width: widthMinimised * 0.8
            height: parent.height

            Text {
                id: txtStepName
                anchors.top: parent.top
                anchors.topMargin: parent.height * 0.05
                width: parent.width
                height: parent.height * 0.05
                verticalAlignment: Text.AlignVCenter
                text: ( (plan !== undefined) && (plan.Steps[stepIndex] !== undefined) ) ? ((stepIndex + 1) + "  " + plan.Steps[stepIndex].Name) : ""
                font.family: fontRobotoBold.name
                font.pixelSize: height * 0.78
                color: colorMap.text01
                elide: Text.ElideRight
            }

            Text {
                id: titleInjectedTotals
                anchors.top: txtStepName.bottom
                anchors.topMargin: parent.height * 0.05
                width: parent.width
                height: parent.height * 0.035
                text: translate("T_InjectedTotals")
                verticalAlignment: Text.AlignVCenter
                font.family: fontRobotoLight.name
                font.pixelSize: height
                color: colorMap.text01
                minimumPixelSize: font.pixelSize * 0.9
                fontSizeMode: Text.Fit
                elide: Text.ElideRight
            }

            Row {
                id: rowInjectedTotals
                anchors.top: titleInjectedTotals.bottom
                anchors.topMargin: parent.height * 0.01
                width: parent.width
                height: parent.height * 0.06
                spacing: parent.width * 0.02

                // Contrast total
                Text {
                    id: iconContrast
                    height: parent.height
                    width: contentWidth
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: height * 0.65
                    font.family: fontIcon.name
                    text: "\ue92f"
                    color: (selectedContrast.ColorCode === "GREEN") ? colorMap.contrast1 : colorMap.contrast2
                }

                Text {
                    id: txtTotalContrast
                    height: parent.height
                    text: localeToFloatStr(totalContrastVol, 1)
                    horizontalAlignment: Text.AlignRight
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: height * 0.8
                    font.family: fontRobotoBold.name
                    color: colorMap.text01
                }

                Text {
                    y: parent.height * 0.1
                    width: Math.min(parent.width * 0.35, contentWidth)
                    height: parent.height - y
                    text: translate("T_Units_ml")
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: height * 0.6
                    font.family: fontRobotoLight.name
                    color: colorMap.text02
                }

                Item {
                    width: parent.width * 0.04
                    height: parent.height
                }

                // Saline total
                Text {
                    height: parent.height
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: height * 0.65
                    font.family: fontIcon.name
                    text: "\ue930"
                    color: colorMap.saline
                }

                Text {
                    id: txtTotalSaline
                    height: parent.height
                    text: localeToFloatStr(totalSalineVol, 1)
                    horizontalAlignment: Text.AlignRight
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: height * 0.8
                    font.family: fontRobotoBold.name
                    color: colorMap.text01
                }

                Text {
                    y: parent.height * 0.1
                    height: parent.height - y
                    width: Math.min(parent.width * 0.35, contentWidth)
                    text: translate("T_Units_ml")
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: height * 0.6
                    font.family: fontRobotoLight.name
                    color: colorMap.text02
                }
            }

            Text {
                id: titleMaximums
                anchors.top: rowInjectedTotals.bottom
                anchors.topMargin: parent.height * 0.05
                width: parent.width
                height: parent.height * 0.035
                text: translate("T_Maximums")
                verticalAlignment: Text.AlignVCenter
                font.family: fontRobotoLight.name
                font.pixelSize: height
                color: colorMap.text01
            }

            Row {
                id: rowMaximums
                anchors.top: titleMaximums.bottom
                anchors.topMargin: parent.height * 0.01
                width: parent.width
                height: parent.height * 0.06
                spacing: parent.width * 0.02

                Text {
                    id: txtMaxFlow
                    height: parent.height
                    text: localeToFloatStr(flowRateMax, 1)
                    horizontalAlignment: Text.AlignRight
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: height * 0.8
                    font.family: fontRobotoBold.name
                    color: colorMap.text01
                }

                Text {
                    y: parent.height * 0.1
                    height: parent.height - y
                    width: Math.min(parent.width * 0.3, contentWidth)
                    text: translate("T_Units_ml/s")
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: height * 0.6
                    font.family: fontRobotoLight.name
                    color: colorMap.text02
                }

                Item {
                    width: parent.width * 0.04
                    height: parent.height
                }

                Text {
                    id: txtMaxPressure
                    height: parent.height
                    text: localeToFloatStr(Util.getPressure(pressureUnit, pressureKpaMax), (pressureUnit == "kg/cm2") ? 1 : 0)
                    horizontalAlignment: Text.AlignRight
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: height * 0.8
                    font.family: fontRobotoBold.name
                    color: colorMap.text01
                }

                Text {
                    y: parent.height * 0.1
                    height: parent.height - y
                    width: Math.min(parent.width * 0.35, contentWidth)
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: height * 0.6
                    font.family: fontRobotoLight.name
                    text: pressureUnit
                    color: colorMap.text02
                }
            }

            Row {
                id: rowTerminatedReason
                anchors.top: rowMaximums.bottom
                anchors.topMargin: parent.height * 0.04
                width: parent.width
                height: parent.height * 0.08
                spacing: parent.width * 0.02

                Item {
                    width: parent.width * 0.1
                    height: parent.height * 0.5
                    Text {
                        id: iconTerminatedReasonNormal
                        visible: (stepProgressDigest !== undefined) && (stepProgressDigest.TerminationReason === "Normal")
                        anchors.fill: parent
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                        font.pixelSize: height
                        font.family: fontIcon.name
                        text: "\ue926"
                        color: colorMap.text01
                    }

                    WarningIcon {
                        id: iconTerminatedReasonAborted
                        visible: !iconTerminatedReasonNormal.visible
                        anchors.fill: parent
                    }
                }

                Text {
                    id: txtTerminatatedReason
                    height: parent.height
                    width: parent.width * 0.9
                    horizontalAlignment: Text.AlignLeft
                    wrapMode: Text.Wrap
                    font.pixelSize: height * 0.46
                    font.family: fontRobotoBold.name
                    color: colorMap.text01
                    text: (stepProgressDigest !== undefined) ? translate("T_DeliveredStepTerminationReason_" + stepProgressDigest.TerminationReason) : ""
                }
            }

            Row {
                id: rowPressureLimited
                visible: (stepProgressDigest !== undefined) && (stepProgressDigest.HasPressureLimited)
                anchors.top: rowTerminatedReason.bottom
                anchors.topMargin: parent.height * 0.01
                width: rowTerminatedReason.width
                height: visible ? rowTerminatedReason.height : 0
                spacing: rowTerminatedReason.spacing

                Item {
                    width: parent.width * 0.1
                    height: parent.height * 0.5

                    WarningIcon {
                        anchors.fill: parent
                    }
                }

                Text {
                    height: parent.height
                    width: parent.width * 0.9
                    horizontalAlignment: Text.AlignLeft
                    wrapMode: Text.Wrap
                    font.pixelSize: height * 0.46
                    font.family: fontRobotoBold.name
                    color: colorMap.text01
                    text: translate("T_PressureLimited")
                }
            }

            Row {
                id: rowPreventingBackflowSaline
                visible: (stepProgressDigest !== undefined) && (stepProgressDigest.HasPreventedBackflowSaline)
                anchors.top: rowPressureLimited.bottom
                anchors.topMargin: parent.height * 0.01
                width: rowTerminatedReason.width
                height: visible ? rowTerminatedReason.height : 0
                spacing: rowTerminatedReason.spacing

                Item {
                    width: parent.width * 0.1
                    height: parent.height * 0.5

                    WarningIcon {
                        anchors.fill: parent
                    }
                }

                Text {
                    height: parent.height
                    width: parent.width * 0.9
                    horizontalAlignment: Text.AlignLeft
                    wrapMode: Text.Wrap
                    font.pixelSize: height * 0.46
                    font.family: fontRobotoBold.name
                    color: colorMap.text01
                    text: translate("T_PreventedBackflowSaline")
                }
            }

            GenericButton {
                id: btnViewPlot
                anchors.top: rowPreventingBackflowSaline.bottom
                anchors.topMargin: parent.height * 0.04
                width: parent.width * 0.9
                height: parent.height * 0.1
                color: colorMap.keypadButton

                content: [
                    Text {
                        width: parent.width * 0.2
                        height: parent.height
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        color: colorMap.text02
                        font.pixelSize: height * 0.5
                        font.family: fontIcon.name
                        text: "\ue954"
                    },

                    Text {
                        width: parent.width * 0.2
                        height: parent.height
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        color: colorMap.text01
                        font.pixelSize: height * 0.5
                        font.family: fontIcon.name
                        text: "\ue955"
                    },

                    Text {
                        id: txtBtnViewPlot
                        x: parent.width * 0.2
                        width: parent.width - x
                        height: parent.height * 0.9
                        anchors.verticalCenter: parent.verticalCenter
                        verticalAlignment: Text.AlignVCenter
                        color: colorMap.text01
                        font.pixelSize: height * 0.4
                        font.family: fontRobotoLight.name
                        text: translate(showGraphBtnText)
                        wrapMode: Text.Wrap
                    }
                ]

                onBtnClicked: {
                    if (root.width == widthMinimised)
                    {
                        if (lastPlotStepIndex != stepIndex)
                        {
                            dsExam.slotUpdateReviewPlot(stepIndex);
                            lastPlotStepIndex = stepIndex;
                        }
                        slotMaximise();
                    }
                    else
                    {
                        slotMinimise();
                    }
                }
            }

            Item {
                anchors.top: btnViewPlot.bottom
                anchors.topMargin: parent.height * 0.03
                width: parent.width * 0.9
                height: parent.height * 0.1

                MouseArea {
                    // Prevent background touch
                    anchors.fill: parent
                }

                GenericButton {
                    id: btnRepeatInjection
                    anchors.fill: parent
                    color: colorMap.keypadButton
                    enabled: getRepeatInjectionButtonState()

                    Text {
                        width: parent.width * 0.2
                        height: parent.height
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        color: colorMap.text01
                        font.pixelSize: height * 0.4
                        font.family: fontIcon.name
                        text: "\ue943"
                    }

                    Text {
                        x: parent.width * 0.2
                        height: parent.height * 0.9
                        width: parent.width - x
                        anchors.verticalCenter: parent.verticalCenter
                        verticalAlignment: Text.AlignVCenter
                        color: colorMap.text01
                        font.pixelSize: height * 0.4
                        font.family: fontRobotoLight.name
                        text: translate("T_RepeatInjection")
                        wrapMode: Text.Wrap
                    }
                    onBtnClicked: {
                        logDebug("Repeating step " + stepIndex);
                        injectionPlanEdit.stepList.newStepRow = stepIndex + 1;
                        dsExam.slotInjectionRepeat();
                        close();
                    }
                }
            }
        },

        // Plot Frame
        Rectangle {
            id: plotFrame
            x: widthMaximised - (width * 1.06)
            width: (widthMaximised - widthMinimised) * 0.96
            height: parent.height * 0.9
            anchors.verticalCenter: parent.verticalCenter
            color: colorMap.mainBackground
            radius: 15

            ExamInjectionPlot {
                id: injectionReviewPlot
                height: parent.height * 0.9
                width: parent.width
                anchors.centerIn: parent
                reviewMode: true
            }
        }
    ]

    onExamScreenStateChanged: {
        if (!visible)
        {
            return;
        }

        if (examScreenState === "ExamManager-InjectionExecution")
        {
            width = widthMinimised;
        }
    }

    onWidthChanged: {
        if (width == widthMinimised)
        {
            blurBackground.close([root]);
            showGraphBtnText = "T_ShowGraph";

            // visible true in slowMaximise
            plotFrame.visible = false;
        }
        else if (width == widthMaximised)
        {
            blurBackground.open([root], true, slotMinimise);
            showGraphBtnText = "T_HideGraph";
        }
    }

    onSignalClosed: {
        close();
    }

    Component.onCompleted: {
        dsExam.qmlReviewPlot = injectionReviewPlot;
        appMain.screenStateChanged.connect(slotScreenStateChanged);
        slotScreenStateChanged();
    }

    Component.onDestruction: {
        appMain.screenStateChanged.disconnect(slotScreenStateChanged);
    }

    function slotMaximise()
    {
        width = widthMaximised;

        // visible false in onWidthChanged
        plotFrame.visible = true;
    }

    function slotMinimise()
    {
        width = widthMinimised;
    }

    function openLast()
    {
        stepIndex = executedSteps.length - 1;
        open(stepIndex);
    }

    function isOpen()
    {
        return state == "OPEN";
    }

    function open(newStepIndex)
    {
        close();
        stepIndex = newStepIndex;
        state = "OPEN";
        root.visible = true;
        lastPlotStepIndex = -1;
        reload();
    }

    function close()
    {
        if (width == widthMaximised)
        {
            blurBackground.close([root]);
        }

        if (state == "CLOSED")
        {
            root.x = xClosed;
        }
        else
        {
            state = "CLOSED";
        }
        root.visible = false;
        slotMinimise();
        stepIndex = -1;
    }

    function slotScreenStateChanged()
    {
        reload();
    }

    function reload()
    {
        if (!visible)
        {
            return;
        }

        if (state !== "OPEN")
        {
            return;
        }

        stepProgressDigest = executedSteps[stepIndex];

        // Update params
        var phaseIdx;
        var newTotalSalineVol = 0;
        var newTotalContrastVol = 0;
        var newPressureKpaMax = 0;
        var newFlowRateMax = 0;
        var newDurationMs = 0;

        for (phaseIdx = 0; phaseIdx < stepProgressDigest.PhaseProgress.length; phaseIdx++)
        {
            var injectedVolumes = stepProgressDigest.PhaseProgress[phaseIdx].InjectedVolumes;
            newTotalSalineVol += injectedVolumes["RS0"];
            newTotalContrastVol += injectedVolumes["RC1"] + injectedVolumes["RC2"];
            newPressureKpaMax = Math.max(newPressureKpaMax, stepProgressDigest.PhaseProgress[phaseIdx].MaxPressure);
            newFlowRateMax = Math.max(newFlowRateMax, stepProgressDigest.PhaseProgress[phaseIdx].MaxFlowRate);
            newDurationMs += stepProgressDigest.PhaseProgress[phaseIdx].ElapsedMillisFromPhaseStart;
        }

        totalSalineVol = newTotalSalineVol;
        totalContrastVol = newTotalContrastVol;
        pressureKpaMax = newPressureKpaMax;
        flowRateMax = newFlowRateMax;
        durationMs = newDurationMs;
    }

    function getRepeatInjectionButtonState()
    {
        // Update repeat injection button state
        var repeatInjectionOk = false;

        if (!btnRepeatInjection.visible)
        {
            // Button is hidden
        }
        else if (stepIndex !== executedSteps.length - 1)
        {
            // Is not the last step
        }
        else if (plan === undefined)
        {
            // Bad plan data
        }
        else if (plan.Steps.length >= stepCountMax)
        {
            // Reached to maximum step count
        }
        else if (plan.Steps[stepIndex] === undefined)
        {
            // Bad step data
        }
        else if (plan.Steps[stepIndex].IsTestInjection)
        {
            var unexecutedTestInjections = 0;
            for (var i = executedSteps.length; i < plan.Steps.length; i++)
            {
                if (plan.Steps[i].IsTestInjection)
                {
                    unexecutedTestInjections++;
                }
            }
            repeatInjectionOk = (unexecutedTestInjections == 0);
        }
        else
        {
            repeatInjectionOk = true;
        }
        return repeatInjectionOk;
    }
}
