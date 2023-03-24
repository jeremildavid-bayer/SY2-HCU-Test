import QtQuick 2.12

Canvas {
    property double percentStart: 0
    property double percentEnd: 0
    property double percent: 0
    property int animationMs: 500
    property string lineColor: colorMap.text01
    property int lineWidth: 8
    property double angleStart: 0 * Math.PI
    property double angleTotal: 2 * Math.PI

    id: root

    PropertyAnimation {
        id: animation
        target: root
        properties: 'percent';
        from: percentStart
        to: percentEnd
        duration: animationMs
    }

    onPercentChanged: {
        requestPaint();
    }

    onPaint: {
        var ctx = getContext("2d");
        var arcRadius = (width / 2) - lineWidth;
        ctx.clearRect(0, 0, width, height);
        ctx.strokeStyle = lineColor;
        ctx.lineWidth = lineWidth;
        ctx.lineCap = "round";
        ctx.beginPath();
        ctx.arc(width / 2, height / 2, arcRadius, angleStart, angleStart + ((angleTotal * percent) / 100), false);
        ctx.stroke();
    }

    Component.onCompleted: {
        appMain.colorMapChanged.connect(requestPaint);
    }

    Component.onDestruction: {
        appMain.colorMapChanged.disconnect(requestPaint);
    }

    function setProgress(newPercent)
    {
        if (percentEnd !== newPercent)
        {
            animation.stop();
            percentStart = percentEnd;
            percentEnd = newPercent;
            animation.start();
        }
    }
}
