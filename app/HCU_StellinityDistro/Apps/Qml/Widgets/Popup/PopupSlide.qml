import QtQuick 2.12
import "../"

PopupMessage {
    showCancelBtn: false
    showOkBtn: false
    showOtherBtn: false
    showFromStartupScreen: false
    showDuringServicing: false

    property bool unlock: sliderLock.unlock

    contentSurroundingHeight: getContentSurroundingHeight() + sliderLock.height

    onOpened: {
        sliderLock.reset();
    }

    onUnlockChanged: {
        if (unlock) {
            close("Slider Unlocked");
        }
    }

    SlideToUnlock {
        id: sliderLock
        anchors.top: mainRect.bottom
        anchors.topMargin: 20
        anchors.left: parent.left
        anchors.right: parent.right
        height: 80
    }
}
