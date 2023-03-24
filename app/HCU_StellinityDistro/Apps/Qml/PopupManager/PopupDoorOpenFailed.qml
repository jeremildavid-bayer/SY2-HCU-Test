import QtQuick 2.12
import "../Widgets/Popup"

PopupMessage {
    property bool doorBtnPressed: dsMcu.doorBtnPressed
    property bool sudsInserted: dsMcu.sudsInserted
    property string statePath: dsSystem.statePath

    type: "INFO"
    titleText: "T_DoorOpenPrevented_Title"
    contentText: "T_DoorOpenPreventedBySUDSInserted_Message";
    showCancelBtn: false

    onBtnOkClicked: {
        close();
    }

    onDoorBtnPressedChanged: {
        if (isOpen())
        {
            return;
        }

        if (statePath != "Idle")
        {
            return;
        }

        if ( (doorBtnPressed) &&
             (sudsInserted) )
        {
            open();
        }
    }
}

