import QtQuick 2.3

Rectangle {
    property int optionIndex: index
    property var titleColor: prop_listOptValues[index].titleColor
    property var titleText: prop_listOptValues[index].titleText
    property var titleTextColor: prop_listOptValues[index].titleTextColor
    property var isStepType: prop_listOptValues[index].isStepType

    width: parent.width;
    height: optionItemH;
    id: optionItem
    state: "IDLE"

    states: [
        State { name: "IDLE" },
        State { name: "DRAGGING" }
    ]

    transitions: [
        Transition {
            from: "IDLE"
            to: "DRAGGING"

            SequentialAnimation {
                ScriptAction { script: {
                        console.log("state= IDLE!");
                    }
                }
            }
        },
        Transition {
            from: "DRAGGING"
            to: "IDLE"

            SequentialAnimation {
                ParallelAnimation {
                    NumberAnimation { target: dragObject; properties: 'x'; to: parent.mapFromItem(optionItem, parent.x, parent.y).x; duration: optionItemPullAnimationMs }
                    NumberAnimation { target: dragObject; properties: 'y'; to: parent.mapFromItem(optionItem, parent.x, parent.y).y; duration: optionItemPullAnimationMs }
                    NumberAnimation { target: optionItem; properties: 'opacity'; to: 1; duration: optionItemPullAnimationMs }
                }

                ScriptAction { script: {
                        optionListView.interactive = true;
                    }
                }
            }
        }
    ]

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

        color: titleColor
        x: 0
        y: phaseListRowSpacing
        width: parent.width
        height: parent.height - (phaseListRowSpacing * 2)

        Text {
            anchors.fill: parent
            verticalAlignment: Text.AlignVCenter
            color: titleTextColor
            text: titleText
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
                optionItem.state = "DRAGGING";
                optionListView.interactive = false;
                optionItem.opacity = 0.5;
                dragObject.init(optionItem, 0);
                optionListView.curOptionData = prop_listOptValues[index];

                if (isStepType)
                {
                    stepList.onInsertStepStarted();
                }
                else
                {
                    stepList.onInsertPhaseStarted();
                }
            }

            onReleased: {
                if (isStepType)
                {
                    stepList.onInsertStepDone();
                }
                else
                {
                    stepList.onInsertPhaseDone();
                    stepList.onSortRowDone();
                }
                optionItem.state = "IDLE";
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
