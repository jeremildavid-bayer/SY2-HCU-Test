import QtQuick 2.12
import "../../Util.js" as Util
import "../../Widgets"

Item {
    property bool licenseEnabledPatientStudyContext: dsCru.licenseEnabledPatientStudyContext
    property int rowHeightMin: examSummaryView.height * 0.15

    visible: licenseEnabledPatientStudyContext
    width: parent.width
    anchors.top: parent.top
    anchors.bottom: parent.bottom
    clip: true

    Flickable {
        id: flickAdvanceFields
        anchors.fill: parent
        flickableDirection: Flickable.VerticalFlick
        contentHeight: rectContent.height
        contentWidth: width

        Column {
            id: rectContent
            width: parent.width

            ExamSummaryAdvanceListItemNote {}

            Rectangle {
                // Separator
                width: parent.width
                height: 2
                color: colorMap.text02
            }

            ExamSummaryAdvanceListItemTechId {}
        }
    }

    ScrollBar {
        flickable: flickAdvanceFields
    }

    ListFade {
        flickable: flickAdvanceFields
    }
}
