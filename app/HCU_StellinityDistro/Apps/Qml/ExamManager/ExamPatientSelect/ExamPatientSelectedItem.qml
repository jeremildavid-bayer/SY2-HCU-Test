import QtQuick 2.12
import "../../Widgets"
import "../../Util.js" as Util

GenericButton {
    property double highlightPercent: 0
    property int borderWidth: 2
    property var examAdvanceInfo: dsExam.examAdvanceInfo
    property int margin: width * 0.01

    id: root
    width: parent.width
    height: rowHeight

    content: [
        Rectangle {
            id: highlightBorder
            x: -margin
            width: (parent.width * highlightPercent) + margin
            radius: buttonRadius
            height: parent.height
            border.width: borderWidth
            border.color: colorMap.actionButtonBackground
            color: colorMap.subPanelBackground
            clip: true

            ExamPatientSelectListItem {
                id: patientSelected
                x: margin
                width: root.width
                height: rowHeight
                isListItem: false
                interactive: false
                rowData: {
                    if ( (examAdvanceInfo !== undefined) &&
                         (examAdvanceInfo.WorklistDetails !== undefined) )
                    {
                        return examAdvanceInfo.WorklistDetails.Entry;
                    }
                    return undefined;
                }
            }

            Rectangle {
                id: borderRightEnd1
                anchors.right: borderRightEnd2.left
                anchors.rightMargin: -buttonRadius
                width: buttonRadius
                height: parent.height
                color: colorMap.actionButtonBackground
            }

            Rectangle {
                id: borderRightEnd2
                radius: buttonRadius
                width: examPatientSelect.width * 0.015
                height: parent.height
                color: colorMap.actionButtonBackground
                anchors.right: parent.right
            }

            Text {
                id: stateIcon
                visible: root.interactive
                anchors.left: borderRightEnd1.right
                anchors.leftMargin: borderRightEnd1.anchors.rightMargin
                anchors.right: parent.right
                height: parent.height
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                color: colorMap.actionButtonText
                font.pixelSize: parent.height * 0.3
                font.family: fontIcon.name
                text: "\ue909"
                font.bold: true
            }
        }
    ]

    onBtnClicked: {
        examPatientSelect.deSelectEntry();
    }
}

