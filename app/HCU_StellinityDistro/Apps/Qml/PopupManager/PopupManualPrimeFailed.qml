import QtQuick 2.12
import "../Widgets/Popup"

PopupMessage {
    property bool primeBtnPressed: dsMcu.primeBtnPressed
    property string statePath: dsSystem.statePath

    type: "INFO"
    titleText: "T_ManualPrimePrevented_Title"
    showCancelBtn: false

    onBtnOkClicked: {
        close();
    }

    onPrimeBtnPressedChanged: {
        if (isOpen())
        {
            return;
        }

        if ( (primeBtnPressed) &&
             (statePath == "Ready/Armed") )
        {
            contentText = "T_ManualPrimePreventedByInjectorArmed_Message";
            open();
        }
    }
}

