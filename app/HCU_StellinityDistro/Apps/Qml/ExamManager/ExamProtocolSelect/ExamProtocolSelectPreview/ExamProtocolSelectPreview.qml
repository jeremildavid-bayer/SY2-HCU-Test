import QtQuick 2.12
import "../../../Widgets"

Item {
    property var planPreview: dsExam.planPreview
    property var selectedContrast: dsExam.selectedContrast
    property int phaseHeight: height * 0.1
    property int rowSpacing: height * 0.007
    property int phaseSpacing: height * 0.01
    property int bottomSpacing: height * 0.03
    property int reminderRowHeight: height * 0.035

    id: protocolSelectPreview
    visible: false

   Rectangle {
       id: sharingInformationLogo
       anchors.left: parent.left
       anchors.verticalCenter: textPlanName.verticalCenter
       height: visible ? (textPlanName.font.pixelSize * 1.4) : 0
       width: height
       visible: examManager.isSharingInformation(planPreview)
       color: colorMap.white01
       radius: sharingInformationBtn.radius

       Image {
           anchors.horizontalCenter: parent.horizontalCenter
           anchors.verticalCenter: parent.verticalCenter
           height: textPlanName.font.pixelSize * 1.2
           width: height
           source: examManager.getSharingInformationIcon(planPreview)
       }
   }
    Text {
        id: textPlanName
        anchors.top: parent.top
        anchors.topMargin: parent.height * 0.021
        anchors.left: sharingInformationLogo.right
        anchors.leftMargin: sharingInformationLogo.width * 0.2
        anchors.right: sharingInformationBtn.left
        height: contentHeight
        text: (planPreview === undefined) ? "" : planPreview.Name
        color: colorMap.text01
        font.pixelSize: parent.height * 0.05
        font.family: fontRobotoLight.name
        wrapMode: Text.Wrap
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
            examManager.sharingInformationPopup(planPreview);
        }
    }

    ListView {
        id: listViewPlan
        anchors.top: textPlanName.bottom
        anchors.topMargin: parent.height * 0.02
        clip: true
        anchors.bottom: parent.bottom
        width: parent.width
        spacing: bottomSpacing
        contentY: 0 - headerItem.height
        delegate: ExamProtocolSelectPreviewStep {}

        header: Item {
            width: parent.width
            height: headerPadding.y + headerPadding.height

            Text {
                id: injectionPlanDescription
                width: parent.width
                height: (text == "") ? 0 : contentHeight
                text: (planPreview === undefined) ? "" : planPreview.Description
                color: colorMap.text02
                font.pixelSize: protocolSelectPreview.height * 0.035
                font.family: fontRobotoLight.name
                wrapMode: Text.Wrap
            }

            ExamProtocolSelectPreviewParamList {
                id: personalizedParamList
                width: parent.width
                anchors.top: injectionPlanDescription.bottom
                anchors.topMargin: (injectionPlanDescription.text == "") ? 0 : (protocolSelectPreview.height * 0.02)
                paramItems: (planPreview === undefined) ? [] : planPreview.PersonalizationInputs
                title: "T_DefaultProtocolParameters"
            }

            Item {
                id: headerPadding
                anchors.top: personalizedParamList.bottom
                width: parent.width
                height: protocolSelectPreview.height * 0.05
            }
        }

        footer: Item {
            width: parent.width
            height: protocolSelectPreview.height * 0.05
        }

        ListFade {}
        ScrollBar {}

        GenericButton {
            anchors.fill: parent
            interactive: (executedSteps.length == 0)
            Component.onCompleted: {
                listViewPlan.dragStarted.connect(reset);
            }

            Component.onDestruction: {
                listViewPlan.dragStarted.disconnect(reset);
            }

            onBtnClicked: {
                navigationBar.signalNavNext();
            }
        }
    }

    onPlanPreviewChanged: {
        reload();
    }

    onSelectedContrastChanged: {
        reload();
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
        visible = (appMain.screenState.indexOf("ExamManager-ProtocolSelection") >= 0);
        reload();
    }

    function reload()
    {
        if (!visible)
        {
            return;
        }

        if (planPreview === undefined)
        {
            return;
        }

        //logDebug("planPreview=" + JSON.stringify(planPreview));

        if (JSON.stringify(listViewPlan.model) != JSON.stringify(planPreview.Steps))
        {
            //logDebug("ExamProtocolSelectPreview: Template model changed");
            listViewPlan.model = planPreview.Steps;
        }
    }
}

