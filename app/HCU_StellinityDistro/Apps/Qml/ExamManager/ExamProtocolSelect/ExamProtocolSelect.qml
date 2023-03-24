import QtQuick 2.12
import "ExamProtocolSelectList"
import "ExamProtocolSelectPreview"
import "../ExamProtocolOverview"

Item {
    property var plan: dsExam.plan
    //property var planPreview: dsExam.planPreview
    property string examProgressState: dsExam.examProgressState
    property var personalizationGenerators: {
        var newPersonalizationGenerators = [];
        if (planPreview === undefined)
        {
            return newPersonalizationGenerators;
        }

        for (var stepIdx = 0; stepIdx < dsExam.planPreview.Steps.length; stepIdx++)
        {
            var key = dsExam.planPreview.Steps[stepIdx].PersonalizationGenerator;
            if (key === "")
            {
                continue;
            }

            var keyInserted = false;
            for (var rowIdx = 0; rowIdx < newPersonalizationGenerators.length; rowIdx++)
            {
                if (newPersonalizationGenerators[rowIdx] === key)
                {
                    keyInserted = true;
                    break;
                }
            }
            if (!keyInserted)
            {
                newPersonalizationGenerators.push(key);
            }
        }
        return newPersonalizationGenerators;
    }
    property var personalizationModifiers: {
        var newPersonalizationModifiers = [];
        if (dsExam.planPreview === undefined)
        {
            return newPersonalizationModifiers;
        }

        for (var stepIdx = 0; stepIdx < dsExam.planPreview.Steps.length; stepIdx++)
        {
            var keyInserted = false;
            if (dsExam.planPreview.Steps[stepIdx].PersonalizationModifiers.length === 0)
            {
                continue;
            }

            for (var modifierIdx = 0; modifierIdx < dsExam.planPreview.Steps[stepIdx].PersonalizationModifiers.length; modifierIdx ++)
            {
                var key = dsExam.planPreview.Steps[stepIdx].PersonalizationModifiers[modifierIdx];
                if (key === "")
                {
                    continue;
                }

                for (var rowIdx = 0; rowIdx < newPersonalizationModifiers.length; rowIdx++)
                {
                    if (newPersonalizationModifiers[rowIdx] === key)
                    {
                        keyInserted = true;
                        break;
                    }
                }
                if (!keyInserted)
                {
                    newPersonalizationModifiers.push(key);
                }
            }
        }
        return newPersonalizationModifiers;
    }

    id: injectionProtocolSelect
    anchors.fill: parent

    ExamProtocolSelectList {
        id: protocolSelectList
        x: frameMargin
        width: leftFrameWidth - (frameMargin * 2)
        height: parent.height
    }

    Rectangle {
        // separator line
        anchors.left: protocolSelectList.right
        anchors.leftMargin: (frameMargin / 2)
        width: frameMargin / 6
        height: parent.height
        color: colorMap.titleBarBackground
    }

    ExamProtocolSelectPreview {
        x: middleFrameX + frameMargin
        width: middleFrameWidth - (frameMargin * 2)
        height: parent.height
    }

    Item {
        id: personalizationsFrame
        y: parent.height * 0.02
        anchors.right: parent.right
        width: rightFrameWidth - frameMargin
        height: parent.height * 0.5

        Text {
            id: personalizationsTitle
            width: parent.width * 0.9
            height: contentHeight
            font.family: fontRobotoBold.name
            font.pixelSize: parent.height * 0.1
            color: colorMap.text01
            wrapMode: Text.Wrap
            text: translate("T_Personalizations")
            visible: ( (personalizationGenerators.length !== 0) || (personalizationModifiers.length !== 0) )
        }

        ListView {
            id: personalizationsItems
            anchors.top: personalizationsTitle.bottom
            anchors.topMargin: parent.height * 0.01
            interactive: false
            height: personalizationGenerators.length === 0 ? 0 : contentHeight
            width: parent.width * 0.9
            model: personalizationGenerators
            spacing: parent.height * 0.015
            delegate: Text {
                width: ListView.view.width
                color: colorMap.text02
                font.family: fontRobotoLight.name
                font.pixelSize: personalizationsFrame.height * 0.08
                text: translate(personalizationGenerators[index])
                wrapMode: Text.Wrap
            }
        }

        ListView {
            id: personalizationsItemsContinued
            anchors.top: personalizationsItems.bottom
            anchors.topMargin: parent.height * 0.01
            interactive: false
            width: parent.width * 0.9
            model: personalizationModifiers
            spacing: parent.height * 0.015
            delegate: Text {
                width: ListView.view.width
                color: colorMap.text02
                font.family: fontRobotoLight.name
                font.pixelSize: personalizationsFrame.height * 0.08
                wrapMode: Text.Wrap
                text:
                {
                    var rawStr = personalizationModifiers[index].split(";");
                    return translate("T_"+rawStr[1].replace(/ /g,""));
                }
            }
        }
    }

    ExamProtocolOverviewInner {
        width: rightFrameWidth - frameMargin
        height: parent.height * 0.23
        anchors.bottom: parent.bottom
        anchors.bottomMargin: parent.height * 0.01
        anchors.right: parent.right
        selectedPlan: dsExam.planPreview
    }

    onExamProgressStateChanged: {
        if (appMain.screenState === "OFF")
        {
            return;
        }

        if (examProgressState === "Idle")
        {
            dsExam.slotLoadPlanTemplateFromPlan(plan);
        }
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
        visible = (appMain.screenState === "ExamManager-ProtocolSelection");
    }

}
