import QtQuick 2.12
import QtQuick.Controls 2.12
import "../../../Widgets"

AdminServiceToolTestTemplate {
    property string testMessageState: dsCru.testMessageState
    property int messageByteLen: 100
    property int txIntervalMs: 50
    property int processingTimeMs: 0
    property var txTime
    property int durationTotalMs
    property int testMessageCount: 0

    activeScreenState: "Admin-Service-Tool-Network"

    paramsTable: [
        AdminServiceToolTestTableRow {
            titleText: "Average Duration (ms)"
            valueText: (testMessageCount == 0) ? "--" : (durationTotalMs / testMessageCount).toFixed(0)
            controlType: "TEXT"
        },
        AdminServiceToolTestTableRow {
            titleText: "Message Bytes"
            valueText: messageByteLen
            onBtnClicked: {
                if (messageByteLen == 100)
                {
                    messageByteLen = 250;
                }
                else if (messageByteLen == 250)
                {
                    messageByteLen = 500;
                }
                else if (messageByteLen == 500)
                {
                    messageByteLen = 1000;
                }
                else
                {
                    messageByteLen = 100;
                }
            }
        },
        AdminServiceToolTestTableRow {
            titleText: "Interval (ms)"
            valueText: txIntervalMs
            onBtnClicked: {
                if (txIntervalMs == 50)
                {
                    txIntervalMs = 100;
                }
                else if (txIntervalMs == 100)
                {
                    txIntervalMs = 200;
                }
                else if (txIntervalMs == 200)
                {
                    txIntervalMs = 500;
                }
                else if (txIntervalMs == 500)
                {
                    txIntervalMs = 1000;
                }
                else if (txIntervalMs == 1000)
                {
                    txIntervalMs = 2000;
                }
                else
                {
                    txIntervalMs = 50;
                }
            }
        },
        AdminServiceToolTestTableRow {
            titleText: "Processing Time (ms)"
            valueText: processingTimeMs
            onBtnClicked: {
                if (processingTimeMs == 0)
                {
                    processingTimeMs = 10;
                }
                else if (processingTimeMs == 10)
                {
                    processingTimeMs = 50;
                }
                else if (processingTimeMs == 50)
                {
                    processingTimeMs = 100;
                }
                else if (processingTimeMs == 100)
                {
                    processingTimeMs = 200;
                }
                else if (processingTimeMs == 200)
                {
                    processingTimeMs = 500;
                }
                else if (processingTimeMs == 500)
                {
                    processingTimeMs = 1000;
                }
                else if (processingTimeMs == 1000)
                {
                    processingTimeMs = 2000;
                }
                else
                {
                    processingTimeMs = 0;
                }
            }
        }
    ]

    Timer {
        id: txTimer
        interval: txIntervalMs
        onTriggered: {
            dsCru.testMessageState = "Put Test Message (N=" + messageByteLen + ", T=" + processingTimeMs + ")";
            txTime = new Date();
            dsCru.slotPutTestMessage(messageByteLen, processingTimeMs);
        }
    }

    onTestMessageStateChanged: {
        if (testMessageState == "OK")
        {
            var durationMs = new Date() - txTime;
            consoleText.append(new Date().toTimeString("hh:mm:ss") + " -  " + testMessageState + ", duration=" + durationMs + "ms");
            durationTotalMs += durationMs;
            testMessageCount++;
        }
        else
        {
            consoleText.append(new Date().toTimeString("hh:mm:ss") + " -  " + testMessageState);
        }
        trimVisibleLog();
        if ( (!txTimer.running) &&
             (isTestStarted()) )
        {
            txTimer.start();
        }
    }

    onTestStarted: {
        durationTotalMs = 0;
        testMessageCount = 0;
        txTimer.start();
    }

    onTestStopped: {
        txTimer.stop();
    }
}
