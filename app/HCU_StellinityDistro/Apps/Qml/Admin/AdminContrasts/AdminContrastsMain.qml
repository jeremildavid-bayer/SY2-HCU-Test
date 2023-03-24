import QtQuick 2.12
import "../../Widgets"
import "../../Util.js" as Util
import "AdminContrasts.js" as ContrastsUtil

Item {
    property int selectedFamilyIdx: contrastsPage.selectedFamilyIdx
    property int selectedGroupIdx: contrastsPage.selectedGroupIdx

    AdminContrastsMainHeader {
        id: header
        anchors.top: parent.top
        anchors.topMargin: parent.height * 0.04
        width: parent.width
        height: parent.height * 0.18
    }

    AdminContrastsMainPackageList {
        id: packageList
        width: parent.width
        anchors.top: header.bottom
        anchors.topMargin: parent.height * 0.05
        anchors.bottom: parent.bottom
        anchors.bottomMargin: parent.height * 0.05
    }

    onSelectedFamilyIdxChanged: {
        reload();
    }

    onSelectedGroupIdxChanged: {
        reload();
    }

    function reload()
    {
        header.reload();
        packageList.reload();
        var groupData = ContrastsUtil.getGroupData(selectedFamilyIdx, selectedGroupIdx);
        //logDebug("AdminContrastsMain: Reload: FamilyIdx=" + selectedFamilyIdx + ", GroupIdx=" + selectedGroupIdx + ", GroupData=" + JSON.stringify(groupData));
        contrastsActionBar.errorText = ContrastsUtil.getErrorFromGroupData(groupData, selectedFamilyIdx, selectedGroupIdx);
    }

    function getGroupDataFromUI()
    {
        var groupData = { Brand: header.groupData.Brand,
                          Concentration: header.groupData.Concentration,
                          FluidPackages: Util.copyObject(packageList.packagesData) };

        return groupData;
    }

    function save()
    {
        // Prepare data from current page
        var groupData = getGroupDataFromUI();
        //logDebug("AdminContrastsMain.qml: groupData=" + JSON.stringify(groupData));
        ContrastsUtil.setReloadReason("MODIFIED");
        ContrastsUtil.saveGroup(groupData, selectedFamilyIdx, selectedGroupIdx);
    }
}

