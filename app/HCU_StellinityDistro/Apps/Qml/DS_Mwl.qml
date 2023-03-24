import QtQuick 2.12

Item {
    // Data
    property string suiteName: ""
    property string patientName: ""
    property string studyDescription: ""
    property var worklistEntries: []

    // Function from QML to CPP
    function slotPatientsReload() { return dsMwlCpp.slotPatientsReload(); }
    function slotSelectWorklistEntry(studyUid) { return dsMwlCpp.slotSelectWorklistEntry(studyUid); }
    function slotDeselectWorklistEntry() { return dsMwlCpp.slotDeselectWorklistEntry(); }
}
