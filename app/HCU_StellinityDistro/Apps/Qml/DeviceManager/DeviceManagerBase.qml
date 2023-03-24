import QtQuick 2.12

Item {
    property var fluidSourceMuds: dsDevice.fluidSourceMuds
    property var fluidSourceSyringe1: dsDevice.fluidSourceSyringe1
    property var fluidSourceSyringe2: dsDevice.fluidSourceSyringe2
    property var fluidSourceSyringe3: dsDevice.fluidSourceSyringe3
    property var fluidSourceBottle1: dsDevice.fluidSourceBottle1
    property var fluidSourceBottle2: dsDevice.fluidSourceBottle2
    property var fluidSourceBottle3: dsDevice.fluidSourceBottle3
    property var fluidSourceBottlePackages1: dsDevice.fluidSourceBottlePackages1
    property var fluidSourceBottlePackages2: dsDevice.fluidSourceBottlePackages2
    property var fluidSourceBottlePackages3: dsDevice.fluidSourceBottlePackages3
    property var fluidSourceWasteContainer: dsDevice.fluidSourceWasteContainer
    property var salineSelectItems: dsDevice.salineSelectItems
    property var contrastSelectItems: dsDevice.contrastSelectItems
    property var fluidSourceSuds: dsDevice.fluidSourceSuds
    property string mudsLineFluidSyringeIndex: dsDevice.mudsLineFluidSyringeIndex
    property bool sameContrasts: dsDevice.sameContrasts
    property var activeAlertsWasteContainer: dsAlert.activeAlertsWasteContainer
    property var activeAlertsMuds: dsAlert.activeAlertsMuds
    property var activeAlertsSuds: dsAlert.activeAlertsSuds
    property var activeAlertsSyringe1: dsAlert.activeAlertsSyringe1
    property var activeAlertsSyringe2: dsAlert.activeAlertsSyringe2
    property var activeAlertsSyringe3: dsAlert.activeAlertsSyringe3
    property var activeAlertsBottle1: dsAlert.activeAlertsBottle1
    property var activeAlertsBottle2: dsAlert.activeAlertsBottle2
    property var activeAlertsBottle3: dsAlert.activeAlertsBottle3
    property double syringeVolume1: getVolume(fluidSourceSyringe1, 0)
    property double syringeVolume2: getVolume(fluidSourceSyringe2, 1)
    property double syringeVolume3: getVolume(fluidSourceSyringe3, 2)
    property string fluidC2Color: sameContrasts ? colorMap.contrast1 : colorMap.contrast2
    property string mudsLineFluidColor: getMudsLineFluidColor()
    property string sudsLineFluidColor: getSudsLineFluidColor()
    property var plungerStates: dsMcu.plungerStates
    property string fluidSourceBrandC1: ( (fluidSourceBottle2 !== undefined) && (fluidSourceBottle2.SourcePackages !== undefined) && (fluidSourceBottle2.SourcePackages.length > 0) ) ? fluidSourceBottle2.SourcePackages[0].Brand : ""
    property string fluidSourceBrandC2: ( (fluidSourceBottle3 !== undefined) && (fluidSourceBottle3.SourcePackages !== undefined) && (fluidSourceBottle3.SourcePackages.length > 0) ) ? fluidSourceBottle3.SourcePackages[0].Brand : ""
    property int fluidSourceConcentrationC1: ( (fluidSourceBottle2 !== undefined) && (fluidSourceBottle2.SourcePackages !== undefined) && (fluidSourceBottle2.SourcePackages.length > 0) ) ? fluidSourceBottle2.SourcePackages[0].Concentration : 0
    property int fluidSourceConcentrationC2: ( (fluidSourceBottle3 !== undefined) && (fluidSourceBottle3.SourcePackages !== undefined) && (fluidSourceBottle3.SourcePackages.length > 0) ) ? fluidSourceBottle3.SourcePackages[0].Concentration : 0
    property string logPrefix: "DeviceManagerBase: "

    property var plan: dsExam.plan

    onMudsLineFluidColorChanged: {
        var fluidColor = mudsLineFluidColor;
        if (mudsLineFluidColor === colorMap.contrast1)
        {
            fluidColor = "Contrast1";
        }
        else if (mudsLineFluidColor === colorMap.contrast2)
        {
            fluidColor = "Contrast2";
        }
        else if (mudsLineFluidColor === colorMap.saline)
        {
            fluidColor = "Saline";
        }
        else if (mudsLineFluidColor === colorMap.deviceIconMissing)
        {
            fluidColor = "Missing";
        }

        logDebug(logPrefix + "mudsLineFluidColor = " + fluidColor);
    }

    function getVolume(fluidSourceSyringe, syringeIdx)
    {
        if ( (fluidSourceSyringe === undefined) ||
             (fluidSourceSyringe.CurrentVolumes === undefined) ||
             (fluidSourceSyringe.CurrentVolumes.length === 0) )
        {
            return -1;
        }

        return Math.floor(fluidSourceSyringe.CurrentVolumes[0]);
    }

    function getMudsLineFluidColor()
    {
        if ((fluidSourceMuds !== undefined) && (fluidSourceMuds.SourcePackages !== undefined) && (fluidSourceMuds.SourcePackages.length > 0))
        {
            var mudsSourceMatchesSyringe2 = false;
            var mudsSourceMatchesSyringe3 = false;
            let mudsSourcePackages = fluidSourceMuds.SourcePackages;
            for (const syringe of [fluidSourceSyringe1, fluidSourceSyringe2, fluidSourceSyringe3]) {
                if ((syringe.SourcePackages !== undefined) && (syringe.SourcePackages.length > 0) &&
                        (syringe.SourcePackages[0].Brand === mudsSourcePackages[mudsSourcePackages.length - 1].Brand) &&
                        (syringe.SourcePackages[0].Concentration === mudsSourcePackages[mudsSourcePackages.length - 1].Concentration)) {
                    if (syringe === fluidSourceSyringe1) return colorMap.saline;
                    if (syringe === fluidSourceSyringe2) mudsSourceMatchesSyringe2 = true;
                    if (syringe === fluidSourceSyringe3) mudsSourceMatchesSyringe3 = true;
                }
            }

            if (mudsSourceMatchesSyringe2)
            {
                if (mudsSourceMatchesSyringe3 && !sameContrasts)
                {
                    return (fluidSourceBottlePackages3[0] !== undefined) ? colorMap.contrast2 : colorMap.contrast1;
                }
                else return colorMap.contrast1;
            }
            else if (mudsSourceMatchesSyringe3)
            {
                return colorMap.contrast2;
            }
        }
        return colorMap.deviceIconMissing;
    }

    function getSudsLineFluidColor()
    {
        var color = colorMap.deviceIconMissing;
        if (fluidSourceSuds === undefined) return color;
        if (fluidSourceSuds.CurrentFluidKinds === undefined) return color;
        if (plan === undefined || plan.Steps === undefined || plan.Steps.length === 0) return color;

        for (var i = 0; i < fluidSourceSuds.CurrentFluidKinds.length; i++)
        {
            if (fluidSourceSuds.CurrentFluidKinds[i] === "Contrast")
            {
                if (plan.Steps[0].ContrastFluidLocationName === "RC1")
                {
                    color = colorMap.contrast1;
                    break;
                }
                else if (plan.Steps[0].ContrastFluidLocationName === "RC2")
                {
                    color = fluidC2Color;
                    break;
                }
            }
            else if (fluidSourceSuds.CurrentFluidKinds[i] === "Flush")
            {
                color = colorMap.saline;
            }
        }

        return color;
    }
}
