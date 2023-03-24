import QtQuick 2.12
import "../../Widgets"
import "../../Util.js" as Util

Item {
    property int familyIndex: index
    property string scrollDir: "NONE" // RIGHT, LEFT
    property int scrollSpeed: 600

    id: root

    width: ListView.view.width
    height: ListView.view.height * 0.286

    Rectangle {
        id: rectShelf
        width: parent.width
        height: parent.height * 0.05
        border.width: 1
        border.color: colorMap.text02
        color: colorMap.grid
        anchors.bottom: parent.bottom
        radius: height * 0.5
    }

    ListView {
        property double lastContentX: 0
        property int scrollAreaWidth: width * 0.1

        id: listViewPackage
        width: parent.width
        anchors.top: parent.top
        anchors.topMargin: parent.height * 0.02
        anchors.bottom: rectShelf.top
        anchors.bottomMargin: parent.height * 0.02
        clip: true
        cacheBuffer: Math.max(contentWidth, parent.width * 10)
        highlightMoveDuration: 0
        highlightRangeMode: ListView.ApplyRange

        orientation: ListView.Horizontal
        delegate: AdminContrastsListGroup {}
        model: (contrastFamilies[familyIndex] === undefined) ? undefined : contrastFamilies[familyIndex].Groups
        ListFade {}

        onDragStarted: {
            listViewFamily.interactive = false;
        }

        onDragEnded: {
            listViewFamily.interactive = true;
        }

        onModelChanged: {
            //logDebug("AdminContrastsListFamily: listViewPackage.model changed = " + JSON.stringify(model));
            contentX = lastContentX;
            if ( (selectedFamilyIdx == familyIndex) &&
                 ( (reloadReason == "INIT") || (reloadReason == "REMOVED") ) )
            {
                // Page First Load: Highlight selected group
                currentIndex = selectedGroupIdx;
            }
        }

        onContentXChanged: {
            if (contentX != -0)
            {
                lastContentX = contentX;
            }
        }
    }

    Timer {
        id: scrollTimer
        interval: 50
        onTriggered: {
            if (!dragObject.isDragging)
            {
                return;
            }
            else if ( (scrollDir === "LEFT") && (listViewPackage.contentX > 0) )
            {
                listViewPackage.flick(scrollSpeed, 0);
                scrollTimer.start();
            }
            else if ( (scrollDir === "RIGHT") && ((listViewPackage.visibleArea.xPosition + listViewPackage.visibleArea.widthRatio) < 1) )
            {
                listViewPackage.flick(-scrollSpeed, 0);
                scrollTimer.start();
            }
        }
    }

    Component.onCompleted: {
        dragObject.xChanged.connect(onDragObjectMoved);
        dragObject.yChanged.connect(onDragObjectMoved);

        if (selectedFamilyIdx == familyIndex)
        {
            listViewPackage.currentIndex = selectedGroupIdx;
        }
    }

    Component.onDestruction: {
        dragObject.xChanged.disconnect(onDragObjectMoved);
        dragObject.yChanged.disconnect(onDragObjectMoved);
    }

    function updateScroll()
    {
        // scroll groups
        scrollDir = "NONE";

        var dragObjPos = listViewPackage.mapFromItem(dragObject, 0, 0);

        if ( ((dragObjPos.y + dragObject.height) < listViewPackage.y) ||
             (dragObjPos.y > (listViewPackage.y + listViewPackage.height)) )
        {
            // dragObject is out of range
            return;
        }

        if ( (listViewPackage.contentX > 0) &&
             ((dragObjPos.x + listViewPackage.scrollAreaWidth) < 0) )
        {
            scrollDir = "LEFT";
        }
        else if ( (listViewPackage.visibleArea.xPosition + listViewPackage.visibleArea.widthRatio < 1) &&
                  ((dragObjPos.x + dragObject.width - listViewPackage.scrollAreaWidth) > listViewPackage.width) )
        {
            scrollDir = "RIGHT";
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

    function onDragObjectMoved()
    {
        if (!dragObject.isOpen())
        {
            // Drag Object is closed by other
            return;
        }

        updateScroll();
    }
}
