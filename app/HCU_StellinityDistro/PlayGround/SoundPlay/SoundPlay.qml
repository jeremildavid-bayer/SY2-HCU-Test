import QtQuick 2.5
import QtMultimedia 5.5

Rectangle {
    Audio {
        id: playSound1
        source: "qrc:/Resources/PlungerMove.ogg"
        loops: Audio.Infinite
        playbackRate: 0.1
        volume: 0.6
    }
    Audio {
        id: playSound2
        source: "qrc:/Resources/PlungerMove.ogg"
        loops: Audio.Infinite
        playbackRate: 0.2
        volume: 0.7
    }
    Audio {
        id: playSound3
        source: "qrc:/Resources/PlungerMove.ogg"
        loops: Audio.Infinite
        playbackRate: 0.3
        volume: 0.8
    }
    Audio {
        id: playSound4
        source: "qrc:/Resources/PlungerMove.ogg"
        loops: Audio.Infinite
        playbackRate: 0.4
        volume: 0.9
    }
    Audio {
        id: playSound5
        source: "qrc:/Resources/PlungerMove.ogg"
        loops: Audio.Infinite
        playbackRate: 0.5
        volume: 1.0
    }

    ListView {
        id: listView
        anchors.fill: parent
        model: ListModel {
            ListElement { name: "1ml/s"; soundPlayRate: 0.5; soundVolume: 0.4; }
            ListElement { name: "2ml/s"; soundPlayRate: 0.5; soundVolume: 0.5; }
            ListElement { name: "3ml/s"; soundPlayRate: 0.6; soundVolume: 0.6; }
            ListElement { name: "4ml/s"; soundPlayRate: 0.6; soundVolume: 0.7; }
            ListElement { name: "5ml/s"; soundPlayRate: 0.7; soundVolume: 0.7; }
            ListElement { name: "6ml/s"; soundPlayRate: 0.7; soundVolume: 0.8; }
            ListElement { name: "7ml/s"; soundPlayRate: 0.8; soundVolume: 0.8; }
            ListElement { name: "8ml/s"; soundPlayRate: 0.8; soundVolume: 0.9; }
            ListElement { name: "9ml/s"; soundPlayRate: 0.9; soundVolume: 1; }
            ListElement { name: "10ml/s"; soundPlayRate: 1.0; soundVolume: 1; }
        }

        delegate: Rectangle {
            border.width: 1
            border.color: "gray"
            height: 40
            width: ListView.view.width
            Text {
                anchors.centerIn: parent
                text: name
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    /*playSound.stop();
                    playSound.playbackRate = soundPlayRate;
                    playSound.volume = soundVolume;
                    playSound.play();*/
                    playSound1.stop();
                    playSound2.stop();
                    playSound3.stop();
                    playSound4.stop();
                    playSound5.stop();

                    if (index == 0) playSound1.play();
                    if (index == 1) playSound1.play();
                    if (index == 2) playSound2.play();
                    if (index == 3) playSound2.play();
                    if (index == 4) playSound3.play();
                    if (index == 5) playSound3.play();
                    if (index == 6) playSound4.play();
                    if (index == 7) playSound4.play();
                    if (index == 8) playSound5.play();
                    if (index == 9) playSound5.play();

                }
            }
        }
    }
}
