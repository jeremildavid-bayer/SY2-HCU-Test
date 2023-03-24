import QtQuick 2.12
import QtGraphicalEffects 1.12
import "../Util.js" as Util

Rectangle {
    property string statePath: dsSystem.statePath
    property string lastStatePath: dsSystem.lastStatePath
    property var executedSteps: dsExam.executedSteps
    property int animationMs: 250
    property int containerHeight: 0
    property bool timerVisible: false
    property int injectionStartedElapsedSec: dsExam.injectionStartedElapsedSec

    id: root
    width: parent.width
    height: (timerVisible) ? containerHeight : 0;
    color: colorMap.elapsedTimeBackground
    opacity: 0.7
    radius: 15

    Behavior on height {
        NumberAnimation { duration: animationMs }
    }

    Text {
        id: txtTime
        anchors.fill: parent
        color: colorMap.text01
        font.bold: true
        font.pixelSize: height * 0.57
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        text: {
            var elapsedTimeStr = "00:00";
            var timeStr = Util.millisecToDurationStr(injectionStartedElapsedSec * 1000);

            var strList = timeStr.split(":");
            if (strList.length >= 3)
            {
                var hour = strList[0];
                var min = strList[1];
                var sec = strList[2];

                var hourInt = parseInt(hour);
                var hourList = hour.split(".");
                if (hourList.length > 1)
                {
                    var day = hourList[0];
                    hourInt = parseInt(hourList[1]);
                    hourInt += parseInt(day) * 24;
                }

                if (hourInt > 99)
                {
                    hour = "99:";
                }
                else if (hourInt === 0)
                {
                    hour = "";
                }
                else
                {
                    hour = hourInt.toString() + ":";
                }

                var secList = sec.split(".");
                if (secList.length > 1)
                {
                    sec = sec.split(".")[0];
                }

                elapsedTimeStr = hour + min + ":" + sec;
            }
            return elapsedTimeStr;
        }
    }

    layer.enabled: true
    layer.effect: Glow {
        radius: 8
        samples: 8
        spread: 0.3
        color: colorMap.buttonShadow
        transparentBorder: true
    }

    onStatePathChanged: {
        reload();
    }

    onExecutedStepsChanged: {
        reload();
    }

    function reload()
    {
        if (statePath == "Ready/Armed")
        {
            timerVisible = true;
        }
        else if (executedSteps.length == 0)
        {
            timerVisible = false;
        }
        else if ( (lastStatePath == "Ready/Armed") && (statePath == "Idle") )
        {
            timerVisible = false;
        }
    }
}
