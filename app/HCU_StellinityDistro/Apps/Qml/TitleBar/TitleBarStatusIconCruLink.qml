import QtQuick 2.12
import "../Widgets"

GenericButton {
    property var cruLinkStatus: dsCru.cruLinkStatus
    property string cruLinkType: dsCfgLocal.cruLinkType
    property string hcuBuildType: dsSystem.hcuBuildType

    visible: (hcuBuildType != "PROD")

    Text {
        id: iconCruWiredLink
        visible: (cruLinkType === "WiredEthernet")
        horizontalAlignment: Text.AlignHCenter
        y: parent.height * 0.06
        width: parent.width
        font.pixelSize: parent.height * 0.7
        font.family: fontIcon.name
        text: "\ue925"
        color: {
            if (cruLinkType === "WiredEthernet")
            {
                if ( (cruLinkStatus !== undefined) &&
                     (cruLinkStatus.State === "Active") )
                {
                    return colorMap.statusIconText1;
                }
            }
            return colorMap.statusIconText2;
        }
    }

    Text {
        id: iconCruWirelessLinkSegment1
        visible: (cruLinkType === "WirelessEthernet")
        horizontalAlignment: Text.AlignHCenter
        y: parent.height * 0.11
        width: parent.width
        font.pixelSize: parent.height * 0.65
        font.family: fontIcon.name
        text: "\ue921"
        color: {
            if (cruLinkType === "WirelessEthernet")
            {
                if (cruLinkStatus === undefined)
                {
                    return colorMap.statusIconText2;
                }
                else if (cruLinkStatus.State === "Active")
                {
                    return colorMap.statusIconText1;
                }
            }
            return colorMap.statusIconText2;
        }
    }

    Text {
        id: iconCruWirelessLinkSegment2
        visible: iconCruWirelessLinkSegment1.visible
        width: parent.width
        horizontalAlignment: Text.AlignHCenter
        y: iconCruWirelessLinkSegment1.y
        font.pixelSize: iconCruWirelessLinkSegment1.font.pixelSize
        font.family: fontIcon.name
        text: "\ue922"
        color: {
            if (cruLinkType === "WirelessEthernet")
            {
                if (cruLinkStatus === undefined)
                {
                    return colorMap.statusIconText2;
                }
                else if (cruLinkStatus.State === "Active")
                {
                    if ( (cruLinkStatus.Quality === "Excellent") ||
                         (cruLinkStatus.Quality === "Good") ||
                         (cruLinkStatus.Quality === "Fair") )
                    {
                        return colorMap.statusIconText1;
                    }
                }
            }
            return colorMap.statusIconText2;
        }
    }

    Text {
        id: iconCruWirelessLinkSegment3
        visible: iconCruWirelessLinkSegment1.visible
        width: parent.width
        horizontalAlignment: Text.AlignHCenter
        y: iconCruWirelessLinkSegment1.y
        font.pixelSize: iconCruWirelessLinkSegment1.font.pixelSize
        font.family: fontIcon.name
        text: "\ue923"
        color: {
            if (cruLinkType === "WirelessEthernet")
            {
                if (cruLinkStatus === undefined)
                {
                    return colorMap.statusIconText2;
                }
                else if (cruLinkStatus.State === "Active")
                {
                    if ( (cruLinkStatus.Quality === "Excellent") ||
                         (cruLinkStatus.Quality === "Good") )
                    {
                        return colorMap.statusIconText1;
                    }
                }
            }
            return colorMap.statusIconText2;
        }
    }

    Text {
        id: iconCruWirelessLinkSegment4
        visible: iconCruWirelessLinkSegment1.visible
        width: parent.width
        horizontalAlignment: Text.AlignHCenter
        y: iconCruWirelessLinkSegment1.y
        font.pixelSize: iconCruWirelessLinkSegment1.font.pixelSize
        font.family: fontIcon.name
        text: "\ue924"
        color: {
            if (cruLinkType === "WirelessEthernet")
            {
                if (cruLinkStatus === undefined)
                {
                    return colorMap.statusIconText2;
                }
                else if (cruLinkStatus.State === "Active")
                {
                    if (cruLinkStatus.Quality === "Excellent")
                    {
                        return colorMap.statusIconText1;
                    }
                }
            }
            return colorMap.statusIconText2;
        }
    }

    Text {
        id: iconCruLinkState
        anchors.fill: parent
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.pixelSize: parent.height * 0.5
        font.family: fontIcon.name
        color: colorMap.statusIconRed
        text: {
            if (cruLinkStatus === undefined)
            {
                return "";
            }

            if (cruLinkStatus.State === "Active")
            {
                return "";
            }
            else if (cruLinkStatus.State === "Recovering")
            {
                return "\ue914";
            }
            return "\ue913";
        }
    }

    onBtnClicked: {
        var phrase1 = translate("T_StatusDetail_LinkType_" + cruLinkType);
        var phrase2 = translate("T_StatusDetail_LinkStatus_" + cruLinkStatus.State);
        var phase3 = cruLinkStatus.SignalLevel;
        var infoText = phrase1 + " " + phrase2 + " " + phase3;
        statusIconInfoPanel.open(infoText);
    }
}
