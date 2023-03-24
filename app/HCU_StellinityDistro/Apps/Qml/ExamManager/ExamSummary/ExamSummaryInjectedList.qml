import QtQuick 2.12
import "../../Util.js" as Util
import "../../Widgets"

Column {
    property var plan: dsExam.plan
    property var selectedContrast: dsExam.selectedContrast
    property var fluidSourceSyringe1: dsDevice.fluidSourceSyringe1
    property var executedSteps: dsExam.executedSteps
    property int rowHeight: examSummaryView.height * 0.1

    width: parent.width
    height: salineInjectedInfo.y + salineInjectedInfo.height

    Text {
        id: textTitle
        width: parent.width
        height: rowHeight * 0.6
        font.pixelSize: height * 0.75
        font.family: fontRobotoMedium.name
        color: colorMap.text01
        verticalAlignment: Text.AlignVCenter
        text: translate("T_InjectedTotals")
    }

    ExamSummaryInjectedListItem {
        id: contrastInjectedInfo
        isContrastType: true
    }

    Rectangle {
        id: separator
        width: parent.width
        height: 2
        color: colorMap.text01
    }

    ExamSummaryInjectedListItem {
        id: salineInjectedInfo
        isContrastType: false
    }

    onVisibleChanged: {
        reload();
    }

    function reload()
    {
        if (!visible)
        {
            return;
        }

        var injectionCount = (executedSteps === undefined) ? 0 : executedSteps.length;
        var salineVolTotal = 0;
        var contrastVolTotal = 0;
        for (var stepIdx = 0; stepIdx < injectionCount; stepIdx++)
        {
            for (var phaseIdx = 0; phaseIdx < executedSteps[stepIdx].PhaseProgress.length; phaseIdx++)
            {
                var phaseValue = executedSteps[stepIdx].PhaseProgress[phaseIdx];
                salineVolTotal += phaseValue.InjectedVolumes["RS0"];
                contrastVolTotal += phaseValue.InjectedVolumes["RC1"] + phaseValue.InjectedVolumes["RC2"];
            }
        }

        contrastInjectedInfo.volInjected = localeToFloatStr(contrastVolTotal, 1);
        salineInjectedInfo.volInjected = localeToFloatStr(salineVolTotal, 1);
    }
}
