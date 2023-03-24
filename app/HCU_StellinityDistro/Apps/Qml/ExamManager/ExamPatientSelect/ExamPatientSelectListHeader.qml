import QtQuick 2.12
import "../../Widgets"

Item {
    width: parent.width

    Row {
        anchors.fill: parent

        ExamPatientSelectListHeaderSortButton {
            field: "studyDateTime"
            fieldText: translate("T_DateTime")
            colWidthPercent: headerColWidthPercentStudyDateTime
            textMargin: colSpacing * 3
        }

        ExamPatientSelectListHeaderSortButton {
            field: "patientName"
            fieldText: translate("T_Name")
            colWidthPercent: headerColWidthPercentPatientName
            leftMargin: colSpacing
        }

        ExamPatientSelectListHeaderSortButton {
            field: "patientSex"
            fieldText: translate("T_PatientSex")
            colWidthPercent: headerColWidthPercentPatientSex
            leftMargin: colSpacing
        }

        ExamPatientSelectListHeaderSortButton {
            field: "patientBirthDate"
            fieldText: translate("T_DateOfBirth")
            colWidthPercent: headerColWidthPercentPatientBirthDate
            leftMargin: colSpacing
        }

        ExamPatientSelectListHeaderSortButton {
            field: "patientId"
            fieldText: translate("T_MrnId")
            colWidthPercent: headerColWidthPercentPatientId
            leftMargin: colSpacing
        }

        ExamPatientSelectListHeaderSortButton {
            field: "accessionNumber"
            fieldText: translate("T_AccessionNumber")
            colWidthPercent: headerColWidthPercentAccessionNumber
            leftMargin: colSpacing
        }

        ExamPatientSelectListHeaderSortButton {
            field: "studyDescription"
            fieldText: translate("T_Description")
            colWidthPercent: headerColWidthPercentStudyDescription
            leftMargin: colSpacing
        }
    }
}
