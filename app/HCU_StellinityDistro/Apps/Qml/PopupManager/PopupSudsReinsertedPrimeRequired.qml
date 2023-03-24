import QtQuick 2.12
import "../Widgets"
import "../Widgets/Popup"

PopupAlertBase {
    alertCodeName: "SUDSReinsertedPrimeRequired"
    titleText: "T_SUDSReinsertedPrimeRequired_Name"
    userDirectionText: "T_SUDSReinsertedPrimeRequired_UserDirection"

    onBtnOkClicked: {
        dsWorkflow.slotSudsAutoPrimeForceStart();
        close();
    }

    onActiveAlertChanged: {
        if (activeAlert === undefined) {
            return;
        }

        showUserPrimeRequired();
    }

    function showUserPrimeRequired()
    {
        contentText = userDirectionText;

        okBtnText = "T_Prime";
        showCancelBtn = false;
    }
}
