import QtQuick 2.12
import QtGraphicalEffects 1.12
import "../../Util.js" as Util

Item {
    property int phaseIndex: index
    property string statePath: dsSystem.statePath
    property var selectedContrast: dsExam.selectedContrast
    property string colorUpper: getColorUpper()
    property string colorLower: getColorLower()
    property var phaseData: (executingStep === undefined) ? undefined : executingStep.Phases[phaseIndex]
    property double progressPercent: getProgressPercent()
    property bool isActive: {
        if ( (statePath !== "Executing") && (statePath !== "Busy/Holding") )
        {
            return false;
        }

        if (stepProgressDigest === undefined)
        {
            return false;
        }

        if (phaseIndex !== stepProgressDigest.PhaseIndex)
        {
            return false;
        }

        return true;
    }

    id: root
    width: ListView.view.width
    height: isActive ? activePhaseHeight : inactivePhaseHeight

    Rectangle {
        id: mainRect
        width: isActive ? parent.width : parent.width * 0.92
        height: parent.height
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        radius: height / 2

        gradient: Gradient {
            GradientStop { position: 0.0; color: colorUpper }
            GradientStop { position: 0.50; color: colorUpper }
            GradientStop { position: 0.51; color: colorLower }
            GradientStop { position: 1.0; color: colorLower }
        }

        Item {
            id: progressBarContainer
            width: parent.width - (parent.height * 0.11)
            height: parent.height - (parent.height * 0.11)
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter

            Rectangle {
                id: progressBarFrame
                anchors.fill: parent
                color: "transparent"
                visible: false

                Rectangle {
                    id: progressBarRect
                    x: parent.width - width
                    width: (parent.width * (100 - progressPercent)) / 100
                    height: parent.height
                    color: colorMap.injectPhaseProgressBackground
                }
            }

            Rectangle {
                id: progressBarMask
                anchors.fill: parent
                radius: mainRect.radius
                visible: false
            }

            OpacityMask {
                anchors.fill: progressBarFrame
                source: progressBarFrame
                maskSource: progressBarMask
            }
        }

        Text {
            id: iconType
            x: parent.width * 0.04
            width: parent.width * 0.1
            height: parent.height
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: parent.height * 0.5
            font.family: fontIcon.name
            color: colorMap.text01
            text: {
                if (phaseData !== undefined)
                {
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
                }
                return "";
            }
        }

        Item {
            id: btnEditDualRatio
            x: iconType.x
            width: parent.width * 0.1
            height: parent.height
            visible: (phaseData !== undefined) && (phaseData.Type === "Fluid") && (phaseData.ContrastPercentage > 0) && (phaseData.ContrastPercentage < 100)

            Text {
                id: txtDualRatio1
                y: parent.height * 0.05
                width: parent.width
                height: (parent.height / 2) - y
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignBottom
                color: colorMap.text01
                font.pixelSize: height * 0.72
                font.family: fontRobotoBold.name
                text: "0"
            }

            Text {
                id: txtDualRatio2
                y: (parent.height * 0.5) + txtDualRatio1.y
                width: txtDualRatio1.width
                height: txtDualRatio1.height
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignTop
                color: txtDualRatio1.color
                font.pixelSize: txtDualRatio1.font.pixelSize
                font.family: txtDualRatio1.font.family
                text: (100 - parseInt(txtDualRatio1.text))
            }
        }

        Item {
            id: btnEditFlowRate
            x: parent.width * 0.18
            width: parent.width * 0.35
            height: parent.height
            visible: (phaseData !== undefined) && (phaseData.Type === "Fluid")

            Text {
                id: txtFlowRate
                height: parent.height
                width: parent.width * 0.5
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignVCenter
                color: colorMap.text01
                font.family: fontRobotoBold.name
                font.pixelSize: parent.height * 0.45
                text: ""
            }

            Text {
                id: txtFlowRateUnit
                x: txtFlowRate.x + txtFlowRate.width + 5
                y: parent.height * 0.1
                height: parent.height - y
                width: parent.width - x
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
                color: colorMap.text02
                font.family: fontRobotoLight.name
                font.pixelSize: parent.height * 0.32
                text: translate("T_Units_ml/s")
                elide: Text.ElideRight
            }
        }

        Item {
            id: delayProgress
            x: btnEditFlowRate.x
            width: btnEditFlowRate.width
            height: parent.height
            visible: (phaseData !== undefined) && (phaseData.Type === "Delay")

            Text {
                id: txtDelayProgress
                opacity: 0.5
                height: parent.height
                width: parent.width * 0.5
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignVCenter
                color: colorMap.text01
                font.family: fontRobotoBold.name
                font.pixelSize: parent.height * 0.45
                text: {
                    if (phaseData === undefined)
                    {
                        return "";
                    }

                    var timeTotalMs = Util.durationStrToMillisec(phaseData.Duration);
                    var timeLeftMs = timeTotalMs * (100 - progressPercent) * 0.01;
                    return Util.getMinimisedDurationStr(Util.millisecToDurationStr(timeLeftMs));
                }
            }
        }

        Item {
            id: btnEditVolume
            x: parent.width * 0.51
            width: parent.width * 0.24
            height: parent.height
            visible: (phaseData !== undefined) && (phaseData.Type === "Fluid")

            Text {
                id: txtVolume
                height: parent.height
                width: parent.width * 0.6
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignVCenter
                color: colorMap.text01
                font.family: fontRobotoBold.name
                font.pixelSize: parent.height * 0.45
                text: ""
            }

            Text {
                id: txtVolumeUnit
                x: txtVolume.x + txtVolume.width + 5
                y: parent.height * 0.1
                width: parent.width - x
                height: parent.height - y
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
                color: colorMap.text02
                font.family: fontRobotoLight.name
                font.pixelSize: parent.height * 0.32
                text: translate("T_Units_ml")
                elide: Text.ElideRight
            }
        }

        Text {
            id: txtDuration
            height: parent.height
            x: parent.width * 0.77
            width: parent.width * 0.22
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            color: colorMap.text01
            font.family: ( (phaseData !== undefined) && (phaseData.Type === "Delay") ) ? fontRobotoBold.name : fontRobotoLight.name
            font.pixelSize: parent.height * 0.45
            text: ""
        }
    }

    Component.onCompleted: {
        reload();
    }

    function reload()
    {
        txtDualRatio1.color = Qt.binding(function() { return colorMap.text01; });
        txtFlowRate.color = Qt.binding(function() { return colorMap.text01; });
        txtVolume.color = Qt.binding(function() { return colorMap.text01; });
        txtDuration.color = Qt.binding(function() { return colorMap.text01; });

        txtDualRatio1.text = Qt.binding(function() { return ((phaseData === undefined) || (phaseData.ContrastPercentage === undefined)) ? 0 : phaseData.ContrastPercentage; });
        txtFlowRate.text = Qt.binding(function() { return ((phaseData === undefined) || (phaseData.FlowRate === undefined)) ? "0" : localeToFloatStr(phaseData.FlowRate, 1); });
        txtVolume.text = Qt.binding(function() { return ((phaseData === undefined) || (phaseData.TotalVolume === undefined)) ? "0" : Math.ceil(phaseData.TotalVolume); });
        txtDuration.text = Qt.binding(function() {  var duration = ( (phaseData !== undefined) && (phaseData.Duration !== undefined) ) ? phaseData.Duration : "00:00";
                                                    var durationText = Util.getMinimisedDurationStr(duration);
                                                    if (durationText === "00:00")
                                                    {
                                                        durationText = "00:01";
                                                    }
                                                    return durationText;
        });
    }

    function getColorUpper()
    {
        if ( (phaseData === undefined) || (phaseData.Type === "Dummy") )
        {
            return colorMap.buttonShadow;
        }
        else if (phaseData.Type === "Fluid")
        {
            if (phaseData.ContrastPercentage === 0)
            {
                return colorMap.saline;
            }
            else
            {
                return (selectedContrast.ColorCode === "GREEN") ? colorMap.contrast1 : colorMap.contrast2;
            }
        }
        else if (phaseData.Type === "Delay")
        {
            return colorMap.paused;
        }
    }

    function getColorLower()
    {
        if ( (phaseData === undefined) || (phaseData.Type === "Dummy") )
        {
            return colorMap.buttonShadow;
        }
        else if (phaseData.Type === "Fluid")
        {
            if (phaseData.ContrastPercentage === 100)
            {
                return (selectedContrast.ColorCode === "GREEN") ? colorMap.contrast1 : colorMap.contrast2;
            }
            else
            {
                return colorMap.saline;
            }
        }
        else if (phaseData.Type === "Delay")
        {
            return colorMap.paused;
        }
    }

    function getProgressPercent()
    {
        if (executingStep === undefined)
        {
            return 0;
        }

        if (executingStep.Phases === undefined)
        {
            return 0;
        }

        if (stepProgressDigest === undefined)
        {
            return 0;
        }

        if ( (statePath == "Idle") || (statePath == "Ready/Armed") )
        {
            return 0;
        }

        if (stepProgressDigest.PhaseIndex === -1)
        {
            return 0;
        }

        var newProgressPercent = 0;
        if (phaseIndex <= stepProgressDigest.PhaseIndex)
        {
            newProgressPercent = stepProgressDigest.PhaseProgress[phaseIndex].Progress;
        }
        return newProgressPercent;
    }
}
