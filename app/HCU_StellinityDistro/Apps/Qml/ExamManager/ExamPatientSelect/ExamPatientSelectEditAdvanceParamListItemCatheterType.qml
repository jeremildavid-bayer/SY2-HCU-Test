import QtQuick 2.12
import "../../Widgets"
import "../../Util.js" as Util

Item {
    property int contentHeight: Math.max(rowHeight, icon1.contentHeight)
    property var dataValue
    property alias textValue: originalString
    property bool interactive: false
    property int alignment: Text.AlignRight

    id: root
    anchors.right: parent.right
    anchors.top: parent.top
    width: parent.width
    height: contentHeight

    Text {
        id: icon1
        height: parent.height
        anchors.rightMargin: width * 0.25
        font.pixelSize: rowHeight * 0.38
        font.family: fontIcon.name
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: alignment
        text: (catherName.text == "--") ? "" : "\ue971"
        color: {
            if (!interactive)
            {
                return colorMap.text02;
            }
            else if ( (dataValue !== undefined) &&
                      (dataValue.HexColor !== null) &&
                      (dataValue.HexColor !== undefined) )
            {
                return dataValue.HexColor;
            }
            return "transparent";
        }

        Component.onCompleted:
        {
            if (alignment === Text.AlignRight)
            {
                anchors.right = catherName.left;
            }
            else if (alignment === Text.AlignLeft)
            {
                anchors.left = parent.left;
            }
        }
    }

    Text {
        id: icon2
        x: icon1.x
        y: icon1.y
        width: icon1.width
        height: icon1.height
        font.pixelSize: icon1.font.pixelSize
        font.family: icon1.font.family
        verticalAlignment: icon1.verticalAlignment
        horizontalAlignment: icon1.horizontalAlignment
        text: ( (dataValue === undefined) || (dataValue.HexColor === null) || (dataValue.HexColor === undefined) ) ? "" : "\ue96f"
        color: {
            if (!interactive)
            {
                return colorMap.text02;
            }
            return colorMap.grid;
        }
    }

    Text {
        id: catherName
        anchors.top: parent.top
        anchors.topMargin: height * 0.04
        width: contentWidth
        height: icon1.height
        font.family: fontRobotoBold.name
        font.pixelSize: height * 0.38
        verticalAlignment: Text.AlignVCenter
        color: {
            if (!interactive)
            {
                return colorMap.text02;
            }
            return colorMap.text01;
        }

        text: ( (dataValue === undefined) || (dataValue.Name === null) || (dataValue.Name === undefined) || (dataValue.Name === "") ) ? "--" : dataValue.Name;

        Component.onCompleted:
        {
            if (alignment === Text.AlignRight)
            {
                anchors.right = parent.right;
            }
            else if (alignment === Text.AlignLeft)
            {
                anchors.left = icon1.right;
                anchors.leftMargin = Qt.binding(function () { return (catherName.text == "--") ? undefined : parent.width * 0.05; });
            }
        }
    }

    // This is non-visible container for original string to be exposed externally.
    // refer to alias and how this is accessed externally
    Text {
        id: originalString
        visible: false
        text: {
            if (dataValue !== undefined)
                return JSON.stringify(dataValue);
            return "";
        }
    }
}
