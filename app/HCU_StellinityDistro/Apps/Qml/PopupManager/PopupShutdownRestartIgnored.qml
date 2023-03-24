import QtQuick 2.12
import "../Widgets"
import "../Widgets/Popup"
import "../Util.js" as Util

PopupMessage {
    type: "PLAIN"
    contentText: "T_ShutdownRestartIgnored_InjectorIsBusy"
    showCancelBtn: false

    onBtnOkClicked: {
        close();
    }

    onOpened: {
        appMain.setInteractiveState(true, "PopupShutdownRestartIgnored Open");
    }
}

