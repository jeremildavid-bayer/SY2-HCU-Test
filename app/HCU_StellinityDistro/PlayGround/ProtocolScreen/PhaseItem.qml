import QtQuick 2.3

Rectangle {
    property bool isPhantom: false

    id: phaseItem
    width: phaseItemW
    height: phaseItemH
    anchors.left: parent.left
    anchors.leftMargin: phaseLeftMargin;
    opacity: ((!isPhantom) && ((curSortingRowIndex === index) || (curSortingStepTitle == stepId))) ? 0.2 : 1

    Rectangle
    {
        id: phaseSpacing1
        y: 0
        x: 0
        width: parent.width
        height: phaseListRowSpacing
    }

    Rectangle {
        id: phaseBody
        x: 0
        y: phaseListRowSpacing
        width: parent.width
        height: parent.height - (phaseListRowSpacing * 2)

        //anchors.fill: parent
        color: titleColor

        Text {
            text: "P:" + titleColor
            anchors.fill: parent
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: 20
            anchors.leftMargin: 10
        }

        MouseArea {
            id: dragHandle
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.margins: dragHandleMargin
            width: dragHandleWidth
            height: dragHandleHeight
            drag.target: dragObject

            Rectangle {
                id: handle
                color: "white"
                anchors.fill: parent
            }

            //onPressAndHold: {
            onPressed: {
                console.log("sorting start from index=" + index);
                dragObject.init(phaseItem, 0);
                stepList.onSortPhaseStarted(stepId, index);
                mouse.accepted = false;
            }
        }
    }

    Rectangle
    {
        id: phaseSpacing2
        y: parent.height - phaseListRowSpacing
        x: 0
        width: parent.width
        height: phaseListRowSpacing
    }
}




