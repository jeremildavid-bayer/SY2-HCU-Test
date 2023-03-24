import QtQuick 2.12
import "../../Widgets"
import "../../Widgets/Popup"
import "../../Util.js" as Util
import "AdminContrasts.js" as ContrastsUtil

Item {
    property string errorText: ""

    width: parent.width
    height: parent.height

    Item {
        id: containerDataError
        anchors.right: btnDelete.left
        anchors.rightMargin: parent.width * 0.03
        width: parent.width * 0.6
        height: parent.height
        visible: errorText != ""

        WarningIcon {
            id: iconDataError
            anchors.right: textDataError.left
            anchors.rightMargin: parent.width * 0.025
            width: height
            height: parent.height * 0.25
            anchors.verticalCenter: parent.verticalCenter
        }

        Text {
            id: textDataError
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            height: parent.height * 0.7
            color: colorMap.errText
            font.family: fontRobotoBold.name
            font.pixelSize: height * 0.2
            verticalAlignment: Text.AlignVCenter
            text: translate(errorText)

            onContentWidthChanged: {
                width = Math.min(parent.width, textDataError.contentWidth);
            }

            onTextChanged: {
                wrapMode = (width >= parent.width) ? Text.Wrap : Text.Normal;
            }
        }

    }

    GenericButton {
        id: btnDelete
        enabled: isDeleteReady()
        x: parent.width - width + radius
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width * 0.14
        height: parent.height * 0.6
        color: colorMap.keypadButton

        Text {
            width: parent.width
            height: parent.height
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.family: fontRobotoBold.name
            font.pixelSize: parent.height * 0.3
            color: colorMap.text01
            text: translate("T_Delete")
        }

        onBtnClicked: {
            ContrastsUtil.setReloadReason("REMOVED");
            ContrastsUtil.removeGroup(selectedFamilyIdx, selectedGroupIdx);

            // Select new group after delete
            if (selectedGroupIdx > 0)
            {
                selectedGroupIdx--;
            }
            else if (selectedFamilyIdx > 0)
            {
                selectedFamilyIdx--;
                if (contrastFamilies[selectedFamilyIdx].Groups.length >= 2)
                {
                    contrastsPage.selectedGroupIdx = contrastFamilies[selectedFamilyIdx].Groups.length - 2;
                }
                else
                {
                    contrastsPage.selectedGroupIdx = 0;
                }
            }
        }
    }

    Component.onCompleted: {
        appMain.screenStateChanged.connect(slotScreenStateChanged);
        slotScreenStateChanged();
    }

    Component.onDestruction: {
        appMain.screenStateChanged.disconnect(slotScreenStateChanged);
    }

    function slotScreenStateChanged()
    {
        visible = (appMain.screenState === "Admin-Contrasts");
        reload();
    }

    function reload()
    {
        if (!visible)
        {
            return;
        }
    }

    function isDeleteReady()
    {
        // check if new item
        if ( (widgetInputPad.isOpen()) ||
             (widgetKeyboard.isOpen()) )
        {
            return false;
        }
        else if ( (selectedFamilyIdx == -1) ||
                  (selectedGroupIdx == -1) )
        {
            return false;
        }
        else if ( (contrastsPage.contrastFamilies[selectedFamilyIdx] === undefined) ||
                  (contrastsPage.contrastFamilies[selectedFamilyIdx].Groups[selectedGroupIdx] === undefined) )
        {
            return false;
        }
        else if (contrastsPage.contrastFamilies[selectedFamilyIdx].Groups[selectedGroupIdx].Brand === "<NEW>")
        {
            return false;
        }

        return true;
    }
}
