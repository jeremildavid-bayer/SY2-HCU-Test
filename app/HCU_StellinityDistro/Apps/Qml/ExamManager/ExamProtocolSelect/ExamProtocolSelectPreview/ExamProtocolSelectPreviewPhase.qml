import QtQuick 2.12
import "../../../Util.js" as Util

Rectangle {
    property int phaseIdx: index
    property var phaseData: stepData.Phases[phaseIdx]
    property int borderWidth: phaseHeight * 0.05
    property string colorUpper: {
        if (phaseData.Type === "Fluid")
        {
            if (phaseData.ContrastPercentage === 0)
            {
                return colorMap.saline;
            }
            else
            {
                return (selectedContrast.ColorCode === "GREEN") ? colorMap.contrast1 : colorMap.contrast2
            }
        }
        return colorMap.paused;
    }

    property string colorLower: {
        if (phaseData.Type === "Fluid")
        {
            if (phaseData.ContrastPercentage === 100)
            {
                return (selectedContrast.ColorCode === "GREEN") ? colorMap.contrast1 : colorMap.contrast2
            }
            else
            {
                return colorMap.saline;
            }
        }
        return colorMap.paused;
    }

    property string titleTextStr: {
        if (phaseData.Type === "Fluid")
        {
            if (phaseData.ContrastPercentage === 100)
            {
                return "T_Contrast";
            }
            else if (phaseData.ContrastPercentage === 0)
            {
                return "T_Saline";
            }
            else
            {
                return "T_DualFlow";
            }
        }
        return "T_PhaseTypeName_Delay";
    }

    width: ListView.view.width
    height: phaseHeight
    radius: height / 2

    gradient: Gradient {
        GradientStop { position: 0.0; color: colorUpper }
        GradientStop { position: 0.50; color: colorUpper }
        GradientStop { position: 0.51; color: colorLower }
        GradientStop { position: 1.0; color: colorLower }
    }

    Rectangle {
        color: colorMap.mainBackground
        radius: parent.radius
        anchors {
            fill: parent
            margins: parent.borderWidth
        }

        Text {
            id: iconText
            visible: {
                if (phaseData.Type === "Fluid")
                {
                    if ( (phaseData.ContrastPercentage !== 100) &&
                         (phaseData.ContrastPercentage !== 0) )
                    {
                        return false;
                    }
                }
                return true;
            }

            x: parent.width * 0.032
            width: parent.width * 0.03
            height: parent.height
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: parent.height * 0.5
            font.family: fontIcon.name
            text: {
                if (phaseData.Type === "Fluid")
                {
                    if (phaseData.ContrastPercentage === 100)
                    {
                        return "\ue92f";
                    }
                    else if (phaseData.ContrastPercentage === 0)
                    {
                        return "\ue930";
                    }
                }
                else if (phaseData.Type === "Delay")
                {
                    return "\ue94c";
                }
                return "";
            }

            color: colorUpper
        }

        Rectangle {
            id: rectDualRatio
            visible: {
                if (phaseData.Type === "Fluid")
                {
                    if ( (phaseData.ContrastPercentage !== 100) &&
                         (phaseData.ContrastPercentage !== 0) )
                    {
                        return true;
                    }
                }
                return false;
            }

            x: parent.width * 0.036
            width: parent.width * 0.03
            height: parent.height
            color: "transparent"

            Text {
                id: txtDualRatio1
                y: 0
                width: parent.width * 0.95
                height: parent.height / 2
                anchors.horizontalCenter: parent.horizontalCenter
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: height * 0.55
                font.family: fontRobotoMedium.name
                text: phaseData.ContrastPercentage + translate("T_Units_%")
                color: colorUpper
            }

            Text {
                id: txtDualRatio2
                y: parent.height / 2
                width: parent.width * 0.95
                height: parent.height / 2
                anchors.horizontalCenter: parent.horizontalCenter
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: height * 0.55
                font.family: fontRobotoMedium.name
                text: (100 - phaseData.ContrastPercentage) + translate("T_Units_%")
                color: colorLower
            }
        }

        Text {
            id: titleText
            x: iconText.x + iconText.width + (parent.width * 0.04)
            height: parent.height
            width: parent.width * 0.26
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: parent.height * 0.42
            font.family: fontRobotoBold.name
            text: translate(titleTextStr)
            color: colorUpper
            elide: Text.ElideRight
        }

        Text {
            id: txtFlowRate
            visible: (phaseData.Type === "Fluid")
            height: parent.height
            x: titleText.x + titleText.width
            width: parent.width * 0.1
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignVCenter
            color: colorMap.text01
            font.pixelSize: parent.height * 0.55
            font.family: fontRobotoBold.name
            text: localeToFloatStr(phaseData.FlowRate, 1);
        }

        Text {
            id: txtFlowRateUnit
            visible: (phaseData.Type === "Fluid")
            x: txtFlowRate.x + txtFlowRate.width + (parent.width * 0.01)
            y: parent.height * 0.1
            width: parent.width * 0.13
            height: parent.height - y
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            color: colorMap.text02
            font.pixelSize: parent.height * 0.35
            font.family: fontRobotoLight.name
            text: translate("T_Units_ml/s")
            elide: Text.ElideRight
        }

        Text {
            id: txtVolume
            visible: (phaseData.Type === "Fluid")
            height: parent.height
            x: txtFlowRateUnit.x + txtFlowRateUnit.width + (parent.width * 0.01)
            width: parent.width * 0.1
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignVCenter
            color: colorMap.text01
            font.pixelSize: parent.height * 0.55
            font.family: fontRobotoBold.name
            text: Math.ceil(phaseData.TotalVolume);
        }

        Text {
            id: txtVolumeUnit
            visible: (phaseData.Type === "Fluid")
            x: txtVolume.x + txtVolume.width + (parent.width * 0.01)
            y: parent.height * 0.1
            width: parent.width * 0.13
            height: parent.height - y
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            color: colorMap.text02
            font.pixelSize: parent.height * 0.35
            font.family: fontRobotoLight.name
            text: translate("T_Units_ml")
            elide: Text.ElideRight
        }

        Text {
            id: txtDuration
            height: parent.height
            x: txtVolumeUnit.x + txtVolumeUnit.width + (parent.width * 0.03)
            width: parent.width * 0.08
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignVCenter
            color: colorMap.text01
            font.pixelSize: parent.height * 0.55
            font.family: (phaseData.Type === "Fluid") ? fontRobotoLight.name : fontRobotoBold.name
            text: Util.getMinimisedDurationStr(phaseData.Duration)
        }
    }
}

