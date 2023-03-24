import QtQuick 2.12

Text {
    font.family: fontIcon.name
    text: "\ue932"
    color: colorMap.red
    font.pixelSize: height
    verticalAlignment: Text.AlignVCenter
    horizontalAlignment: Text.AlignHCenter

    Text {
         font.family: fontIcon.name
         text: "\ue93f"
         color: colorMap.white01
         anchors.fill: parent
         font.pixelSize: parent.font.pixelSize
         verticalAlignment: parent.verticalAlignment
         horizontalAlignment: parent.horizontalAlignment
    }
}
