import QtQuick 2.12
import QtQuick.Controls 2.12
import "../../../Widgets"

Item {
    property var ledColorMap: null
    property var curSelectedLed: null
    property int btnWidth: width * 0.078
    property int btnHeight: btnWidth
    property int ledRed;
    property int ledGreen;
    property int ledBlue;

    signal signalReloadLedControls()
    signal signalTurnOffLed()

    id: root
    anchors.fill: parent

    Row {
        y: parent.height * 0.01
        spacing: parent.width * 0.005

        Repeater {
            model: ListModel {
                ListElement { title: "Saline"; }
                ListElement { title: "Contrast1"; }
                ListElement { title: "Contrast2"; }
                ListElement { title: "Door1"; }
                ListElement { title: "Door2"; }
                ListElement { title: "Door3"; }
                ListElement { title: "Door4"; }
                ListElement { title: "Suds1"; }
                ListElement { title: "Suds2"; }
                ListElement { title: "Suds3"; }
                ListElement { title: "Suds4"; }
                ListElement { title: "AirDoor"; }
            }

            delegate: GenericIconButton {
                border.width: height * 0.04
                width: btnWidth
                height: btnHeight
                iconText: title
                iconFontPixelSize: height * 0.18
                iconStyle: Text.Outline

                onBtnClicked: {
                    var ledIdx = curSelectedLed.indexOf(title);
                    if (ledIdx >= 0)
                    {
                        curSelectedLed.splice(ledIdx, 1);
                    }
                    else
                    {
                        curSelectedLed.push(title);
                    }
                    updateLedColorMap();
                    root.signalReloadLedControls();

                    dsMcu.slotLedControl(ledColorMap);
                }

                Component.onCompleted: {
                    root.signalReloadLedControls.connect(reload);
                }

                Component.onDestruction: {
                    root.signalReloadLedControls.disconnect(reload);
                }

                function reload()
                {
                    //logDebug(title + ": color changed=" + ledColorMap[title]);
                    border.color = curSelectedLed.indexOf(title) >= 0 ? colorMap.actionButtonBackground : ledColorMap[title];
                    iconColor = curSelectedLed.indexOf(title) >= 0 ? colorMap.actionButtonBackground : "white"
                    color = ledColorMap[title];
                }
            }
        }
    }

    Rectangle {
        id: rectControl
        y: parent.height * 0.3
        x: parent.width * 0.05
        width: parent.width * 0.9
        height: parent.height * 0.68
        color: colorMap.navBtnSelectedBorder
        radius: parent.height * 0.02

        Column {
            y: parent.height * 0.06
            x: parent.width * 0.02
            spacing: parent.width * 0.02

            Repeater {
                model: ListModel {
                    ListElement { title: "R"; baseColor: "red"; }
                    ListElement { title: "G"; baseColor: "green"; }
                    ListElement { title: "B"; baseColor: "blue"; }
                }

                delegate: Rectangle {
                    width: rectControl.width
                    height: rectControl.height * 0.18
                    color: "transparent"

                    Slider {
                        property string fillColor: {
                            if (title === "R")
                            {
                                return Qt.rgba(slider.value / 255.0, 0, 0, 1);
                            }
                            else if (title === "G")
                            {
                                return Qt.rgba(0, slider.value / 255.0, 0, 1);
                            }
                            else if (title === "B")
                            {
                                return Qt.rgba(0, 0, slider.value / 255.0, 1);
                            }
                            return "transparent";
                        }

                        id: slider
                        x: parent.width * 0.1
                        width: parent.width * 0.7
                        height: parent.height
                        to: 255
                        stepSize: 1

                        onValueChanged: {
                            if (title === "R")
                            {
                                ledRed = slider.value;
                            }
                            else if (title === "G")
                            {
                                ledGreen = slider.value;
                            }
                            else if (title === "B")
                            {
                                ledBlue = slider.value;
                            }

                            updateLedColorMap();
                            root.signalReloadLedControls();

                            dsMcu.slotLedControl(ledColorMap);
                        }

                        background: Rectangle {
                            color: "transparent"

                            Rectangle {
                                width: slider.visualPosition * parent.width
                                height: slider.height * 0.2
                                y: slider.height/2 - height/2
                                radius: 0.2 * height
                                color: baseColor
                            }
                            Rectangle {
                                x: slider.visualPosition * parent.width
                                width:(1 - slider.visualPosition) * parent.width
                                height: slider.height * 0.2
                                y: slider.height/2 - height/2
                                radius: 0.2 * height
                                border.width: 1
                                border.color: "#888"
                                gradient: Gradient {
                                    GradientStop { color: "#bbb" ; position: 0 }
                                    GradientStop { color: "#ccc" ; position: 0.6 }
                                    GradientStop { color: "#ccc" ; position: 1 }
                                }
                            }
                        }

                        handle: Rectangle {
                            id: hanleRect
                            x: -(width / 2) + (slider.visualPosition * slider.availableWidth)
                            y: slider.topPadding + slider.availableHeight / 2 - height / 2
                            color: "transparent"
                            border.color: slider.pressed ? baseColor : "darkgray"
                            border.width: slider.height * 0.05
                            implicitWidth: slider.height
                            implicitHeight: slider.height
                            radius: slider.height * 0.5

                            Rectangle {//hailine vertical line indicating the position where the color is selected
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.horizontalCenter: parent.horizontalCenter
                                width: 0.1 * hanleRect.height
                                height: 0.4 * hanleRect.height
                                color: baseColor
                                }
                        }

                        Component.onCompleted: {
                            root.signalTurnOffLed.connect(slotTurnOffLed);
                        }

                        Component.onDestruction: {
                            root.signalTurnOffLed.disconnect(slotTurnOffLed);
                        }

                        function slotTurnOffLed()
                        {
                            slider.value = 0;
                        }
                    }

                    Text {
                        x: parent.width * 0.88
                        height: parent.height
                        text: title + " " + slider.value.toFixed(0)
                        color: baseColor
                        font.pixelSize: height * 0.35
                        verticalAlignment: Text.AlignVCenter

                    }
                }
            }
        }

        GenericIconButton {
            anchors.horizontalCenter: parent.horizontalCenter
            y: parent.height * 0.75
            width: parent.width * 0.2
            height: parent.height * 0.15
            iconText: "Turn Off"
            color: colorMap.actionButtonBackground;
            iconColor: colorMap.actionButtonText;
            onBtnClicked: {
                root.signalTurnOffLed();
                dsMcu.slotLedControl(ledColorMap);
            }
        }
    }


    Component.onCompleted: {
        ledColorMap = {};
        ledColorMap["Saline"] = "#000000";
        ledColorMap["Contrast1"] = "#000000";
        ledColorMap["Contrast2"] = "#000000";
        ledColorMap["Door1"] = "#000000";
        ledColorMap["Door2"] = "#000000";
        ledColorMap["Door3"] = "#000000";
        ledColorMap["Door4"] = "#000000";
        ledColorMap["Suds1"] = "#000000";
        ledColorMap["Suds2"] = "#000000";
        ledColorMap["Suds3"] = "#000000";
        ledColorMap["Suds4"] = "#000000";
        ledColorMap["AirDoor"] = "#000000";


        curSelectedLed = [];

        appMain.screenStateChanged.connect(slotScreenStateChanged);
        slotScreenStateChanged();
    }

    Component.onDestruction: {
        appMain.screenStateChanged.disconnect(slotScreenStateChanged);
    }

    function updateLedColorMap()
    {
        for (var i = 0; i < curSelectedLed.length; i++)
        {
            var ledCtrl = curSelectedLed[i];
            //var newcolor = Qt.rgba(ledRed / 255.0, ledGreen / 255.0, ledBlue / 255.0, 1);
            var hexR = Number(ledRed).toString(16);
            if (hexR.length == 1)
            {
                hexR = "0" + hexR;
            }
            var hexG = Number(ledGreen).toString(16);
            if (hexG.length == 1)
            {
                hexG = "0" + hexG;
            }
            var hexB = Number(ledBlue).toString(16);
            if (hexB.length == 1)
            {
                hexB = "0" + hexB;
            }
            var newColor = "#" + hexR + hexG + hexB;
            ledColorMap[ledCtrl] = newColor;
        }
    }

    function slotScreenStateChanged()
    {
        visible = (appMain.screenState === "Admin-Service-Tool-LedControl");
        reload();
    }

    function reload()
    {
        if (!visible)
        {
            return;
        }

        signalReloadLedControls();
    }
}

