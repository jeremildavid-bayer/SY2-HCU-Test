import QtQuick 2.12
import "../../Widgets"
import "../../Util.js" as Util

Item {
    property var paramData: paramItems[index]

    id: root
    anchors.horizontalCenter: parent.horizontalCenter
    width: ListView.view.width * 0.94
    height: rowHeight

    Text {
        height: parent.height * 0.9
        anchors.right: btnEdit.left
        anchors.left: parent.left
        anchors.leftMargin: parent.width * 0.02
        anchors.verticalCenter: parent.verticalCenter
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignLeft
        wrapMode: Text.Wrap
        font.pixelSize: height * 0.4
        font.family: fontRobotoLight.name
        text: translate("T_P3T_" + paramData.Name + "_Name")
        color: paramData.IsEnabled ? colorMap.text01 : colorMap.text02
        fontSizeMode: Text.Fit
        minimumPixelSize: font.pixelSize * 0.7
    }

    GenericButton {
        id: btnEdit
        radius: 0
        width: parent.width * 0.45
        height: parent.height
        anchors.right: parent.right
        interactive: paramData.IsEnabled

        Text {
            id: textValue
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.rightMargin: textUnit.contentWidth + (root.width * 0.055)
            height: parent.height
            font.pixelSize: height * 0.48
            font.family: fontRobotoBold.name
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignRight
            text: getTextValue()
            color: getTextColor()
        }

        Text {
            id: textUnit
            anchors.right: parent.right
            anchors.rightMargin: root.width * 0.04
            anchors.top: parent.top
            anchors.topMargin: height * 0.04
            width: root.width * 0.3
            height: parent.height
            font.family: fontRobotoLight.name
            font.pixelSize: textValue.font.pixelSize * 0.73
            wrapMode: Text.Wrap
            color: getUnitColor()
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignRight
            text: {
                if (paramData.Units !== "")
                {
                    return translate("T_Units_" + paramData.Units);
                }
                return "";
            }
        }

        onBtnClicked: {
            startEdit();
        }
    }

    Rectangle {
        id: separatorLine
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        width: parent.width
        height: 1
        color: colorMap.text02
        visible: index < (paramItems.length - 1)
    }

    Component.onCompleted: {
        listViewStep.dragStarted.connect(resetButtons);
        injectionPlanEdit.signalResumeEditProtParam.connect(resumeEdit);
    }

    Component.onDestruction: {
        listViewStep.dragStarted.disconnect(resetButtons);
        injectionPlanEdit.signalResumeEditProtParam.disconnect(resumeEdit);
    }

    function resetButtons()
    {
        btnEdit.reset();
    }

    function slotInputPadValChanged(newValue)
    {
        textValue.text = newValue;
    }

    function startEdit()
    {
        if (widgetInputPad.isOpen())
        {
            widgetInputPad.close(true);
        }

        injectionPlanEdit.setCurEditParamInfo({ "State": "EDITING_PROT_PARAM", "IsPlanLevel": isPlanLevel, "Index": index});

        widgetInputPad.signalValueChanged.disconnect(slotInputPadValChanged);
        widgetInputPad.signalClosed.disconnect(slotInputPadClosed);

        if (paramData.ValidDataType === "double")
        {
            widgetInputPad.openFloatPad(btnEdit, textValue, paramData.Units, 1, paramData.ValidRange.LowerLimitInclusive, paramData.ValidRange.UpperLimitInclusive);
        }
        else if (paramData.ValidDataType === "int")
        {
            widgetInputPad.openIntegerPad(btnEdit, textValue, paramData.Units, paramData.ValidRange.LowerLimitInclusive, paramData.ValidRange.UpperLimitInclusive);
        }
        else if (paramData.ValidDataType === "list")
        {
            widgetInputPad.openSelectListPad(btnEdit, textValue, paramData.ValidList, paramData.Units);
        }

        widgetInputPad.signalValueChanged.connect(slotInputPadValChanged);
        widgetInputPad.signalClosed.connect(slotInputPadClosed);
    }

    function resumeEdit()
    {
        var editParamInfo = injectionPlanEdit.getCurEditParamInfo();
        if ( (editParamInfo.State === "EDITING_PROT_PARAM") &&
             (editParamInfo.IsPlanLevel === isPlanLevel) &&
             (editParamInfo.Index === index) )
        {
            startEdit();
        }
    }

    function slotInputPadClosed(modified)
    {
        widgetInputPad.signalValueChanged.disconnect(slotInputPadValChanged);
        widgetInputPad.signalClosed.disconnect(slotInputPadClosed);

        if (modified)
        {
            if ( (!paramData.IsValueVisible) ||
                 (paramData.Value !== widgetInputPad.currentValue.toString()) )
            {
                // paramData is changed by (1)Not visible to visible OR (2)value(string type) changed: update to CRU


                if (paramData.ValidDataType === "double")
                {
                    paramData.Value = localeFromFloatStr(widgetInputPad.currentValue);
                }
                else
                {
                    paramData.Value = widgetInputPad.currentValue;
                }
                //logDebug("paramData.Value=" + paramData.Value + ", planEditData.PersonalizationInputs=" + JSON.stringify(planEditData.PersonalizationInputs));

                // Send the parameter changed to CRU
                dsCru.slotPostUpdateInjectionParameter(paramData);
            }
        }

        textValue.text = Qt.binding(function() { return getTextValue(); });
        textValue.color = Qt.binding(function() { return getTextColor(); });

        injectionPlanEdit.setCurEditParamInfo({ "State": "IDLE" });
    }

    function getTextValue()
    {
        if (!paramData.IsValueVisible)
        {
            return "--";
        }
        else if (paramData.ValidDataType === "double")
        {
            return localeToFloatStr(paramData.Value, 1);
        }
        return paramData.Value;
    }

    function getTextColor()
    {
        if (paramData.IsEnabled)
        {
            return (textValue.text.indexOf("--") >= 0) ? colorMap.errText : colorMap.text01;
        }
        return colorMap.text02;
    }

    function getUnitColor()
    {
        if (paramData.IsEnabled)
        {
            return (textValue.text.indexOf("--") >= 0) ? colorMap.errText : colorMap.text02;
        }
        return colorMap.text02;
    }
}
