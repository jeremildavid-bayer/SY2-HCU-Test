import QtQuick 2.9
import QtGraphicalEffects 1.0

Item {
    property var flickable: parent
    property bool vertical: flickable.flickableDirection === Flickable.VerticalFlick
    property int fadeSize: vertical ? flickable.height * 0.05 : flickable.width * 0.05
    property string fadeColor: colorMap.mainBackground
    property bool forceShowTop: false
    property bool forceShowBottom: false

    anchors.fill: parent
    z: flickable.z + 1

    Rectangle {
        color: "transparent"
        width: vertical ? flickable.width : fadeSize
        height: vertical ? fadeSize : flickable.height
        opacity: forceShowTop || getOpacity()

        LinearGradient {
            id: topFade
            anchors.fill: parent
            start: Qt.point(0, 0)
            end: vertical ? Qt.point(0, height) : Qt.point(width, 0)

            gradient: Gradient {
                GradientStop { position: 0.0; color: fadeColor }
                GradientStop { position: 1.0; color: "#00000000" }
            }
        }

        function getOpacity()
        {
            if (vertical)
            {
                if (flickable.visibleArea.heightRatio >= 1)
                {
                    return 0;
                }
                else if (flickable.contentY <= 0)
                {
                    return 0;
                }
                else
                {
                    return 1;
                }
            }
            else
            {
                if (flickable.visibleArea.widthRatio >= 1)
                {
                    return 0;
                }
                else if (flickable.contentX <= 0)
                {
                    return 0;
                }
                else
                {
                    return 1;
                }
            }
        }
    }

    Rectangle {
        color: "transparent"
        width: vertical ? flickable.width : fadeSize
        height: vertical ? fadeSize : flickable.height
        opacity: forceShowBottom || getOpacity()
        y: vertical ? (flickable.height - fadeSize) : 0
        x: vertical ? 0 : (flickable.width - fadeSize)

        LinearGradient {
            id: bottomFade
            anchors.fill: parent
            start: Qt.point(0, 0)
            end: vertical ? Qt.point(0, height) : Qt.point(width, 0)

            gradient: Gradient {
                GradientStop { position: 0.0; color: "#00000000" }
                GradientStop { position: 1.0; color: fadeColor   }
            }
        }

        function getOpacity()
        {
            if (vertical)
            {
                if (flickable.visibleArea.heightRatio >= 1)
                {
                    // content height is less than visible area: don't need scroll
                    return 0;
                }
                else if ((flickable.visibleArea.yPosition + flickable.visibleArea.heightRatio) >= 0.99)
                {
                    return 0;
                }
                else
                {
                    return 1;
                }
            }
            else
            {
                if (flickable.visibleArea.widthRatio >= 1)
                {
                    // content height is less than visible area: don't need scroll
                    return 0;
                }
                else if ((flickable.visibleArea.xPosition + flickable.visibleArea.widthRatio) >= 0.99)
                {
                    return 0;
                }
                else
                {
                    return 1;
                }
            }
        }
    }
}
