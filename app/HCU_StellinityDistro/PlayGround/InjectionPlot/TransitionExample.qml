import QtQuick 2.5

Rectangle {
    id: root
    anchors.fill: parent;

    states: [
        State {
            name: "rotated"
            PropertyChanges { target: page1; rotation: 180   }
        }
    ]

    transitions: Transition {
        RotationAnimation {
            duration: 1000;
            direction: RotationAnimation.Counterclockwise
        }
    }


    Rectangle {
        id: page1
        width: parent.width
        height: parent.height
        color: "red"

        Rectangle {
            id: btn1
            y: 50
            x: 50
            width: 100
            height: 100
            color: "green"

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    console.log("btn1 clicked");
                    root.state = "rotatedaa";
                    root.state = "rotated";

                }
            }
        }

    }

    Rectangle {
        id: page2
        width: parent.width
        height: parent.height
        color: "blue"
    }

    Component.onCompleted: {
        page2.visible = false;
    }
}
