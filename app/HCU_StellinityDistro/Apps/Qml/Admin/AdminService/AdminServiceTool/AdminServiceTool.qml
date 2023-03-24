import QtQuick 2.12
import "../../../Widgets"

Item {
    property int frameMargin: dsCfgLocal.screenW * 0.02
    property int rightFrameX: leftFrameWidth
    property int rightFrameWidth: dsCfgLocal.screenW - leftFrameWidth
    property int leftFrameX: 0
    property int leftFrameWidth: (dsCfgLocal.screenW * 0.22) + (frameMargin * 2)
    property int listViewItemHeight: parent.height * 0.11
    property int cellMargin: width * 0.01

    id: root
    anchors.fill: parent
    visible: false

    ListView {
        id: listView
        x: leftFrameX
        y: parent.height * 0.04
        width: leftFrameWidth - frameMargin
        height: parent.height - y
        highlightMoveDuration: 0
        clip: true

        ScrollBar {}
        ListFade {}

        model: ListModel {
            ListElement { title: "Pistons & Stopcocks" }
            ListElement { title: "Special Actions" }
            ListElement { title: "Calibration - General" }
            ListElement { title: "Calibration - Pressure" }
            ListElement { title: "Network Test" }
            ListElement { title: "Barcode Reader Test" }
            ListElement { title: "Piston Cycle Test" }
            ListElement { title: "Stopcock Cycle Test" }
            ListElement { title: "LED Control" }
            ListElement { title: "Touch Screen Test" }
            ListElement { title: "Audio Test" }
            ListElement { title: "BMS" }
            ListElement { title: "BMS Console" }
        }

        footer: Item {
            width: ListView.view ? ListView.view.width : 0
            height: ListView.view ? ListView.view.height * 0.1 : 0
        }

        delegate: GenericButton {
            width: ListView.view.width
            height: listViewItemHeight
            border.color: colorMap.text02
            border.width: 2
            radius: 0
            color: (listView.currentIndex === index) ? colorMap.text02 : "transparent"

            Text {
                text: title
                anchors.fill: parent
                anchors.leftMargin: cellMargin
                anchors.rightMargin: cellMargin
                font.pixelSize: height * 0.35
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

    Item {
        x: rightFrameX
        width: parent.width - x
        height: parent.height

        AdminServiceToolPistonsStopcocks {}
        AdminServiceToolLedControl {}
        AdminServiceToolSpecialActions {}
        AdminServiceToolCalibration {}
        AdminServiceToolPressureCalibration {}
        AdminServiceToolNetworkTest {}
        AdminServiceToolBarcodeReaderTest {}
        AdminServiceToolPistonCycleTest {}
        AdminServiceToolStopcockCycleTest {}
        AdminServiceToolTouchScreenTest {}
        AdminServiceToolAudioTest {}
        AdminServiceToolBMS {
            id: adminBMS
        }
        AdminServiceToolBMSConsole {}
    }

    AdminServiceToolActionBar {
        id: serviceToolActionBar
    }

    onVisibleChanged: {
        dsMcu.slotHwDigestMonitorActive(visible);
    }

    Component.onCompleted:
    {
        serviceToolActionBar.parent = actionBar;
        appMain.screenStateChanged.connect(slotScreenStateChanged);
        slotScreenStateChanged();
    }

    function slotScreenStateChanged()
    {
        var prevVisible = (appMain.screenStatePrev.indexOf("Admin-Service-Tool") >= 0);
        visible = (appMain.screenState.indexOf("Admin-Service-Tool") >= 0);
        if ( (visible) && (!prevVisible))
        {
            selectPage(listView.currentIndex);
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

        if (listView.currentIndex === 0)
        {
            appMain.setScreenState("Admin-Service-Tool-PistonsStopcocks");
        }
        else if (listView.currentIndex === 1)
        {
            appMain.setScreenState("Admin-Service-Tool-SpecialActions");
        }
        else if (listView.currentIndex === 2)
        {
            appMain.setScreenState("Admin-Service-Tool-Calibration");
        }
        else if (listView.currentIndex === 3)
        {
            appMain.setScreenState("Admin-Service-Tool-PressureCalibration");
        }
        else if (listView.currentIndex === 4)
        {
            appMain.setScreenState("Admin-Service-Tool-Network");
        }
        else if (listView.currentIndex === 5)
        {
            appMain.setScreenState("Admin-Service-Tool-BarcodeReader");
        }
        else if (listView.currentIndex === 6)
        {
            appMain.setScreenState("Admin-Service-Tool-PistonCycleTest");
        }
        else if (listView.currentIndex === 7)
        {
            appMain.setScreenState("Admin-Service-Tool-StopcockCycleTest");
        }
        else if (listView.currentIndex === 8)
        {
            appMain.setScreenState("Admin-Service-Tool-LedControl");
        }
        else if (listView.currentIndex === 9)
        {
            appMain.setScreenState("Admin-Service-Tool-TouchScreenTest");
        }
        else if (listView.currentIndex === 10)
        {
            appMain.setScreenState("Admin-Service-Tool-AudioTest");
        }
        else if (listView.currentIndex === 11)
        {
            appMain.setScreenState("Admin-Service-Tool-BMS");
        }
        else if (listView.currentIndex === 12)
        {
            appMain.setScreenState("Admin-Service-Tool-BMSConsole");
        }
    }
}
