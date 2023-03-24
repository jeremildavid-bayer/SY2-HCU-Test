import QtQuick 2.12
import QtQuick.Controls 2.12

import "../../../Widgets"

Item {
    property int btnWidth: width * 0.15
    property int btnHeight: height * 0.15

    id: root
    anchors.fill: parent

    Item {
        id: itemDevShell
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: consoleItem.left

        height: parent.height * 0.3

        GenericButton
        {
            id: commandTextButton
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: frameMargin/2
            height: 50

            radius: 0
            interactive: true
            color: "white"

            Text {
                id: commandText
                anchors.fill: parent

                verticalAlignment: Text.AlignBottom
                horizontalAlignment: Text.AlignLeft
                wrapMode: Text.Wrap
                font.pixelSize: height * 0.8
                font.family: fontRobotoLight.name
                text: "ifconfig -a"

                fontSizeMode: Text.Fit
            }

            onBtnClicked:
            {
                widgetKeyboard.open(commandText.parent, commandText, 0, 255);
                widgetKeyboard.signalValueChanged.connect(slotKeyboardValChanged);
                widgetKeyboard.signalClosed.connect(slotKeyboardClosed);
            }

            function slotKeyboardValChanged(newValue)
            {
                commandText.text = newValue;
            }

            function slotKeyboardClosed(modified)
            {
                widgetKeyboard.signalValueChanged.disconnect(slotKeyboardValChanged);
            }
        }

        GenericIconButton {
            id: commandSendButton
            anchors.top: commandTextButton.bottom
            anchors.left: parent.left
            anchors.margins: frameMargin/2

            width: root.btnWidth
            height: root.btnHeight

            color: colorMap.actionButtonBackground
            iconText: "Send to Shell"
            iconColor: colorMap.actionButtonText
            iconFontPixelSize: height * 0.2
            onBtnClicked: {
                callShellCommand(commandText.text);
            }
        }
    }

    Item {
        id: itemEasyNetwork
        anchors.top: itemDevShell.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: consoleItem.left

        property string shellOutputText: dsDevTools.shellOutput
        property var networkList: []
        property bool update: false

        GenericIconButton {
            id: networkRefreshButton

            anchors.top: parent.top
            anchors.left: parent.left
            anchors.margins: frameMargin/2
            width: root.btnWidth
            height: root.btnHeight

            color: colorMap.actionButtonBackground
            iconText: ((itemEasyNetwork.networkList.length > 0) ? "Re-scan Network Interfaces" : "Scan Network Interfaces")
            iconColor: colorMap.actionButtonText
            iconFontPixelSize: height * 0.2

            onBtnClicked: {
                itemEasyNetwork.scanNetworkInterfaces();
            }
        }

        GenericIconButton {
            id: quickifconfigBtn

            anchors.top: parent.top
            anchors.left: networkRefreshButton.right
            anchors.margins: frameMargin/2
            width: root.btnWidth
            height: root.btnHeight

            color: colorMap.actionButtonBackground
            iconText: "ifconfig -a"
            iconColor: colorMap.actionButtonText
            iconFontPixelSize: height * 0.2

            onBtnClicked: {
                callShellCommand("ifconfig -a");
            }
        }

        function scanNetworkInterfaces() {
            update = true;
            callShellCommand("ls /sys/class/net");
        }

        onShellOutputTextChanged:
        {
            if (shellOutputText != "" && update) {
                var fullList = shellOutputText.split("\n");
                var filteredList = [];
                for (var a in fullList) {
                    if (fullList[a] === "lo") continue;
                    if (fullList[a] === "wlp1s0") continue;

                    var entry = {};
                    entry["name"] = fullList[a];
                    entry["note"] = "";

                    if (fullList[a] === dsCapabilities.ethernetInterface)
                    {
                        entry["note"] = "reserved for CRU connection";
                    }

                    filteredList.push(entry);
                }

                networkList = filteredList;

                logDebug(shellOutputText);

                update = false;
            }
        }

        Item {
            id: itemNetworkList
            anchors.top: networkRefreshButton.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: frameMargin/2

            height: parent.height * 0.6

            property var networkList: itemEasyNetwork.networkList

            Column {
                property var networkList: parent.networkList
                Repeater {
                    model: parent.networkList

                    delegate: Rectangle {
                        width: itemNetworkList.width
                        height: itemNetworkList.height * 0.2
                        color: "transparent"
                        border.width: 1
                        border.color: colorMap.text02
                        property var paramData: parent.networkList[index]

                        Row {
                            spacing: 10
                            anchors.fill: parent
                            Text {
                                width: parent.width * 0.4
                                height: parent.height
                                text:  (paramData.name + ((paramData.note.length > 0) ? (" (" + paramData.note + ")") : ""))

                                verticalAlignment: Text.AlignVCenter
                                horizontalAlignment: Text.AlignLeft
                                color: colorMap.text01
                                font.pixelSize: height * 0.4
                                font.family: fontRobotoLight.name
                                fontSizeMode: Text.Fit
                            }

                            GenericIconButton {
                                anchors.verticalCenter: parent.verticalCenter
                                width: parent.width * 0.15
                                height: parent.height * 0.8
                                iconText: "up"
                                radius: 2

                                color: colorMap.actionButtonBackground
                                iconColor: colorMap.actionButtonText
                                iconFontPixelSize: height * 0.8

                                onBtnClicked: {
                                    callShellCommand(["sudo","ifconfig",paramData.name,"up"].join(" "));
                                }
                            }

                            GenericIconButton {
                                anchors.verticalCenter: parent.verticalCenter
                                width: parent.width * 0.15
                                height: parent.height * 0.8
                                iconText: "down"
                                radius: 2

                                color: colorMap.actionButtonBackground
                                iconColor: colorMap.actionButtonText
                                iconFontPixelSize: height * 0.8

                                onBtnClicked: {
                                    callShellCommand(["sudo","ifconfig",paramData.name,"down"].join(" "));
                                }
                            }

                            GenericIconButton {
                                anchors.verticalCenter: parent.verticalCenter
                                width: parent.width * 0.2
                                height: parent.height * 0.8
                                iconText: "dhclient"
                                radius: 2

                                color: colorMap.actionButtonBackground
                                iconColor: colorMap.actionButtonText
                                iconFontPixelSize: height * 0.8

                                onBtnClicked: {
                                    callShellCommand(["sudo","dhclient",paramData.name].join(" "));
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    Rectangle {
        id: consoleItem
        anchors.right: parent.right
        anchors.rightMargin: parent.width * 0.02
        height: parent.height * 0.85
        width: parent.width * 0.4
        color: colorMap.consoleBackground
        clip: true
        visible: true
        property string shellOutputText: dsDevTools.shellOutput
        onShellOutputTextChanged:
        {
            var temp = (consoleTextArea.text.length > 0)?"\n":"";
            updateConsoleTextArea(temp + shellOutputText);
            updateConsoleTextArea("\n##### Shell Command End\n");
        }

        Flickable {
            id: flickTextArea
            width: parent.width
            height: parent.height
            flickableDirection: Flickable.VerticalFlick

            TextArea.flickable: TextArea {
                id: consoleTextArea
                readOnly: true
                color: colorMap.text02
                font.pixelSize: flickTextArea.height * 0.03
                wrapMode: Text.Wrap

                onTextChanged:
                {
                    cursorPosition = text.length;
                }
            }
        }

        ScrollBar {
            flickable: flickTextArea
            autoHide: false
        }
    }

    GenericIconButton {
        anchors.bottom: parent.bottom
        anchors.bottomMargin: height * 0.1
        anchors.right: parent.right
        anchors.rightMargin: height * 0.1
        width: parent.width * 0.15
        height: parent.height * 0.1
        color: colorMap.actionButtonBackground
        iconText: "CLEAR"
        iconColor: colorMap.actionButtonText
        iconFontPixelSize: height * 0.4
        onBtnClicked: {
            consoleTextArea.text = "";
        }
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
        visible = (appMain.screenState === "Admin-Service-DeveloperTools");
    }

    function updateConsoleTextArea(text)
    {
        consoleTextArea.text += text;
    }

    function callShellCommand(shellCommand)
    {
        updateConsoleTextArea("\n##### Shell Command : \"" + shellCommand + "\"\n");

        var fullCommandList = shellCommand.split(" ");
        var command = fullCommandList.shift();
        var argsList = fullCommandList.join(" ");
        dsDevTools.slotShellCommand(command, argsList);
    }
}
