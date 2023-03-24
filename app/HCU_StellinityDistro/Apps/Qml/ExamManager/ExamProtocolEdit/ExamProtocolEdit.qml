import QtQuick 2.12
import "../../Widgets"
import "../../Widgets/Popup"

Item {
    anchors.fill: parent
    visible: false

    ExamProtocolEditPlan {
        id: injectionPlanEdit
        x: frameMargin
        height: parent.height
        width: parent.width - rightFrameWidth - frameMargin
    }

    Text {
        id: templateModified
        width: middleFrameX - frameMargin
        height: ((plan !== undefined) && plan.IsModifiedFromTemplate) ? contentHeight : 0
        anchors.top: parent.top
        anchors.topMargin: parent.height * 0.03
        anchors.left: parent.left
        anchors.leftMargin: frameMargin
        font.pixelSize: parent.height * 0.042
        font.family: fontRobotoMedium.name
        color: colorMap.text01
        wrapMode: Text.Wrap
        text: ((plan !== undefined) && plan.IsModifiedFromTemplate) ? translate("T_PlanIsModifiedFromTemplate") : ""
    }

    ExamProtocolEditWarnings {
        width: middleFrameX - frameMargin
        anchors.left: templateModified.left
        anchors.top: templateModified.bottom
        anchors.topMargin: parent.height * 0.03
        anchors.bottom: parent.bottom
    }

    ExamProtocolEditReview {
        id: injectionStepReview
        widthMinimised: middleFrameX
        widthMaximised: parent.width * 0.86
        height: parent.height
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
            injectionStepReview.close();
            return;
        }
    }

    function openLastStepReview()
    {
        injectionStepReview.openLast();
    }
}
