import QtQuick 2.12
import "../../Widgets"
import "../../Util.js" as Util
import "AdminContrasts.js" as ContrastsUtil

Item {
    property int dummyFamilyIndex: -1
    property int dummyGroupIndex: -1
    property int dragStartFamilyIndex: -1
    property int dragStartGroupIndex: -1
    property var dragDroppedPos
    property string scrollDir: "NONE" // UP, DOWN
    property int scrollSpeed: 600

    signal signalReload(int familyIdx, int groupIdx)

    id: contrastsList

    ListView {
        property double lastContentY: 0
        property int scrollAreaHeight: height * 0.1

        id: listViewFamily
        anchors.top: parent.top
        anchors.topMargin: parent.height * 0.04
        anchors.bottom: parent.bottom
        width: parent.width
        clip: true
        cacheBuffer: Math.max(contentHeight * 2, height * 10)
        delegate: AdminContrastsListFamily {}

        footer: Item {
            width: ListView.view ? ListView.view.width : 0
            height: widgetKeyboard.isOpen() ? (widgetKeyboard.keyboardHeight - actionBarHeight) : 0
        }

        ScrollBar {}
        ListFade {}

        onContentYChanged: {
            if (contentY != -0)
            {
                lastContentY = contentY;
            }
        }

        onModelChanged: {
            contentY = lastContentY;
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
            else if ( (scrollDir === "UP") && (listViewFamily.contentY > 0) )
            {
                listViewFamily.flick(0, scrollSpeed);
                scrollTimer.start();
            }
            else if ( (scrollDir === "DOWN") && ((listViewFamily.visibleArea.yPosition + listViewFamily.visibleArea.heightRatio) < 1) )
            {
                listViewFamily.flick(0, -scrollSpeed);
                scrollTimer.start();
            }
        }
    }

    DragRectangle {
        id: dragObject
        visible: false
    }

    Component.onCompleted: {
        dragObject.xChanged.connect(onDragObjectMoved);
        dragObject.yChanged.connect(onDragObjectMoved);
    }

    Component.onDestruction: {
        dragObject.xChanged.disconnect(onDragObjectMoved);
        dragObject.yChanged.disconnect(onDragObjectMoved);
    }

    function reload()
    {
        if (!visible)
        {
            return;
        }

        dummyFamilyIndex = -1;
        dummyGroupIndex = -1;

        if (JSON.stringify(listViewFamily.model) != JSON.stringify(contrastFamilies))
        {
            //logDebug("\n\ncontrastFamilies = " + JSON.stringify(contrastsPage.contrastFamilies, null, " ") + "\n\n");
            listViewFamily.model = contrastFamilies;

            if (listViewFamily.model.length > 0)
            {
                if ( (selectedFamilyIdx == -1) ||
                     (selectedGroupIdx == -1) )
                {
                    contrastsPage.selectedFamilyIdx = 0;
                    contrastsPage.selectedGroupIdx = 0;
                }
            }
            else
            {
                contrastsPage.selectedFamilyIdx = -1;
                contrastsPage.selectedGroupIdx = -1;
            }
        }
        //emitSignalStartEdit();

        if ( (reloadReason == "INIT") ||
             (reloadReason == "REMOVED") )
        {
            // Highlight selected family
            listViewFamily.currentIndex = selectedFamilyIdx;
        }
    }

    function updateScroll()
    {
        // scroll groups
        scrollDir = "NONE";

        var dragObjPos = listViewFamily.mapFromItem(dragObject, 0, 0);

        if (((dragObjPos.x + dragObject.width) < listViewFamily.x) ||
             (dragObjPos.x > (listViewFamily.x + listViewFamily.width)) )
        {
            // dragObject is out of range
            return;
        }

        if ( (listViewFamily.contentY > 0) &&
             ((dragObjPos.y + listViewFamily.scrollAreaHeight) < 0) )
        {
            scrollDir = "UP";
        }
        else if ( (listViewFamily.visibleArea.yPosition + listViewFamily.visibleArea.heightRatio < 1) &&
                  ((dragObjPos.y + dragObject.height - listViewFamily.scrollAreaHeight) > listViewFamily.height) )
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
