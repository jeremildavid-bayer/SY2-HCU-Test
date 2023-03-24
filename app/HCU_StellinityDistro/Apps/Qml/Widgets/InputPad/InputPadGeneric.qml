import QtQuick 2.12
import "../"

Item {
    property alias content: mainRect.children
    property int btnSpacing: width * 0.03
    property int btnWidth: ((width - (btnSpacing * 2)) / 3)
    property string textDescription: ""
    property bool okBtnVisible: true

    width: parent.width
    height: parent.height

    Text {
        id: textDescriptionField
        width: parent.width
        height: textDescription == "" ? 0 : parent.height * 0.07
        color: colorMap.text01
        font.family: fontRobotoLight.name
        font.pixelSize: height * 0.6
        font.bold: true
        text: textDescription
        verticalAlignment: Text.AlignBottom
    }

    Item {
        id: mainRect
        width: parent.width
        anchors.top: textDescriptionField.bottom
        anchors.topMargin: btnSpacing
        anchors.bottomMargin: btnSpacing * 2
        anchors.bottom: btnsControl.top
    }

    Item {
        id: btnsControl
        anchors.bottom: parent.bottom
        anchors.bottomMargin: parent.height * 0.05
        width: parent.width
        height: btnWidth

        GenericIconButton {
            id: btnCancel
            anchors.left: parent.left
            width: btnWidth
            height: btnWidth
            radius: 0
            color: colorMap.keypadCancelButton
            iconColor: colorMap.actionButtonText
            iconFontFamily: fontIcon.name
            iconText: "\ue939"
            pressedSoundCallback: keyPressedSoundCallback
            onBtnClicked: {
                close(false);
            }
        }

        GenericIconButton {
            id: btnOk
            anchors.left: btnCancel.right
            anchors.leftMargin: btnSpacing
            anchors.right: parent.right
            visible: okBtnVisible
            height: btnWidth
            radius: 0
            color: colorMap.actionButtonBackground
            iconColor: colorMap.actionButtonText
            iconFontFamily: fontRobotoBold.name
            iconText: translate("T_OK")
            pressedSoundCallback: keyPressedSoundCallback
            onBtnClicked: {
                close(true);
            }
        }
    }
}
