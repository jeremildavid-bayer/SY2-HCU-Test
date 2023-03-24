import QtQuick 2.12
import "../../Widgets"
import "../../Util.js" as Util
import "AdminContrasts.js" as ContrastsUtil

Item {
    property int selectedFamilyIdx: contrastsPage.selectedFamilyIdx
    property int selectedGroupIdx: contrastsPage.selectedGroupIdx
    property var packagesData

    signal signalReloadPackages()
    signal signalCheckData()

    id: contrastsPackageList
    clip: true

    Item {
        id: labels
        clip: true
        width: parent.width
        height: parent.height * 0.1

        Text {
            id: labelSize
            anchors.left: parent.left
            anchors.leftMargin: parent.width * 0.02
            width: parent.width * 0.17
            height: parent.height
            verticalAlignment: Text.AlignVCenter
            text: translate("T_PackageSize")
            font.pixelSize: height * 0.4
            font.family: fontRobotoLight.name
            color: colorMap.text01
            wrapMode: Text.Wrap
        }

        Text {
            id: labelMaxUseLife
            anchors.left: labelSize.right
            anchors.leftMargin: parent.width * 0.04
            width: parent.width * 0.17
            height: parent.height
            verticalAlignment: Text.AlignVCenter
            text: translate("T_MaxUseLife")
            font.pixelSize: height * 0.4
            font.family: fontRobotoLight.name
            color: colorMap.text01
            wrapMode: Text.Wrap
            elide: Text.ElideRight
        }

        Text {
            id: labelBarcode
            anchors.left: labelMaxUseLife.right
            anchors.leftMargin: parent.width * 0.04
            anchors.right: parent.right
            height: parent.height
            verticalAlignment: Text.AlignVCenter
            text: translate("T_Barcode")
            font.pixelSize: height * 0.4
            font.family: fontRobotoLight.name
            color: colorMap.text01
            wrapMode: Text.Wrap
        }
    }

    ListView {
        property double lastContentY: 0

        id: listViewPackage
        anchors.top: labels.bottom
        anchors.bottom: parent.bottom
        width: parent.width
        clip: true
        cacheBuffer: Math.max(contentHeight * 2, height * 10)
        highlightMoveDuration: 0
        highlightRangeMode: ListView.ApplyRange
        spacing: height * 0.05
        delegate: AdminContrastsMainPackageListItem {}

        footer: Item {
            width: ListView.view ? ListView.view.width : 0
            height: widgetKeyboard.isOpen() ? (widgetKeyboard.keyboardHeight - actionBarHeight) : 0
        }

        model: packagesData

        ScrollBar {}
        ListFade {}

        onContentYChanged: {
            if (contentY != -0)
            {
                lastContentY = contentY;
            }
        }

        onModelChanged: {
            //logDebug("AdminContrastsMainPackageList: Model changed: Model=" + JSON.stringify(model));
            contentY = lastContentY;
        }
    }

    onSelectedFamilyIdxChanged: {
        listViewPackage.lastContentY = 0;
    }

    onSelectedGroupIdxChanged: {
        listViewPackage.lastContentY = 0;
    }

    function reload()
    {
        if ( (contrastsPage.contrastFamilies[selectedFamilyIdx] === undefined) ||
             (contrastsPage.contrastFamilies[selectedFamilyIdx].Groups[selectedGroupIdx] === undefined) )
        {
            packagesData = undefined;
            return;
        }

        if (!Util.compareObjects(packagesData, contrastsPage.contrastFamilies[selectedFamilyIdx].Groups[selectedGroupIdx].FluidPackages))
        {
            packagesData = Util.copyObject(contrastsPage.contrastFamilies[selectedFamilyIdx].Groups[selectedGroupIdx].FluidPackages);
            signalReloadPackages();
        }
    }

    function setPackageData(packageIndex, packageData)
    {
        packagesData[packageIndex] = Util.copyObject(packageData);
    }

    function slotAddPackage()
    {
        if (widgetKeyboard.isOpen())
        {
            widgetKeyboard.close(true);
        }

        if (widgetInputPad.isOpen())
        {
            widgetInputPad.close(true);
        }

        //logDebug("AdminContrastsMainPackageList: slotAddPackage()");

        /*var groupData = ContrastsUtil.getGroupData(selectedFamilyIdx, selectedGroupIdx);
        if (groupData.Brand === "<NEW>")
        {
            // The group is not <NEW> anymore.
            var newGroupData = Util.copyObject(groupData);
            newGroupData.Brand = "";
            header.groupData = newGroupData;
        }*/
        if (header.groupData.Brand === "<NEW>")
        {
            // The group is not <NEW> anymore, create new group.
            header.groupData.Brand = "";
        }

        // Convert <NEW> to <NEW_EDIT>: Can start edit
        packagesData[packagesData.length - 1].Brand = "<NEW_EDIT>";

        // Add <NEW> at the end
        packagesData.push({ BarcodePrefixes: [], Brand: "<NEW>", Volume: 0 });
        contrastsMain.save();

        listViewPackage.currentIndex = packagesData.length - 1;
    }

    function slotDeletePackage(packageIndex)
    {
        if (widgetKeyboard.isOpen())
        {
            widgetKeyboard.close(true);
        }

        if (widgetInputPad.isOpen())
        {
            widgetInputPad.close(true);
        }

        //logDebug("AdminContrastsMainPackageList: slotDeletePackage()");

        // Use packagesDataBuf to set new packagesData
        // NOTE: packagesData.splice() didn't trigger the packagesData changed to set the new model for listViewPackage
        var packagesDataBuf = Util.copyObject(packagesData);
        packagesDataBuf.splice(packageIndex, 1);
        packagesData = Util.copyObject(packagesDataBuf);
        signalReloadPackages();
        contrastsMain.save();
    }
}
