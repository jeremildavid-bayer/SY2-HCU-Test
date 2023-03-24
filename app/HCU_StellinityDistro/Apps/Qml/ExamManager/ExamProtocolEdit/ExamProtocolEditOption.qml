import QtQuick 2.12
import "../../Util.js" as Util

Rectangle {
    property var plan: dsExam.plan
    property var selectedContrast: dsExam.selectedContrast
    property var executedSteps: dsExam.executedSteps
    property var executingStep: dsExam.executingStep
    property int stepCountMax: dsCapabilities.stepCountMax
    property var prevSelectedContrast
    property string itemType: "NONE" // "NONE", "TEST_INJECTION", "NEW_STEP", "CONTRAST_PHASE", "DUAL_PHASE", "SALINE_PHASE", "DELAY_PHASE"
    property string iconBackgroundColor1: getIconBackgroundColor(true)
    property string iconBackgroundColor2: getIconBackgroundColor(false)
    property bool isTitleFontBold: ( (itemType == "TEST_INJECTION") || (itemType == "NEW_STEP") )
    property string titleText: getTitleText()
    property string iconText: getIconText()
    property int animationMs: 200

    id: root
    visible: false
    color: "transparent"
    radius: {
        if ( (itemType == "TEST_INJECTION") ||
             (itemType == "NEW_STEP") )
        {
            return buttonRadius;
        }
        return height * 0.5;
    }

    enabled: getEnabled()
    state: "IDLE"
    states: [
        State { name: "LOADING" },
        State { name: "IDLE" },
        State { name: "DRAGGING" },
        State { name: "DRAG_STOPPED" }
    ]

    transitions: [
        Transition {
            from: "DRAGGING"
            to: "DRAG_STOPPED"

            SequentialAnimation {
                ParallelAnimation {
                    NumberAnimation { target: dragObject; properties: 'x'; to: dragObject.dragStartPos.x; duration: animationMs; easing.type: Easing.InOutQuart}
                    NumberAnimation { target: dragObject; properties: 'y'; to: dragObject.dragStartPos.y; duration: animationMs; easing.type: Easing.InOutQuart}
                    NumberAnimation { target: draggingStateCover; properties: 'opacity'; to: 0; duration: animationMs }
                }

                ScriptAction { script: {
                        // Plan data might be changed while dragging
                        dragObject.close();
                        root.state = "IDLE";
                    }
                }
            }
        },

        Transition {
            from: "IDLE"
            to: "DRAGGING"

            SequentialAnimation {
                ParallelAnimation {
                    NumberAnimation { target: draggingStateCover; properties: 'opacity'; to: 1; duration: animationMs * 0.5 }
                }
            }
        }
    ]

    Rectangle {
        id: mainRect
        anchors.fill: parent
        radius: root.radius
        border.width: root.enabled ? 0 : 2
        border.color: root.enabled ? "transparent" : colorMap.grid
        gradient: Gradient {
            GradientStop { position: 0.0; color: iconBackgroundColor1 }
            GradientStop { position: 0.50; color: iconBackgroundColor1 }
            GradientStop { position: 0.51; color: iconBackgroundColor2 }
            GradientStop { position: 1.0; color: iconBackgroundColor2 }
        }

        Text {
            id: icon
            x: width * 0.05
            width: parent.width * 0.2
            height: parent.height
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            color: root.enabled ? colorMap.text01 : colorMap.grid
            font.pixelSize: parent.height * 0.45
            font.family: fontIcon.name
            text: iconText
        }

        Item {
            id: rectTitle
            width: iconDrag.x - x
            x: ( (itemType == "TEST_INJECTION") || (itemType == "NEW_STEP") ) ? 0 : icon.width
            height: parent.height

            Text {
                id: textTitle
                height: parent.height
                anchors.left: parent.left
                anchors.leftMargin: root.width * 0.06
                anchors.right: parent.right
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
                color: root.enabled ? colorMap.text01 : colorMap.grid
                font.family: isTitleFontBold ? fontRobotoBold.name : fontRobotoLight.name
                font.pixelSize: height * 0.35
                text: translate(titleText)
                elide: Text.ElideRight
            }
        }

        Text {
            id: iconDrag
            anchors.right: parent.right
            anchors.rightMargin: root.width * 0.05
            height: parent.height
            width: contentWidth
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignVCenter
            color: root.enabled ? colorMap.text01 : colorMap.grid
            font.pixelSize: parent.height * 0.3
            font.family: fontIcon.name
            text: "\ue949"
        }
    }

    Item {
        id: mainRectCover
        anchors.centerIn: parent
        width: parent.width + 2
        height: parent.height + 2
        visible: root.enabled

        Rectangle {
            id: rectTitleCoverBackground
            height: parent.height
            anchors.right: parent.right
            width: root.width - rectTitle.width + radius
            color: colorMap.comboBoxBackground
            radius: root.radius
        }

        Rectangle {
            id: rectTitleCover
            height: parent.height
            color: colorMap.comboBoxBackground
            radius: ( (itemType == "TEST_INJECTION") || (itemType == "NEW_STEP") ) ? root.radius : 0
            x: (root.enabled) ? rectTitle.x : 0
            width: (root.enabled) ? rectTitle.width : 0

            Text {
                id: textTitleCover
                height: textTitle.height
                x: textTitle.x
                width: textTitle.width
                horizontalAlignment: textTitle.horizontalAlignment
                verticalAlignment: textTitle.verticalAlignment
                color: colorMap.text01
                font.family: textTitle.font.family
                font.pixelSize: textTitle.font.pixelSize
                text: textTitle.text
                elide: textTitle.elide
            }
        }

        Text {
            height: iconDrag.height
            anchors.right: parent.right
            anchors.rightMargin: root.width * 0.05
            width: iconDrag.width
            horizontalAlignment: iconDrag.horizontalAlignment
            verticalAlignment: iconDrag.verticalAlignment
            color: colorMap.text01
            font.pixelSize: iconDrag.font.pixelSize
            font.family: iconDrag.font.family
            text: iconDrag.text
        }
    }

    Rectangle {
        id: draggingStateCover
        opacity: 0
        radius: root.radius
        anchors.fill: parent
        color: colorMap.injectPhaseProgressBackground
    }

    Image {
        id: imageDrag
        anchors.fill: parent
        visible: false
    }

    MouseArea {
        id: dragHandle
        anchors.fill: parent
        drag.target: dragObject

        onPressed: {
            //logDebug("ExamProtocolEditOption[" + itemType + "]: Pressed: root.state=" + root.state);

            if (root.state != "IDLE")
            {
                logDebug("ExamProtocolEditOption[" + itemType + "]: Pressed: State=" + root.state + ", event ignored");
                return;
            }

            if (dragObject.isOpen())
            {
                logDebug("ExamProtocolEditOption[" + itemType + "]: Pressed: dragObject.isOpen, event ignored");
                return;
            }

            soundPlayer.playPressGood();
            root.state = "DRAGGING";

            dragObject.open(root, imageDrag.source);

            if ( (itemType === "TEST_INJECTION") ||
                 (itemType === "NEW_STEP") )
            {
                var injectionName = translate((itemType === "TEST_INJECTION") ? "T_TestInjection" : "T_Injection");
                var contrastPercentage = (itemType === "TEST_INJECTION") ? 0 : 100;
                var pressureLimit;
                if (executingStep !== undefined)
                {
                    pressureLimit = executingStep.PressureLimit;
                }
                else
                {
                    // current executing step is not defined. set last one
                    var lastExecutingStep = plan.Steps[executedSteps.length - 1];
                    pressureLimit = lastExecutingStep.PressureLimit;
                }

                dragObject.userData = { IsTestInjection: (itemType === "TEST_INJECTION"),
                                        PressureLimit: pressureLimit,
                                        Name: injectionName,
                                        ContrastFluidLocationName: selectedContrast.Location,
                                        Phases:[ { Type: "Fluid", ContrastPercentage: contrastPercentage } ] };


            }
            else if (itemType === "CONTRAST_PHASE")
            {
                dragObject.userData = { Type: "Fluid", ContrastPercentage: 100 };
            }
            else if (itemType === "DUAL_PHASE")
            {
                dragObject.userData = { Type: "Fluid", ContrastPercentage: 50 };
            }
            else if (itemType === "SALINE_PHASE")
            {
                dragObject.userData = { Type: "Fluid", ContrastPercentage: 0 };
            }
            else if (itemType === "DELAY_PHASE")
            {
                dragObject.userData = { Type: "Delay" };
            }

            if ( (itemType === "TEST_INJECTION") ||
                 (itemType === "NEW_STEP") )
            {
                injectionPlanEdit.setCurEditParamInfo({ "State": "INSERTING_NEW_STEP"});
            }
            else if ( (itemType === "CONTRAST_PHASE") ||
                      (itemType === "DUAL_PHASE") ||
                      (itemType === "SALINE_PHASE") ||
                      (itemType === "DELAY_PHASE") )
            {
                injectionPlanEdit.setCurEditParamInfo({ "State": "INSERTING_NEW_PHASE"});
            }
        }

        onReleased: {
            //logDebug("ExamProtocolEditOption[" + itemType + "]: Released: root.state=" + root.state);

            if ( (!dragObject.isOpen()) &&
                 (root.state !== "DRAGGING") )
            {
                logDebug("ExamProtocolEditOption[" + itemType + "]: Released: dragObject.isOpen()=" + (dragObject.isOpen() ? "TRUE" : "FALSE") + ", State=" + root.state + ", event ignored");
                return;
            }

            if (root.state == "DRAGGING")
            {
                root.state = "DRAG_STOPPED";
            }
            dragObject.released();
        }

        onClicked: {
            //logDebug("ExamProtocolEditOption[" + itemType + "]: Clicked: root.state=" + root.state);

            if (root.state !== "DRAG_STOPPED")
            {
                logDebug("ExamProtocolEditOption[" + itemType + "]: Clicked: State=" + root.state + ", event ignored");
                if (dragObject.isOpen())
                {
                    logDebug("ExamProtocolEditOption[" + itemType + "]: Clicked: State=" + root.state + ", event ignored: dragObject closing..");
                    dragObject.close();
                }
                return;
            }

            // Add new step or new phase
            if (itemType == "TEST_INJECTION")
            {
                stepList.newStepRow = executedSteps.length;
                plan.Steps.splice(stepList.newStepRow, 0, dragObject.userData);
                injectionPlanEdit.savePlan(plan);
            }
            else if (itemType == "NEW_STEP")
            {
                stepList.newStepRow = plan.Steps.length;
                plan.Steps.splice(stepList.newStepRow, 0, dragObject.userData);
                injectionPlanEdit.savePlan(plan);
            }
            else if ( (itemType == "CONTRAST_PHASE") ||
                      (itemType == "DUAL_PHASE") ||
                      (itemType == "SALINE_PHASE") ||
                      (itemType == "DELAY_PHASE") )
            {
                if (plan.Steps.length > 0)
                {
                    var stepIdx = plan.Steps.length - 1;
                    var curStep = plan.Steps[stepIdx];
                    if (stepIdx < executedSteps.length)
                    {
                        // Can't add phase to executed step
                    }
                    else if ( (curStep.IsTestInjection) &&
                              ( (itemType == "CONTRAST_PHASE") || (itemType == "DUAL_PHASE") || (itemType == "DELAY_PHASE") ) )
                    {
                        // Can't add contrast phase to test inject step
                    }
                    else
                    {
                        var phaseIdx = plan.Steps[stepIdx].Phases.length;

                        if (plan.Steps[stepIdx].Phases.length < dsCapabilities.phaseCountMax)
                        {
                            stepList.newStepRow = stepIdx;
                            stepList.newPhaseRow = phaseIdx;
                            plan.Steps[stepIdx].Phases.splice(phaseIdx, 0, dragObject.userData);
                            injectionPlanEdit.savePlan(plan);
                        }
                    }
                }
            }
        }
    }

    onPlanChanged: {
        reload();
    }

    onSelectedContrastChanged: {
        if ( (selectedContrast === undefined) ||
             (selectedContrast.FluidPackage === undefined) )
        {
            return;
        }

        if (prevSelectedContrast === undefined)
        {
            reload();
        }
        else if (!Util.compareObjects(selectedContrast, prevSelectedContrast))
        {
            reload();
        }
        prevSelectedContrast = selectedContrast;
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
        visible = (appMain.screenState === "ExamManager-ProtocolModification");
        reload();
    }

    function reload()
    {
        if (!visible)
        {
            state = "IDLE";
            return;
        }

        state = "LOADING";

        dragObject.close();
        draggingStateCover.opacity = 0;


        // Give a little moment (component may get deleted soon) and capture the image.
        timerSingleShot(10, function() {
            if ( (mainRect != null) && (mainRect.visible) && (mainRect.width > 0) && (mainRect.height > 0) )
            {
                mainRect.grabToImage(function(result) {
                    imageDrag.source = result.url;
                    state = "IDLE";
                }, Qt.size(mainRect.width, mainRect.height));
            }
        });
    }


    function getEnabled()
    {
        if ( (plan === undefined) || (plan.Steps === undefined) )
        {
            return false;
        }

        if (itemType == "TEST_INJECTION")
        {
            if (plan.Steps.length >= stepCountMax)
            {
                return false;
            }
            else
            {
                var allowTestInjection = true;
                for (var stepIdx = 0; stepIdx < plan.Steps.length; stepIdx++)
                {
                    if (stepIdx < executedSteps.length)
                    {
                        // Executed
                        if (!plan.Steps[stepIdx].IsTestInjection)
                        {
                            // Non-test injection is executed
                            allowTestInjection = false;
                            break;
                        }
                    }
                    else
                    {
                        // Not exectued
                        if (plan.Steps[stepIdx].IsTestInjection)
                        {
                            // Test injection is already exist
                            allowTestInjection = false;
                            break;
                        }
                    }
                }

                if (!allowTestInjection)
                {
                    return false;
                }
            }
        }
        else if (itemType == "NEW_STEP")
        {
            // If step count is reached to max, disable item
            if (plan.Steps.length >= stepCountMax)
            {
                return false;
            }
        }
        else if ( (itemType == "SALINE_PHASE") ||
                  (itemType == "CONTRAST_PHASE") ||
                  (itemType == "DUAL_PHASE") ||
                  (itemType == "SALINE_PHASE") ||
                  (itemType == "DELAY_PHASE") )
        {
            if (plan.Steps.length <= executedSteps.length)
            {
                return false;
            }
        }
        return true;
    }

    function getIconBackgroundColor(isTop)
    {
        if (root.enabled)
        {
            if (itemType == "TEST_INJECTION")
            {
                return colorMap.comboBoxBackground;
            }
            else if (itemType == "NEW_STEP")
            {
                return colorMap.comboBoxBackground;
            }
            else if (itemType == "CONTRAST_PHASE")
            {
                return (selectedContrast.ColorCode === "GREEN") ? colorMap.contrast1 : colorMap.contrast2;
            }
            else if (itemType == "DUAL_PHASE")
            {
                if (isTop)
                {
                    return (selectedContrast.ColorCode === "GREEN") ? colorMap.contrast1 : colorMap.contrast2;
                }
                else
                {
                    return colorMap.saline;
                }
            }
            else if (itemType == "SALINE_PHASE")
            {
                return colorMap.saline;
            }
            else if (itemType == "DELAY_PHASE")
            {
                return colorMap.gry01;
            }
        }
        return "transparent";
    }

    function getIconText()
    {
        if (itemType == "CONTRAST_PHASE")
        {
            return "\ue92f";
        }
        else if (itemType == "DUAL_PHASE")
        {
            return "\ue94b";
        }
        else if (itemType == "SALINE_PHASE")
        {
            return "\ue930";
        }
        else if (itemType == "DELAY_PHASE")
        {
            return "\ue94c";
        }
        return "";
    }

    function getTitleText()
    {
        if (itemType == "TEST_INJECTION")
        {
            return "T_TestInjection";
        }
        else if (itemType == "NEW_STEP")
        {
            return "T_Injection";
        }
        else if (itemType == "CONTRAST_PHASE")
        {
            if ( (selectedContrast.FluidPackage !== undefined) &&
                 (selectedContrast.FluidPackage.SourcePackages !== undefined) &&
                 (selectedContrast.FluidPackage.SourcePackages.length > 0) )
            {
                return selectedContrast.FluidPackage.SourcePackages[0].Brand + " " + selectedContrast.FluidPackage.SourcePackages[0].Concentration;
            }
            else
            {
                return "T_Contrast";
            }
        }
        else if (itemType == "DUAL_PHASE")
        {
            return "T_PhaseTypeName_DualFlow";
        }
        else if (itemType == "SALINE_PHASE")
        {
            return "T_Saline";
        }
        else if (itemType == "DELAY_PHASE")
        {
            return "T_PhaseTypeName_Delay";
        }
        return "";
    }
}
