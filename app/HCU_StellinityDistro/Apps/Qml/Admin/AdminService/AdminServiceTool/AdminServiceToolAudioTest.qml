import QtQuick 2.12
import "../../../Widgets"

AdminServiceToolTestTemplate {
    property int soundVolume: (dsCfgLocal.soundVolumeMin + dsCfgLocal.soundVolumeMax) / 2
    property int soundVolumeMin: dsCfgLocal.soundVolumeMin
    property int soundVolumeMax: dsCfgLocal.soundVolumeMax
    property double audioLevelNormal
    property double audioLevelNotification
    property double audioLevelInjection
    property double audioLevelSudsPrimed
    property bool audioKeyClicksEnabled

    activeScreenState: "Admin-Service-Tool-AudioTest"
    consoleVisible: false

    GenericSlider {
        id: sliderCtrl
        x: parent.width * 0.52
        width: parent.width * 0.42
        height: parent.height * 0.2
        Component.onCompleted: {
            var sliderCtrlLabels = [];
            for (var label = soundVolumeMin; label <= soundVolumeMax; label++)
            {
                if (label == 0)
                {
                    sliderCtrlLabels.push("\ue981");
                }
                else
                {
                    sliderCtrlLabels.push(label);
                }
            }
            setLabels(sliderCtrlLabels);
            setValue(soundVolume);
        }
        onSignalValueChanged: (newValue) => {
            // audio volume is ranged from 0 to 1
            var audioVolume = newValue / soundVolumeMax;
            dsCfgLocal.audioLevelNormal = audioVolume;
            dsCfgLocal.audioLevelInjection = audioVolume;
            dsCfgLocal.audioLevelSudsPrimed = audioVolume;
            dsCfgLocal.audioLevelNotification = audioVolume;
        }
    }

    GenericIconButton {
        anchors.top: sliderCtrl.bottom
        anchors.topMargin: parent.height * 0.05
        x: sliderCtrl.x
        width: parent.width * 0.18
        height: parent.height * 0.13
        iconText: "Stop"
        iconColor: colorMap.white01
        color: colorMap.gry01
        onBtnClicked: {
            soundPlayer.stop();
        }
    }

    paramsTable: [
        AdminServiceToolTestTableRow {
            titleText: "PRESS GOOD"
            valueText: "Play"
            onBtnClicked: {
                soundPlayer.playPressGood();
            }
        },
        AdminServiceToolTestTableRow {
            titleText: "PRESS ALL STOP"
            valueText: "Play"
            onBtnClicked: {
                soundPlayer.playPressAllStop();
            }
        },
        AdminServiceToolTestTableRow {
            titleText: "NEXT"
            valueText: "Play"
            onBtnClicked: {
                soundPlayer.playNext();
            }
        },
        AdminServiceToolTestTableRow {
            titleText: "ARMED"
            valueText: "Play"
            onBtnClicked: {
                soundPlayer.playArmed();
            }
        },
        AdminServiceToolTestTableRow {
            titleText: "START"
            valueText: "Play"
            onBtnClicked: {
                soundPlayer.playStart();
            }
        },
        AdminServiceToolTestTableRow {
            titleText: "DISARM/STOP"
            valueText: "Play"
            onBtnClicked: {
                soundPlayer.playDisarmStop();
            }
        },
        AdminServiceToolTestTableRow {
            titleText: "INJECT_PULSE"
            valueText: "Play"
            onBtnClicked: {
                soundPlayer.playInjPulse();
            }
        },
        AdminServiceToolTestTableRow {
            titleText: "REMINDER_IMMINENT"
            valueText: "Play"
            onBtnClicked: {
                soundPlayer.playReminderImminent();
            }
        },
        AdminServiceToolTestTableRow {
            titleText: "REMINDER_EXPIRED"
            valueText: "Play"
            onBtnClicked: {
                soundPlayer.playReminderExpired();
            }
        },
        AdminServiceToolTestTableRow {
            titleText: "INJ_COMPLETE"
            valueText: "Play"
            onBtnClicked: {
                soundPlayer.playInjComplete();
            }
        },
        AdminServiceToolTestTableRow {
            titleText: "MODAL_EXPECTED"
            valueText: "Play"
            onBtnClicked: {
                soundPlayer.playModalExpected();
            }
        },
        AdminServiceToolTestTableRow {
            titleText: "MODAL_UNEXPECTED"
            valueText: "Play"
            onBtnClicked: {
                soundPlayer.playModalUnexpected();
            }
        },
        AdminServiceToolTestTableRow {
            titleText: "ERROR"
            valueText: "Play"
            onBtnClicked: {
                soundPlayer.playError();
            }
        },
        AdminServiceToolTestTableRow {
            titleText: "T01 300Hz 0dB LR"
            valueText: "Play"
            onBtnClicked: {
                soundPlayer.playTestSample1();
            }
        },
        AdminServiceToolTestTableRow {
            titleText: "T02 1kHz 0dB LR"
            valueText: "Play"
            onBtnClicked: {
                soundPlayer.playTestSample2();
            }
        },
        AdminServiceToolTestTableRow {
            titleText: "T03 3.4kHz 0dB LR"
            valueText: "Play"
            onBtnClicked: {
                soundPlayer.playTestSample3();
            }
        }
    ]

    function reload()
    {
        var prevVisible = (appMain.screenStatePrev === "Admin-Service-Tool-AudioTest");

        if ( (visible) && (!prevVisible) )
        {
            // Save volume for later
            audioLevelNormal = dsCfgLocal.audioLevelNormal;
            audioLevelNotification = dsCfgLocal.audioLevelNotification;
            audioLevelInjection = dsCfgLocal.audioLevelInjection;
            audioLevelSudsPrimed = dsCfgLocal.audioLevelSudsPrimed;
            audioKeyClicksEnabled = dsCfgLocal.audioKeyClicksEnabled;
            dsCfgLocal.audioKeyClicksEnabled = false;
        }
        else if ( (!visible) && (prevVisible) )
        {
            // Restore volumes
            dsCfgLocal.audioLevelNormal = audioLevelNormal;
            dsCfgLocal.audioLevelNotification = audioLevelNotification;
            dsCfgLocal.audioLevelInjection = audioLevelInjection;
            dsCfgLocal.audioLevelSudsPrimed = audioLevelSudsPrimed;
            dsCfgLocal.audioKeyClicksEnabled = audioKeyClicksEnabled;
        }
    }
}
