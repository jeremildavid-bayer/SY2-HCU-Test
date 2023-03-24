import QtQuick 2.12
import QtQuick.Controls 2.12
import "../../../Widgets"

Item {
    property string activeScreenState: ""
    property int paramsTableRowWidth: width * 0.43
    property int paramsTableRowHeight: height * 0.098
    property double titleWidthPercent: 0.7
    property alias paramsTable: paramsTable.children
    property alias consoleText: consoleText
    property bool consoleVisible: true
    property alias btnStart: btnStart

    signal testStarted()
    signal testStopped()

    id: root
    anchors.top: parent.top
    anchors.topMargin: parent.height * 0.04
    anchors.bottom: parent.bottom
    width: parent.width

    visible: false

    Rectangle {
        id: rectParamsTable
        x: parent.width * 0.02
        width: paramsTableRowWidth
        height: parent.height * 0.98
        color: "transparent"
        clip: true

        ScrollBar {
            flickable: flickTable
        }

        ListFade {
            flickable: flickTable
        }

        Flickable {
            id: flickTable
            width: parent.width
            height: parent.height
            contentHeight: paramsTable.height + flickTableFooter.height
            contentWidth: paramsTable.width
            flickableDirection: Flickable.VerticalFlick
            enabled: btnStart.iconText == "START"

            Column {
                id: paramsTable
            }

            Item {
                id: flickTableFooter
                anchors.top: paramsTable.bottom
                width: parent.width
                height: paramsTableRowHeight * 2
            }
        }
    }

    Rectangle {
        color: colorMap.consoleBackground
        anchors.left: rectParamsTable.right
        anchors.leftMargin: parent.width * 0.02
        anchors.right: parent.right
        anchors.rightMargin: parent.width * 0.02
        height: parent.height * 0.85
        clip: true
        visible: consoleVisible

        Flickable {
            id: flickTextArea
            width: parent.width
            height: parent.height
            flickableDirection: Flickable.VerticalFlick

            TextArea.flickable: TextArea {
                id: consoleText
                readOnly: true
                color: colorMap.text02
                font.pixelSize: flickTextArea.height * 0.03
                wrapMode: Text.Wrap
            }
        }

        ScrollBar {
            flickable: flickTextArea
            autoHide: false
        }
    }

    GenericIconButton {
        id: btnStart
        visible: consoleVisible
        x: parent.width * 0.46
        y: parent.height * 0.87
        width: parent.width * 0.15
        height: parent.height * 0.1
        color: colorMap.actionButtonBackground
        iconText: "START"
        iconColor: colorMap.actionButtonText
        iconFontPixelSize: height * 0.4
        onBtnClicked: {
            if (btnStart.iconText == "START")
            {
                consoleText.append("");
                trimVisibleLog();

                testStarted();
                btnStart.iconText = "STOP";
            }
            else if (btnStart.iconText == "STOP")
            {
                testStopped();
                btnStart.iconText = "START";
            }
        }
    }

    GenericIconButton {
        visible: consoleVisible
        x: parent.width * 0.82
        y: parent.height * 0.87
        width: parent.width * 0.15
        height: parent.height * 0.1
        color: colorMap.actionButtonBackground
        iconText: "CLEAR"
        iconColor: colorMap.actionButtonText
        iconFontPixelSize: height * 0.4
        onBtnClicked: {
            consoleText.cursorPosition = 0;
            consoleText.text = "";
        }
    }

    Component.onCompleted: {
        appMain.screenStateChanged.connect(slotScreenStateChanged);
        slotScreenStateChanged();
    }

    Component.onDestruction: {
        appMain.screenStateChanged.disconnect(slotScreenStateChanged);
    }

    function isTestStarted()
    {
        return (btnStart.iconText === "STOP");
    }

    function stopTest()
    {
        btnStart.iconText = "START";
        consoleText.append("\n");
        trimVisibleLog();
    }

    function slotScreenStateChanged()
    {
        visible = (appMain.screenState === activeScreenState);
        reload();
    }

    function reload()
    {
        if (!visible)
        {
            return;
        }
    }

    function trimVisibleLog()
    {
        if (consoleText.lineCount > 2000)
        {
            consoleText.remove(0, 100);
        }
    }
}
