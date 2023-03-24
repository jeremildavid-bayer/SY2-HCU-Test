import QtQuick 2.12

Text {
    y: (parent.height - parent.contentHeight) / 2
    width: contentWidth
    anchors.left: parent.left
    anchors.leftMargin: parent.contentWidth + (contentWidth * 0.2)
    font.family: fontIcon.name
    color: colorMap.errText
    font.pixelSize: dsCfgLocal.screenH * 0.02
    text: "\ue945"
}
