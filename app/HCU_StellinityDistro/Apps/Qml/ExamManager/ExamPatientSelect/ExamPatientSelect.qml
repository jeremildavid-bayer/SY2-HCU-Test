import QtQuick 2.12
import "../../Widgets"
import "../../Util.js" as Util

Item {
    property string screenState: appMain.screenState
    // only enable workliseEntries if license is enabled
    property bool licenseEnabledWorklistSelection: dsCru.licenseEnabledWorklistSelection
    property var worklistEntries: licenseEnabledWorklistSelection ? dsMwl.worklistEntries : []
    property var anonymousPatientData
    property var worklistEntriesSorted
    property int workItemSeletedIndex: -1
    property int rowHeight: parent.height * 0.12
    property int rowHeightMin: rowHeight * 0.7
    property int colSpacing: 5

    property double headerColWidthPercentStudyDateTime: 0.11
    property double headerColWidthPercentPatientName: 0.21
    property double headerColWidthPercentPatientSex: 0.07
    property double headerColWidthPercentPatientBirthDate: 0.09
    property double headerColWidthPercentPatientId: 0.14
    property double headerColWidthPercentAccessionNumber: 0.14
    property double headerColWidthPercentStudyDescription: 0.24
    property string curSortType: "studyDateTime-Up"
    property var examAdvanceInfo: dsExam.examAdvanceInfo
    property bool isExamStarted: dsExam.isExamStarted
    property bool isPatientSelected: dsExam.isPatientSelected
    property var cruLinkStatus: dsCru.cruLinkStatus
    property int animationMs: 500
    property string searchStr: ""

    id: examPatientSelect
    anchors.fill: parent

    state: "UNKNOWN"
    states: [
        State { name: "SELECTING" },
        State { name: "EDITING" }
    ]

    transitions: [
        Transition {
            to: "EDITING"
            SequentialAnimation {
                ScriptAction { script: {
                        // TODO: Locate patientSelectedItem to currently selected row.
                        patientEditWidgets.opacity = 0;
                        patientEditWidgets.visible = true;
                        patientSelectedItem.visible = true;
                        patientSelectedItem.highlightPercent = 0;
                        listViewAdvanceParams.height = 0;
                    }
                }

                ParallelAnimation {
                    NumberAnimation { target: patientSelectWidgets; properties: 'opacity'; to: 0; duration: animationMs }
                    NumberAnimation { target: patientEditWidgets; properties: 'opacity'; to: 1; duration: animationMs }
                    NumberAnimation { target: patientSelectedItem; properties: 'y'; to: rowHeightMin; duration: animationMs }
                }

                ParallelAnimation {
                      NumberAnimation { target: patientSelectedItem; properties: 'highlightPercent'; to: 1; duration: animationMs }
                      NumberAnimation { target: listViewAdvanceParams; properties: 'height'; to: patientEditWidgets.height; duration: animationMs }
                }

                ScriptAction { script: {
                        patientSelectWidgets.visible = false;
                    }
                }
            }
        },
        Transition {
            to: "SELECTING"
            SequentialAnimation {
                ScriptAction { script: {
                        patientSelectWidgets.opacity = 0;
                        patientSelectWidgets.visible = true;
                    }
                }

                ParallelAnimation {
                      NumberAnimation { target: patientSelectedItem; properties: 'highlightPercent'; to: 0; duration: animationMs }
                      NumberAnimation { target: listViewAdvanceParams; properties: 'height'; to: 0; duration: animationMs }
                }

                ParallelAnimation {
                    NumberAnimation { target: patientSelectWidgets; properties: 'opacity'; to: 1; duration: animationMs }
                    NumberAnimation { target: patientEditWidgets; properties: 'opacity'; to: 0; duration: animationMs }
                }

                ScriptAction { script: {
                        patientSelectedItem.visible = false;
                        patientEditWidgets.visible = false;
                    }
                }
            }
        }
    ]

    DicomHelper {
        id: dicomHelper
    }

    ExamPatientSelectListHeader {
        id: patientSelectListHeader
        height: rowHeight * 0.7
    }

    Item {
        id: patientSelectWidgets
        anchors.top: patientSelectListHeader.bottom
        anchors.bottom: parent.bottom
        width: parent.width

        ExamPatientSelectSearch {
            totalRows: (patientSelectListView.model !== undefined) ? patientSelectListView.model.length : 0
            searchStrFound: patientSelectListView.searchStrFoundCount
        }

        ExamPatientSelectListItem {
            id: anonymousPatientRow
            width: parent.width
            rowData: anonymousPatientData
            isListItem: false
        }

        Rectangle {
            id: headerBorder
            anchors.top: anonymousPatientRow.bottom
            color: colorMap.text01
            width: parent.width
            height: 4
        }

        ExamPatientSelectList {
            id: patientSelectListView
            width: parent.width
            anchors.top: headerBorder.bottom
            anchors.bottom: parent.bottom
        }
    }

    ExamPatientSelectedItem {
        id: patientSelectedItem
        visible: false // should be hidden during startup
        interactive: (!isExamStarted) && (listViewAdvanceParams.getCurEditParamInfo().State === "IDLE") && licenseEnabledWorklistSelection
    }

    Item {
        id: rectCruDiconnectedWarning
        width: parent.width * 0.7
        height: parent.height * 0.5
        anchors.centerIn: parent
        visible: blurBackground.visible

        WarningIcon {
            id: iconActiveAlert
            width: parent.width * 0.05
            height: width
            anchors.right: txtAlert.left
            anchors.rightMargin: (-(txtAlert.width - txtAlert.contentWidth) * 0.5) + (width * 0.25)
            anchors.top: parent.top
            anchors.topMargin: -height * 0.11
        }

        Text {
            id: txtAlert
            width: parent.width
            height: parent.height
            text: translate("T_ExamNotAvailableCruDisconnected")
            font.pixelSize: parent.height * 0.12
            font.family: fontRobotoBold.name
            color: colorMap.text01
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
        }
    }

    Item {
        id: patientEditWidgets
        anchors.top: patientSelectedItem.bottom
        anchors.bottom: parent.bottom
        width: parent.width
        opacity: 0
        visible: false

        ExamPatientSelectDicomTagList {
            id: dicomTagsSummary
            worklistDetailsType: "Panel"
            width: leftFrameWidth + frameMargin - separatorLine.width
            height: parent.height
            listLeftOffset: frameMargin
        }

        Rectangle {
            // separator line
            id: separatorLine
            anchors.right: listViewAdvanceParams.left
            width: frameMargin / 6
            height: parent.height
            color: colorMap.titleBarBackground
        }

        ExamPatientSelectEditAdvanceParamList {
            id: listViewAdvanceParams
            x: middleFrameX + frameMargin
            width: middleFrameWidth - (frameMargin * 2)
            height: parent.height
        }

        Drawer {
            id: dicomFlyout
            property bool showFlyout: (dicomHelper.getDicomTagsData("All").length > 0)
            type: "LEFT"
            width: leftFrameWidth + frameMargin - separatorLine.width
            height: parent.height
            interactive: (listViewAdvanceParams.getCurEditParamInfo().State === "IDLE")
            visible: interactive && showFlyout
            content: [
                ExamPatientSelectDicomTagList {
                    id: dicomTagListAll
                    worklistDetailsType: "All"
                    width: parent.width
                    height: parent.height
                    listLeftOffset: frameMargin * 0.5
                    useDrawerColors: true
                }
            ]

            onStateChanged:
            {
                if (state === "OPEN")
                {
                    listViewAdvanceParams.interactive = false;
                }
                else
                {
                    listViewAdvanceParams.interactive = true;
                }
            }
        }
    }

    onWorklistEntriesChanged: {
        updateWorklistEntriesSorted();
        updateAnonymousPatient();
    }

    onCurSortTypeChanged: {
        updateWorklistEntriesSorted();
    }

    onIsPatientSelectedChanged: {
        reload();
    }

    onCruLinkStatusChanged: {
        reload();
    }

    onVisibleChanged: {
        reload();
    }

    onScreenStateChanged: {
        visible = (screenState === "ExamManager-PatientSelection");
    }

    function reload()
    {
        if  (isPatientSelected)
        {
            // Exam is started so patient is locked
            examPatientSelect.state = "EDITING";
        }
        else
        {
            examPatientSelect.state = "SELECTING";
        }

        if ( (!visible) ||
             (cruLinkStatus.State === "Active") )
        {
            // Hide CRU Link Down Warning
            if (blurBackground.isOpen())
            {
                blurBackground.close([rectCruDiconnectedWarning, frameActionBar]);
            }
        }
        else
        {
            // Show CRU Link Down Warning

            if (!blurBackground.isOpen())
            {
                blurBackground.open([rectCruDiconnectedWarning, frameActionBar], false, null);
            }
        }

        if (dicomFlyout.isOpened())
        {
            dicomFlyout.close();
        }

        // automatically select anonymous patient if CENT-ISI-CCT license... this condition is checked by licenseEnabledWorklistSelection
        if (visible && !licenseEnabledWorklistSelection && !isPatientSelected)
        {
            dsMwl.slotSelectWorklistEntry("00000000-0000-0000-0000-000000000000");
        }
    }

    function updateWorklistEntriesSorted()
    {
        var sortArgs = examPatientSelect.curSortType.split("-");
        var sortName = sortArgs[0];
        var sortDir = sortArgs[1];

        //logDebug("ExamPatientSelect: Sorting with name=" + sortName + ", dir=" + sortDir + "\nworklistEntries=" + JSON.stringify(worklistEntries));

        // Create workListEntriesMap with given sortName. Note, the item is an array of items as there might be multiple items with same key
        var workListEntriesMap = {};
        var keyList = [];
        for (var entryIdx = 0; entryIdx < worklistEntries.length; entryIdx++)
        {
            var entryData = worklistEntries[entryIdx];
            if (entryData.IsAnonymous)
            {
                continue;
            }

            var DataKey = entryData.DicomFields[sortName].Value;
            var itemGroup = workListEntriesMap[DataKey];

            if (itemGroup === undefined)
            {
                keyList.push(DataKey);
                itemGroup = [];
            }
            itemGroup.push(entryData);
            workListEntriesMap[DataKey] = itemGroup;
        }

        //logDebug("workListEntriesMap=" + JSON.stringify(workListEntriesMap));

        // Create workListEntriesSortedList
        var sortedKeyList = keyList.sort();
        var newWorklistEntriesSorted = [];

        for (var keyIdx = 0; keyIdx < sortedKeyList.length; keyIdx++)
        {
            var key = sortedKeyList[keyIdx];
            var itemGroupBuf = workListEntriesMap[key];

            for (var itemIdx = 0; itemIdx < itemGroupBuf.length; itemIdx++)
            {
                var itemData = itemGroupBuf[itemIdx];

                if (sortDir === "Up")
                {
                    itemData.isEndOfGroup = (itemIdx === itemGroupBuf.length - 1);
                    newWorklistEntriesSorted.push(itemData);
                }
                else
                {
                    itemData.isEndOfGroup = (itemIdx === 0);
                    newWorklistEntriesSorted.unshift(itemData);
                }
            }
        }

        worklistEntriesSorted = newWorklistEntriesSorted;

        //logDebug("\nworklistEntriesSorted=" + JSON.stringify(worklistEntriesSorted) + "\n");
    }

    function updateAnonymousPatient()
    {
        for (var entryIdx = 0; entryIdx < worklistEntries.length; entryIdx++)
        {
            var entryData = worklistEntries[entryIdx];
            if (entryData.IsAnonymous)
            {
                anonymousPatientData = Util.copyObject(entryData);
                //logDebug("anonymousPatientData=" + JSON.stringify(anonymousPatientData));
                return;
            }
        }

        anonymousPatientData = undefined;
    }

    function selectEntry(rowData)
    {
        logDebug("ExamPatientSelect: Worklist(Uid=" + rowData.StudyInstanceUid + ") selected");
        dsMwl.slotSelectWorklistEntry(rowData.StudyInstanceUid);
        examPatientSelect.state = "EDITING";
    }

    function deSelectEntry()
    {
        logDebug("ExamPatientSelect: Worklist deselected");
        dsMwl.slotDeselectWorklistEntry();
        examPatientSelect.state = "SELECTING";
    }

    function isEntrySelected()
    {
        return (examPatientSelect.state == "EDITING");
    }
}
