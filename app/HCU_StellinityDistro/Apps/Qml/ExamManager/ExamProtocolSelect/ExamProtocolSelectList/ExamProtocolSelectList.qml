import QtQuick 2.12
import "../../../Widgets"

Item {
    property var planPreview: dsExam.planPreview
    property bool planTemplateGroupsReady: dsExam.planTemplateGroupsReady
    property var planTemplateGroups: dsExam.planTemplateGroups
    property string examProgressState: dsExam.examProgressState
    property string previousExamProgressState: ""
    property var cruLinkStatus: dsCru.cruLinkStatus
    property int selectedGroupIdx: -1
    property int selectedPlanIdx: -1
    property bool doNotSelectSpotlightProtocol: false
    property int planTemplateGroupsReadyDebounceMs: 1000
    property bool planTemplatesChanged: false
    property bool wasButtonPressed: false

    signal signalReloadRows();

    id: protocolSelectList

    Timer {
        // The planTemplateGroups data changes a lot due to the internal plan digest GUID change.
        // For efficient UI update, the data shall be debounced.
        id: tmrPlanTemplateGroupsReadyDebounce
        interval: planTemplateGroupsReadyDebounceMs // Debounce time
        repeat: false
        onTriggered: {
            if (JSON.stringify(listViewPlanGroup.model) != JSON.stringify(planTemplateGroups))
            {
                logDebug("ExamProtocolSelectList: listViewPlanGroup.model updated") ;
                listViewPlanGroup.model = planTemplateGroups;
            }

            setSelectedIndexes();
        }
    }

    ListView {
        property double lastContentY: 0

        id: listViewPlanGroup
        clip: true
        anchors.fill: parent
        anchors.topMargin: parent.height * 0.04
        anchors.bottomMargin: parent.height * 0.04

        delegate: ExamProtocolSelectListGroup {}

        cacheBuffer: Math.max(contentHeight * 2, height * 10)

        footer: Item {
            width: ListView.view ? ListView.view.width : 0
            height: ListView.view ? ListView.view.height * 0.1 : 0
        }

        ScrollBar {}
        ListFade {}

        onContentYChanged: {
            if (contentY != -0)
            {
                lastContentY = contentY;
            }
        }

        onModelChanged: {
            contentY = lastContentY;
        }
    }

    MouseArea {
        anchors.fill: parent
        visible: (cruLinkStatus.State === "Active") && (!planTemplateGroupsReady)
        LoadingGif {
            visible: parent.visible
        }
    }

    onPlanPreviewChanged: {
        reload();
    }

    onPlanTemplateGroupsChanged: {
        if (!doNotSelectSpotlightProtocol)
        {
            planTemplatesChanged = true;
            selectedGroupIdx = -1;
            selectedPlanIdx = -1;
        }

        reload();
    }

    onExamProgressStateChanged: {
        if (previousExamProgressState === "Completing" && examProgressState === "Completed")
        {
            setDoNotSelectSpotlightProtocol(false);
            setWasButtonPressed(false);
        }
        previousExamProgressState = examProgressState;
    }

    onCruLinkStatusChanged: {
        // Update the listview model soon
        tmrPlanTemplateGroupsReadyDebounce.stop();
        tmrPlanTemplateGroupsReadyDebounce.start();
    }

    Component.onCompleted: {
        appMain.screenStateChanged.connect(slotScreenStateChanged);
        slotScreenStateChanged();
    }

    Component.onDestruction: {
        appMain.screenStateChanged.disconnect(slotScreenStateChanged);
    }

    function setSelectedIndexes() //returns true if indexes changed
    {
        var foundPlan = false;

        if ( (planTemplateGroups === undefined) ||
             (planPreview === undefined) )
        {
            return;
        }

        var planPreviewTemplate = planPreview.Template;
        var newGroupIdx = -1;
        var newPlanIdx = -1;
        var isSelectedFromSpotlight = false;
        var spotlightGroupExists = false;

        for (var groupIdx = 0; groupIdx < planTemplateGroups.length; groupIdx++)
        {
            if ((planTemplateGroups[groupIdx].Name === "T_SpotlightProtocols") && (cruLinkStatus.State === "Active"))
            {
                spotlightGroupExists = true;
            }

            if (planTemplatesChanged && !wasButtonPressed)
            {
                break;
            }

            for (var planIdx = 0; planIdx < planTemplateGroups[groupIdx].PlanDigests.length; planIdx++)
            {
                //logDebug("Compare with guid: " + listViewPlanGroup[groupIdx].Plans[planIdx].Template);
                if (planPreviewTemplate === planTemplateGroups[groupIdx].PlanDigests[planIdx].Plan.Template)
                {
                    if (doNotSelectSpotlightProtocol && (planTemplateGroups[groupIdx].Name === "T_SpotlightProtocols"))
                    {
                        //This isn't the 'right' version of this protocol...
                        continue;
                    }

                    newGroupIdx = groupIdx;
                    newPlanIdx = planIdx;
                    foundPlan = true;
                    break;
                }
            }

            if (foundPlan)
            {
                isSelectedFromSpotlight = (planTemplateGroups[groupIdx].Name === "T_SpotlightProtocols");
                break;
            }
        }

        if ( (selectedGroupIdx !== newGroupIdx) ||
             (selectedPlanIdx !== newPlanIdx) )
        {
            selectedGroupIdx = newGroupIdx;
            selectedPlanIdx = newPlanIdx;
        }

        // if there is a spotlight protocol, and the user hasn't selected a different protocol, auto-select the first spotlight protocol.
        if (!doNotSelectSpotlightProtocol && spotlightGroupExists && (newGroupIdx === -1))
        {
            isSelectedFromSpotlight = true;
            selectedGroupIdx = 0;
            selectedPlanIdx = 0;
        }
        // if nothing is selected, but there is only one entry in the group, show it open
        else if ((selectedGroupIdx === -1) && (planTemplateGroups.length === 1))
        {
            selectedGroupIdx = 0;
        }

        if (selectedGroupIdx !== -1 && selectedPlanIdx !== -1)
        {
            var groupData = planTemplateGroups[selectedGroupIdx];
            var planDigest = groupData.PlanDigests[selectedPlanIdx];
            var planData = planDigest.Plan;
            dsExam.slotPlanPreviewChanged(planData);
            signalReloadRows();
            listViewPlanGroup.positionViewAtIndex(selectedGroupIdx, ListView.Center);
        }

        // if nothing is selected, use the default protocol
        if (!wasButtonPressed && !isSelectedFromSpotlight && (planPreview.Template !== defaultInjectPlanTemplateGuid))
        {
            dsExam.slotDefaultPlanTemplateSelected();
        }
    }

    function setDoNotSelectSpotlightProtocol(avoidSpotlightTemplate)
    {
        doNotSelectSpotlightProtocol = avoidSpotlightTemplate;
    }

    function setWasButtonPressed(buttonPressed)
    {
        wasButtonPressed = buttonPressed;
    }

    function slotScreenStateChanged()
    {
        var prevVisible = visible;
        visible = (appMain.screenState === "ExamManager-ProtocolSelection");
        reload();

        if ( (visible) && (!prevVisible) )
        {
            signalReloadRows();
            listViewPlanGroup.positionViewAtIndex(selectedGroupIdx, ListView.Center);
        }
    }

    function reload()
    {
        if (!visible)
        {
            // Let the list updated while invisible. This is to prevent busy-loading when first visible.
            //return;
        }

        if ( (screenState == "OFF") ||
             (screenState == "Startup") )
        {
            // Reload after startup screen
            return;
        }

        if (JSON.stringify(listViewPlanGroup.model) != JSON.stringify(planTemplateGroups))
        {
            logDebug("ExamProtocolSelectList: planTemplateGroups changed") ;

            // Update the listview model soon.
            tmrPlanTemplateGroupsReadyDebounce.stop();
            tmrPlanTemplateGroupsReadyDebounce.start();
        }
        else
        {
            setSelectedIndexes();
        }
    }
}
