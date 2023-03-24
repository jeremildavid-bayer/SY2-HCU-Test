import QtQuick 2.5

Rectangle {
    property int buttonSpacing: frameKeyboard.width * 0.01
    property int buttonWidth: ((frameKeyboard.width - buttonSpacing) / 11) - buttonSpacing
    property int buttonHeight: (frameKeyboard.height / 4) - buttonSpacing
    property int animationMs: 250
    property string mode: "NORMAL" // "NORMAL", "CAP", "NUM1", "NUM2"

    signal signalActivated()
    signal signalDeactivated()

    id: keyboardInner

    y: window.height
    color: "#020202"
    width: window.width
    height: window.height / 2

    state: "INACTIVE"

    states: [
        State { name: "ACTIVE" },
        State { name: "INACTIVE" }
    ]

    transitions: [
        Transition {
            to: "ACTIVE"
            SequentialAnimation {
                ParallelAnimation {
                    NumberAnimation { target: keyboardInner; properties: "y"; to: window.height - keyboardInner.height; duration: animationMs }
                    NumberAnimation { target: window; properties: "y"; to:newMainWindowY; duration: animationMs }
                }

                ScriptAction { script: {
                        signalActivated();
                    }
                }
            }
        },

        Transition {
            to: "INACTIVE"

            SequentialAnimation {
                ParallelAnimation {
                    NumberAnimation { target: keyboardInner; properties: "y"; to: window.height; duration: animationMs }
                    NumberAnimation { target: window; properties: "y"; to:prevMainWindowY; duration: animationMs }
                }

                ScriptAction { script: {
                        signalDeactivated();
                    }
                }
            }
        }
    ]

    MouseArea {
        // Prevent background touch
        anchors.fill: parent
    }

    Item {
        //color: "pink"
        id: frameKeyboard
        width: parent.width * 0.95
        height: parent.height * 0.82
        anchors.centerIn: parent

        Column {
            spacing: buttonSpacing
            Row {
                spacing: buttonSpacing
                KeyboardInnerBtn { keys: ["q", "Q", "1", "1"] }
                KeyboardInnerBtn { keys: ["w", "W", "2", "2"] }
                KeyboardInnerBtn { keys: ["e", "E", "3", "3"] }
                KeyboardInnerBtn { keys: ["r", "R", "4", "4"] }
                KeyboardInnerBtn { keys: ["t", "T", "5", "5"] }
                KeyboardInnerBtn { keys: ["y", "Y", "6", "6"] }
                KeyboardInnerBtn { keys: ["u", "U", "7", "7"] }
                KeyboardInnerBtn { keys: ["i", "I", "8", "8"] }
                KeyboardInnerBtn { keys: ["o", "O", "9", "9"] }
                KeyboardInnerBtn { keys: ["p", "P", "0", "0"] }
                KeyboardInnerBtn { keys: ["\ue965", "\ue965", "\ue965", "\ue965"]; width: buttonWidth + (buttonSpacing * 2) }
            }

            Row {
                spacing: buttonSpacing
                x: (editMode == "SINGLE_LINE") ? (buttonSpacing * 5) : (buttonSpacing * 2)
                KeyboardInnerBtn { keys: ["a", "A", "!", "`"] }
                KeyboardInnerBtn { keys: ["s", "S", "@", "~"] }
                KeyboardInnerBtn { keys: ["d", "D", "#", "["] }
                KeyboardInnerBtn { keys: ["f", "F", "$", "]"] }
                KeyboardInnerBtn { keys: ["g", "G", "%", "{"] }
                KeyboardInnerBtn { keys: ["h", "H", "^", "}"] }
                KeyboardInnerBtn { keys: ["j", "J", "&", "\\"] }
                KeyboardInnerBtn { keys: ["k", "K", "*", "|"] }
                KeyboardInnerBtn { keys: ["l", "L", "<", ";"] }
                KeyboardInnerBtn { keys: ["'", "\"", ">", ":"] }
                KeyboardInnerBtn { keys: ["<-", "<-", "<-", "<-"]; width: buttonWidth; visible: editMode == "MULTI_LINE"}
            }

            Row {
                spacing: buttonSpacing
                KeyboardInnerBtn { keys: ["\ue966", "\ue911", "1/2", "2/2"]; width: buttonWidth + (buttonSpacing * 2) }
                KeyboardInnerBtn { keys: ["z", "Z", "_", "_"] }
                KeyboardInnerBtn { keys: ["x", "X", "-", "-"] }
                KeyboardInnerBtn { keys: ["c", "C", "=", "="] }
                KeyboardInnerBtn { keys: ["v", "V", "+", "+"] }
                KeyboardInnerBtn { keys: ["b", "B", "(", "("] }
                KeyboardInnerBtn { keys: ["n", "N", ")", ")"] }
                KeyboardInnerBtn { keys: ["m", "M", "/", "/"] }
                KeyboardInnerBtn { keys: [",", ";", ",", ","] }
                KeyboardInnerBtn { keys: [".", ":", ".", "."] }
                KeyboardInnerBtn { keys: ["?", "!", "?", "?"] }
            }

            Row {
                spacing: buttonSpacing
                KeyboardInnerBtn { keys: ["\ue957\n\ue906", "\ue957\n\ue906", "\ue957\n\ue906", "\ue957\n\ue906"]; width: (buttonWidth + (buttonSpacing * 5)); backgroundColor: "#888888"; btnTextFontPixelSize: height * 0.35 }
                KeyboardInnerBtn { keys: ["?123", "?123", "ABC", "ABC"]; }
                KeyboardInnerBtn { keys: [" ", " ", " ", " "]; width: (buttonWidth + buttonSpacing) * 5 }
                KeyboardInnerBtn { keys: ["\ue909", "\ue909", "\ue909", "\ue909"]; }
                KeyboardInnerBtn { keys: ["\ue908", "\ue908", "\ue908", "\ue908"]; }
                KeyboardInnerBtn { keys: ["OK", "OK", "OK", "OK"]; width: (buttonWidth + (buttonSpacing * 5)); backgroundColor: "yellow" }
            }
        }
    }
}
