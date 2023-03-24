import QtQuick 2.12
import "../../../Widgets"
import "../../../Util.js" as Util

ListView {
    id: listView
    orientation: ListView.Horizontal
    spacing: actionButtonWidth * 0.02
    clip: true
    enabled: selectOtherSourcePackageEnabled
    model: fluidSelectItems
    highlightMoveDuration: 0
    highlightRangeMode: ListView.NoHighlightRange

    ListFade {}

    onVisibleChanged: {
        selectItem(currentIndex);
    }

    delegate: DeviceManagerPanelBottleGroupListItem {}

    function selectItem(itemIdx)
    {
        currentIndex = -1;
        currentIndex = itemIdx;
    }

}

