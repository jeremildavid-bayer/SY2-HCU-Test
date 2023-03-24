import QtQuick 2.12
import "../../Widgets"
import "../../Util.js" as Util

Item {
    property int reminderIndex: index
    property var reminder: curReminders[reminderIndex]
    property string state: getReminderState(reminderIndex)
    property string fontColor: getFontColor()
    property int remainingMs: (examReminders.reminderRemainingMs[reminderIndex] === undefined) ? 0 : examReminders.reminderRemainingMs[reminderIndex]
    property string reminderTextStr: getRemainingTime()

    id: root
    height: getHeight()
    width: ListView.view.width


    Item {
        id: rectMain
        anchors.fill: parent
        anchors.leftMargin: parent.width * 0.06
        anchors.rightMargin: parent.width * 0.06
        anchors.topMargin: parent.height * 0.01
        anchors.bottomMargin: parent.height * 0.01
        clip: true

        Text {
            id: icon
            anchors.left: parent.left
            width: contentWidth
            height: parent.height
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: height * 0.37
            font.family: fontIcon.name
            text: (reminder === undefined) ? "" : reminder.StartAfterStepCompletes ? "\ue915" : "\ue917"
            color: fontColor
        }

        Text {
            id: textName
            anchors.left: icon.right
            anchors.leftMargin: parent.width * 0.03
            anchors.right: textRemainTime.left
            anchors.rightMargin: parent.width * 0.02
            height: parent.height
            font.family: fontRobotoLight.name
            font.pixelSize: height * 0.32
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.Wrap
            elide: Text.ElideRight
            clip: true
            text: (reminder === undefined) ? "" : reminder.Name
            color: fontColor
        }

        Text {
            id: textRemainTime
            anchors.right: parent.right
            width: contentWidth
            height: parent.height
            font.family: fontRobotoBold.name
            font.pixelSize: height * 0.45
            verticalAlignment: Text.AlignVCenter
            color: fontColor
            text: getRemainingTime()
        }
    }

    Rectangle {
        id: spacing
        width: parent.width
        height: 2
        y: parent.height - height
        color: colorMap.mainBackground
        visible: reminderIndex < curReminders.length - 1
    }

    function getRemainingTime()
    {
        if (remainingMs <= 0)
        {
            return "00:00";
        }
        else
        {
            var durationStr = Util.millisecToDurationStr(remainingMs);
            durationStr = Util.getMinimisedDurationStr(durationStr);
            return durationStr;
        }
    }

    function getFontColor()
    {
        //logDebug("ExamReminderItem: Reminder[" + index + "].state=" + state);

        if ( (state == "Active") ||
             (state == "Active/Waiting") )
        {
            return colorMap.text01;
        }
        else if (state == "Imminent")
        {
            return colorMap.orange;
        }
        else
        {
            return colorMap.text02;
        }
    }

    SequentialAnimation {
        id: animationImminent
        NumberAnimation { target: root; properties: 'opacity'; from: 0; to: 1; duration: animationMs; }
    }

    onStateChanged: {
        if (state == "Imminent")
        {
            animationImminent.start();
        }
    }

    function getHeight()
    {
        if (examReminders.state === "EXPANDED")
        {
            return rowHeight;
        }
        else if (examReminders.state === "COMPRESSED")
        {
            if (animating)
            {
                return height;
            }
            else if (state == "Imminent")
            {
                return rowHeight;
            }
            else if (imminentReminderCount == 0)
            {
                // No imminent reminder
                if (statePath == "Ready/Armed")
                {
                    // Armed: Display first reminder
                    if (reminderIndex == 0)
                    {
                        return rowHeight;
                    }
                    else
                    {
                        return 0;
                    }
                }
                else
                {
                    // Executing, Executed: Display closest active reminder
                    var lowestRemainingMs = 1000000;
                    for (var reminderIdx = 0; reminderIdx < curReminders.length; reminderIdx++)
                    {
                        if ( (getReminderState(reminderIdx) === "Active") &&
                             (reminderRemainingMs[reminderIdx] > 0) )
                        {
                            lowestRemainingMs = Math.min(lowestRemainingMs, reminderRemainingMs[reminderIdx]);
                        }
                    }

                    if (lowestRemainingMs < 1000000)
                    {
                        for (reminderIdx = 0; reminderIdx < reminderRemainingMs.length; reminderIdx++)
                        {
                            if (reminderRemainingMs[reminderIdx] === lowestRemainingMs)
                            {
                                if (reminderIdx == reminderIndex)
                                {
                                    return rowHeight;
                                }
                            }
                        }
                        return 0;
                    }
                }
            }
            else
            {
                return 0;
            }
        }
        return height;
    }
}
