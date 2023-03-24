import QtQuick 2.12
import "../../../Widgets"

Item {
    property int groupIndex: index
    property var groupData: planTemplateGroups[groupIndex]
    property var planDigests: groupData.PlanDigests
    property var cruLinkStatus: dsCru.cruLinkStatus
    property bool expanded: false
    property bool hideSpotlight: ( (groupData.Name === "T_SpotlightProtocols") && (cruLinkStatus.State !== "Active") )
    property int listSpacing: hideSpotlight ? 0 : 1
    property int bottomBorderWidth: hideSpotlight ? 0 : listSpacing
    property int itemMargin: hideSpotlight ? 0 : parent.width * 0.08
    property int rowHeight: hideSpotlight ? 0 : ListView.view.height * 0.1
    property string borderColor: colorMap.grid
    property int animationMs: 250

    id: root
    state: "COMPRESSED"

    width: ListView.view.width
    height: getRowHeight()

    Rectangle {
        id: borderTop
        x: groupNameRow.x
        width: groupNameRow.width
        height: bottomBorderWidth
        color: borderColor
    }

    GenericButton {
        id: groupNameRow
        height: rowHeight
        x: parent.width * 0.015
        width: parent.width - (x * 2)
        color: "transparent"

        content: [
            Text {
                height: parent.height
                width: parent.width * 0.9
                verticalAlignment: Text.AlignVCenter
                text: (groupData.Name === "T_SpotlightProtocols") ? translate(groupData.Name) : groupData.Name
                color: colorMap.text01
                font.family: (groupIndex == selectedGroupIdx) ? fontRobotoBold.name : fontRobotoLight.name
                font.pixelSize: height * 0.45
                font.bold: (groupIndex == selectedGroupIdx)
                elide: Text.ElideRight
                wrapMode: Text.Wrap
            },

            Text {
                id: spotlightIcon
                anchors.right: stateIconFrame.left
                anchors.rightMargin: - height * 0.1
                anchors.verticalCenter: parent.verticalCenter
                color: borderColor
                font.pixelSize: parent.height * 0.45
                font.family: fontIcon.name
                text: expanded ? "\ue989" : "\ue98A"
                visible: groupData.Name === "T_SpotlightProtocols"
            },

            Rectangle {
                id: stateIconFrame
                anchors.right: parent.right
                anchors.rightMargin: 0
                anchors.verticalCenter: parent.verticalCenter
                height: hideSpotlight ? 0 : rowHeight * 0.8
                width: height
                color: "transparent"

                Text {
                    id: stateIcon
                    anchors.fill: parent
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    color: borderColor
                    font.pixelSize: parent.height * 0.45
                    font.family: fontIcon.name
                    text: "\ue906"
                }
            }
        ]

        Rectangle {
            id: borderBottom
            y: groupNameRow.height - bottomBorderWidth
            x: groupNameRow.x
            width: groupNameRow.width
            height: bottomBorderWidth
            color: borderColor
        }


        onBtnClicked: {
            root.state = (root.state === "COMPRESSED") ? "EXPANDED" : "COMPRESSED";
        }

        Component.onCompleted: {
            listViewPlanGroup.dragStarted.connect(reset);
        }

        Component.onDestruction: {
            listViewPlanGroup.dragStarted.disconnect(reset);
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
                    NumberAnimation { target: root; properties: 'height'; to: rowHeight + getContentHeight(); duration: animationMs }
                    NumberAnimation { target: listViewPlan; properties: 'height'; to: getContentHeight(); duration: animationMs }
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
                    NumberAnimation { target: root; properties: 'height'; to: rowHeight; duration: animationMs }
                    NumberAnimation { target: listViewPlan; properties: 'height'; to: 0; duration: animationMs }
                }

                ScriptAction { script: {
                        expanded = false;
                    }
                }
            }
        }
    ]

    ListView {
        id: listViewPlan
        clip: true
        cacheBuffer: rowHeight * 10
        height: 0
        width: parent.width
        y: hideSpotlight ? 0 : rowHeight
        interactive: false
        model: planDigests
        delegate: ExamProtocolSelectListItem {}
    }

    Component.onCompleted: {
        protocolSelectList.signalReloadRows.connect(reload);
        reload();
    }

    Component.onDestruction: {
        protocolSelectList.signalReloadRows.disconnect(reload);
    }

    function getRowHeight()
    {
        if (hideSpotlight)
        {
            return 0;
        }
        else
        {
            return expanded ? rowHeight + getContentHeight() : rowHeight;
        }
    }

    function getContentHeight()
    {
        if (hideSpotlight)
        {
            return 0;
        }

        var itemCount = 0;
        for (var itemIdx = 0; itemIdx < groupData.PlanDigests.length; itemIdx++)
        {
            var plan = groupData.PlanDigests[itemIdx].Plan;
            if (plan !== undefined)
            {
                itemCount++;
            }
        }

        return itemCount * rowHeight;
    }

    function reload()
    {
        var isSpotlightGroup = (groupData.Name === "T_SpotlightProtocols");

        if (isSpotlightGroup && hideSpotlight)
        {
            root.state = "COMPRESSED";
        }
        else if (selectedGroupIdx == groupIndex)
        {
            //The spotlight folder may be retained as "Expanded" without being expanded. This is a hack to prevent this.
            if (isSpotlightGroup)
            {
                root.state = "COMPRESSED";
            }

            root.state = "EXPANDED";
        }
        else
        {
            root.state = "COMPRESSED";
        }
    }
}

