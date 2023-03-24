import QtQuick 2.12
import "../../Widgets"
import "../../Util.js" as Util

ListView {
    property var examAdvanceInfo: dsExam.examAdvanceInfo
    property var linkedAccessions: ( (examAdvanceInfo !== undefined) && (examAdvanceInfo.LinkedAccessions !== undefined) ) ? examAdvanceInfo.LinkedAccessions : []
    property int animationMs: 200
    property double textHMargin: parent.width * 0.02

    interactive: false
    clip: true
    width: ListView.view ? ListView.view.width * 0.94 : 0
    anchors.horizontalCenter: parent.horizontalCenter
    height: {
        if (linkedAccessions.length === 0)
        {
            return 0;
        }
        else if (state == "COMPRESSED")
        {
            return rowHeight
        }
        else
        {
            return (linkedAccessions.length + 1) * rowHeight;
        }
    }

    Behavior on height {
        PropertyAnimation {
            duration: animationMs
        }
    }

    header: GenericButton {
        id: root
        radius: 0
        width: parent.width
        height: rowHeight;
        interactive: (cruLinkStatus.State === "Active")

        Text {
            anchors.right: stateIconFrame.right
            anchors.left: parent.left
            height: parent.height
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: height * 0.35
            font.family: fontRobotoLight.name
            text: translate("T_LinkOtherAccession")
            color: root.interactive ? colorMap.text01 : colorMap.text02
        }

        Item {
            id: stateIconFrame
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            height: parent.height
            width: stateIcon.width
            rotation: isCompressed()  ? 180 : 0
            Behavior on rotation {
                PropertyAnimation {
                    duration: animationMs
                }
            }

            Text {
                id: stateIcon
                width: contentWidth
                height: parent.height
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                color: root.interactive ? colorMap.text01 : colorMap.text02
                font.pixelSize: parent.height * 0.3
                font.family: fontIcon.name
                text: "\ue906"
                font.bold: true
            }
        }

        Rectangle {
            id: separatorLine
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            width: parent.width
            height: 1
            color: colorMap.text02
        }

        onBtnClicked: {
            if (isCompressed())
            {
                setExpanded();
            }
            else
            {
                setCompressed();
            }
        }

        Component.onCompleted: {
            listViewAdvanceParamsList.dragStarted.connect(reset);
        }

        Component.onDestruction: {
            listViewAdvanceParamsList.dragStarted.disconnect(reset);
        }
    }

    state: "COMPRESSED"
    states: [
        State { name: "COMPRESSED" },
        State { name: "EXPANDED" }
    ]

    model: linkedAccessions
    delegate: ExamPatientSelectEditAdvanceParamLinkOtherAccessionListItem {}

    function setCompressed()
    {
        state = "COMPRESSED";
    }

    function setExpanded()
    {
        state = "EXPANDED";
    }

    function isCompressed()
    {
        return (state === "COMPRESSED");
    }
}
