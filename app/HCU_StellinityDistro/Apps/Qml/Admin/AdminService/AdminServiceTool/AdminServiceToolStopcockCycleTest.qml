import QtQuick 2.12
import "../../../Widgets"

AdminServiceToolTestTemplate {
    property var testStatus: dsTest.testStatus
    property bool mudsInserted: dsMcu.mudsInserted
    property int testCycles: 1
    property bool stopcockEnabled1: true
    property bool stopcockEnabled2: true
    property bool stopcockEnabled3: true
    property int pauseAfterMoveMs: mudsInserted ? 5000 : 1000

    activeScreenState: "Admin-Service-Tool-StopcockCycleTest"

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
            titleText: "Stopcock-S"
            initToggledValue: stopcockEnabled1
            controlType: "TOGGLE"
            onToggled: (activated) => {
                stopcockEnabled1 = activated;
            }
        },
        AdminServiceToolTestTableRow {
            titleText: "Stopcock-C1"
            initToggledValue: stopcockEnabled2
            controlType: "TOGGLE"
            onToggled: (activated) => {
                stopcockEnabled2 = activated;
            }
        },
        AdminServiceToolTestTableRow {
            titleText: "Stopcock-C2"
            initToggledValue: stopcockEnabled3
            controlType: "TOGGLE"
            onToggled: (activated) => {
                stopcockEnabled3 = activated;
            }
        },
        AdminServiceToolTestTableRow {
            titleText: "Pause Time Interval"
            valueText: (pauseAfterMoveMs / 1000) + "s"
            onBtnClicked: {
                if (pauseAfterMoveMs < 5000)
                {
                    pauseAfterMoveMs += 1000;
                }
                else if (pauseAfterMoveMs === 5000)
                {
                    pauseAfterMoveMs = 10000;
                }
                else
                {
                    pauseAfterMoveMs += 10000;
                }

                if (pauseAfterMoveMs > 30000)
                {
                    pauseAfterMoveMs = mudsInserted ? 5000 : 0;
                }
            }
        }
    ]

    onTestStarted: {
        // Start test
        var params = [];
        params.push(testCycles);
        params.push(stopcockEnabled1);
        params.push(stopcockEnabled2);
        params.push(stopcockEnabled3);
        params.push(pauseAfterMoveMs);
        dsTest.slotTestStart("Stopcock", params);
    }

    onTestStopped: {
        dsTest.slotTestStop();
    }

    onTestStatusChanged: {
        if (testStatus === undefined)
        {
            return;
        }

        if (testStatus.Type === "Stopcock")
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

