import QtQuick 2.12
import "../../Widgets"
import "../../Util.js" as Util
import "ExamProtocolEdit.js" as ExamProtocolEdit

Item {
    property var executedSteps: dsExam.executedSteps
    property var planEditData
    property int phaseCountMax: dsCapabilities.phaseCountMax
    property int stepCountMax: dsCapabilities.stepCountMax

    property int dummyStepIdx: -1
    property int dummyStepCount: 0
    property int dummyPhaseCount: 0

    property int scrollSpeed: 600
    property string scrollDir: "NONE" // UP, DOWN

    property int newStepRow: -1
    property int newPhaseRow: -1
    property int optionAddedScrollSpeed: 250

    id: root

    MouseArea {
        anchors.fill: parent
        drag.target: dragObject

        onPositionChanged: {
            dragObject.x = dragObject.dragStartPos.x;
        }

        onReleased: {
            dragObject.released();
        }
    }

    ListView {
        property int scrollAreaHeight: height * 0.1
        property int lastHeaderHeight: 0

        id: listViewStep
        anchors.fill: parent
        clip: true
        cacheBuffer: Math.max(contentHeight * 2, height * 10)
        highlightMoveDuration: optionAddedScrollSpeed

        ScrollBar {}
        ListFade {
            headerSize: parent.headerItem.height
        }

        header: Item {
            width: parent.width
            height: headerPadding.y + headerPadding.height

            ExamProtocolEditParamList {
                id: personalizedParamList
                width: parent.width * 0.97
                anchors.top: parent.top
                paramItems: (planEditData === undefined) ? [] : planEditData.PersonalizationInputs
                isActive: true
                name: "Plan"
            }

            Item {
                id: headerPadding
                anchors.top: personalizedParamList.bottom
                width: parent.width
                height: (personalizedParamList.height === 0) ? 0 : root.height * 0.04
            }

            onHeightChanged: {
                var heightChanged = height - listViewStep.lastHeaderHeight;
                if (heightChanged > 0)
                {
                    listViewStep.contentY -= heightChanged;
                }
                listViewStep.lastHeaderHeight = height;
            }
        }

        footer: Item {
            width: parent.width
            height: root.height * 0.05
        }

        model: stepCountMax
        delegate: ExamProtocolEditStep {}

        onHeightChanged: {
            focusActiveStep();
        }
    }

    Timer {
        id: scrollTimer
        interval: 50
        onTriggered: {
            var editParamInfo = injectionPlanEdit.getCurEditParamInfo();
            if ( (editParamInfo.State !== "INSERTING_NEW_STEP") &&
                 (editParamInfo.State !== "SORTING_STEP") &&
                 (editParamInfo.State !== "INSERTING_NEW_PHASE") &&
                 (editParamInfo.State !== "SORTING_PHASE") )
            {
                // drag object is already released, stop scroll
                return;
            }

            if ( (scrollDir === "UP") && (listViewStep.contentY > 0) )
            {
                listViewStep.flick(0, scrollSpeed);
                scrollTimer.start();
            }
            else if ( (scrollDir === "DOWN") && ((listViewStep.visibleArea.yPosition + listViewStep.visibleArea.heightRatio) < 1) )
            {
                listViewStep.flick(0, -scrollSpeed);
                scrollTimer.start();
            }
        }
    }

    Timer {
        id: scrollNewStepTimer
        interval: 100
        onTriggered: {
            listViewStep.positionViewAtIndex(listViewStep.currentIndex, ListView.SnapPosition);
        }
    }


    onPlanEditDataChanged: {
        reload();
    }

    Component.onCompleted: {
        dragObject.xChanged.connect(onDragObjectMoved);
        dragObject.yChanged.connect(onDragObjectMoved);
        dragObject.signalReleased.connect(onDragObjectReleased);
        appMain.screenStateChanged.connect(slotScreenStateChanged);
        slotScreenStateChanged();
    }

    Component.onDestruction: {
        dragObject.xChanged.disconnect(onDragObjectMoved);
        dragObject.yChanged.disconnect(onDragObjectMoved);
        dragObject.signalReleased.disconnect(onDragObjectReleased);
        appMain.screenStateChanged.disconnect(slotScreenStateChanged);
    }

    function slotScreenStateChanged()
    {
        visible = (appMain.screenState === "ExamManager-ProtocolModification");

        if (visible)
        {
            reload();
            focusActiveStep();
        }
    }

    function reloadAll()
    {
        for (var stepIdx = 0; stepIdx < stepCountMax; stepIdx++)
        {
            injectionPlanEdit.signalStepReload(stepIdx);
        }
        scrollToNewStepRow();
    }

    function reload()
    {
        if (planEditData === undefined)
        {
            return;
        }

        injectionPlanEdit.reloadAllSteps();

        // Get dummy indexes and count values
        dummyStepIdx = -1;
        dummyStepCount = 0;
        dummyPhaseCount = 0;

        for (var i = 0; i < planEditData.Steps.length; i++)
        {
            if (planEditData.Steps[i].GUID === "Dummy")
            {
                dummyStepIdx = i;
                dummyStepCount++;
            }

            for (var j = 0; j < planEditData.Steps[i].Phases.length; j++)
            {
                if (planEditData.Steps[i].Phases[j].Type === "Dummy")
                {
                    dummyPhaseCount++;
                }
            }
        }
    }

    function scrollToNewStepRow()
    {
        if (newStepRow != -1)
        {
            if (listViewStep.currentIndex !== newStepRow)
            {
                // new step added
                listViewStep.currentIndex = newStepRow;
            }
            else
            {
                // new phase added
                scrollNewStepTimer.stop();
                scrollNewStepTimer.start();
            }
        }
        newStepRow = -1;
        newPhaseRow = -1;
    }

    function focusActiveStep()
    {
        var stepIndex = executedSteps.length;

        // Ensure next executed step is highlighed in the list
        //logDebug("focusActiveStep: stepIdx=" + stepIndex + ", listViewStep.height=" + listViewStep.height + ", listViewStep.footerHeight=" + listViewStep.footerHeight);
        if (stepIndex == 0)
        {
            listViewStep.positionViewAtBeginning();
        }
        else
        {
            listViewStep.positionViewAtIndex(stepIndex, ListView.Center);
        }
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
        if ( (editParamInfo.State !== "INSERTING_NEW_STEP") && (editParamInfo.State !== "SORTING_STEP") )
        {
            return;
        }

        sortStep();
    }

    function onDragObjectReleased()
    {
        var editParamInfo = injectionPlanEdit.getCurEditParamInfo();
        if ( (editParamInfo.State !== "INSERTING_NEW_STEP") && (editParamInfo.State !== "SORTING_STEP") )
        {
            return;
        }

        if (dummyStepIdx != -1)
        {
            dragObject.close();
            listViewStep.interactive = true;
            planEditData.Steps[dummyStepIdx] = dragObject.userData;

            if (editParamInfo.State === "SORTING_STEP")
            {
                // Ensure that dummy step is reloaded (sometimes, model change won't recreate phase item)
                injectionPlanEdit.signalStepReload(dummyStepIdx);
            }
            dummyStepCount--;
            dummyStepIdx = -1;
            injectionPlanEdit.setCurEditParamInfo({ "State": "IDLE" });

            injectionPlanEdit.savePlan(planEditData);
        }
        else
        {
            injectionPlanEdit.setCurEditParamInfo({ "State": "IDLE" });
        }
    }

    function sortStep()
    {
        //logDebug("STEP_SWAP: Before: planEditData.Steps=" + JSON.stringify(planEditData.Steps));

        var insertRowIndex = ExamProtocolEdit.getInsertRowIndex(dummyStepIdx, dragObject, listViewStep, planEditData.Steps.length);

        if (insertRowIndex !== -1)
        {
            if (dummyStepIdx == -1)
            {
                // Ready to add dummy, check conditions
                if (insertRowIndex < executedSteps.length)
                {
                    // step is already executed
                }
                else if ( (insertRowIndex < planEditData.Steps.length) &&
                          (planEditData.Steps[insertRowIndex].IsTestInjection) )
                {
                    // Cannot insert to test injection step
                }
                else if ( (insertRowIndex > 0) &&
                          (!planEditData.Steps[insertRowIndex - 1].IsTestInjection) &&
                          (dragObject.userData.IsTestInjection) )
                {
                    // Cannot insert test injection step after normal injection step
                }
                else
                {
                    //logDebug("StepList: Inserting dummy to " + insertRowIndex);
                    planEditData.Steps.splice(insertRowIndex, 0, { GUID: "Dummy", Phases:[ { Type: "Dummy" } ] });
                    reload();
                }
            }
            else
            {
                // Ready to swap dummy to drag object, check conditions
                if (insertRowIndex >= planEditData.Steps.length)
                {
                    // Dummy is present, shouldn't add another dummy
                    //logDebug("StepList: Shoud NOT moving dummy from " + dummyStepIdx + " to " + insertRowIndex);
                    return;
                }

                if (insertRowIndex === dummyStepIdx)
                {
                    // same dummy index
                }
                else if (insertRowIndex < executedSteps.length)
                {
                    // step is already executed
                }
                else if ( (insertRowIndex < planEditData.Steps.length) &&
                          (planEditData.Steps[insertRowIndex].IsTestInjection) )
                {
                    // step is test injection type
                }
                else if ( (insertRowIndex > 0) &&
                          (dragObject.userData.IsTestInjection) )
                {
                    // move test injection step to non-first row
                }
                else
                {
                    //logDebug("StepList: Moving dummy from " + dummyStepIdx + " to " + insertRowIndex);

                    var swapIndex;

                    if (dummyStepIdx < insertRowIndex)
                    {
                        for (swapIndex = dummyStepIdx; swapIndex < insertRowIndex; swapIndex++)
                        {
                            //logDebug("StepList: Swap steps " + swapIndex + " <-> " + (swapIndex + 1));
                            planEditData.Steps = Util.swapListItem(planEditData.Steps, swapIndex, swapIndex + 1);
                        }
                    }
                    else
                    {
                        for (swapIndex = dummyStepIdx; swapIndex > insertRowIndex; swapIndex--)
                        {
                            //logDebug("StepList: Swap steps " + swapIndex + " <-> " + (swapIndex - 1));
                            planEditData.Steps = Util.swapListItem(planEditData.Steps, swapIndex, swapIndex - 1);
                        }
                    }

                    var swapIndex1 = Math.min(dummyStepIdx, insertRowIndex);
                    var swapIndex2 = Math.max(dummyStepIdx, insertRowIndex);

                    for (swapIndex = swapIndex1; swapIndex <= swapIndex2; swapIndex++)
                    {
                        injectionPlanEdit.signalStepReload(swapIndex);
                    }

                    dummyStepIdx = insertRowIndex;
                }
            }
        }
        else
        {
            // Out from list, cancel insertion
            if (dummyStepIdx != -1)
            {
                var editParamInfo = injectionPlanEdit.getCurEditParamInfo();
                if (editParamInfo.State === "INSERTING_NEW_STEP")
                {
                    //logDebug("StepList: Removing dummy step " + dummyStepIdx);
                    planEditData.Steps.splice(dummyStepIdx, 1);
                    reload();
                }
            }
        }
    }

    function updateScroll()
    {
        scrollDir = "NONE";

        var dragObjPos = listViewStep.mapFromItem(dragObject, 0, 0);

        if ( ((dragObjPos.x + dragObject.width) < listViewStep.x) ||
             (dragObjPos.x > (listViewStep.x + listViewStep.width)) )
        {
            // dragObject is out of range
            return;
        }

        if ( (listViewStep.contentY > 0) &&
             ((dragObjPos.y + listViewStep.scrollAreaHeight) < 0) )
        {
            scrollDir = "UP";
        }
        else if ( (listViewStep.visibleArea.yPosition + listViewStep.visibleArea.heightRatio < 1) &&
                  ((dragObjPos.y + dragObject.height - listViewStep.scrollAreaHeight) > listViewStep.height) )
        {
            scrollDir = "DOWN";
        }
        else
        {
            return;
        }

        if (!scrollTimer.running)
        {
            scrollTimer.start();
        }
    }
}
