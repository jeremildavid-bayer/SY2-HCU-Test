import QtQuick 2.12
import "../../Widgets"
import "../../Util.js" as Util
import "AdminContrasts.js" as ContrastsUtil

Item {
    property int fluidOptionContrastBrandLenMax: dsCapabilities.fluidOptionContrastBrandLenMax
    property int fluidOptionContrastConcentrationMin: dsCapabilities.fluidOptionContrastConcentrationMin
    property int fluidOptionContrastConcentrationMax: dsCapabilities.fluidOptionContrastConcentrationMax
    property var groupData
    property string editType: "NONE" // "BRAND", "CONCENTRATION"
    id: root

    Text {
        id: labelBrand
        anchors.left: parent.left
        width: parent.width * 0.68
        height: parent.height * 0.3
        verticalAlignment: Text.AlignVCenter
        text: translate("T_Brand")
        font.pixelSize: height * 0.65
        font.family: fontRobotoLight.name
        color: colorMap.text01
    }

    GenericButton {
        id: btnBrand
        x: labelBrand.x
        width: labelBrand.width
        anchors.top: labelBrand.bottom
        anchors.bottom: parent.bottom
        color: colorMap.editFieldBackground

        content: [
            Text {
                id: textBrand
                anchors.fill: parent
                anchors.margins: root.width * 0.015
                font.pixelSize: height * 0.56
                font.family: fontRobotoBold.name
                verticalAlignment: Text.AlignVCenter
                color: isBrandValueOk() ? colorMap.text01 : colorMap.errText
                elide: Text.ElideRight
            }
        ]
        onBtnClicked: {
            startEdit("BRAND");
        }
    }

    Text {
        id: labelConcentration
        anchors.left: labelBrand.right
        anchors.leftMargin: parent.width * 0.03
        anchors.right: parent.right
        anchors.rightMargin: parent.width * 0.078
        height: parent.height * 0.3
        verticalAlignment: Text.AlignVCenter
        text: translate("T_Concentration")
        font.pixelSize: height * 0.65
        font.family: fontRobotoLight.name
        color: colorMap.text01
    }

    GenericButton {
        id: btnConcentration
        x: labelConcentration.x
        width: labelConcentration.width
        anchors.top: labelConcentration.bottom
        anchors.bottom: parent.bottom
        color: colorMap.editFieldBackground

        content: [
            Text {
                id: textConcentration
                anchors.left: parent.left
                anchors.margins: root.width * 0.015
                width: contentWidth
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                font.pixelSize: height * 0.56
                font.family: fontRobotoBold.name
                verticalAlignment: Text.AlignVCenter
                color: isConcentrationValueOk() ? colorMap.text01 : colorMap.errText
            },
            Text {
                id: concentrationUnit
                anchors.left: textConcentration.right
                anchors.right: parent.right
                anchors.leftMargin: root.width * 0.008
                anchors.rightMargin: root.width * 0.02
                anchors.top: parent.top
                anchors.topMargin: root.width * 0.02
                anchors.bottom: parent.bottom
                anchors.bottomMargin: root.width * 0.02
                font.pixelSize: height * 0.6
                font.family: fontRobotoLight.name
                verticalAlignment: Text.AlignVCenter
                color: colorMap.text02
                text: translate("T_Units_mg/ml")
                elide: Text.ElideRight
            }
        ]
        onBtnClicked: {
            startEdit("CONCENTRATION");
        }
    }

    function reload()
    {
        if ( (selectedFamilyIdx === -1) ||
             (selectedGroupIdx === -1) ||
             (contrastsPage.contrastFamilies[selectedFamilyIdx] === undefined) ||
             (contrastsPage.contrastFamilies[selectedFamilyIdx].Groups[selectedGroupIdx] === undefined) )
        {
            groupData = undefined;
            return;
        }

        var groupDataBuf = ContrastsUtil.getGroupData(selectedFamilyIdx, selectedGroupIdx);
        groupData =  Util.copyObject(groupDataBuf);

        //logDebug("AdminContrastsMainHeader: Reload: reloadReason=" + reloadReason + ", FamilyIdx=" + selectedFamilyIdx + ", GroupIdx=" + selectedGroupIdx + ", GroupData=" + JSON.stringify(groupData));

        if ( (groupData.Brand === "") ||
             (groupData.Brand === "<NEW>") )
        {
            textBrand.text = "--";
        }
        else
        {
            textBrand.text = groupData.Brand;
        }

        if (groupData.Concentration === 0)
        {
            textConcentration.text = "--";
        }
        else
        {
            textConcentration.text = groupData.Concentration.toString();
        }

        if ( (reloadReason == "ADDED") &&
             (groupData.Brand === "<NEW>") )
        {
            // "Add Group" selected, start edit brand
            //logDebug("reloadReason=" + reloadReason + ", startEdit(BRAND)");
            startEdit("BRAND");
        }
    }

    function isBrandValueOk()
    {
        if (textBrand.text == "--")
        {
            return false;
        }
        else if (ContrastsUtil.getErrorFromGroupDataBrand(textBrand.text) !== "")
        {
            return false;
        }
        else if (ContrastsUtil.checkContrastGroupExist(textBrand.text, parseInt(textConcentration.text), selectedFamilyIdx, selectedGroupIdx))
        {
            return false;
        }

        return true;
    }

    function isConcentrationValueOk()
    {
        if (textConcentration.text == "--")
        {
            return false;
        }
        else if (ContrastsUtil.getErrorFromGroupDataConcentration(textConcentration.text) !== "")
        {
            return false;
        }
        else if (ContrastsUtil.checkContrastGroupExist(textBrand.text, parseInt(textConcentration.text), selectedFamilyIdx, selectedGroupIdx))
        {
            return false;
        }

        return true;
    }

    function startEdit(newEditType)
    {
        if (widgetKeyboard.isOpen())
        {
            widgetKeyboard.close(true);
        }

        if (widgetInputPad.isOpen())
        {
            widgetInputPad.close(true);
        }

        editType = newEditType;

        if (editType === "BRAND")
        {
            widgetKeyboard.signalClosed.disconnect(slotKeyboardClosed);
            widgetInputPad.setBackgroundSlide();
            widgetKeyboard.open(btnBrand, textBrand, 0, fluidOptionContrastBrandLenMax);
            widgetKeyboard.signalClosed.connect(slotKeyboardClosed);
        }
        else if (editType === "CONCENTRATION")
        {
            widgetInputPad.signalClosed.disconnect(slotInputPadClosed);
            widgetInputPad.setBackgroundSlide();
            widgetInputPad.openIntegerPad(btnConcentration, textConcentration, concentrationUnit.text, fluidOptionContrastConcentrationMin, fluidOptionContrastConcentrationMax);
            widgetInputPad.signalClosed.connect(slotInputPadClosed);
        }
    }

    function slotKeyboardClosed(modified)
    {
        widgetKeyboard.signalClosed.disconnect(slotKeyboardClosed);
        slotEditClosed(modified, widgetKeyboard.currentValue);
    }

    function slotInputPadClosed(modified)
    {
        widgetInputPad.signalClosed.disconnect(slotInputPadClosed);
        slotEditClosed(modified, widgetInputPad.currentValue);
    }

    function slotEditClosed(modified, newVal)
    {
        textBrand.color = Qt.binding(function() { return isBrandValueOk() ? colorMap.text01 : colorMap.errText; });
        textConcentration.color = Qt.binding(function() { return isConcentrationValueOk() ? colorMap.text01 : colorMap.errText; });

        if (modified)
        {
            var saveRequired = false;

            if (editType === "BRAND")
            {
                newVal = newVal.toString();
                if (newVal === "--")
                {
                    // bad value
                }
                else if (groupData.Brand !== newVal)
                {
                    groupData.Brand = newVal;
                    saveRequired = true;
                }

                if (newVal.length === 0)
                {
                    textBrand.text = "--";
                }
            }
            else if (editType === "CONCENTRATION")
            {
                newVal = newVal.toString();
                if (newVal === "--")
                {
                    // bad value
                }
                else
                {
                    newVal = parseInt(newVal);
                    if (groupData.Concentration !== newVal)
                    {
                        groupData.Concentration = newVal;
                        saveRequired = true;
                    }
                }
            }

            if (saveRequired)
            {
                if (groupData.Brand === "<NEW>")
                {
                    // The group is not <NEW> anymore, create new group.
                    groupData.Brand = "";
                }

                contrastsMain.save();
            }
        }
    }

}
