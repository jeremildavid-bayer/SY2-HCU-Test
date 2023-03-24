import QtQuick 2.12
import QtGraphicalEffects 1.12
import "../../Widgets"
import "../../Util.js" as Util

Item {
    property int phaseIndex: index
    property var selectedContrast: dsExam.selectedContrast
    property bool sameContrasts: dsDevice.sameContrasts
    property bool phaseDataChanged: false
    property var phaseData
    property int borderWidth: phaseHeight * 0.05
    property int rowHeight: (phaseData === undefined) ? 0 : (phaseHeight + rowSpacing)
    property double flowRateMin: dsCapabilities.flowRateMin
    property double flowRateMax: dsCapabilities.flowRateMax
    property double volumeMin: dsCapabilities.volumeMin
    property double volumeMax: dsCapabilities.volumeMax
    property int delayMsMin: dsCapabilities.delayMsMin
    property int delayMsMax: dsCapabilities.delayMsMax
    property string statePath: dsSystem.statePath
    property string colorUpper: getColorUpper()
    property string colorLower: getColorLower()
    property string editType: "NULL" // NULL, FLOW, VOLUME, DURATION
    property double progressPercent: getProgressPercent()
    property int animationDeleteMs: 250
    property int animationAddMs: 250

    id: root
    width: ListView.view.width
    height: rowHeight
    visible: phaseData !== undefined

    Rectangle {
        id: mainRect
        y: rowSpacing * 0.5
        width: parent.width * 0.88
        height: phaseHeight
        radius: height * 0.5

        gradient: Gradient {
            GradientStop { position: 0.0; color: colorUpper }
            GradientStop { position: 0.50; color: colorUpper }
            GradientStop { position: 0.51; color: colorLower }
            GradientStop { position: 1.0; color: colorLower }
        }

        Image {
            id: imageDrag
            anchors.fill: parent
            visible: false
        }

        Item {
            id: progressBarContainer
            width: parent.width - (parent.height * 0.11)
            height: parent.height - (parent.height * 0.11)
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter

            Rectangle {
                id: progressBarFrame
                anchors.fill: parent
                color: "transparent"
                visible: false

                Rectangle {
                    id: progressBarRect
                    x: parent.width - width
                    width: (parent.width * (100 - progressPercent)) / 100
                    height: parent.height
                    color: colorMap.injectPhaseProgressBackground
                }
            }

            Rectangle {
                id: progressBarMask
                anchors.fill: parent
                radius: mainRect.radius
                visible: false
            }

            OpacityMask {
                anchors.fill: progressBarFrame
                source: progressBarFrame
                maskSource: progressBarMask
            }

            onVisibleChanged: {
                if (visible)
                {
                    // If image preview is never initialised, start grab now
                    updateDragImage();
                }
            }
        }

        Item {
            id: btnDrag
            x: parent.width * 0.028
            width: parent.width * 0.05
            height: parent.height
            visible: {
                if ( (stepData === undefined) || (phaseData === undefined) )
                {
                    return false;
                }
                else if (phaseData.Type === "Dummy")
                {
                    return false;
                }
                else if (stepIndex < executedSteps.length)
                {
                    return false;
                }
                else
                {
                    return stepData.Phases.length > 1;
                }
            }

            Text {
                anchors.fill: parent
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                color: colorMap.text02
                font.pixelSize: parent.height * 0.3
                font.family: fontIcon.name
                text: "\ue949"
            }

            MouseArea {
                anchors.fill: parent

                onPressed: (mouse) => {
                    soundPlayer.playPressGood();

                    if (widgetInputPad.isOpen())
                    {
                        widgetInputPad.close(true);
                    }

                    listViewStep.interactive = false;

                    dragObject.open(mainRect, imageDrag.source);
                    dragObject.userData = Util.copyObject(phaseData);

                    injectionPlanEdit.setCurEditParamInfo({ "State": "SORTING_PHASE", "Index": phaseIndex });
                    dummyPhaseIdx = phaseIndex;
                    planEditData.Steps[stepIndex].Phases[phaseIndex].Type = "Dummy";
                    stepData.Phases[phaseIndex].Type = "Dummy";
                    dummyPhaseCount++;

                    reloadAll(stepIndex, phaseIndex);
                    mouse.accepted = false;
                }
            }
        }

        GenericButton {
            id: btnType
            radius: 0
            x: parent.width * 0.1
            width: parent.width * 0.1
            height: parent.height
            visible: (phaseData !== undefined) && (phaseData.Type !== "Dummy")

            Text {
                id: iconType
                anchors.fill: parent
                font.family: fontIcon.name
                font.pixelSize: height * 0.5
                color: colorMap.text01
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                text: {
                    if (phaseData !== undefined)
                    {
                        if (phaseData.Type === "Fluid")
                        {
                            if (phaseData.ContrastPercentage === 100)
                            {
                                return "\ue92f";
                            }
                            else if (phaseData.ContrastPercentage === 0)
                            {
                                return "\ue930";
                            }
                        }
                        else if (phaseData.Type === "Delay")
                        {
                            return "\ue94c";
                        }
                    }
                    return "";
                }
            }

            onBtnClicked: {
                editOptionList.open();
            }
        }

        GenericButton {
            id: btnEditDualRatio
            x: parent.width * 0.1
            width: parent.width * 0.1
            radius: 0
            y: (editType == "DUAL_RATIO") ? -(rowSpacing / 2) : 0
            z: (editType == "DUAL_RATIO") ? root.z + 1 : root.z
            height: (editType == "DUAL_RATIO") ? phaseHeight + rowSpacing : phaseHeight
            color: "transparent"
            visible: (phaseData !== undefined) && (phaseData.Type === "Fluid") && (phaseData.ContrastPercentage > 0) && (phaseData.ContrastPercentage < 100)

            content: [
                Text {
                    id: txtDualRatio1
                    y: parent.height * 0.05
                    width: parent.width
                    height: (parent.height / 2) - y
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignBottom
                    color: colorMap.text01
                    font.pixelSize: height * 0.72
                    font.family: fontRobotoBold.name
                    text: "0"
                },

                Text {
                    id: txtDualRatio2
                    y: (parent.height * 0.5) + txtDualRatio1.y
                    width: txtDualRatio1.width
                    height: txtDualRatio1.height
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignTop
                    color: txtDualRatio1.color
                    font.pixelSize: txtDualRatio1.font.pixelSize
                    font.family: txtDualRatio1.font.family
                    text: (100 - parseInt(txtDualRatio1.text))
                }
            ]

            onBtnClicked: {
                startEdit("DUAL_RATIO");
            }
        }

        GenericButton {
            id: btnEditFlowRate
            x: parent.width * 0.2
            width: parent.width * 0.33
            radius: 0
            y: (editType == "FLOW") ? -(rowSpacing / 2) : 0
            z: (editType == "FLOW") ? root.z + 1 : root.z
            height: (editType == "FLOW") ? phaseHeight + rowSpacing : phaseHeight
            color: "transparent"
            visible: (phaseData !== undefined) && (phaseData.Type === "Fluid")

            Text {
                id: txtFlowRate
                height: parent.height
                width: parent.width * 0.5
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignVCenter
                color: colorMap.text01
                font.family: fontRobotoBold.name
                font.pixelSize: parent.height * 0.45
            }

            Text {
                id: txtFlowRateUnit
                x: txtFlowRate.x + txtFlowRate.width + 5
                y: parent.height * 0.1
                width: parent.width - x
                height: parent.height - y
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
                color: colorMap.text02
                font.family: fontRobotoLight.name
                font.pixelSize: parent.height * 0.32
                text: translate("T_Units_ml/s")
                elide: Text.ElideRight
            }

            onBtnClicked: {
                startEdit("FLOW");
            }
        }

        GenericButton {
            id: btnEditVolume
            x: parent.width * 0.52
            width: parent.width * 0.24
            radius: 0
            y: (editType == "VOLUME") ? -(rowSpacing / 2) : 0
            z: (editType == "VOLUME") ? root.z + 1 : root.z
            height: (editType == "VOLUME") ? phaseHeight + rowSpacing : phaseHeight
            color: "transparent"
            visible: (phaseData !== undefined) && (phaseData.Type === "Fluid")

            Text {
                id: txtVolume
                height: parent.height
                width: parent.width * 0.6
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignVCenter
                color: colorMap.text01
                font.family: fontRobotoBold.name
                font.pixelSize: parent.height * 0.45
            }

            Text {
                id: txtVolumeUnit
                x: txtVolume.x + txtVolume.width + 5
                y: parent.height * 0.1
                width: parent.width - x
                height: parent.height - y
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
                color: colorMap.text02
                font.family: fontRobotoLight.name
                font.pixelSize: parent.height * 0.32
                text: translate("T_Units_ml")
                elide: Text.ElideRight
            }

            onBtnClicked: {
                startEdit("VOLUME");
            }
        }

        GenericButton {
            id: btnEditDuration
            x: parent.width * 0.77
            width: parent.width * 0.22
            disabledColor: color
            radius: 0
            y: (editType == "DURATION") ? -(rowSpacing / 2) : 0
            z: (editType == "DURATION") ? root.z + 1 : root.z
            height: (editType == "DURATION") ? phaseHeight + rowSpacing : phaseHeight
            color: "transparent"
            visible: (phaseData !== undefined) && ( (phaseData.Type === "Fluid") || (phaseData.Type === "Delay") )
            enabled: (phaseData !== undefined) && (phaseData.Type === "Delay")

            Text {
                id: txtDuration
                height: parent.height
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                color: colorMap.text01
                font.family: btnEditDuration.enabled ? fontRobotoBold.name : fontRobotoLight.name
                font.pixelSize: parent.height * 0.45
            }

            onBtnClicked: {
                startEdit("DURATION");
            }
        }

        Rectangle {
            id: deactivatedCover
            color: colorMap.injectPhaseProgressBackground
            width: parent.width - (parent.height * 0.05 * 2)
            height: parent.height - (parent.height * 0.05 * 2)
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            radius: mainRect.radius
            visible: (stepIndex < executedSteps.length)
            MouseArea {
                anchors.fill: parent
                // Covers any mouse areas in main frame
            }
            z: root.z + 2
        }
    }

    SequentialAnimation {
        id: animationDeleted
        ScriptAction { script: {
                if (widgetInputPad.isOpen())
                {
                    widgetInputPad.close(true);
                }
            }
        }

        NumberAnimation { target: root; properties: 'opacity'; to: 0; duration: animationDeleteMs; }
        ScriptAction { script: {
                if (planEditData.Steps[stepIndex].Phases.length > 1)
                {
                    // Delete only if there are more than one phases
                    planEditData.Steps[stepIndex].Phases.splice(phaseIndex, 1);
                    injectionPlanEdit.signalStepReload(stepIndex);
                    injectionPlanEdit.savePlan(planEditData);
                }
                opacity = 1;
            }
        }
    }

    GenericButton {
        id: btnDelete
        visible: {
            if ( (stepData === undefined) || (phaseData === undefined) )
            {
                return false;
            }
            else if (phaseData.Type === "Dummy")
            {
                return false;
            }
            else if (stepIndex < executedSteps.length)
            {
                return false;
            }
            return stepData.Phases.length > 1;
        }

        anchors.right: parent.right
        anchors.rightMargin: phaseWidth * 0.022
        anchors.verticalCenter: parent.verticalCenter
        height: phaseHeight * 0.9
        width: phaseHeight * 0.9
        color: "transparent"
        content: [
            Text {
                anchors.fill: parent
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                color: colorMap.text02
                font.pixelSize: height * 0.5
                font.family: fontIcon.name
                text: "\ue94d"
            }
        ]

        onBtnClicked: {
            animationDeleted.start();
        }
    }

    SequentialAnimation {
        id: animationAdded
        NumberAnimation { target: root; properties: 'opacity'; from: 1; to: 0; duration: animationAddMs }
        NumberAnimation { target: root; properties: 'opacity'; from: 0; to: 1; duration: animationAddMs }
        NumberAnimation { target: root; properties: 'opacity'; from: 1; to: 0; duration: animationAddMs }
        NumberAnimation { target: root; properties: 'opacity'; from: 0; to: 1; duration: animationAddMs }

        ScriptAction { script: {
                // Blink animation completed. Update Step drag image
                injectionStepEdit.updateDragImage();
            }
        }
    }

    function getColorUpper()
    {
        if ( (phaseData === undefined) || (phaseData.Type === "Dummy") )
        {
            return colorMap.buttonShadow;
        }
        else if (phaseData.Type === "Fluid")
        {
            if (phaseData.ContrastPercentage === 0)
            {
                return colorMap.saline;
            }
            else
            {
                return (selectedContrast.ColorCode === "GREEN") ? colorMap.contrast1 : colorMap.contrast2;
            }
        }
        else if (phaseData.Type === "Delay")
        {
            return colorMap.paused;
        }
    }

    function getColorLower()
    {
        if ( (phaseData === undefined) || (phaseData.Type === "Dummy") )
        {
            return colorMap.buttonShadow;
        }
        else if (phaseData.Type === "Fluid")
        {
            if (phaseData.ContrastPercentage === 100)
            {
                return (selectedContrast.ColorCode === "GREEN") ? colorMap.contrast1 : colorMap.contrast2;
            }
            else
            {
                return colorMap.saline;
            }
        }
        else if (phaseData.Type === "Delay")
        {
            return colorMap.paused;
        }
    }

    function getProgressPercent()
    {
        if (stepIndex < executedSteps.length)
        {
            if (executedSteps[stepIndex].PhaseProgress[phaseIndex] !== undefined)
            {
                return executedSteps[stepIndex].PhaseProgress[phaseIndex].Progress;
            }
        }
        return 0;
    }

    onSelectedContrastChanged: {
        updateDragImage();
    }

    Component.onCompleted: {
        listViewStep.dragStarted.connect(resetButtons);
        injectionPlanEdit.signalPhaseReload.connect(reloadAll);
        injectionPlanEdit.signalResumeEditProtParam.connect(resumeEdit);
    }

    Component.onDestruction: {
        listViewStep.dragStarted.disconnect(resetButtons);
        injectionPlanEdit.signalPhaseReload.disconnect(reloadAll);
        widgetInputPad.signalValueChanged.disconnect(slotInputPadValChanged);
        widgetInputPad.signalClosed.disconnect(slotInputPadClosed);
        injectionPlanEdit.signalResumeEditProtParam.disconnect(resumeEdit);
    }

    function updateDragImage()
    {
        // Give a little moment (component may get deleted soon) and capture the image.
        timerSingleShot(10, function() {
            if ( (mainRect != null) && (mainRect.visible) && (mainRect.width > 0) && (mainRect.height > 0) )
            {
                //logDebug("ExamProtocolEditPhase[" + phaseIndex + "]: updateDragImage() Started");
                mainRect.grabToImage(function(result) {
                    imageDrag.source = result.url;
                    //logDebug("ExamProtocolEditPhase[" + phaseIndex + "]: updateDragImage() Done");
                }, Qt.size(mainRect.width, mainRect.height));
            }
        });
    }

    function reloadAll(stepIdx, phaseIdx)
    {
        if ( (stepIdx === stepIndex) && (phaseIdx === phaseIndex) )
        {
            if (!Util.compareObjects(phaseData, planEditData.Steps[stepIndex].Phases[phaseIndex]))
            {
                phaseData = Util.copyObject(planEditData.Steps[stepIndex].Phases[phaseIndex]);
                phaseDataChanged = true;
            }
            reload();
        }

        if ( (stepIndex == newStepRow) && (phaseIndex == newPhaseRow) )
        {
            //logDebug("Phase[" + stepIndex + "]: AnimationAddedStart");
            animationAdded.start();
        }
    }

    function resetButtons()
    {
        btnDelete.reset();
        btnEditFlowRate.reset();
        btnEditVolume.reset();
        btnEditDuration.reset();
        btnEditDualRatio.reset();
        btnType.reset();
    }

    function reload()
    {
        if (phaseData === undefined)
        {
            return;
        }

        txtDualRatio1.color = Qt.binding(function() { return colorMap.text01; });
        txtFlowRate.color = Qt.binding(function() { return colorMap.text01; });
        txtVolume.color = Qt.binding(function() { return colorMap.text01; });
        txtDuration.color = Qt.binding(function() { return colorMap.text01; });

        txtDualRatio1.text = Qt.binding(function() { return ((phaseData === undefined) || (phaseData.ContrastPercentage === undefined)) ? 0 : phaseData.ContrastPercentage; });
        txtFlowRate.text = Qt.binding(function() { return ((phaseData === undefined) || (phaseData.FlowRate === undefined)) ? "0" : localeToFloatStr(phaseData.FlowRate, 1); });
        txtVolume.text = Qt.binding(function() { return ((phaseData === undefined) || (phaseData.TotalVolume === undefined)) ? "0" : Math.ceil(phaseData.TotalVolume); });
        txtDuration.text = Qt.binding(function() {  var duration = ( (phaseData !== undefined) && (phaseData.Duration !== undefined) ) ? phaseData.Duration : "00:00";
                                                    var durationText = Util.getMinimisedDurationStr(duration);
                                                    if (durationText === "00:00")
                                                    {
                                                        durationText = "00:01";
                                                    }
                                                    return durationText;
        });

        if (!phaseDataChanged)
        {
            return;
        }

        phaseDataChanged = false;
        updateDragImage();
    }

    function resumeEdit()
    {
        var editParamInfo = injectionPlanEdit.getCurEditParamInfo();
        if ( (editParamInfo.State === "EDITING_PHASE_PARAM") &&
             (editParamInfo.StepIndex === stepIndex) &&
             (editParamInfo.PhaseIndex === phaseIndex) )
        {
            if (phaseData === undefined)
            {
                // Resume failed. phaseData deleted
                if (widgetInputPad.isOpen())
                {
                    widgetInputPad.close(false);
                }
            }
            else
            {
                //logDebug("Step[" + stepIndex + "].Phase[" + phaseIndex + "]: ResumeEdit");
                startEdit(editParamInfo.Type);
            }
        }
    }

    function startEdit(curEditType)
    {
        if (editType === curEditType)
        {
            return;
        }

        if (widgetInputPad.isOpen())
        {
            widgetInputPad.close(true);
        }

        //editValueOk = true;
        editType = curEditType;
        phaseEditType = curEditType;

        widgetInputPad.signalValueChanged.disconnect(slotInputPadValChanged);
        widgetInputPad.signalClosed.disconnect(slotInputPadClosed);

        if (editType == "FLOW")
        {
            widgetInputPad.openFloatPad(btnEditFlowRate, txtFlowRate, txtFlowRateUnit.text, 1, flowRateMin, flowRateMax);
        }
        else if (editType == "VOLUME")
        {
            var editVolumeMax = volumeMax;
            if ( (phaseData.ContrastPercentage > 0) && (phaseData.ContrastPercentage < 100) )
            {
                // dual phase
                editVolumeMax = volumeMax * 2;
            }
            widgetInputPad.openIntegerPad(btnEditVolume, txtVolume, txtVolumeUnit.text, volumeMin, editVolumeMax)
        }
        else if (editType == "DURATION")
        {
            widgetInputPad.openDurationPad(btnEditDuration, txtDuration, delayMsMin, delayMsMax);
        }
        else if (editType == "DUAL_RATIO")
        {
            widgetInputPad.openDualRatioPad(btnEditDualRatio, txtDualRatio1);
        }

        widgetInputPad.signalValueChanged.connect(slotInputPadValChanged);
        widgetInputPad.signalClosed.connect(slotInputPadClosed);

        injectionPlanEdit.setCurEditParamInfo({ "State": "EDITING_PHASE_PARAM", "StepIndex": stepIndex, "PhaseIndex": phaseIndex, "Type": editType });
    }

    function slotInputPadValChanged(newValue)
    {
        calculateParams();
    }

    function closeInputPad(modified)
    {
        if (modified)
        {
            updateParamValues();
        }

        editType = "NULL";
        phaseEditType = "NULL";
        reload();
    }

    function slotInputPadClosed(modified)
    {
        widgetInputPad.signalValueChanged.disconnect(slotInputPadValChanged);
        widgetInputPad.signalClosed.disconnect(slotInputPadClosed);

        closeInputPad(modified);

        if (modified)
        {
            injectionPlanEdit.savePlan(planEditData);
            injectionPlanEdit.signalStepReload(stepIndex);
        }

        injectionPlanEdit.setCurEditParamInfo({ "State": "IDLE" });
    }

    function calculateParams()
    {
        if ( (editType == "FLOW") ||
             (editType == "VOLUME") )
        {
            var flowRate = parseFloat(txtFlowRate.text);
            var volume = parseFloat(txtVolume.text);
            var durationMs = (flowRate === 0) ? 0 : (volume / flowRate) * 1000;
            var durationText = Util.getMinimisedDurationStr(Util.millisecToDurationStr(durationMs));
            if (durationText === "00:00")
            {
                durationText = "00:01";
            }
            txtDuration.text = durationText;
        }
    }

    function updateParamValues()
    {
        if (phaseData.Type === "Fluid")
        {
            if ( (phaseData.ContrastPercentage > 0) ||
                 (phaseData.ContrastPercentage < 100) )
            {
                var dualRatioStr = txtDualRatio1.text;
                dualRatioStr = dualRatioStr.replace(/%/g, "");
                planEditData.Steps[stepIndex].Phases[phaseIndex].ContrastPercentage = parseInt(dualRatioStr);
            }

            var flowRate = localeFromFloatStr(txtFlowRate.text);
            planEditData.Steps[stepIndex].Phases[phaseIndex].FlowRate = flowRate;

            var volume = parseInt(txtVolume.text);
            planEditData.Steps[stepIndex].Phases[phaseIndex].TotalVolume = volume;

            var durationMs = (flowRate === 0) ? 0 : (volume / flowRate) * 1000;
            planEditData.Steps[stepIndex].Phases[phaseIndex].Duration = Util.millisecToDurationStr(durationMs);

            //logDebug("durationMs=" + durationMs + ", planEditData.Steps[stepIndex].Phases[phaseIndex].Duration=" + planEditData.Steps[stepIndex].Phases[phaseIndex].Duration)
        }
        else if (phaseData.Type === "Delay")
        {
            var delayMs = Util.durationStrToMillisec(txtDuration.text);
            planEditData.Steps[stepIndex].Phases[phaseIndex].Duration = Util.millisecToDurationStr(delayMs);
        }

        // Note: phaseData shall be updated from reloadAll()
    }
}




