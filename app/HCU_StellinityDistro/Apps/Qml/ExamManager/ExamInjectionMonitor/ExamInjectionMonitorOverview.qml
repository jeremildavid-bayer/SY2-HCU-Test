import QtQuick 2.12
import "../../Util.js" as Util

Row {
    property var selectedContrast: dsExam.selectedContrast
    property string pressureUnit: dsCfgGlobal.pressureUnit

    spacing: width * 0.04

    Item {
        width: parent.width * 0.22
        height: parent.height

        Text {
            id: titlePressure
            width: parent.width
            height: parent.height * 0.4
            text: translate("T_Pressure")
            font.family: fontRobotoLight.name
            font.pixelSize: height * 0.47
            color: colorMap.text02
            wrapMode: Text.Wrap
            verticalAlignment: Text.AlignVCenter
        }

        Item {
            anchors.top: titlePressure.bottom
            height: parent.height * 0.4
            width: parent.width

            Text {
                id: textPressure
                height: parent.height
                width: contentWidth
                color: colorMap.text01
                font.family: fontRobotoBold.name
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: height * 0.9
                text: "0"
            }

            Text {
                id: textPressureUnit
                anchors.left: textPressure.right
                anchors.leftMargin: 8
                y: parent.height * 0.15
                width: contentWidth
                height: parent.height - y
                color: colorMap.text02
                font.pixelSize: height * 0.6
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
                font.family: fontRobotoLight.name
                text: pressureUnit
            }
        }
    }

    Item {
        width: parent.width * 0.23
        height: parent.height

        Text {
            id: titleFlowRate
            width: parent.width
            height: parent.height * 0.4
            text: translate("T_FlowRate")
            font.family: fontRobotoLight.name
            font.pixelSize: height * 0.47
            color: colorMap.text02
            wrapMode: Text.Wrap
            verticalAlignment: Text.AlignVCenter
        }

        Item {
            anchors.top: titleFlowRate.bottom
            height: parent.height * 0.4
            width: parent.width

            Text {
                id: textFlowRate
                height: parent.height
                color: colorMap.text01
                font.family: fontRobotoBold.name
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: height * 0.9
                text: "0"
            }

            Text {
                id: textFlowRateUnit
                anchors.left: textFlowRate.right
                anchors.leftMargin: 8
                y: parent.height * 0.15
                height: parent.height - y
                color: colorMap.text02
                width: contentWidth
                font.pixelSize: height * 0.6
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignVCenter
                font.family: fontRobotoLight.name
                text: translate("T_Units_ml/s")
            }
        }
    }

    Item {
        height: parent.height
        width: parent.width * 0.56

        Text {
            id: titleInjectedTotals
            width: parent.width
            height: parent.height * 0.4
            text: translate("T_InjectedTotals")
            font.family: fontRobotoLight.name
            font.pixelSize: height * 0.47
            color: colorMap.text02
            wrapMode: Text.Wrap
            verticalAlignment: Text.AlignVCenter
        }

        Item {
            anchors.top: titleInjectedTotals.bottom
            height: parent.height * 0.4

            Text {
                id: iconContrast
                height: parent.height
                width: contentWidth
                font.family: fontIcon.name
                font.pixelSize: height * 0.7
                verticalAlignment: Text.AlignVCenter
                text: "\ue92f"
                color: ((selectedContrast === undefined) || (selectedContrast.ColorCode === "GREEN")) ? colorMap.contrast1 : colorMap.contrast2;
            }

            Text {
                id: textContrastVol
                anchors.left: iconContrast.right
                anchors.leftMargin: 6
                width: contentWidth
                height: parent.height
                color: colorMap.text01
                font.family: fontRobotoBold.name
                text: "0"
                font.pixelSize: height * 0.9
                verticalAlignment: Text.AlignVCenter
            }

            Text {
                id: textContrastVolUnit
                anchors.left: textContrastVol.right
                anchors.leftMargin: 8
                y: parent.height * 0.15
                width: contentWidth
                height: parent.height - y
                color: colorMap.text02
                font.family: fontRobotoLight.name
                text: translate("T_Units_ml")
                font.pixelSize: height * 0.6
                verticalAlignment: Text.AlignVCenter
            }

            Text {
                id: iconSaline
                anchors.left: textContrastVolUnit.right
                anchors.leftMargin: 36
                height: parent.height
                width: contentWidth
                font.family: fontIcon.name
                font.pixelSize: height * 0.7
                verticalAlignment: Text.AlignVCenter
                text: "\ue930"
                color: colorMap.saline
            }

            Text {
                id: textSalineVol
                anchors.left: iconSaline.right
                anchors.leftMargin: 6
                width: contentWidth
                height: parent.height
                color: colorMap.text01
                font.family: fontRobotoBold.name
                text: "0"
                font.pixelSize: height * 0.9
                verticalAlignment: Text.AlignVCenter
            }

            Text {
                id: textSalineVolUnit
                anchors.left: textSalineVol.right
                anchors.leftMargin: 8
                y: parent.height * 0.15
                width: contentWidth
                height: parent.height - y
                color: colorMap.text02
                font.family: fontRobotoLight.name
                text: translate("T_Units_ml")
                font.pixelSize: height * 0.6
                verticalAlignment: Text.AlignVCenter
            }
        }
    }

    Component.onCompleted: {
        dsExam.qmlInjectionMonitorOverview = this;
    }

    function slotUpdate(maxPressureKpa, isPressureLimiting, pressureKpa, flowRate, volContrast, volSaline)
    {
        if (pressureUnit == "kg/cm2")
        {
            textPressure.text = localeToFloatStr(Util.getPressure(pressureUnit, pressureKpa), 1);
        }
        else
        {
            textPressure.text = localeToFloatStr(Util.getPressure(pressureUnit, pressureKpa), 0);
        }

        textFlowRate.text = localeToFloatStr(flowRate, 1);
        textContrastVol.text = localeToFloatStr(volContrast, 1);
        textSalineVol.text = localeToFloatStr(volSaline, 1);

        titlePressure.color = (pressureKpa < maxPressureKpa) ? colorMap.text02 : colorMap.errText;
        textPressure.color = (pressureKpa < maxPressureKpa) ? colorMap.text01 : colorMap.errText;
        textPressureUnit.color = (pressureKpa < maxPressureKpa) ? colorMap.text02 : colorMap.errText;

        titleFlowRate.color = (!isPressureLimiting) ? colorMap.text02 : colorMap.errText;
        textFlowRate.color = (!isPressureLimiting) ? colorMap.text01 : colorMap.errText;
        textFlowRateUnit.color = (!isPressureLimiting) ? colorMap.text02 : colorMap.errText;
    }
}
