import QtQuick 2.12
import QtGraphicalEffects 1.12
import "../../Util.js" as Util
import "../../Widgets"

Rectangle {
    property string statePath: dsSystem.statePath
    property string lastStatePath: dsSystem.lastStatePath
    property string examProgressState: dsExam.examProgressState
    property var reminders: dsExam.reminders
    property var curReminders: []
    property int heightExpanded: (curReminders.length * rowHeight) + radius
    property int injectionStartedElapsedSec: dsExam.injectionStartedElapsedSec
    property int injectionCompletedElapsedSec: dsExam.injectionCompletedElapsedSec

    property int imminentReminderCount: {
        var imminentReminderCountBuf = 0;
        for (var reminderIdx = 0; reminderIdx < curReminders.length; reminderIdx++)
        {
            var state = getReminderState(reminderIdx);

            if (state === "Imminent")
            {
                imminentReminderCountBuf++;
            }
        }
        return imminentReminderCountBuf;
    }

    property int heightCompressed: {
        var rowCount = imminentReminderCount;
        if ( (rowCount == 0) &&
             (curReminders.length > 0) )
        {
            rowCount = 1;
        }
        return (rowCount > 0) ? ((rowCount * rowHeight) + radius) : 0
    }
    property bool animating: false
    property int animationMs: 250
    property int yEnd: parent.height - actionBarHeight
    property int rowHeight: dsCfgLocal.screenH * 0.08
    property var reminderDelayedMs: []
    property var reminderRemainingMs: []

    id: examReminders
    visible: false
    radius: 15
    color: colorMap.remindersBackground
    y: yEnd - height + radius
    x: -radius
    clip: true
    width: parent.width * 0.265
    state: "HIDDEN"

    states: [
        State { name: "COMPRESSED" },
        State { name: "EXPANDED" },
        State { name: "HIDDEN" }
    ]

    transitions: [
        Transition {
            to: "EXPANDED"
            SequentialAnimation {
                ScriptAction { script: { animating = true; } }
                ParallelAnimation {
                    NumberAnimation { target: examReminders; properties: "height"; to: heightExpanded; duration: animationMs }
                    NumberAnimation { target: stateIconFrame; properties: 'rotation'; to: 180; duration: animationMs }
                }
                ScriptAction { script: { animating = false; } }
            }
        },

        Transition {
            to: "COMPRESSED"
            SequentialAnimation {
                ScriptAction { script: { animating = true; } }
                ParallelAnimation {
                    NumberAnimation { target: examReminders; properties: "height"; to: heightCompressed; duration: animationMs }
                    NumberAnimation { target: stateIconFrame; properties: 'rotation'; to: 0; duration: animationMs }
                }
                ScriptAction { script: { animating = false; } }
            }
        },

        Transition {
            to: "HIDDEN"

            SequentialAnimation {
                ScriptAction { script: { animating = true; } }
                ParallelAnimation {
                    NumberAnimation { target: examReminders; properties: "height"; to: 0; duration: animationMs }
                    NumberAnimation { target: stateIconFrame; properties: 'rotation'; to: 0; duration: animationMs }
                }
                ScriptAction { script: { animating = false; } }
            }
        }
    ]

    onHeightCompressedChanged: {
        if (state == "COMPRESSED")
        {
            animating = true;

            // Redraw the reminders
            timerSingleShot(1, function() {
                state = "IDLE";
                state = "COMPRESSED";
            });
        }
    }

    Item {
        id: stateIconFrame
        anchors.horizontalCenter: parent.horizontalCenter
        height: rowHeight * 0.32
        width: parent.width * 0.25
        visible: curReminders.length > 1

        Text {
            id: iconExpandedState
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            color: colorMap.text01
            font.pixelSize: height
            font.family: fontIcon.name
            text: "\ue907"
        }
    }

    ListView {
        id: listViewReminders
        clip: true
        width: parent.width
        height: parent.height - examReminders.radius
        delegate: ExamReminderItem {}
        interactive: false
    }

    layer.enabled: true
    layer.effect: Glow {
        radius: 8
        samples: 8
        spread: 0.3
        color: colorMap.buttonShadow
        transparentBorder: true
    }

    MouseArea {
        // Prevent click over reminders
        anchors.fill: parent
    }

    GenericButton {
        visible: curReminders.length > 1
        anchors.fill: parent
        onBtnClicked: {
            animating = true;
            examReminders.state = (examReminders.state === "EXPANDED") ? "COMPRESSED" : "EXPANDED";
        }
    }

    onInjectionStartedElapsedSecChanged: {
        updateRemindersRemainingMs();
    }

    onExamProgressStateChanged: {
        reload();
    }

    onRemindersChanged: {
        reload();
    }

    onStatePathChanged: {
        reload();
    }

    Component.onCompleted: {
        dsExam.qmlReminders = this;
        appMain.screenStateChanged.connect(slotScreenStateChanged);
        slotScreenStateChanged();
    }

    Component.onDestruction: {
        appMain.screenStateChanged.disconnect(slotScreenStateChanged);
    }

    function slotScreenStateChanged()
    {
        visible = (appMain.screenState.indexOf("ExamManager-") >= 0);
    }

    function updateCurReminders()
    {
        //logDebug("StatePath=" + statePath + ", lastStatePath=" + lastStatePath);

        curReminders = Util.copyObject(reminders);
        listViewReminders.model = curReminders;

        var reminderRemainingMsBuf = [];
        var reminderDelayedMsBuf = [];

        for (var reminderIdx = 0; reminderIdx < curReminders.length; reminderIdx++)
        {
            reminderRemainingMsBuf.push(Util.durationStrToMillisec(curReminders[reminderIdx].PostStepTriggerDelay));
            reminderDelayedMsBuf.push(0);
        }

        reminderRemainingMs = Util.copyObject(reminderRemainingMsBuf);
        reminderDelayedMs = Util.copyObject(reminderDelayedMsBuf);

        //logDebug("curReminders=" + JSON.stringify(curReminders));
    }

    function updateRemindersRemainingMs()
    {
        if (examReminders.state === "HIDDEN")
        {
            return;
        }

        if (statePath == "Busy/Holding")
        {
            // don't update during user-hold
            return;
        }

        var soundPlay = "None";

        var reminderRemainingMsBuf = Util.copyObject(reminderRemainingMs);

        for (var reminderIdx = 0; reminderIdx < curReminders.length; reminderIdx++)
        {
            var reminderState = getReminderState(reminderIdx);

            if ( (reminderState === "Unknown") ||
                 (reminderState === "Expired") )
            {
                continue;
            }

            var timePastMs;
            if (statePath == "Ready/Armed")
            {
                timePastMs = 0;
            }
            else if (!curReminders[reminderIdx].StartAfterStepCompletes)
            {
                timePastMs = injectionStartedElapsedSec * 1000;
            }
            else if (curReminders[reminderIdx].StartAfterStepCompletes)
            {
                timePastMs = injectionCompletedElapsedSec * 1000;
            }

            var expireAtMs = Util.durationStrToMillisec(curReminders[reminderIdx].PostStepTriggerDelay);

            if (reminderRemainingMsBuf[reminderIdx] > 0)
            {
                var newRemainingMs = Math.max(expireAtMs - timePastMs + reminderDelayedMs[reminderIdx], 0);

                if (timePastMs !== 0)
                {
                    if (newRemainingMs < 1000)
                    {
                        newRemainingMs = 0;
                        soundPlay = "Expired";
                    }
                    else if (newRemainingMs <= 10000)
                    {
                        if (soundPlay == "None")
                        {
                            soundPlay = "Imminent";
                        }
                    }
                }

                reminderRemainingMsBuf[reminderIdx] = newRemainingMs;
                //logDebug("ExamReminders: Reminder[" + reminderIdx + "].remainingMs=" + reminderRemainingMsBuf[reminderIdx] + ", timePastMs=" + timePastMs + ", expireAtMs=" + expireAtMs);
            }
        }

        if (soundPlay == "Expired")
        {
            soundPlayer.playReminderExpired();
        }
        else if (soundPlay == "Imminent")
        {
            soundPlayer.playReminderImminent();
        }

        reminderRemainingMs = Util.copyObject(reminderRemainingMsBuf);
    }

    function slotAddUserPausedData(pausedMs)
    {
        var reminderDelayedMsBuf = Util.copyObject(reminderDelayedMs);

        for (var reminderIdx = 0; reminderIdx < curReminders.length; reminderIdx++)
        {
            if ( (!curReminders[reminderIdx].StartAfterStepCompletes) &&
                 (reminderRemainingMs[reminderIdx] > 0) )
            {
                reminderDelayedMsBuf[reminderIdx] += pausedMs;
            }
        }

        reminderDelayedMs = Util.copyObject(reminderDelayedMsBuf);
    }

    function reload()
    {
        if (examProgressState == "Completed")
        {
            state = "HIDDEN";
            return;
        }

        if ( (lastStatePath == "Ready/Armed") && (statePath == "Idle") )
        {
            state = "HIDDEN";
            return;
        }

        if ( (lastStatePath == "Idle") && (statePath == "Ready/Armed") )
        {
            updateCurReminders();
            updateRemindersRemainingMs();
            state = "IDLE";
            state = "COMPRESSED";
            return;
        }

        if (curReminders.length == 0)
        {
            state = "HIDDEN";
            return;
        }
    }

    function getReminderState(reminderIdx)
    {
        if (curReminders[reminderIdx] === undefined)
        {
            return "Invalid";
        }

        if (curReminders[reminderIdx].StartAfterStepCompletes)
        {
            // Post Reminder
            if ( (statePath == "Ready/Armed") ||
                 (statePath == "Executing") ||
                 (statePath == "Busy/Holding") ||
                 (statePath == "Busy/Finishing") )
            {
                return "Inactive/Waiting";
            }
            else if (statePath == "Idle")
            {
                if (reminderRemainingMs[reminderIdx] > 10000)
                {
                    return "Active";
                }
                else if (reminderRemainingMs[reminderIdx] > 0)
                {
                    return "Imminent";
                }
                else
                {
                    return "Expired";
                }
            }
        }
        else
        {
            // Normal Reminder
            if (statePath == "Ready/Armed")
            {
                return "Active/Waiting";
            }
            else if ( (statePath == "Executing") ||
                      (statePath == "Busy/Holding") ||
                      (statePath == "Busy/Finishing") ||
                      (statePath == "Idle") )
            {
                if (reminderRemainingMs[reminderIdx] > 10000)
                {
                    return "Active";
                }
                else if (reminderRemainingMs[reminderIdx] > 0)
                {
                    return "Imminent";
                }
                else
                {
                    return "Expired";
                }
            }
        }

        return "Unknown";
    }
}
