import QtQuick 2.12
import QtGraphicalEffects 1.12

Rectangle {
    property var userData: null
    property var dragStartPos
    property bool isDragging: false

    signal signalReleased()

    opacity: 0.6
    color: "transparent"

    Image {
        id: image
        anchors.fill: parent

        layer.enabled: true
        layer.effect: Glow {
            radius: 17
            samples: 17
            spread: 0.5
            color: colorMap.buttonShadow
            transparentBorder: true
        }
    }

    function open(srcItem, imageSourceUrl)
    {
        width = srcItem.width;
        height = srcItem.height;

        x = parent.mapFromItem(srcItem, 0, 0).x;
        y = parent.mapFromItem(srcItem, 0, 0).y;

        setImage(imageSourceUrl);
        visible = true;

        isDragging = true;
        dragStartPos = { x: x, y: y };
    }

    function setImage(imageSourceUrl)
    {
        image.source = imageSourceUrl;
    }

    function isOpen()
    {
        return visible;
    }

    function close()
    {
        visible = false;
    }

    function released()
    {
        isDragging = false;
        signalReleased();
    }
}

