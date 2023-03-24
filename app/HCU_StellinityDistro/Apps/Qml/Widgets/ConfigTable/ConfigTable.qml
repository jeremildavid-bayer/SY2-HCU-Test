import QtQuick 2.12
import "../../Widgets"

Item {
    property var allConfigList
    property var configList: []
    property var customConfigRows
    property string configKeyName: "Settings_General_"
    property string visibleScreenState: "Admin-Settings-General"
    property int selectedRow: -1
    property bool translationRequired: true
    property bool interactive: true

    signal signalRowSelected(int rowIndex)
    signal signalReloadRow()
    signal signalConfigChanged(var configItem)

    id: configTable
    anchors.fill: parent
    visible: false

    ListView {
        property double lastContentY: 0

        id: listViewSettings
        cacheBuffer: Math.max(contentHeight * 2, height * 10)
        anchors.fill: parent
        clip: true
        delegate: ConfigTableRow {}
        interactive: configTable.interactive

        highlightMoveDuration: 0
        highlightRangeMode: ListView.NoHighlightRange

        header: Item {
            width: ListView.view ? ListView.view.width : 0
            height: widgetKeyboard.isOpen() ? (-widgetKeyboard.newMainWindowY) : 0
        }

        footer: Item {
            width: ListView.view ? ListView.view.width : 0
            height: widgetKeyboard.isOpen() ? (widgetKeyboard.keyboardHeight / 2) : (ListView.view ? ListView.view.height : 0) * 0.3
        }

        ScrollBar {}
        ListFade {
            headerSize: parent.headerItem.height
        }

        model: configList.length

        onModelChanged: {
            contentY = lastContentY;
        }

        onContentYChanged: {
            if (contentY != -0)
            {
                lastContentY = contentY;
            }
        }
    }

    onAllConfigListChanged: {
        if (allConfigList === undefined)
        {
            return;
        }

        var newConfigList = [];
        for (var i = 0; i < allConfigList.length; i++)
        {
            var configItem = allConfigList[i];
            //logDebug("allConfigList[" + i + "]: " + JSON.stringify(configItem));

            if ( (configItem.KeyName !== undefined) &&
                 (configItem.KeyName.indexOf(configKeyName) === 0) )
            {
                // config item for current filter found
                newConfigList.push(configItem);
            }
        }

        //logDebug("newConfigList = " + JSON.stringify(newConfigList));
        configList = newConfigList;
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
        visible = (appMain.screenState === visibleScreenState);
        reload();
    }

    function reload()
    {
        if (!visible)
        {
            return;
        }

        if (widgetInputPad.isOpen())
        {
            widgetInputPad.close(false);
        }

        if (widgetKeyboard.isOpen())
        {
            widgetKeyboard.close(false);
        }

        signalReloadRow();
        signalRowSelected(selectedRow);
    }
}
