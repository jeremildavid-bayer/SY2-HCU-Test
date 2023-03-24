import QtQuick 2.12
import "../"

Item {
    id: root
    height: (btnWidth + btnSpacing) * 3

    Repeater {
        model: 9
        delegate: GenericIconButton {
            x: ((index % 3) * width) + ((index % 3) * btnSpacing)
            // Subtracting from (3 - 1) so we get keyboard-looking number pad not phone-looking numberpad
            y: ((2 - parseInt(index / 3)) * height) + ((2 - parseInt(index / 3)) * btnSpacing)
            width: btnWidth
            height: width
            radius: 0
            color: colorMap.keypadButton
            iconColor: colorMap.text01
            iconFontFamily: fontRobotoLight.name
            iconText: index + 1
            pressedSoundCallback: keyPressedSoundCallback
            onBtnClicked: {
                slotBtnPressed((index + 1).toString());
            }
        }
    }
}
