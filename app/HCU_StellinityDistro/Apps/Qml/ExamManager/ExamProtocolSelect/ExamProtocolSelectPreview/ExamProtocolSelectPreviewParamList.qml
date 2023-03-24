import QtQuick 2.12

Rectangle {
    property int rowHeight: protocolSelectPreview.height * 0.09
    property var paramItems: []
    property string title: ""

    id: root
    color: "transparent"
    height: listView.height
    radius: 15
    border.width: 3
    border.color: colorMap.keypadButton

    ListView {
        id: listView
        clip: true
        interactive: false
        width: parent.width
        height: visible ? ((rowHeight * paramItems.length) + headerItem.height + footerItem.height) : 0
        visible: (paramItems != undefined) && (paramItems.length > 0)
        header: Rectangle {
            id: rectHeader
            radius: root.radius
            width: ListView.view ? ListView.view.width : 0
            height: rowHeight
            color: colorMap.keypadButton
            Text {
                anchors.fill: parent
                anchors.leftMargin: parent.width * 0.03
                anchors.right: parent.right
                anchors.rightMargin: parent.width * 0.03
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: height * 0.45
                font.family: fontRobotoBold.name
                text: translate(title)
                color: colorMap.text01
                wrapMode: Text.Wrap
            }
            Rectangle {
                width: parent.width
                height: parent.radius
                anchors.bottom: parent.bottom
                color: parent.color
            }
        }
        footer: Item {
            width: parent.width
            height: rowHeight * 0.1
        }

        model: paramItems
        delegate: ExamProtocolSelectPreviewParamItem {}
    }
}
