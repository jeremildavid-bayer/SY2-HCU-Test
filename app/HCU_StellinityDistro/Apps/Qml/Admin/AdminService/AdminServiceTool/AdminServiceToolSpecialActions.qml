import QtQuick 2.12
import "../../../Widgets"
import "../../../Widgets/Popup"

Rectangle {
    property var syringeStates: dsMcu.syringeStates
    property string hcuBuildType: dsSystem.hcuBuildType
    property int btnWidth: width * 0.2
    property int btnHeight: height * 0.2
    property bool mudsInserted: dsMcu.mudsInserted

    id: root
    anchors.fill: parent
    color: "transparent"

    Column {
        x: root.width * 0.02
        y: root.height * 0.05
        spacing: root.height * 0.04

        Row {
            spacing: root.width * 0.05

            GenericIconButton {
                width: btnWidth
                height: btnHeight
                iconText: "MCU\nRESET"
                iconFontPixelSize: height * 0.23
                iconColor: colorMap.actionButtonText
                color: colorMap.actionButtonBackground
                onBtnClicked: {
                    dsMcu.slotResetMcu();
                }
            }

            GenericIconButton {
                width: btnWidth
                height: btnHeight
                iconText: "STOPCOCK\nRESET"
                iconFontPixelSize: height * 0.23
                iconColor: colorMap.actionButtonText
                color: colorMap.actionButtonBackground
                onBtnClicked: {
                    dsMcu.slotResetStopcock();
                }
            }

            GenericIconButton {
                width: btnWidth
                height: btnHeight
                iconText: "PM DONE"
                iconFontPixelSize: height * 0.23
                iconColor: colorMap.actionButtonText
                color: colorMap.statusIconRed
                onBtnClicked: {
                    popupManager.popupActionConfirmation.titleText = "Preventative Maintenance Performed";
                    popupManager.popupActionConfirmation.contentText = "Reset the next PM Due date to be 1 year from today?";
                    popupManager.popupActionConfirmation.okFunction = dsCfgLocalCpp.slotSetLastPMPerformedAtToNow;
                    popupManager.popupActionConfirmation.okFunctionArgumentsList = [];
                    popupManager.popupActionConfirmation.open();
                }
            }
        }

        Row {
            spacing: root.width * 0.05

            GenericIconButton {
                width: btnWidth
                height: btnHeight
                iconText: "HEATER\nON"
                iconFontPixelSize: height * 0.23
                iconColor: colorMap.actionButtonText
                color: colorMap.actionButtonBackground
                captionColor: colorMap.actionButtonText
                captionText: mudsInserted ? "" : "MUDS Missing!"
                onBtnClicked: {
                    dsCfgLocal.slotHeaterActive(true);
                }
            }

            GenericIconButton {
                width: btnWidth
                height: btnHeight
                iconText: "HEATER\nOFF"
                iconFontPixelSize: height * 0.23
                iconColor: colorMap.actionButtonText
                color: colorMap.actionButtonBackground
                onBtnClicked: {
                    dsCfgLocal.slotHeaterActive(false);
                }
            }

            GenericIconButton {
                width: btnWidth
                height: btnHeight
                iconText: "DOOR\nLOCK"
                iconFontPixelSize: height * 0.23
                iconColor: colorMap.actionButtonText
                color: colorMap.actionButtonBackground
                onBtnClicked: {
                    dsMcu.slotDoorLock(true);
                }
            }

            GenericIconButton {
                width: btnWidth
                height: btnHeight
                iconText: "DOOR\nUNLOCK"
                iconFontPixelSize: height * 0.23
                iconColor: colorMap.actionButtonText
                color: colorMap.actionButtonBackground
                onBtnClicked: {
                    dsMcu.slotDoorLock(false);
                }
            }
        }

        Row {
            spacing: root.width * 0.05

            GenericIconButton {
                width: btnWidth
                height: btnHeight
                iconText: "FACTORY\nDEFAULT"
                iconFontPixelSize: height * 0.23
                iconColor: colorMap.actionButtonText
                color: colorMap.actionButtonBackground
                onBtnClicked: {
                    if (mudsInserted)
                    {
                        popupManager.popupEjectMudsRequired.open();
                    }
                    else
                    {
                        popupFactoryReset.open();
                    }
                }
            }

            GenericIconButton {
                width: btnWidth
                height: btnHeight
                iconText: "SAVE\nUSER DATA"
                iconFontPixelSize: height * 0.23
                iconColor: colorMap.actionButtonText
                color: colorMap.actionButtonBackground
                onBtnClicked: {
                    dsSystem.slotSaveUserData();
                }
            }

            GenericIconButton {
                width: btnWidth
                height: btnHeight
                iconText: "PULL OUT\nPLUNGERS"
                iconFontPixelSize: height * 0.22
                iconColor: colorMap.actionButtonText
                color: colorMap.actionButtonBackground
                onBtnClicked: {
                    popupPullPlungers.open();
                }
            }

            GenericIconButton {
                width: btnWidth
                height: btnHeight
                iconText: "TEST\nBASE FAN"
                iconFontPixelSize: height * 0.22
                iconColor: colorMap.actionButtonText
                color: colorMap.actionButtonBackground
                onBtnClicked: {
                    // turn on base fan by setting trigger temp to 0 degrees so it forces the fan to be on
                    dsMcu.slotSetBaseFanTemperature(0);
                    // revert back after 10 seconds
                    // MCU will re-initialize this value ater reboots as well
                    timerSingleShot(10000, function() {
                        dsMcu.slotSetBaseFanTemperature(40);
                    });
                }
            }
        }

        Row {
            spacing: root.width * 0.05

            GenericIconButton {
                width: btnWidth
                height: btnHeight
                iconText: "RENEW\nMUDS"
                iconFontPixelSize: height * 0.23
                iconColor: colorMap.actionButtonText
                color: colorMap.actionButtonBackground
                onBtnClicked: {
                    popupMudsResetWarning.contentText = "Please check that all plungers are engaged and the stopcocks are in the desired position.";
                    popupMudsResetWarning.showCancelBtn = true;
                    popupMudsResetWarning.showOkBtn = true;
                    popupMudsResetWarning.open();
                }
            }

            GenericIconButton {
                width: btnWidth
                height: btnHeight
                iconText: "SAFE RENEW\nMUDS"
                iconFontPixelSize: height * 0.23
                iconColor: colorMap.actionButtonText
                color: colorMap.actionButtonBackground
                onBtnClicked: {
                    dsAlert.slotActivateAlert("InitiatingSafeRenewMuds", "service");
                    popupManager.popupSafeRenewMuds.open();
                }
            }
        }
    }

    PopupMessage {
        id: popupMudsResetWarning
        type: "PLAIN"
        translationRequired: false
        visible: false
        onBtnOkClicked: {
            popupMudsResetWarning.showCancelBtn = false;
            popupMudsResetWarning.showOkBtn = false;
            dsMcu.slotResetMuds();
            popupMudsResetWarning.contentText = "MUDS Reset in progress..";
            close();
        }
        onBtnCancelClicked: {
            close();
        }
    }

    PopupMessage {
        id: popupFactoryReset
        translationRequired: false
        type: "WARNING"
        titleText: "WARNING"
        contentText: "All data will be lost and will reboot the system.\nReady to continue?"
        visible: false
        onBtnOkClicked: {
            dsSystem.slotFactoryReset();
            close();
        }
        onBtnCancelClicked: {
            close();
        }
    }

    PopupMessage {
        id: popupPullPlungers
        translationRequired: false
        type: "WARNING"
        titleText: "WARNING"
        contentText: "Plungers will be pulled out of syringes. Fluid may leak everywhere. Do you wish to continue?"
        visible: false
        okBtnText: "Yes"
        onBtnOkClicked: {
            dsMcu.slotPullPlungers();
            close();
        }
        onBtnCancelClicked: {
            close();
        }
    }

    onSyringeStatesChanged: {
        if (popupMudsResetWarning.visible)
        {
            var mudsResetCompleted = true;
            for (var i = 0; i < syringeStates.length; i++)
            {
                if (syringeStates[i] !== "COMPLETED")
                {
                    mudsResetCompleted = false;
                    break;
                }
            }

            if (mudsResetCompleted)
            {
                popupMudsResetWarning.contentText = "MUDS Reset completed";
                popupMudsResetWarning.showOkBtn = true;
            }
        }
    }


    Component.onCompleted:
    {
        appMain.screenStateChanged.connect(slotScreenStateChanged);
        slotScreenStateChanged();
    }

    Component.onDestruction: {
        appMain.screenStateChanged.disconnect(slotScreenStateChanged);
    }

    function slotScreenStateChanged()
    {
        visible = (appMain.screenState === "Admin-Service-Tool-SpecialActions");
        reload();
    }

    function reload()
    {
        if (!visible)
        {
            return;
        }
    }
}

