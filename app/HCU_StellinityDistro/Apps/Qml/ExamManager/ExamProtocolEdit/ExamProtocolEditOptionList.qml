import QtQuick 2.12
import "../../Widgets"

Drawer {
    property int rowHeight: height * 0.11
    property int rowWidth: width * 0.7
    property int rowSpacing: parent.height * 0.025

    content: [
        Item {
            y: (injectionElapsedTime.height == 0) ? (parent.height * 0.11) : (parent.height * 0.03) + injectionElapsedTime.height
            width: rowWidth
            height: (rowHeight * 6) + (rowSpacing * 5)
            anchors.horizontalCenter: parent.horizontalCenter

            MouseArea {
                // Prevent background touch
                anchors.fill: parent
            }

            Column {
                spacing: rowSpacing
                ExamProtocolEditOption { height: rowHeight; width: rowWidth; itemType: "TEST_INJECTION" }
                ExamProtocolEditOption { height: rowHeight; width: rowWidth; itemType: "NEW_STEP" }
                ExamProtocolEditOption { height: rowHeight; width: rowWidth; itemType: "CONTRAST_PHASE" }
                ExamProtocolEditOption { height: rowHeight; width: rowWidth; itemType: "DUAL_PHASE" }
                ExamProtocolEditOption { height: rowHeight; width: rowWidth; itemType: "SALINE_PHASE" }
                ExamProtocolEditOption { height: rowHeight; width: rowWidth; itemType: "DELAY_PHASE" }
            }
        }
    ]

    onSignalOpen: {
        protocolOverview.enabled = false;
    }

    onSignalClosed: {
        protocolOverview.enabled = true;
    }
}
