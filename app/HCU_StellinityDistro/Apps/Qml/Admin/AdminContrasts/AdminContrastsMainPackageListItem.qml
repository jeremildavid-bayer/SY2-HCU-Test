import QtQuick 2.12
import "../../Widgets"
import "../../Util.js" as Util
import "AdminContrasts.js" as ContrastsUtil

Item {
    property int fluidOptionContrastVolumeMin: dsCapabilities.fluidOptionContrastVolumeMin
    property int fluidOptionContrastVolumeMax: dsCapabilities.fluidOptionContrastVolumeMax
    property int fluidOptionContrastMaxUseLifeHourMin: dsCapabilities.fluidOptionContrastMaxUseLifeHourMin
    property int fluidOptionContrastMaxUseLifeHourMax: dsCapabilities.fluidOptionContrastMaxUseLifeHourMax

    property int packageIndex: index
    property var packageData: packagesData[packageIndex]
    property var barcodesData
    property int rowHeight: ListView.view.height * 0.16
    property int contentHorizontalMargin: ListView.view.width * 0.02
    property int contentVerticalMargin: ListView.view.height * 0.04
    property int barcodeListRowSpacing: ListView.view.height * 0.015
    property string editType: "NONE" // "VOLUME", "MAX_USE_LIFE"
    property int animationDeleteMs: 250

    id: contrastsPackageListItem
    width: ListView.view.width
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

        NumberAnimation { target: contrastsPackageListItem; properties: 'opacity'; to: 0; duration: animationDeleteMs; }
        ScriptAction { script: {
                contrastsPackageListItem.opacity = 1;
                contrastsPackageList.slotDeletePackage(packageIndex);
            }
        }
    }

    Rectangle {
        id: backgroundGrid
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.rightMargin: btnDelete.width
        height: parent.height
        color: "transparent"
        radius: buttonRadius
        border.color: colorMap.text02
        border.width: 2
    }

    GenericButton {
        id: btnAdd
        visible: false
        width: backgroundGrid.width
        height: backgroundGrid.height
        x: backgroundGrid.x
        y: backgroundGrid.y
        color: "transparent"
        content: [
            Text {
                anchors.fill: parent
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                color: colorMap.text02
                font.pixelSize: height * 0.45
                font.family: fontIcon.name
                text: "\ue952"
            }
        ]

        onBtnClicked: {
            contrastsPackageList.slotAddPackage();
        }

        Component.onCompleted: {
            listViewPackage.dragStarted.connect(reset);
        }

        Component.onDestruction: {
            listViewPackage.dragStarted.disconnect(reset);
        }
    }

    Item {
        id: rectMain
        width: parent.width - (contentHorizontalMargin * 2)
        height: parent.height - (contentVerticalMargin * 2)
        anchors.centerIn: parent

        GenericButton {
            id: btnVolume
            x: parent.mapFromItem(labelSize, 0, 0).x - contentHorizontalMargin
            anchors.top: parent.top
            width: labelSize.width
            height: rowHeight
            color: colorMap.editFieldBackground
            content: [
                Text {
                    id: textVolume
                    anchors.left: parent.left
                    anchors.leftMargin: rectMain.width * 0.02
                    anchors.margins: rectMain.width * 0.01
                    width: contentWidth
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    font.pixelSize: height * 0.72
                    font.family: fontRobotoBold.name
                    verticalAlignment: Text.AlignVCenter
                    color: isVolumeValueOk() ? colorMap.text01 : colorMap.errText
                },
                Text {
                    id: textVolumeUnit
                    anchors.left: textVolume.right
                    anchors.right: parent.right
                    anchors.leftMargin: rectMain.width * 0.006
                    anchors.rightMargin: rectMain.width * 0.01
                    anchors.top: parent.top
                    anchors.topMargin: rectMain.width * 0.01
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: rectMain.width * 0.01
                    font.pixelSize: height * 0.57
                    font.family: fontRobotoLight.name
                    verticalAlignment: Text.AlignVCenter
                    color: colorMap.text02
                    text: translate("T_Units_ml")
                    elide: Text.ElideRight
                }
            ]
            onBtnClicked: {
                startEdit("VOLUME");
            }

            Component.onCompleted: {
                listViewPackage.dragStarted.connect(reset);
            }

            Component.onDestruction: {
                listViewPackage.dragStarted.disconnect(reset);
            }
        }

        GenericButton {
            id: btnMaxUseLife
            x: parent.mapFromItem(labelMaxUseLife, 0, 0).x - contentHorizontalMargin
            anchors.top: parent.top
            width: labelMaxUseLife.width
            height: rowHeight
            color: colorMap.editFieldBackground
            enabled: isVolumeValueOk()

            content: [
                Text {
                    id: textMaxUseLife
                    anchors.left: parent.left
                    anchors.margins: rectMain.width * 0.01
                    width: contentWidth
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    font.pixelSize: height * 0.72
                    font.family: fontRobotoBold.name
                    verticalAlignment: Text.AlignVCenter
                    color: isMaxUseLifeValueOk() ? colorMap.text01 : colorMap.errText
                },

                Text {
                    id: textMaxUseLifeUnit
                    anchors.left: textMaxUseLife.right
                    anchors.right: parent.right
                    anchors.leftMargin: rectMain.width * 0.006
                    anchors.rightMargin: rectMain.width * 0.01
                    anchors.top: parent.top
                    anchors.topMargin: rectMain.width * 0.01
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: rectMain.width * 0.01
                    font.pixelSize: height * 0.57
                    font.family: fontRobotoLight.name
                    verticalAlignment: Text.AlignVCenter
                    color: colorMap.text02
                    text: translate("T_Units_h")
                    elide: Text.ElideRight
                }
            ]
            onBtnClicked: {
                startEdit("MAX_USE_LIFE");
            }

            Component.onCompleted: {
                listViewPackage.dragStarted.connect(reset);
            }

            Component.onDestruction: {
                listViewPackage.dragStarted.disconnect(reset);
            }
        }

        ListView {
            id: listViewBarcode
            x: parent.mapFromItem(labelBarcode, 0, 0).x - contentHorizontalMargin
            width: labelBarcode.width - btnDelete.width
            height: parent.height
            interactive: false
            spacing: barcodeListRowSpacing
            model: barcodesData
            delegate: AdminContrastsMainPackageListItemBarcodeItem {}
            onModelChanged: {
                //logDebug("AdminContrastsMainPackageListItem[" + packageIndex + "]: Barcode Model changed: Model=" + JSON.stringify(model));
                contrastsPackageListItem.height = getHeightTotal();
            }
        }

        GenericButton {
            id: btnDelete
            y: -contentVerticalMargin
            anchors.right: parent.right
            anchors.rightMargin: -contentHorizontalMargin
            height: rowHeight
            width: parent.width * 0.08
            color: "transparent"
            visible: packagesData.length > 1
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
    }

    Component.onCompleted: {
        //logDebug("AdminContrastMainPackageListItem[" + packageIndex + "]: Created");
        contrastsPackageList.signalReloadPackages.connect(reload);
        contrastsPackageList.signalCheckData.connect(checkData);

        reload();
    }

    Component.onDestruction: {
        //logDebug("AdminContrastMainPackageListItem[" + packageIndex + "]: Destroyed");
        contrastsPackageList.signalReloadPackages.disconnect(reload);
        contrastsPackageList.signalCheckData.disconnect(checkData);
    }

    function checkData()
    {
        // Force to call config check function by change the text value
        var prevVal = textVolume.text;
        textVolume.text = "";
        textVolume.text = prevVal;

        prevVal = textMaxUseLife.text;
        textMaxUseLife.text = "";
        textMaxUseLife.text = prevVal;
    }

    function reload()
    {
        //logDebug("AdminContrastMainPackageListItem[" + packageIndex + "]: Reload(): packageData=" + JSON.stringify(packageData));
        if (packageData === undefined)
        {
            return;
        }

        if (!Util.compareObjects(barcodesData, packageData.BarcodePrefixes))
        {
            //logDebug("AdminContrastMainPackageListItem[" + packageIndex + "]: barcodesData = " + JSON.stringify(barcodesData) + " -> " + JSON.stringify(packageData.BarcodePrefixes));
            barcodesData = Util.copyObject(packageData.BarcodePrefixes);
        }

        contrastsPackageListItem.height = getHeightTotal();

        if (packageData.Brand === "<NEW>")
        {
            btnAdd.visible = true;
            rectMain.visible = false;
            return;
        }

        btnAdd.visible = false;
        rectMain.visible = true;

        // Set volume
        if (packageData.Brand === "<NEW_EDIT>")
        {
            textVolume.text = "--";
        }

        if (packageData.Volume === 0)
        {
            textVolume.text = "--";
        }
        else
        {
            textVolume.text = packageData.Volume;
        }

        // Set Max Use Life
        if ( (packageData.MaximumUseDuration === undefined) ||
             (packageData.MaximumUseDuration === null) )
        {
            textMaxUseLife.text = "--";
        }
        else
        {
            var maxUseDurationHours = Util.durationStrToMillisec(packageData.MaximumUseDuration) / (1000 * 60 * 60);
            textMaxUseLife.text = maxUseDurationHours.toString();
        }

        if (packageData.Brand === "<NEW_EDIT>")
        {
            // For new added packaged, set Brand to Family Brand and start editing volume
            var brand = contrastsPage.contrastFamilies[selectedFamilyIdx].Groups[selectedGroupIdx].Brand;
            packageData.Brand = brand;
            ContrastsUtil.setFluidPackage(selectedFamilyIdx, selectedGroupIdx, packageIndex, packageData);
            listViewPackage.currentIndex = packageIndex;
            startEdit("VOLUME");
        }
    }

    function getHeightTotal()
    {
        if (packageData === undefined)
        {
            return rowHeight;
        }
        else if (packageData.Brand === "<NEW>")
        {
            return rowHeight;
        }
        else
        {
            var rowCount = Math.max(1, barcodesData.length);
            var newHeight = (rowHeight * rowCount) + (contentVerticalMargin * 2) + (barcodeListRowSpacing * (rowCount - 1));
            return newHeight;
        }
    }

    function isVolumeValueOk()
    {
        var volumeDataValue = parseInt(textVolume.text);
        if (isNaN(volumeDataValue))
        {
            volumeDataValue = 0;
        }

        if (ContrastsUtil.getErrorFromGroupDataPackageVolume(volumeDataValue, packageIndex, packagesData) !== "")
        {
            return false;
        }
        return true;
    }

    function isMaxUseLifeValueOk()
    {
        var maxUseLifeDataValue = parseInt(textMaxUseLife.text);
        if (isNaN(maxUseLifeDataValue))
        {
            maxUseLifeDataValue = 0;
        }

        if (ContrastsUtil.getErrorFromGroupDataPackageMaximumUseDuration(maxUseLifeDataValue) !== "")
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

        if (editType === "VOLUME")
        {
            widgetInputPad.signalValueChanged.disconnect(slotInputPadValChanged);
            widgetInputPad.signalClosed.disconnect(slotInputPadClosed);
            widgetInputPad.setBackgroundSlide();
            widgetInputPad.openIntegerPad(btnVolume, textVolume, textVolumeUnit.text, fluidOptionContrastVolumeMin, fluidOptionContrastVolumeMax);
            widgetInputPad.signalValueChanged.connect(slotInputPadValChanged);
            widgetInputPad.signalClosed.connect(slotInputPadClosed);
        }
        else if (editType === "MAX_USE_LIFE")
        {
            widgetInputPad.signalValueChanged.disconnect(slotInputPadValChanged);
            widgetInputPad.signalClosed.disconnect(slotInputPadClosed);
            widgetInputPad.setBackgroundSlide();
            widgetInputPad.openIntegerPad(btnMaxUseLife, textMaxUseLife, textMaxUseLifeUnit.text, fluidOptionContrastMaxUseLifeHourMin, fluidOptionContrastMaxUseLifeHourMax);
            widgetInputPad.signalValueChanged.connect(slotInputPadValChanged);
            widgetInputPad.signalClosed.connect(slotInputPadClosed);
        }
    }

    function slotInputPadValChanged(newValue)
    {
        if (editType === "VOLUME")
        {
            if (parseInt(newValue) === 0)
            {
                textVolume.text = "--";
            }
            else
            {
                textVolume.text = newValue;
            }
            textVolume.color = isVolumeValueOk() ? colorMap.actionButtonText : colorMap.errText;
        }
        else if (editType === "MAX_USE_LIFE")
        {
            if (parseInt(newValue) === 0)
            {
                textMaxUseLife.text = "--";
            }
            else
            {
                textMaxUseLife.text = newValue;
            }
            // Force to set normal color for '--' (which is not supported by keypad)
            textMaxUseLife.color = isMaxUseLifeValueOk() ? colorMap.actionButtonText : colorMap.errText;
        }
    }

    function slotInputPadClosed(modified)
    {
        widgetInputPad.signalValueChanged.disconnect(slotInputPadValChanged);
        widgetInputPad.signalClosed.disconnect(slotInputPadClosed);

        textVolume.color = Qt.binding(function() { return isVolumeValueOk() ? colorMap.text01 : colorMap.errText; });
        textMaxUseLife.color = Qt.binding(function() { return isMaxUseLifeValueOk() ? colorMap.text01 : colorMap.errText; });

        if (modified)
        {
            var saveRequired = false;
            var newVal;

            if (editType === "VOLUME")
            {
                newVal = parseInt(widgetInputPad.currentValue);
                if (isNaN(newVal))
                {
                    newVal = 0;
                }

                if ( (widgetInputPad.endValue === "--") ||
                     (parseInt(widgetInputPad.endValue) === 0) )
                {
                    textVolume.text = "--";
                    newVal = 0;
                }

                if (packageData.Volume !== newVal)
                {
                    packageData.Volume = newVal;
                    saveRequired = true;
                }
            }
            else if (editType === "MAX_USE_LIFE")
            {
                newVal = parseInt(widgetInputPad.currentValue);
                if (isNaN(newVal))
                {
                    newVal = undefined;
                }

                if ( (widgetInputPad.endValue === "--") ||
                     (parseInt(widgetInputPad.endValue) === 0) )
                {
                    textMaxUseLife.text = "--";
                    newVal = undefined;
                }

                if (newVal !== undefined)
                {
                    var durationMs = newVal * 60 * 60 * 1000;
                    newVal = Util.millisecToDurationStr(durationMs);
                }

                if (packageData.MaximumUseDuration !== newVal)
                {
                    packageData.MaximumUseDuration = newVal;
                    saveRequired = true;
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

                // Volume data is changed. Check data for each package for duplicted volume
                contrastsPackageList.signalCheckData();

                contrastsMain.save();
            }
        }
    }

    function slotAddBarcode()
    {
        if (widgetKeyboard.isOpen())
        {
            widgetKeyboard.close(true);
        }

        if (widgetInputPad.isOpen())
        {
            widgetInputPad.close(true);
        }

        if (header.groupData.Brand === "<NEW>")
        {
            // The group is not <NEW> anymore, create new group.
            header.groupData.Brand = "";
        }

        logDebug("AdminContrastsMainPackageListItem: slotAddBarcode()");

        // Use barcodesDataBuf to set new barcodesData
        var barcodesDataBuf = Util.copyObject(packageData.BarcodePrefixes);

        for (var barcodeIdx = 0; barcodeIdx < barcodesDataBuf.length; barcodeIdx++)
        {
            if (barcodesDataBuf[barcodeIdx] === "<NEW_EDIT>")
            {
                barcodesDataBuf[barcodeIdx] = "";
            }
        }

        barcodesDataBuf[barcodesDataBuf.length - 1] = "<NEW_EDIT>";
        barcodesDataBuf.push("<NEW>");

        // Setting barcodesData will reload barcode rows
        barcodesData = Util.copyObject(barcodesDataBuf);
        packageData.BarcodePrefixes = barcodesData;
        //logDebug("slotAddBarcode(): packageData=" + JSON.stringify(packageData));
        contrastsPackageList.setPackageData(packageIndex, packageData);
        contrastsMain.save();
    }

    function slotDeleteBarcode(barcodeIndex)
    {
        if (widgetKeyboard.isOpen())
        {
            widgetKeyboard.close(true);
        }

        if (widgetInputPad.isOpen())
        {
            widgetInputPad.close(true);
        }

        //logDebug("AdminContrastsMainPackageListItem: slotDeleteBarcode()");

        // Use barcodesDataBuf to set new barcodesData
        var barcodesDataBuf = Util.copyObject(packageData.BarcodePrefixes);
        barcodesDataBuf.splice(barcodeIndex, 1);
        barcodesData = Util.copyObject(barcodesDataBuf);
        packageData.BarcodePrefixes = barcodesData;
        contrastsPackageList.setPackageData(packageIndex, packageData);
        contrastsMain.save();
    }
}
