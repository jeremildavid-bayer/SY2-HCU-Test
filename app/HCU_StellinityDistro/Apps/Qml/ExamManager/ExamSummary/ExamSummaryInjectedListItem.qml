import QtQuick 2.12
import "../../Util.js" as Util
import "../../Widgets"

Item {
    property bool isContrastType: false
    property string volInjected: localeToFloatStr(0, 1);

    width: parent.width
    height: rowHeight

    Text {
        id: iconType
        height: parent.height
        width: parent.width * 0.08
        verticalAlignment: Text.AlignVCenter
        font.family: fontIcon.name
        font.pixelSize: height * 0.5
        text: isContrastType ? "\ue92f" : "\ue930"
        color: {
            if (isContrastType)
            {
                return (selectedContrast.ColorCode === "GREEN") ? colorMap.contrast1 : colorMap.contrast2;
            }
            return colorMap.saline;
        }
    }

    Text {
        id: textType
        anchors.left: iconType.right
        anchors.leftMargin: parent.width * 0.01
        anchors.right: textVolTotal.left
        anchors.rightMargin: parent.width * 0.01
        verticalAlignment: Text.AlignVCenter
        height: parent.height
        font.family: fontRobotoLight.name
        font.pixelSize: height * 0.43
        minimumPixelSize: font.pixelSize * 0.3
        fontSizeMode: Text.Fit
        color: colorMap.text01
        wrapMode: Text.Wrap
        text: {
            if (isContrastType)
            {
                if ( (selectedContrast.FluidPackage === undefined) ||
                     (selectedContrast.FluidPackage.LoadedAt === undefined) )
                {
                    return translate("T_Contrast");
                }
                return selectedContrast.FluidPackage.Brand + " " + selectedContrast.FluidPackage.Concentration;
            }
            return translate("T_Saline");
        }
    }

    Text {
        id: textVolTotal
        height: parent.height
        anchors.right: textUnit.left
        anchors.rightMargin: parent.width * 0.01
        width: contentWidth
        verticalAlignment: Text.AlignVCenter
        font.family: fontRobotoBold.name
        font.pixelSize: height * 0.6
        color: colorMap.text01
        text: volInjected
    }

    Text {
        id: textUnit
        anchors.topMargin: height * 0.1
        height: parent.height
        anchors.right: parent.right
        width: contentWidth
        verticalAlignment: Text.AlignVCenter
        font.family: fontRobotoLight.name
        font.pixelSize: height * 0.4
        color: colorMap.text02
        text: translate("T_Units_ml")
    }
}
