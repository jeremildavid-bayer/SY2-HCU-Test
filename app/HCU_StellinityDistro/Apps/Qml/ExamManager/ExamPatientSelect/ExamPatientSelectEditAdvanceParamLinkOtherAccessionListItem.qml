import QtQuick 2.12
import "../../Widgets"
import "../../Util.js" as Util

GenericButton {
    property var rowData: linkedAccessions[index]
    property var entry: rowData.Entry
    property var dicomFields: entry.DicomFields
    property double textHMargin: parent.width * 0.01

    anchors.right: parent.right
    width: ListView.view.width * 0.96
    height: rowHeight

    Text {
        id: textAccessionNumber
        anchors.left: parent.left
        anchors.right: textStudyDescription.left
        anchors.rightMargin: textHMargin
        height: parent.height
        verticalAlignment: Text.AlignVCenter
        font.pixelSize: height * 0.3
        font.family: fontRobotoLight.name
        text: dicomFields.accessionNumber.Value
        color: colorMap.text01
        wrapMode: Text.Wrap
        fontSizeMode: Text.Fit
        minimumPixelSize: font.pixelSize * 0.5
    }

    Text {
        id: textStudyDescription
        anchors.right: toggleButton.left
        anchors.rightMargin: textHMargin
        width: parent.width * 0.5
        height: parent.height
        verticalAlignment: Text.AlignVCenter
        font.pixelSize: height * 0.3
        font.family: fontRobotoLight.name
        text: dicomFields.studyDescription.Value
        color: colorMap.text01
        wrapMode: Text.Wrap
        fontSizeMode: Text.Fit
        minimumPixelSize: font.pixelSize * 0.5
    }

    GenericToggleButton {
        id: toggleButton
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width * 0.18
        height: parent.height * 0.75
        isReadOnly: true
    }

    Rectangle {
        id: separatorLine
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        width: parent.width
        height: 1
        color: colorMap.text02
    }

    onBtnClicked: {
        toggleButton.toggle();
        dsCru.slotPostUpdateLinkedAccession(entry, toggleButton.activated);
    }

    Component.onCompleted: {
        listViewAdvanceParamsList.dragStarted.connect(reset);
    }

    Component.onDestruction: {
        listViewAdvanceParamsList.dragStarted.disconnect(reset);
    }

    onRowDataChanged: {
        // Anonymize entire row data object if VNV or REL
        var rowDataLog = "rowData[" + index + "]= " + JSON.stringify(rowData);
        if ((hcuBuildType === "REL") || (hcuBuildType === "VNV"))
        {
            rowDataLog = "";
        }
        //logDebug(rowDataLog);
        toggleButton.activated = rowData.IsLinked;
    }
}
