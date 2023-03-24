import QtQuick 2.12
import "../"

InputPadGeneric {
    property var selectOptions
    property string selectUnits: ""
    property var customListItems: null

    okBtnVisible: false

    content: [
        ListView {
            id: listView
            anchors.fill: parent
            clip: true
            spacing: parent.height * 0.02

            ScrollBar {}
            ListFade {
                fadeColor: colorMap.subPanelBackground
            }

            delegate: GenericButton {
                width: ListView.view.width
                height: ListView.view.height * 0.17
                color: colorMap.keypadButton
                radius: 0

                content: [
                    Item {
                        width: parent.width * 0.9
                        height: parent.height
                        anchors.horizontalCenter: parent.horizontalCenter

                        Text {
                            id: defaultItem
                            anchors.fill: parent
                            font.pixelSize: height * 0.36
                            verticalAlignment: Text.AlignVCenter
                            font.family: fontRobotoLight.name
                            color: colorMap.text01
                            text: {
                                if (visible)
                                {
                                    var optionData = selectOptions[index];

                                    if ( (optionData === undefined) ||
                                            (optionData.title === "") )
                                    {
                                        return "--";
                                    }
                                    return optionData.title + " " + translate(selectUnits);
                                }
                                else
                                {
                                    return "";
                                }
                            }
                            wrapMode: Text.Wrap
                            fontSizeMode: Text.Fit
                            minimumPixelSize: font.pixelSize * 0.7
                        }

                        // reset customListItems in case it changed
                        onVisibleChanged:
                        {
                            resetCustomListItems(this);
                        }

                        Component.onCompleted: {
                            resetCustomListItems(this);
                        }

                        function resetCustomListItems(newParent)
                        {
                            if (customListItems !== undefined && customListItems[index] !== undefined)
                            {
                                customListItems[index].parent = newParent;
                                customListItems[index].visible = true;
                                defaultItem.visible = false;
                            }
                            else
                            {
                                defaultItem.visible = true;
                            }
                        }
                    }
                ]

                Component.onCompleted: {
                    listView.dragStarted.connect(reset);
                }

                Component.onDestruction: {
                    listView.dragStarted.disconnect(reset);
                }

                pressedSoundCallback: keyPressedSoundCallback
                onBtnClicked: {
                    if (currentValue !== selectOptions[index].title)
                    {
                        currentValue = selectOptions[index].title;
                        textWidget.text = currentValue;
                        signalValueChanged(currentValue);
                    }
                    close(true);
                }
            }
        }
    ]

    function init(selectList, newSelectUnits, newCustomListItems)
    {
        customListItems = newCustomListItems;
        selectUnits = newSelectUnits;

        // Set select list
        var newSelectOptions = [];
        for (var i = 0; i < selectList.length; i++)
        {
            newSelectOptions.push({ "title" : selectList[i] });
        }

        selectOptions = newSelectOptions;
        listView.model = selectOptions;        
    }
}

