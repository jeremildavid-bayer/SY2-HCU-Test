import QtQuick 2.12
import "../../../Widgets"
import "../../../Util.js" as Util

Column {
    property int rowHeight: parent.height * 0.15
    property var mandatoryFields: dsCfgGlobal.mandatoryFields
    property string lotBatchText: ""
    property string expirationText: ""
    property bool mandatoryFieldsFilled: rowLotBatch.isReady && rowExpiration.isReady

    onExpirationTextChanged:  {
        checkExpiration();
    }

    DeviceManagerPanelBottleParamListItem {
        id: rowLotBatch
        labelText: translate("T_LotBatch")
        valueText: lotBatchText
        interactive: editSourcePackageEnabled
        isMandatoryField: {
            if (mandatoryFields === undefined)
            {
                return false;
            }

            if (syringeIndex == 0)
            {
                return (mandatoryFields.SalineLotBatch !== undefined) &&
                       (mandatoryFields.SalineLotBatch);
            }
            else
            {
                return (mandatoryFields.ContrastLotBatch !== undefined) &&
                       (mandatoryFields.ContrastLotBatch);
            }
        }

        onEditStarted: {
            widgetKeyboard.signalClosed.connect(slotKeyboardClosed);
            widgetKeyboard.open(valueTextContainerObj, valueTextObj, fluidBottleLotBatchTextLenMin, fluidBottleLotBatchTextLenMax);
        }

        function slotKeyboardClosed(modified)
        {
            widgetKeyboard.signalClosed.disconnect(slotKeyboardClosed);

            if (modified)
            {
                var valueTextBuf = widgetKeyboard.currentValue;

                if (valueTextBuf === "")
                {
                    valueTextBuf = "--";
                }
                lotBatchText = valueTextBuf;

                if (checkBottleChanged())
                {
                    loadBottle();
                }
            }
            setDataBindings();

        }
    }

    Rectangle {
        // Separator
        width: parent.width
        height: 2
        color: colorMap.text02
    }

    DeviceManagerPanelBottleParamListItem {
        id: rowExpiration
        labelText: translate("T_Expiration")
        valueText: expirationText
        interactive: editSourcePackageEnabled
        isMandatoryField: {
            if (mandatoryFields === undefined)
            {
                return false;
            }

            if (syringeIndex == 0)
            {
                return (mandatoryFields.SalineExpiration !== undefined) &&
                       (mandatoryFields.SalineExpiration);
            }
            else
            {
                return (mandatoryFields.ContrastExpiration !== undefined) &&
                       (mandatoryFields.ContrastExpiration);
            }
        }

        onEditStarted: {
            widgetInputPad.signalClosed.connect(slotInputPadClosed);
            widgetInputPad.signalValueChanged.connect(slotInputPadValChanged);
            widgetInputPad.setBackgroundSlide();
            widgetInputPad.openDateTimePad(valueTextContainerObj, valueTextObj, "yyyy/MM");
        }

        function slotInputPadValChanged(newValue)
        {
            if (newValue === "yyyy/MM")
            {
                // yyyy/MM means we want to delete expiry date
                widgetInputPad.valueOk = true;
                valueTextObj.color = colorMap.actionButtonText;
            }
        }

        function slotInputPadClosed(modified)
        {
            widgetInputPad.signalClosed.disconnect(slotInputPadClosed);
            widgetInputPad.signalValueChanged.disconnect(slotInputPadValChanged);

            if (modified)
            {
                var valueTextBuf = widgetInputPad.currentValue;

                if (valueTextBuf === "yyyy/MM")
                {
                    valueTextBuf = "--";
                }
                expirationText = valueTextBuf;

                if (checkBottleChanged())
                {
                    loadBottle();
                }
            }
            else
            {
                // check expiration even if same date was entered (not modified)
                checkExpiration();
            }

            setDataBindings();
        }
    }

    function updateLotBatchAndExpiration()
    {
        var newLotBatchText = "--";
        var newExpiration = "--";

        if ( (fluidSourceBottle !== undefined) &&
             (fluidSourceBottle.SourcePackages !== undefined) &&
             (fluidSourceBottle.SourcePackages.length > 0) )
        {
            var sourcePackage = fluidSourceBottle.SourcePackages[0];
            if ( (sourcePackage.LotBatch !== undefined) &&
                 (sourcePackage.LotBatch !== "") &&
                 (sourcePackage.LotBatch.length >= fluidBottleLotBatchTextLenMin) )
            {
                newLotBatchText = sourcePackage.LotBatch;
            }

            if ( (sourcePackage.ExpirationDate !== undefined) &&
                 (sourcePackage.ExpirationDate !== "") )
            {
                newExpiration = Util.utcDateTimeToExpiryFormat(sourcePackage.ExpirationDate);
            }
        }

        lotBatchText = newLotBatchText;
        expirationText = newExpiration;
    }

    function changeExpirationText(newExp)
    {
        // Call this function to ensure Expiration date check is performed also on same entry
        expirationText = ""; // set to dummy first
        expirationText = newExp;
    }

    function checkExpiration()
    {
        if (!visible)
        {
            // don't check if the page is not visible (eg, on reboots)
            return;
        }

        if ((expirationText !== "") && (expirationText !== "--"))
        {
            var expMillisec = Util.utcDateTimeToMillisec(Util.localeDateFormattoUTCDateTime("yyyy/MM",expirationText));

            // use now in same format as expiration text to compare correctly
            var nowDateTimeStr = Util.getCurrentDateTimeStr("yyyy/MM","",0,Date.now());
            var nowMillisec = Util.utcDateTimeToMillisec(Util.localeDateFormattoUTCDateTime("yyyy/MM", nowDateTimeStr));
            if (expMillisec < nowMillisec)
            {
                // NOTE: "BS0", "BC1" and "BC2" are not defined on qml side...
                var location = "";
                if (syringeIndex === 0) {
                    location = "BS0";
                } else if (syringeIndex === 1) {
                    location = "BC1";
                } else if (syringeIndex === 2) {
                    location = "BC2";
                }

                var supplyStr = "";
                if (syringeIndex === 0) {
                    supplyStr = "Saline";
                } else {
                    var brand = fluidSelectItems[bottleSelectedIndex].Brand;
                    var concentration = fluidSelectItems[bottleSelectedIndex].Concentration;
                    var volume = fluidSelectItems[bottleSelectedIndex].Volumes[volumeSelectedIndex];
                    var volumeText = (volume !== 0) ? volume + translate("T_Units_ml") : "";
                    supplyStr = brand + " " + concentration + ((volumeText !== "") ? (" " + volumeText) : "");
                }

                var alertData = {};
                alertData["Location"] = location;
                alertData["Expiration"] = expirationText;
                alertData["Supply"] = supplyStr;

                dsAlert.slotActivateAlert("FluidExpirationDatePastDue", JSON.stringify(alertData));
            }
        }
    }
}
