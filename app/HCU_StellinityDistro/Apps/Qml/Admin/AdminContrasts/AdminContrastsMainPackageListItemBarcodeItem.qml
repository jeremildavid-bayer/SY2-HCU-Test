import QtQuick 2.12
import "../../Widgets"
import "../../Util.js" as Util
import "AdminContrasts.js" as ContrastsUtil

Item {
    property int fluidOptionContrastBarcodeLenMin: dsCapabilities.fluidOptionContrastBarcodeLenMin
    property int fluidOptionContrastBarcodeLenMax: dsCapabilities.fluidOptionContrastBarcodeLenMax
    property int barcodeIndex: index
    property var barcodeData: barcodesData[barcodeIndex]
    property string editType: "NONE" // "BARCODE"
    property int animationDeleteMs: 250
    property var barcodeInfo: dsDevice.barcodeInfo

    id: root
    width: ListView.view.width
    height: rowHeight
    clip: true

    SequentialAnimation {
        id: animationDeleted

        ScriptAction { script: {
                if (widgetKeyboard.isOpen())
                {
                    widgetKeyboard.close(true);
                }

                if (widgetInputPad.isOpen())
                {
                    widgetInputPad.close(true);
                }
            }
        }

        NumberAnimation { target: root; properties: 'opacity'; to: 0; duration: animationDeleteMs; }
        ScriptAction { script: {
                contrastsPackageListItem.slotDeleteBarcode(barcodeIndex);
                opacity = 1;
            }
        }
    }

    Item {
        id: barcodeEditField
        anchors.left: parent.left
        anchors.right: btnDelete.left
        anchors.rightMargin: parent.width * 0.01
        height: parent.height

        GenericButton {
            id: btnEdit
            width: parent.width
            height: parent.height
            color: colorMap.editFieldBackground
            visible: barcodeData !== "<NEW>"

            content: [
                Text {
                    id: textBarcode
                    anchors.left: parent.left
                    anchors.margins: rectMain.width * 0.01
                    width: contentWidth
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    font.pixelSize: height * 0.62
                    font.family: fontRobotoBold.name
                    verticalAlignment: Text.AlignVCenter
                    color: isBarcodeValueOk() ? colorMap.text01 : colorMap.errText
                }
            ]
            onBtnClicked: {
                if (editType === "NONE")
                {
                    startEdit("BARCODE");
                }
            }

            Component.onCompleted: {
                listViewPackage.dragStarted.connect(reset);
            }

            Component.onDestruction: {
                listViewPackage.dragStarted.disconnect(reset);
            }
        }
    }

    GenericButton {
        id: btnDelete
        anchors.right: parent.right
        anchors.rightMargin: parent.width * 0.02
        anchors.verticalCenter: parent.verticalCenter
        height: parent.height
        width: parent.width * 0.15
        color: "transparent"
        visible: barcodeData !== "<NEW>"
        content: [
            Text {
                anchors.fill: parent
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                color: colorMap.text02
                font.pixelSize: height * 0.45
                font.family: fontIcon.name
                text: "\ue94d"
            }
        ]

        onBtnClicked: {
            animationDeleted.start();
        }

        Component.onCompleted: {
            listViewPackage.dragStarted.connect(reset);
        }

        Component.onDestruction: {
            listViewPackage.dragStarted.disconnect(reset);
        }
    }

    GenericButton {
        id: btnNew
        visible: (barcodeData === "<NEW>")
        interactive: isReadyToAdd()
        anchors.left: parent.left
        anchors.right: btnDelete.left
        anchors.rightMargin: parent.width * 0.01
        height: parent.height
        content: [
            Text {
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.margins: rectMain.width * 0.02
                width: contentWidth
                verticalAlignment: Text.AlignVCenter
                text: "\ue95b"
                font.pixelSize: height * 1.2
                font.family: fontIcon.name
                color: btnNew.interactive ? colorMap.text02 : colorMap.buttonDisabled
            }
        ]
        onBtnClicked: {
            contrastsPackageListItem.slotAddBarcode();
        }

        Component.onCompleted: {
            listViewPackage.dragStarted.connect(reset);
        }

        Component.onDestruction: {
            listViewPackage.dragStarted.disconnect(reset);
        }
    }

    Component.onCompleted: {
        reload();
    }

    function reload()
    {
        //logDebug("AdminContrastMainPackageListItemBarcode[" + barcodeIndex + "]: Reload(): barcodeData=" + JSON.stringify(barcodeData));

        if (barcodeData ===  undefined)
        {
            return;
        }

        if (barcodeData === "<NEW>")
        {
            // Nothing to process but display + button
            return;
        }

        if (barcodeData === "<NEW_EDIT>")
        {
            textBarcode.text = "--";
            barcodeData = "--";
            //logDebug("reload(): <NEW_EDIT>: packageData=" + JSON.stringify(packageData));
            packageData.BarcodePrefixes[barcodeIndex] = "";
            ContrastsUtil.setFluidPackage(selectedFamilyIdx, selectedGroupIdx, packageIndex, packageData);
            startEdit("BARCODE");
        }
        else
        {
            textBarcode.text = (barcodeData === "") ? "--" : barcodeData;
        }
    }

    function isBarcodeValueOk()
    {
        var barcodeDataValue = (textBarcode.text == "--") ? "" : textBarcode.text;
        if (ContrastsUtil.getErrorFromGroupDataBarcodePrefix(barcodeDataValue, selectedFamilyIdx, selectedGroupIdx, packageIndex, barcodeIndex) !== "")
        {
            return false;
        }
        return true;
    }

    function isReadyToAdd()
    {
         if (!contrastsPackageListItem.isVolumeValueOk())
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

        if (editType === "BARCODE")
        {
            widgetInputPad.signalValueChanged.disconnect(slotInputPadValChanged);
            widgetInputPad.signalClosed.disconnect(slotInputPadClosed);
            widgetInputPad.setBackgroundSlide();
            // Editing/scanning barcode. Disable other edit fields
            widgetInputPad.setModalObjects([btnEdit]);
            widgetInputPad.blurBackgroundVisible = true;
            widgetInputPad.openTextPad(btnEdit, textBarcode, fluidOptionContrastBarcodeLenMin, fluidOptionContrastBarcodeLenMax);
            widgetInputPad.signalValueChanged.connect(slotInputPadValChanged);
            widgetInputPad.signalClosed.connect(slotInputPadClosed);

            // Turn on and listen for barcode reader changes
            dsDevice.slotBarcodeReaderStart(0);
        }
    }

    function slotInputPadValChanged(newValue)
    {
        if (editType === "BARCODE")
        {
            if (newValue === "")
            {
                textBarcode.text = "--";
            }
            else if (newValue === "-")
            {
                widgetInputPad.setCurrentValue("");
                textBarcode.text = "";
            }
            else
            {
                textBarcode.text = newValue;
            }
            textBarcode.color = isBarcodeValueOk() ? colorMap.actionButtonText : colorMap.errText;
        }
    }

    function slotInputPadClosed(modified)
    {
        widgetInputPad.signalValueChanged.disconnect(slotInputPadValChanged);
        widgetInputPad.signalClosed.disconnect(slotInputPadClosed);
        textBarcode.color = Qt.binding(function() { return isBarcodeValueOk() ? colorMap.text01 : colorMap.errText; });

        // Needs to be here so that when input pad is closed manually it stops the reader
        dsDevice.slotBarcodeReaderStop();

        if (modified)
        {
            var saveRequired = false;
            var newVal;

            if (editType === "BARCODE")
            {
                newVal = widgetInputPad.currentValue;
                if (newVal === "--")
                {
                    newVal = "";
                }

                if (barcodeData !== newVal)
                {
                    barcodeData = newVal;
                    saveRequired = true;
                }

                if (saveRequired)
                {
                    var scannedTimePastMs = 10000;
                    if (barcodeInfo !== undefined)
                    {
                        scannedTimePastMs = new Date() - Util.utcDateTimeToMillisec(barcodeInfo.ScannedAt);
                    }

                    if (scannedTimePastMs > 500)
                    {
                        packageData.BarcodePrefixes[barcodeIndex] = "<WAITING_BARCODE>";
                        logDebug("Barcode Entered manually. Simulate barcode scan..");
                        timerSingleShot(10, function() {
                            dsDevice.slotBarcodeReaderSetBarcodeData(barcodeData);
                        });
                    }
                }
            }

            if (saveRequired)
            {
                if (header.groupData.Brand === "<NEW>")
                {
                    // The group is not <NEW> anymore, create new group.
                    header.groupData.Brand = "";
                }

                contrastsPackageList.setPackageData(packageIndex, packageData);
                contrastsMain.save();
            }
        }
        else
        {
            if (barcodesData[barcodeIndex] === "<NEW_EDIT>")
            {
                // Cancel from new edit, delete current row
                slotDeleteBarcode(barcodeIndex);
            }
        }
    }
}
