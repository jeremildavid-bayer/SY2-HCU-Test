import QtQuick 2.12
import "../../Widgets"
import "../../Util.js" as Util

Item {
    property var activeAlerts: dsAlert.activeAlerts
    property var plan: dsExam.plan
    property var noticeList: {
        var newNoticeList = [];

        if (plan === undefined)
        {
            return newNoticeList;
        }

        for (var stepIdx = 0; stepIdx < plan.Steps.length; stepIdx++)
        {
            var step = Util.copyObject(plan.Steps[stepIdx]);
            step.Index = stepIdx;
            // Get Personlaizationnotices from ProgrammedPlan.Steps
            var notices = step.PersonalizationNotices;
            if (notices.length > 0)
            {
                for (var noticeIdx = 0; noticeIdx < notices.length; noticeIdx++)
                {
                    var notice = notices[noticeIdx];
                    var noticeExist = false;
                    for (var noticeIdx2 = 0; noticeIdx2 < newNoticeList.length; noticeIdx2++)
                    {
                        if (newNoticeList[noticeIdx2].Notice.Name === notice.Name)
                        {
                            // Add item
                            newNoticeList[noticeIdx2].Steps = newNoticeList[noticeIdx2].Steps.concat( ', ', (stepIdx + 1).toString());
                            noticeExist = true;
                            break;
                        }
                    }

                    if (!noticeExist)
                    {
                        // Create new item
                        var noticeItem = { Type: "Personalization", Notice: notice, Steps: (stepIdx + 1).toString(), Values: [] };
                        newNoticeList.push(noticeItem);
                    }
                }
            }
            // Get Notices from ProgrammedPlan.Steps
            // Notices.Values contains 2 values: [0]Volume/Flow Rate delta & [1]Applicable Phase index
            notices = step.Notices;
            for (noticeIdx = 0; noticeIdx < notices.length; noticeIdx++)
            {
                notice = notices[noticeIdx];
                noticeExist = false;

                if (notice.Values[1] === -1) //Summary notice has phase index -1
                {
                    for (noticeIdx2 = 0; noticeIdx2 < newNoticeList.length; noticeIdx2++)
                    {
                        if (newNoticeList[noticeIdx2].Notice.Name === notice.Name)
                        {
                            // Add item
                            newNoticeList[noticeIdx2].Steps.push((stepIdx + 1).toString());
                            newNoticeList[noticeIdx2].Values.push(notice.Values[0]);
                            noticeExist = true;
                            break;
                        }

                    }

                    if (!noticeExist)
                    {
                        // Create new item
                        noticeItem = { Type: "Context", Notice: notice, Steps: [(stepIdx + 1).toString()], Values: [notice.Values[0]] };
                        newNoticeList.push(noticeItem);
                    }
                }
            }
        }
        //logDebug("newNoticeList=" + JSON.stringify(newNoticeList));
        return newNoticeList;
    }

    id: root

    Item {
        id: injectionPlanWarning
        width: parent.width
        height: (textAlert.text == "") ? 0 : Math.max(iconWarning.height, textAlert.height)

        WarningIcon {
            id: iconWarning
            width: (textAlert.text == "") ? 0 : parent.width * 0.086
            height: width
            visible: textAlert.text !== ""
        }

        Text {
            id: textAlert
            anchors.left: iconWarning.right
            anchors.leftMargin: parent.width * 0.02
            anchors.right: parent.right
            height: contentHeight
            font.pixelSize: root.height * 0.042
            font.family: fontRobotoMedium.name
            color: colorMap.text01
            wrapMode: Text.Wrap
            text: {
                for (var alertIdx = 0; alertIdx < activeAlerts.length; alertIdx++)
                {
                    if (activeAlerts[alertIdx].CodeName === "P3TUnusableConcentration")
                    {
                        return translate("T_" + activeAlerts[alertIdx].CodeName + "_UserDirection;" + activeAlerts[alertIdx].Data);
                    }
                    else if (activeAlerts[alertIdx].CodeName === "SI2009NonLoadedConcentration")
                    {
                        return translate("T_" + activeAlerts[alertIdx].CodeName + "_UserDirection;" + activeAlerts[alertIdx].Data);
                    }
                    else if (activeAlerts[alertIdx].CodeName === "DeparameterizedProtocol")
                    {
                        return translate("T_" + activeAlerts[alertIdx].CodeName + "_UserDirection;" + activeAlerts[alertIdx].Data);
                    }
                }
                return "";
            }
        }
    }

    Item {
        anchors.top: injectionPlanWarning.bottom
        anchors.topMargin: (injectionPlanWarning.height == 0) ? 0 : root.height * 0.03
        anchors.bottom: parent.bottom
        width: parent.width
        visible: {
            for (var alertIdx = 0; alertIdx < activeAlerts.length; alertIdx++)
            {
                if (activeAlerts[alertIdx].CodeName === "DeparameterizedProtocol")
                {
                    return false;
                }
            }
            return true;
        }

        ListView {
            anchors.top: parent.top
            anchors.topMargin: root.height * 0.02
            width: parent.width
            anchors.bottom: parent.bottom
            model: noticeList
            clip: true
            cacheBuffer: Math.max(contentHeight * 2, height * 10)
            ScrollBar {}
            ListFade {}

            delegate: Item {
                width: ListView.view.width
                height: textNoticeInjection.y + textNoticeInjection.height + root.height * 0.02
                Text {
                    id: infoNoticeIcon
                    width: parent.width * 0.11
                    height: width
                    color: colorMap.text01
                    font.family: fontIcon.name
                    font.pixelSize: height
                    visible: noticeList[index].Notice.Importance === "Info"
                    text: "\ue93a"
                    opacity: disableNotice(noticeList[index].Type, noticeList[index].Steps)
                }
                Text {
                    id: warningNoticeIcon
                    width: parent.width * 0.11
                    height: width
                    color: colorMap.text01
                    font.family: fontAwesome.name
                    font.pixelSize: height
                    visible: noticeList[index].Notice.Importance === "Warning"
                    text: "\uf071"
                    opacity: disableNotice(noticeList[index].Type, noticeList[index].Steps)
                }
                WarningIcon {
                    id: errorNoticeIcon
                    visible: noticeList[index].Notice.Importance === "Error"
                    opacity: disableNotice(noticeList[index].Type, noticeList[index].Steps)
                }
                Text {
                    id: textNotice
                    anchors.left: parent.left
                    anchors.leftMargin: parent.width * 0.14
                    anchors.right: parent.right
                    height: contentHeight
                    width: parent.width
                    color: colorMap.text01
                    font.pixelSize: root.width * 0.07
                    font.family: fontRobotoMedium.name
                    wrapMode: Text.Wrap
                    text:
                    {
                        var translationStr = translate("T_PersonalizationNoticeType_" + noticeList[index].Notice.Name);
                        if (noticeList[index].Type === "Personalization")
                        {
                            if ( (noticeList[index].Notice.Values !== null) && (translationStr.indexOf("{0}") >= 0) )
                            {
                                translationStr = translationStr.replace("{0}", noticeList[index].Notice.Values[0]);
                            }
                        }
                        else if (noticeList[index].Type === "Context")
                        {
                            translationStr = translate("T_PersonalizationNoticeType_" + noticeList[index].Notice.Name + "_PlanDisplay")
                        }

                        return translationStr;
                    }
                    opacity: disableNotice(noticeList[index].Type, noticeList[index].Steps)
                }
                Text {
                    id: textNoticeInjection
                    height: contentHeight
                    anchors.top: textNotice.bottom
                    anchors.left: textNotice.left
                    anchors.leftMargin: textNotice.width * 0.05
                    anchors.right: parent.right
                    font.pixelSize: root.width * 0.06
                    font.family: textNotice.font.family
                    color: colorMap.text02
                    wrapMode: Text.Wrap
                    opacity: disableNotice(noticeList[index].Type, noticeList[index].Steps)
                    text:
                    {
                        var textStr = "";
                        if (noticeList[index].Type === "Personalization")
                        {
                            textStr = translate("T_Injections") + translate("T_:") + " " + noticeList[index].Steps
                        }
                        else if (noticeList[index].Type === "Context")
                        {
                            var unit = (noticeList[index].Notice.Name.indexOf("FlowRate") >= 0) ? "ml/s" : "ml";
                            var precision = (noticeList[index].Notice.Name.indexOf("FlowRate") >= 0) ? 1 : 0;
                            for (var stepIndex = 0; stepIndex < noticeList[index].Steps.length; stepIndex++)
                            {
                                textStr += translate("T_Injection") + " " + noticeList[index].Steps[stepIndex] + translate("T_:") + " "
                                        + localeToFloatStr(noticeList[index].Values[stepIndex], precision) + " " + unit + "\n"
                            }
                        }
                        return textStr;
                    }
                }
            }
        }
    }

    function disableNotice(type, steps)
    {
        var noticeSteps = [];
        if (type === "Personalization")
        {
            noticeSteps = steps.split(', ');
        }
        else if (type === "Context")
        {
            for (var stepIndex = 0; stepIndex < steps.length; stepIndex++)
            {
                noticeSteps = steps[stepIndex];
            }

        }
            if ( (executingStep === undefined) || ( noticeSteps[noticeSteps.length - 1] < (executingStep.Index + 1) ) )
            {
                return 0.4;
            }
            else
            {
                return 1;
            }
    }
}
