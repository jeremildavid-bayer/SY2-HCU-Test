import QtQuick 2.12
import "../../Util.js" as Util
import "../../Widgets"

Item {
    property var examAdvanceInfo: dsExam.examAdvanceInfo
    property bool licenseEnabledPatientStudyContext: dsCru.licenseEnabledPatientStudyContext
    property var cruLinkStatus: dsCru.cruLinkStatus
    property bool isExamStarted: dsExam.isExamStarted

    id: root

    visible: {
        if ( (licenseEnabledPatientStudyContext) &&
             (cruLinkStatus.State === "Active") &&
             (isExamStarted) &&
             (!examAdvanceInfo.MandatoryFieldsEntered) )
        {
            return true;
        }
        return false;
    }

    Text {
        id: iconWarning
        width: contentWidth
        height: contentHeight
        font.family: fontIcon.name
        color: colorMap.errText
        font.pixelSize: parent.width * 0.05
        font.bold: true
        text: "\ue945"

    }

    Text {
        id: textAlert
        anchors.left: iconWarning.right
        anchors.leftMargin: iconWarning.width * 0.5
        anchors.right: parent.right
        anchors.rightMargin: parent.width * 0.05
        height: contentHeight
        font.pixelSize: root.height * 0.038
        font.family: fontRobotoLight.name
        color: colorMap.text01
        wrapMode: Text.Wrap
        text: translate("T_MandatoryFieldsNotEntered")
    }
}
