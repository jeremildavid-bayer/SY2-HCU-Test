import QtQuick 2.12
import "../../Widgets"
import "../../Util.js" as Util

Item {
    property int contentHeight: Math.max(rowHeight, textValue.contentHeight)
    property alias textValue: textValue
    property alias textUnit: textUnit
    property var paramData

    property var textValuefunc: getTextValue
    property var textUnitfunc: getUnitValue
    property var textValueColorfunc: getTextValueColor
    property var textUnitColorfunc: getTextUnitColor

    property var interactive

    visible: true
    anchors.top: parent.top
    anchors.right: parent.right
    width: parent.width
    height: contentHeight

    Text {
        id: textValue
        anchors.left: parent.left
        anchors.right: textUnit.left
        anchors.rightMargin: (textUnit.contentWidth == 0) ? 0 : parent.width * 0.023
        height: parent.height
        font.pixelSize: rowHeight * 0.38
        font.family: fontRobotoBold.name
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignRight
        wrapMode: Text.Wrap
        text: textValuefunc()
        color: textValueColorfunc()
    }

    Text {
        id: textUnit
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.topMargin: height * 0.04
        width: contentWidth
        height: textValue.height
        font.family: fontRobotoLight.name
        font.pixelSize: textValue.font.pixelSize * 0.73
        verticalAlignment: Text.AlignVCenter
        text: textUnitfunc()
        color: textUnitColorfunc()
    }

    function getTextValue()
    {
        if (!visible || paramData === undefined)
        {
            return "--";
        }
        else if (paramData.ValidDataType === "double")
        {
            var decimalDigits = Util.getDecimalDigits(paramData.ValidRange.ResolutionIncrement.toString());
            return localeToFloatStr(paramData.Value, decimalDigits);
        }
        else if (paramData.Name === "CatheterPlacement")
        {
            var catheterPlacement = JSON.parse(paramData.Value).Name;
            if ( (catheterPlacement === null) || (catheterPlacement === undefined) || (catheterPlacement === "") )
            {
                return "--";
            }
            return catheterPlacement;
        }
        else if (paramData.Value === "")
        {
            return "--"
        }
        return paramData.Value;
    }

    function getUnitValue()
    {
        if (paramData === undefined)
        {
            return "";
        }
        else if (paramData.Units !== "")
        {
            return translate("T_Units_" + paramData.Units);
        }
        return "";
    }

    function getTextValueColor()
    {
        return (interactive ? colorMap.text01 : colorMap.text02);
    }

    function getTextUnitColor()
    {
        return colorMap.text02;
    }

    function init()
    {
        textValue.text = Qt.binding(function() { return textValuefunc(); });
        textUnit.text = Qt.binding(function() { return textUnitfunc(); });
        textValue.color = Qt.binding(function() { return textValueColorfunc(); });
        textUnit.color = Qt.binding(function() { return textUnitColorfunc();} );
    }
}
