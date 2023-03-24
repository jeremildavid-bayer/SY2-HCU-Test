import QtQuick 2.12
import "../Widgets"
import "../Widgets/Popup"

PopupMessage {
    property var syringeState: dsMcu.syringeStates
    property var fluidSourceBottle1: dsDevice.fluidSourceBottle1
    property var fluidSourceBottle2: dsDevice.fluidSourceBottle2
    property var fluidSourceBottle3: dsDevice.fluidSourceBottle3

    property bool isFillStopPending: {
        return ( ( (fluidSourceBottle1 !== undefined) && (fluidSourceBottle1.IsBusy !== undefined) && (fluidSourceBottle1.IsBusy) && (syringeState[0] === "STOP_PENDING")) ||
                 ( (fluidSourceBottle2 !== undefined) && (fluidSourceBottle2.IsBusy !== undefined) && (fluidSourceBottle2.IsBusy) && (syringeState[1] === "STOP_PENDING")) ||
                 ( (fluidSourceBottle3 !== undefined) && (fluidSourceBottle3.IsBusy !== undefined) && (fluidSourceBottle3.IsBusy) && (syringeState[2] === "STOP_PENDING")) );
    }

    type: "INFO"
    titleText: "T_FillAbortWaiting_Title"
    contentText: "T_FillAbortWaiting_Message"
    showOkBtn: false
    showCancelBtn: false
    btnHeight: 0

    Timer {
        id: tmrFillAbort
        interval: 500
        onTriggered: {
            if (!isOpen())
            {
                return;
            }
        }
    }

    onIsFillStopPendingChanged: {
        if (isFillStopPending)
        {
            if (!isOpen())
            {
                logInfo("PopupBottleFillAbortWaiting: Stop button pressed during filling.");
                open();
                tmrFillAbort.start();
            }
        }
        else
        {
            if (isOpen())
            {
                close();
            }
        }
    }
}
