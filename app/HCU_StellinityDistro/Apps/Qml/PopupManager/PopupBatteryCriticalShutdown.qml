import QtQuick 2.12
import "../Widgets"
import "../Widgets/Popup"

PopupMessage {
    property var powerStatus: dsMcu.powerStatus

    type: "WARNING"
    showCancelBtn: false
    showOkBtn: false
    titleText: "T_InjectorBatteryIsCritical_Name"
    contentText: "T_InjectorBatteryIsCritical_UserDirection"

    onPowerStatusChanged: {
        if (powerStatus === undefined)
        {
            return;
        }

        if ( (!powerStatus.IsAcPowered) &&
             (powerStatus.BatteryLevel === "Critical") )
        {
            open();
        }
    }
}

