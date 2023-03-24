import QtQuick 2.12
import "../Widgets"
import "../Widgets/Popup"
import "../Util.js" as Util

Popup {
    property var injectionRequestProcessStatus: dsExam.injectionRequestProcessStatus
    property var plan: dsExam.plan
    property var executingStep: dsExam.executingStep
    property var examAdvanceInfo: dsExam.examAdvanceInfo
    property var examInputs: (examAdvanceInfo !== undefined) ? examAdvanceInfo.ExamInputs : undefined
    property string pressureUnit: dsCfgGlobal.pressureUnit
    property double pressureLimit: 0.0
    property double flowRateLimit: 0.0

    type: "SPECIAL_WARNING"
    titleText: "T_LimitsExceeded"
    heightMin: Math.max(dsCfgLocal.screenH * 0.44, ( (dsCfgLocal.screenH * 0.3) + textCatheterType.height + rowCatheterFlowRate.height + rowCatheterPressure.height + textInjectionSite.height + rowInjectionSiteFlowRate.height + textAutoAdjustInstruction.height))

    // OtherBtn will be used for "Yes" to swap "Yes" and "No" buttons
    showOtherBtn: true
    enableOtherBtn: true
    showOkBtn: false
    enableOkBtn: false

    otherBtnText: "T_Yes"
    cancelBtnText: "T_No"

    content: [
        Text {
            id: textCatheterType
            anchors.top: parent.top
            anchors.topMargin: contentHeight * 0.2
            width: parent.width
            text: ( (examInputs === undefined) || (examInputs["CatheterType"] === undefined) ) ? "" : "'" + JSON.parse(examInputs["CatheterType"].Value).Name + "' " + translate("T_Catheter")
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            color: colorMap.blk01
            font.pixelSize: contentfontPixelSize
            font.family: fontRobotoMedium.name
        },

        Item {
            id: rowCatheterPressure
            anchors.top: textCatheterType.bottom
            anchors.left: parent.left
            anchors.leftMargin: (parent.width - catheterPressureText.width - catheterPressureValue.width - 20) * 0.5

            Text {
                id: catheterPressureText
                verticalAlignment: Text.AlignVCenter
                text: translate("T_MaximumPressure") + translate("T_:")
                color: colorMap.blk01
                font.pixelSize: contentfontPixelSize
                font.family: fontRobotoLight.name
            }

            Text {
                id: catheterPressureValue
                height: catheterPressureText.contentHeight
                anchors.left: catheterPressureText.right
                anchors.leftMargin: 20
                verticalAlignment: Text.AlignVCenter
                color: colorMap.blk01
                font.pixelSize: contentfontPixelSize
                font.family: fontRobotoMedium.name
            }
        },

        Item {
            id: rowCatheterFlowRate
            anchors.top: rowCatheterPressure.bottom
            anchors.left: parent.left
            anchors.leftMargin: (parent.width - catheterFlowRateText.width - catheterFlowRateValue.width- 20) * 0.5
            width: contentWidth

            Text {
                id: catheterFlowRateText
                verticalAlignment: Text.AlignVCenter
                text: translate("T_MaximumFlowRate") + translate("T_:")
                color: colorMap.blk01
                font.pixelSize: contentfontPixelSize
                font.family: fontRobotoLight.name
            }


            Text {
                id: catheterFlowRateValue
                height: catheterFlowRateText.contentHeight
                anchors.left: catheterFlowRateText.right
                anchors.leftMargin: 20
                verticalAlignment: Text.AlignVCenter
                color: colorMap.blk01
                font.pixelSize: contentfontPixelSize
                font.family: fontRobotoMedium.name
            }

        },

        Text {
            id: textInjectionSite
            anchors.top: rowCatheterFlowRate.bottom
            anchors.topMargin: contentHeight * 0.2
            width: parent.width
            text: (examInputs === undefined || examInputs["CatheterPlacement"] === undefined) ? "" : "'" + JSON.parse(examInputs["CatheterPlacement"].Value).Name + "' " + translate("T_InjectionSite")
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            color: colorMap.blk01
            font.pixelSize: contentfontPixelSize
            font.family: fontRobotoMedium.name
        },

        Item {
            id: rowInjectionSiteFlowRate
            anchors.top: textInjectionSite.bottom
            anchors.left: parent.left
            anchors.leftMargin: (parent.width - injectionSiteFlowRateText.width - injectionSiteFlowRateValue.width - 20) * 0.5

            Text {
                id: injectionSiteFlowRateText
                verticalAlignment: Text.AlignVCenter
                text: translate("T_MaximumFlowRate") + translate("T_:")
                color: colorMap.blk01
                font.pixelSize: contentfontPixelSize
                font.family: fontRobotoLight.name
            }

            Text {
                id: injectionSiteFlowRateValue
                height: injectionSiteFlowRateText.contentHeight
                anchors.left: injectionSiteFlowRateText.right
                anchors.leftMargin: 20
                verticalAlignment: Text.AlignVCenter
                color: colorMap.blk01
                font.pixelSize: contentfontPixelSize
                font.family: fontRobotoMedium.name
            }
        },

        Text {
            id: textAutoAdjustInstruction
            anchors.bottom: parent.bottom
            width: parent.width
            height: contentHeight * 1.2
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            color: colorMap.blk01
            font.pixelSize: contentfontPixelSize
            font.family: fontRobotoLight.name
            wrapMode: Text.Wrap
        }
    ]

    // This is "Yes" button
    onBtnOtherClicked: {
        dsCru.slotApplyLimits();
        close();
    }

    onBtnCancelClicked: {
        dsAlert.slotActivateAlert("CatheterLimitsExceededAccepted","");
        close();
        logInfo("PopupInjectionRequestFailed: Catheter limit accepted by user. Arm again..");
        dsExam.slotInjectionArmed();
    }

    onInjectionRequestProcessStatusChanged: {
        if (injectionRequestProcessStatus === undefined)
        {
            return;
        }

        if ( (injectionRequestProcessStatus.State.indexOf("T_ARMFAILED_CatheterCheckNeeded") >= 0) &&
                (injectionRequestProcessStatus.RequestedByHcu) )
        {
            getPopupContent();
            open();
        }
        else
        {
            close();
        }
    }

    onPlanChanged: {
        if (isOpen())
        {
            // Plan changed outside (possible from CRU). Close popup.
            close();
        }
    }

    function getPopupContent()
    {
        var personalizationNoticeName;

        textCatheterType.visible = false;
        textCatheterType.height = 0;

        rowCatheterPressure.visible = false;
        rowCatheterPressure.height = 0;
        rowCatheterFlowRate.visible = false;
        rowCatheterFlowRate.height = 0;

        textInjectionSite.visible = false;
        textInjectionSite.height = 0;

        rowInjectionSiteFlowRate.visible = false;
        rowInjectionSiteFlowRate.height = 0;

        pressureLimit = 0;
        flowRateLimit = 0;

        if ( (executingStep === undefined) || (plan === undefined) )    
        {
            return;
        }
        else
        {
            for (var idx = executingStep.Index ; idx < plan.Steps.length ; idx++)
            {
                for (var noticeIdx = 0 ; noticeIdx < plan.Steps[idx].PersonalizationNotices.length ; noticeIdx++)
                {
                    personalizationNoticeName = plan.Steps[idx].PersonalizationNotices[noticeIdx].Name;

                    if (personalizationNoticeName === "PressureLimitExceedsCatheterTypeLimit")
                    {
                        textCatheterType.visible = true;
                        textCatheterType.height = textCatheterType.contentHeight * 1.5;
                        rowCatheterPressure.visible = true;
                        rowCatheterPressure.height = catheterPressureText.contentHeight * 1.2;
                        pressureLimit = plan.Steps[idx].PersonalizationNotices[noticeIdx].Values[0];
                        var pressureLimitStr = "";
                        if (pressureUnit == "kg/cm2")
                        {
                            pressureLimitStr = localeToFloatStr(Util.getPressure(pressureUnit, pressureLimit), 1);
                        }
                        else
                        {
                            pressureLimitStr = localeToFloatStr(Util.getPressure(pressureUnit, pressureLimit), 0);
                        }
                        catheterPressureValue.text = pressureLimitStr + " " + translate("T_Units_" + pressureUnit);
                    }
                    else if (personalizationNoticeName === "FlowRateExceedsCatheterTypeLimit")
                    {
                        textCatheterType.visible = true;
                        textCatheterType.height = textCatheterType.contentHeight * 1.5;
                        rowCatheterFlowRate.visible = true;
                        rowCatheterFlowRate.height = catheterFlowRateText.contentHeight * 1.2;
                        if (flowRateLimit === 0)
                        {
                            flowRateLimit = plan.Steps[idx].PersonalizationNotices[noticeIdx].Values[0];
                        }
                        else
                        {
                            flowRateLimit = Math.min(flowRateLimit, plan.Steps[idx].PersonalizationNotices[noticeIdx].Values[0]);
                        }
                        catheterFlowRateValue.text = localeToFloatStr(flowRateLimit, 1) + " " + translate("T_Units_ml/s");
                    }
                    else if (personalizationNoticeName === "FlowRateExceedsCatheterPlacementLimit")
                    {
                        textInjectionSite.visible = true;
                        textInjectionSite.height = textInjectionSite.contentHeight * 1.5;
                        rowInjectionSiteFlowRate.visible = true;
                        rowInjectionSiteFlowRate.height = injectionSiteFlowRateText.contentHeight * 1.2;
                        if (flowRateLimit === 0)
                        {
                            flowRateLimit = plan.Steps[idx].PersonalizationNotices[noticeIdx].Values[0];
                        }
                        else
                        {
                            flowRateLimit = Math.min(flowRateLimit, plan.Steps[idx].PersonalizationNotices[noticeIdx].Values[0]);
                        }
                        injectionSiteFlowRateValue.text = localeToFloatStr(flowRateLimit, 1) + " " + translate("T_Units_ml/s");
                    }
                }
            }

            if ( (flowRateLimit !== 0) && (pressureLimit !== 0) )
            {
                textAutoAdjustInstruction.text = translate("T_AutomaticallyReduceFlowRateAndChangePressureLimit");
            }
            else if (pressureLimit !== 0)
            {
                textAutoAdjustInstruction.text = translate("T_AutomaticallyChangePressureLimit");
            }
            else if (flowRateLimit !== 0)
            {
                textAutoAdjustInstruction.text = translate("T_AutomaticallyReduceFlowRate");
            }
            else
                textAutoAdjustInstruction.visible = false;
        }
    }
}

