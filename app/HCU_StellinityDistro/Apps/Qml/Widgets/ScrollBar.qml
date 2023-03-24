import QtQuick 2.12
import "../Util.js" as Util

Rectangle {
    // The flickable to which the scrollbar is attached to, must be set
    property var flickable: parent

    // True for vertical ScrollBar, false for horizontal
    property bool vertical: flickable.flickableDirection === Flickable.VerticalFlick

    // If set to false, scrollbar is visible even when not scrolling
    property bool autoHide: true

    // Thickness of the scrollbar, in pixels
    property int scrollbarWidth: Util.getPixelH(14)
    property int animationMs: 500

    property double maxOpacity: 0.6
    property int positionOffset: 2

    color: colorMap.scrollBar
    radius: vertical ? width / 2 : height / 2

    // Scrollbar appears automatically when content is bigger than the Flickable
    opacity: setOpacity()

    // Calculate width/height and position based on the content size and position of
    // the Flickable
    width: getWidth()
    height: getHeight()
    x: getX()
    y: getY()
    z: flickable.z + 1

    function setOpacity()
    {
        if (autoHide)
        {
            if (flickable.flicking || flickable.moving)
            {
                if (vertical)
                {
                    if (height >= flickable.height)
                    {
                        return 0;
                    }
                    else
                    {
                        return maxOpacity;
                    }
                }
                else
                {
                    if (width >= flickable.width)
                    {
                        return 0;
                    }
                    else
                    {
                        return maxOpacity;
                    }
                }
            }
            else
            {
                return 0;
            }
        }
        else
        {
            if (vertical)
            {
                if (flickable.visibleArea.heightRatio === 1)
                {
                    return 0;
                }
            }
            else
            {
                if (flickable.visibleArea.widthRatio === 1)
                {
                    return 0
                }
            }

            return maxOpacity;
        }

    }

    function getWidth()
    {
        var newWidth;
        if (vertical)
        {
            newWidth = scrollbarWidth;
        }
        else
        {
            newWidth = flickable.visibleArea.widthRatio * flickable.width;
        }
        return newWidth;
    }

    function getHeight()
    {
        var newHeight;
        if (vertical)
        {
            newHeight = flickable.visibleArea.heightRatio * flickable.height;
        }
        else
        {
            newHeight = scrollbarWidth;
        }
        return newHeight;
    }

    function getX()
    {
        var newX;
        if (vertical)
        {
            newX = flickable.width - width - positionOffset;
        }
        else
        {
            newX = flickable.visibleArea.xPosition * (flickable.width);
        }
        return newX;
    }

    function getY()
    {
        var newY;
        if (vertical)
        {
            newY = flickable.visibleArea.yPosition * (flickable.height);
        }
        else
        {
            newY = flickable.height - height - positionOffset;
        }
        return newY;
    }

    // Animate scrollbar appearing/disappearing
    Behavior on opacity {
        NumberAnimation { duration: animationMs }
    }
}
