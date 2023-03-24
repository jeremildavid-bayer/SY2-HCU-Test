import QtQuick 2.12
import "../Widgets"
import "../Widgets/Popup"

Popup {
    property var selectedContrast: dsExam.selectedContrast
    property var injectionRequestProcessStatus: dsExam.injectionRequestProcessStatus
    property var plan: dsExam.plan

    type: "SPECIAL_WARNING"
    titleText: "T_InsufficientVolume"
    heightMin: dsCfgLocal.screenH * 0.55

    // OtherBtn will be used for "Yes" to swap "Yes" and "No" buttons
    showOtherBtn: true
    enableOtherBtn: true
    showOkBtn: false
    enableOkBtn: false

    otherBtnText: "T_Yes"
    cancelBtnText: "T_No"


    content: [
        Text {
            y: parent.height * 0.05
            width: parent.width
            height: parent.height * 0.2
            text: translate("T_ThereIsNotEnoughFluidForThisInjection.")
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            color: colorMap.blk01
            font.pixelSize: contentfontPixelSize
            font.family: fontRobotoLight.name
        },

        Row {
            y: parent.height * 0.4
            height: parent.height * 0.15
            anchors.horizontalCenter: parent.horizontalCenter
            width: textRequired.width + (frameContrastNeedsAdjust.visible ? frameContrastNeedsAdjust.width : 0) + (frameSalineNeedsAdjust.visible ? frameSalineNeedsAdjust.width : 0) + (spacing * 2)
            spacing: parent.width * 0.03

            Text {
                id: textRequired
                width: contentWidth
                height: parent.height
                text: translate("T_Required") + translate("T_:")
                verticalAlignment: Text.AlignVCenter
                color: colorMap.blk01
                font.pixelSize: contentfontPixelSize
                font.family: fontRobotoLight.name
            }

            Row {
                id: frameContrastNeedsAdjust
                width: iconContrast.width + txtContrastRequired.width + txtContrastUnit.width + (spacing * 2)
                height: parent.height
                spacing: parent.spacing * 0.21

                Text {
                    id: iconContrast
                    width: contentWidth
                    height: parent.height
                    verticalAlignment: Text.AlignVCenter
                    font.family: fontIcon.name
                    text: "\ue92f"
                    font.pixelSize: height * 0.9
                    color: colorMap.contrast1
                }
                Text {
                    id: txtContrastRequired
                    width: contentWidth
                    height: parent.height
                    text: "100"
                    horizontalAlignment: Text.AlignRight
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: contentfontPixelSize * 1.4
                    color: colorMap.blk01
                    font.family: fontRobotoBold.name
                }
                Text {
                    id: txtContrastUnit
                    y: parent.height * 0.15
                    width: contentWidth
                    height: parent.height - y
                    text: translate("T_Units_ml")
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: contentfontPixelSize
                    color: colorMap.gry01
                    font.family: fontRobotoLight.name
                }
            }

            Row {
                id: frameSalineNeedsAdjust
                width: iconSaline.width + txtSalineRequired.width + txtSalineUnit.width + (spacing * 2)
                height: parent.height
                spacing: parent.spacing * 0.21

                Text {
                    id: iconSaline
                    width: contentWidth
                    height: parent.height
                    verticalAlignment: Text.AlignVCenter
                    font.family: fontIcon.name
                    text: "\ue930"
                    font.pixelSize: height * 0.9
                    color: colorMap.saline
                }
                Text {
                    id: txtSalineRequired
                    width: contentWidth
                    height: parent.height
                    text: "100"
                    horizontalAlignment: Text.AlignRight
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: contentfontPixelSize * 1.4
                    color: colorMap.blk01
                    font.family: fontRobotoBold.name
                }
                Text {
                    id: txtSalineUnit
                    y: parent.height * 0.15
                    width: contentWidth
                    height: parent.height - y
                    text: translate("T_Units_ml")
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: contentfontPixelSize
                    color: colorMap.gry01
                    font.family: fontRobotoLight.name
                }
            }
        },
        Text {
            id: txtAutoAdjustInstruction
            y: parent.height * 0.75
            width: parent.width
            height: parent.height * 0.2
            text: translate("T_AutomaticallyAdjustTheProtocol")
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            color: colorMap.blk01
            font.pixelSize: contentfontPixelSize
            font.family: fontRobotoLight.name
        }
    ]

    // This is "Yes" button
    onBtnOtherClicked: {
        dsExam.slotAdjustInjectionVolume();
        close();
    }

    onBtnCancelClicked: {
        close();
    }

    onInjectionRequestProcessStatusChanged: {
        if (injectionRequestProcessStatus === undefined)
        {
            return;
        }

        if ( (injectionRequestProcessStatus.State.indexOf("T_ARMFAILED_InsufficientVolume") >= 0) &&
                // There is another alert that shares same string and will be detected by this logic. Ensure it doesn't catch that alert
             (injectionRequestProcessStatus.State.indexOf("T_ARMFAILED_InsufficientVolumeForSteps") === -1) &&
             (injectionRequestProcessStatus.RequestedByHcu) )
        {
            var stateStrList = injectionRequestProcessStatus.State.split(" ");
            if (stateStrList.length === 2)
            {
                var dataStr = stateStrList[1];
                var args = dataStr.split(";");

                if (args.length !== 3)
                {
                    logError("PopupArmFluidVolumeAdjust: Bad ARM State = " + injectionRequestProcessStatus.State);
                    return;
                }

                var salineNeededVol = Math.ceil(args[0]);
                var contrastNeededVol = Math.ceil(args[1]);
                var autoAdjustVolPossible = (args[2] === "true");

                reloadRequiredFluidVolumes(salineNeededVol, contrastNeededVol);

                showOtherBtn = autoAdjustVolPossible;
                cancelBtnText = autoAdjustVolPossible ? "T_No" : "T_OK"
                txtAutoAdjustInstruction.visible = autoAdjustVolPossible;
                open();
            }
        }
        else
        {
            close();
        }
    }

    onPlanChanged: {
        if (isOpen())
        {
            // Plan changed outside (possible from CRU). Close popup.
            close();
        }
    }

    function reloadRequiredFluidVolumes(salineNeededVol, contrastNeededVol)
    {
        if (salineNeededVol > 0)
        {
            txtSalineRequired.text = salineNeededVol;
            frameSalineNeedsAdjust.visible = true;
        }
        else
        {
            frameSalineNeedsAdjust.visible = false;
        }

        if (contrastNeededVol > 0)
        {
            txtContrastRequired.text = contrastNeededVol;
            frameContrastNeedsAdjust.visible = true;
            iconContrast.color = (selectedContrast.ColorCode === "GREEN") ? colorMap.contrast1 : colorMap.contrast2;
        }
        else
        {
            frameContrastNeedsAdjust.visible = false;
        }
    }
}

