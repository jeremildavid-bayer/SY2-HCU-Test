import QtQuick 2.3
import QtQuick.Window 2.2
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.3

Window {
    id: topWindow
    x: 0
    width: 1900
    height: 1200
    visible: true
    objectName: "main"
    color: "white"

    FontLoader {
        id: fontIcon
        source: "file:///home/user/Imaxeon/bin/resources/Fonts/Stellinity2-UI.ttf"
    }

    Rectangle {
        id: window
        x: 0
        y: 0
        width: 1900
        height: 1200
        color: "lightblue"

        Rectangle {
            id: editArea1
            color: "white"
            x: parent.width * 0.1
            width: parent.width * 0.5
            y: parent.height * 0.2
            height: parent.height * 0.1

            Text {
                id: textEdit1
                anchors.fill: parent
                font.pixelSize:  height * 0.8
                verticalAlignment: Text.AlignVCenter
                text: "Click to edit"
            }

            MouseArea {
                anchors.fill: parent
                onPressed: {
                    editArea1.opacity = 0.5;
                }
                onReleased: {
                    editArea1.opacity = 1;
                }
                onClicked: {
                    keyboard.open(editArea1, textEdit1);
                }
            }
        }

        Rectangle {
            id: editArea2
            color: "white"
            x: parent.width * 0.1
            width: parent.width * 0.5
            y: parent.height * 0.7
            height: parent.height * 0.1

            Text {
                id: textEdit2
                anchors.fill: parent
                font.pixelSize:  height * 0.8
                verticalAlignment: Text.AlignVCenter
                text: "Click to edit"
            }

            MouseArea {
                anchors.fill: parent
                onPressed: {
                    editArea2.opacity = 0.5;
                }
                onReleased: {
                    editArea2.opacity = 1;
                }
                onClicked: {
                    keyboard.open(editArea2, textEdit2);
                }
            }
        }
    }

    Keyboard {
        id: keyboard
    }

    Component.onCompleted: {
        keyboard.signalValueChanged.connect(slotKeyboardValChanged);
        keyboard.signalClosed.connect(slotKeyboardClosed);
    }

    Component.onDestruction: {
        keyboard.signalValueChanged.disconnect(slotKeyboardValChanged);
        keyboard.signalClosed.disconnect(slotKeyboardClosed);
    }

    function slotKeyboardClosed(modified)
    {
    }

    function slotKeyboardValChanged(newValue)
    {
    }
}





