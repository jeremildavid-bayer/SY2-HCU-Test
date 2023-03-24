import QtQuick 2.12
import "../Widgets"
import "../Widgets/Popup"
import "../Util.js" as Util

PopupMessage {
    property var alertData
    property bool isHCUInternalSoftwareErrorType: ( (alertData !== undefined) && (alertData.CodeName === "HCUInternalSoftwareError") )

    type: isHCUInternalSoftwareErrorType ? "WARNING" : "PLAIN"
    titleText: "SRU SW Critical Error"
    showCancelBtn: false
    okBtnText: translate("T_OK")
    heightMin: isHCUInternalSoftwareErrorType ? dsCfgLocal.screenH * 0.8 : dsCfgLocal.screenH * 0.4
    translationRequired: !isHCUInternalSoftwareErrorType
    contentText: {
        if (isHCUInternalSoftwareErrorType)
        {
            return alertData.Data;
        }
        if ( (alertData !== undefined) &&
             (alertData.Data !== undefined) &&
             (alertData.Data.indexOf("T_EXAMOPERATIONFAILED") >= 0) )
        {
            return alertData.Data;
        }
        return "T_SoftwareError";
    }


    onBtnOkClicked: {
        if (contentText === "T_EXAMOPERATIONFAILED_ExamDoesNotExist")
        {
            examDoesNotExist_RePrapareExam();
        }
        popupHCUSoftwareErrorAlertManager.removeAlert(alertData);
        close();
    }

    function examDoesNotExist_RePrapareExam()
    {
        dsExam.slotExamProgressStateChanged("Prepared");

        if (dsCru.licenseEnabledPatientStudyContext)
        {
            dsExam.examScreenState = "ExamManager-PatientSelection";
            dsMwl.slotPatientsReload();
        }
        else
        {
            dsExam.examScreenState = "ExamManager-ProtocolSelection";
        }

        appMain.setScreenState(dsExam.examScreenState);
    }
}




