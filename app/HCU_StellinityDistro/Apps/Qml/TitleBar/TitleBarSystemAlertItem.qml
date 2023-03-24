import QtQuick 2.12
import "../Util.js" as Util
import "../Widgets"

Rectangle {
    property int currentUtcOffsetMinutes: dsCfgLocal.currentUtcOffsetMinutes
    property int alertIdx: activeSystemAlerts.length - index - 1
    property string dateFormat: dsCfgGlobal.dateFormat
    property string timeFormat: dsCfgGlobal.timeFormat
    property int systemAlertItemHeight: frameSystemAlertsPanel.height * 0.17
    property int systemAlertItemHeightMin: systemAlertItemHeight

    width: ListView.view.width
    height: content.height + (systemAlertItemHeight * 0.1)
    color: colorMap.systemAlertsPanelBackground

    Item {
        id: content
        width: parent.width * 0.95
        height: name.height + userDirection.height + txtDateTime.height + (systemAlertItemHeight * 0.15)
        anchors.centerIn: parent

        WarningIcon {
            id: fatalAlertIcon
            visible: (activeSystemAlerts[alertIdx]["Severity"] === "Fatal")
            anchors.top: parent.top
            anchors.left: parent.left
            // adding 0.3 padding at end so name doesn't start right next to it
            width: visible ? (contentWidth + (contentWidth * 0.3)) : 0
            // 1.05 added to make sure it looks OK with multi-line name as well
            height: font.pixelSize * 1.05

            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignBottom
            font.pixelSize: name.font.pixelSize
        }

        Text {
            id: name
            anchors.top: parent.top
            anchors.left: fatalAlertIcon.right
            width: parent.width - fatalAlertIcon.width
            height: Math.max(systemAlertItemHeight * 0.25, contentHeight)
            color: colorMap.text01
            verticalAlignment: Text.AlignVCenter
            font.family: fontRobotoBold.name
            font.pixelSize: systemAlertItemHeight * 0.18
            text: getNameText()
            wrapMode: Text.Wrap
        }

        Text {
            id: userDirection
            anchors.top: name.bottom
            anchors.topMargin: systemAlertItemHeight * 0.05
            width: parent.width
            height: contentHeight
            color: colorMap.text01
            wrapMode: Text.Wrap
            font.family: fontRobotoLight.name
            font.pixelSize: systemAlertItemHeight * 0.15
            text: getUserDirectionText()

        }

        Text {
            id: txtDateTime
            anchors.top: userDirection.bottom
            anchors.topMargin: systemAlertItemHeight * 0.1
            width: parent.width
            height: systemAlertItemHeight * 0.25
            color: colorMap.text02
            font.family: fontRobotoLight.name
            font.pixelSize: height * 0.6
            horizontalAlignment: Text.AlignRight
            text: {
                var activatedAtMs = Util.utcDateTimeToMillisec(activeSystemAlerts[alertIdx].ActiveAt);
                var dateFormatLocale = translate("T_ConfigItem_Settings_General_DateFormat_" + dateFormat);
                var timeFormatLocale = translate("T_ConfigItem_Settings_General_TimeFormat_" + timeFormat);
                return Util.getDateTimeStrFromEpochMs(dateFormatLocale, timeFormatLocale, currentUtcOffsetMinutes, activatedAtMs);
            }
        }
    }

    SequentialAnimation {
        id: animationNewAlert
        NumberAnimation { target: content; properties: 'opacity'; from: 1; to: 0; duration: 350 }
        NumberAnimation { target: content; properties: 'opacity'; from: 0; to: 1; duration: 350 }
        NumberAnimation { target: content; properties: 'opacity'; from: 1; to: 0; duration: 350 }
        NumberAnimation { target: content; properties: 'opacity'; from: 0; to: 1; duration: 350 }
        NumberAnimation { target: content; properties: 'opacity'; from: 1; to: 0; duration: 350 }
        NumberAnimation { target: content; properties: 'opacity'; from: 0; to: 1; duration: 350 }
        NumberAnimation { target: content; properties: 'opacity'; from: 1; to: 0; duration: 350 }
        NumberAnimation { target: content; properties: 'opacity'; from: 0; to: 1; duration: 350 }
    }

    Component.onCompleted: {
        var activatedAtMs = Util.utcDateTimeToMillisec(activeSystemAlerts[alertIdx].ActiveAt);
        var curTimeMs = Util.getCurrentUtcDateTime().getTime();
        var timePastMs = curTimeMs - activatedAtMs;

        //logDebug("Alert" + alertIdx + ": ActiveAt=" + activeSystemAlerts[alertIdx].ActiveAt + ", Ms=" + activatedAtMs + ", timePastMs=" + timePastMs);

        if (timePastMs < 1000)
        {
            animationNewAlert.start();
        }
    }

    function getNameText()
    {
        if (activeSystemAlerts[alertIdx] === undefined)
        {
            return "";
        }

        return translate("T_" + activeSystemAlerts[alertIdx].CodeName + "_Name");
    }

    function getUserDirectionText()
    {
        if (activeSystemAlerts[alertIdx] === undefined)
        {
            return "";
        }
        else if (activeSystemAlerts[alertIdx].Data.length > 0)
        {
            var argStr = activeSystemAlerts[alertIdx].Data;
            argStr = argStr.replace(/;/g, ":");
            return translate("T_" + activeSystemAlerts[alertIdx].CodeName + "_UserDirection;" + argStr);
        }

        return translate("T_" + activeSystemAlerts[alertIdx].CodeName + "_UserDirection");
    }
}
