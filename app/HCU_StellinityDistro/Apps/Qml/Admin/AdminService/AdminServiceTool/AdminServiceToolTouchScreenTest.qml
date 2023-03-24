import QtQuick 2.12
import "../../../Widgets"

Item {
    property int xPos
    property int yPos
    property int prevXPos
    property int prevYPos
    property bool drawStarted: false
    property string brushColor: colorMap.text01
    property string backgroundColor: colorMap.mainBackground
    property int brushSize: 7

    id: root
    anchors.fill: parent

    Rectangle {
        id: areaTest
        color: backgroundColor
        anchors.fill: parent

        Canvas {
            id: canvas
            anchors.fill: parent
            onPaint: {
                if (drawStarted)
                {
                    var ctx = getContext("2d");

                    ctx.save();
                    ctx.strokeStyle = brushColor;
                    ctx.lineWidth = brushSize;
                    ctx.fillStyle = brushColor;
                    ctx.lineJoin = "round";
                    ctx.lineCap = "round";

                    if ( (xPos == prevXPos) &&
                         (yPos == prevYPos) )
                    {
                        ctx.fillRect(xPos - (brushSize - 2), yPos - (brushSize - 2), brushSize, brushSize)
                    }
                    else
                    {
                        ctx.beginPath();
                        ctx.moveTo(prevXPos, prevYPos);
                        ctx.lineTo(xPos, yPos);
                        ctx.closePath();
                        ctx.fill();
                        ctx.stroke();
                        prevXPos = xPos;
                        prevYPos = yPos;
                    }

                    drawStarted = true;
                }
            }

            MouseArea {
                anchors.fill: parent

                onReleased: {
                    drawStarted = false;
                }

                onPressed: {
                    drawStarted = true;
                    prevXPos = mouseX;
                    prevYPos = mouseY;
                    canvas.draw(mouseX, mouseY);
                }

                onMouseXChanged: {
                    canvas.draw(mouseX, mouseY);
                }

                onMouseYChanged: {
                    canvas.draw(mouseX, mouseY);
                }
            }

            function draw(xPosNew, yPosNew)
            {
                xPos = xPosNew;
                yPos = yPosNew;
                canvas.requestPaint();
            }

            function clear()
            {
                var ctx = getContext("2d");
                ctx.reset();
                canvas.requestPaint();
            }
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            y: parent.height * 0.05
            height: parent.height * 0.1
            spacing: root.width * 0.04

            GenericIconButton {
                opacity: 0.3
                color: colorMap.keypadButton
                width: root.width * 0.16
                height: parent.height
                iconText: entireBlurBackground.isOpen() ? "Minimise" : "Maximise"
                iconFontPixelSize: height * 0.28
                onBtnClicked: {
                    if (entireBlurBackground.isOpen())
                    {
                        entireBlurBackground.close([areaTest]);
                    }
                    else
                    {
                        entireBlurBackground.open([areaTest], false, null);
                    }
                }
            }

            GenericIconButton {
                opacity: 0.3
                color: colorMap.keypadButton
                width: root.width * 0.16
                height: parent.height
                iconText: "Clear"
                iconFontPixelSize: height * 0.28
                onBtnClicked: {
                    canvas.clear();
                }
            }

            GenericIconButton {
                opacity: 0.3
                color: colorMap.keypadButton
                width: root.width * 0.16
                height: parent.height
                iconText: "Change\nBackground"
                border.width: 5
                border.color: backgroundColor
                iconFontPixelSize: height * 0.28
                onBtnClicked: {
                    if (backgroundColor === colorMap.mainBackground) backgroundColor = "white";
                    else if (backgroundColor === "white") backgroundColor = "black";
                    else if (backgroundColor === "black") backgroundColor = "gray";
                    else if (backgroundColor === "gray") backgroundColor = "red";
                    else if (backgroundColor === "red") backgroundColor = "orange";
                    else if (backgroundColor === "orange") backgroundColor = "yellow";
                    else if (backgroundColor === "yellow") backgroundColor = "green";
                    else if (backgroundColor === "green") backgroundColor = "blue";
                    else if (backgroundColor === "blue") backgroundColor = "purple";
                    else backgroundColor = colorMap.mainBackground;
                }
            }

            GenericIconButton {
                opacity: 0.3
                color: colorMap.keypadButton
                width: root.width * 0.16
                height: parent.height
                iconText: "Change\nBrush"
                border.width: 5
                border.color: brushColor
                iconFontPixelSize: height * 0.28
                onBtnClicked: {
                    if (brushColor === colorMap.text01) brushColor = "white";
                    else if (brushColor === "white") brushColor = "black";
                    else if (brushColor === "black") brushColor = "gray";
                    else if (brushColor === "gray") brushColor = "red";
                    else if (brushColor === "red") brushColor = "orange";
                    else if (brushColor === "orange") brushColor = "yellow";
                    else if (brushColor === "yellow") brushColor = "green";
                    else if (brushColor === "green") brushColor = "blue";
                    else if (brushColor === "blue") brushColor = "purple";
                    else brushColor = colorMap.text01;
                }
            }
        }
    }

    Component.onCompleted: {
        appMain.screenStateChanged.connect(slotScreenStateChanged);
        slotScreenStateChanged();
    }

    Component.onDestruction: {
        appMain.screenStateChanged.disconnect(slotScreenStateChanged);
    }

    function slotScreenStateChanged()
    {
        var prevVisible = visible;
        visible = (appMain.screenState === "Admin-Service-Tool-TouchScreenTest");
    }
}

