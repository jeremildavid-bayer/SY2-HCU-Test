import QtQuick 2.3

ListView {
    property int listSpacing: 4

    property int animationMs: 250
    property int rowHeight: 80
    property int textMargin: 10
    property string titleBackgroundColor: "#ffcccccc"
    property string titleTextColor: "black"
    property int titleTextFontPixelSize: 22

    property int itemMargin: 30
    property string itemBackgroundColor: "#ff777777"
    property string itmeTextColor: "white"
    property int itemTextFontPixelSize: 20

    property int lastContentY: 0

    signal listDragStarted();
    signal itemClicked(int index, string item);
    signal itemsExpanded(int index, bool expanded);

    id: listMain
    objectName: "listMain"
    spacing: listSpacing
    model: prop_listValues
    delegate: OptionItem {}

    cacheBuffer: 10000

    onDragStarted: {
        listDragStarted();
    }

    onDragEnded: {
        lastContentY = contentY;
    }

    onItemClicked: {
        lastContentY = contentY;
    }

    onModelChanged: {
        console.log("model: " + JSON.stringify(prop_listValues));
        contentY = lastContentY;
    }
}

