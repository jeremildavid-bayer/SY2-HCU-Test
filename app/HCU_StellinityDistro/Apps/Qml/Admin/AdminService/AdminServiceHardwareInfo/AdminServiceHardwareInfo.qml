import QtQuick 2.12
import "../../../Widgets"
import "../../../Widgets/ConfigTable"

Item {
    property int frameMargin: dsCfgLocal.screenW * 0.02
    property int leftFrameWidth: (dsCfgLocal.screenW * 0.21) + (frameMargin * 2)
    property int listViewItemHeight: parent.height * 0.12
    property int cellMargin: width * 0.015

    anchors.fill: parent
    visible: false

    ListView {
        id: listView
        width: leftFrameWidth
        height: parent.height
        clip: true

        ScrollBar {}
        ListFade {}

        model: ListModel {
            ListElement { title: "General" }
            ListElement { title: "Boards" }
        }

        delegate: GenericButton {
            width: ListView.view.width
            height: listViewItemHeight
            radius: 0
            color: (listView.currentIndex === index) ? colorMap.text02 : "transparent"

            Text {
                text: title
                anchors.fill: parent
                anchors.leftMargin: cellMargin
                anchors.rightMargin: cellMargin
                font.family: (listView.currentIndex === index) ? fontRobotoBold.name : fontRobotoLight.name
                font.pixelSize: height * 0.4
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
                color: (listView.currentIndex === index) ? colorMap.white01 : colorMap.text01
                wrapMode: Text.Wrap
            }

            onBtnClicked: {
                selectPage(index);
            }

            Component.onCompleted: {
                listView.dragStarted.connect(reset);
            }

            Component.onDestruction: {
                listView.dragStarted.disconnect(reset);
            }
        }
    }

    Rectangle {
        // separator line
        id: rectSeparator
        anchors.left: listView.right
        width: frameMargin / 6
        height: parent.height
        color: colorMap.titleBarBackground
    }

    Item {
        id: mainRect
        anchors.left: rectSeparator.right
        anchors.leftMargin: frameMargin
        width: parent.width - x - frameMargin
        height: parent.height

        ConfigTable {
            allConfigList: dsHardwareInfo.configTable
            translationRequired: false
            configKeyName: "General_"
            visibleScreenState: "Admin-Service-HardwareInfo-General"
            onSignalConfigChanged: (configItem) => {
                dsHardwareInfo.slotConfigChanged(configItem);
            }
        }

        AdminServiceHardwareInfoBoardList {}
    }

    Component.onCompleted: {
        appMain.screenStateChanged.connect(slotScreenStateChanged);
        slotScreenStateChanged();
    }

    Component.onDestruction: {
        appMain.screenStateChanged.disconnect(slotScreenStateChanged);
    }

    function slotScreenStateChanged()
    {
        var prevVisible = (appMain.screenStatePrev.indexOf("Admin-Service-HardwareInfo") >= 0);
        visible = (appMain.screenState.indexOf("Admin-Service-HardwareInfo") >= 0);
        if ( (visible) && (!prevVisible))
        {
            selectPage(0);
        }
        reload();
    }

    function reload()
    {
        if (!visible)
        {
            widgetInputPad.close();
            return;
        }
    }

    function selectPage(pageIdx)
    {
        listView.currentIndex = pageIdx;

        if (listView.currentIndex == 0)
        {
            appMain.setScreenState("Admin-Service-HardwareInfo-General");
        }
        else if (listView.currentIndex == 1)
        {
            appMain.setScreenState("Admin-Service-HardwareInfo-Boards");
        }
    }
}
