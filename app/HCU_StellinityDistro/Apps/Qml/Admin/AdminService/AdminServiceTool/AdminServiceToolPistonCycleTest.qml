import QtQuick 2.12
import "../../../Widgets"

AdminServiceToolTestTemplate {
    property int injectVol: -1
    property double injectFlow: dsCapabilities.flowRateMax
    property double fillFlow: dsCapabilities.flowRateMax
    property var testStatus: dsTest.testStatus
    property int testCycles: 1
    property bool pistonEnabled1: true
    property bool pistonEnabled2: true
    property bool pistonEnabled3: true
    property bool disengageAfterFill: true
    property bool purgeAfterEngage: false
    property int pauseAfterFillMs: 1000
    property int pauseAfterDisengageMs: 1000

    activeScreenState: "Admin-Service-Tool-PistonCycleTest"


    paramsTable: [
        AdminServiceToolTestTableRow {
            titleText: "Test Cycles"
            valueText: testCycles
            onBtnClicked: {
                if (testCycles < 10)
                {
                    testCycles++;
                }
                else if (testCycles < 20)
                {
                    testCycles += 5;
                }
                else if (testCycles < 100)
                {
                    testCycles += 10;
                }
                else if (testCycles < 1000)
                {
                    testCycles += 100;
                }
                else if (testCycles < 10000)
                {
                    testCycles += 1000;
                }
                else if (testCycles < 200000)
                {
                    testCycles += 10000;
                }
                else
                {
                    testCycles = 1;
                }
            }
        },
        AdminServiceToolTestTableRow {
            titleText: "Piston-S"
            initToggledValue: pistonEnabled1
            controlType: "TOGGLE"
            onToggled: (activated) => {
                pistonEnabled1 = activated;
            }
        },
        AdminServiceToolTestTableRow {
            titleText: "Piston-C1"
            initToggledValue: pistonEnabled2
            controlType: "TOGGLE"
            onToggled: (activated) => {
                pistonEnabled2 = activated;
            }
        },
        AdminServiceToolTestTableRow {
            titleText: "Piston-C2"
            initToggledValue: pistonEnabled3
            controlType: "TOGGLE"
            onToggled: (activated) => {
                pistonEnabled3 = activated;
            }
        },
        AdminServiceToolTestTableRow {
            titleText: "Disengage After Fill"
            initToggledValue: disengageAfterFill
            controlType: "TOGGLE"
            onToggled: (activated) => {
                disengageAfterFill = activated;
            }
        },
        AdminServiceToolTestTableRow {
            titleText: "Inject Flow"
            valueText: injectFlow.toFixed(1).toString() + "ml/s"
            onBtnClicked: {
                injectFlow += 0.5;
                if (injectFlow > dsCapabilities.flowRateMax)
                {
                    injectFlow = 1.0;
                }
            }
        },
        AdminServiceToolTestTableRow {
            titleText: "Fill Flow"
            valueText: fillFlow.toFixed(1).toString() + "ml/s"
            onBtnClicked: {
                fillFlow += 0.5;
                if (fillFlow > dsCapabilities.flowRateMax)
                {
                    fillFlow = 1.0;
                }
            }
        },
        AdminServiceToolTestTableRow {
            titleText: "Pause Time After Fill"
            valueText: (pauseAfterFillMs / 1000).toString() + "s"
            onBtnClicked: {
                if (pauseAfterFillMs < 5000)
                {
                    pauseAfterFillMs += 1000;
                }
                else if (pauseAfterFillMs === 5000)
                {
                    pauseAfterFillMs = 10000;
                }
                else
                {
                    pauseAfterFillMs += 10000;
                }

                if (pauseAfterFillMs > 100000)
                {
                    pauseAfterFillMs = 1000;
                }
            }
        },
        AdminServiceToolTestTableRow {
            titleText: "Pause Time After Disengage"
            valueText: (pauseAfterDisengageMs / 1000).toString() + "s"
            onBtnClicked: {
                if (pauseAfterDisengageMs < 5000)
                {
                    pauseAfterDisengageMs += 1000;
                }
                else if (pauseAfterDisengageMs === 5000)
                {
                    pauseAfterDisengageMs = 10000;
                }
                else
                {
                    pauseAfterDisengageMs += 10000;
                }

                if (pauseAfterDisengageMs > 100000)
                {
                    pauseAfterDisengageMs = 1000;
                }
            }
        },
        AdminServiceToolTestTableRow {
            titleText: "Purge after engage"
            initToggledValue: purgeAfterEngage
            controlType: "TOGGLE"
            onToggled: (activated) => {
                purgeAfterEngage = activated;
            }
        }
    ]

    onTestStarted: {
        // Start test
        var params = [];
        params.push(testCycles);
        params.push(pistonEnabled1);
        params.push(pistonEnabled2);
        params.push(pistonEnabled3);
        params.push(disengageAfterFill);
        params.push(injectVol);
        params.push(injectFlow);
        params.push(fillFlow);
        params.push(pauseAfterFillMs);
        params.push(pauseAfterDisengageMs);
        params.push(purgeAfterEngage);
        dsTest.slotTestStart("Piston", params);
    }

    onTestStopped: {
        dsTest.slotTestStop();
    }

    onTestStatusChanged: {
        if (testStatus === undefined)
        {
            return;
        }

        //logDebug("testStatus=" + JSON.stringify(testStatus));
        if (testStatus.Type === "Piston")
        {
            consoleText.append(new Date().toTimeString("hh:mm:ss") + " -  " + testStatus.StateStr);
            trimVisibleLog();
            if (testStatus.IsFinished)
            {
                if (isTestStarted())
                {
                    stopTest();
                }
            }
        }
    }
}

