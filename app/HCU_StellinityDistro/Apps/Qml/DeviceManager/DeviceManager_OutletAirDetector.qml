import QtQuick 2.9
import "Widgets"
import "../Widgets"
import "../Util.js" as Util

DeviceManager_InfoBase {
    property string cultureCode: dsCfgGlobal.cultureCode
    property string outletDoorState: dsDevice.outletDoorState
    property bool sudsBubbleDetected: dsDevice.sudsBubbleDetected
    property var fluidSourceMuds: dsDevice.fluidSourceMuds
    property var activeAlerts: dsAlert.activeAlerts

    id: root
    pageName: "DeviceManager-PatientAir"
    titleText: "T_OutletAirDetector"
    color: colorMap.mainBackground
    visible: false
    strRow1Text: ""

    state: "NO_MUDS"

    states: [
        State {
            name: "NO_MUDS"
            PropertyChanges { target: root; strRow1Text: "" }
        },
        State {
            name: "DOOR_OPEN"
        },
        State {
            name: "NORMAL"
        },
        State {
            name: "AIR_DETECTED"
        },
        State {
            name: "UNKNOWN"
            PropertyChanges { target: root; strRow2Text: "T_UnknownState" }
            PropertyChanges { target: root; row2AlertActive: true }
        }
    ]

    onCultureCodeChanged: {
        reload();
    }

    onOutletDoorStateChanged: {
        reload();
    }

    onSudsBubbleDetectedChanged: {
        reload();
    }

    onFluidSourceMudsChanged: {
        reload();
    }

    onActiveAlertsChanged: {
        if (!visible)
        {
            return;
        }

        checkOutletAirDetectorAlerts();
    }

    function reload()
    {
        if (!visible)
        {
            return;
        }

        setPatientAirState();
        checkOutletAirDetectorAlerts();
    }

    function setPatientAirState()
    {
        if ( (fluidSourceMuds === undefined) ||
             (fluidSourceMuds === null) )
        {
            root.state = "NO_MUDS";
            return;
        }

        if (outletDoorState === "CLOSED")
        {
            root.state = sudsBubbleDetected ? "AIR_DETECTED" : "NORMAL";
        }
        else if (outletDoorState === "OPEN")
        {
            root.state = "DOOR_OPEN";
        }
        else
        {
            root.state = "UNKNOWN";
        }
    }

    function checkOutletAirDetectorAlerts()
    {
        var newAlerts = [];
        for (var i = 0; i < activeAlerts.length; i++)
        {
            if ( (activeAlerts[i].CodeName === "OutletAirDetectorFault") ||
                 (activeAlerts[i].CodeName === "OutletAirDoorLeftOpen") ||
                 (activeAlerts[i].CodeName === "OutletAirDetected") )
            {
                newAlerts.push(activeAlerts[i]);
            }
        }

        root.alerts = newAlerts;
    }
}
