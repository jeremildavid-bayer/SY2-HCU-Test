import QtQuick 2.12
import "../../Qml/Widgets"
import "../Util.js" as Util

Rectangle {
    property bool mcuSimulatorEnabled: dsMcuSim.simulatorEnabled
    property var activeSystemAlerts: dsAlert.activeSystemAlerts
    property var ledControlStatus: dsMcuSim.ledControlStatus
    property int widthExpanded: dsCfgLocal.screenW - x - (dsCfgLocal.screenW * 0.005)
    property int btnWidth: width * 0.045
    property int btnHeight: height * 0.9
    property int animationMs: 250
    property double backgroundOpacity: 0.9
    property bool mudsPresent: dsMcu.mudsPresent
    property bool mudsLatched: dsMcu.mudsLatched
    property bool sudsInserted: dsMcu.sudsInserted
    property string outletDoorState: dsMcu.outletDoorState
    property string doorState: dsMcu.doorState
    property var fluidSourceWasteContainer: dsDevice.fluidSourceWasteContainer
    property var powerStatus: dsMcu.powerStatus
    property var heatMaintainerStatus: dsMcu.heatMaintainerStatus
    property var stepProgressDigest: dsExam.stepProgressDigest
    property string temperatureUnit: dsCfgGlobal.temperatureUnit
    property bool sudsBubbleDetected: dsMcu.sudsBubbleDetected
    property var bottleBubbleStates: dsMcu.bottleBubbleStates
    property double syringesAirVolume: dsMcuSim.syringesAirVolume
    property double labelFontPixelSize: btnWidth * 0.5
    property var stopcockPositions: dsMcu.stopcockPositions
    property string mudsLineFluidSyringeIndex: dsDevice.mudsLineFluidSyringeIndex
    property string baseBoardType: dsHardwareInfo.baseBoardType

    id: root
    x: (activeSystemAlerts.length == 0) ? dsCfgLocal.screenW * 0.08 : dsCfgLocal.screenW * 0.13
    width: 0
    height: titleBarHeight
    color: colorMap.subPanelBackground
    clip: true

    McuSimulatorSoundPlunger {}
    McuSimulatorSoundStopcock {}

    state: "INACTIVE"
    visible: false

    states: [
        State { name: "ACTIVE" },
        State { name: "INACTIVE" }
    ]

    transitions: [
        Transition {
            from: "INACTIVE"
            to: "ACTIVE"
            SequentialAnimation {
                ScriptAction { script: {
                        root.opacity = 0;
                        root.visible = true;
                    }
                }

                ParallelAnimation {
                    NumberAnimation { target: root; properties: "width"; from: 0; to: widthExpanded; duration: animationMs; easing.type: Easing.InOutQuart }
                    NumberAnimation { target: root; properties: 'opacity'; from: 0; to: backgroundOpacity; duration: animationMs }
                }
            }
        },
        Transition {
            from: "ACTIVE"
            to: "INACTIVE"
            SequentialAnimation {
                ParallelAnimation {
                    NumberAnimation { target: root; properties: "width"; to: 0; from: widthExpanded; duration: animationMs; easing.type: Easing.InOutQuart }
                    NumberAnimation { target: root; properties: 'opacity'; to: 0; from: backgroundOpacity; duration: animationMs }                }

                ScriptAction { script: {
                        root.visible = false;
                    }
                }
            }
        }
    ]

    Flickable {
        id: flickable
        width: root.width
        height: root.height
        contentHeight: rowFrame.height
        contentWidth: rowFrame.width
        flickableDirection: Flickable.HorizontalFlick
        clip: true

        Row {
            id: rowFrame
            anchors.verticalCenter: parent.verticalCenter
            height: btnHeight
            clip: true

            GenericIconButton {
                width: btnWidth
                height: btnHeight
                color: "transparent"
                iconColor: "red"
                iconFontFamily: fontAwesome.name
                iconText: "\uf28e"
                iconFontPixelSize: labelFontPixelSize
                captionText: "STOP"

                onBtnPressed: {
                    dsMcuSim.slotStopButtonPressed(true);
                }
                onBtnReleased: {
                    dsMcuSim.slotStopButtonPressed(false);
                }
            }

            GenericIconButton {
                width: btnWidth
                height: btnHeight
                color: "transparent"
                iconColor: "blue"
                iconFontFamily: fontAwesome.name
                iconText: "\uf1b0"
                iconFontPixelSize: labelFontPixelSize
                captionText: "PRIME"

                onBtnPressed: {
                    dsMcuSim.slotManualPrimeButtonPressed(true);
                }
                onBtnReleased: {
                    dsMcuSim.slotManualPrimeButtonPressed(false);
                }
            }

            GenericIconButton {
                id: mudsPresence
                width: btnWidth
                height: btnHeight
                color: "transparent"
                iconFontFamily: fontAwesome.name
                iconText: "\uf1fd"
                iconFontPixelSize: labelFontPixelSize

                captionText: mudsPresent ? "PRESENT" : "MISSING"

                onBtnClicked: {
                    dsMcuSim.slotMudsPresent(!mudsPresent);
                }

                Text {
                    color: "red"
                    opacity: 0.8
                    width: parent.width
                    height: parent.captionY
                    visible: !mudsPresent
                    text: "\uf00d"
                    font.pixelSize: labelFontPixelSize
                    font.family: fontAwesome.name
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            GenericIconButton {
                id: mudsLatch
                width: btnWidth
                height: btnHeight
                color: "transparent"
                iconText: mudsLatched ? "\uf0ed" : "\uf0ee"
                iconFontPixelSize: labelFontPixelSize
                iconColor: mudsLatched ? "lightgreen" : "white"
                captionText: mudsLatched ? "LATCHED" : "NOT\nLATCHED"
                onBtnClicked: {
                    dsMcuSim.slotMudsLatched(!mudsLatched);
                }
            }

            GenericIconButton {
                id: outletAirDoor
                width: btnWidth
                height: btnHeight
                color: "transparent"
                iconText: (outletDoorState === "OPEN") ? "\uf10c" : "\uf111"
                iconFontPixelSize: labelFontPixelSize
                iconColor: (outletDoorState === "OPEN") ? "white" : "lightgreen"
                captionText: outletDoorState

                onBtnClicked: {
                    dsMcuSim.slotOutletDoorStateChanged((outletDoorState === "OPEN") ? "CLOSED" : "OPEN");
                }
            }

            GenericIconButton {
                id: door
                width: btnWidth
                height: btnHeight
                color: "transparent"
                iconText: (doorState === "OPEN") ? "\uf115" : "\uf07b"
                iconFontPixelSize: labelFontPixelSize
                captionText: doorState

                onBtnClicked: {
                    dsMcuSim.slotDoorStateChanged((doorState === "OPEN") ? "CLOSED" : "OPEN");
                }
            }

            GenericIconButton {
                id: suds
                width: btnWidth
                height: btnHeight
                color: "transparent"
                iconFontFamily: fontAwesome.name
                iconText: "\uf0a1"
                iconFontPixelSize: labelFontPixelSize

                captionText: sudsInserted ? "INSERTED" : "MISSING"

                onBtnClicked: {
                    dsMcuSim.slotSudsInserted(!sudsInserted);
                }

                Text {
                    color: "red"
                    opacity: 0.8
                    width: parent.width
                    height: parent.captionY
                    visible: !sudsInserted
                    text: "\uf00d"
                    font.pixelSize: labelFontPixelSize
                    font.family: fontAwesome.name
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            GenericIconButton {
                id: wasteBin
                width: btnWidth
                height: btnHeight
                color: "transparent"
                iconFontFamily: fontAwesome.name
                iconText: "\uf014"
                iconFontPixelSize: labelFontPixelSize

                captionText: {
                    if ( (fluidSourceWasteContainer === undefined) ||
                         (fluidSourceWasteContainer.InstalledAt === undefined) ||
                         (fluidSourceWasteContainer.CurrentVolumes === undefined) ||
                         (fluidSourceWasteContainer.CurrentVolumes.length === 0) )
                    {
                        return "MISSING";
                    }
                    else if (fluidSourceWasteContainer.NeedsReplaced)
                    {
                        return "FULL";
                    }
                    else if (fluidSourceWasteContainer.CurrentVolumes[0] === 0)
                    {
                        return "LOW";
                    }
                    else if (fluidSourceWasteContainer.CurrentVolumes[0] === 1)
                    {
                        return "HIGH";
                    }
                    return "UNKNOWN";
                }

                onBtnClicked: {
                    var newState;
                    if (captionText === "MISSING") {
                        newState = "LOW";
                    }
                    else if (captionText === "LOW") {
                        newState = "HIGH";
                    }
                    else if (captionText === "HIGH") {
                        newState = "FULL";
                    }
                    else
                    {
                        newState = "MISSING";
                    }
                    dsMcuSim.slotWasteBinStateChanged(newState);
                }

                Rectangle {
                    id: rectFilled
                    color: colorMap.text01
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width * 0.34
                    y: {
                        if (wasteBin.captionText === "LOW") {
                            return parent.height * 0.46;
                        }
                        else if (wasteBin.captionText === "HIGH") {
                            return parent.height * 0.36;
                        }
                        else if (wasteBin.captionText === "FULL") {
                            return parent.height * 0.26;
                        }
                        return 0;
                    }

                    height: {
                        if (wasteBin.captionText === "LOW") {
                            return parent.height * 0.1;
                        }
                        else if (wasteBin.captionText === "HIGH") {
                            return parent.height * 0.2;
                        }
                        else if (wasteBin.captionText === "FULL") {
                            return parent.height * 0.3;
                        }
                        return 0;
                    }
                }

                Text {
                    color: "red"
                    visible: wasteBin.captionText === "MISSING"
                    opacity: 0.8
                    width: parent.width
                    height: parent.captionY
                    text: "\uf00d"
                    font.pixelSize: labelFontPixelSize
                    font.family: fontAwesome.name
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            GenericIconButton {
                width: btnWidth
                height: btnHeight
                color: "transparent"
                iconText: getIcon()
                iconFontPixelSize: labelFontPixelSize * 0.98
                iconColor: getIconColor()
                interactive: baseBoardType == "Battery"
                captionText: {
                    if (powerStatus !== undefined)
                    {
                        if ( (powerStatus.BatteryLevel === "Critical") ||
                             (powerStatus.BatteryLevel === "Dead") ||
                             (powerStatus.BatteryLevel === "Flat") ||
                             (powerStatus.BatteryLevel === "Low") ||
                             (powerStatus.BatteryLevel === "Medium") ||
                             (powerStatus.BatteryLevel === "High") ||
                             (powerStatus.BatteryLevel === "Full") )
                        {
                            return powerStatus.BatteryLevel.toUpperCase();
                        }
                        else if (powerStatus.BatteryLevel === "NoBattery")
                        {
                            return "NO\nBATTERY";
                        }
                        return powerStatus.BatteryLevel + "(?)";
                    }
                    return "UNKNOWN";
                }

                onBtnClicked: {
                    var newState;
                    if (captionText === "FULL")
                    {
                        newState = "HIGH";
                    }
                    else if (captionText === "HIGH")
                    {
                        newState = "MEDIUM";
                    }
                    else if (captionText === "MEDIUM")
                    {
                        newState = "LOW";
                    }
                    else if (captionText === "LOW")
                    {
                        newState = "FLAT";
                    }
                    else if (captionText === "FLAT")
                    {
                        newState = "DEAD";
                    }
                    else if (captionText === "DEAD")
                    {
                        newState = "CRITICAL";
                    }
                    else if (captionText === "CRITICAL")
                    {
                        newState = "NO_BATTERY";
                    }
                    else
                    {
                        newState = "FULL";
                    }

                    dsMcuSim.slotBatteryStateChanged(newState);
                }

                function getIconColor()
                {
                    if (captionText === "DEAD")
                    {
                        return "orange";
                    }
                    else if (captionText === "CRITICAL")
                    {
                        return "red";
                    }
                    else if (captionText === "NO\nBATTERY")
                    {
                        return "gray";
                    }
                    else
                    {
                        return colorMap.text01;
                    }
                }

                function getIcon()
                {
                    if (captionText === "FULL")
                    {
                        return "\uf240";
                    }
                    else if (captionText === "HIGH")
                    {
                        return "\uf241";
                    }
                    else if (captionText === "MEDIUM")
                    {
                        return "\uf242";
                    }
                    else if (captionText === "LOW")
                    {
                        return "\uf243";
                    }
                    return "\uf244";
                }
            }

            GenericIconButton {
                id: acConnected
                width: btnWidth
                height: btnHeight
                color: "transparent"
                iconFontFamily: fontAwesome.name
                iconText: "\uf1e6"
                iconFontPixelSize: labelFontPixelSize
                interactive: baseBoardType == "Battery"
                captionText: {
                    if (powerStatus === undefined)
                    {
                        return "UNKNOWN";
                    }
                    else
                    {
                        return powerStatus.IsAcPowered ? "AC" : "BATTERY"
                    }
                }


                onBtnClicked: {
                    dsMcuSim.slotAcConnectedStateChanged(captionText == "AC" ? "BATTERY" : "AC");
                }

                Text {
                    color: "red"
                    opacity: 0.8
                    width: parent.width
                    height: parent.captionY
                    visible: (acConnected.captionText == "BATTERY")
                    text: "\uf00d"
                    font.pixelSize: labelFontPixelSize
                    font.family: fontAwesome.name
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            GenericIconButton {
                property double temperatureReading: (heatMaintainerStatus === undefined) ? 0 : heatMaintainerStatus.TemperatureReadings.DOOR
                id: temperature1
                width: btnWidth
                height: btnHeight
                color: "transparent"
                iconText: {
                    if (heatMaintainerStatus === undefined)
                    {
                        return "";
                    }
                    return (temperatureReading <= 35) ? "\uf0c2" : "\uf185";
                }
                iconColor: getIconColor()
                iconFontPixelSize: labelFontPixelSize

                captionText: {
                    if (heatMaintainerStatus === undefined)
                    {
                        return "MUDS\n?" + ((temperatureUnit == "degreesF") ? "°F" : "°C");
                    }
                    else
                    {
                        return "DOOR\n" + Util.getTemperature(temperatureUnit, temperatureReading) + ((temperatureUnit == "degreesF") ? "°F" : "°C");
                    }
                }

                onBtnClicked: {
                    var newTemperature;

                    if (temperatureReading <= 35) {
                        newTemperature = 42;
                    }
                    else if (temperatureReading <= 42) {
                        newTemperature = 45;
                    }
                    else {
                        newTemperature = 35;
                    }

                    dsMcuSim.slotTemperature1Changed(newTemperature);
                }

                function getIconColor()
                {
                    if (heatMaintainerStatus === undefined)
                    {
                        return "red"
                    }
                    else if (temperatureReading <= 35)
                    {
                        return "lightblue";
                    }
                    else if (temperatureReading <= 42)
                    {
                        return "orange";
                    }
                    else
                    {
                        return "red";
                    }
                }
            }

            GenericIconButton {
                property double temperatureReading: (heatMaintainerStatus === undefined) ? 0 : heatMaintainerStatus.TemperatureReadings.MUDS
                id: temperature2
                width: btnWidth
                height: btnHeight
                color: "transparent"
                iconText: {
                    if (heatMaintainerStatus === undefined)
                    {
                        return "";
                    }
                    return (temperatureReading <= 35) ? "\uf0c2" : "\uf185";
                }
                iconColor: getIconColor()
                iconFontPixelSize: labelFontPixelSize

                captionText: {
                    if (heatMaintainerStatus === undefined)
                    {
                        return "MUDS\n?" + ((temperatureUnit == "degreesF") ? "°F" : "°C");
                    }
                    else
                    {
                        return "MUDS\n" + Util.getTemperature(temperatureUnit, temperatureReading) + ((temperatureUnit == "degreesF") ? "°F" : "°C");
                    }
                }

                onBtnClicked: {
                    var newTemperature;

                    if (temperatureReading <= 35) {
                        newTemperature = 42;
                    }
                    else if (temperatureReading <= 42) {
                        newTemperature = 45;
                    }
                    else {
                        newTemperature = 35;
                    }

                    dsMcuSim.slotTemperature2Changed(newTemperature);
                }

                function getIconColor()
                {
                    if (heatMaintainerStatus === undefined)
                    {
                        return "red"
                    }
                    else if (temperatureReading <= 35)
                    {
                        return "lightblue";
                    }
                    else if (temperatureReading <= 42)
                    {
                        return "orange";
                    }
                    else
                    {
                        return "red";
                    }
                }
            }


            GenericIconButton {
                id: btnSyringesAir
                width: btnWidth
                height: btnHeight
                color: "transparent"
                iconFontFamily: fontAwesome.name
                iconText: "\uf127"
                iconFontPixelSize: labelFontPixelSize
                iconColor: "lightgray"
                captionText: "SYRINGES\n" + (syringesAirVolume > 0 ? "AIR" : "NO-AIR")

                onBtnClicked: {
                    dsMcuSim.slotSyringesAir();
                }

                Text {
                    color: "lightblue"
                    visible: btnSyringesAir.captionText.indexOf("NO-AIR") < 0
                    opacity: 0.8
                    width: parent.width
                    height: parent.captionY
                    text: "\uf00d"
                    font.pixelSize: labelFontPixelSize
                    font.family: fontAwesome.name
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            GenericIconButton {
                id: btnOutletAirSensor
                width: btnWidth
                height: btnHeight
                color: "transparent"
                iconFontFamily: fontAwesome.name
                iconText: "\uf127"
                iconFontPixelSize: labelFontPixelSize

                captionText: {
                    if (sudsBubbleDetected)
                    {
                        return "PL-AIR"
                    }
                    return "PL-FLUID"
                }

                onBtnClicked: {
                    dsMcuSim.slotBubblePatientLine();
                }

                Text {
                    color: "lightblue"
                    visible: btnOutletAirSensor.captionText.indexOf("PL-FLUID") < 0
                    opacity: 0.8
                    width: parent.width
                    height: parent.captionY
                    text: "\uf00d"
                    font.pixelSize: labelFontPixelSize
                    font.family: fontAwesome.name
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            GenericIconButton {
                id: btnInletAir1
                width: btnWidth
                height: btnHeight
                color: "transparent"
                iconFontFamily: fontAwesome.name
                iconText: "\uf127"
                iconFontPixelSize: labelFontPixelSize
                iconColor: colorMap.saline
                captionText: "S0\n" + bottleBubbleStates[0]

                onBtnClicked: {
                    dsMcuSim.slotBubbleSaline();
                }

                Text {
                    color: (btnInletAir1.captionText.indexOf("MISSING") >= 0) ? "red" : "lightblue"
                    visible: (btnInletAir1.captionText.indexOf("MISSING") >= 0) || (btnInletAir1.captionText.indexOf("AIR") >= 0)
                    opacity: 0.8
                    width: parent.width
                    height: parent.captionY
                    text: "\uf00d"
                    font.pixelSize: labelFontPixelSize
                    font.family: fontAwesome.name
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            GenericIconButton {
                id: btnInletAir2
                width: btnWidth
                height: btnHeight
                color: "transparent"
                iconFontFamily: fontAwesome.name
                iconText: "\uf127"
                iconFontPixelSize: labelFontPixelSize
                iconColor: colorMap.contrast1
                captionText: "C1\n" + bottleBubbleStates[1]

                onBtnClicked: {
                    dsMcuSim.slotBubbleContrast1();
                }

                Text {
                    color: (btnInletAir2.captionText.indexOf("MISSING") >= 0) ? "red" : "lightblue"
                    visible: (btnInletAir2.captionText.indexOf("MISSING") >= 0) || (btnInletAir2.captionText.indexOf("AIR") >= 0)
                    opacity: 0.8
                    width: parent.width
                    height: parent.captionY
                    text: "\uf00d"
                    font.pixelSize: labelFontPixelSize
                    font.family: fontAwesome.name
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            GenericIconButton {
                id: btnInletAir3
                width: btnWidth
                height: btnHeight
                color: "transparent"
                iconFontFamily: fontAwesome.name
                iconText: "\uf127"
                iconFontPixelSize: labelFontPixelSize
                iconColor: colorMap.contrast2
                captionText: "C2\n" + bottleBubbleStates[2]

                onBtnClicked: {
                    dsMcuSim.slotBubbleContrast2();
                }

                Text {
                    color: (btnInletAir3.captionText.indexOf("MISSING") >= 0) ? "red" : "lightblue"
                    visible: (btnInletAir3.captionText.indexOf("MISSING") >= 0) || (btnInletAir3.captionText.indexOf("AIR") >= 0)
                    opacity: 0.8
                    width: parent.width
                    height: parent.captionY
                    text: "\uf00d"
                    font.pixelSize: labelFontPixelSize
                    font.family: fontAwesome.name
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            GenericIconButton {
                id: btnPressureLimit
                width: btnWidth
                height: btnHeight
                color: "transparent"
                iconFontFamily: fontAwesome.name
                iconText: "\uf0ab"
                iconFontPixelSize: labelFontPixelSize
                captionFontPixelSize: height * 0.12
                captionY: height - (captionFontPixelSize * 2)
                iconColor: {
                    if (stepProgressDigest === undefined)
                    {
                        return "grey";
                    }
                    else if (stepProgressDigest.AdaptiveFlowState === "Active")
                    {
                        return "orange";
                    }
                    else if (stepProgressDigest.AdaptiveFlowState === "ActiveCritical")
                    {
                        return "red";
                    }
                    else
                    {
                        return "white";
                    }
                }

                captionText: {
                    if (stepProgressDigest === undefined)
                    {
                        return "NOT\nINJECTING";
                    }
                    else if (stepProgressDigest.AdaptiveFlowState === "Active")
                    {
                        return "AF\nACTIVE";
                    }
                    else if (stepProgressDigest.AdaptiveFlowState === "ActiveCritical")
                    {
                        return "AF\nCRITICAL";
                    }
                    else
                    {
                        return "AF\nOFF";
                    }
                }

                onBtnClicked: {
                    if (stepProgressDigest.AdaptiveFlowState === undefined)
                    {
                        return;
                    }
                    var nextState;

                    if (stepProgressDigest.AdaptiveFlowState === "Active")
                    {
                        nextState = "AF_CRITICAL";
                    }
                    else if (stepProgressDigest.AdaptiveFlowState === "ActiveCritical")
                    {
                        nextState = "AF_OFF";
                    }
                    else
                    {
                        nextState = "AF_ACTIVE"
                    }
                    dsMcuSim.slotAdaptiveFlowStateChanged(nextState);
                }
            }

            Item {
                width: btnWidth * 2
                height: btnHeight
                clip: true

                GenericIconButton {
                    id: btnAlarmSelect
                    anchors.left: parent.left
                    width: parent.width * 0.5
                    height: btnHeight
                    captionText: " "
                    iconFontFamily: fontAwesome.name
                    iconText: "\uf150"
                    iconFontPixelSize: labelFontPixelSize * 0.8
                    iconColor: "red"
                    onBtnClicked: {
                        textAlarm.text = dsMcuSim.slotAlarmGetNext(textAlarm.text);
                    }
                }

                GenericIconButton {
                    id: btnAlarmTrigger
                    anchors.right: parent.right
                    width: parent.width * 0.5
                    height: btnHeight
                    captionText: " "
                    iconFontFamily: fontAwesome.name
                    iconText: "\uf003"
                    iconFontPixelSize: labelFontPixelSize
                    iconColor: "red"
                    onBtnClicked: {
                        dsMcuSim.slotAlarmActivate(textAlarm.text);
                    }
                }

                Text {
                    id: textAlarm
                    width: parent.width
                    y: parent.height * 0.6
                    height: parent.height - y
                    color: colorMap.text01
                    font.pixelSize: parent.height * 0.12
                    font.family: fontRobotoLight.name
                    text: "MCU_POST_BAD_SYSTEM_CRC"
                    wrapMode: Text.Wrap
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                }
            }

            Item {
                // spacing
                width: parent.width * 0.01
                height: parent.height
            }

            Item {
                property int borderWidth: height * 0.03

                anchors.verticalCenter: parent.verticalCenter
                id: ledContainer
                width: btnWidth * 1.2
                height: btnHeight

                Rectangle {
                    id: ledSaline
                    width: parent.width / 3
                    height: width
                    anchors.top: parent.top
                    anchors.topMargin: height * 0.3
                    anchors.right: ledContrast1.left
                    color: "transparent"
                    border.color: colorMap.text01
                    border.width: ledContainer.borderWidth

                    Text {
                        anchors.fill: parent
                        color: "white"
                        style: Text.Outline
                        styleColor: "black"
                        font.pixelSize: height * 0.7
                        text: (stopcockPositions.length > 0) ? stopcockPositions[0][0] : ""
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
                Rectangle {
                    id: ledContrast1
                    width: parent.width / 3
                    height: width
                    anchors.top: parent.top
                    anchors.topMargin: height * 0.3
                    anchors.horizontalCenter: parent.horizontalCenter
                    color: "transparent"
                    border.color: colorMap.text01
                    border.width: ledContainer.borderWidth

                    Text {
                        anchors.fill: parent
                        color: "white"
                        style: Text.Outline
                        styleColor: "black"
                        font.pixelSize: height * 0.7
                        text: (stopcockPositions.length > 0) ? stopcockPositions[1][0] : ""
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                Rectangle {
                    id: ledContrast2
                    width: parent.width / 3
                    height: width
                    anchors.top: parent.top
                    anchors.topMargin: height * 0.3
                    anchors.left: ledContrast1.right
                    color: "transparent"
                    border.color: colorMap.text01
                    border.width: ledContainer.borderWidth

                    Text {
                        anchors.fill: parent
                        color: "white"
                        style: Text.Outline
                        styleColor: "black"
                        font.pixelSize: height * 0.7
                        text: (stopcockPositions.length > 0) ? stopcockPositions[2][0] : ""
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                Rectangle {
                    id: ledOutletAirDoor
                    x: ledContrast2.x - (width * 0.6)
                    width: parent.width * 0.17
                    height: width
                    anchors.top: ledContrast1.bottom
                    anchors.topMargin: height * 0.4
                    radius: width / 2
                    color: "transparent"
                    border.color: colorMap.text01
                    border.width: ledContainer.borderWidth
                }

                Rectangle {
                    id: ledSuds
                    width: parent.width * 0.5
                    height: width
                    anchors.bottom: parent.bottom
                    radius: width / 2
                    color: "transparent"
                    border.color: colorMap.text01
                    border.width: ledContainer.borderWidth
                    anchors.horizontalCenter: parent.horizontalCenter

                    Text {
                        anchors.fill: parent
                        color: "white"
                        style: Text.Outline
                        styleColor: "black"
                        font.pixelSize: height * 0.55
                        text: mudsLineFluidSyringeIndex.replace("R", "").replace("NONE", "--")
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
                Rectangle {
                    id: ledDoorLeft
                    anchors.left: parent.left
                    anchors.bottom: parent.bottom
                    width: parent.width * 0.2
                    height: parent.height * 0.5
                    color: "transparent"
                    border.color: colorMap.text01
                    border.width: ledContainer.borderWidth
                }
                Rectangle {
                    id: ledDoorRight
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    width: parent.width * 0.2
                    height: parent.height * 0.5
                    color: "transparent"
                    border.color: colorMap.text01
                    border.width: ledContainer.borderWidth
                }
            }
        }
    }

    Item {
        x: flickable.x
        y: flickable.height * 0.95
        width: flickable.width
        height: flickable.height - y
        ScrollBar {
            y: 0
            flickable: flickable
        }
    }

    Item {
        x: flickable.x
        y: flickable.y
        width: flickable.width
        height: flickable.height
        ListFade {
            flickable: flickable
        }
    }

    onBaseBoardTypeChanged: {
        if (appMain.screenState == "OFF")
        {
            return;
        }

        if ( (baseBoardType == "OCS") ||
             (baseBoardType == "NoBattery") )
        {
            dsMcuSim.slotBatteryStateChanged("NO_BATTERY");
            dsMcuSim.slotAcConnectedStateChanged("AC");
        }
    }

    onLedControlStatusChanged: {
        if (ledControlStatus === undefined)
        {
            return;
        }
        ledSaline.color = getLedColor(ledSaline.color, ledControlStatus.Saline);
        ledContrast1.color = getLedColor(ledContrast1.color, ledControlStatus.Contrast1);
        ledContrast2.color = getLedColor(ledContrast2.color, ledControlStatus.Contrast2);
        ledOutletAirDoor.color = getLedColor(ledOutletAirDoor.color, ledControlStatus.AirDoor);
        ledSuds.color = getLedColor(ledSuds.color, ledControlStatus.Suds1);
        ledDoorLeft.color = getLedColor(ledDoorLeft.color, ledControlStatus.Door1);
        ledDoorRight.color = getLedColor(ledDoorRight.color, ledControlStatus.Door3);
    }

    onMcuSimulatorEnabledChanged: {
        if (!mcuSimulatorEnabled)
        {
            state = "INACTIVE";
        }
    }

    function getLedColor(curColor, newColorStr)
    {
        if (newColorStr === "NOCHANGE")
        {
            return curColor;
        }
        else if (newColorStr === "OFF")
        {
            return "transparent";
        }
        return "#" + newColorStr;
    }

    function toggleWidgetsVisible()
    {
        if (!mcuSimulatorEnabled)
        {
            return;
        }

        if (state == "INACTIVE")
        {
            state = "ACTIVE";
        }
        else
        {
            state = "INACTIVE";
        }
    }
}


