import QtQuick 2.3

Rectangle {
    property bool devMode: true

    property int optionItemW: 500
    property int optionItemH: 100
    property int optionListSpacing: -5
    property int optionItemPullAnimationMs: 100

    property int stepItemW: 800
    property int stepHeaderItemH: 100
    property int stepListViewFooterHeight: stepHeaderItemH * 1.5

    property int phaseItemW: 700
    property int phaseItemH: stepHeaderItemH
    property int phaseLeftMargin: stepItemW - phaseItemW
    property int phaseListRowSpacing: 4

    property int dragHandleMargin: 5
    property int dragHandleWidth: 50
    property int dragHandleHeight: 40

    property int scrollAreaHeight: stepHeaderItemH / 2
    property int scrollSpeed: 500

    signal signalRemoveDummyPhase();
    signal signalSetDummyPhase(int rowIdx, string stepId);
    signal signalInsertPhaseFromDummy(string titleColor);
    signal signalInsertStepFromDummy(string titleColor);
    signal signalSwapSteps(int from, int to);
    signal signalSwapRows(int from, int to);
    signal signalSetStepIdForPhase(int rowIdx, string stepId);

    objectName: "mainScreen"
    id: mainScreen
    anchors.fill: parent


    DragObject {
        id: dragObject
        z: 1
    }

    OptionList {
        id: optionList
        anchors.left: parent.left
        width: optionItemW
        height: parent.height
    }

    StepList {
        id: stepList
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: stepItemW
        height: parent.height
    }

    function slotListPhaseValuesChanged(listPhaseValues)
    {
        stepList.setListModel(listPhaseValues);
    }

}
