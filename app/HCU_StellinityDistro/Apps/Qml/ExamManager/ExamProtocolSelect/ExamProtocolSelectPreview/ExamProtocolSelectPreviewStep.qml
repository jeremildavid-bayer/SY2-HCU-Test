import QtQuick 2.12
import "../../../Util.js" as Util
import "../../../Widgets"

Item {
    property var stepData: planPreview.Steps[index]
    property int remindersPerRow: 2

    width: ListView.view.width
    height: gridPostReminders.y + gridPostReminders.height

    Text {
        id: txtStepId
        width: parent.width * 0.07
        text: (index + 1)
        color: txtStepName.color
        height: txtStepName.height
        font.family: txtStepName.font.family
        font.pixelSize: txtStepName.font.pixelSize * 1.1
        verticalAlignment: Text.AlignVCenter
    }

    Image {
        id: personalizedImage
        anchors.left: txtStepId.right
        anchors.top: parent.top
        anchors.topMargin: 5
        source: {
            var containsModifier = false;
            var containsGenerator = false;
            var imgSource = imageMap.examProtocolPersonalised;

            for (var modifierIdx = 0 ; modifierIdx < stepData.PersonalizationModifiers.length ; modifierIdx++ )
            {
                if (stepData.PersonalizationModifiers[modifierIdx].includes("Tube Voltage Based Modifier"))
                {
                    containsModifier = true;
                    break;
                }
            }

            if (stepData.PersonalizationGenerator !== "")
            {
                containsGenerator = true;
            }

            if (containsModifier && containsGenerator)
            {
                imgSource = imageMap.examProtocolPersonalisedKVP;
            }
            else if (containsModifier)
            {
                imgSource = imageMap.examProtocolKVP;
            }
            else if (containsGenerator)
            {
                imgSource = imageMap.examProtocolPersonalised;
            }
            else
            {
                visible = false;
            }

            return imgSource;
        }
        width: height * 1.6
        height: visible ? txtStepId.height * 0.7 : 0
        sourceSize.width: width
        sourceSize.height: height
        visible: (planPreview === undefined) ? false : planPreview.IsPersonalized
    }

    Text {
        id: txtStepName
        anchors.left: personalizedImage.right
        anchors.leftMargin: 5
        anchors.right: iconIsNotScannerSynchronized.right
        anchors.rightMargin: parent.width * 0.02
        height: contentHeight
        text: stepData.Name
        color: colorMap.text01
        font.family: fontRobotoBold.name
        font.pixelSize: protocolSelectPreview.height * 0.04
        wrapMode: Text.Wrap
    }

    Text {
        id: iconIsNotScannerSynchronized
        anchors.right: parent.right
        anchors.rightMargin: parent.width * 0.02
        width: contentWidth
        height: txtStepName.height
        verticalAlignment: Text.AlignVCenter
        font.family: fontIcon.name
        color: txtStepName.color
        font.bold: true
        font.pixelSize: txtStepName.font.pixelSize * 1.3
        text: stepData.IsNotScannerSynchronized ? "\ue910" : ""
    }

    ExamProtocolSelectPreviewParamList {
        id: personalizedParamList
        width: parent.width * 0.96
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: txtStepName.bottom
        anchors.topMargin: protocolSelectPreview.height * 0.02
        paramItems: (stepData === undefined) ? [] : stepData.PersonalizationInputs
        title: "T_DefaultInjectionParameters"
    }

    ListView {
        id: listViewPhase
        anchors.top: personalizedParamList.bottom
        anchors.topMargin: (personalizedParamList.height == 0) ? 0 : protocolSelectPreview.height * 0.02
        height: (stepData.Phases.length * phaseHeight) + (spacing * (stepData.Phases.length - 1))
        width: parent.width * 0.96
        anchors.horizontalCenter: parent.horizontalCenter
        delegate: ExamProtocolSelectPreviewPhase {}
        interactive: false
        model: stepData.Phases
        spacing: phaseSpacing
    }

    Text {
        id: iconReminders
        anchors.right: gridReminders.left
        anchors.rightMargin: parent.width * 0.01
        visible: gridReminders.height > 0
        anchors.top: listViewPhase.bottom
        anchors.topMargin: rowSpacing * 2
        height: reminderRowHeight
        verticalAlignment: Text.AlignVCenter
        font.family: fontIcon.name
        font.pixelSize: height * 0.7
        color: colorMap.text01
        text: "\ue948"
    }

    GridView {
        id: gridReminders
        y: iconReminders.y
        width: parent.width * 0.85
        anchors.horizontalCenter: parent.horizontalCenter
        interactive: false
        cellHeight: reminderRowHeight
        cellWidth: width / remindersPerRow
        height: contentHeight

        delegate: Text {
            width: gridReminders.cellWidth
            height: gridReminders.cellHeight
            font.family: fontRobotoLight.name
            font.pixelSize: height * 0.9
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
            text: Util.getMinimisedDurationStr(gridReminders.model[index].PostStepTriggerDelay) + " " + gridReminders.model[index].Name
            color: colorMap.grid
        }
    }

    Text {
        id: iconPostReminders
        x: iconReminders.x
        visible: gridPostReminders.height > 0
        anchors.top: gridReminders.bottom
        height: reminderRowHeight
        verticalAlignment: Text.AlignVCenter
        font.family: fontIcon.name
        font.pixelSize: height * 0.75
        color: colorMap.text01
        text: "\ue926"
    }

    GridView {
        id: gridPostReminders
        x: gridReminders.x
        y: iconPostReminders.y
        width: gridReminders.width
        interactive: false
        cellHeight: reminderRowHeight
        cellWidth: width / remindersPerRow
        delegate: Text {
            width: gridReminders.cellWidth
            height: gridReminders.cellHeight
            font.family: fontRobotoLight.name
            font.pixelSize: height * 0.9
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
            text: Util.getMinimisedDurationStr(gridPostReminders.model[index].PostStepTriggerDelay) + " " + gridPostReminders.model[index].Name
            color: colorMap.grid
        }
    }

    Component.onCompleted: {
        stepData = planPreview.Steps[index];
    }

    onStepDataChanged: {
        updateReminders();
    }

    function updateReminders()
    {
        var reminderModel = [];
        var postReminderModel = [];

        for (var reminderIdx = 0; reminderIdx < stepData.Reminders.length; reminderIdx++)
        {
            if (stepData.Reminders[reminderIdx].StartAfterStepCompletes)
            {
                postReminderModel.push(stepData.Reminders[reminderIdx]);
            }
            else
            {
                reminderModel.push(stepData.Reminders[reminderIdx]);
            }
        }

        var reminderRowCounts = reminderModel.length / remindersPerRow + ((reminderModel.length % remindersPerRow > 0) ? 1 : 0);
        gridReminders.height = reminderRowCounts * reminderRowHeight;
        gridReminders.model = reminderModel;

        var postReminderRowCounts = postReminderModel.length / remindersPerRow + ((postReminderModel.length % remindersPerRow > 0) ? 1 : 0);
        gridPostReminders.height = postReminderRowCounts * reminderRowHeight;
        gridPostReminders.model = postReminderModel;
    }
}

