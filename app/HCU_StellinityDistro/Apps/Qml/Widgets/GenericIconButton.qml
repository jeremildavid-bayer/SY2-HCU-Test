import QtQuick 2.12

GenericButton {
    property string iconFontFamily: fontIcon.name
    property string iconText: ""
    property string iconColor: colorMap.text01
    property double iconFontPixelSize: height * 0.38
    property int iconStyle: Text.Normal
    property string iconStyleColor: "black"

    property string captionText: ""
    property string captionColor: colorMap.text01
    property string captionFontPixelSize: height * 0.13
    property string captionFontFamily: fontRobotoLight.name
    property int captionY: height - (captionFontPixelSize * 1.5)

    content: [
        Text {
            id: icon
            width: parent.width
            height: (captionText == "") ? parent.height : captionY
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            color: iconColor
            font.pixelSize: iconFontPixelSize
            font.family: iconFontFamily
            text: iconText
            elide: Text.ElideRight
            wrapMode: Text.Wrap
            style: iconStyle
            styleColor: iconStyleColor
            opacity: enabled ? 1 : 0.4
        },
        Text {
            id: caption
            y: captionY
            width: parent.width
            height: parent.height * 0.1
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text: captionText
            font.pixelSize: captionFontPixelSize
            font.family: captionFontFamily
            color: captionColor
            opacity: enabled ? 1 : 0.4
            wrapMode: Text.Wrap
        }
    ]
}


