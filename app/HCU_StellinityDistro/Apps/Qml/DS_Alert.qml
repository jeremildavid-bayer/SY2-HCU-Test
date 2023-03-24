import QtQuick 2.12

Item {
    // Data
    property var activeAlerts: []
    property var activeSystemAlerts: []
    property var inactiveAlerts: []

    property var activeAlertsMuds: []
    property var activeAlertsSuds: []
    property var activeAlertsBottle1: []
    property var activeAlertsBottle2: []
    property var activeAlertsBottle3: []
    property var activeAlertsSyringe1: []
    property var activeAlertsSyringe2: []
    property var activeAlertsSyringe3: []
    property var activeAlertsWasteContainer: []

    property bool showDeviceAlertIcon: (activeAlertsMuds.length > 0) ||
                                       (activeAlertsSuds.length > 0) ||
                                       (activeAlertsSyringe1.length > 0) ||
                                       (activeAlertsSyringe2.length > 0) ||
                                       (activeAlertsSyringe3.length > 0) ||
                                       (activeAlertsBottle1.length > 0) ||
                                       (activeAlertsBottle2.length > 0) ||
                                       (activeAlertsBottle3.length > 0) ||
                                       (activeAlertsWasteContainer.length > 0)

    // Function from QML to CPP
    function slotActivateAlert(codeName, data)  { return dsAlertCpp.slotActivateAlert(codeName, data); }
    function slotDeactivateAlert(codeName, data)  { return dsAlertCpp.slotDeactivateAlert(codeName, data); }
    function slotDeactivateAlertWithReason(codeName, newData, oldData, ignoreData)  { return dsAlertCpp.slotDeactivateAlertWithReason(codeName, newData, oldData, ignoreData); }
    function slotGetActiveAlertFromCodeName(codeName)  { return dsAlertCpp.slotGetActiveAlertFromCodeName(codeName); }
    function slotGetAlertFromCodeName(codeName)  { return dsAlertCpp.slotGetAlertFromCodeName(codeName); }
    function slotGetMergedAlerts(alerts) { return dsAlertCpp.slotGetMergedAlerts(alerts); }
}
