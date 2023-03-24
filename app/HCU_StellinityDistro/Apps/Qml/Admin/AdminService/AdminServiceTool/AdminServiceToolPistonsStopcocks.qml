import QtQuick 2.12
import "../../../Widgets"

Rectangle {
    property double pistonFlow1: 5.0
    property double pistonFlow2: 5.0
    property double pistonFlow3: 5.0

    id: root
    anchors.fill: parent
    color: "transparent"

    Text {
        x: parent.width * 0.02
        text: "Pistons"
        color: colorMap.text01
        font.bold: true
        font.underline: true
        font.pixelSize: parent.height * 0.05
    }

    Repeater {
        model: 3
        delegate: Rectangle {
            property int btnWidth: width * 0.48
            property int btnHeight: height * 0.3
            property string btnColor: getBtnColor(index)

            x: getPosX(index)
            y: parent.height * 0.10
            width: parent.width * 0.25
            height: parent.height * 0.5
            color: "transparent"

            Row {
                spacing: parent.width * 0.1
                Column {
                    spacing: parent.height * 0.04

                    GenericIconButton {
                        width: btnWidth
                        height: btnHeight
                        iconText: "Engage"
                        iconFontPixelSize: height * 0.22
                        color: btnColor
                        onBtnClicked: {
                            dsMcu.slotPistonEngage(index);
                        }
                    }

                    GenericIconButton {
                        width: btnWidth
                        height: btnHeight
                        iconText: "Disengage"
                        iconFontPixelSize: height * 0.22
                        color: btnColor
                        onBtnClicked: {
                            dsMcu.slotPistonDisengage(index);
                        }
                    }

                    GenericIconButton {
                        width: btnWidth
                        height: btnHeight
                        color: "red"
                        iconText: "STOP"
                        iconFontPixelSize: height * 0.3
                        iconColor: "white"
                        onBtnClicked: {
                            dsMcu.slotPistonStop(index);
                        }
                    }
                }

                Column {
                    spacing: parent.height * 0.04

                    GenericIconButton {
                        id: btnPistonUp
                        width: btnWidth
                        height: btnHeight
                        color: "transparent"
                        iconText: "\ue907"
                        iconFontPixelSize: height * 0.8
                        iconColor: btnColor

                        MultiPointTouchArea {
                            anchors.fill: parent

                            onPressed: {
                                btnPistonUp.pressedSoundCallback();
                                dsMcu.slotPistonUp(index, getPistonFlow(index));
                            }
                            onReleased: {
                                dsMcu.slotPistonStop(index);
                            }
                        }
                    }

                    GenericIconButton {
                        id: btnPistonFlow
                        width: btnWidth
                        height: btnHeight
                        iconText: getPistonFlow(index).toFixed(1).toString() + "ml/s"
                        iconFontPixelSize: height * 0.22
                        color: btnColor
                        onBtnClicked: {
                            var newFlow = getPistonFlow(index) + 0.5;
                            if (newFlow > dsCapabilities.flowRateMax)
                            {
                                // Using 1ml/s for ease of use
                                newFlow = 1;
                            }
                            setPistonFlow(index, newFlow);
                        }
                    }

                    GenericIconButton {
                        id: btnPistonDown
                        width: btnWidth
                        height: btnHeight
                        color: "transparent"
                        iconText: "\ue906"
                        iconFontPixelSize: height * 0.8
                        iconColor: btnColor

                        MultiPointTouchArea {
                            anchors.fill: parent

                            onPressed: {
                                dsMcu.slotPistonDown(index, getPistonFlow(index));
                            }
                            onReleased: {
                                dsMcu.slotPistonStop(index);
                            }
                        }
                    }
                }
            }
        }
    }

    Text {
        x: parent.width * 0.02
        y: parent.height * 0.68
        text: "Stopcocks"
        color: colorMap.text01
        font.bold: true
        font.underline: true
        font.pixelSize: parent.height * 0.05
    }

    Repeater {
        model: 3
        delegate: Rectangle {
            property int btnWidth: width * 0.32
            property int btnHeight: height
            property string btnColor: getBtnColor(index)

            x: getPosX(index)
            y: parent.height * 0.78
            width: parent.width * 0.25
            height: parent.height * 0.15
            color: "transparent"

            Row {
                spacing: parent.width * 0.04

                GenericIconButton {
                    width: btnWidth
                    height: btnHeight
                    iconText: "Fill"
                    iconFontPixelSize: height * 0.22
                    color: btnColor
                    onBtnClicked: {
                        dsDevice.slotStopcockFillPosition(getSyringeIdxStr(index));
                    }
                }

                GenericIconButton {
                    width: btnWidth
                    height: btnHeight
                    iconText: "Inject"
                    iconFontPixelSize: height * 0.22
                    color: btnColor
                    onBtnClicked: {
                        dsDevice.slotStopcockInjectPosition(getSyringeIdxStr(index));
                    }
                }

                GenericIconButton {
                    width: btnWidth
                    height: btnHeight
                    iconText: "Close"
                    iconFontPixelSize: height * 0.22
                    color: btnColor
                    onBtnClicked: {
                        dsDevice.slotStopcockClosePosition(getSyringeIdxStr(index));
                    }
                }
            }
        }
    }

    function getPosX(index)
    {
        if (index === 0) return parent.width * 0.02;
        if (index === 1) return parent.width * 0.37;
        if (index === 2) return parent.width * 0.72;
        return "transparent";
    }

    function getBtnColor(index)
    {
        if (index === 0) return colorMap.saline;
        if (index === 1) return colorMap.contrast1;
        if (index === 2) return colorMap.contrast2;
        return "transparent";
    }

    function getPistonFlow(index)
    {
        if (index === 0) return pistonFlow1;
        if (index === 1) return pistonFlow2;
        if (index === 2) return pistonFlow3;
        return 0;
    }

    function setPistonFlow(index, newFlow)
    {
        if (index === 0) pistonFlow1 = newFlow;
        if (index === 1) pistonFlow2 = newFlow;
        if (index === 2) pistonFlow3 = newFlow;
    }

    function getSyringeIdxStr(index)
    {
        if (index === 0) return "RS0";
        if (index === 1) return "RC1";
        if (index === 2) return "RC2";
        return "Unknown";
    }

    Component.onCompleted:
    {
        appMain.screenStateChanged.connect(slotScreenStateChanged);
        slotScreenStateChanged();
    }

    Component.onDestruction: {
        appMain.screenStateChanged.disconnect(slotScreenStateChanged);
    }

    function slotScreenStateChanged()
    {
        visible = (appMain.screenState === "Admin-Service-Tool-PistonsStopcocks");
        reload();
    }

    function reload()
    {
        if (!visible)
        {
            return;
        }
    }
}

