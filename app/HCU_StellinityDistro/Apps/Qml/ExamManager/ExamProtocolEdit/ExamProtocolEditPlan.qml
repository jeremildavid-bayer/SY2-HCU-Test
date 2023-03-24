import QtQuick 2.12
import "../../Widgets"
import "../../Util.js" as Util

Item {
    property var plan: dsExam.plan
    property int stepCountMax: dsCapabilities.stepCountMax
    property var curEditParamInfo: { "State": "IDLE" } // State: IDLE, INSERTING_NEW_STEP, INSERTING_NEW_PHASE, SORTING_STEP, SORTING_PHASE, EDITING_PHASE_PARAM, EDITING_PROT_PARAM
    property string lastPlanGuid: ""
    property alias stepList: stepList

    // signal to other qml
    signal signalStepReload(int stepIdx)
    signal signalPhaseReload(int stepIdx, int phaseIdx)
    signal signalResumeEditProtParam()

    id: injectionPlanEdit
    visible: false

    Column {
        id: columnMain
        x: middleFrameX
        width: middleFrameWidth - frameMargin
        height: parent.height

        Item {
            height: parent.height * 0.1
            width: columnMain.width - frameMargin

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
                height: parent.height
                anchors.left: sharingInformationLogo.right
                anchors.leftMargin: sharingInformationLogo.width * 0.2
                anchors.right: sharingInformationBtn.left
                color: colorMap.text01
                font.pixelSize: height * 0.5
                font.family: fontRobotoLight.name
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }

            Text {
                id: textPlanNameStar
                height: parent.height
                anchors.left: textPlanName.right
                anchors.leftMargin: 5
                text: ""
                color: colorMap.text01
                font.pixelSize: height * 0.65
                font.family: fontRobotoLight.name
                verticalAlignment: Text.AlignVCenter
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

        ExamProtocolEditStepList {
            id: stepList
            height: parent.height - y
            width: parent.width
        }
    }

    ExamProtocolEditOptionList {
        id: editOptionList
        width: rightFrameWidth
        height: parent.height
    }

    DragRectangle {
        id: dragObject
        visible: false
    }

    onPlanChanged: {
        if (plan === undefined)
        {
            return;
        }

        reload();

        if ( (stepList.planEditData === undefined) ||
             (!Util.compareObjects(plan, stepList.planEditData)) )
        {
            var lastEditParamInfo = Util.copyObject(curEditParamInfo);
            var editResumeRequired = false;

            if (widgetInputPad.isOpen())
            {
                if (lastPlanGuid !== plan.GUID)
                {
                    // Plan GUID changed. Closing current input pad
                    widgetInputPad.close(false);
                }
                else if ( (curEditParamInfo.State === "EDITING_PROT_PARAM") ||
                          (curEditParamInfo.State === "EDITING_PHASE_PARAM") )
                {
                    // Protocol Parameter is current edited by HCU, don't close the inputPad
                    widgetInputPad.emitSignalClosed(false);
                    editResumeRequired = true;
                }
                else
                {
                    // Steps are changed from other (e.g. CRU). Closing current input pad
                    widgetInputPad.close(false);
                }
            }

            setPlanEditData(plan);

            if (editResumeRequired)
            {
                setCurEditParamInfo(lastEditParamInfo);
                signalResumeEditProtParam();
            }
        }
        lastPlanGuid = plan.GUID;
    }

    function setPlanEditData(plan)
    {
        stepList.planEditData = Util.copyObject(plan);
    }

    function savePlan(newPlan)
    {
        dsExam.slotPlanChanged(newPlan);
    }

    function getCurEditParamInfo()
    {
        return curEditParamInfo;
    }

    function setCurEditParamInfo(newCurEditParamInfo)
    {
        curEditParamInfo = newCurEditParamInfo;
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
        visible = (appMain.screenState === "ExamManager-ProtocolModification");
        reload();
    }

    function reload()
    {
        if (!visible)
        {
            editOptionList.state = "CLOSED";
            return;
        }

        textPlanName.text = plan.Name
        textPlanNameStar.text = plan.IsModifiedFromTemplate ? "*" : "";
    }

    function reloadAllSteps()
    {
        stepList.reloadAll();
    }
}
