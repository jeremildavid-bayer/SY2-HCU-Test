import QtQuick 2.12
import "../../Util.js" as Util
import "../../Widgets"

Item {
    property bool planIsDeparameterized: false
    property var activeAlerts: dsAlert.activeAlerts
    property string screenState: appMain.screenState
    property var executedSteps: dsExam.executedSteps
    property var injectionCount: (executedSteps === undefined) ? 0 : executedSteps.length
    property string pressureUnit: dsCfgGlobal.pressureUnit
    property string injectionsText
    property string pressureMaxText
    property string flowrateMaxText

    anchors.fill: parent
    visible: screenState === "ExamManager-SummaryConfirmation"

    ExamSummaryWarnings {
        id: examSummaryWarning
        width: middleFrameX - frameMargin
        anchors.left: parent.left
        anchors.leftMargin: frameMargin
        anchors.top: parent.top
        anchors.topMargin: parent.height * 0.06
        anchors.bottom: parent.bottom
    }

    Item {
        id: examSummaryView
        x: middleFrameX + frameMargin
        width: middleFrameWidth - (frameMargin * 2)
        height: parent.height

        Item {
            id: rectHeading
            anchors.top: parent.top
            anchors.topMargin: parent.height * 0.03
            height: parent.height * 0.06
            width: parent.width


            Rectangle {
                id: sharingInformationLogo
                anchors.left: parent.left
                anchors.verticalCenter: textPlanName.verticalCenter
                height: visible ? (textPlanName.font.pixelSize * 1.4) : 0
                width: height
                visible: examManager.isSharingInformation(plan)
                color: colorMap.white01
                radius: sharingInformationBtn.radius

                Image {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    height: textPlanName.font.pixelSize * 1.2
                    width: height
                    source: examManager.getSharingInformationIcon(plan)
                }
            }
            Text {
                id: textPlanName
                width: parent.width - sharingInformationLogo.width - (sharingInformationBtn.width * 1.2)
                height: parent.height
                anchors.left: sharingInformationLogo.right
                anchors.leftMargin: sharingInformationLogo.width * 0.2
                font.pixelSize: height * 0.85
                font.family: fontRobotoLight.name
                color: colorMap.text01
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
                fontSizeMode: Text.Fit
                minimumPixelSize: font.pixelSize * 0.9
                text: (plan !== undefined) ? plan.Name : ""
            }

            GenericButton {
                id: sharingInformationBtn
                anchors.verticalCenter: textPlanName.verticalCenter
                anchors.right: parent.right
                width: height
                height: sharingInformationLogo.height
                color: colorMap.keypadButton

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                    height: textPlanName.font.pixelSize
                    color:  colorMap.text01
                    font.family: fontIcon.name
                    font.pixelSize: height
                    text: "\ue93a"
                }

                onBtnClicked: {
                    examManager.sharingInformationPopup(plan);
                }
            }

        }

        Item {
            id: summaryArea
            anchors.top: rectHeading.bottom
            anchors.topMargin: parent.height * 0.02
            width: parent.width
            height : Math.max(parent.height * 0.06, summaryInjectionsText.contentHeight, summaryPressureMaxText.contentHeight, summaryFlowRateMaxText.contentHeight)

            // define fixed fontSize, otherwise changing height affects font size
            property int fontSize: dsCfgLocal.screenH * 0.023

            Row {
                id: summaryRow
                anchors.fill: parent
                spacing: parent.width * 0.025
                width: parent.width

                Text {
                    id: summaryInjectionsText
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    height : parent.height
                    width : parent.width * 0.3
                    text: translate(injectionsText)
                    font.pixelSize: summaryArea.fontSize
                    font.family: fontRobotoLight.name
                    color: colorMap.text02
                    verticalAlignment: Text.AlignTop
                    wrapMode: Text.Wrap;
                }

                Rectangle {
                    id: summaryTextDivider1
                    height : Math.max(summaryInjectionsText.contentHeight, summaryPressureMaxText.contentHeight, summaryFlowRateMaxText.contentHeight)
                    width: 2
                    color: colorMap.text02
                }

                Text {
                    id: summaryPressureMaxText
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    height : parent.height
                    width : parent.width * 0.3
                    text: translate(pressureMaxText + " " + translate("T_Units_" + pressureUnit))
                    font.pixelSize: summaryArea.fontSize
                    font.family: fontRobotoLight.name
                    color: colorMap.text02
                    verticalAlignment: Text.AlignTop
                    wrapMode: Text.Wrap;
                }

                Rectangle {
                    id: summaryTextDivider2
                    height : summaryTextDivider1.height
                    width: 2
                    color: colorMap.text02
                }

                Text {
                    id: summaryFlowRateMaxText
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    height : parent.height
                    width : parent.width * 0.3
                    text: translate(flowrateMaxText + " " + translate("T_Units_ml/s"))
                    font.pixelSize: summaryArea.fontSize
                    font.family: fontRobotoLight.name
                    color: colorMap.text02
                    verticalAlignment: Text.AlignTop
                    wrapMode: Text.Wrap;
                }
            }
        }

        ExamSummaryInjectedList {
            id: injectedList
            anchors.top: summaryArea.bottom
            anchors.topMargin: parent.height * 0.03
            width: parent.width
        }

        ExamSummaryAdvanceList {
            id: advanceList
            anchors.top: injectedList.bottom
            anchors.topMargin: parent.height * 0.03
            anchors.bottom: parent.bottom
            width: parent.width
        }
    }

    onVisibleChanged: {
        reload();
    }

    onActiveAlertsChanged: {
        reload();
    }

    function reload() {
        if (!visible)
        {
            return;
        }

        var activatedAlert = dsAlert.slotGetActiveAlertFromCodeName("DeparameterizedProtocol");
        planIsDeparameterized = (activatedAlert.GUID !== "00000000-0000-0000-0000-000000000000");

        var pressureKpaMax = 0;
        var flowRateMax = 0;

        for (var stepIdx = 0; stepIdx < injectionCount; stepIdx++)
        {
            for (var phaseIdx = 0; phaseIdx < executedSteps[stepIdx].PhaseProgress.length; phaseIdx++)
            {
                var phaseValue = executedSteps[stepIdx].PhaseProgress[phaseIdx];
                pressureKpaMax = Math.max(pressureKpaMax, phaseValue.MaxPressure);
                flowRateMax = Math.max(flowRateMax, phaseValue.MaxFlowRate);
            }
        }

        var pressureMax;
        if (pressureUnit == "kg/cm2")
        {
            pressureMax = localeToFloatStr(Util.getPressure(pressureUnit, pressureKpaMax), 1);
        }
        else
        {
            pressureMax = localeToFloatStr(Util.getPressure(pressureUnit, pressureKpaMax), 0);
        }

        injectionsText = (((injectionCount === 1) ? "T_XXX_Injection;" : "T_XXX_Injections;") + injectionCount.toString());
        pressureMaxText = ("T_XXX_Max;" + pressureMax.toString());
        flowrateMaxText = ("T_XXX_Max;" + localeToFloatStr(flowRateMax, 1));
    }
}
