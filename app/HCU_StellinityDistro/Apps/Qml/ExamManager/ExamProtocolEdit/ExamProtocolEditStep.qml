import QtQuick 2.12
import "../../Widgets"
import "../../Util.js" as Util
import "ExamProtocolEdit.js" as ExamProtocolEdit

Item {
    property var plan: dsExam.plan
    property var executedSteps: dsExam.executedSteps
    property var activeAlerts: dsAlert.activeAlerts
    property int phaseCountMax: dsCapabilities.phaseCountMax
    property int stepIndex: index // current row index (changes during sorting)
    property int stepIndexOriginal: {
        if ( (plan !== undefined) && (stepData !== undefined) )
        {
            for (var stepIdx = 0; stepIdx < plan.Steps.length; stepIdx++)
            {
                if (plan.Steps[stepIdx].GUID === stepData.GUID)
                {
                    return stepIdx;
                }
            }
        }
        return stepIndex;
    }

    property var stepData
    property bool stepDataChanged: false
    property int listHeight: ListView.view.height
    property int rowHeight: (stepData === undefined) ? 0 : (mainRect.y + mainRect.height + bottomSpacing)
    property int stepNameHeight: (stepData === undefined) ? 0 : listHeight * 0.08
    property int phaseListHeight: (stepData === undefined) ? 0 : (stepData.Phases.length * (phaseHeight + rowSpacing))
    property int phaseHeight: (stepData === undefined) ? 0 : (listHeight * 0.12)
    property int phaseWidth: (stepData === undefined) ? 0 : (mainRect.width * 0.92)
    property int rowSpacing: listHeight * 0.02
    property int bottomSpacing: listHeight * 0.02
    property int borderWidth: 1
    property int dummyPhaseIdx: -1
    property string phaseEditType: "NULL" // NULL, FLOW, VOLUME, DURATION
    property int animationDeleteMs: 250
    property int animationAddMs: 250
    property string prevPlanGuid: ""
    property bool isActiveStep: (stepIndexOriginal === executedSteps.length)

    id: injectionStepEdit
    width: ListView.view.width
    height: rowHeight
    visible: stepData !== undefined

    Rectangle {
        id: mainRect
        width: parent.width * 0.97
        height: listViewPhase.y + listViewPhase.height + rowSpacing
        radius: buttonRadius
        color: ((stepData === undefined) || (stepData.GUID !== "Dummy")) ? "transparent" : colorMap.buttonShadow
        border.width: ((stepData === undefined) || (stepData.GUID !== "Dummy")) ? borderWidth : 0
        border.color: {
            var nextExecuteStepIdx = executedSteps.length;

            if (stepIndexOriginal === injectionStepReview.stepIndex)
            {
                return colorMap.text01;
            }
            else if (stepIndexOriginal < nextExecuteStepIdx)
            {
                return colorMap.grid;
            }
            else if (stepIndexOriginal === nextExecuteStepIdx)
            {
                return colorMap.actionButtonBackground;
            }
            else
            {
                return colorMap.text02;
            }
        }

        Image {
            id: imageDrag
            anchors.fill: parent
            visible: false
        }

        Item {
            id: btnDrag
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: parent.width * 0.05
            width: parent.width * 0.05
            height: stepNameHeight
            visible: getDragBtnVisibleState()

            Text {
                anchors.fill: parent
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                color: colorMap.text02
                font.pixelSize: height * 0.8
                font.family: fontIcon.name
                text: "\ue94a"
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
                    dragObject.userData = Util.copyObject(stepData);

                    injectionPlanEdit.setCurEditParamInfo({ "State": "SORTING_STEP", "Index": stepIndex });

                    dummyStepIdx = stepIndex;

                    planEditData.Steps[stepIndex].GUID = "Dummy";

                    if (dummyStepCount == -1)
                    {
                        dummyStepCount = 1;
                    }
                    else
                    {
                        dummyStepCount++;
                    }

                    reloadAll(stepIndex);

                    mouse.accepted = false;
                }
            }

            onVisibleChanged: {
                if (visible)
                {
                    // If image preview is never initialised, start grab now
                    updateDragImage();
                }
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

            NumberAnimation { target: injectionStepEdit; properties: 'opacity'; to: 0; duration: animationDeleteMs; }
            ScriptAction { script: {
                    if (planEditData.Steps.length > 1)
                    {
                        var planEditDataBuf = Util.copyObject(planEditData);
                        planEditDataBuf.Steps.splice(stepIndex, 1);
                        setPlanEditData(planEditDataBuf);
                        injectionPlanEdit.reloadAllSteps();
                        injectionPlanEdit.savePlan(planEditData);
                    }
                    opacity = 1;
                }
            }
        }

        GenericButton {
            id: btnDelete
            visible: {
                if (stepData === undefined)
                {
                    return false;
                }
                else if (stepData.GUID === "Dummy")
                {
                    return false;
                }
                else if (stepIndex === dummyStepIdx)
                {
                    return false;
                }
                else if (stepIndex < executedSteps.length)
                {
                    return false;
                }
                //logDebug("planEditData.Steps.length=" + planEditData.Steps.length);
                return planEditData.Steps.length > 1;
            }

            anchors.right: parent.right
            anchors.rightMargin: phaseWidth * 0.022
            anchors.top: parent.top
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
                // Delete only if there are more than one steps
                animationDeleted.start();
            }
        }

        Text {
            visible: getCompletedIconVisibleState()
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.topMargin: height * 0.05
            height: phaseHeight * 0.9
            width: phaseHeight * 0.9
            color: getCompletedIconColor()
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: height * 0.5
            font.family: fontIcon.name
            text: "\ue915"
        }

        Rectangle {
            id: armSelectedBorder
            width: parent.width * 0.02
            height: parent.height
            x: parent.width - width
            color: "transparent"
            visible: {
                if ( (stepData === undefined) ||
                     (stepData.GUID === "Dummy") )

                {
                    return false;
                }

                var nextExecuteStepIdx = executedSteps.length;
                if (stepIndexOriginal === injectionStepReview.stepIndex)
                {
                    return false;
                }
                else if (stepIndexOriginal < nextExecuteStepIdx)
                {
                    return false;
                }
                else if (stepIndexOriginal === nextExecuteStepIdx)
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }

            Rectangle {
                width: parent.width * 0.5
                height: parent.height
                color: colorMap.actionButtonBackground
            }

            Rectangle {
                width: parent.width
                height: parent.height
                color: colorMap.actionButtonBackground
                radius: mainRect.radius
            }
        }

        Rectangle {
            id: reviewSelectedBorder
            width: parent.width * 0.02
            height: parent.height
            color: "transparent"
            visible: {
                if ( (stepData === undefined) ||
                     (stepData.GUID === "Dummy") )
                {
                    return false;
                }

                var nextExecuteStepIdx = executedSteps.length;
                if (stepIndex === injectionStepReview.stepIndex)
                {
                    return true;
                }
                return false;
            }

            Rectangle {
                x: parent.width * 0.5
                width: parent.width * 0.5
                height: parent.height
                color: colorMap.text01
            }

            Rectangle {
                width: parent.width
                height: parent.height
                color: colorMap.text01
                radius: mainRect.radius
            }
        }

        Text {
            id: txtStepId
            anchors.top: parent.top
            anchors.topMargin: height * 0.06
            anchors.left: parent.left
            anchors.leftMargin: parent.width * 0.05
            height: stepNameHeight
            text: (stepIndexOriginal + 1)
            color: {
                var nextExecuteStepIdx = executedSteps.length;
                if (stepIndexOriginal === injectionStepReview.stepIndex)
                {
                    return colorMap.text01;
                }
                else if (stepIndexOriginal < nextExecuteStepIdx)
                {
                    return colorMap.grid;
                }
                else if (stepIndexOriginal === nextExecuteStepIdx)
                {
                    return colorMap.actionButtonBackground;
                }
                return colorMap.text01;
            }
            opacity: ((stepData === undefined) || (stepData.GUID !== "Dummy")) ? 1 : 0

            font.family: fontRobotoBold.name
            font.pixelSize: height * 0.6
            verticalAlignment: Text.AlignBottom
            elide: Text.ElideRight
        }

        Image {
            id: personalizedImage
            anchors.left: txtStepId.right
            anchors.leftMargin: 20
            anchors.top: txtStepId.top
            anchors.topMargin: 25
            source: getPersonalizedImageType()
            width: height * 1.6
            height: visible ? txtStepId.height * 0.45 : 0
            sourceSize.width: width
            sourceSize.height: height
            visible: getPersonalizedImageVisibleState()
            opacity:
            {
                var nextExecuteStepIdx = executedSteps.length;
                if (stepIndexOriginal === injectionStepReview.stepIndex)
                {
                    return 1;
                }
                else if (stepIndexOriginal < nextExecuteStepIdx)
                {
                    return 0.7;
                }
                else if (stepIndexOriginal === nextExecuteStepIdx)
                {
                    return 1;
                }
                return 1;
            }
        }

        Text {
            id: txtStepName
            anchors.top: parent.top
            anchors.topMargin: height * 0.06
            anchors.left: (personalizedImage.visible) ? personalizedImage.right : txtStepId.right
            anchors.leftMargin: (personalizedImage.visible) ? 5 : 20
            anchors.right: iconIsNotScannerSynchronized.right
            anchors.rightMargin: parent.width * 0.02
            height: stepNameHeight
            text: (stepData === undefined || stepData.Name === undefined) ? "" : (stepData.Name)
            color: {
                var nextExecuteStepIdx = executedSteps.length;
                if (stepIndexOriginal === injectionStepReview.stepIndex)
                {
                    return colorMap.text01;
                }
                else if (stepIndexOriginal < nextExecuteStepIdx)
                {
                    return colorMap.grid;
                }
                else if (stepIndexOriginal === nextExecuteStepIdx)
                {
                    return colorMap.actionButtonBackground;
                }
                return colorMap.text01;
            }
            opacity: ((stepData === undefined) || (stepData.GUID !== "Dummy")) ? 1 : 0
            font.family: fontRobotoBold.name
            font.pixelSize: height * 0.6
            verticalAlignment: Text.AlignBottom
            elide: Text.ElideRight
        }

        Text {
            id: iconIsPreloaded
            anchors.top: parent.top
            anchors.topMargin: txtStepName.height * 0.02
            anchors.right: iconIsNotScannerSynchronized.left
            anchors.rightMargin: parent.width * 0.025
            width: contentWidth
            height: txtStepName.height - anchors.topMargin
            verticalAlignment: Text.AlignBottom
            font.family: fontIcon.name
            font.bold: true
            color: txtStepName.color
            font.pixelSize: height * 0.65
            text: ((stepData !== undefined) && (stepData.IsPreloaded)) ? "\ue983" : ""
        }

        Text {
            id: iconIsNotScannerSynchronized
            anchors.top: parent.top
            anchors.topMargin: txtStepName.height * 0.02
            anchors.right: btnDelete.left
            anchors.rightMargin: parent.width * 0.01
            width: contentWidth
            height: txtStepName.height - anchors.topMargin
            verticalAlignment: Text.AlignBottom
            font.family: fontIcon.name
            font.bold: true
            color: txtStepName.color
            font.pixelSize: height * 0.8
            text: getIsNotScannerSynchronizedVisibleState() ? "\ue910" : ""
        }

        ExamProtocolEditParamList {
            id: personalizedParamList
            z: btnStepReview.z + 1
            anchors.left: parent.left
            anchors.leftMargin: parent.width * 0.09
            anchors.right: parent.right
            anchors.rightMargin: parent.width * 0.05
            anchors.top: txtStepName.bottom
            anchors.topMargin: injectionPlanEdit.height * 0.03
            paramItems: (stepData === undefined) ? [] : stepData.PersonalizationInputs
            opacity: ((stepData === undefined) || (stepData.GUID !== "Dummy")) ? 1 : 0
            isActive: isActiveStep
            name: "Step" + stepIndex
            isPlanLevel: false
        }

        ListView {
            id: listViewPhase
            cacheBuffer: height
            x: parent.width - width
            anchors.top: personalizedParamList.bottom
            anchors.topMargin: (personalizedParamList.height == 0) ? 0 : injectionPlanEdit.height * 0.04
            width: phaseWidth
            height: phaseListHeight
            model: phaseCountMax
            delegate: ExamProtocolEditPhase {}
            interactive: false
            opacity: ((stepData === undefined) || (stepData.GUID !== "Dummy")) ? 1 : 0
        }

        GenericButton {
            id: btnStepReview
            anchors.fill: parent
            interactive: {
                var nextExecuteStepIdx = executedSteps.length;
                if (stepIndex === injectionStepReview.stepIndex)
                {
                    return true;
                }
                else if (stepIndex < nextExecuteStepIdx)
                {
                    return true;
                }
                else if (stepIndex === nextExecuteStepIdx)
                {
                    return false;
                }
                return false;
            }

            onBtnClicked: {
                injectionStepReview.open(stepIndex);
            }
        }
    }

    SequentialAnimation {
        id: animationAdded
        NumberAnimation { target: injectionStepEdit; properties: 'opacity'; from: 1; to: 0; duration: animationAddMs }
        NumberAnimation { target: injectionStepEdit; properties: 'opacity'; from: 0; to: 1; duration: animationAddMs }
        NumberAnimation { target: injectionStepEdit; properties: 'opacity'; from: 1; to: 0; duration: animationAddMs }
        NumberAnimation { target: injectionStepEdit; properties: 'opacity'; from: 0; to: 1; duration: animationAddMs }
    }

    onPlanChanged: {
        if ( (plan === undefined) ||
             (plan.GUID === undefined) )
        {
            return;
        }

        if (plan.GUID !== prevPlanGuid)
        {
            //logDebug("ExamProtocolEditStep[" + stepIndex + "]: Plan selected from " + prevPlanGuid + " to " + plan.GUID);

            // Reload paramItems to reload all personalized param list
            personalizedParamList.paramItems = [];
            personalizedParamList.paramItems = Qt.binding(function() { return (stepData === undefined) ? [] : stepData.PersonalizationInputs; });
            personalizedParamList.handleIsActive();
            prevPlanGuid = plan.GUID;
        }
    }

    onActiveAlertsChanged: {
        reloadPersonalizedImage();
    }


    Component.onCompleted: {
        listViewStep.dragStarted.connect(resetButtons);
        dragObject.xChanged.connect(onDragObjectMoved);
        dragObject.yChanged.connect(onDragObjectMoved);
        dragObject.signalReleased.connect(onDragObjectReleased);
        injectionPlanEdit.signalStepReload.connect(reloadAll);
    }

    Component.onDestruction: {
        listViewStep.dragStarted.disconnect(resetButtons);
        dragObject.xChanged.disconnect(onDragObjectMoved);
        dragObject.yChanged.disconnect(onDragObjectMoved);
        dragObject.signalReleased.disconnect(onDragObjectReleased);
        injectionPlanEdit.signalStepReload.disconnect(reloadAll);
    }

    function reloadAll(stepIdx)
    {
        if (stepIdx === stepIndex)
        {
            if (!Util.compareObjects(stepData, planEditData.Steps[stepIndex]))
            {
                stepData = Util.copyObject(planEditData.Steps[stepIndex]);
                stepDataChanged = true;
            }
            reload();
        }

        if ( (stepIndex == newStepRow) && (newPhaseRow == -1) )
        {
            //logDebug("Step[" + stepIndex + "] stepData: AnimationAddedStart");
            animationAdded.start();
        }
    }

    function resetButtons()
    {
        btnDelete.reset();
        btnStepReview.reset();
    }

    function reload()
    {
        if (!stepDataChanged)
        {
            return;
        }

        if (stepData === undefined)
        {
            return;
        }

        if (stepData.GUID === "Dummy")
        {
            return;
        }

        for (var phaseIdx = 0; phaseIdx < listViewPhase.model; phaseIdx++)
        {
            injectionPlanEdit.signalPhaseReload(stepIndex, phaseIdx);
        }

        // update dummyPhaseIdx
        dummyPhaseIdx = -1;
        for (var i = 0; i < stepData.Phases.length; i++)
        {
            if (stepData.Phases[i].Type === "Dummy")
            {
                dummyPhaseIdx = i;
                break;
            }
        }

        stepDataChanged = false;
        updateDragImage();
        reloadPersonalizedImage();
    }

    function reloadPersonalizedImage()
    {
        if (plan === undefined || activeAlerts === undefined)
        {
            return;
        }

        var deparameterized = false;
        for (var alertIdx = 0; alertIdx < activeAlerts.length; alertIdx++)
        {
            if (activeAlerts[alertIdx].CodeName === "DeparameterizedProtocol")
            {
                deparameterized = true;
                break;
            }
        }
        personalizedImage.visible = !deparameterized && getPersonalizedImageVisibleState();
    }

    function updateDragImage()
    {
        // Give a little moment (component may get deleted soon) and capture the image.
        timerSingleShot(10, function() {
            if ( (mainRect != null) && (mainRect.visible) && (mainRect.width > 0) && (mainRect.height > 0) )
            {
                //logDebug("ExamProtocolEditStep[" + stepIndex + "]: updateDragImage() Started");
                mainRect.grabToImage(function(result) {
                    imageDrag.source = result.url;
                    //logDebug("ExamProtocolEditStep[" + stepIndex + "]: updateDragImage() Done");
                }, Qt.size(mainRect.width, mainRect.height));
            }
        });
    }

    function getDragBtnVisibleState()
    {
        if (stepData === undefined)
        {
            return false;
        }
        else if (stepData.GUID === "Dummy")
        {
            return false;
        }
        else if (stepIndex === dummyStepIdx)
        {
            return false;
        }
        else if (stepData.IsTestInjection)
        {
            return false;
        }
        else if (stepIndex < executedSteps.length)
        {
            return false;
        }
        else if (planEditData.Steps.length <= 1)
        {
            return false;
        }
        return true;
    }

    function getCompletedIconVisibleState()
    {
        if (stepData === undefined)
        {
            return false;
        }
        else if (stepIndex >= executedSteps.length)
        {
            return false;
        }
        return true;
    }

    function getCompletedIconColor()
    {
        if ( (stepData === undefined) ||
             (stepIndex >= executedSteps.length) )
        {
            return txtStepName.color;
        }

        var terminationReason = executedSteps[stepIndex].TerminationReason;
        if (terminationReason === "Normal")
        {
            return txtStepName.color;
        }
        else
        {
            return colorMap.errText;
        }
    }

    function getPersonalizedImageType()
    {
        var containsModifier = false;
        var containsGenerator = false;
        var imgSource = imageMap.examProtocolPersonalised;

        if (getPersonalizedImageVisibleState() === true)
        {
            for (var modifierIdx = 0 ; modifierIdx < stepData.PersonalizationModifiers.length ; modifierIdx++ )
            {
                if (stepData.PersonalizationModifiers[modifierIdx].includes("Tube Voltage Based Modifier"))
                {
                    containsModifier = true;
                    break;
                }
            }

            if (stepData.PersonalizationGenerator !== "")
            {
                containsGenerator = true;
            }

            if (containsModifier && containsGenerator)
            {
                imgSource = imageMap.examProtocolPersonalisedKVP;
            }
            else if (containsModifier)
            {
                imgSource = imageMap.examProtocolKVP;
            }
            else if (containsGenerator)
            {
                imgSource = imageMap.examProtocolPersonalised;
            }
            else
            {
                personalizedImage.visible = false;
            }
        }
        return imgSource;
    }

    function getPersonalizedImageVisibleState()
    {
        if (stepData === undefined || plan === undefined)
        {
           return false;
        }
        else if (stepData.GUID === "Dummy")
        {
            return false;
        }
        else if (stepIndex === dummyStepIdx)
        {
            return false;
        }
        else if (!plan.IsPersonalized)
        {
            return false;
        }
        else if (stepData.PersonalizationGenerator === "" && stepData.PersonalizationModifiers.length === 0)
        {
            return false;
        }
        return true;
    }

    function getIsNotScannerSynchronizedVisibleState()
    {
        if (stepData === undefined)
        {
            return false;
        }
        else if (stepData.GUID === "Dummy")
        {
            return false;
        }
        else if (stepIndex === dummyStepIdx)
        {
            return false;
        }
        else if (!stepData.IsNotScannerSynchronized)
        {
            return false;
        }
        return true;
    }

    function onDragObjectMoved()
    {
        if (!dragObject.isOpen())
        {
            // Drag Object is closed by other
            return;
        }

        updateScroll();

        var editParamInfo = injectionPlanEdit.getCurEditParamInfo();
        if ( (editParamInfo.State !== "INSERTING_NEW_PHASE") &&
             (editParamInfo.State !== "SORTING_PHASE") )
        {
            return;
        }

        if (planEditData.Steps[stepIndex] === undefined)
        {
            //logDebug("planEditData.Steps[" + stepIndex + "] === undefined, Not sorting")
            return;
        }

        sortPhase();
    }

    function onDragObjectReleased()
    {
        var editParamInfo = injectionPlanEdit.getCurEditParamInfo();

        if ( (editParamInfo.State !== "INSERTING_NEW_PHASE") &&
             (editParamInfo.State !== "SORTING_PHASE") )
        {
            return;
        }

        if (dummyPhaseIdx != -1)
        {
            dragObject.close();
            listViewStep.interactive = true;

            planEditData.Steps[stepIndex].Phases[dummyPhaseIdx] = dragObject.userData;

            injectionPlanEdit.signalStepReload(stepIndex);

            dummyPhaseCount--;
            dummyPhaseIdx = -1;

            //logDebug("Sort Phase done!");
            injectionPlanEdit.setCurEditParamInfo({ "State": "IDLE" });


            injectionPlanEdit.savePlan(planEditData);
        }
        else
        {
            if (stepIndex >= (planEditData.Steps.length - 1))
            {
                // current item is end row, set state to IDLE
                injectionPlanEdit.setCurEditParamInfo({ "State": "IDLE" });
            }
        }
    }

    function sortPhase()
    {
        var insertRowIndex = ExamProtocolEdit.getInsertRowIndex(dummyPhaseIdx, dragObject, listViewPhase, planEditData.Steps[stepIndex].Phases.length);

        //logDebug("Step" + stepIndex + ": SortPhase: " + " insertRowIndex=" + insertRowIndex + ", dummyPhaseIdx=" + dummyPhaseIdx);
        //logDebug("planEditData.Steps=" + JSON.stringify(planEditData.Steps));

        if (insertRowIndex !== -1)
        {
            if (dummyPhaseIdx == -1)
            {
                // Dummy is not present: Check condition for adding dummy..
                if (stepData.Phases.length >= dsCapabilities.phaseCountMax)
                {
                    // Max phase reached
                }
                else if (stepIndex < executedSteps.length)
                {
                    // Step already executed
                }
                else if ( (stepData.IsTestInjection) &&
                          (dragObject.userData.Type === "Fluid") &&
                          (dragObject.userData.ContrastPercentage !== 0) )
                {
                    // Cannot add contrast phase to test injection
                }
                else if ( (stepData.IsTestInjection) &&
                          (dragObject.userData.Type === "Delay") )
                {
                    // Cannot add contrast phase to test injection
                }
                else
                {
                    //logDebug("Step[" + stepIndex + "]: Inserting dummy to " + insertRowIndex);
                    planEditData.Steps[stepIndex].Phases.splice(insertRowIndex, 0, { Type: "Dummy" });
                    injectionPlanEdit.signalStepReload(stepIndex);
                    dummyPhaseIdx = insertRowIndex;
                    dummyPhaseCount++;
                }
            }
            else
            {
                // Dummy is present
                if (insertRowIndex >= planEditData.Steps[stepIndex].Phases.length)
                {
                    // Dummy is present, but bad new insert position
                    //logDebug("Step[" + stepIndex + "]: Shoud NOT moving dummy from " + dummyPhaseIdx + " to " + insertRowIndex);
                    return;
                }
                else if (insertRowIndex !== dummyPhaseIdx)
                {
                    //logDebug("Step[" + stepIndex + "]: Moving dummy from " + dummyPhaseIdx + " to " + insertRowIndex);

                    var swapIndex;

                    if (dummyPhaseIdx < insertRowIndex)
                    {
                        for (swapIndex = dummyPhaseIdx; swapIndex < insertRowIndex; swapIndex++)
                        {
                            //logDebug("Step[" + stepIndex + "]: Swap phases " + swapIndex + " <-> " + (swapIndex + 1));
                            planEditData.Steps[stepIndex].Phases = Util.swapListItem(planEditData.Steps[stepIndex].Phases, swapIndex, swapIndex + 1);
                        }
                    }
                    else
                    {
                        for (swapIndex = dummyPhaseIdx; swapIndex > insertRowIndex; swapIndex--)
                        {
                            //logDebug("Step[" + stepIndex + "]: Swap phases " + swapIndex + " <-> " + (swapIndex - 1));
                            planEditData.Steps[stepIndex].Phases = Util.swapListItem(planEditData.Steps[stepIndex].Phases, swapIndex, swapIndex - 1);
                        }
                    }

                    //logDebug("planEditData.Steps[stepIndex].Phases=" + JSON.stringify(planEditData.Steps[stepIndex].Phases));

                    var swapIndex1 = Math.min(dummyPhaseIdx, insertRowIndex);
                    var swapIndex2 = Math.max(dummyPhaseIdx, insertRowIndex);

                    for (swapIndex = swapIndex1; swapIndex <= swapIndex2; swapIndex++)
                    {
                        injectionPlanEdit.signalPhaseReload(stepIndex, swapIndex);
                    }
                    dummyPhaseIdx = insertRowIndex;
                }

            }
        }
        else
        {
            // Out from list, cancel insertion
            if (dummyPhaseIdx != -1)
            {
                var editParamInfo = injectionPlanEdit.getCurEditParamInfo();
                if ( (editParamInfo.State === "INSERTING_NEW_PHASE") ||
                     (dummyPhaseCount > 1) )
                {
                    // Drag object no longer located in this step, remove it
                    //logDebug("Step[" + index + "]: Removing dummy phase " + dummyPhaseIdx);

                    planEditData.Steps[stepIndex].Phases.splice(dummyPhaseIdx, 1);
                    injectionPlanEdit.signalStepReload(stepIndex);
                    dummyPhaseIdx = -1;
                    dummyPhaseCount--;
                }
            }
        }
    }
}




