import QtQuick 2.12
import "../ExamInjectionPlot"
import "../../Widgets"
import "../../Widgets/Popup"
import "../../Util.js" as Util

Item {
    property int leftFrameWidth: (dsCfgLocal.screenW * 0.33) + (frameMargin * 2)

    anchors.fill: parent
    visible: false

    ExamInjectionMonitorStep {
        id: injectionProgressStep
        x: frameMargin
        width: leftFrameWidth - (frameMargin * 2)
        height: parent.height
    }

    Rectangle {
        id: separatorLine
        x: injectionProgressStep.x + injectionProgressStep.width
        width: parent.width * 0.003
        height: parent.height
        color: colorMap.titleBarBackground
    }

    Item {
        x: separatorLine.x + separatorLine.width
        width: parent.width - x
        height: parent.height

        ExamInjectionPlot {
            id: injectionPlot
            width: parent.width
            height: parent.height * 0.85
        }

        ExamInjectionMonitorOverview {
            anchors.left: parent.left
            anchors.leftMargin: parent.width * 0.1
            y: parent.height - height
            width: parent.width - x
            height: parent.height * 0.18
        }
    }

    Component.onCompleted: {
        dsExam.qmlInjectionMonitor = this;
        dsExam.qmlInjectionPlot = injectionPlot;
        appMain.screenStateChanged.connect(slotScreenStateChanged);
        slotScreenStateChanged();
    }

    Component.onDestruction: {
        appMain.screenStateChanged.disconnect(slotScreenStateChanged);
    }

    function slotScreenStateChanged()
    {
        visible = (appMain.screenState === "ExamManager-InjectionExecution");
        reload();
    }

    function reload()
    {
        if (!visible)
        {
            return;
        }
    }
}
