function cloneMap(from)
{
    var ret = {};
    for (var i in from)
    {
        ret[i] = from[i];
    }

    return ret;
}

function swapListItem(list, idx1, idx2)
{
    var itemBuf = list[idx1];
    list[idx1] = list[idx2];
    list[idx2] = itemBuf;
    return list;
}

function getInsertRowIndex(dummyIdx, dragObj, listView)
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

    //console.log("Util.GetInsertRowIdex: indexes are " + insertTopIdx + "," + insertCenterIdx + "," + insertBottomIdx);

    if (insertBottomIdx == -1)
    {
        // insert to last row
        insertBottomIdx = listView.count;
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
        else if (insertBottomIdx === listView.count)
        {
            insertCenterIdx = insertBottomIdx;
        }
        else
        {
            // This shouldn't happen!
            console.error("ERROR: Util.getInsertRowIndex: insertCenterIdx=-1, T:"+ insertTopIdx + ",B:" + insertBottomIdx + ",dummyIdx=" + dummyIdx);
            insertCenterIdx = insertTopIdx;
        }
    }

    //console.log("Util.GetInsertRowIdex: indexes are dummyIdx=" + dummyIdx + ": " + insertTopIdx + "," + insertCenterIdx + "," + insertBottomIdx);

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

function getMinimisedDurationStr(srcStr)
{
    // Convert from "hh:mm:ss.zzz" to "mm:ss"
    var strList = srcStr.split(":");
    if (strList.length >= 3)
    {
        if (strList[0] !== "00")
        {
            return "99:59";
        }
        else
        {
            var secStr = strList[2].split(".")[0];
            var durationStr = strList[1] + ":" + secStr;
            return durationStr;
        }
    }
    return srcStr;
}

function millisecToDurationStr(msec)
{
    var sec = Math.floor(msec / 1000);
    msec = msec % 1000;
    var min =  Math.floor(sec / 60);
    sec = sec % 60;
    var hour =  Math.floor(min / 60);
    min = min % 60;
    hour = hour % 24;

    msec = msec.toString();
    sec = sec.toString();
    min = min.toString();
    hour = hour.toString();

    while (msec.length < 3)
    {
        msec = "0" + msec;
    }

    while (sec.length < 2)
    {
        sec = "0" + sec;
    }

    while (min.length < 2)
    {
        min = "0" + min;
    }

    while (hour.length < 2)
    {
        hour = "0" + hour;
    }

    return hour + ":" + min + ":" + sec + "." + msec;
}

function durationStrToMillisec(durationStr)
{
    durationStr = getMinimisedDurationStr(durationStr);

    var strList = durationStr.split(":");
    if (strList.length >= 2)
    {
        var min = parseInt(strList[0]);
        var sec = parseInt(strList[1]);
        return (min * 60 * 1000) + (sec * 1000);
    }
    return 0;
}

function getPressure(pressureUnit, pressureKpa)
{
    if (pressureUnit === "psi")
    {
        return pressureKpa * 0.14503773;
    }
    else if (pressureUnit === "kg/cm2")
    {
        return pressureKpa * 0.0101972;
    }
    return pressureKpa;
}

function getPressureKpa(pressureUnit, pressure)
{
    if (pressureUnit === "psi")
    {
        return pressure / 0.14503773;
    }
    else if (pressureUnit === "kg/cm2")
    {
        return pressure / 0.0101972;
    }
    return pressure;
}

function utcDateTimeToMillisec(utcDateTimeStr)
{
    //"yyyy-MM-ddTHH:mm:ss.zzzZ"
    return Date.parse(utcDateTimeStr);
}
