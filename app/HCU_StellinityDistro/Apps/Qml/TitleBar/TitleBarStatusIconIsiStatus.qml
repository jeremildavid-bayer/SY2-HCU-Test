import QtQuick 2.12
import "../Widgets"

GenericButton {
    property var scannerInterlocks: dsExam.scannerInterlocks
    property var cruLinkStatus: dsCru.cruLinkStatus

    visible: {
        if ( (hcuBuildType != "PROD") &&
             (isiIcon.text != "") &&
             (scannerInterlocks !== undefined) &&
             ( (scannerInterlocks.InterfaceStatus === "Enabled") ||
               (scannerInterlocks.InterfaceStatus === "Active") ||
               (scannerInterlocks.InterfaceStatus === "Waiting") ) )
        {
            return true;
        }
        return false;
    }

    Text {
        id: isiArrowIcon
        height: parent.height
        width: parent.width * 0.2
        horizontalAlignment: Text.AlignRight
        verticalAlignment: Text.AlignVCenter
        font.pixelSize: parent.height * 0.6
        font.family: fontIcon.name
        text: {
            if ( (cruLinkStatus !== undefined) &&
                 (cruLinkStatus.State === "Active") )
            {
                if ( (scannerInterlocks !== undefined) &&
                     (scannerInterlocks.InterfaceStatus === "Active") )
                {
                    if ( (scannerInterlocks.InterfaceMode === "Control") ||
                         (scannerInterlocks.InterfaceMode === "Synchronization") )
                    {
                        // Dual arrows
                        return "\ue90c";
                    }
                    else if (scannerInterlocks.InterfaceMode === "Tracking")
                    {
                        // One arrow
                        return "\ue90d";
                    }
                }
            }
            return "";
        }

        color: colorMap.statusIconText1
    }

    Item {
        id: isiIconFrame
        anchors.left: isiArrowIcon.right
        anchors.leftMargin: parent.width * 0.05
        anchors.right: parent.right
        height: parent.height

        Text {
            id: isiIcon
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: parent.height * 0.6
            font.family: fontIcon.name
            text: {
                if ( (cruLinkStatus === undefined) ||
                     (scannerInterlocks === undefined) )
                {
                    return "";
                }
                else if (cruLinkStatus.State === "Active")
                {
                    if (scannerInterlocks.InterfaceStatus === "Active")
                    {
                        if (scannerInterlocks.ScannerReady)
                        {
                            return "\ue90b";
                        }
                        else
                        {
                            return "\ue90e";
                        }
                    }
                    else if (scannerInterlocks.InterfaceStatus === "Waiting")
                    {
                        return "\ue90b";
                    }
                    else if (scannerInterlocks.InterfaceStatus === "Enabled")
                    {
                        return "\ue90b";
                    }
                }
                else
                {
                    if ( (scannerInterlocks.InterfaceStatus === "Enabled") ||
                         (scannerInterlocks.InterfaceStatus === "Active") ||
                         (scannerInterlocks.InterfaceStatus === "Waiting") )
                    {
                        return "\ue90b";
                    }
                }
                return "";
            }

            color: {
                if ( (cruLinkStatus === undefined) ||
                     (scannerInterlocks === undefined) )
                {
                    return colorMap.statusIconText2;
                }
                else if (cruLinkStatus.State === "Active")
                {
                    if ( (scannerInterlocks.InterfaceStatus === "Active") ||
                         (scannerInterlocks.InterfaceStatus === "Waiting") )
                    {
                        return colorMap.statusIconText1;
                    }
                }
                return colorMap.statusIconText2;
            }
        }

        Text {
            id: isiStateIcon
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: parent.height * 0.5
            font.family: fontIcon.name
            color: colorMap.statusIconRed
            text: {
                if ( (cruLinkStatus === undefined) ||
                     (scannerInterlocks === undefined) )
                {
                    return "";
                }
                else if (cruLinkStatus.State === "Active")
                {
                    if (scannerInterlocks.InterfaceStatus === "Active")
                    {
                        if (!scannerInterlocks.ScannerReady)
                        {
                            return "\ue90f";
                        }
                    }
                    else if (scannerInterlocks.InterfaceStatus === "Waiting")
                    {
                        return "\ue914";
                    }
                    else if (scannerInterlocks.InterfaceStatus === "Enabled")
                    {
                        return "\ue913";
                    }
                }
                else
                {
                    if ( (scannerInterlocks.InterfaceStatus === "Enabled") ||
                         (scannerInterlocks.InterfaceStatus === "Active") ||
                         (scannerInterlocks.InterfaceStatus === "Waiting") )
                    {
                        return "\ue913";
                    }
                }
                return "";
            }
        }
    }

    onBtnClicked: {
        var infoText;

        if (scannerInterlocks.InterfaceStatus === "Active")
        {
            var phrase1 = translate(scannerInterlocks.ScannerReady ? "T_StatusDetail_ScannerInterlocks_ScannerReady" : "T_StatusDetail_ScannerInterlocks_ScannerNotReady");
            var phrase2 = translate("T_StatusDetail_ScannerInterlocks_Mode_" + scannerInterlocks.InterfaceMode);
            var phrase3 = "";

            if (scannerInterlocks.ArmLockedOut)
            {
                phrase3 += translate("T_StatusDetail_ScannerInterlocks_ArmLockedOut") + " ";
            }
            if (scannerInterlocks.ConfigLockedOut)
            {
                phrase3 += translate("T_StatusDetail_ScannerInterlocks_ConfigLockedOut") + " ";
            }
            if (scannerInterlocks.ResumeLockedOut)
            {
                phrase3 += translate("T_StatusDetail_ScannerInterlocks_ResumeLockedOut") + " ";
            }
            if (scannerInterlocks.StartLockedOut)
            {
                phrase3 += translate("T_StatusDetail_ScannerInterlocks_StartLockedOut") + " ";
            }
            infoText = phrase1 + " " + phrase2 + " " + phrase3;
        }
        else
        {
            infoText = translate("T_StatusDetail_ScannerInterlocks_NotActive");
        }

        statusIconInfoPanel.open(infoText);
    }
}
