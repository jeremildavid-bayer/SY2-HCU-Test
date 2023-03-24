import QtQuick 2.12
import "../Widgets"
import "../Widgets/Popup"

PopupMessage {
    property var alertData
    property var args: alertData.Data.split(";")

    type: "WARNING"

    showOkBtn: (alertData.CodeName !== "GenericNondismissablePopup")
    okBtnText: "T_OK"
    showCancelBtn: false

    titleText: args.length > 0 ? args[0] : ""
    contentText: args.length > 1 ? args[1] : ""

    onBtnOkClicked: {
        dsAlert.slotDeactivateAlert(alertData.CodeName, alertData.Data);
    }
}
