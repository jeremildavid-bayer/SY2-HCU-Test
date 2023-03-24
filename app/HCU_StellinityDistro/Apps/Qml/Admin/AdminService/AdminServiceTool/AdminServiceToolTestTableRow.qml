import QtQuick 2.12
import QtQuick.Controls 2.12
import "../../../Widgets"

Row {
    property string titleText: ""
    property string valueText: ""
    property bool initToggledValue: true
    property string controlType: "BUTTON" // BUTTON, TOGGLE, TEXT, HEADING, OBJECT, NONE
    property bool isReadOnly: false
    property alias actionContent: rectAction.children
    property alias customContent: customContent.children

    signal btnClicked()
    signal toggled(bool activated)

    id: root

    Rectangle {
        id: rectTitle
        color: "transparent"
        width: (controlType == "HEADING") ? paramsTableRowWidth : paramsTableRowWidth * titleWidthPercent
        height: paramsTableRowHeight
        border.color: colorMap.text02
        border.width: 1

        Text {
            anchors.fill: parent
            anchors.leftMargin: paramsTableRowWidth * 0.02
            anchors.rightMargin: paramsTableRowWidth * 0.02
            height: parent.height
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            color: colorMap.text02
            font.pixelSize: (controlType == "HEADING") ? parent.height * 0.45 : parent.height * 0.34
            text: titleText
            font.bold: (controlType == "HEADING")
        }
    }

    Rectangle {
        id: rectAction
        visible: (controlType == "BUTTON") || (controlType == "TOGGLE") || (controlType == "TEXT") || (controlType == "OBJECT")
        color: "transparent"
        width: paramsTableRowWidth - rectTitle.width
        height: paramsTableRowHeight
        border.color: colorMap.text02
        border.width: 1
        clip: true

        GenericIconButton {
            visible: controlType == "BUTTON"
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width * 0.9
            height: parent.height * 0.8
            color: colorMap.gry01
            iconText: valueText
            iconColor: colorMap.white01
            iconFontPixelSize: height * 0.4
            enabled: !isReadOnly
            pressedSoundCallback: function() {}
            onBtnClicked: {
                root.btnClicked();
            }
        }

        GenericToggleButton {
            id: toggleButton
            visible: controlType == "TOGGLE"
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width * 0.8
            height: parent.height * 0.8
            isReadOnly: root.isReadOnly
            onToggled: (activated) => {
                root.toggled(activated);
            }

            Component.onCompleted: (activated) => {
                activated = initToggledValue;
            }
        }

        Text {
            visible: controlType == "TEXT"
            anchors.fill: parent
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: height * 0.38
            text: valueText
            font.family: fontRobotoLight.name
            color: colorMap.text01
        }

        Item {
            visible: controlType == "OBJECT"
            id: customContent
            anchors.fill: parent
        }
    }

    function toggle(activated)
    {
        toggleButton.activated = activated;
    }
}
