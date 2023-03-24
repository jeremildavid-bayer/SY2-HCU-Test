import QtQuick 2.12
import "../../../Widgets"
import "../../../Util.js" as Util

Drawer {
    id: root
    handleRectWidth: 0
    interactive: false

    property var egfrItem
    property int rowHeight: height * 0.12

    property var newParent

    property var curEditParamInfo: { "State": "IDLE" } // State: IDLE, EDITING_EGFR_PARAM

    signal signalResumeEditEgfrParam();

    Component.onCompleted: {
        if (newParent !== undefined)
        {
            parent = newParent;
        }
    }

    onEgfrItemChanged:
    {
        // If Edit was in progress, handle last edit.
        var lastEditParamInfo = Util.copyObject(curEditParamInfo);
        var editResumeRequired = false;
        if (widgetInputPad.isOpen())
        {
            if (curEditParamInfo.State === "EDITING_EGFR_PARAM")
            {
                // Advance Parameter is current edited by HCU, don't close the inputPad
                widgetInputPad.emitSignalClosed(false);
                editResumeRequired = true;
            }
        }

        // Update UI
        egfrPanelList.model = egfrItem.Value.Parameters;
        egfrNoticesList.model = egfrItem.Value.Notices;

        // Resume edit if required
        if (editResumeRequired)
        {
            logDebug("EGFR Draw: Resume editing: lastEditParamInfo=" + JSON.stringify(lastEditParamInfo));
            setCurEditParamInfo(lastEditParamInfo);
            signalResumeEditEgfrParam();
        }
    }

    function getCurEditParamInfo()
    {
        return curEditParamInfo;
    }

    function setCurEditParamInfo(newCurEditParamInfo)
    {
        curEditParamInfo = newCurEditParamInfo;
    }

    content: [
        Item {
            id: rectDrawer
            width: parent.width
            height: parent.height
            anchors.fill: parent
            anchors.leftMargin: frameMargin / 2

            Item {
                id: egfrPanelRect
                // ensure changing the layout doesn't screw up input pad area
                width: parent.width - widgetInputPad.numPadW - (verticalSeparatorLine.width)
                height: parent.height
                anchors.left: parent.left

                Text {
                    id: egfrTitleText
                    anchors.top: parent.top
                    anchors.right: parent.right
                    anchors.left: parent.left
                    height: parent.height * 0.1
                    verticalAlignment: Text.AlignVCenter
                    wrapMode: Text.Wrap
                    font.pixelSize: height * 0.6
                    font.family: fontRobotoMedium.name
                    text: translate("T_eGFRCalculatorTitle")
                    color: colorMap.text01
                    fontSizeMode: Text.Fit
                }

                EgfrList {
                    id: egfrPanelList
                    anchors.top: egfrTitleText.bottom
                    anchors.right: parent.right
                    anchors.left: parent.left
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: parent.height * 0.02
                    clip: true
                    interactive: true
                }
            }

            Rectangle {
                id: verticalSeparatorLine
                anchors.left: egfrPanelRect.right
                width: frameMargin / 6
                height: parent.height
                color: "gray"
            }

            Item
            {
                width: parent.width - egfrPanelRect.width - verticalSeparatorLine.width
                height: parent.height
                anchors.right: parent.right

                Item {
                    id: rectEgfrNoticesList
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    width: parent.width
                    height: parent.height - okButton.height - (okButton.anchors.bottomMargin *2 )

                    ListView {
                        ScrollBar {
                            color: "gray"
                        }
                        ListFade {
                            fadeColor: colorMap.subPanelBackground
                        }

                        id: egfrNoticesList
                        anchors.fill: parent
                        anchors.topMargin: root.height * 0.02
                        anchors.leftMargin: anchors.topMargin
                        anchors.rightMargin: anchors.topMargin
                        width: parent.width

                        clip: true

                        delegate: Item {
                            width: ListView.view.width
                            height: textNoticeIcon.y + textNotice.height + (frameMargin * 0.2)
                            Text {
                                id: textNoticeIcon
                                width: parent.width * 0.086
                                height: width
                                color: colorMap.text01
                                font.family: fontIcon.name
                                font.pixelSize: height * 0.92
                                text: "\ue978"
                            }
                            Text {
                                id: textNotice
                                anchors.left: textNoticeIcon.right
                                anchors.leftMargin: parent.width * 0.03
                                anchors.right: parent.right
                                height: contentHeight
                                color: colorMap.text01
                                font.pixelSize: root.height * 0.042
                                font.family: fontRobotoMedium.name
                                wrapMode: Text.Wrap
                                text: translate("T_PersonalizationNoticeType_" + egfrItem.Value.Notices[index])
                            }
                        }
                    }
                }

                GenericIconButton {
                    id: okButton
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: frameMargin / 2
                    anchors.right: parent.right
                    anchors.rightMargin: frameMargin / 2
                    visible: true
                    height: parent.height * 0.13
                    width: parent.width * 0.4
                    radius: buttonRadius
                    color: colorMap.actionButtonBackground
                    iconColor: colorMap.actionButtonText
                    iconFontFamily: fontRobotoBold.name
                    iconText: translate("T_OK")
                    pressedSoundCallback: function() { soundPlayer.playPressKey(); }
                    onBtnClicked: {
                        close();
                    }
                }
            }
        }
    ]

    onStateChanged: {
        if (state === "OPEN")
        {
            listViewAdvanceParamsList.interactive = false;

            // Disable NavNext button
            navigationBar.disableNavNextButtonCount++;
        }
        else
        {
            if (navigationBar.disableNavNextButtonCount > 0)
            {
                // re-enable NavNext button
                navigationBar.disableNavNextButtonCount--;
            }
        }
    }

    onSignalClosed: {
        listViewAdvanceParamsList.interactive = true;
    }
}
