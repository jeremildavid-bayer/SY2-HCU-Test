import QtQuick 2.9

Item {
    FontLoader {
        id: fontIcon
        source: "file:///home/user/Imaxeon/bin/resources/Fonts/Stellinity2-UI.ttf"
    }

    MySlider {
        id: mySlider
        width: parent.width * 0.8
        height: parent.height * 0.3
        anchors.centerIn: parent
    }

    Component.onCompleted: {
        var labels = [];
        for (var i = 0; i < 20; i++)
        {
            labels.push(i * 10);
        }

        mySlider.setLabels(labels)
        mySlider.setValueIndex(1);
    }

    /*Rectangle {
        color: "pink"
        width: 400
        height: 400
        anchors.centerIn: parent
    }

    Item {
        id: cursor

        width: 30
        height: 30

        Rectangle {
            color: "black"
            anchors.verticalCenter: parent.verticalCenter
            width: parent.width
            height: 1
        }

        Rectangle {
            color: "black"
            anchors.horizontalCenter: parent.horizontalCenter
            width: 1
            height: parent.height
        }

    }

    MouseArea {
        anchors.fill: parent
        drag.target: cursor

        onClicked: {
            cursor.x = mouse.x - (cursor.width / 2);
            cursor.y = mouse.y - (cursor.height / 2);
        }

    }*/



}
