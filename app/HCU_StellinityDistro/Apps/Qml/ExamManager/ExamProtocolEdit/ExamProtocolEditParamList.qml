import QtQuick 2.12
import "../../Widgets"

Rectangle {
    property int rowHeight: injectionPlanEdit.height * 0.1
    property var paramItems: []
    property int animationMs: 200
    property int heightExpanded: listView.height
    property int heightCompressed: (listView.height === 0) ? 0 : listView.headerItem.height
    property var activeAlerts: dsAlert.activeAlerts
    property bool isActive: false
    property string name: ""
    property bool isPlanLevel: true

    id: root
    color: "transparent"
    radius: 15
    border.width: 3
    border.color: colorMap.keypadButton
    clip: true

    state: "UNKNOWN"

    states: [
        State { name: "COMPRESSED" },
        State { name: "EXPANDED" }
    ]

    transitions: [
        Transition {
            to: "EXPANDED"
            SequentialAnimation {
                ParallelAnimation {
                    NumberAnimation { target: stateIconFrame; properties: 'rotation'; to: 0; duration: animationMs }
                    NumberAnimation { target: root; properties: 'height'; to: heightExpanded; duration: animationMs }
                }

                ScriptAction { script: {
                        root.height = Qt.binding(function() { return heightExpanded; });
                    }
                }
            }
        },

        Transition {
            to: "COMPRESSED"
            SequentialAnimation {
                ParallelAnimation {
                    NumberAnimation { target: stateIconFrame; properties: 'rotation'; to: 180; duration: animationMs }
                    NumberAnimation { target: root; properties: 'height'; to: heightCompressed; duration: animationMs }
                }

                ScriptAction { script: {
                        root.height = Qt.binding(function() { return heightCompressed; });
                    }
                }
            }
        }
    ]

    ListView {
        id: listView
        clip: true
        interactive: false
        width: parent.width
        height: {
            if ( (paramItems == undefined) ||
                 (paramItems.length == 0) )
            {
                return 0;
            }

            for (var alertIdx = 0; alertIdx < activeAlerts.length; alertIdx++)
            {
                if (activeAlerts[alertIdx].CodeName === "DeparameterizedProtocol")
                {
                    return 0;
                }
            }

            return ((rowHeight * model.length) + headerItem.height + footerItem.height);
        }

        onHeightChanged: {
            if ( (height == 0) &&
                 (root.height > 0) )
            {
                // listView is hidden now, hide root too
                root.height = 0;
            }
        }

        header: GenericButton {
            id: rectHeader
            width: ListView.view ? ListView.view.width : 0
            height: rowHeight

            content: Rectangle {
                anchors.fill: parent
                radius: root.radius
                color: colorMap.keypadButton

                Text {
                    anchors.fill: parent
                    anchors.leftMargin: parent.width * 0.04
                    anchors.right: parent.right
                    anchors.rightMargin: parent.width * 0.04
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: height * 0.42
                    font.family: fontRobotoBold.name
                    text: isPlanLevel ? translate("T_ProtocolParameters") : translate("T_InjectionParameters")
                    color: colorMap.text01
                    wrapMode: Text.Wrap
                }

                Rectangle {
                    visible: (root.state == "EXPANDED")
                    width: parent.width
                    height: parent.radius
                    anchors.bottom: parent.bottom
                    color: parent.color
                }
            }
            onBtnClicked: {
                if (root.state === "COMPRESSED")
                {
                    root.state = "EXPANDED";
                }
                else
                {
                    root.state = "COMPRESSED";
                }
            }

            Component.onCompleted: {
                listViewStep.dragStarted.connect(reset);
            }

            Component.onDestruction: {
                listViewStep.dragStarted.disconnect(reset);
            }
        }
        footer: Item {
            width: parent.width
            height: rowHeight * 0.02
        }

        delegate: ExamProtocolEditParamItem {}

        onModelChanged: {
            timerSingleShot(1, function() {
                if (isActive)
                {
                    setState("EXPANDED");
                }
            });
        }
    }


    Rectangle {
        id: stateIconFrame
        anchors.right: parent.right
        anchors.rightMargin: parent.width * 0.02
        anchors.top: parent.top
        height: listView.headerItem ? listView.headerItem.height : 0
        width: height
        color: "transparent"

        Text {
            id: stateIcon
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            color: colorMap.text01
            font.pixelSize: parent.height * 0.45
            font.family: fontIcon.name
            text: "\ue907"
        }
    }

    onParamItemsChanged: {
        if (JSON.stringify(listView.model) === JSON.stringify(paramItems))
        {
            return;
        }
        //logDebug("ExamProtocolEditParamList[" + name + "]: ParamItemsChanged:\n" + JSON.stringify(listView.model) + "\n->\n" + JSON.stringify(paramItems));
        listView.model = paramItems;
    }

    onIsActiveChanged: {
        handleIsActive();
    }

    function setState(newState)
    {
        root.state = "UNKNOWN";
        root.state = newState;
    }

    function handleIsActive()
    {
        timerSingleShot(1, function() {
            //logDebug("ExamProtocolEditParamList: name=" + name + ": isActive=" + isActive);
            if (isActive)
            {
                setState("EXPANDED");
            }
            else
            {
                setState("COMPRESSED");
            }
        });
    }
}
