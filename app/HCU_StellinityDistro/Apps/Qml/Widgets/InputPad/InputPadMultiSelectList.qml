import QtQuick 2.12
import "../"

InputPadGeneric {
    property var selectOptions
    property bool keyboardBtnEnabled: false
    property var currentContentY

    content: [
        ListView {
            id: listView

            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: keyboardBtn.top

            clip: true

            ScrollBar {}
            ListFade {
                fadeColor: colorMap.subPanelBackground
            }

            delegate: Item {
                property var itemData: selectOptions[index]

                id: root
                width: ListView.view.width
                height: ( (root.itemData === undefined) || (root.itemData.visible === undefined) || (root.itemData.visible) ) ? ListView.view.height * 0.18 : 0
                clip: true

                GenericButton {
                    id: btn
                    width: parent.width
                    height: parent.height * 0.92
                    color: colorMap.keypadButton
                    radius: 0

                    Rectangle {
                        id: iconValue
                        x: visible ? width / 2 : 0
                        width: visible ? parent.width * 0.1 : 0
                        radius: width / 2
                        height: width
                        anchors.verticalCenter: parent.verticalCenter
                        color: ( (root.itemData.value !== undefined) && (root.itemData.value) ) ? colorMap.actionButtonBackground : colorMap.gry01
                        visible: root.itemData.value !== undefined
                    }

                    Text {
                        anchors.left: iconValue.right
                        anchors.leftMargin: parent.width * 0.08
                        anchors.right: parent.right
                        anchors.rightMargin: parent.width * 0.05
                        height: parent.height
                        font.pixelSize: height * 0.33
                        verticalAlignment: Text.AlignVCenter
                        font.family: fontRobotoLight.name
                        color: colorMap.text01
                        text: {
                            if (root.itemData === undefined)
                            {
                                return "";
                            }

                            if (root.itemData.text !== undefined)
                            {
                                return root.itemData.text;
                            }
                            return root.itemData.title;
                        }
                        wrapMode: Text.Wrap
                        fontSizeMode: Text.Fit
                        minimumPixelSize: font.pixelSize * 0.5
                    }

                    Component.onCompleted: {
                        listView.dragStarted.connect(reset);
                    }

                    Component.onDestruction: {
                        listView.dragStarted.disconnect(reset);
                    }

                    onBtnClicked: {
                        if (itemData.value !== undefined)
                        {
                            selectOptions[index].value = !itemData.value;
                            root.itemData = Qt.binding(function() { return selectOptions[index];});
                            currentValue = selectOptions;
                            signalValueChanged(currentValue);
                        }
                        else
                        {
                            // Custom Button (e.g. keyboard)
                            signalValueChanged(itemData.title);
                        }
                    }
                }
                Item {
                    // spacing
                    anchors.top: btn.bottom
                    anchors.bottom: parent.bottom
                    width: parent.width
                }
            }
        },

        // if Keyboard is enabled, show it at the bottom of list but not scrolling with the list
        GenericButton
        {
            id: keyboardBtn
            visible: keyboardBtnEnabled
            anchors.bottom: parent.bottom
            width: parent.width
            height: visible ? parent.height * 0.135 : 0
            color: colorMap.keypadButton
            radius: 0

            Text
            {
                id: keyboardBtnText
                anchors.fill: parent
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: parent.height * 0.4
                font.family: fontRobotoLight.name
                color: colorMap.text01
                text: "\ue957"
            }

            onBtnClicked: {
                signalValueChanged(keyboardBtnText.text);
            }
        }

    ]

    function init(newSelectOptions, newKeyboardBtnEnabled)
    {
        if (newKeyboardBtnEnabled === undefined)
        {
            newKeyboardBtnEnabled = false;
        }

        if (newKeyboardBtnEnabled !== keyboardBtnEnabled)
        {
            keyboardBtnEnabled = newKeyboardBtnEnabled;
        }

        // Set select list
        selectOptions = newSelectOptions;
        listView.model = selectOptions;
    }

    // call when list options should be updated while it's still open (QuickNote)
    function resetSelectOptions(newSelectOptions)
    {
        //invalidate and re-set
        selectOptions = newSelectOptions;
        currentContentY = listView.contentY;
        listView.model = selectOptions;
        currentValue = selectOptions;

        // don't make list jump to top of list again
        listView.contentY = currentContentY;
    }
}

