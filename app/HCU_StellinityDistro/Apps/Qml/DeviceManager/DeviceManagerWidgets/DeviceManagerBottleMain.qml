import QtQuick 2.12
import "../../Widgets"

DeviceManagerGenericButton {
    property var fluidSourceBottle
    property string bottleColor: "transparent"
    property string titleIconText: ""
    property int syringeIndex

    id: root
    isBusy: ((fluidSourceBottle === undefined) || (fluidSourceBottle.IsBusy === undefined)) ? false : fluidSourceBottle.IsBusy

    color: "transparent"
    useDefaultFrameBackground: false

    pressedColor: "transparent"
    pressedCoverContent: [
        Text {
            id: customPressedCover
            anchors.fill: parent
            font.family: fontDeviceIcon.name
            color: Qt.tint(root.color, colorMap.buttonShadow)
            font.pixelSize: height
            text: "\ue909"
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
        }
    ]

    onBtnClicked: {
        barcodeReaderStart();
    }

    content: [
        Item {
            // Selected border widget
            anchors.centerIn: parent
            width: parent.width + (2 * buttonSelectedBorderWidth)
            height: parent.height + (2 * buttonSelectedBorderWidth)
            clip: true

            Text {
                font.family: fontDeviceIcon.name
                anchors.fill: parent
                color: isSelected ? colorMap.deviceIconSelected : "transparent"
                font.pixelSize: height
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                text: "\ue909"
            }
        },
        Item {
            anchors.fill: parent
            clip: true

            Text {
                id: frameBackground
                anchors.centerIn: parent
                font.family: fontDeviceIcon.name
                font.pixelSize: parent.height
                text: "\ue908"
                color: colorMap.deviceButtonBackground
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
            }

            Text {
                id: outer
                anchors.centerIn: parent
                font.family: fontDeviceIcon.name
                font.pixelSize: parent.height
                text: "\ue908"
                color: {
                    if (isAlertActive)
                    {
                        return colorMap.deviceButtonRed;
                    }
                    else if ( (fluidSourceBottle === undefined) ||
                              (fluidSourceBottle.InstalledAt === undefined) ||
                              (fluidSourceBottle.SourcePackages.length === 0) )
                    {
                        return colorMap.deviceButtonBackground;
                    }
                    return colorMap.deviceButtonBackground;
                }

                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
            }

            Item {
                anchors.centerIn: parent
                width: parent.width - (3 * buttonSelectedBorderWidth)
                height: parent.height - (3 * buttonSelectedBorderWidth)
                clip: true
                Text {
                    id: inner
                    font.family: fontDeviceIcon.name
                    anchors.fill: parent
                    color: {
                        if ( (fluidSourceBottle === undefined) ||
                             (fluidSourceBottle.InstalledAt === undefined) ||
                             (fluidSourceBottle.SourcePackages.length === 0) )
                        {
                            return colorMap.deviceIconMissing;
                        }
                        else if (fluidSourceBottle.NeedsReplaced)
                        {
                            return colorMap.deviceIconWarningState;
                        }
                        return bottleColor;
                    }

                    text: "\ue90a"
                    font.pixelSize: height
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                }
            }

            Item {
                id: textColumn
                width: parent.width - (8 * buttonSelectedBorderWidth)
                height: parent.height * 0.95
                anchors.centerIn: parent

                visible: {
                    if ( (fluidSourceBottle === undefined) ||
                         (fluidSourceBottle.InstalledAt === undefined) ||
                         (fluidSourceBottle.SourcePackages.length === 0) )
                    {
                        return false;
                    }
                    return true;
                }

                Text {
                    id: textBrand
                    anchors.top: parent.top
                    anchors.topMargin: -((height - contentHeight) / 2)
                    width: parent.width
                    height: (syringeIndex == 0) ? (parent.height * 0.95) : (parent.height * 0.7)
                    font.family: fontRobotoLight.name
                    text: {
                        if (syringeIndex === 0)
                        {
                            return translate("T_Saline");
                        }
                        else if ( (fluidSourceBottle === undefined) ||
                                  (fluidSourceBottle.SourcePackages === undefined) ||
                                  (fluidSourceBottle.SourcePackages.length === 0) )
                        {
                            return "--";
                        }
                        return fluidSourceBottle.SourcePackages[0].Brand
                    }

                    color: colorMap.white01
                    font.pixelSize: root.height * 0.17
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignBottom
                    elide: Text.ElideRight
                    wrapMode: Text.Wrap
                }

                Text {
                    anchors.top: textBrand.bottom
                    anchors.topMargin: parent.height * 0.02
                    anchors.bottom: parent.bottom
                    width: parent.width
                    font.family: fontRobotoBold.name
                    text: {
                        if ( (fluidSourceBottle === undefined) ||
                             (fluidSourceBottle.SourcePackages === undefined) ||
                             (fluidSourceBottle.SourcePackages.length === 0) ||
                             (syringeIndex == 0) )
                        {
                            return "";
                        }
                        return fluidSourceBottle.SourcePackages[0].Concentration;
                    }

                    color: colorMap.white01
                    font.pixelSize: root.height * 0.18
                    horizontalAlignment: Text.AlignHCenter
                }
            }

            Text {
                anchors.fill: parent
                text: titleIconText
                font.pixelSize: parent.height * 0.5
                font.family: fontIcon.name
                visible: {
                    if ( (fluidSourceBottle === undefined) ||
                         (fluidSourceBottle.InstalledAt === undefined) ||
                         (fluidSourceBottle.SourcePackages.length === 0) )
                    {
                        return true;
                    }
                    return false;
                }

                color: colorMap.deviceButtonBackground
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                opacity: 0.5
            }
        }
    ]
}
