import QtQuick 2.12
import "../../Widgets"

ListView {
    property var worklistEntriesSorted: examPatientSelect.worklistEntriesSorted
    property int searchStrFoundCount: 0

    id: worklistView
    clip: true
    cacheBuffer: Math.max(1, Math.max(contentHeight * 2, height * 10))

    ScrollBar {}
    ListFade {}

    delegate: ExamPatientSelectListItem {
        onSearchStrFoundChanged: {
            if (searchStrFound)
            {
                worklistView.searchStrFoundCount++;
            }
            else
            {
                worklistView.searchStrFoundCount--;
            }
        }
    }

    footer: Item {
        width: ListView.view ? ListView.view.width : 0
        height: ListView.view ? ListView.view.height * 0.1 : 0
    }

    onWorklistEntriesSortedChanged: {
        reload();
    }

    onVisibleChanged: {
        reload();
    }

    function reload()
    {
        if (!visible)
        {
            return;
        }
        if (JSON.stringify(worklistView.model) != JSON.stringify(worklistEntriesSorted))
        {
            searchStrFoundCount = 0;
            worklistView.model = [];
            worklistView.model = worklistEntriesSorted;
        }
    }
}
