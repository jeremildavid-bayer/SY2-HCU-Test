import QtQuick 2.3

Rectangle {
    property string taskState: "idle"

    property string curSortingStepTitle: ""
    property int curStepIndex: -1
    property int curStepCount: -1
    property int curPhaseCount: -1
    property int curSortingRowIndex: -1
    property var stepInfoTable;  // format:  [ { "stepId": <string>, "phaseCount": <int> }, ... ]

    property int curDummyIndex: -1
    property int scrollDir: 0


    MouseArea {
        id: mainMouseArea
        property var mouseStartOffset;
        anchors.fill: parent

        onPositionChanged: {
            if ( (taskState === "sortPhase") ||
                 (taskState === "sortStep") )
            {
                dragObject.y = mouse.y - mouseStartOffset.y;
            }
        }

        onPressed: {
            var dragObjectPos;

            if ( (taskState === "sortPhase") ||
                 (taskState === "sortStep") )
            {
                dragObjectPos = stepListView.mapFromItem(dragObject, stepListView.x, stepListView.y);
                mouseStartOffset = { x: mouse.x - dragObjectPos.x, y: mouse.y - dragObjectPos.y };
            }
        }

        onReleased: {
            if ( (taskState === "sortPhase") ||
                 (taskState === "sortStep") )
            {
                console.log("StepList Released");
                onSortRowDone();
            }
        }
    }

    Timer {
        property int counter: 0;
        id: scrollTimer
        interval: 30
        onTriggered: {
            if ( (scrollDir === 1) && (stepListView.contentY > 0) )
            {
                stepListView.flick(0, scrollSpeed);
                scrollTimer.start();
            }
            else if ( (scrollDir === -1) && ((stepListView.visibleArea.yPosition + stepListView.visibleArea.heightRatio) < 1) )
            {
                stepListView.flick(0, -scrollSpeed);
                scrollTimer.start();
            }

            counter++;

            if (counter >= 6)
            {
                taskStateProcess();
                counter = 0;
            }
        }
    }

    Rectangle {
        id: scrollUpArea
        x: 0
        y: 0
        width: parent.width
        height: scrollAreaHeight
        z: stepListView.z + 1
        color: "#70ff0000"
    }

    Rectangle {
        id: scrollDownArea
        x: 0
        y: parent.height - scrollAreaHeight
        width: parent.width
        height: scrollAreaHeight
        z: stepListView.z + 1
        color: "#70ff0000"
    }


    ListView {
        cacheBuffer: 100000
        id: stepListView
        anchors.fill: parent

        model: ListModel {
            id: listModel
        }

        delegate: PhaseItem {}
        section.property: "stepId"
        section.criteria: ViewSection.FullString
        section.delegate: StepItem {}

        footer: Rectangle {
            id: stepListViewFooter
            color: "yellow"
            width: parent.width
            height: stepListViewFooterHeight
        }
    }

    ListView {
        z: stepListView.z - 1
        cacheBuffer: 100000

        id: stepListViewPhantom

        width: stepListView.width
        height: stepListView.height

        model: ListModel {
            id: listModelPhantom
        }

        delegate: PhaseItem {
            isPhantom: true
        }
        section.property: "stepId"
        section.criteria: ViewSection.FullString
        section.delegate: StepItem {
            isPhantom: true
        }

        function setVisibleStep(stepId)
        {
            console.log("creating model for stepId=" + stepId + ",listModel.length=" + listModel.count);
            listModelPhantom.clear();

            for (var i = 0; i < listModel.count; i++)
            {
                var item = listModel.get(i);
                if (item.stepId === stepId)
                {
                    listModelPhantom.append(item);
                }
            }

            console.log("stepListViewPhantom: updated");
            stepListViewPhantom.height = stepHeaderItemH + (listModelPhantom.count * phaseItemH);
        }
    }


    function setListModel(listPhaseValues)
    {
        stepListView.visible = false;

        //console.log("onModelChanged: prop_listPhaseValues=" + JSON.stringify(listPhaseValues));

        var listModelBuf = listModel;
        var stepInfoTableBuf = [];
        listModelBuf.clear();

        var stepId = "";
        var phaseIndex = -1;

        for (var i = 0; i < listPhaseValues.length; i++)
        {
            var curSortingStepTitle = listPhaseValues[i].stepId;
            if (curSortingStepTitle !== stepId)
            {
                if (phaseIndex >= 0)
                {
                    stepInfoTableBuf.push({ "stepId": stepId, "phaseCount": phaseIndex + 1 });
                }
                stepId = curSortingStepTitle;
                phaseIndex = 0;
            }
            else
            {
                phaseIndex++;
            }

            listModelBuf.append( {  "isDummy": listPhaseValues[i].isDummy,
                                    "stepId": stepId,
                                    "titleColor": listPhaseValues[i].titleColor,
                                    "phaseIndex": phaseIndex    });
        }

        stepInfoTableBuf.push({"stepId": listPhaseValues[listPhaseValues.length - 1].stepId, "phaseCount": phaseIndex + 1});

        if (stepInfoTable !== stepInfoTableBuf)
        {
            stepInfoTable = stepInfoTableBuf;
        }

        if (listModel !== listModelBuf)
        {
            listModel = listModelBuf;
        }

        if (taskState === "insertingDummyStep")
        {
            onSortStepStarted("DUMMY");
            taskState = "sortingDummyStep";
        }

        stepListView.visible = true;


        // Print out model
        var stringBuf = "";
        for (var i = 0; i < listModel.count; i++)
        {
           stringBuf += listModel.get(i)["stepId"] + "(" + listModel.get(i)["titleColor"] + ") ";
        }
        console.log("StepId[" + listModel.count + "]=" + stringBuf);

    }

    function onInsertStepStarted()
    {
        taskState = "insertStep";
        curDummyIndex = -1;
    }

    function onInsertStepDone()
    {
        signalInsertStepFromDummy(optionList.curOptionData.titleColor);
        onSortRowDone();
    }

    function onInsertPhaseStarted()
    {
        taskState = "insertPhase";
        curDummyIndex = -1;
    }

    function onInsertPhaseDone()
    {
        console.log("InsertPhaseDone");
        signalInsertPhaseFromDummy(optionList.curOptionData.titleColor);
        taskState = "idle";
    }

    function onSortStepStarted(section)
    {
        stepListViewPhantom.setVisibleStep(section);
        var stepPos = getStepHeaderRect(section);

        if (taskState !== "insertingDummyStep")
        {
            dragObject.init(stepListViewPhantom, stepPos.y - stepListView.contentY);
        }

        stepListView.interactive = false;

        curSortingStepTitle = section;
        curStepIndex = -1;
        curStepCount = stepInfoTable.length;

        for (var i = 0; i < stepInfoTable.length; i++)
        {
            if (stepInfoTable[i].stepId === section)
            {
                curStepIndex = i;
                break;
            }
        }

        taskState = "sortStep";
    }

    function onSortPhaseStarted(stepId, rowIndex)
    {
        stepListView.interactive = false;
        curSortingRowIndex = rowIndex;
        curPhaseCount = 0;

        for (var i = 0; i < stepInfoTable.length; i++)
        {
            if (stepInfoTable[i].stepId === stepId)
            {
                curPhaseCount = stepInfoTable[i].phaseCount;
                break;
            }
        }

        taskState = "sortPhase";
    }

    function onSortRowDone()
    {
        dragObject.close();
        taskState = "idle";
        curSortingRowIndex = -1;
        curSortingStepTitle = "";
        curStepIndex = -1;
        stepListView.interactive = true;
    }

    function updateScroll()
    {
        // Check if scroll is required
        var dragObjectPos = stepListView.mapFromItem(dragObject, stepListView.x, stepListView.y);

        scrollDir = 0;

        if (stepListView.contentY > 0)
        {
            if ( (isInsideRect({x: dragObjectPos.x + dragObject.width, y: dragObject.y}, scrollUpArea)) ||
                 (isInsideRect({x: dragObjectPos.x + dragObject.width, y: dragObject.y + (dragObject.height / 2)}, scrollUpArea)) ||
                 (isInsideRect({x: dragObjectPos.x + dragObject.width, y: dragObject.y + dragObject.height}, scrollUpArea)) )
            {
                console.log("scroll up");
                scrollDir = 1;
            }
        }

        if (stepListView.visibleArea.yPosition + stepListView.visibleArea.heightRatio < 1)
        {
             if ( (isInsideRect({x: dragObjectPos.x + dragObject.width, y: dragObject.y}, scrollDownArea)) ||
                  (isInsideRect({x: dragObjectPos.x + dragObject.width, y: dragObject.y + (dragObject.height / 2)}, scrollDownArea)) ||
                  (isInsideRect({x: dragObjectPos.x + dragObject.width, y: dragObject.y + dragObject.height}, scrollDownArea)) )
            {
                console.log("scroll down");
                scrollDir = -1;
            }
        }

        if (scrollDir !== 0)
        {
            if (!scrollTimer.running)
            {
                scrollTimer.start();
            }
        }
    }

    function taskStateProcess()
    {
        if (taskState === "sortPhase")
        {
            forceDragPhasePos();
            sortPhases();
        }
        else if (taskState === "sortStep")
        {
            //forceDragStepPos();
            sortSteps();
        }
        else if (taskState == "sortingDummyStep")
        {
            //forceDragStepPos();
            sortDummySteps();
        }
        else if (taskState === "insertPhase")
        {
            forceDragPhaseInsertPos();
            insertPhase();
        }
        else if (taskState === "insertStep")
        {
            //forceDragStepInsertPos();
            insertStep();
        }
    }

    function onDragObjectMoved()
    {
        updateScroll();

        if (scrollDir !== 0)
        {
            // scrolling
            //return;
        }
        taskStateProcess();
    }

    function insertStep()
    {
        var dragObjectPos = stepListView.mapFromItem(dragObject, stepListView.x, stepListView.y);

        if ((dragObjectPos.x + dragObject.width) < 0)
        {
            // out from list
            //console.log("out from list");
            mainScreen.signalRemoveDummyPhase();
            curDummyIndex = -1;
        }
        else
        {
            var insertRowIdx = -1;
            var dragObjectPosHotspot = { x: dragObjectPos.x + dragObject.width, y: dragObjectPos.y + (dragObject.height / 2) + stepListView.contentY }

            //console.log("stepListView.contentY=" + stepListView.contentY + ", dragObjectPosHotspot=" + dragObjectPosHotspot.y);

            for (var stepIdx = 0; stepIdx < stepInfoTable.length; stepIdx++)
            {
                var stepRect = getStepRectFromStepIdx(stepIdx);
                if (isInsideRect(dragObjectPosHotspot, stepRect))
                {
                    insertRowIdx = getFirstRowIdxFromStepIdx(stepIdx);
                    //console.log("Inside step area step:" + stepIdx + ", firstRow:" + insertRowIdx);
                    break;
                }
            }

            if (insertRowIdx == -1)
            {
                var footerRect = getFooterRect();
                if (isInsideRect(dragObjectPosHotspot, footerRect))
                {
                    insertRowIdx = stepListView.count;
                }
            }

            if (insertRowIdx == -1)
            {
                //mainScreen.signalRemoveDummyPhase();
                //curDummyIndex = -1;
            }
            else if (curDummyIndex !== insertRowIdx)
            {
                taskState = "insertingDummyStep";
                console.log("Inserting dummy step to " + insertRowIdx);
                mainScreen.signalSetDummyPhase(insertRowIdx, "DUMMY");
                curDummyIndex = insertRowIdx;
            }
        }

    }

    function insertPhase()
    {
        var dragObjectPos = stepListView.mapFromItem(dragObject, stepListView.x, stepListView.y);

        if ((dragObjectPos.x + dragObject.width) < phaseLeftMargin)
        {
            // out from list, cancel insertion
            mainScreen.signalRemoveDummyPhase();
            curDummyIndex = -1;
            curSortingRowIndex = -1;
        }
        else if (curDummyIndex === -1)
        {
            // insert dummy
            var dragObjectPosHotspot = { x: dragObjectPos.x + dragObject.width, y: dragObjectPos.y + (dragObject.height / 2) + stepListView.contentY }
            var insertRowIndex = stepListView.indexAt(dragObjectPosHotspot.x, dragObjectPosHotspot.y);

            if (insertRowIndex === -1)
            {
                // see if insertion is done at beginning
                var insertStepIdx = -1;

                for (var stepIdx = 0; stepIdx < stepInfoTable.length; stepIdx++)
                {
                    var lastRowRect = getLastHiddenRowRectFromStepIdx(stepIdx);
                    if (isInsideRect(dragObjectPosHotspot, lastRowRect))
                    {
                        insertStepIdx = stepIdx;
                        break;
                    }
                }

                if (insertStepIdx === -1)
                {
                    mainScreen.signalRemoveDummyPhase();
                    curDummyIndex = -1;
                }
                else
                {
                    // Insert dummy
                    insertRowIndex = getLastRowIdxFromStepIdx(insertStepIdx) + 1;
                    var curStepTitle = getStepTitleFromRowIdx(insertRowIndex - 1);
                    mainScreen.signalSetDummyPhase(insertRowIndex, curStepTitle);
                    curDummyIndex = insertRowIndex;
                    console.log("Adding dummy to " + insertRowIndex);
                    curSortingRowIndex = insertRowIndex;
                }
            }
            else if (curDummyIndex !== insertRowIndex)
            {
                console.log("Moving Dummy Phase to " + insertRowIndex);
                curStepTitle = getStepTitleFromRowIdx(insertRowIndex);
                mainScreen.signalSetDummyPhase(insertRowIndex, curStepTitle);
                curDummyIndex = insertRowIndex;
                curSortingRowIndex = insertRowIndex;
            }
        }
        else
        {
            sortPhases();
        }
    }

    function sortDummySteps()
    {
        var dragObjectPos = stepListView.mapFromItem(dragObject, stepListView.x, stepListView.y);
        if ((dragObjectPos.x + dragObject.width) < 0)
        {
            mainScreen.signalRemoveDummyPhase();
            curDummyIndex = -1;
            taskState = "insertStep";
            return
        }
        sortSteps();
    }

    function sortSteps()
    {
        var dragObjectPos = stepListView.mapFromItem(dragObject, stepListView.x, stepListView.y);
        var origPos = getStepHeaderRectFromStepIdx(curStepIndex);
        origPos.y -= stepListView.contentY;

        var nextPos;

        //console.log("dragObjectPos.y=" + dragObjectPos.y + ", origPos.y=" + origPos.y);

        if (dragObjectPos.y < origPos.y)
        {
            //console.log("moving up");

            // moving up
            var prevStepIndex = curStepIndex - 1;
            if (prevStepIndex < 0)
            {
                prevStepIndex = 0;
            }

            nextPos = getStepHeaderRectFromStepIdx(prevStepIndex);
            nextPos.y -= stepListView.contentY;

            console.log("moving up, nextPos=" + nextPos.y);
            if (dragObjectPos.y <= nextPos.y)
            {
                console.log("Swaping step " + curStepIndex + "<->" + (curStepIndex - 1));
                mainScreen.signalSwapSteps(curStepIndex, prevStepIndex);
                curStepIndex = prevStepIndex;
                curSortingStepTitle = stepInfoTable[curStepIndex].stepId;

            }
        }
        else if (dragObjectPos.y > origPos.y)
        {
            //console.log("moving down");

            // moving down
            var nextStepIndex = curStepIndex + 1;
            if (nextStepIndex >= stepInfoTable.length)
            {
                nextStepIndex = stepInfoTable.length - 1;
            }

            nextPos = getStepHeaderRectFromStepIdx(nextStepIndex);
            nextPos.y -= stepListView.contentY;

            var swapPosY = nextPos.y + stepHeaderItemH + (stepInfoTable[nextStepIndex].phaseCount * phaseItemH);
            var dragObjectBottomY = dragObjectPos.y + stepHeaderItemH + (stepInfoTable[curStepIndex].phaseCount * phaseItemH);

            //console.log("dragObjectBottomY=" + dragObjectBottomY + ", swapPosY=" + swapPosY);
            if (dragObjectBottomY >= swapPosY)
            {
                console.log("Swaping step " + curStepIndex + "<->" + (nextStepIndex));
                mainScreen.signalSwapSteps(curStepIndex, nextStepIndex);
                curStepIndex = nextStepIndex;
                curSortingStepTitle = stepInfoTable[curStepIndex].stepId;
            }
        }
    }

    function sortPhases()
    {
        var dragObjectPos = stepListView.mapFromItem(dragObject, stepListView.x, stepListView.y);
        var curHotspotPos = { x: dragObjectPos.x + (dragObject.width / 2), y: dragObjectPos.y + (dragObject.height / 2) + stepListView.contentY };
        var newRowIndex = stepListView.indexAt(curHotspotPos.x, curHotspotPos.y);

        if (newRowIndex === -1)
        {
            movePhaseToOtherStep();
        }
        else if (newRowIndex !== curSortingRowIndex)
        {
            swapPhaseRows(newRowIndex);
        }
    }

    function swapPhaseRows(newRowIndex)
    {
        console.log("sortPhases: Swapping row " + curSortingRowIndex + " -> " + newRowIndex);

        var rowDiff = curSortingRowIndex - newRowIndex;
        if (Math.abs(rowDiff) == 1)
        {
            //console.log("sortPhases: Swapping row " + curSortingRowIndex + " -> " + newRowIndex);
            mainScreen.signalSwapRows(curSortingRowIndex, newRowIndex);
            curSortingRowIndex = newRowIndex;
        }
        else
        {
            // perform sequential swap

            if (rowDiff > 0)
            {
                var swapIdx = curSortingRowIndex;
                while (swapIdx > newRowIndex)
                {
                    //console.log("sortPhases: Simulate Swapping row " + swapIdx + " -> " + (swapIdx - 1));
                    mainScreen.signalSwapRows(swapIdx, swapIdx - 1);
                    swapIdx--;
                }
                curSortingRowIndex = newRowIndex;
            }
            else
            {
                var swapIdx = curSortingRowIndex;
                while (swapIdx < newRowIndex)
                {
                    //console.log("sortPhases: Simulate Swapping row " + swapIdx + " -> " + (swapIdx + 1));
                    mainScreen.signalSwapRows(swapIdx, swapIdx + 1);
                    swapIdx++;
                }
                curSortingRowIndex = newRowIndex;
            }
        }
    }
    function movePhaseToOtherStep()
    {
        var dragObjectPos = stepListView.mapFromItem(dragObject, stepListView.x, stepListView.y);
        var curHotspotPos = { x: dragObjectPos.x + (dragObject.width / 2), y: dragObjectPos.y + (dragObject.height / 2) + stepListView.contentY };
        var touchStepIdx = getStepIdxFromStepHeaderPos(curHotspotPos.x, curHotspotPos.y);

        if (touchStepIdx !== -1)
        {
            var newStepIdx = getStepIndexFromRowIdx(curSortingRowIndex);
            var newStepId;
            if (newStepIdx === touchStepIdx)
            {
                if (newStepIdx > 0)
                {
                    // Check if current row is first phase of current step
                    var firstPhaseRowRect = getFirstRowRectFromStepIdx(touchStepIdx);
                    if (isInsideRect(curHotspotPos, firstPhaseRowRect))
                    {
                        // phase moving upward out from current step
                        newStepId = stepInfoTable[newStepIdx - 1].stepId;
                        console.log("sortPhases: setting stepId(" + newStepId + ") for row(" + curSortingRowIndex + ")");
                        mainScreen.signalSetStepIdForPhase(curSortingRowIndex, newStepId);
                    }
                }
            }
            else
            {
                // Check if current row is last phase of current step
                var lastPhaseRowRect = getLastRowRectFromStepIdx(touchStepIdx);
                if (isInsideRect(curHotspotPos, lastPhaseRowRect))
                {
                    // phase moving downward out from current step
                    newStepId = stepInfoTable[touchStepIdx].stepId;
                    console.log("sortPhases: setting stepId(" + newStepId + ") for row(" + curSortingRowIndex + ")");
                    mainScreen.signalSetStepIdForPhase(curSortingRowIndex, newStepId);
                }
            }
        }
        else
        {
            //console.warn("Failed to find step header from curren position");
        }
    }

    function forceDragStepPos()
    {
        if ( (taskState === "sortStep") ||
             (taskState === "sortingDummyStep") )
        {
            var dragObjectPos = stepListView.mapFromItem(dragObject, stepListView.x, stepListView.y);
            var toBeMoved = false;

            var boundaryY = getStepHeaderRectFromStepIdx(curStepIndex).y - stepListView.contentY;

            // Check for upper/lower position
            if ( (curStepIndex === 0) && (dragObjectPos.y < boundaryY) )
            {
                toBeMoved = true;
            }

            if ( (curStepIndex === (curStepCount - 1)) && (dragObjectPos.y > boundaryY) )
            {
                toBeMoved = true;
            }

            if ( (dragObjectPos.x + dragObject.width) > stepItemW)
            {
                dragObject.x = mainScreen.mapFromItem(stepListView, mainScreen.x, mainScreen.y).x + stepItemW - dragObject.width;
            }

            if (toBeMoved)
            {
                dragObject.y = mainScreen.mapFromItem(stepListView, mainScreen.x, mainScreen.y).y + boundaryY;
            }
        }
    }

    function forceDragPhasePos()
    {
        if (taskState === "sortPhase")
        {
            var dragObjectPos = stepListView.mapFromItem(dragObject, stepListView.x, stepListView.y);
            var toBeMoved = false;

            var boundaryY = getItemGeometry(curSortingRowIndex).y - stepListView.contentY;

            // Check for upper/lower position
            if ( (curSortingRowIndex == 0) && (dragObjectPos.y < boundaryY) )
            {
                // force position to 1st row
                toBeMoved = true;
            }

            if ( (curSortingRowIndex === (stepListView.count - 1)) && (dragObjectPos.y > boundaryY) )
            {
                // force position to last row
                toBeMoved = true;
            }

            if (toBeMoved)
            {
                dragObject.y = mainScreen.mapFromItem(stepListView, mainScreen.x, mainScreen.y).y + boundaryY;
            }
        }
    }


    function forceDragStepInsertPos()
    {
        if (taskState == "insertStep")
        {
            var dragObjectPos = stepListView.mapFromItem(dragObject, stepListView.x, stepListView.y);

            if ((dragObjectPos.x + dragObject.width) > stepListView.width)
            {
                dragObject.x = mainScreen.mapFromItem(stepListView, mainScreen.x, mainScreen.y).x + stepListView.width - dragObject.width;
            }

            var boundaryY0 = getItemGeometry(0).y - stepHeaderItemH - stepListView.contentY;
            var boundaryY1 = getItemGeometry(stepListView.count - 1).y + phaseItemH + stepListViewFooterHeight - stepListView.contentY;

            if (dragObjectPos.y < boundaryY0)
            {
                dragObject.y = mainScreen.mapFromItem(stepListView, mainScreen.x, mainScreen.y).y + boundaryY0;
            }
            else if ((dragObjectPos.y + dragObject.height) > boundaryY1)
            {
                dragObject.y = mainScreen.mapFromItem(stepListView, mainScreen.x, mainScreen.y).y + boundaryY1 - dragObject.height;
            }
        }
    }

    function forceDragPhaseInsertPos()
    {
        if (taskState == "insertPhase")
        {
            var dragObjectPos = stepListView.mapFromItem(dragObject, stepListView.x, stepListView.y);

            if ((dragObjectPos.x + dragObject.width) > stepListView.width)
            {
                dragObject.x = mainScreen.mapFromItem(stepListView, mainScreen.x, mainScreen.y).x + stepListView.width - dragObject.width;
            }

            var boundaryY0 = getItemGeometry(0).y - stepListView.contentY;
            var boundaryY1 = getItemGeometry(stepListView.count - 1).y + phaseItemH + stepListViewFooterHeight - stepListView.contentY;

            if (dragObjectPos.y < boundaryY0)
            {
                dragObject.y = mainScreen.mapFromItem(stepListView, mainScreen.x, mainScreen.y).y + boundaryY0;
            }
            else if ((dragObjectPos.y + dragObject.height) > boundaryY1)
            {
                dragObject.y = mainScreen.mapFromItem(stepListView, mainScreen.x, mainScreen.y).y + boundaryY1 - dragObject.height;
            }
        }
    }

    // =============================================
    // Utilities

    function isInsideRect(pos, rect)
    {
        return ( (pos.x >= rect.x) &&
                 (pos.x <= (rect.x + rect.width)) &&
                 (pos.y >= rect.y) &&
                 (pos.y <= (rect.y + rect.height)) );
    }

    function getFooterRect()
    {
        var footerRect = getItemGeometry(stepListView.count - 1);
        footerRect.y += phaseItemH;
        footerRect.x = 0;
        footerRect.width = stepItemW;
        footerRect.height = stepListViewFooterHeight;
        return footerRect;
    }

    function getItemGeometry(index)
    {
        var prevContentY = stepListView.contentY;
        var oldIdx = stepListView.currentIndex;

        stepListView.currentIndex = index;

        var rect = { x: stepListView.currentItem.x, y: stepListView.currentItem.y, width: stepListView.currentItem.width,  height: stepListView.currentItem.height };

        if (stepListView.currentIndex !== oldIdx)
        {
            stepListView.currentIndex = oldIdx;
        }

        if (stepListView.contentY != prevContentY)
        {
            stepListView.contentY = prevContentY;
        }

        return rect;
    }

    function getStepHeaderRect(stepId)
    {
        for (var rowIdx = 0; rowIdx < listModel.count; rowIdx++)
        {
            if (listModel.get(rowIdx).stepId === stepId)
            {
                var firstRowPos = getItemGeometry(rowIdx);
                return { x: firstRowPos.x, y: firstRowPos.y - stepHeaderItemH, width: stepItemW, height: stepHeaderItemH };
            }
        }
    }

    function getFirstRowIdxFromStepIdx(stepIdx)
    {
        var rowIdx = 0;

        for (var i = 0; i < stepInfoTable.length; i++)
        {
            if (stepIdx === i)
            {
                return rowIdx;
            }
            rowIdx += stepInfoTable[i].phaseCount;
        }
        return -1;
    }

    function getLastRowIdxFromStepIdx(stepIdx)
    {
        var rowIdx = 0;

        for (var i = 0; i < stepInfoTable.length; i++)
        {
            rowIdx += stepInfoTable[i].phaseCount - 1;
            if (stepIdx === i)
            {
                return rowIdx;
            }
            rowIdx++;
        }
        return -1;
    }

    function getFirstRowRectFromStepIdx(stepIdx)
    {
        var rowIdx = getFirstRowIdxFromStepIdx(stepIdx);
        if (rowIdx !== -1)
        {
            var rowRect = getItemGeometry(rowIdx);
            return rowRect;
        }
        return null;
    }

    function getLastRowRectFromStepIdx(stepIdx)
    {
        var rowIdx = getLastRowIdxFromStepIdx(stepIdx);
        if (rowIdx !== -1)
        {
            var rowRect = getItemGeometry(rowIdx);
            return rowRect;
        }
        return null;
    }


    // NOTE: stepList contains 'hidden' row at last for insertion purpose.
    function getLastHiddenRowRectFromStepIdx(stepIdx)
    {
        var rowIdx = getLastRowIdxFromStepIdx(stepIdx);
        if (rowIdx !== -1)
        {
            var rowRect = getItemGeometry(rowIdx);
            rowRect.y += phaseItemH;
            return rowRect;
        }
        return null;
    }

    function getStepHeaderRectFromStepIdx(stepIdx)
    {
        if ( (stepIdx >= 0) && (stepIdx < stepInfoTable.length) )
        {
            return getStepHeaderRect(stepInfoTable[stepIdx].stepId);
        }
        console.log("getStepHeaderRectFromStepIdx: Bad stepIdx=" + stepIdx);
        return null;
    }

    function getStepRectFromStepIdx(stepIdx)
    {
        var stepRect = getStepHeaderRect(stepInfoTable[stepIdx].stepId);
        stepRect.height += stepInfoTable[stepIdx].phaseCount * phaseItemH;
        return stepRect;
    }

    function getStepTitleFromRowIdx(rowIdx)
    {
        var rowIdxBuf = rowIdx;

        for (var i = 0; i < stepInfoTable.length; i++)
        {
            rowIdxBuf -= stepInfoTable[i].phaseCount;
            if (rowIdxBuf < 0)
            {
                // curStep found
                return stepInfoTable[i].stepId;
            }
        }

        return "";
    }

    function getStepIndexFromRowIdx(rowIdx)
    {
        var rowIdxBuf = rowIdx;

        for (var stepIdx = 0; stepIdx < stepInfoTable.length; stepIdx++)
        {
            rowIdxBuf -= stepInfoTable[stepIdx].phaseCount;
            if (rowIdxBuf < 0)
            {
                return stepIdx;
            }
        }

        return -1;
    }

    function getRowIndexFromPos(x, y)
    {
        for (var rowIdx = 0; rowIdx < stepListView.count; rowIdx++)
        {
            var itemRect = getItemGeometry(rowIdx);

            // check if {x,y} are inside the rowItem
            if ( (x >= itemRect.x) &&
                 (x <= (itemRect.x + itemRect.width)) &&
                 (y >= itemRect.y) &&
                 (y <= (itemRect.y + itemRect.height)) )
            {
                return rowIdx;
            }
        }

        return -1;
    }

    function getStepIdxFromStepHeaderRect(rect)
    {
        var rectPos = { x: rect.x + (rect.width / 2), y: rect.y + (rect.height / 2) }

        for (var stepIdx = 0; stepIdx < stepInfoTable.length; stepIdx++)
        {
            var rectBuf = getStepHeaderRectFromStepIdx(stepIdx);
            if (isInsideRect(rectPos, rectBuf))
            {
                return stepIdx;
            }
        }

        return -1;
    }

    function getStepIdxFromStepHeaderPos(x, y)
    {
        for (var stepIdx = 0; stepIdx < stepInfoTable.length; stepIdx++)
        {
            var rect = getStepHeaderRectFromStepIdx(stepIdx);
            var pos = { x: x, y: y };
            if (isInsideRect(pos, rect))
            {
                return stepIdx;
            }
        }

        return -1;
    }
}
