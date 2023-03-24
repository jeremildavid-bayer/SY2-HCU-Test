import QtQuick 2.12
import "DeviceManagerWidgets"
import "DeviceManagerPanel"
import "../Widgets"

DeviceManagerBase {
    property int leftFrameWidth: (dsCfgLocal.screenW * 0.42)
    property int rightFrameWidth: dsCfgLocal.screenW - leftFrameWidth
    property int listViewItemHeight: parent.height * 0.1
    property string lastDmScreenState: "Home"
    property string activePanelName: "MUDS"

    signal signalStartBarcodeReader()

    // device manager doesn't show bottom bar which makes the input pads to have empty area at the bottom which appears white.
    // adding a full screen background only for device manager page to avoid this. It would be waste to draw this for other pages
    Rectangle {
        id: deviceManagerBackground
        width: parent.width
        height: parent.height
        color: colorMap.mainBackground
        visible: deviceManager.visible

        // putting rectangle here and re-parenting so it's self-contained within DeviceManager
        Component.onCompleted: {
            parent = appMainWindow;
            z = appMainWindow.z - 1;
        }
    }

    Rectangle {
        anchors.fill:parent
        color: colorMap.mainBackground

        Item {
            width: parent.width
            height: parent.height * 0.91
            anchors.centerIn: parent

            DeviceManagerContainerMain {
                id: deviceManagerContainer
                anchors.left: parent.left
                anchors.leftMargin: parent.width * 0.11
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: leftFrameWidth
            }

            DeviceManagerPanelContainer {
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.left: deviceManagerContainer.right
                anchors.leftMargin: parent.width * 0.02
                anchors.right: parent.right
                anchors.rightMargin: parent.width * 0.02
            }

            GenericIconButton {
                id: btnBack
                width: parent.width * 0.08
                height: parent.height * 0.15
                anchors.left: parent.left
                anchors.leftMargin: -radius
                anchors.bottom: parent.bottom
                color: colorMap.keypadButton
                iconFontFamily: fontIcon.name
                iconFontPixelSize: height * 0.3
                iconColor: colorMap.text01
                iconText: "\ue909"
                onBtnClicked: {
                    appMain.setScreenState(lastDmScreenState);
                }
            }
        }
    }


    onVisibleChanged: {
        if (visible)
        {
            lastDmScreenState = appMain.screenStatePrev;
        }
    }

    Component.onCompleted: {
        appMain.screenStateChanged.connect(slotScreenStateChanged);
        slotScreenStateChanged();
    }

    Component.onDestruction: {
        appMain.screenStateChanged.disconnect(slotScreenStateChanged);
    }

    function setActivePanel(panelName)
    {
        activePanelName = panelName;
    }

    function slotScreenStateChanged()
    {
        visible = (appMain.screenState.indexOf("DeviceManager") >= 0);
    }

    function barcodeReaderStart()
    {
        signalStartBarcodeReader();
    }
}
