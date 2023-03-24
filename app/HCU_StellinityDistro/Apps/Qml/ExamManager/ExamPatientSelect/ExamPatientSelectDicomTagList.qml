import QtQuick 2.12
import "../../Widgets"
import "../../Util.js" as Util

Item {
    id: dicomTagList
    property string worklistDetailsType
    property var examAdvanceInfo: dsExam.examAdvanceInfo
    property var dicomTagsData: dicomHelper.getDicomTagsData(worklistDetailsType);
    property int rowHeight: height * 0.16
    property int listLeftOffset: 0

    property bool useDrawerColors: false

    anchors.top: parent.top
    anchors.left: parent.left

    ListView {
        id: listView
        anchors.left: parent.left
        anchors.leftMargin: listLeftOffset
        anchors.top: parent.top
        height: parent.height
        width: parent.width - listLeftOffset
        orientation: ListView.Vertical
        clip: true
        interactive: true
        delegate: ExamPatientSelectDicomTagListItem { }
        model: dicomTagsData
        cacheBuffer: Math.max(contentHeight * 2, height * 10)
        ScrollBar {
            color: useDrawerColors ? colorMap.text02 : colorMap.scrollBar
        }
        ListFade {
            fadeColor: useDrawerColors ? colorMap.subPanelBackground : colorMap.mainBackground
        }
    }
}
