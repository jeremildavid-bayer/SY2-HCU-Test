import QtQuick 2.12
import "../Widgets"
import "../Util.js" as Util

Rectangle {
    property var testStatus: dsTest.testStatus
    property bool continuousExamsTestEnabled: dsCapabilities.continuousExamsTestEnabled
    property int continuousExamsTestLimit: dsCapabilities.continuousExamsTestLimit
    property double heightCompressed: titleBarHeight * 0.3
    property double heightExpanded: titleBarHeight
    property double backgroundOpacity: 0.9
    property int animationMs: 250
    property string infoFieldFontFamily: fontRobotoLight.name
    property string infoFieldFontColor: colorMap.blk01
    property double infoFieldWidth: width * 0.12
    property double infoFieldFontSize: titleBarHeight * 0.18

    property bool testActive: (testStatus.Type === "ContinuousExams") && (!testStatus.IsFinished)
    property string testState: ( (testStatus.Type === "ContinuousExams") && (testStatus.StateStr !== undefined) ) ? testStatus.StateStr : "--"
    property string errStr: ( (testStatus.Type === "ContinuousExams") && (testStatus.StatusMap.Error !== undefined) ) ? (testStatus.StatusMap.Error === "" ? "--" : testStatus.StatusMap.Error) : "--"
    property string elapsedTimeStr: ""
    property string executedExams: ( (testStatus.Type === "ContinuousExams") && (testStatus.StatusMap.ExecutedExams !== undefined) ) ? testStatus.StatusMap.ExecutedExams.toString() : "--"
    property string linkDropped: ( (testStatus.Type === "ContinuousExams") && (testStatus.StatusMap.LinkDropped !== undefined) ) ? testStatus.StatusMap.LinkDropped.toString() : "--"
    property string alertsOccurred: ( (testStatus.Type === "ContinuousExams") && (testStatus.StatusMap.AlertsOccurred !== undefined) ) ? testStatus.StatusMap.AlertsOccurred.toString() : "--"


    id: root
    width: parent.width
    height: heightCompressed
    color: colorMap.yellow02
    clip: true
    opacity: backgroundOpacity

    state: "COMPRESSED"
    visible: continuousExamsTestEnabled

    states: [
        State { name: "EXPANDED" },
        State { name: "COMPRESSED" }
    ]

    transitions: [
        Transition {
            from: "COMPRESSED"
            to: "EXPANDED"
            SequentialAnimation {
                ParallelAnimation {
                    NumberAnimation { target: root; properties: "height"; to: heightExpanded; duration: animationMs; easing.type: Easing.InOutQuart }
                    NumberAnimation { target: root; properties: "opacity"; to: backgroundOpacity; duration: animationMs }
                }
            }
        },
        Transition {
            from: "EXPANDED"
            to: "COMPRESSED"
            SequentialAnimation {
                ParallelAnimation {
                    NumberAnimation { target: root; properties: "height"; to: heightCompressed; duration: animationMs; easing.type: Easing.InOutQuart }
                    NumberAnimation { target: root; properties: "opacity"; to: backgroundOpacity; duration: animationMs }                }
            }
        }
    ]

    GenericButton {
        id: btnExpand
        width: parent.width
        height: heightCompressed
        color: colorMap.yellow01
        radius: 0
        onBtnClicked: {
            if (root.state == "COMPRESSED")
            {
                root.state = "EXPANDED";
            }
            else
            {
                root.state = "COMPRESSED";
            }
        }

        Text {
            color: colorMap.blk01
            anchors.fill: parent
            text: "CONTINUOUS EXAMS TEST" + (testActive ? " ACTIVE" : " INACTIVE")
            font.pixelSize: height * 0.95
            font.family: fontRobotoBold.name
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        Text {
            color: colorMap.blk01
            anchors.fill: parent
            anchors.rightMargin: parent.width * 0.02
            text: ((root.state == "COMPRESSED") ? "\ue906" : "\ue907")
            font.pixelSize: height * 0.95
            font.family: fontIcon.name
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignVCenter
        }
    }

    Item {
        visible: (root.state == "EXPANDED")
        y: heightCompressed
        width: parent.width
        height: root.height - y

        GenericIconButton {
            id: btnStart
            x: parent.width * 0.01
            width: parent.width * 0.08
            y: parent.height * 0.05
            height: parent.height * 0.9
            visible: !testActive
            color: colorMap.actionButtonBackground
            iconFontFamily: fontIcon.name
            iconColor: colorMap.actionButtonText
            iconFontPixelSize: height * 0.4
            iconText: "\ue94e"
            onBtnClicked: {
                dsTest.slotTestStart("ContinuousExams", []);
            }
        }

        GenericIconButton {
            id: btnStop
            x: btnStart.x
            width: btnStart.width
            y: btnStart.y
            height: btnStart.height
            visible: testActive
            color: colorMap.red
            iconColor: colorMap.blk01
            iconFontFamily: fontIcon.name
            iconFontPixelSize: height * 0.4
            iconText: "\ue951"
            onBtnClicked: {
                dsTest.slotTestStop();
            }
        }

        Text {
            id: txtState
            x: btnStart.x + btnStart.width + (parent.width * 0.02)
            width: infoFieldWidth * 1.8
            height: parent.height
            color: infoFieldFontColor
            font.family: infoFieldFontFamily
            text: "State:<p><b>" + testState + "</b>"
            font.pixelSize: infoFieldFontSize
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.Wrap
        }

        Text {
            id: txtError
            x: txtState.x + txtState.width + (parent.width * 0.02)
            width: infoFieldWidth * 1.8
            height: parent.height
            color: infoFieldFontColor
            font.family: infoFieldFontFamily
            text: {
                var textHeader = "Error:<p><b>";
                var textTail = "</b>";
                var textBody = (errStr == "--") ? errStr : ("<font color=\"red\">" + errStr + "</font>");
                return textHeader + textBody + textTail;
            }
            font.pixelSize: infoFieldFontSize
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.Wrap
        }

        Text {
            id: txtElapsedTime
            x: txtError.x + txtError.width
            width: infoFieldWidth
            height: parent.height
            color: infoFieldFontColor
            font.family: infoFieldFontFamily
            text: "Elapsed Time:<p><b>" + elapsedTimeStr + "</b>"
            font.pixelSize: infoFieldFontSize
            verticalAlignment: Text.AlignVCenter
        }

        Text {
            id: txtExecutedExams
            x: txtElapsedTime.x + txtElapsedTime.width
            width: infoFieldWidth
            height: parent.height
            color: infoFieldFontColor
            font.family: infoFieldFontFamily
            text: "Executed Exams:<p><b>" + executedExams + " / " + (continuousExamsTestLimit > 0 ? continuousExamsTestLimit.toString() : "Unlimited") + "</b>"
            font.pixelSize: infoFieldFontSize
            verticalAlignment: Text.AlignVCenter
        }

        Text {
            id: txtAlertCount
            x: txtExecutedExams.x + txtExecutedExams.width
            width: infoFieldWidth
            height: parent.height
            color: infoFieldFontColor
            font.family: infoFieldFontFamily
            text: "Alerts Occurred:<p><b>" + alertsOccurred + "</b>"
            font.pixelSize: infoFieldFontSize
            verticalAlignment: Text.AlignVCenter
        }

        Text {
            id: txtLinkDropped
            x: txtAlertCount.x + txtAlertCount.width
            width: infoFieldWidth
            height: parent.height
            color: infoFieldFontColor
            font.family: infoFieldFontFamily
            text: "Link Dropped:<p><b>" + linkDropped + "</b>"
            font.pixelSize: infoFieldFontSize
            verticalAlignment: Text.AlignVCenter
        }
    }

    Component.onCompleted: {
        appMain.signalUpdateDateTime.connect(updateDuration);
    }

    Component.onDestruction: {
        appMain.signalUpdateDateTime.disconnect(updateDuration);
    }

    function updateDuration()
    {
        if ( (testStatus.Type !== "ContinuousExams") ||
             (testStatus.StatusMap.StartedAt === undefined) )
        {
            elapsedTimeStr = "--";
            return;
        }

        var currentDate = new Date();
        var startedEpochMs = Util.utcDateTimeToMillisec(testStatus.StatusMap.StartedAt);
        var timePastMs = currentDate - startedEpochMs;
        elapsedTimeStr = Util.durationStrToHumanReadableFormatStr(Util.millisecToDurationStr(timePastMs));

        if (timePastMs < 60000)
        {
            // Elapsed time is short, update timer for every 1 sec
            timerSingleShot(1000, function() {
                updateDuration();
            });
        }
    }
}


