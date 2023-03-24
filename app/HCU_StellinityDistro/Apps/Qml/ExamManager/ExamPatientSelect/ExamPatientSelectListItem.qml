import QtQuick 2.12
import "../../Widgets"

GenericButton {
    property var rowData: worklistEntriesSorted[index]
    property var dicomFields: (rowData === undefined) ? undefined : rowData["DicomFields"]
    property int margin: colSpacing * 3
    property bool isListItem: true
    property bool searchStrFound: fieldStudyDateTime.searchStrFound || fieldPatientName.searchStrFound || fieldPatientSex.searchStrFound || fieldPatientBirthDate.searchStrFound || fieldPatientId.searchStrFound || fieldAccessionNumber.searchStrFound || fieldStudyDescription.searchStrFound
    property int animationMs: 200

    width: ListView.view.width
    height: {
        if (rowData === undefined)
        {
            return 0;
        }
        else if ( (isListItem) && (!searchStrFound) )
        {
            return 0;
        }
        return rowHeight + Math.max(rowBorder.height, groupBorder.height);
    }
    radius: 0
    clip: true

    Behavior on height {
        PropertyAnimation {
            duration: animationMs
        }
    }

    Row {
        width: parent.width
        height: rowHeight

        ExamPatientSelectListItemField {
            id: fieldStudyDateTime
            dicomField: (dicomFields === undefined) ? undefined : dicomFields["studyDateTime"]
            colWidthPercent: headerColWidthPercentStudyDateTime
            leftMargin: margin
        }

        ExamPatientSelectListItemField {
            id: fieldPatientName
            dicomField: (dicomFields === undefined) ? undefined : dicomFields["patientName"]
            colWidthPercent: headerColWidthPercentPatientName
            leftMargin: colSpacing
        }

        ExamPatientSelectListItemField {
            id: fieldPatientSex
            dicomField: (dicomFields === undefined) ? undefined : dicomFields["patientSex"]
            colWidthPercent: headerColWidthPercentPatientSex
            leftMargin: colSpacing
        }

        ExamPatientSelectListItemField {
            id: fieldPatientBirthDate
            dicomField: (dicomFields === undefined) ? undefined : dicomFields["patientBirthDate"]
            colWidthPercent: headerColWidthPercentPatientBirthDate
            leftMargin: colSpacing
        }

        ExamPatientSelectListItemField {
            id: fieldPatientId
            dicomField: (dicomFields === undefined) ? undefined : dicomFields["patientId"]
            colWidthPercent: headerColWidthPercentPatientId
            leftMargin: colSpacing
        }

        ExamPatientSelectListItemField {
            id: fieldAccessionNumber
            dicomField: (dicomFields === undefined) ? undefined : dicomFields["accessionNumber"]
            colWidthPercent: headerColWidthPercentAccessionNumber
            leftMargin: colSpacing
        }

        ExamPatientSelectListItemField {
            id: fieldStudyDescription
            dicomField: (dicomFields === undefined) ? undefined : dicomFields["studyDescription"]
            colWidthPercent: headerColWidthPercentStudyDescription
            leftMargin: colSpacing
            rightMargin: margin
        }
    }

    Rectangle {
        id: rowBorder
        y: parent.height - 1
        x: margin
        width: parent.width - margin - x
        height: (isListItem) && (index < worklistEntriesSorted.length - 1) ? 1 : 0
        color: colorMap.text02
        opacity: 0.5
    }

    Rectangle {
        id: groupBorder
        y: parent.height - height
        x: margin
        width: parent.width - margin - x
        height: (isListItem) && (index < worklistEntriesSorted.length) && (rowData.isEndOfGroup !== undefined) && (rowData.isEndOfGroup === true) ? 3 : 0
        color: colorMap.text01
        opacity: 0.8
    }

    onBtnClicked: {
        examPatientSelect.selectEntry(rowData);
    }

    Component.onCompleted: {
        if (isListItem)
        {
            worklistView.dragStarted.connect(reset);
        }
    }

    Component.onDestruction: {
        if (isListItem)
        {
            worklistView.dragStarted.disconnect(reset);
        }
    }
}
