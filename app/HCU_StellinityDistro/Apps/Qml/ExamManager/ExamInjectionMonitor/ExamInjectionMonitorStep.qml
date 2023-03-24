import QtQuick 2.12
import QtGraphicalEffects 1.12
import "../../Widgets"

Item {
    property var plan: dsExam.plan
    property var stepProgressDigest: dsExam.stepProgressDigest
    property var executingStep: dsExam.executingStep
    property int planNameHeight: height * 0.05
    property int activeStepHeight: height * 0.11
    property int inactiveStepHeight: height * 0.07
    property int activePhaseHeight: height * 0.1
    property int inactivePhaseHeight: height * 0.09
    property int rowSpacing: parent.height * 0.02
    property var nextSteps

    id: injectionProgressInfo
    visible: false

    Rectangle {
        id: sharingInformationLogo
        anchors.left: parent.left
        anchors.verticalCenter: txtPlanName.verticalCenter
        height: visible ? (txtPlanName.font.pixelSize * 1.4) : 0
        width: height
        visible: examManager.isSharingInformation(plan)
        color: colorMap.white01
        radius: 10

        Image {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            height: txtPlanName.font.pixelSize * 1.2
            width: height
            source: examManager.getSharingInformationIcon(plan)
        }
    }

    Text {
        id: txtPlanName
        anchors.top: parent.top
        anchors.topMargin: parent.height * 0.02
        anchors.left: sharingInformationLogo.right
        anchors.leftMargin: sharingInformationLogo.width * 0.2
        anchors.right: parent.right
        anchors.rightMargin: parent.width * 0.05
        height: planNameHeight
        verticalAlignment: Text.AlignVCenter
        color: colorMap.text01
        font.family: fontRobotoBold.name
        font.pixelSize: height * 0.9
        text: (plan !== undefined) ? plan.Name : ""
        elide: Text.ElideRight
    }

    Text {
        id: txtLastStepName
        anchors.top: txtPlanName.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.rightMargin: parent.width * 0.05
        height: ((executingStep !== undefined) && (executingStep.Index > 0)) ? inactiveStepHeight : 0
        verticalAlignment: Text.AlignVCenter
        color: colorMap.gry01
        font.family: fontRobotoMedium.name
        font.pixelSize: activeStepHeight * 0.38
        text: ((executingStep !== undefined) && (executingStep.Index > 0) && (plan.Steps[executingStep.Index - 1] !== undefined)) ? (executingStep.Index + "   " + plan.Steps[executingStep.Index - 1].Name) : ""
        elide: Text.ElideRight
    }

    Text {
        id: txtStepName
        anchors.top: txtLastStepName.bottom
        anchors.right: iconIsNotScannerSynchronized.left
        anchors.rightMargin: parent.width * 0.02
        anchors.left: parent.left
        height: activeStepHeight
        text: (executingStep !== undefined) ? ((executingStep.Index + 1) + "   " + executingStep.Name) : ""
        color: colorMap.text01
        font.family: fontRobotoBold.name
        font.pixelSize: activeStepHeight * 0.4
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    Text {
        id: iconIsPreloaded
        anchors.top: txtLastStepName.bottom
        anchors.topMargin: txtStepName.height * 0.06
        anchors.right: iconIsNotScannerSynchronized.left
        anchors.rightMargin: parent.width * 0.02
        width: contentWidth
        height: txtStepName.height - (txtStepName.height * 0.02)
        verticalAlignment: Text.AlignVCenter
        font.family: fontIcon.name
        color: colorMap.text01
        font.bold: true
        font.pixelSize: height * 0.4
        text: (executingStep !== undefined) && (executingStep.IsPreloaded) ? "\ue983" : ""
    }

    Text {
        id: iconIsNotScannerSynchronized
        anchors.top: txtLastStepName.bottom
        anchors.topMargin: txtStepName.height * 0.02
        anchors.right: parent.right
        anchors.rightMargin: parent.width * 0.05
        width: contentWidth
        height: txtStepName.height - anchors.topMargin
        verticalAlignment: Text.AlignVCenter
        font.family: fontIcon.name
        color: colorMap.text01
        font.bold: true
        font.pixelSize: height * 0.5
        text: (executingStep !== undefined) && (executingStep.IsNotScannerSynchronized) ? "\ue910" : ""
    }

    ListView {
        id: listViewPhase
        anchors.top: txtStepName.bottom
        height: 0
        width: parent.width * 0.95
        spacing: rowSpacing
        delegate: ExamInjectionMonitorPhase {}
        interactive: false
    }

    ListView {
        id: listViewNextSteps
        clip: true
        width: parent.width
        anchors.top: listViewPhase.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.rightMargin: parent.width * 0.05
        interactive: false
        delegate: Text {
            height: inactiveStepHeight
            width: ListView.view.width
            verticalAlignment: Text.AlignVCenter
            color: colorMap.gry01
            font.family: fontRobotoBold.name
            font.pixelSize: height * 0.6
            text: (nextSteps[index] !== undefined) ? ((executingStep.Index + 2 + index) + "   " + nextSteps[index].Name) : ""
            elide: Text.ElideRight
        }
    }

    Item {
        id: topFade
        anchors.top: txtPlanName.bottom
        width: parent.width
        height: parent.height * 0.15
        visible: (executingStep !== undefined) && (executingStep.Index > 1)
        LinearGradient {
            anchors.fill: parent
            start: Qt.point(0, 0)
            end: Qt.point(0, height)

            gradient: Gradient {
                GradientStop { position: 0.0; color: colorMap.mainBackground }
                GradientStop { position: 1.0; color: "#00000000" }
            }
        }
    }

    Item {
        id: bottomFade
        width: parent.width
        y: parent.height - height
        height: parent.height * 0.15
        visible: (listViewNextSteps.contentHeight > listViewNextSteps.height)
        LinearGradient {
            anchors.fill: parent
            start: Qt.point(0, 0)
            end: Qt.point(0, height)

            gradient: Gradient {
                GradientStop { position: 0.0; color: "#00000000" }
                GradientStop { position: 1.0; color: colorMap.mainBackground }
            }
        }
    }

    onStepProgressDigestChanged: {
        reloadPhases();
    }

    Component.onCompleted: {
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

    function reloadPhases()
    {
        if (!visible)
        {
            return;
        }

        if ( (executingStep.Phases !== undefined) &&
             (JSON.stringify(listViewPhase.model) != JSON.stringify(executingStep.Phases)) )
        {
            listViewPhase.model = executingStep.Phases;
        }
    }

    function reload()
    {
        if (!visible)
        {
            return;
        }

        // Update next steps
        var nextStepsBuf = [];
        for (var nextStepIdx = executingStep.Index + 1; nextStepIdx < plan.Steps.length; nextStepIdx++)
        {
            nextStepsBuf.push(plan.Steps[nextStepIdx]);
        }

        nextSteps = nextStepsBuf;
        if (JSON.stringify(listViewNextSteps.model) != JSON.stringify(nextSteps))
        {
            listViewNextSteps.model = nextSteps;
        }

        if (executingStep.Phases !== undefined)
        {
            listViewPhase.height = ((executingStep.Phases.length - 1) * inactivePhaseHeight) + activePhaseHeight;
            listViewPhase.height += (rowSpacing * executingStep.Phases.length);
        }
        reloadPhases();
    }
}
