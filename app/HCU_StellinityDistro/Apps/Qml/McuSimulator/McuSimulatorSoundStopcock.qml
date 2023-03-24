import QtQuick 2.12
import QtMultimedia 5.5

Item {
    property bool mcuSimulatorEnabled: dsMcuSim.simulatorEnabled
    property double audioLevelNormal: dsCfgLocal.audioLevelNormal
    property string pathSound: dsCapabilities.pathSound

    Repeater {
        model: 3
        delegate: Item {
            property var stopcockPositions: dsMcu.stopcockPositions
            property string curPosition: "UNKNOWN"

            Audio {
                id: audioSimStopcockMove
                source: pathSound + "/Sim-StopcockMove.wav"
                loops: 1
                playbackRate: 1.5
                volume: 0.4 * audioLevelNormal
            }

            onStopcockPositionsChanged: {
                if (stopcockPositions.length != 3)
                {
                    return;
                }

                var newPosition = stopcockPositions[index];

                if (newPosition !== curPosition)
                {
                    audioSimStopcockMove.stop();

                    curPosition = newPosition;

                    if (!mcuSimulatorEnabled)
                    {
                        // only play for simulator mode
                    }
                    else if (curPosition === "MOVING")
                    {
                        audioSimStopcockMove.play();
                    }
                }
            }
        }
    }

}
