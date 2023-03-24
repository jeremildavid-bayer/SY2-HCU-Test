function getInsertRowIndex(dummyIdx, dragObj, listView, rowCount)
{
    var dragObjPos = listView.mapFromItem(dragObj, 0, 0);

    var posTopLeft = { x: dragObjPos.x, y: dragObjPos.y + listView.contentY };
    var posTopRight = { x: dragObjPos.x + dragObj.width, y: dragObjPos.y + listView.contentY };

    var posCenterLeft = { x: dragObjPos.x, y: dragObjPos.y + (dragObj.height / 2) + listView.contentY };
    var posCenterRight = { x: dragObjPos.x + dragObj.width, y: dragObjPos.y + (dragObj.height / 2) + listView.contentY };

    var posBottomLeft = { x: dragObjPos.x, y: dragObjPos.y + dragObj.height + listView.contentY };
    var posBottomRight = { x: dragObjPos.x + dragObj.width, y: dragObjPos.y + dragObj.height + listView.contentY };

    var insertTopIdx = Math.max(listView.indexAt(posTopLeft.x, posTopLeft.y), listView.indexAt(posTopRight.x, posTopRight.y));
    var insertCenterIdx = Math.max(listView.indexAt(posCenterLeft.x, posCenterLeft.y), listView.indexAt(posCenterRight.x, posCenterRight.y));
    var insertBottomIdx = Math.max(listView.indexAt(posBottomLeft.x, posBottomLeft.y), listView.indexAt(posBottomRight.x, posBottomRight.y));

    if ( (insertTopIdx == -1) &&
         (insertCenterIdx == -1) &&
         (insertBottomIdx == -1) )
    {
        // dragObj is completely out from listView
        return -1;
    }

    //logDebug("Util.GetInsertRowIdex: indexes are " + insertTopIdx + "," + insertCenterIdx + "," + insertBottomIdx);

    if (insertBottomIdx == -1)
    {
        // insert to last row
        insertBottomIdx = rowCount;
    }

    if (insertCenterIdx == -1)
    {
        if (insertTopIdx == insertBottomIdx)
        {
            insertCenterIdx = insertTopIdx;
        }
        else if (insertTopIdx == -1)
        {
            insertCenterIdx = insertTopIdx;
        }
        else if (insertBottomIdx === rowCount)
        {
            insertCenterIdx = insertBottomIdx;
        }
        else
        {
            // This shouldn't happen!
            //logError("ERROR: ExamProtocolEdit.getInsertRowIndex: insertCenterIdx=-1, T:"+ insertTopIdx + ",B:" + insertBottomIdx + ",dummyIdx=" + dummyIdx);
            insertCenterIdx = insertTopIdx;
        }
    }

    //logDebug("Util.GetInsertRowIdex: indexes are dummyIdx=" + dummyIdx + ": " + insertTopIdx + "," + insertCenterIdx + "," + insertBottomIdx);

    if (dummyIdx === -1)
    {
        // Dummy is not present
        if (insertTopIdx == insertCenterIdx)
        {
            return insertTopIdx;
        }
        else
        {
            return insertTopIdx + 1;
        }
    }
    else
    {
        // Dummy is present
        if (insertTopIdx < (dummyIdx - 1))
        {
            // dragObj is in next upper row
            return insertTopIdx + 1;
        }
        else if (insertBottomIdx > (dummyIdx + 1))
        {
            // dragObj is in next bottom row
            return insertBottomIdx - 1;
        }
        else
        {
            return dummyIdx;
        }
    }
}
