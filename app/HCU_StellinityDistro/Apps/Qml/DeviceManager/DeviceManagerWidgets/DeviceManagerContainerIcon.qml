import QtQuick 2.12
import ".."
import "../../Widgets"

DeviceManagerBase {
    Row {
        id: bottleGroup
        anchors.top: parent.top
        anchors.left: parent.left
        spacing: parent.width * 0.03
        height: parent.height * 0.155

        DeviceManagerBottleIcon {
            height: parent.height
            syringeIndex: 0
            bottleColor: colorMap.saline
            fluidSourceBottle: fluidSourceBottle1
            isAlertActive: activeAlertsBottle1.length > 0
        }
        DeviceManagerBottleIcon {
            height: parent.height
            syringeIndex: 1
            bottleColor: colorMap.contrast1
            fluidSourceBottle: fluidSourceBottle2
            isAlertActive: activeAlertsBottle2.length > 0
        }
        DeviceManagerBottleIcon {
            height: parent.height
            syringeIndex: 2
            bottleColor: fluidC2Color
            fluidSourceBottle: fluidSourceBottle3
            isAlertActive: activeAlertsBottle3.length > 0
        }
    }

    DeviceManagerMudsLineIcon {
        id: mudsLine
        anchors.left: bottleGroup.left
        anchors.top: bottleGroup.bottom
        anchors.topMargin: parent.height * 0.022
        width: parent.width * 0.8
        height: parent.height * 0.55
    }

    DeviceManagerMudsIcon {
        id: muds
        spacing: bottleGroup.spacing
        anchors.top: bottleGroup.bottom
        anchors.topMargin: parent.height * 0.07
        anchors.left: bottleGroup.left
        height: parent.height * 0.447
    }

    LoadingGif {
        x: muds.x
        y: muds.y
        width: muds.width
        height: muds.height
        visible: {
            if ( (fluidSourceMuds === undefined) ||
                 (fluidSourceSyringe1 === undefined) ||
                 (fluidSourceSyringe2 === undefined) ||
                 (fluidSourceSyringe3 === undefined) )
            {
                return false;
            }

            if ( (fluidSourceMuds.IsBusy) ||
                 (fluidSourceSyringe1.IsBusy) ||
                 (fluidSourceSyringe2.IsBusy) ||
                 (fluidSourceSyringe3.IsBusy) )
            {
                return true;
            }
            return false;
        }
    }

    DeviceManagerWCIcon {
        id: wasteBin
        height: parent.height * 0.16
        width: parent.height * 0.188
        anchors.top: mudsLine.bottom
        anchors.topMargin: parent.height * 0.02
        anchors.left: bottleGroup.left
    }

    DeviceManagerSudsIcon {
        id: sudsIcon
        height: parent.height * 0.165
        anchors.top: wasteBin.top
        anchors.horizontalCenter: muds.horizontalCenter
    }

    Item {
        id: bottleLabels
        anchors.top: sudsIcon.bottom
        anchors.topMargin: parent.height * 0.06
        width: parent.width
        height: parent.height * 0.16
        visible: displaySourcePackageInfo

        Item {
            id: frameBottleC1
            anchors.left: parent.left
            anchors.leftMargin: -parent.width * 0.03
            width: parent.width
            height: (fluidSourceBrandC1 == "") ? 0 : parent.height * 0.5
            visible: fluidSourceBrandC1 != ""

            Text {
                id: iconBottleC1
                width: contentWidth * 1.5
                height: parent.height
                font.family: fontIcon.name
                text: "\ue92f"
                font.pixelSize: height * 0.6
                color: colorMap.contrast1
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            LabelAndText {
                anchors.left: iconBottleC1.right
                anchors.right: parent.right
                height: parent.height
                maximumLabelWidth: width * 0.8

                widgetLabel.text: fluidSourceBrandC1
                widgetLabel.color: colorMap.text01
                widgetLabel.font.pixelSize: height * 0.7
                widgetLabel.font.family: fontRobotoLight.name

                widgetText.text: (fluidSourceConcentrationC1 == 0) ? "" : fluidSourceConcentrationC1.toString()
                widgetText.color: colorMap.text01
                widgetText.font.pixelSize: height * 0.7
                widgetText.font.family: fontRobotoBold.name
            }
        }

        Item {
            id: frameBottleC2
            anchors.top: frameBottleC1.bottom
            anchors.left: parent.left
            anchors.leftMargin: frameBottleC1.anchors.leftMargin
            width: frameBottleC1.width
            height: (fluidSourceBrandC2 == "") ? 0 : parent.height * 0.5
            visible: {
                if (sameContrasts)
                {
                    return (fluidSourceBrandC1 == "");
                }
                return (fluidSourceBrandC2 != "");
            }

            Text {
                id: iconBottleC2
                width: contentWidth * 1.5
                height: parent.height
                font.family: fontIcon.name
                text: "\ue92f"
                font.pixelSize: height * 0.6
                color: fluidC2Color
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            LabelAndText {
                anchors.left: iconBottleC2.right
                anchors.right: parent.right
                height: parent.height
                maximumLabelWidth: width * 0.8

                widgetLabel.text: fluidSourceBrandC2
                widgetLabel.color: colorMap.text01
                widgetLabel.font.pixelSize: height * 0.7
                widgetLabel.font.family: fontRobotoLight.name

                widgetText.text: (fluidSourceConcentrationC2 == 0) ? "" : fluidSourceConcentrationC2.toString()
                widgetText.color: colorMap.text01
                widgetText.font.pixelSize: height * 0.7
                widgetText.font.family: fontRobotoBold.name
            }
        }
    }
}
