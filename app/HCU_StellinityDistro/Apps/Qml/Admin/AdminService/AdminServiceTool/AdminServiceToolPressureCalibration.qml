import QtQuick 2.12
import QtQuick.Controls 2.12
import "../../../Widgets"
import "../../../Widgets/Popup"

Item {
    property int btnWidth: width * 0.3
    property int btnHeight: height * 0.2
    property bool mudsLatched: dsMcu.mudsLatched

    id: root
    anchors.fill: parent
    visible: false

    Column {
        y: parent.height * 0.03
        x: parent.width * 0.02
        spacing: parent.width * 0.02

        Row {
            spacing: root.width * 0.02
            Repeater {
                model: ListModel {
                    ListElement { title: "Start\nPressure\nCalibration" }
                }

                delegate: GenericIconButton {
                    width: btnWidth
                    height: btnHeight
                    iconText: title
                    iconFontPixelSize: height * 0.2
                    iconColor: colorMap.white01
                    color: colorMap.gry01
                    enabled: mudsLatched
                    onBtnClicked: {
                        if (index === 0)
                        {
                            popupPressureCalibration.open();
                        }
                    }
                }
            }
        }
    }


    Rectangle {
        color: colorMap.consoleBackground
        y: parent.height * 0.03
        x: parent.width * 0.45
        width: parent.width * 0.52
        height: parent.height * 0.8
        clip: true

        Text {
            anchors.fill: parent
            anchors.margins: parent.width * 0.01
            font.pixelSize: parent.height * 0.04
            color: colorMap.text01
            verticalAlignment: Text.AlignLeft
            wrapMode: Text.Wrap

            text: "* Pressure Calibration Setup:\n\n\n" +
                  "1. Connect the Pressure Calibration Tool to the FL-CAL port.\n\n" +
                  "2. Insert the fixture into the MUDS slot.\n\n" +
                  "3. Latch down.\n\n";
        }
    }

    AdminServiceToolPressureCalibrationPopup {
        id: popupPressureCalibration
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
        visible = (appMain.screenState === "Admin-Service-Tool-PressureCalibration");
    }
}

