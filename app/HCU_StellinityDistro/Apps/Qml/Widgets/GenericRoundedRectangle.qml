import QtQuick 2.12
import QtQuick.Controls 2.12

Item {
    property int radius1: 0
    property int radius2: 0
    property int radius3: 0
    property int radius4: 0
    property string color: colorMap.text01
    property string borderColor: color
    property int borderWidth: 0
    property alias content: mainRect.children

    id: root

    Canvas {
        anchors.fill: parent

        onPaint: {
            var ctx = getContext("2d");
            ctx.save();
            ctx.clearRect(0, 0, width, height);
            ctx.strokeStyle = root.backgroundColor;
            ctx.lineWidth = root.borderWidth;
            ctx.fillStyle = root.color;
            ctx.lineJoin = "round";
            ctx.lineCap = "round";
            ctx.beginPath();

            // Draw top
            if (radius1 == 0)
            {
                ctx.moveTo(0, 0);
                ctx.lineTo(width, 0);
            }
            else
            {
                ctx.moveTo(0, radius1);
                ctx.arcTo(0, 0, radius1, 0, radius1);
            }

            if (radius2 == 0)
            {
                ctx.lineTo(width, 0);
            }
            else
            {
                ctx.lineTo(width - radius2, 0);
                ctx.arcTo(width, 0, width, radius2, radius2);
            }

            if (radius3 == 0)
            {
                ctx.lineTo(width, height);
            }
            else
            {
                ctx.lineTo(width, height - radius3);
                ctx.arcTo(width, height, width - radius3, height, radius3);
            }

            if (radius4 == 0)
            {
                ctx.lineTo(0, height);
            }
            else
            {
                ctx.lineTo(radius4, height);
                ctx.arcTo(0, height, 0, height - radius4, radius4);
            }

            ctx.closePath();
            ctx.fill();
            ctx.stroke();
        }
    }

    Item {
        id: mainRect
        anchors.fill: parent
    }
}
