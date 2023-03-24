import QtQuick 2.12
import QtMultimedia 5.5

Item {
    property bool mcuSimulatorEnabled: dsMcuSim.simulatorEnabled
    property double audioLevelNormal: dsCfgLocal.audioLevelNormal
    property string pathSound: dsCapabilities.pathSound

    Repeater {
        model: 3
        delegate: Item {
            property var syringeFlows: dsMcu.syringeFlows
            property int curAudioPlayIdx: -1

            Audio {
                id: audioSimPlungerMove1
                source: pathSound + "/Sim-PlungerMove.ogg"
                loops: Audio.Infinite
                playbackRate: 0.1
                volume: 0.3 * audioLevelNormal
            }
            Audio {
                id: audioSimPlungerMove2
                source: pathSound + "/Sim-PlungerMove.ogg"
                loops: Audio.Infinite
                playbackRate: 0.2
                volume: 0.4 * audioLevelNormal
            }
            Audio {
                id: audioSimPlungerMove3
                source: pathSound + "/Sim-PlungerMove.ogg"
                loops: Audio.Infinite
                playbackRate: 0.3
                volume: 0.5 * audioLevelNormal
            }
            Audio {
                id: audioSimPlungerMove4
                source: pathSound + "/Sim-PlungerMove.ogg"
                loops: Audio.Infinite
                playbackRate: 0.4
                volume: 0.6 * audioLevelNormal
            }

            onSyringeFlowsChanged: {
                if (syringeFlows.length != 3)
                {
                    return;
                }

                var flow = Math.abs(syringeFlows[index].toFixed(0));
                var newAudioPlayIdx = curAudioPlayIdx;

                if (!mcuSimulatorEnabled)
                {
                    // only play for simulator mode
                    newAudioPlayIdx = -1;
                }
                else if ( (flow > 0) && (flow <= 2) )
                {
                    newAudioPlayIdx = 0;
                }
                else if ( (flow > 2) && (flow <= 5) )
                {
                    newAudioPlayIdx = 1;
                }
                else if ( (flow > 5) && (flow <= 8) )
                {
                    newAudioPlayIdx = 2;
                }
                else if (flow > 8)
                {
                    newAudioPlayIdx = 3;
                }
                else
                {
                    newAudioPlayIdx = -1;
                }

                if (newAudioPlayIdx != curAudioPlayIdx)
                {
                    //logDebug("Plunger" + index + ": Playing index=" + newAudioPlayIdx);

                    audioSimPlungerMove1.stop();
                    audioSimPlungerMove2.stop();
                    audioSimPlungerMove3.stop();
                    audioSimPlungerMove4.stop();

                    curAudioPlayIdx = newAudioPlayIdx;

                    if (curAudioPlayIdx == 0)
                    {
                        audioSimPlungerMove1.play();
                    }
                    else if (curAudioPlayIdx == 1)
                    {
                        audioSimPlungerMove2.play();
                    }
                    else if (curAudioPlayIdx == 2)
                    {
                        audioSimPlungerMove3.play();
                    }
                    else if (curAudioPlayIdx == 3)
                    {
                        audioSimPlungerMove4.play();
                    }
                }
            }
        }
    }
}
