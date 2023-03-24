import QtQuick 2.3

Rectangle {
    property bool isPhantom: false

    id: stepItem
    width: stepItemW
    height: stepHeaderItemH

    Rectangle
    {
        id: stepSpacing1
        y: 0
        x: 0
        width: parent.width
        height: phaseListRowSpacing
    }

    Rectangle
    {
        id: stepBody
        x: 0
        y: phaseListRowSpacing
        width: parent.width
        height: parent.height - (phaseListRowSpacing * 2)

        color: "lightsteelblue"
        opacity: ((!isPhantom) && (curSortingStepTitle == section)) ? 0.2 : 1

        Text {
            text: section
            font.bold: true
            font.pixelSize: 20
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
                console.log("Step: sorting start from " + section);
                stepList.onSortStepStarted(section);
                mouse.accepted = false;

            }
        }
    }

    Rectangle
    {
        id: stepSpacing2
        y: parent.height - phaseListRowSpacing
        x: 0
        width: parent.width
        height: phaseListRowSpacing
    }
}



