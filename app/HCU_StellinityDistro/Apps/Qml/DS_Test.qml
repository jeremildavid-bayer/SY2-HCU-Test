import QtQuick 2.12

Item {
    // Data
    property var testStatus: ({})

    // Signal from QML to CPP
    function slotTestStart(testName, testParams) { return dsTestCpp.slotTestStart(testName, testParams); }
    function slotTestStop() { return dsTestCpp.slotTestStop(); }
}
