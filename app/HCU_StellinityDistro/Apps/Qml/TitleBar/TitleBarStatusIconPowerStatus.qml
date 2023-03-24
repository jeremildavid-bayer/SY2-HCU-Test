import QtQuick 2.12
import QtQml 2.15
import "../Widgets"
import "../Util.js" as Util

GenericButton {
    property var powerStatus: dsMcu.powerStatus
    property int blinkBatteryAnimationMs: 750
    property string mcuLinkState: dsMcu.mcuLinkState
    property string baseBoardType: dsHardwareInfo.baseBoardType
    property string workflowState: dsWorkflow.workflowState

    property var activeAlerts: []
    Binding on activeAlerts {
        when: visible
        value: dsAlert.activeAlerts;
        restoreMode: Binding.RestoreBinding
    }

    onActiveAlertsChanged:
    {
        var aqdInProgress = false;
        for (var i = 0; i < activeAlerts.length; i++)
        {
            if (activeAlerts[i].CodeName === "AutomaticQualifiedDischargeInProgress")
            {
                aqdInProgress = true;
                break;
            }
        }

        if (aqdInProgress !== aqdStatus.visible)
        {
            aqdStatus.visible = aqdInProgress;
        }
    }

    SequentialAnimation {
        id: animationPowerIconBlink
        loops: Animation.Infinite

        NumberAnimation { target: root; properties: 'opacity'; from: 0; to: 1; duration: blinkBatteryAnimationMs }
        NumberAnimation { target: root; properties: 'opacity'; from: 1; to: 0; duration: blinkBatteryAnimationMs }

        ScriptAction { script: {
                // Play Warning Audio Tone
                if (powerStatus.BatteryLevel === "Dead")
                {
                    soundPlayer.playPressAllStop();
                }
            }
        }
    }

    id: root

    visible: {
        if ( (baseBoardType === "Battery") &&
             (mcuLinkState === "CONNECTED") &&
             (powerStatus !== undefined) &&
             (powerStatus.BatteryLevel !== "NoBattery") &&
             (powerStatus.BatteryLevel !== "Unknown") )
        {
            return true;
        }
        return false;
    }

    Text {
        id: iconBatteryContainer
        anchors.left: parent.left
        width: parent.width * 0.4
        height: parent.height
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.pixelSize: parent.height * 0.55
        font.family: fontIcon.name
        text: "\ue919"
        color: {
            if (powerStatus === undefined)
            {
                return colorMap.statusIconText2;
            }
            else if ( (powerStatus.BatteryLevel === "Full") ||
                     (powerStatus.BatteryLevel === "High") ||
                     (powerStatus.BatteryLevel === "Medium") ||
                     (powerStatus.BatteryLevel === "Low") )
            {
                return colorMap.statusIconText1;
            }
            else if ( (powerStatus.BatteryLevel === "Flat") ||
                     (powerStatus.BatteryLevel === "Dead") ||
                     (powerStatus.BatteryLevel === "Critical") )
            {
                return colorMap.statusIconBatteryRed;
            }
            return colorMap.statusIconText2;
        }

        Text {
            id: iconBatteryCell
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: parent.height * 0.55
            font.family: fontIcon.name
            text: {
                if (powerStatus === undefined)
                {
                    return "";
                }
                else if (powerStatus.BatteryLevel === "Full")
                {
                    return "\ue91a";
                }
                else if (powerStatus.BatteryLevel === "High")
                {
                    return "\ue91b";
                }
                else if (powerStatus.BatteryLevel === "Medium")
                {
                    return "\ue91c";
                }
                else if (powerStatus.BatteryLevel === "Low")
                {
                    return "\ue91e";
                }
                else if (powerStatus.BatteryLevel === "Flat")
                {
                    return "\ue91d";
                }
                return "";
            }
            color: {
                if (powerStatus === undefined)
                {
                    return colorMap.statusIconText2;
                }
                else if (powerStatus.BatteryLevel === "Full")
                {
                    return colorMap.statusIconText1;
                }
                else if ( (powerStatus.BatteryLevel === "High") ||
                         (powerStatus.BatteryLevel === "Medium") )
                {
                    if (!powerStatus.IsAcPowered)
                    {
                        return colorMap.statusIconText1;
                    }
                }
                else if (powerStatus.BatteryLevel === "Low")
                {
                    return colorMap.statusIconBatteryOrange;
                }
                else if ( (powerStatus.BatteryLevel === "Flat") ||
                         (powerStatus.BatteryLevel === "Dead") ||
                         (powerStatus.BatteryLevel === "Critical") )
                {
                    return colorMap.statusIconBatteryRed;
                }
                return colorMap.statusIconText2;
            }
        }

        Text {
            id: iconAcPowered
            visible: (powerStatus !== undefined) && (powerStatus.IsAcPowered)
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: parent.height * 0.55
            font.family: fontIcon.name
            text: "\ue91f"
            color: colorMap.statusIconText1

            onVisibleChanged: {
                if (workflowState !== "STATE_INACTIVE")
                {
                    if (visible)
                    {
                        soundPlayer.playACConnect();
                    }
                    else
                    {
                        soundPlayer.playACDisconnect();
                    }
                }
            }
        }

        Text {
            id: iconAcCharging
            visible: {
                if ( (!aqdStatus.visible) &&
                        (powerStatus !== undefined) &&
                        (powerStatus.IsAcPowered) &&
                        ( (powerStatus.BatteryLevel === "Critical") ||
                         (powerStatus.BatteryLevel === "Dead") ||
                         (powerStatus.BatteryLevel === "Flat") ||
                         (powerStatus.BatteryLevel === "Low") ||
                         (powerStatus.BatteryLevel === "Medium") ||
                         (powerStatus.BatteryLevel === "High") ) )
                {
                    return true;
                }
                return false;
            }

            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: parent.height * 0.55
            font.family: fontIcon.name
            text: "\ue920"
            color: colorMap.statusIconText1
        }

        Text {
            id: iconBatteryState
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: parent.height * 0.5
            font.family: fontIcon.name
            color: colorMap.statusIconRed
            text: {
                if (powerStatus === undefined)
                {
                    return ""
                }
                else if ( (powerStatus.BatteryLevel === "Unknown") ||
                         (powerStatus.BatteryLevel === "NoBattery") ||
                         (powerStatus.BatteryLevel === "Invalid") )
                {
                    return "\ue914";
                }
                return "";
            }
        }

        Item {
            // aqd status icon to be placed in the middle of the battery icon
            id: aqdStatus
            anchors.fill: parent
            visible: false
            Text {
            anchors.fill: parent
            anchors.topMargin: 16
                id: aqdStatusIconBackground
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: parent.height * 0.26
                font.family: fontAwesome.name
                text: "\uf111"
                color: colorMap.mainBackground
            }

            Text {
            anchors.fill: parent
            anchors.topMargin: 16
                id: aqdStatusIcon
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: parent.height * 0.26
                font.family: fontAwesome.name
                text: "\uf055"
                color: colorMap.statusIconBatteryAQD
            }
        }
    }

    Text {
        id: textBatteryPercent
        anchors.left: iconBatteryContainer.right
        anchors.right: parent.right
        topPadding: height * 0.1
        height: parent.height
        font.family: fontRobotoLight.name
        font.pixelSize: height * 0.27
        verticalAlignment: Text.AlignVCenter
        color: colorMap.text01
        text: {
            if (powerStatus === undefined)
            {
                return ""
            }
            return powerStatus.BatteryCharge + translate("T_Units_%");
        }
    }

    WarningIcon {
        id: iconBatteryWarning
        visible: {
            if ( (powerStatus !== undefined) &&
                 (!powerStatus.IsAcPowered) &&
                 ( (powerStatus.BatteryLevel === "Dead") ||
                   (powerStatus.BatteryLevel === "Critical") ) )
            {
                return true;
            }
            return false;
        }

        height: parent.height * 0.24
        anchors.horizontalCenter: iconBatteryContainer.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: parent.height * 0.45
    }

    onPowerStatusChanged: {
        if (powerStatus === undefined)
        {
            return;
        }

        // Update BatteryLevel
        var blinkRequired = false;

        if ( (powerStatus.BatteryLevel === "Flat") ||
             (powerStatus.BatteryLevel === "Dead") ||
             (powerStatus.BatteryLevel === "Critical") )
        {
            if (!powerStatus.IsAcPowered)
            {
                blinkRequired = true;
            }
        }

        if (blinkRequired)
        {
            if (!animationPowerIconBlink.running)
            {
                animationPowerIconBlink.start();
            }
        }
        else
        {
            animationPowerIconBlink.stop();
            root.opacity = 1;
        }
    }

    onBtnClicked: {
        var infoText = "";

        if (powerStatus.BatteryLevel === "NoBattery")
        {
            infoText += translate("T_StatusDetail_PowerType_Main");
        }
        else if (aqdStatus.visible)
        {
            infoText += translate("T_StatusDetail_PowerBatteryLevel_" + powerStatus.BatteryLevel);
            infoText += "\n" + translate("T_StatusDetail_PowerStatus_AqdInProgress");
        }
        else
        {
            infoText += translate("T_StatusDetail_PowerBatteryLevel_" + powerStatus.BatteryLevel);
            if (powerStatus.IsAcPowered)
            {
                infoText += " " + translate("T_StatusDetail_PowerStatus_IsAcPowered");
            }
        }

        statusIconInfoPanel.open(infoText);
    }
}
