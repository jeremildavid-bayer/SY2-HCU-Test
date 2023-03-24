import QtQuick 2.12
import "../Widgets"
import "../Util.js" as Util

Item {
    property int iconSpacing: 0

    width: btnMcuLink.x + btnMcuLink.width

    TitleBarStatusIconDateTime {
        id: dateTimeFrame
        width: dsCfgLocal.screenW * 0.11
        anchors.top: parent.top
        anchors.topMargin: parent.height * 0.06
        anchors.bottom: parent.bottom
    }

    TitleBarStatusIconTradeshowMode {
        id: tradeshowModeIcon
        width: visible ? dsCfgLocal.screenW * 0.025 : 0
        height: parent.height
        anchors.left: dateTimeFrame.right
    }

    TitleBarStatusIconIsiStatus {
        id: btnIsi
        width: visible ? dsCfgLocal.screenW * 0.048 : 0
        height: parent.height
        anchors.left: tradeshowModeIcon.right
        anchors.leftMargin: tradeshowModeIcon.visible ? iconSpacing * 2 : 0
    }

    TitleBarStatusIconCruLink {
        id: btnCruLink
        height: parent.height
        width: visible ? dsCfgLocal.screenW * 0.043 : 0
        anchors.left: btnIsi.right
        anchors.leftMargin: btnIsi.visible ? iconSpacing : 0
    }

    TitleBarStatusIconPowerStatus {
        id: btnPowerStatus
        height: parent.height
        width: visible ? dsCfgLocal.screenW * 0.08 : 0
        anchors.left: btnCruLink.right
        anchors.leftMargin: btnCruLink.visible ? iconSpacing : 0
    }

    TitleBarStatusIconMcuLink {
        id: btnMcuLink
        height: parent.height
        width: visible ? dsCfgLocal.screenW * 0.035 : 0
        anchors.left: btnPowerStatus.right
        anchors.leftMargin:  btnPowerStatus.visible ? iconSpacing : 0

    }
}
