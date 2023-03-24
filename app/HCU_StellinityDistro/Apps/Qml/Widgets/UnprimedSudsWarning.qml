import QtQuick 2.12

Rectangle {
    id: root
    visible: ensureNoPatientConnected()
    anchors.fill: parent
    color: "transparent"
    border.color: "red"
    border.width: 15
    function ensureNoPatientConnected() {
        var idx = dsAlert.activeAlerts.findIndex(alert => alert.CodeName === "EnsureNoPatientConnected");
        return (idx !== -1);
    }

    Rectangle {
        id: textBackground
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        width: unprimedSudsWarningText.contentWidth * 1.5
        height: parent.height * 0.03
        color: colorMap.mainBackground
        Text {
            id: unprimedSudsWarningText
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: parent.height
            text: translate("T_EnsureNoPatientConnected")
            color: colorMap.text01
            font.family: fontRobotoBold.name
            font.pixelSize: height
            horizontalAlignment: Text.AlignHCenter
        }
    }
}
