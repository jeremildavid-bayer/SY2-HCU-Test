import QtQuick 2.12
import "../../../Widgets"
import "../../../Util.js" as Util

GenericButton {
    height: ListView.view.height
    width: ListView.view.width * 0.3
    color: (index === listView.currentIndex) ? border.color : colorMap.keypadButton
    border.color: getFluidSelectItemColor(fluidSelectItems[index])
    border.width: buttonSelectedBorderWidth

    content: [
        Item {
            anchors.fill: parent
            anchors.margins: parent.width * 0.1

            Text {
                id: textBrand
                width: parent.width
                height: (syringeIndex == 0) ? parent.height : (parent.height * 0.5)
                font.family: fontRobotoLight.name
                text: (syringeIndex == 0) ? translate("T_Saline") : fluidSelectItems[index].Brand
                color: (index === listView.currentIndex) ? colorMap.white01 : colorMap.text02
                font.pixelSize: parent.height * 0.4
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }

            Text {
                anchors.top: textBrand.bottom
                anchors.bottom: parent.bottom
                width: parent.width
                font.family: fontRobotoBold.name
                text: (syringeIndex == 0) ? "" : fluidSelectItems[index].Concentration
                color: (index === listView.currentIndex) ? colorMap.white01 : colorMap.text02
                font.pixelSize: height * 0.9
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
        }
    ]

    onBtnClicked: {
        if (listView.currentIndex !== index) {
            if (isFillCurrentSupplyOk(fluidSelectItems[index])) {
                panelMain.bottleSelectedIndex = index;
                if (checkBottleChanged()) {
                    loadBottle();
                }
            }
            else {
                if (dsCfgGlobal.changeContrastEnabled) {
                    startChangeContrastBottle(index);
                }
                else {
                    popupManager.popupContrastChangeDisabled.open();
                }
            }
        }
    }

    Component.onCompleted: {
        listView.dragStarted.connect(reset);
    }

    Component.onDestruction: {
        listView.dragStarted.disconnect(reset);
    }
}

