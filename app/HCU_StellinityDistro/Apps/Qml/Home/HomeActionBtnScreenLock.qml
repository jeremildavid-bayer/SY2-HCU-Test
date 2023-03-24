import QtQuick 2.12
import "../Widgets"

Item {
    property int screenLockProgress: 0
    property bool screenLockStarted
    property string screenLockCaption: "T_ScreenLock"

    signal signalLocked();

    GenericIconButton {
        id: btnLock
        anchors.fill: parent
        color: "transparent"
        iconText: "\ue903"
        iconFontPixelSize: height * 0.5
        captionText: translate(screenLockCaption)

        onBtnPressed: {
            screenLockStarted = true;
            screenLockProgress = 0;
            progressArc.setProgress(screenLockProgress);
            screenLockProgressTimer.start();
        }

        onBtnReleased: {
            screenLockStarted = false;
            if (screenLockProgress < 100)
            {
                progressArc.setProgress(0);
            }
            screenLockProgressTimer.stop();
        }

        onBtnExit: {
            screenLockStarted = false;
            if (screenLockProgress < 100)
            {
                progressArc.setProgress(0);
            }
            screenLockProgressTimer.stop();
        }
    }

    Timer {
        id: screenLockProgressTimer
        interval: 5
        onTriggered: {
            if (!screenLockStarted)
            {
                return;
            }

            screenLockProgress += 2;

            if (screenLockProgress >= 100)
            {
                screenLockStarted = false;
                signalLocked();
                btnLock.reset();
            }
            else
            {
                screenLockProgressTimer.start();
            }

            progressArc.setProgress(screenLockProgress);
        }
    }

    ProgressArc {
        id: progressArc
        width: parent.width
        height: parent.height * 1.3
        y: parent.height * 0.04
        animationMs: 5
        lineColor: colorMap.text01
        angleStart: (1 - 0.1) * Math.PI
        angleTotal: (1 + 0.2) * Math.PI
    }

    function reset()
    {
        progressArc.setProgress(0);
    }
}


