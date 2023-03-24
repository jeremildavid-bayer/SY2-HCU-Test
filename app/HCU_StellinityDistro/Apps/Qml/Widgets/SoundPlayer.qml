import QtQuick 2.12
import QtMultimedia 5.5

Item {
    property double audioLevelNormal: dsCfgLocal.audioLevelNormal
    property double audioLevelNotification: dsCfgLocal.audioLevelNotification
    property double audioLevelInjection: dsCfgLocal.audioLevelInjection
    property double audioLevelSudsPrimed: dsCfgLocal.audioLevelSudsPrimed
    property bool audioKeyClicksEnabled: dsCfgLocal.audioKeyClicksEnabled
    property string pathSound: dsCapabilities.pathSound

    Audio {
        id: audioPressGood
        source: pathSound + "/02_PRESS_GOOD.wav"
        volume: audioLevelNormal
    }

    Audio {
        id: audioPressBad
        source: pathSound + "/03_PRESS_BAD.wav"
        volume: audioLevelNotification
    }

    Audio {
        id: audioNext
        source: pathSound + "/04_NEXT.wav"
        volume: audioLevelNormal
    }

    Audio {
        id: audioArmed
        source: pathSound + "/05_ARMED.wav"
        volume: audioLevelInjection
    }

    Audio {
        id: audioStart
        source: pathSound + "/06_START.wav"
        volume: audioLevelInjection
    }

    Audio {
        id: audioDisarmStop
        source: pathSound + "/07_DISARM_STOP.wav"
        volume: audioLevelInjection
    }

    Audio {
        id: audioInjPulse
        source: pathSound + "/08_INJ_PULSE.wav"
        volume: audioLevelNotification
    }

    Audio {
        id: audioReminderImminent
        source: pathSound + "/04_NEXT.wav"
        volume: audioLevelInjection
    }

    Audio {
        id: audioReminderExpired
        source: pathSound + "/09_REMINDER.wav"
        volume: audioLevelInjection
    }

    Audio {
        id: audioInjComplete
        source: pathSound + "/10_INJ_COMPLETE.wav"
        volume: audioLevelInjection
    }

    Audio {
        id: audioModalExpected
        source: pathSound + "/11_MODAL_EXPECTED.wav"
        volume: audioLevelNotification
    }

    Audio {
        id: audioModalUnexpected
        source: pathSound + "/11_MODAL_UNEXPECTED.wav"
        volume: audioLevelNotification
    }

    Audio {
        id: audioError
        source: pathSound + "/12_ERROR.wav"
        volume: audioLevelNotification
    }

    Audio {
        id: audioACConnect
        source: pathSound + "/13_AC_CONNECT.wav"
        volume: audioLevelNotification
    }

    Audio {
        id: audioACDisconnect
        source: pathSound + "/13_AC_DISCONNECT.wav"
        volume: audioLevelNotification
    }

    Audio {
        id: audioSUDSPrimed
        source: pathSound + "/14_SUDS_PRIMED.wav"
        volume: audioLevelSudsPrimed
    }

    Audio {
        id: autdioTestSample1
        source: pathSound + "/T01_300Hz_0dBLR.wav"
        volume: audioLevelNormal
    }

    Audio {
        id: autdioTestSample2
        source: pathSound + "/T02_1kHz_0dBLR.wav"
        volume: audioLevelNormal
    }

    Audio {
        id: autdioTestSample3
        source: pathSound + "/T03_3_4kHz_0dBLR.wav"
        volume: audioLevelNormal
    }

    onAudioLevelNormalChanged:  {
        if (appMain.screenState == "Admin-Settings-Sound")
        {
            playNext();
        }
    }

    onAudioLevelInjectionChanged: {
        if (appMain.screenState == "Admin-Settings-Sound")
        {
            playStart();
        }
    }

    onAudioLevelNotificationChanged: {
        if (appMain.screenState == "Admin-Settings-Sound")
        {
            playError();
        }
    }

    onAudioLevelSudsPrimedChanged: {
        if (appMain.screenState == "Admin-Settings-Sound")
        {
            playSUDSPrimed();
        }
    }

    function stop()
    {
        autdioTestSample1.stop();
        autdioTestSample2.stop();
        autdioTestSample3.stop();
    }

    function isSoundPlaying(soundName)
    {
        return (soundName.playbackState === Audio.PlayingState);
    }

    function playPressKey()
    {
        if (audioKeyClicksEnabled)
        {
            audioPressGood.stop();
            audioPressGood.play();
        }
    }

    function playPressGood(volume)
    {
        audioPressGood.stop();
        audioPressGood.play();
    }

    function playPressAllStop()
    {
        audioPressBad.stop();
        audioPressBad.play();
    }

    function playNext()
    {
        audioNext.stop();
        audioNext.play();
    }

    function playArmed()
    {
        audioArmed.stop();
        audioArmed.play();
    }

    function playStart()
    {
        audioStart.stop();
        audioStart.play();
    }

    function playDisarmStop()
    {
        audioDisarmStop.stop();
        audioDisarmStop.play();
    }

    function playInjPulse()
    {
        audioInjPulse.stop();
        audioInjPulse.play();
    }

    function playReminderImminent()
    {
        audioReminderImminent.stop();
        audioReminderImminent.play();
    }

    function playReminderExpired()
    {
        audioReminderExpired.stop();
        audioReminderExpired.play();
    }

    function playInjComplete()
    {
        audioInjComplete.stop();
        audioInjComplete.play();
    }

    function playModalExpected()
    {
        audioModalExpected.stop();
        audioModalExpected.play();
    }

    function playModalUnexpected()
    {
        audioModalUnexpected.stop();
        audioModalUnexpected.play();
    }

    function playError()
    {
        audioError.stop();
        audioError.play();
    }

    function playACConnect()
    {
        audioACConnect.stop();
        audioACConnect.play();
    }

    function playACDisconnect()
    {
        audioACDisconnect.stop();
        audioACDisconnect.play();
    }

    function playSUDSPrimed()
    {
        audioSUDSPrimed.stop();
        audioSUDSPrimed.play();
    }

    function playTestSample1()
    {
        autdioTestSample1.stop();
        autdioTestSample1.play();
    }

    function playTestSample2()
    {
        autdioTestSample2.stop();
        autdioTestSample2.play();
    }

    function playTestSample3()
    {
        autdioTestSample3.stop();
        autdioTestSample3.play();
    }
}
