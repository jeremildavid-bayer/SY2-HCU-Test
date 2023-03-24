import QtQuick 2.12
import "../../Widgets"
import "../../DeviceManager"
import "../../Util.js" as Util

Rectangle {
    property string statePath: dsSystem.statePath
    property var executingStep: dsExam.executingStep
    property var selectedContrast: dsExam.selectedContrast
    property var stepProgressDigest: dsExam.stepProgressDigest
    property var curPhase: {
        if ( (executingStep === undefined) ||
             (stepProgressDigest === undefined) )
        {
            return undefined;
        }
        return executingStep.Phases[stepProgressDigest.PhaseIndex];
    }
    property double flowRateMin: dsCapabilities.flowRateMin
    property double flowRateMax: dsCapabilities.flowRateMax
    property bool displayDeviceManagerDuringInjection: dsCapabilities.displayDeviceManagerDuringInjection
    property int injectionSkipPhaseEnableWaitingMs: dsCapabilities.injectionSkipPhaseEnableWaitingMs
    property int skipButtonDebounceTimerMs: dsCapabilities.defaultButtonDebounceTimerMs
    property var scannerInterlocks: dsExam.scannerInterlocks
    property int animationMs: 400
    property int btnWidth: width * 0.16
    property int btnHeight: height * 0.7
    property int injectionResumedElapsedSec: dsExam.injectionResumedElapsedSec

    id: root
    width: parent.width
    height: parent.height
    color: colorMap.injectControlBarBackground

    Rectangle {
        id: progressStripe1
        width: parent.width
        height: parent.height * 0.04
        color: {
            if (statePath == "Ready/Armed")
            {
                return "transparent";
            }
            else if ( (statePath == "Busy/Holding") ||
                      (statePath == "Busy/Finishing") ||
                      (curPhase === undefined) )
            {
                return progressStripe1.color;
            }
            else if (statePath == "Executing")
            {
                if (curPhase.Type === "Fluid")
                {
                    var contrastColor = (selectedContrast.ColorCode === "GREEN") ? colorMap.contrast1 : colorMap.contrast2;
                    if (curPhase.ContrastPercentage === 0)
                    {
                        return colorMap.saline;
                    }
                    else
                    {
                        return contrastColor;
                    }
                }
            }
            return colorMap.paused;
        }
    }

    Rectangle {
        id: progressStripe2
        y: progressStripe1.y + progressStripe1.height
        width: parent.width
        height: progressStripe1.height
        color: {
            if (statePath == "Ready/Armed")
            {
                return "transparent";
            }
            else if ( (statePath == "Busy/Holding") ||
                      (statePath == "Busy/Finishing") ||
                      (curPhase === undefined) )
            {
                return progressStripe2.color;
            }
            else if (statePath == "Executing")
            {
                if (curPhase.Type === "Fluid")
                {
                    var contrastColor = (selectedContrast.ColorCode === "GREEN") ? colorMap.contrast1 : colorMap.contrast2;
                    if (curPhase.ContrastPercentage === 100)
                    {
                        return contrastColor;
                    }
                    else
                    {
                        return colorMap.saline;
                    }
                }
            }
            return colorMap.paused;
        }
    }

    Item {
        width: parent.width
        height: parent.height - progressStripe2.height - progressStripe2.y
        y: parent.height - height

        Rectangle {
            id: rectDeviceManagerIcon
            anchors.centerIn: parent
            color: root.color
            width: parent.width * 0.086
            height: parent.height * 0.82
        }

        GenericIconButton {
            id: btnAbort
            visible: {
                if ( (statePath == "Ready/Armed") ||
                     (statePath == "Busy/Holding") ||
                     (statePath == "Executing") )
                {
                    return true;
                }
                return false;
            }

            x: parent.width * 0.05
            anchors.verticalCenter: parent.verticalCenter
            width: btnWidth
            height: btnHeight
            color: colorMap.red
            iconColor: colorMap.blk01
            iconFontFamily: fontIcon.name
            iconText: "\ue951"
            iconFontPixelSize: height * 0.4
            onBtnClicked: {
                dsExam.slotInjectionAborted();
            }
        }

        GenericIconButton {
            id: btnSkip
            visible: {
                if (statePath == "Executing")
                {
                    return true;
                }
                return false;
            }
            enabled: shouldEnableSkipBtn()

            x: parent.width - width - (parent.width * 0.05)
            anchors.verticalCenter: parent.verticalCenter
            width: btnWidth
            height: btnHeight
            color: colorMap.keypadButton
            iconColor: colorMap.text01
            iconFontFamily: fontIcon.name
            iconText: "\ue950"
            iconFontPixelSize: height * 0.4
            onBtnClicked: {
                dsExam.slotInjectionSkipped();

                // Button debouncer
                enabled = false;
                timerSingleShot(skipButtonDebounceTimerMs, function() {
                    enabled = Qt.binding(shouldEnableSkipBtn);
                });
            }

            function shouldEnableSkipBtn()
            {
                if (stepProgressDigest === undefined)
                {
                    return false;
                }

                if (executingStep === undefined)
                {
                    return false;
                }

                if (executingStep.IsPreloaded)
                {
                    return false;
                }

                if (stepProgressDigest.PhaseIndex >= executingStep.Phases.length - 1)
                {
                    return false;
                }

                if ((injectionResumedElapsedSec * 1000) < injectionSkipPhaseEnableWaitingMs)
                {
                    // Disable Skip button for few moments
                    return false;
                }

                return true;
            }
        }

        GenericIconButton {
            id: btnStart
            visible: {
                if ( (statePath == "Ready/Armed") ||
                     (statePath == "Busy/Holding") )
                {
                    return true;
                }
                return false;
            }

            x: btnSkip.x
            anchors.verticalCenter: parent.verticalCenter
            width: btnWidth
            height: btnHeight
            color: colorMap.actionButtonBackground
            iconFontFamily: fontIcon.name
            iconColor: colorMap.actionButtonText
            iconFontPixelSize: height * 0.4
            iconText: {
                var ret = "\ue94e";
                if ( (statePath == "Ready/Armed") &&
                     (executingStep !== undefined) &&
                     (!executingStep.IsNotScannerSynchronized) &&
                     (scannerInterlocks !== undefined) &&
                     (scannerInterlocks.InterfaceStatus === "Active") &&
                     (scannerInterlocks.InterfaceMode !== "Unknown") &&
                     (scannerInterlocks.InterfaceMode !== "Monitor"))
                {
                    ret += "\ue90b";
                }
                return ret;
            }
            onBtnClicked: {
                dsExam.slotInjectionStarted();
            }
            pressedSoundCallback: function() { soundPlayer.playStart(); }
        }

        Rectangle {
            id: btnFlowAdjust
            visible: {
                if ( (statePath == "Executing") &&
                     (executingStep.Phases[stepProgressDigest.PhaseIndex] !== undefined) &&
                     (executingStep.Phases[stepProgressDigest.PhaseIndex].Type === "Fluid") &&
                     (executingStep.IsTestInjection) &&
                     (executingStep.Phases.length === 1) )
                {
                    return true;
                }
                return false;
            }

            x: btnSkip.x
            anchors.verticalCenter: parent.verticalCenter
            width: btnWidth
            height: btnHeight
            color: "transparent"

            Rectangle {
                id: rectFlowAdjustRow
                radius: btnStart.radius
                width: parent.width
                height: radius * 2
                color: rectFlowAdjustText.color
            }

            Rectangle {
                id: rectFlowAdjustText
                y: rectFlowAdjustRow.height / 2
                width: parent.width
                height: parent.height * 0.3
                color: {
                    if ( (curPhase !== undefined) &&
                         (curPhase.ContrastPercentage > 0) )
                    {
                        return (selectedContrast.ColorCode === "GREEN") ? colorMap.contrast1 : colorMap.contrast2;
                    }
                    return colorMap.saline;
                }

                Text {
                    id: textCurFlowRate
                    anchors.fill: parent
                    anchors.bottomMargin: rectFlowAdjustRow.radius
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    color: colorMap.white01
                    font.pixelSize: height * 0.6
                    text: {
                        if (curPhase === undefined)
                        {
                            return "";
                        }
                        return localeToFloatStr(curPhase.FlowRate, 1) + translate("T_Units_ml/s");
                    }
                }
            }

            Rectangle {
                id: spacer1
                color: colorMap.keypadButton
                z: rectFlowAdjustText.z - 1
                y: rectFlowAdjustText.y + rectFlowAdjustText.height
                x: (parent.width - width) / 2
                width: btnStart.radius * 2
                height: parent.height - y
            }

            GenericIconButton {
                id: btnFlowAdjustDown
                enabled: {
                    if (stepProgressDigest === undefined)
                    {
                        return false;
                    }
                    if ( (stepProgressDigest.AdaptiveFlowState === "Active") ||
                         (stepProgressDigest.AdaptiveFlowState === "ActiveCritical") )
                    {
                        return false;
                    }
                    if (curPhase === undefined)
                    {
                        return false;
                    }
                    var curFlowRateStr = localeToFloatStr(curPhase.FlowRate, 1);
                    var curFlowRate = localeFloatStrToFloat(curFlowRateStr);
                    return curFlowRate > flowRateMin;
                }

                y: rectFlowAdjustText.height
                z: rectFlowAdjustText.z - 1
                width: parent.width * 0.5
                height: parent.height - y
                color: colorMap.keypadButton
                iconFontFamily: fontIcon.name
                iconColor: colorMap.text01
                iconText: "\ue953"
                iconFontPixelSize: height * 0.4
                onBtnClicked: {
                    dsExam.slotInjectionFlowAdjusted(false);
                }
            }

            GenericIconButton {
                id: btnFlowAdjustUp
                enabled: {
                    if (stepProgressDigest === undefined)
                    {
                        return false;
                    }
                    if ( (stepProgressDigest.AdaptiveFlowState === "Active") ||
                         (stepProgressDigest.AdaptiveFlowState === "ActiveCritical") )
                    {
                        return false;
                    }
                    if (curPhase === undefined)
                    {
                        return false;
                    }

                    var curFlowRateStr = localeToFloatStr(curPhase.FlowRate, 1);
                    var curFlowRate = localeFloatStrToFloat(curFlowRateStr);
                    return curFlowRate < flowRateMax;
                }

                y: btnFlowAdjustDown.y
                z: rectFlowAdjustText.z - 1
                x: parent.width * 0.5
                width: parent.width * 0.5
                height: parent.height - y
                color: colorMap.keypadButton
                iconFontFamily: fontIcon.name
                iconColor: colorMap.text01
                iconText: "\ue952"
                iconFontPixelSize: height * 0.4
                onBtnClicked: {
                    dsExam.slotInjectionFlowAdjusted(true);
                }
            }



            Rectangle {
                id: spacer2
                color: colorMap.injectControlBarBackground
                y: spacer1.y
                width: parent.width
                height: parent.height * 0.03
            }

            Rectangle {
                id: spacer3
                color: colorMap.injectControlBarBackground
                y: spacer1.y
                x: (parent.width - width) / 2
                width: spacer2.height
                height: parent.height - y
            }
        }


        GenericIconButton {
            id: btnPause
            visible: {
                if (statePath == "Executing")
                {
                    return true;
                }
                return false;
            }

            x: btnSkip.x - width - (parent.width * 0.03)
            anchors.verticalCenter: parent.verticalCenter
            width: btnWidth
            height: btnHeight
            color: colorMap.keypadButton
            iconFontFamily: fontIcon.name
            iconColor: colorMap.text01
            iconText: "\ue94f"
            iconFontPixelSize: height * 0.4
            onBtnClicked: {
                dsExam.slotInjectionPaused();
            }
        }
    }

    SequentialAnimation {
        id: animationActivated
        NumberAnimation { target: root; properties: 'y'; from: height; to: 0; duration: animationMs; easing.type: Easing.InOutQuart }
    }

    onVisibleChanged: {
        if (visible)
        {
            if (displayDeviceManagerDuringInjection)
            {
                deviceManagerIcon.visible = true;
                deviceManagerIcon.parent = rectDeviceManagerIcon;
                deviceManagerIcon.displaySourcePackageInfo = false;
                deviceManagerIcon.rootBackgroundColor = Qt.binding(function() { return rectDeviceManagerIcon.color; });
            }

            btnAbort.reset();
            btnPause.reset();
            btnSkip.reset();
            btnStart.reset();
            btnFlowAdjustUp.reset();
            btnFlowAdjustDown.reset();
            animationActivated.start();
        }
    }

    Component.onCompleted: {
        appMain.screenStateChanged.connect(slotScreenStateChanged);
        slotScreenStateChanged();
    }

    Component.onDestruction: {
        appMain.screenStateChanged.disconnect(slotScreenStateChanged);
    }

    function slotScreenStateChanged()
    {
        visible = (appMain.screenState === "ExamManager-InjectionExecution");
    }
}








