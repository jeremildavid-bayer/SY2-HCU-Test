import QtQuick 2.3

Image {
    //Drag.active: false
    //Drag.source: null
    //Drag.active: true
    //Drag.source: mainMouseArea

    Drag.hotSpot.x: 0
    Drag.hotSpot.y: height / 2

    function init(srcItem, yOffset)
    {
        //console.log("DragObject: init");
        width = srcItem.width;
        height = srcItem.height;

        y = parent.mapFromItem(srcItem, parent.x, parent.y).y + yOffset;
        x = parent.mapFromItem(srcItem, parent.x, parent.y).x;
        opacity = 0.5;
        visible = true;

        srcItem.grabToImage(function(result) {
            //console.log("DragObject: image ready");
            source = result.url;
        }, Qt.size(srcItem.width, srcItem.height));
    }

    function close()
    {
        visible = false;
    }

    onXChanged: {
        stepList.onDragObjectMoved();
    }

    onYChanged: {
        stepList.onDragObjectMoved();
    }
}

