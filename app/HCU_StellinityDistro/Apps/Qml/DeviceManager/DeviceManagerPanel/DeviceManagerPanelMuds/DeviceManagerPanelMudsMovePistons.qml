import QtQuick 2.12
import ".."
import "../../DeviceManagerWidgets"
import "../../../Widgets"

Drawer {
    property var syringeStates: dsMcu.syringeStates
    property int btnHeight: height * 0.15
    property int btnWidth: width * 0.5
    property int rowSpacing: height * 0.07

    edgeDragEnabled: false

    content: [
        Rectangle {
            id: root
            width: parent.width * 0.6
            height: parent.height * 0.8
            anchors.centerIn: parent
            color: colorMap.subPanelBackground

            Item {
                id: textTitle
                width: parent.width
                height: parent.height * 0.1

                Text {
                    id: icon
                    font.family: fontIcon.name
                    text: "\ue971"
                    height: parent.height
                    width: contentWidth * 2
                    font.pixelSize: height * 0.7
                    color: colorMap.text01
                    verticalAlignment: Text.AlignVCenter

                }

                Text {
                    anchors.left: icon.right
                    anchors.right: parent.right
                    height: parent.height
                    font.family: fontRobotoBold.name
                    text: translate("T_DaySetCleaningMode_Title")
                    font.pixelSize: height * 0.6
                    color: colorMap.text01
                    verticalAlignment: Text.AlignVCenter
                    wrapMode: Text.Wrap
                }
            }


            GenericButton {
                id: btnExtendAll
                color: colorMap.keypadButton
                anchors.top: textTitle.bottom
                anchors.topMargin: rowSpacing * 1.5
                anchors.horizontalCenter: parent.horizontalCenter
                width: btnWidth
                height: btnHeight

                content: [
                    Row {
                        anchors.horizontalCenter: parent.horizontalCenter
                        height: parent.height
                        Repeater {
                            model: 3
                            Text {
                                width: contentWidth * 2
                                height: parent.height
                                font.family: fontIcon.name
                                text: "\uf176"
                                font.pixelSize: height * 0.3
                                color: colorMap.text01
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                        }
                    }
                ]
                onBtnClicked: {
                    dsMcu.slotMovePistonToCleaningPos(0);
                    dsMcu.slotMovePistonToCleaningPos(1);
                    dsMcu.slotMovePistonToCleaningPos(2);
                }
            }


            GenericButton {
                id: btnRetractAll
                color: colorMap.keypadButton
                anchors.top: btnExtendAll.bottom
                anchors.topMargin: rowSpacing
                anchors.horizontalCenter: parent.horizontalCenter
                width: btnWidth
                height: btnHeight

                content: [
                    Row {
                        anchors.horizontalCenter: parent.horizontalCenter
                        height: parent.height
                        Repeater {
                            model: 3
                            Text {
                                width: contentWidth * 2
                                height: parent.height
                                font.family: fontIcon.name
                                text: "\uf175"
                                font.pixelSize: height * 0.3
                                color: colorMap.text01
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                        }
                    }
                ]
                onBtnClicked: {
                    dsMcu.slotPistonDisengage(0);
                    dsMcu.slotPistonDisengage(1);
                    dsMcu.slotPistonDisengage(2);
                }
            }

            GenericButton {
                id: stopAll
                color: colorMap.red
                anchors.top: btnRetractAll.bottom
                anchors.topMargin: rowSpacing
                anchors.horizontalCenter: parent.horizontalCenter
                width: btnWidth
                height: btnHeight

                content: [
                    Text {
                        anchors.fill: parent
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        color: colorMap.blk01
                        font.family: fontIcon.name
                        text: "\ue951"
                        font.pixelSize: btnHeight * 0.4
                    }
                ]
                onBtnPressed: {
                    dsMcu.slotPistonStop(0);
                    dsMcu.slotPistonStop(1);
                    dsMcu.slotPistonStop(2);
                }
            }
        }
    ]

    onSyringeStatesChanged: {
        if (syringeStates === undefined)
        {
            return;
        }

        var isProcessing = false;
        for (var stateIdx = 0; stateIdx < syringeStates.length; stateIdx++)
        {
            if (syringeStates[stateIdx] === "PROCESSING")
            {
                isProcessing = true;
                break;
            }
        }
        setBusyState(isProcessing);
    }

    function setBusyState(isBusy)
    {
        btnExtendAll.enabled = !isBusy;
        btnRetractAll.enabled = !isBusy;
    }
}
