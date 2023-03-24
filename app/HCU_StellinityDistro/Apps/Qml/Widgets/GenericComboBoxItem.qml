import QtQuick 2.12

Rectangle {
    property alias stateIconFrame: stateIconFrame
    property string type: "ITEM" // ITEM, HEADER

    property string iconText: ""
    property string iconColor: colorMap.text01
    property string valueText: ""
    property string valueColor: colorMap.text01
    property string unitText: ""
    property string unitColor: colorMap.text02
    property int iconMargin: width * 0.05

    id: header
    height: parent.height
    width: parent.width

    property bool useSvgIcon: false
    property string svgIconSource: ""
    property int svgIconWidth: 20
    property int svgIconHeight: 20

    Item {
        id: icon
        anchors.left: parent.left
        anchors.leftMargin: iconMargin
        height: parent.height
        width: textIcon.visible ? textIcon.width : imageIcon.width

        Text {
            id: textIcon
            anchors.left: parent.left
            height: parent.height
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: itemIconTextFontPixelSize
            font.family: fontIcon.name
            width: contentWidth
            text: iconText
            color: iconColor
        }

        Image {
            id: imageIcon
            anchors.verticalCenter: parent.verticalCenter
            height: svgIconHeight
            width: svgIconWidth
            source: svgIconSource
            // sourceSize.width/height seems to be bound to the height and width. Not needed?
            //sourceSize.width: svgIconWidth
            //sourceSize.height: svgIconHeight
        }

        Component.onCompleted: {
            textIcon.visible = !useSvgIcon;
            imageIcon.visible = useSvgIcon;
        }
    }

    Item {
        property int maximumWidth: stateIconFrame.x - icon.width - icon.x - (iconMargin * 2) - unitValue.contentWidth
        property int firstTextWidth: textValue.width - textValue.contentWidth
        property int spacing: (textValue.width > 0) ? (iconMargin / 2) : 0

        id: containerTextValue
        anchors.left: icon.right
        anchors.leftMargin: (icon.width > 0) ? iconMargin : 0
        height: parent.height
        width: maximumWidth

        Text {
            id: textValue
            width: parent.maximumWidth
            height: parent.height
            font.family: itemValueFontFamily
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: itemValueTextFontPixelSize
            text: valueText
            color: valueColor
        }

        Text {
            id: unitValue
            anchors.left: parent.right
            anchors.leftMargin: parent.spacing - parent.firstTextWidth
            anchors.right: parent.right
            font.family: itemUnitFontFamily
            height: parent.height
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: itemUnitTextFontPixelSize
            text: unitText
            color: unitColor
        }
    }

    Item {
        id: stateIconFrame
        visible: (!readOnly) && (type == "HEADER")
        anchors.right: parent.right
        anchors.rightMargin: height * 0.2
        anchors.verticalCenter: parent.verticalCenter
        height: parent.height
        width: stateIcon.contentWidth

        Text {
            id: stateIcon
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            color: expandSymbolColor
            font.pixelSize: height * 0.3
            font.family: fontIcon.name
            text: "\ue906"
        }
    }
}

