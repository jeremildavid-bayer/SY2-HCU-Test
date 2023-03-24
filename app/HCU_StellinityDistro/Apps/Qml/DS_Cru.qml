import QtQuick 2.12

Item {
    // Data
    property var cruLinkStatus: { "Type": "Unknown", "State": "Inactive" }
    property string testMessageState: ""
    property string wifiSsid: ""
    property string wifiPassword: ""
    property bool licenseEnabledWorklistSelection: false
    property bool licenseEnabledPatientStudyContext: false

    // Function from QML to CPP
    function slotGetTestMessage() { return dsCruCpp.slotGetTestMessage(); }
    function slotPutTestMessage(messageByteLen, processingTimeMs) { return dsCruCpp.slotPutTestMessage(messageByteLen, processingTimeMs); }
    function slotPostUpdateInjectionParameter(param) { return dsCruCpp.slotPostUpdateInjectionParameter(param); }
    function slotPostUpdateExamField(param) { return dsCruCpp.slotPostUpdateExamField(param); }
    function slotPostUpdateExamFieldParameter(param) { return dsCruCpp.slotPostUpdateExamFieldParameter(param); }
    function slotPostUpdateLinkedAccession(entry, isLinked) { return dsCruCpp.slotPostUpdateLinkedAccession(entry, isLinked); }
    function slotApplyLimits() { return dsCruCpp.slotApplyLimits(); }
}
