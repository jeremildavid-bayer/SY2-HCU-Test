import QtQuick 2.12
import QtQuick.Controls 2.12
import "../../../Widgets"
import "../../../Widgets/Popup"

Item {
    property int btnWidth: width * 0.125
    property int btnHeight: height * 0.15
    property string calibrationResult: dsMcu.calibrationResult
    property bool isMudsPresent: dsMcu.isMudsPresent
    property string calibrationLocation
    property var calibrationInfo: dsHardwareInfo.calibrationInfo

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
                    ListElement { title: "Air Detector\n\nS0" }
                    ListElement { title: "Air Detector\n\nC1" }
                    ListElement { title: "Air Detector\n\nC2" }
                }

                delegate: GenericIconButton {
                    width: btnWidth
                    height: btnHeight
                    iconText: title
                    iconFontPixelSize: height * 0.2
                    iconColor: colorMap.white01
                    color: getColor(index)
                    onBtnClicked: {
                        popUpCalibrationProgress.onCalibrationRequested(index.toString(), "Please disconnect the Spike Adaptor and press 'Continue'");
                    }
                }
            }
        }

        Row {
            spacing: root.width * 0.02
            Repeater {
                model: ListModel {
                    ListElement { title: "Motor\n\nS0" }
                    ListElement { title: "Motor\n\nC1" }
                    ListElement { title: "Motor\n\nC2" }
                }

                delegate: GenericIconButton {
                    width: btnWidth
                    height: btnHeight
                    iconText: title
                    iconFontPixelSize: height * 0.2
                    iconColor: colorMap.white01
                    color: getColor(index)
                    onBtnClicked: {
                        popUpCalibrationProgress.onCalibrationRequested(index.toString(), "Please remove MUDS and press 'Continue'");
                   }
                }
            }
        }

        Row {
            spacing: root.width * 0.02
            Repeater {
                model: ListModel {
                    ListElement { title: "Plunger\n\nS0" }
                    ListElement { title: "Plunger\n\nC1" }
                    ListElement { title: "Plunger\n\nC2" }
                }

                delegate: GenericIconButton {
                    width: btnWidth
                    height: btnHeight
                    iconText: title
                    iconFontPixelSize: height * 0.2
                    iconColor: colorMap.white01
                    color: getColor(index)
                    onBtnClicked: {
                        popUpCalibrationProgress.onCalibrationRequested(index.toString(), "Please place the Plunger Calibration Tool and press 'Continue'");
                    }
                }
            }
        }

        Row {
            spacing: root.width * 0.02
            Repeater {
                model: ListModel {
                    ListElement { title: "Pressure\nPort" }
                    ListElement { title: "SUDS" }
                }

                delegate: GenericIconButton {
                    width: btnWidth
                    height: btnHeight
                    iconText: title
                    iconFontPixelSize: height * 0.2
                    iconColor: colorMap.white01
                    color: colorMap.gry01
                    onBtnClicked: {
                        if (index === 0)
                        {
                            popUpCalibrationProgress.onCalibrationRequested(title, "Please connect the Pressure Meter and press 'Continue'");
                        }
                        else if (index === 1)
                        {
                            popUpCalibrationProgress.onCalibrationRequested(title, "Please disconnect the SUDS and press 'Continue'");
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
        }
    }

    GenericIconButton {
        x: parent.width * 0.46
        y: parent.height * 0.85
        width: parent.width * 0.15
        height: parent.height * 0.1
        color: colorMap.actionButtonBackground
        iconText: "GET REPORT"
        iconColor: colorMap.actionButtonText
        iconFontPixelSize: height * 0.4
        onBtnClicked: {
            consoleText.append("\n\n========= CALIBRATION HISTORY =========\n\n" + JSON.stringify(calibrationInfo, null, " ") + "\n\n");
            trimVisibleLog();
        }
    }

    GenericIconButton {
        x: parent.width * 0.64
        y: parent.height * 0.85
        width: parent.width * 0.15
        height: parent.height * 0.1
        color: colorMap.actionButtonBackground
        iconText: "GET CURRENT"
        iconColor: colorMap.actionButtonText
        iconFontPixelSize: height * 0.4
        onBtnClicked: {
            dsMcu.calibrationResult = "\n\n========= CALIBRATION INFO =========";
            dsMcu.slotGetCalibrationStatus();
        }
    }

    GenericIconButton {
        x: parent.width * 0.82
        y: parent.height * 0.85
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

    PopupMessage {
        id: popUpCalibrationProgress
        type: "PLAIN"
        translationRequired: false
        visible: false
        okBtnText: "Close"
        showCancelBtn: false

        onBtnOkClicked: {
            if (contentText == "Please disconnect the Spike Adaptor and press 'Continue'")
            {
                dsMcu.calibrationResult = "Air Detector " + calibrationLocation + " calibration started";
                onCalibrationStarted();
                dsMcu.slotCalibrateAirDetector(parseInt(calibrationLocation));
            }
            else if (contentText == "Please remove MUDS and press 'Continue'")
            {
                dsMcu.calibrationResult = "Motor " + calibrationLocation + " calibration started";
                onCalibrationStarted();
                dsMcu.slotCalibrateMotor(parseInt(calibrationLocation));
            }
            else if (contentText == "Please place the Plunger Calibration Tool and press 'Continue'")
            {
                dsMcu.calibrationResult = "Plunger " + calibrationLocation + " calibration started";
                onCalibrationStarted();
                dsMcu.slotCalibratePlunger(parseInt(calibrationLocation));
            }
            else if (contentText == "Please connect the Pressure Meter and press 'Continue'")
            {
                dsMcu.calibrationResult = calibrationLocation + " calibration started";
                onCalibrationStarted();
                dsMcu.slotCalibrateSetPressureMeter();
            }
            else if (contentText == "Please disconnect the SUDS and press 'Continue'")
            {
                dsMcu.calibrationResult = calibrationLocation + " calibration started";
                onCalibrationStarted();
                dsMcu.slotCalibrateSuds();
            }
            else if (contentText == "Motor0 Calibration Completed")
            {
                dsMcu.slotPistonDisengage(0);
                close();
            }
            else if (contentText == "Motor1 Calibration Completed")
            {
                dsMcu.slotPistonDisengage(1);
                close();
            }
            else if (contentText == "Motor2 Calibration Completed")
            {
                dsMcu.slotPistonDisengage(2);
                close();
            }
            else
            {
                close();
            }
        }

        onBtnCancelClicked: {
            okBtnText = "Close";
            showOkBtn = false;
            showCancelBtn = false;
            close();
        }

        function onCalibrationRequested(location, instruction)
        {
            popUpCalibrationProgress.type = "PLAIN";
            calibrationLocation = location;
            contentText = instruction;
            showOkBtn = true;
            showCancelBtn = true;
            okBtnText = "Continue";
            cancelBtnText = "Cancel";
            open();
        }

        function onCalibrationStarted()
        {
            popUpCalibrationProgress.type = "PLAIN";
            contentText = "Calibration is in progress..";
            okBtnText = "Close";
            showOkBtn = false;
            showCancelBtn = false;
        }
    }

    function getColor(index)
    {
        if (index === 0)
        {
            return colorMap.saline;
        }
        else if (index === 1)
        {
            return colorMap.contrast1;
        }
        else if (index === 2)
        {
            return colorMap.contrast2;
        }
        return colorMap.gry01;
    }

    function trimVisibleLog()
    {
        if (consoleText.lineCount > 2000)
        {
            consoleText.remove(0, 100);
        }
    }

    onCalibrationResultChanged: {
        consoleText.append(new Date().toTimeString("hh:mm:ss") + " -  " + calibrationResult);
        trimVisibleLog();

        if (calibrationResult.indexOf("ERROR") >= 0)
        {
            // Error occurred
            var contentText = calibrationResult;
            contentText.replace("ERROR: ", "");
            popUpCalibrationProgress.type = "WARNING";
            popUpCalibrationProgress.titleText = "Calibration Failed"
            popUpCalibrationProgress.contentText = contentText;
        }
        else
        {
            // No error
            popUpCalibrationProgress.type = "PLAIN";
            popUpCalibrationProgress.contentText = calibrationResult;
        }
        popUpCalibrationProgress.showOkBtn = true;
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
        visible = (appMain.screenState === "Admin-Service-Tool-Calibration");
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

