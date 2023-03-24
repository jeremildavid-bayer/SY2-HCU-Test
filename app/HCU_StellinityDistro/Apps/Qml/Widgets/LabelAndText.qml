import QtQuick 2.12

Item {
    property alias widgetLabel: widgetLabel
    property alias widgetText: widgetText

    property int maximumLabelWidth: parent.width * 0.5
    property int spaceAfterLabel: widgetLabel.width - widgetLabel.contentWidth
    property int spacing: parent.width * 0.015

    clip: true

    Item {
        width: maximumLabelWidth
        height: parent.height

        Text {
            id: widgetLabel
            width: parent.width
            height: parent.height
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
        }
    }

    Text {
        id: widgetText
        anchors.left: parent.right
        anchors.leftMargin: spacing - spaceAfterLabel - (parent.width - maximumLabelWidth)
        anchors.right: parent.right
        height: parent.height
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.Wrap
    }
}
