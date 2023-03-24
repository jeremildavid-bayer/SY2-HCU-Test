import QtQuick 2.12
import "../../../Widgets"
import "../../../Widgets/Popup"

Rectangle {
    property string hcuBuildType: dsSystem.hcuBuildType
    property var upgradeDigest: dsUpgrade.upgradeDigest
    property string upgradeStatusStr
    property int rebootCountSec: 0
    property int infoTableRowWidth: root.width * 0.75
    property int infoTableRowHeight: root.height * 0.11

    id: root
    color: "transparent"
    anchors.fill: parent

    // FILE SELECT --------------------------------------
    FileExplorer {
        id: fileDialog
        translationRequired: false
        rootPath: "file:///media/user"
        extFilters: ["*.hex", "*.mot", "*.gz", "*.pkg"]
        titleText: "Please select an upgrade file"
        cancelBtnText: "Cancel"
        okBtnText: "OK"

        onBtnOkClicked: {
            fileDialog.close();
            filePath.text = fileDialog.selectedFilePath.replace(rootPath, "USB:/");
            dsUpgrade.slotUpdateSelectedPackageInfo(fileDialog.selectedFilePath);
            close();
        }

        onBtnCancelClicked: {
            close();
        }
    }

    Row {
        y: parent.height * 0.05
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: root.width * 0.02

        Rectangle {
            height: root.height * 0.12
            width: root.width * 0.6
            color: colorMap.subPanelBackground
            radius: root * 0.01

            Text {
                id: filePath
                anchors.fill: parent
                anchors.leftMargin: parent.width * 0.01
                anchors.rightMargin: parent.width * 0.01
                text: ""
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                font.pixelSize: height * 0.25
                elide: Text.ElideRight
                color: colorMap.text01
            }
        }

        GenericIconButton {
            color: colorMap.actionButtonBackground
            height: root.height * 0.12
            width: root.width * 0.15
            enabled: upgradeDigest.State === "Ready"
            iconText: "Open File"
            iconColor: colorMap.actionButtonText

            onBtnClicked: {
                fileDialog.open();
            }
        }
    }

    // STATUS INFO TABLE --------------------------------------
    Column {
        anchors.top: parent.top
        anchors.topMargin: parent.height * 0.24
        anchors.horizontalCenter: parent.horizontalCenter

        Repeater {
            model: ListModel {
                ListElement { title: "File Size (KB)" }
                ListElement { title: "Progress" }
                ListElement { title: "Status" }
            }

            delegate: Row {
                Rectangle {
                    color: "transparent"
                    anchors.verticalCenter: parent.verticalCenter
                    width: infoTableRowWidth * 0.2
                    height: infoTableRowHeight
                    border.color: colorMap.text02
                    border.width: 1

                    Text {
                        anchors.fill: parent
                        anchors.leftMargin: infoTableRowWidth * 0.017
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                        color: colorMap.text02
                        font.pixelSize: parent.height * 0.35
                        text: title
                    }
                }

                Rectangle {
                    color: "transparent"
                    anchors.verticalCenter: parent.verticalCenter
                    width: infoTableRowWidth * 0.8
                    height: infoTableRowHeight
                    border.color: colorMap.text02
                    border.width: 1

                    Text {
                        anchors.fill: parent
                        anchors.leftMargin: infoTableRowWidth * 0.017
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                        color: colorMap.text01
                        font.pixelSize: parent.height * 0.38
                        text: getInfoTableRowText(title)
                        wrapMode: Text.Wrap
                        elide: Text.ElideRight
                    }
                }
            }
        }

        Rectangle {
            // spacer
            color: "transparent"
            height: parent.height * 0.12
            width: parent.width
        }

        // START BUTTON --------------------------------------
        Item {
            anchors.horizontalCenter: parent.horizontalCenter
            height: root.height * 0.12
            width: root.width * 0.5

            GenericIconButton {
                id: btnStartUpgrade
                anchors.fill: parent
                enabled: isReadyToStart()
                color: colorMap.actionButtonBackground
                iconText: "Start Upgrade"
                iconColor: colorMap.actionButtonText

                onBtnClicked: {
                    dsUpgrade.slotUpgrade();
                }
            }

            LoadingGif {
                visible: (upgradeDigest.State !== "Ready") && (upgradeDigest.State !== "Upgrade SRU Package - Done") && (upgradeDigest.State !== "Upgrade SRU Package - Failed")
            }

        }
    }

    PopupMessage {
        id: popupUpgradeComplete
        translationRequired: false
        type: "WARNING"
        titleText: "Upgrade Completed"
        visible: false
        showCancelBtn: false
        okBtnText: "Reboot"
        onBtnOkClicked: {
            if (okBtnText == "Reboot")
            {
                logInfo("Reboot after upgrade completed..");
                reboot();
            }
            else
            {
                close();
            }
        }
    }

    Timer {
        id: shutdownCountTimer
        interval: 1000
        repeat: true
        onTriggered: {
            if (rebootCountSec > 0)
            {
                rebootCountSec--;
            }
            else
            {
                logInfo("Reboot after upgrade completed..");
                repeat = false;
                reboot();
            }
        }
    }

    onUpgradeDigestChanged: {
        if ( (upgradeDigest === undefined) ||
             (upgradeDigest.Sru === undefined) )
        {
            return;
        }

        if ( (upgradeDigest.State === "Ready") &&
             (upgradeDigest.Sru.Err !== "") )
        {
            // Upgrade is failed, don't update text
            return;
        }

        if (upgradeDigest.State === "Upgrade SRU Package - Done")
        {
            rebootCountSec = 10;

            if (hcuBuildType === "DEV")
            {
                popupUpgradeComplete.contentText = "Please Restart Application."
            }
            else
            {
                shutdownCountTimer.start();
                popupUpgradeComplete.contentText = Qt.binding(function() { return "Rebooting in " + rebootCountSec + " seconds.."; });
            }

            popupUpgradeComplete.titleText = "Upgrade Completed";
            popupUpgradeComplete.okBtnText = "Reboot";
            popupUpgradeComplete.open();
            return;
        }
        else if (upgradeDigest.State === "Upgrade SRU Package - Failed")
        {
            popupUpgradeComplete.titleText = "Upgrade Failed";
            popupUpgradeComplete.contentText = Qt.binding(function() { return upgradeDigest.Sru.Err; });
            popupUpgradeComplete.okBtnText = "OK";
            popupUpgradeComplete.open();
            return;
        }

        upgradeStatusStr = upgradeDigest.State;

        if (upgradeDigest.Sru.Err !== "")
        {
            upgradeStatusStr += (" (" + upgradeDigest.Sru.Err + ")");
        }
    }

    Component.onCompleted:
    {
        appMain.screenStateChanged.connect(slotScreenStateChanged);
        slotScreenStateChanged();
    }

    Component.onDestruction: {
        appMain.screenStateChanged.disconnect(slotScreenStateChanged);
    }

    function slotScreenStateChanged()
    {
        visible = (appMain.screenState === "Admin-Service-Upgrade");
        reload();
    }

    function reload()
    {
        if (!visible)
        {
            fileDialog.close();
            return;
        }
    }

    function isReadyToStart()
    {
        if ( (upgradeDigest === undefined) ||
             (upgradeDigest.Sru === undefined) )
        {
            return false;
        }
        else if ( (upgradeDigest.Sru.PathFile === "") &&
                  (upgradeDigest.Hcu.PathFile === "") &&
                  (upgradeDigest.Mcu.PathFile === "") &&
                  (upgradeDigest.Stopcock.PathFile === "") )
        {
            // No file is selected
            return false;
        }
        else if (upgradeDigest.State !== "Ready")
        {
            // Busy state
            return false;
        }
        return true;
    }

    function getInfoTableRowText(rowTitle, curText)
    {
        if ( (upgradeDigest === undefined) ||
             (upgradeDigest.Sru === undefined) )
        {
            return "";
        }

        if ( (upgradeDigest.Sru.PathFile === "") &&
             (upgradeDigest.Hcu.PathFile === "") &&
             (upgradeDigest.Mcu.PathFile === "") &&
             (upgradeDigest.Stopcock.PathFile === "") )
        {
            // No file is selected
            return "";
        }

        if (rowTitle === "File Size (KB)")
        {
            return upgradeDigest.Sru.FileSizeKB;
        }
        else if (rowTitle === "Progress")
        {
            return upgradeDigest.Sru.Progress + "%";
        }
        else if (rowTitle === "Status")
        {
            return upgradeStatusStr;
        }
    }

    function reboot()
    {
        shutdownCountTimer.stop();
        dsSystem.slotShutdown(false);
        appMain.setInteractiveState(false, "AdminServiceUpgrade: reboot");
    }
}
