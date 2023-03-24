import QtQuick 2.12
import "../../../Widgets"
import "../../../Util.js" as Util
import "../"

ListView {
    ListFade {
        fadeColor: colorMap.subPanelBackground
        fadeSize: rowHeight * 0.7 // making the fade size slightly bigger so it's easier to see
    }
    ScrollBar {
        color: "gray"
        autoHide: false
    }

    id: root

    delegate: EgfrListItem {
        id: egfrItemButton
        anchors.horizontalCenter: parent.horizontalCenter
        width: ListView.view.width * 0.94
        height: rowHeight
    }

    footer:  Item {
        id: footerItem
        visible: true
        height: rectEgfrItem.visible ? rectEgfrItem.contentHeight : rowHeight
        anchors.left: parent.left
        anchors.leftMargin: parent.width * 0.03
        width: parent.width * 0.94

        Text {
            id: eGFRValueName
            height: rowHeight
            anchors.left: parent.left
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignLeft
            wrapMode: Text.Wrap
            font.pixelSize: rowHeight * 0.35
            font.family: fontRobotoLight.name
            text: translate("T_eGFRValue")
            color: colorMap.text01
            fontSizeMode: Text.Fit
            minimumPixelSize: font.pixelSize * 0.7
        }

        EgfrValue {
            id: rectEgfrItem
            paramData: egfrItem
            anchors.fill: undefined
            anchors.top: parent.top
            visible: (paramData !== undefined) && (paramData.Name === "EgfrValue")
            height: visible ? egfrContentHeight : 0
            interactive: root.interactive

            // CRU doesn't show units when there is no egfr value. This is different to showing Egfr Value in Advance Param List
            textUnitfunc: getEgfrTextUnit

            // Drawer's backgroud is also grey so make the unit text white for visibility
            textUnit.color: textValue.color

            function getEgfrTextUnit()
            {
                return (!visible || (rectEgfrItem.textValue.text !== "--")) ?  getUnitValue() : "";
            }
        }
    }
}
