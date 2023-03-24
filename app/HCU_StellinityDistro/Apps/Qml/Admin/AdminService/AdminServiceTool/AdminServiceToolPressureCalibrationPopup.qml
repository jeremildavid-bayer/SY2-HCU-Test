import QtQuick 2.12
import QtQuick.Controls 2.12
import "../../../Widgets"
import "../../../Widgets/Popup"

Popup {
    property var pressureCalibrationStatus: dsMcu.pressureCalibrationStatus
    property int selectedSyringeIdx: (cmbSyringeSelect === undefined) ? -1 : cmbSyringeSelect.currentIndex
    property int expectedTotalDurationSecs: 205
    property int progressPercent: 0
    property var calibrationStartedAtMs
    property var syringeVols: dsMcu.syringeVols
    property double syringeVol: (selectedSyringeIdx == -1) ? 0 : syringeVols[selectedSyringeIdx]

    widthMin: dsCfgLocal.screenW * 0.75
    heightMin: dsCfgLocal.screenH * 0.8
    titleText: "Pressure Calibration"
    btnWidth: dsCfgLocal.screenW * 0.16
    okBtnText: "Save Calibration"
    cancelBtnText: "Abort"
    otherBtnText: "Close"
    showOkBtn: false
    showCancelBtn: true
    showOtherBtn: false
    translationRequired: false

    content: [
        Item {
            anchors.fill: parent

            GenericComboBox {
                id: cmbSyringeSelect
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.topMargin: parent.height * 0.06
                rowHeight: parent.height * 0.15

                width: parent.width * 0.3

                optionList: [ { unit: "", icon: "\ue930", value: "Saline", iconColor: colorMap.saline },
                              { unit: "", icon: "\ue92f", value: "Contrast1", iconColor: colorMap.contrast1 },
                              { unit: "", icon: "\ue92f", value: "Contrast2", iconColor: colorMap.contrast2 }]
            }

            Rectangle {
                property string statusText: ""
                property bool isErrorState: false

                id: statusBox
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: progressBar.top
                anchors.bottomMargin: parent.height * 0.1
                width: parent.width * 0.8
                height: parent.height * 0.5
                radius: 10
                border.width: 3
                border.color: isErrorState ? "red" : "grey"
                color: isErrorState ? "orange" : "lightgrey"

                Text {
                    id: textStatus
                    width: parent.width * 0.7
                    height: parent.height * 0.6
                    text: statusBox.statusText
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                    wrapMode: Text.Wrap
                    font.pixelSize: parent.height * 0.11
                    font.family: fontRobotoLight.name
                }

                Text {
                    anchors.top: textStatus.bottom
                    anchors.bottom: parent.bottom
                    width: parent.width * 0.7
                    anchors.horizontalCenter: parent.horizontalCenter

                    text: {
                        if (pressureCalibrationStatus === undefined)
                        {
                            return "";
                        }

                        var params = pressureCalibrationStatus.StagesParams[pressureCalibrationStatus.InjectStageIdx];

                        if (params === undefined)
                        {
                            return "";
                        }

                        var dataCheckInfo = pressureCalibrationStatus.DataCheckInfo;
                        var homePos = (pressureCalibrationStatus.HomePosition === -1) ? "--" : (pressureCalibrationStatus.HomePosition.toFixed(1) + "ml");
                        var engagedPos = (pressureCalibrationStatus.EngagedPosition === -1) ? "--" : (pressureCalibrationStatus.EngagedPosition.toFixed(1) + "ml");
                        var firstGoodSampleAt = (pressureCalibrationStatus.FirstGoodAdcValuePosition === -1) ? "--" : (pressureCalibrationStatus.FirstGoodAdcValuePosition.toFixed(1) + "ml");

                        return "FlowRate: " + params.FlowRate + "ml/s" + ",  SampleInterval: "  + params.DataCaptureIntervalMs + "ms\n" +
                               "CurVol: " + syringeVol.toFixed(1) + "ml,  Home: " + homePos + ",  " + "Engaged: " + engagedPos + "\n" +
                               "FirstGoodSampleAt: " + firstGoodSampleAt + "\n" +
                               "ADC Reading: " + pressureCalibrationStatus.AdcReadValue.toFixed(0) + " (MovingMean=" + dataCheckInfo.LastMovingAverage.toFixed(0) + ")" + "\n";
                    }

                    verticalAlignment: Text.AlignBottom
                    wrapMode: Text.Wrap
                    font.pixelSize: parent.height * 0.08
                    font.family: fontRobotoLight.name
                    color: "grey"
                }
            }

            Rectangle {
                id: progressBar
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: parent.height * 0.01
                height: parent.height * 0.08
                width: parent.width * 0.8
                border.color: "grey"
                border.width: 2
                radius: height * 0.1
                clip: true

                Rectangle {
                    opacity: 0.6
                    color: "lightblue"
                    x: parent.border.width
                    y: parent.border.width
                    width: ((parent.width * progressPercent) / 100) - (parent.border.width * 2)
                    height: parent.height - (parent.border.width * 2)
                }

                Text {
                    anchors.fill: parent
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: parent.height * 0.5
                    font.family: fontRobotoLight.name
                    text: {
                        var remainingTimeSec = Math.floor((expectedTotalDurationSecs * (100 - progressPercent)) / 100);
                        var progressText = progressPercent.toString() + "%";

                        if (tmrProgressUpdate.running)
                        {
                            progressText += "   " + remainingTimeSec + "s Remaining";
                        }
                        return progressText;
                    }
                }
            }
        }
    ]

    Timer {
        id: tmrProgressUpdate
        interval: 1000
        repeat: true
        onTriggered: {
            var elapsedTimeSec = (new Date() - calibrationStartedAtMs) / 1000;
            progressPercent = Math.min(99, (elapsedTimeSec * 100) / expectedTotalDurationSecs);
        }
    }

    onOpened: {
        resetPage();
    }

    onBtnOkClicked: {
        if (okBtnText == "Start")
        {
            cmbSyringeSelect.readOnly = true;
            dsMcu.slotCalibratePressureStart(selectedSyringeIdx);
            calibrationStartedAtMs = new Date();
            tmrProgressUpdate.start();
            showOkBtn = false;
            showOtherBtn = false;
            showCancelBtn = true;
        }
        else if (okBtnText == "Save Calibration")
        {
            dsMcu.slotCalibrateSetPressure(selectedSyringeIdx);
            resetPage();
            close();
        }
    }

    onBtnCancelClicked: {
        dsMcu.slotCalibratePressureStop();
    }

    onBtnOtherClicked: {
        resetPage();
        close();
    }

    onPressureCalibrationStatusChanged: {
        if (!visible)
        {
            return;
        }

        //logDebug("pressureCalibrationStatus = " + JSON.stringify(pressureCalibrationStatus));
        if ( (pressureCalibrationStatus === undefined) ||
             (pressureCalibrationStatus.State === undefined) )
        {
            return;
        }

        var state = pressureCalibrationStatus.State;

        if (state === "PRESSURE_CAL_STATE_STARTED")
        {
            statusBox.statusText = "Repositioning Piston...";
            statusBox.isErrorState = false;
        }
        else if (state === "PRESSURE_CAL_STATE_DISENGAGE_STARTED")
        {
            statusBox.statusText = "Stage" + (pressureCalibrationStatus.InjectStageIdx + 1) + ": Disengaging...";
            statusBox.isErrorState = false;
        }
        else if (state === "PRESSURE_CAL_STATE_FIND_PLUNGER_STARTED")
        {
            statusBox.statusText = "Stage" + (pressureCalibrationStatus.InjectStageIdx + 1) + ": Engaging...";
            statusBox.isErrorState = false;
        }
        else if (state === "PRESSURE_CAL_STATE_INJECT_STAGE_STARTED")
        {
            statusBox.statusText = "Stage" + (pressureCalibrationStatus.InjectStageIdx + 1) + ": Capturing Data...";
            statusBox.isErrorState = false;
        }
        else if (state === "PRESSURE_CAL_STATE_DONE")
        {
            progressPercent = 100;
            tmrProgressUpdate.stop();
            okBtnText = "Save Calibration";
            showOkBtn = true;
            statusBox.statusText = "Data Capture Complete.\nPress '" + okBtnText + "' to complete."
        }
        else if (state === "PRESSURE_CAL_STATE_FAILED")
        {
            statusBox.statusText = Qt.binding(function() { return pressureCalibrationStatus.Err; });
            statusBox.isErrorState = true;
            tmrProgressUpdate.stop();
            showOtherBtn = true;
            showCancelBtn = false;
        }
    }

    function resetPage()
    {
        okBtnText = "Start";
        showOkBtn = true;
        showCancelBtn = false;
        showOtherBtn = true;

        tmrProgressUpdate.stop();
        progressPercent = 0;
        cmbSyringeSelect.setCurrentIndex(0);
        cmbSyringeSelect.readOnly = false;

        statusBox.statusText = "Ready To Start";
        statusBox.isErrorState = false;
    }
}
