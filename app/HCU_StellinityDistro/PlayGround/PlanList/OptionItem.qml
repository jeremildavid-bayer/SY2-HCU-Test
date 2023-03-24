import QtQuick 2.3

Rectangle {
    property int rowIndex: index
    property var titleText: prop_listValues[index].titleText
    property var values: prop_listValues[index].items
    property bool expanded: false//prop_listValues[index].expanded

    id: root
    width: parent.width
    state: "COMPRESSED"
    height: expanded ? title.height + items.height : title.height

    Rectangle {
        id: title
        opacity: 1
        height: rowHeight
        width: parent.width
        color: titleBackgroundColor

        Text {
            anchors.fill: parent
            verticalAlignment: Text.AlignVCenter
            text: titleText
            color: titleTextColor
            font.pixelSize: titleTextFontPixelSize
            anchors.leftMargin: textMargin
        }

        Rectangle {
            id: stateIconFrame
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.margins: textMargin
            width: height
            color: "transparent"

            Image {
                id: stateIcon
                anchors.fill: parent
                source: expanded ? "./circleMinus.svg" : "./circlePlus.svg"
                sourceSize.height: parent.height
                sourceSize.width: parent.width
            }
        }

        MouseArea {
            anchors.fill: parent
            onPressed: {
                title.opacity = 0.5
            }

            onReleased: {
                title.opacity = 1;
            }

            onClicked: {
                title.opacity = 1
                root.state = (root.state === "COMPRESSED") ? "EXPANDED" : "COMPRESSED";
                console.log("Setting root.state=" + root.state);
            }
        }
    }

    states: [
        State { name: "COMPRESSED" },
        State { name: "EXPANDED" }
    ]

    transitions: [
        Transition {
            from: "COMPRESSED"
            to: "EXPANDED"
            SequentialAnimation {
                ParallelAnimation {
                    NumberAnimation { target: stateIconFrame; properties: 'rotation'; to: 180; duration: animationMs }
                    NumberAnimation { target: root; properties: 'height'; to: title.height + items.height; duration: animationMs }
                    NumberAnimation { target: items; properties: 'opacity'; to: 1; duration: animationMs }
                }

                ScriptAction { script: {
                        expanded = true;
                    }
                }
            }
        },

        Transition {
            from: "EXPANDED"
            to: "COMPRESSED"

            SequentialAnimation {
                ParallelAnimation {
                    NumberAnimation { target: stateIconFrame; properties: 'rotation'; to: 0; duration: animationMs }
                    NumberAnimation { target: root; properties: 'height'; to: title.height; duration: animationMs }
                    NumberAnimation { target: items; properties: 'opacity'; to: 0; duration: animationMs }
                }

                ScriptAction { script: {
                        expanded = false;
                    }
                }
            }
        }
    ]

    ListView {
        opacity: expanded ? 1 : 0
        id: items
        height: prop_listValues[index].items.length * (rowHeight + listSpacing)
        width: parent.width - itemMargin
        y: rowHeight + listSpacing
        x: itemMargin
        model: values
        spacing: listSpacing
        interactive: false
        delegate: Rectangle {
            id: itemDelegate
            height: rowHeight
            width: parent.width
            color: itemBackgroundColor

            Text {
                anchors.fill: parent
                verticalAlignment: Text.AlignVCenter
                text: values[index]
                color: itmeTextColor
                font.pixelSize: itemTextFontPixelSize
                anchors.leftMargin: textMargin
            }

            MouseArea {
                enabled: expanded
                anchors.fill: parent

                onPressed: {
                    itemDelegate.opacity = 0.5
                }

                onReleased: {
                    itemDelegate.opacity = 1;
                }

                onClicked: {
                    itemDelegate.opacity = 1
                    listMain.itemClicked(rowIndex, values[index]);
                }
            }

            Component.onCompleted: {
                listMain.listDragStarted.connect(onListDragStarted);
            }


            function onListDragStarted () {
                if (itemDelegate !== undefined)
                {
                    itemDelegate.opacity = 1.0;
                }
            }
        }
    }

    Component.onCompleted: {
        listMain.listDragStarted.connect(onListDragStarted);
    }


    function onListDragStarted () {
        if (title !== undefined)
        {
            title.opacity = 1.0;
        }
    }
}

