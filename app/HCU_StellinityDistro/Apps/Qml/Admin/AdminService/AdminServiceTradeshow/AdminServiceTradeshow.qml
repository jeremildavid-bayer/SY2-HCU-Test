import QtQuick 2.12
import "../../../Widgets"
import "../../../Widgets/Popup"

Rectangle {
    property int btnWidth: width * 0.2
    property int btnHeight: height * 0.2

    id: root
    anchors.fill: parent
    color: "transparent"

    Column {
        x: root.width * 0.04
        y: root.height * 0.10
        spacing: root.height * 0.04

        Row {
            spacing: root.width * 0.05

            GenericIconButton {
                width: btnWidth
                height: btnHeight
                iconText: "SAFE RENEW\nMUDS"
                iconFontPixelSize: height * 0.23
                iconColor: colorMap.actionButtonText
                color: colorMap.actionButtonBackground
                onBtnClicked: {
                    dsAlert.slotActivateAlert("InitiatingSafeRenewMuds", "tradeshow");
                    popupManager.popupSafeRenewMuds.open();
                }
            }

            GenericIconButton {
                width: btnWidth
                height: btnHeight
                iconText: "LOAD\nSAMPLE\nCONTRASTS"
                iconFontPixelSize: height * 0.23
                iconColor: colorMap.actionButtonText
                color: colorMap.actionButtonBackground
                onBtnClicked: {
                    dsCfgLocal.slotLoadSampleFluidOptions();
                }
            }

            GenericIconButton {
                width: btnWidth
                height: btnHeight
                iconText: "LOAD\nSAMPLE\nPROTOCOLS"
                iconFontPixelSize: height * 0.23
                iconColor: colorMap.actionButtonText
                color: colorMap.actionButtonBackground
                onBtnClicked: {
                    dsCfgLocal.slotLoadSampleInjectionPlans();
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
        visible = (appMain.screenState === "Admin-Service-Tradeshow");
    }

}
