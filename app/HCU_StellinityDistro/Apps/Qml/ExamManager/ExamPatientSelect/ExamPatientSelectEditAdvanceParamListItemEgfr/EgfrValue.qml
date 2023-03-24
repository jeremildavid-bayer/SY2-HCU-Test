import QtQuick 2.12
import "../../../Widgets"
import "../../../Util.js" as Util
import "../"

ExamPatientSelectEditAdvanceParamListItemValue {
    property int egfrContentHeight: contentHeight + eGFRResultColorIndicator.height + eGFRResultMessage.height

    id: rectEgfrItem
    anchors.topMargin: (rectEgfrItem.textValue.text === "--") ? 0 : -(egfrContentHeight * 0.28)

    function getTextValue()
    {
        if (!visible || paramData === undefined || paramData.Value.ResultValue === undefined || paramData.Value.ResultValue === 0)
        {
            return "--";
        }
        else
        {
            var decimalDigits = Util.getDecimalDigits(paramData.ValidRange.ResolutionIncrement.toString());
            return localeToFloatStr(paramData.Value.ResultValue, decimalDigits);
        }
    }

    Rectangle {
        id: eGFRResultColorIndicator
        anchors.top: parent.top
        anchors.topMargin: egfrContentHeight * 0.68
        anchors.right: rectEgfrItem.textUnit.right
        height: getColor() === "transparent" ? 0 : 20
        width: root.width * 0.45
        color: getColor()

        function getColor()
        {
            if (!visible || (paramData === undefined) || (paramData.Value.ResultHexColor === "") || (paramData.Value.ResultHexColor === null))
            {
                return "transparent";
            }
            return paramData.Value.ResultHexColor;
        }
    }

    Text {
        id:eGFRResultMessage
        anchors.top: eGFRResultColorIndicator.bottom
        anchors.topMargin: 10
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.leftMargin: parent.width * 0.10
        height: getResultMessage() === "" ? 0 : contentHeight
        font.pixelSize: rectEgfrItem.textValue.font.pixelSize * 0.8
        font.family: fontRobotoMedium.name
        color: getTextValueColor()
        horizontalAlignment: Text.AlignRight
        wrapMode: Text.Wrap
        text: getResultMessage()

        function getResultMessage()
        {
            if (!visible || (paramData === undefined) || (paramData.Value.ResultMessage === "") || (paramData.Value.ResultMessage === undefined))
            {
                return "";
            }
            return paramData.Value.ResultMessage;
        }
    }
}
