import QtQuick 2.12
import "../../Widgets"
import "../../Util.js" as Util

Item {
    id: root
    property string cultureCode: dsCfgGlobal.cultureCode
    property var selectedPlan: null
    property var planExecutingStep
    property var planExecutedSteps
    property var selectedContrast: dsExam.selectedContrast

    Text {
        id: summaryTitle
        width: parent.width
        height: parent.height * 0.22
        text: translate("T_ProgrammedTotals")
        color: colorMap.text01
        font.pixelSize: height * 0.9
        font.family: fontRobotoLight.name
        verticalAlignment: Text.AlignVCenter
        anchors.bottom: volumes.top
        anchors.bottomMargin: parent.height * 0.05
        fontSizeMode: Text.Fit
    }

    Row {
        id: volumes
        width: parent.width
        height: parent.height * 0.36
        anchors.bottom: duration.top
        anchors.bottomMargin: parent.height * 0.05
        spacing: parent.width * 0.02

        // Contrast total
        Text {
            id: iconContrast
            height: parent.height
            width: contentWidth
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: height * 0.7
            font.family: fontIcon.name
            text: "\ue92f"
            color: colorMap.contrast1
        }

        Text {
            id: txtTotalContrast
            height: parent.height
            text: "0"
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: height * 0.65
            font.family: fontRobotoBold.name
            color: colorMap.text01
        }

        Text {
            y: parent.height * 0.1
            height: parent.height - y
            text: translate("T_Units_ml")
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: height * 0.5
            font.family: fontRobotoLight.name
            color: colorMap.text02
        }

        // Spacer
        Item {
            width: parent.width * 0.03
            height: parent.height
        }

        // Saline total
        Text {
            height: parent.height
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: height * 0.7
            font.family: fontIcon.name
            text: "\ue930"
            color: colorMap.saline
        }

        Text {
            id: txtTotalSaline
            height: parent.height
            text: "0"
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: height * 0.65
            font.family: fontRobotoBold.name
            color: colorMap.text01
        }

        Text {
            y: parent.height * 0.1
            height: parent.height - y
            text: translate("T_Units_ml")
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: height * 0.5
            font.family: fontRobotoLight.name
            color: colorMap.text02
        }
    }

    // show duration of current step
    Row{
        id: duration
        width: parent.width
        height: (visible ? parent.height * 0.36 : 0)
        anchors.bottom: root.bottom
        spacing: parent.width * 0.04
        visible: false

        Text {
            id: durationIcon
            width: contentWidth
            height: parent.height
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: height * 0.55
            font.family: fontIcon.name
            text: "\ue94c"
            color: colorMap.text01
        }

        Text {
            id: durationText
            height: parent.height
            width: contentWidth
            text: "00:00"
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: height * 0.65
            font.family: fontRobotoBold.name
            color: colorMap.text01
        }
    }

    onCultureCodeChanged: {
        if (!visible)
        {
            return;
        }
        reloadInjectionPlan();
        reloadContrastValue();
    }

    onSelectedPlanChanged: {
        if (!visible)
        {
            return;
        }
        reloadInjectionPlan();
    }

    onPlanExecutedStepsChanged: {
        if (!visible)
        {
            return;
        }
        reloadInjectionPlan();
    }

    onPlanExecutingStepChanged: {
        if (!visible)
        {
            return;
        }
        reloadInjectionPlan();
    }

    onSelectedContrastChanged:  {
        if (!visible)
        {
            return;
        }
        reloadContrastValue();
    }

    onVisibleChanged: {
        if (!visible)
        {
            return;
        }

        reloadInjectionPlan();
        reloadContrastValue();
    }

    function showPlanOverview()
    {
        // if all injections are done then show T_PlanTotals, if this is only overview (no injections done) then show T_ProgrammedTotals
        if ((planExecutedSteps !== undefined) && (planExecutedSteps.length === 0))
        {
            summaryTitle.text = translate("T_ProgrammedTotals");
        }
        else
        {
            summaryTitle.text = translate("T_PlanTotals");
        }

        txtTotalContrast.text = localeToFloatStr(Math.ceil(selectedPlan.ContrastTotal), 0);
        txtTotalSaline.text = localeToFloatStr(Math.ceil(selectedPlan.SalineTotal), 0);

        // hide and reset duration
        duration.visible = false;
        durationText.text = "00:00";
    }

    function showCurrentStepOverview()
    {
        summaryTitle.text = translate("T_StepTotals")
        txtTotalContrast.text = localeToFloatStr(Math.ceil(getContrastTotalFromStep()), 0);
        txtTotalSaline.text = localeToFloatStr(Math.ceil(getSalineTotalFromStep()), 0);

        duration.visible = true;
        durationText.text = Util.getFormattedDurationStr(Util.millisecToDurationStr(getTotalDurationMillisecFromStep()), "H:mm:ss");
    }

    function getContrastTotalFromStep()
    {
        var total = 0;
        for (var i = 0; i < planExecutingStep.Phases.length; i++)
        {
            if (planExecutingStep.Phases[i].Type === "Fluid")
            {
                total += ((planExecutingStep.Phases[i].ContrastPercentage * planExecutingStep.Phases[i].TotalVolume) / 100);
            }
        }

        return total;
    }

    function getSalineTotalFromStep()
    {
        var total = 0;
        for (var i = 0; i < planExecutingStep.Phases.length; i++)
        {
            if (planExecutingStep.Phases[i].Type === "Fluid")
            {
                total += (((100 - planExecutingStep.Phases[i].ContrastPercentage) * planExecutingStep.Phases[i].TotalVolume) / 100);
            }
        }

        return total;
    }

    function getTotalDurationMillisecFromStep()
    {
        var total = 0;
        for (var i = 0; i < planExecutingStep.Phases.length; i++)
        {
            total += Util.durationStrToMillisec(planExecutingStep.Phases[i].Duration);
        }

        return total;
    }

    function reloadContrastValue()
    {
        if (selectedContrast === undefined)
        {
            return;
        }

        iconContrast.color = (selectedContrast.ColorCode === "GREEN") ? colorMap.contrast1 : colorMap.contrast2;
    }

    function reloadInjectionPlan()
    {
        // Update total fluid usage
        if ( (selectedPlan === null ) ||
             (selectedPlan === undefined) )
        {
            return;
        }

        // if there is no executing step (either all injections are done or this is just overview) then show plan overview
        if ( planExecutingStep === undefined)
        {
            showPlanOverview();
        }
        else
        {
            showCurrentStepOverview();
        }
    }
}

