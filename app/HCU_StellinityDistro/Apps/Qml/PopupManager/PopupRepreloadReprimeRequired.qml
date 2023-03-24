import QtQuick 2.12
import "../Widgets"

PopupAlertBase {
    property string examProgressState: dsExam.examProgressState

    alertCodeName: "RepreloadReprimeRequired"
    titleText: "T_RepreloadReprimeRequired_Name"
    userDirectionText: {
        if ( (activeAlert !== undefined) &&
             ((activeAlert.Data.indexOf("PreloadedInjectionChanged") >= 0) ||
              (activeAlert.Data.indexOf("ContrastFluidLocationChanged") >= 0)) )
        {
            return  "T_RepreloadReprimeRequired_UserDirection";
        }
        return "T_RepreloadReprimeRequired_ReprimeOnly";
    }

    showCancelBtn: userDirectionText === "T_RepreloadReprimeRequired_UserDirection"
    okBtnText: "T_Reprime"
    cancelBtnText: "T_Preload"

    onBtnOkClicked: {
        dsWorkflow.slotSudsAutoPrimeForceStart();
    }

    onBtnCancelClicked: {
        dsWorkflow.slotPreloadProtocolStart(false);
        close();
    }

    onExamProgressStateChanged: {
        if ( (isOpen()) &&
             ((examProgressState === "Completing") || (examProgressState == "Completed")) )
        {
            close();
        }
    }

}
