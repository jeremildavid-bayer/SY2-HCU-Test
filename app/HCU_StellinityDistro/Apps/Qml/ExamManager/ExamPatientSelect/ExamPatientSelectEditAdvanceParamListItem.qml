import QtQuick 2.12
import "../../Widgets"
import "../../Util.js" as Util
import "./ExamPatientSelectEditAdvanceParamListItemEgfr"

GenericButton {
    property var cruLinkStatus: dsCru.cruLinkStatus
    property var paramData: paramItems[index]
    id: root
    radius: 0
    anchors.horizontalCenter: parent.horizontalCenter
    height: rectValue.height
    width: ListView.view.width
    interactive: (listViewAdvanceParams.interactive && listViewAdvanceParamsList.interactive) && ((paramData !== undefined) && paramData.IsEnabled)

    Text {
        height: rectValue.height
        anchors.right: rectValue.left
        anchors.left: parent.left
        anchors.leftMargin: 5
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignLeft
        wrapMode: Text.Wrap
        font.pixelSize: rowHeight * 0.306
        font.family: fontRobotoLight.name
        text: (paramData === undefined) ? "" : translate("T_ExamField_" + paramData.Name + "_Name")
        color: root.interactive ? colorMap.text01 : colorMap.text02
        fontSizeMode: Text.Fit
        minimumPixelSize: font.pixelSize * 0.7

        MandatoryFieldMark {
            visible: (paramData !== undefined) && (cruLinkStatus.State === "Active") && (paramData.IsMandatory) && (!paramData.IsMandatoryEntered)
        }
    }

    Rectangle {
        id: rectValue
        width: parent.width * 0.5
        height: rectGenericItem.height + rectCatheterTypeItem.height + rectEgfrItem.height
        anchors.right: parent.right
        color: "transparent"

        ExamPatientSelectEditAdvanceParamListItemValue {
            id: rectGenericItem
            paramData: root.paramData
            visible: (paramData !== undefined) && (paramData.Name !== "CatheterType") && (paramData.Name !== "EgfrValue")
            height: visible ? contentHeight : 0
            interactive: root.interactive
        }

        ExamPatientSelectEditAdvanceParamListItemCatheterType {
            id: rectCatheterTypeItem
            visible: (paramData !== undefined) && (paramData.Name === "CatheterType")
            height: visible ? contentHeight : 0
            dataValue: paramDataValue
            interactive: root.interactive
            property var paramDataValue: {
                if (!visible || paramData === undefined)
                {
                    return "";
                }
                else if (paramData.ValidDataType === "object")
                {
                    return JSON.parse(paramData.Value);
                }
                return paramData.Value;
            }
        }

        EgfrValue {
            id: rectEgfrItem
            paramData: root.paramData
            visible: (paramData !== undefined) && (paramData.Name === "EgfrValue")
            height: visible ? egfrContentHeight : 0
            interactive: root.interactive
            // below is to override default width and provide wider width for EGFR
            width: root.width
            anchors.fill: undefined
            anchors.right: parent.right
            anchors.top: parent.top
        }
    }

    Rectangle {
        id: separatorLine
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        width: parent.width
        height: 1
        color: colorMap.text02
    }

    onBtnClicked: {
        if (paramData.Name === "EgfrValue")
        {
            if (widgetInputPad.isOpen() && curEditParamInfo.State === "EDITING_ADVANCE_PARAM")
            {
                widgetInputPad.close(true);
            }

            listViewAdvanceParams.setCurEditParamInfo({"State": "EGFR_PANEL_OPEN"});
            egfrPanel.open();
        }
        else
        {
            startEdit();
        }
    }

    Component.onCompleted: {
        listViewAdvanceParamsList.dragStarted.connect(reset);
        listViewAdvanceParams.signalResumeEditAdvanceParam.connect(resumeEdit);
        egfrPanel.signalClosed.connect(doneEdit);
    }

    Component.onDestruction: {
        listViewAdvanceParamsList.dragStarted.disconnect(reset);
        listViewAdvanceParams.signalResumeEditAdvanceParam.disconnect(resumeEdit);
        egfrPanel.signalClosed.disconnect(doneEdit);
    }

    function slotInputPadValChanged(newValue)
    {
        if (paramData.Name === "CatheterType")
        {
            rectCatheterTypeItem.textValue.text = newValue;
        }
        else
        {
            rectGenericItem.textValue.text = newValue;
        }
    }

    function startEdit()
    {
        if (widgetInputPad.isOpen())
        {
            widgetInputPad.close(true);
        }

        listViewAdvanceParams.setCurEditParamInfo({"State": "EDITING_ADVANCE_PARAM", "Index": index});

        widgetInputPad.signalValueChanged.disconnect(slotInputPadValChanged);
        widgetInputPad.signalClosed.disconnect(slotInputPadClosed);

        if (paramData.Name === "CatheterType")
        {
            widgetInputPad.openSelectListPad(rectValue, rectCatheterTypeItem.textValue, paramData.ValidList, paramData.Units, catheterTypeItemObjects);
        }
        else if (paramData.Name === "CatheterPlacement")
        {
            var validList = [];
            for (var idx = 0 ; idx < paramData.ValidList.length; idx++)
            {
                validList[idx] = JSON.parse(paramData.ValidList[idx]).Name;
                validList[idx] = (validList[idx] === null) ? "--" : validList[idx]
            }
            widgetInputPad.openSelectListPad(rectValue, rectGenericItem.textValue, validList, paramData.Units);
        }
        else if (paramData.ValidDataType === "double")
        {
            var decimalDigits = Util.getDecimalDigits(paramData.ValidRange.ResolutionIncrement.toString());
            widgetInputPad.openFloatPad(rectValue, rectGenericItem.textValue, paramData.Units, decimalDigits, paramData.ValidRange.LowerLimitInclusive, paramData.ValidRange.UpperLimitInclusive);
        }
        else if (paramData.ValidDataType === "int")
        {
            widgetInputPad.openIntegerPad(rectValue, rectGenericItem.textValue, paramData.Units, paramData.ValidRange.LowerLimitInclusive, paramData.ValidRange.UpperLimitInclusive);
        }
        else if (paramData.ValidDataType === "list")
        {
            widgetInputPad.openSelectListPad(rectValue, rectGenericItem.textValue, paramData.ValidList, paramData.Units);
        }

        widgetInputPad.signalValueChanged.connect(slotInputPadValChanged);
        widgetInputPad.signalClosed.connect(slotInputPadClosed);
    }

    function resumeEdit()
    {
        var editParamInfo = listViewAdvanceParams.getCurEditParamInfo();
        if ( (editParamInfo.State === "EDITING_ADVANCE_PARAM") &&
             (editParamInfo.Index === index) )
        {
            startEdit();
        }
    }

    function doneEdit()
    {
        listViewAdvanceParams.setCurEditParamInfo({ "State": "IDLE" });
    }

    function slotInputPadClosed(modified)
    {
        widgetInputPad.signalValueChanged.disconnect(slotInputPadValChanged);
        widgetInputPad.signalClosed.disconnect(slotInputPadClosed);

        if (modified)
        {
            if (paramData.Value !== widgetInputPad.currentValue.toString())
            {
                if (paramData.Name === "CatheterType")
                {
                    paramData.Value = widgetInputPad.currentValue;
                }
                else if (paramData.Name === "CatheterPlacement")
                {
                    if (widgetInputPad.currentValue === "--")
                    {
                        paramData.Value = paramData.ValidList[0];
                    }
                    else
                    {
                        for (var idx = 1 ; idx < paramData.ValidList.length; idx++)
                        {
                            if (JSON.parse(paramData.ValidList[idx]).Name === widgetInputPad.currentValue)
                            {
                                paramData.Value = paramData.ValidList[idx];
                                break;
                            }
                        }
                    }
                }
                else if (paramData.ValidDataType === "double")
                {
                    var decimalDigits = Util.getDecimalDigits(paramData.ValidRange.ResolutionIncrement.toString());
                    paramData.Value = localeFromFloatStr(widgetInputPad.currentValue, decimalDigits);
                }
                else
                {
                    // generic rule: if text is "--" then it should be empty
                    paramData.Value = ((widgetInputPad.currentValue === "--") ? "" : widgetInputPad.currentValue);
                }

                dsCru.slotPostUpdateExamField(paramData);

                // Invalidate the paramData. This is to force-update paramDataValue
                paramData = undefined;
                paramData = Qt.binding(function() { return paramItems[index]});

            }
        }

        if (paramData.Name === "EgfrValue")
        {
            rectEgfrItem.init();
        }
        else if (paramData.Name !== "CatheterType")
        {
            rectGenericItem.init();
        }

        doneEdit();
    }
}
