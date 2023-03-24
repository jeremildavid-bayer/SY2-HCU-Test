import QtQuick 2.12
import "../../Widgets"

Item {
    property string field: ""
    property string fieldText: ""
    property double colWidthPercent: 0.1
    property int textMargin: height * 0.1
    property bool isActive: (curSortType.indexOf(field) == 0)
    property bool isActiveUp: (curSortType.indexOf(field + "-Up") == 0)
    property int animationMs: 250
    property int leftMargin: 0

    width: parent.width * colWidthPercent
    height: parent.height

    GenericButton {
        x: leftMargin
        width: parent.width - x
        height: parent.height
        color: colorMap.homeBackground
        radius: 0
        interactive: !isEntrySelected()

        content: [
            Item {
                anchors.fill: parent
                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: textMargin
                    anchors.right: stateIconFrame.left
                    anchors.rightMargin: textMargin
                    font.pixelSize: isActive ? parent.height * 0.4 : parent.height * 0.35
                    height: parent.height
                    font.family: isActive ? fontRobotoBold.name : fontRobotoMedium.name
                    font.bold: isActive
                    text: fieldText
                    wrapMode: Text.Wrap
                    color: colorMap.text01
                    verticalAlignment: Text.AlignVCenter
                    fontSizeMode: Text.Fit
                }

                Rectangle {
                    id: stateIconFrame
                    visible: isActive
                    anchors.right: parent.right
                    anchors.rightMargin: textMargin
                    anchors.verticalCenter: parent.verticalCenter
                    height: parent.height * 0.8
                    width: stateIcon.width
                    color: "transparent"
                    rotation: isActiveUp ? 180 : 0
                    Behavior on rotation {
                        PropertyAnimation {
                            duration: animationMs
                        }
                    }

                    Text {
                        id: stateIcon
                        width: contentWidth
                        height: parent.height
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        color: colorMap.text01
                        font.pixelSize: parent.height * 0.3
                        font.family: fontIcon.name
                        text: "\ue906"
                        font.bold: true
                    }
                }
            }
        ]

        onBtnClicked: {
            if (isActiveUp)
            {
                examPatientSelect.curSortType = field + "-Dn";
            }
            else
            {
                examPatientSelect.curSortType = field + "-Up";
            }
        }
    }
}
