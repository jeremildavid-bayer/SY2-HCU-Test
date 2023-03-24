import QtQuick 2.12

Item {
    property var paramData: paramItems[index]

    anchors.horizontalCenter: parent.horizontalCenter
    width: ListView.view.width * 0.94
    height: rowHeight

    Text {
        width: parent.width * 0.48
        height: parent.height * 0.9
        anchors.left: parent.left
        anchors.leftMargin: parent.width * 0.02
        anchors.verticalCenter: parent.verticalCenter
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignLeft
        font.pixelSize: height * 0.45
        font.family: fontRobotoLight.name
        text: translate("T_P3T_" + paramData.Name + "_Name")
        color: colorMap.text01
        wrapMode: Text.Wrap
        fontSizeMode: Text.Fit
        minimumPixelSize: font.pixelSize * 0.7
    }

    Text {
        width: parent.width * 0.48
        height: parent.height * 0.9
        anchors.right: parent.right
        anchors.rightMargin: parent.width * 0.0
        anchors.verticalCenter: parent.verticalCenter
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignRight
        font.pixelSize: height * 0.45
        font.family: fontRobotoLight.name
        text: getTextValue()
        wrapMode: Text.Wrap
        color: colorMap.text01
    }

    Rectangle {
        id: separatorLine
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        width: parent.width
        height: 1
        color: colorMap.text02
        visible: index < (paramItems.length - 1)
    }

    function getTextValue()
    {
        if (paramData.ValidDataType === "double")
        {
            return localeToFloatStr(paramData.Value, 1) + " " + translate("T_Units_" + paramData.Units);
        }
        return paramData.Value + " " + translate("T_Units_" + paramData.Units);
    }
}
