import QtQuick.Window 2.3
import QtQuick 2.9
import QtQuick.Controls 1.4

Window {
    property int launchRemainSec: 0

    signal qmlSignalClose()
    signal qmlSignalLaunchStart()

    //------------------------------------------
    // App Main Properties
    objectName: "appMain"
    id: appMain
    visible: false
    x: 40
    y: 40

    width: 1200
    height: 80
    title: "HCU_StellinityDistroLauncher"
    flags: Qt.SplashScreen
    color: "transparent"

    Action {
        shortcut: "Alt+4"
        enabled: true
        onTriggered: {
            qmlSignalClose();
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            appMain.requestActivate();
        }
    }

    Timer {
        id: launcherTimer
        interval: 1000
        onTriggered: {
            launchRemainSec--;
            textRemainSec.text = launchRemainSec;
            if (launchRemainSec < 1)
            {
                qmlSignalLaunchStart();
                appMain.visible = false;
            }
            else
            {
                launcherTimer.start();
            }
        }
    }

    Row {
        id: rectSavingLogData
        Text {
            text: "Saving Application Log Data.."
            font.pointSize: 36
            color: "white"
        }
    }

    Row {
        id: rectAppStartCountDown
        Text {
            text: "Main Application is starting in "
            font.pointSize: 36
            color: "white"
        }

        Text {
            color: "orange"
            id: textRemainSec
            text: "XX"
            font.pointSize: 36
        }

        Text {
            text: " seconds.."
            font.pointSize: 36
            color: "white"
        }
    }

    Component.onCompleted: {
        appMain.requestActivate();
    }

    function slotSavingLogData()
    {
        rectSavingLogData.visible = true
        rectAppStartCountDown.visible = false;
        appMain.visible = true;
    }

    function slotStartCountDown(countDownTimeSec)
    {
        rectSavingLogData.visible = false
        rectAppStartCountDown.visible = true;

        launchRemainSec = countDownTimeSec;
        textRemainSec.text = launchRemainSec;
        launcherTimer.start();
        appMain.visible = true;
        //console.log("Start counting down..");
    }
}
