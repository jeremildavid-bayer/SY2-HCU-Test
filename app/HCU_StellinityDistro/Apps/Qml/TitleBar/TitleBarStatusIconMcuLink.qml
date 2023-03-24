import QtQuick 2.12
import "../Widgets"

GenericButton {
    property string mcuLinkState: dsMcu.mcuLinkState

    visible: {
        if ( (mcuLinkState === "DISCONNECTED") ||
             (mcuLinkState === "CONNECTED_RX_TIMEOUT") )
        {
            return true;
        }
        return false;
    }

    Text {
        horizontalAlignment: Text.AlignHCenter
        y: parent.height * 0.3
        width: parent.width
        font.pixelSize: parent.height * 0.5
        font.family: fontAwesome.name
        text: "\uf013"
        color: colorMap.statusIconText2
    }

    Text {
        anchors.fill: parent
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.pixelSize: parent.height * 0.5
        font.family: fontIcon.name
        text: "\ue913"
        color: colorMap.statusIconRed
    }

    onBtnClicked: {
        var infoText = "MCU Link: " + mcuLinkState;
        statusIconInfoPanel.open(infoText);
    }
}
