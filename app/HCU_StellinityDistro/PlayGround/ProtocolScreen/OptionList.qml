import QtQuick 2.3

ListView {
    property var curOptionData;

    id: optionListView
    spacing: optionListSpacing

    model: prop_listOptValues
    delegate: OptionItem {}
}

