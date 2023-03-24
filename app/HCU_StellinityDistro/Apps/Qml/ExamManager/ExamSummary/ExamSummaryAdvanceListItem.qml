import QtQuick 2.12
import "../../Util.js" as Util
import "../../Widgets"

GenericButton {
    property string labelText: ""
    property string valueText: ""
    property alias valueTextObject: textValue
    property bool isExamStarted: dsExam.isExamStarted
    property var cruLinkStatus: dsCru.cruLinkStatus
    property var examAdvanceInfo: dsExam.examAdvanceInfo
    property var paramData

    radius: 0
    interactive: (isExamStarted) && (cruLinkStatus.State === "Active")
    width: parent.width
    height: Math.max(textValue.height, rowHeightMin)

    Text {
        id: textLabel
        height: rowHeightMin
        width: parent.width * 0.35
        verticalAlignment: Text.AlignVCenter
        font.family: fontRobotoLight.name
        font.pixelSize: rowHeightMin * 0.3
        minimumPixelSize: font.pixelSize * 0.24
        fontSizeMode: Text.Fit
        color: interactive ? colorMap.text01 : colorMap.text02
        text: labelText
        wrapMode: Text.Wrap

        MandatoryFieldMark {
            visible: (paramData !== undefined) && (cruLinkStatus.State === "Active") && (isExamStarted) && (paramData.IsMandatory) && (!paramData.IsMandatoryEntered)
        }
    }

    Rectangle {
        id: editContainer
        color: "transparent"
        anchors.left: textLabel.right
        anchors.right: parent.right
        height: textValue.height

        Text {
            id: textValue
            width: {
                if ( (widgetKeyboard.isOpenFor(this)) ||
                     (widgetInputPad.isOpenFor(this)) )
                {
                    return parent.width * 0.98;
                }
                return parent.width;
            }

            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignRight
            height: (contentHeight > rowHeightMin) ? (contentHeight + (font.pixelSize * 2)) : rowHeightMin
            text: valueText
            color: interactive ? colorMap.text01 : colorMap.text02
            wrapMode: Text.Wrap
            font.family: fontRobotoBold.name
            font.pixelSize: rowHeightMin * 0.31
        }
    }

    Component.onCompleted: {
        flickAdvanceFields.dragStarted.connect(reset);
    }

    function setDataBindings()
    {
        valueText = Qt.binding(function() {
            if ( (paramData !== undefined) &&
                 (paramData.Value !== "") )
            {
                return paramData.Value;
            }
            return "--";
        });

        textValue.text = Qt.binding(function() { return valueText; });
        textValue.color = Qt.binding(function() { return interactive ? colorMap.text01 : colorMap.text02; });
    }
}
