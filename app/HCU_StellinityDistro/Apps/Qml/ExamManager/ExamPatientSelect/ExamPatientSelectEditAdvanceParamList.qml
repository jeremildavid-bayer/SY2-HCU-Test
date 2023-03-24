import QtQuick 2.12
import "../../Widgets"
import "../../Util.js" as Util
import "ExamPatientSelectEditAdvanceParamListItemEgfr"

Item {
    property var examAdvanceInfo: dsExam.examAdvanceInfo
    property string examProgressState: dsExam.examProgressState
    property var examInputs: (examAdvanceInfo !== undefined) ? examAdvanceInfo.ExamInputs : undefined
    property var lastExamInputs
    property var paramItems: []
    property var curEditParamInfo: { "State": "IDLE" } // State: IDLE, EDITING_ADVANCE_PARAM, EGFR_PANEL_OPEN

    property var catheterTypeItemComponent
    property var catheterTypeItemObjects: []

    property var eGFRTypeItem: []

    signal signalResumeEditAdvanceParam()

    property bool interactive: true

    id: root

    ListView {
        
        ListFade {}
        ScrollBar {}

        id: listViewAdvanceParamsList
        clip: true
        model: paramItems
        anchors.fill: parent
        delegate: ExamPatientSelectEditAdvanceParamListItem { }
        header: ExamPatientSelectEditAdvanceParamLinkOtherAccessionList {}

    }

    EgfrDrawer {
        id: egfrPanel
        width: appMainView.width * 0.715
        height: parent.height
        edgeDragEnabled: false
        type: "RIGHT"
        newParent: examPatientSelect

        onVisibleChanged:
        {
            if (!visible)
            {
                close();
            }
        }
    }

    onExamProgressStateChanged: {
        // CRU can start exam while eGFR flyout is open. force close in this case
        if (examProgressState == "Started")
        {
            egfrPanel.close();
        }
    }

    onExamInputsChanged: {
        if (JSON.stringify(examInputs) === JSON.stringify(lastExamInputs))
        {
            return;
        }

        // Set paramItems from new examInputs
        var newParamItems = [];

        if (examInputs["PatientHeight"] !== undefined)
        {
            newParamItems.push(examInputs["PatientHeight"]);
        }
        if (examInputs["PatientWeight"] !== undefined)
        {
            newParamItems.push(examInputs["PatientWeight"]);
        }
        if (examInputs["CatheterType"] !== undefined)
        {
            newParamItems.push(examInputs["CatheterType"]);

            // Update catheterTypeItemObjects
            var validList = examInputs["CatheterType"].ValidList;
            for (var itemIdx = 0; itemIdx < validList.length; itemIdx++)
            {
                if (catheterTypeItemObjects.length > itemIdx)
                {
                    catheterTypeItemObjects[itemIdx].dataValue = JSON.parse(validList[itemIdx]);
                    catheterTypeItemObjects[itemIdx].visible = false;
                }
                else
                {
                    var catheterTypeItemObject = catheterTypeItemComponent.createObject(this, {interactive : root.interactive, alignment: Text.AlignLeft});
                    catheterTypeItemObject.dataValue = JSON.parse(validList[itemIdx]);
                    catheterTypeItemObject.visible = false;
                    catheterTypeItemObjects.push(catheterTypeItemObject);
                }
            }

            // Clear catheterTypeItemObjects
            for (var catheterTypeIdx = validList.length; catheterTypeIdx.length; catheterTypeIdx++)
            {
                catheterTypeItemObjects[catheterTypeIdx].destroy();
            }
        }
        if (examInputs["CatheterPlacement"] !== undefined)
        {
            newParamItems.push(examInputs["CatheterPlacement"]);
        }
        if (examInputs["EgfrValue"] !== undefined)
        {
            newParamItems.push(examInputs["EgfrValue"]);
            eGFRTypeItem = examInputs["EgfrValue"];
            eGFRTypeItem.Value = JSON.parse(eGFRTypeItem.Value);
            egfrPanel.egfrItem = eGFRTypeItem;
        }
        
        // If Edit was in progress, handle last edit.
        var lastEditParamInfo = Util.copyObject(curEditParamInfo);
        var editResumeRequired = false;
        if (widgetInputPad.isOpen())
        {
            if (curEditParamInfo.State === "EDITING_ADVANCE_PARAM")
            {
                // Advance Parameter is current edited by HCU, don't close the inputPad
                widgetInputPad.emitSignalClosed(false);
                editResumeRequired = true;
            }
        }

        // Update UI
        paramItems = newParamItems;

        lastExamInputs = Util.copyObject(examInputs);
        
        // Resume edit if required
        if (editResumeRequired)
        {
            logDebug("ExamPatientSelectEditAdvanceParamList: Resume editing: lastEditParamInfo=" + JSON.stringify(lastEditParamInfo));
            setCurEditParamInfo(lastEditParamInfo);
            signalResumeEditAdvanceParam();
        }

        logDebug("ExamPatientSelectEditAdvanceParamList: model changed");
        //logDebug("\nExamPatientSelectEditAdvanceParamList: model changed = " + JSON.stringify(paramItems) + "\n");
    }

    Component.onCompleted: {
        catheterTypeItemComponent = Qt.createComponent("ExamPatientSelectEditAdvanceParamListItemCatheterType.qml");
    }

    function getCurEditParamInfo()
    {
        return curEditParamInfo;
    }

    function setCurEditParamInfo(newCurEditParamInfo)
    {
        curEditParamInfo = newCurEditParamInfo;
    }
}
