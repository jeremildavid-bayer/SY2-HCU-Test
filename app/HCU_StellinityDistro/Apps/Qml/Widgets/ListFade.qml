import QtQuick 2.12
import QtGraphicalEffects 1.12

Item {
    property var flickable: parent
    property int headerSize: 0
    property bool vertical: flickable.flickableDirection === Flickable.VerticalFlick
    property int fadeSize: vertical ? (dsCfgLocal.screenH * 0.03) : (dsCfgLocal.screenW * 0.02)
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
                else if (flickable.contentY + headerSize <= 0.1)
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
                else if (flickable.contentX + headerSize <= 0.1)
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
                else if ((flickable.visibleArea.yPosition + flickable.visibleArea.heightRatio) >= 0.9999)
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
                else if ((flickable.visibleArea.xPosition + flickable.visibleArea.widthRatio) >= 0.9999)
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
