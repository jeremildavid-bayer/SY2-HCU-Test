import QtQuick 2.12
import Qt.labs.folderlistmodel 2.1

import "Popup"

Popup {
    property bool usbStickInserted: dsSystem.usbStickInserted
    property string selectedFilePath: ""
    property string rootPath: "file:///"
    property var extFilters: ["*.*"]

    id: root
    heightMin: dsCfgLocal.screenH * 0.8
    widthMin: dsCfgLocal.screenW * 0.8

    content: [
        Rectangle {
            id: pathStr
            anchors.horizontalCenter: parent.horizontalCenter
            height: parent.height * 0.15
            width: parent.width * 0.8
            color: colorMap.consoleBackground

            Text {
                id: selectedFilePathText
                anchors.fill: parent
                anchors.leftMargin: parent.width * 0.01
                anchors.rightMargin: parent.width * 0.01
                font.pixelSize: height * 0.25
                verticalAlignment: Text.AlignVCenter
                color: colorMap.text01
                elide: Text.ElideRight
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                text: ""
            }
        },

        ListView {
            signal signalReloadAllRows()

            id: fileListview
            y: parent.height * 0.18
            anchors.horizontalCenter: parent.horizontalCenter
            height: parent.height * 0.8
            width: parent.width * 0.8
            clip: true

            ScrollBar {}
            ListFade {
                fadeColor: colorMap.popupBackground
            }

            FolderListModel {
                id: modelFolder
                rootFolder: rootPath
                folder: rootPath
                showDirsFirst: true
                showDotAndDotDot: true
                showDirs: true
                showHidden: false
                nameFilters: extFilters

                onFolderChanged: {
                    selectedFilePath = modelFolder.folder;
                    reload();
                }

                onCountChanged: {
                    selectedFilePath = modelFolder.folder;
                    reload();
                }
            }

            model: modelFolder

            delegate: Rectangle {
                id: row
                width: fileListview.width
                height: fileListview.height * 0.15
                border.width: fileListview.height * 0.002
                border.color: colorMap.grid

                Row {
                    Text {
                        id: rowIcon
                        width: row.width * 0.1
                        height: row.height
                        font.pixelSize: height * 0.5
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                        text: fileIsDir ? "\uf07b" : "\uf1c6"
                    }

                    Text {
                        id: rowText
                        width: row.width * 0.8
                        height: row.height
                        text: fileName
                        font.pixelSize: height * 0.3
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                        wrapMode: Text.Wrap
                    }
                }

                MouseArea {
                    anchors.fill: parent

                    onPressed: {
                        soundPlayer.playPressGood();
                        row.color = colorMap.actionButtonBackground;
                        rowIcon.color = colorMap.actionButtonText;
                        rowText.color = colorMap.actionButtonText;
                    }

                    onReleased: {
                    }

                    onClicked: {
                        if (fileIsDir)
                        {
                            modelFolder.folder = "file://" + filePath;
                        }
                        else
                        {
                            selectedFilePath = "file://" + filePath;
                            root.reload();
                        }
                    }
                }

                function reload()
                {
                    if ( (fileName == ".") || (fileName == "$RECYCLE.BIN") )
                    {
                        row.visible = false;
                        row.height = 0;
                    }
                    else if (selectedFilePath === ("file://" + filePath))
                    {
                        row.color = colorMap.actionButtonBackground;
                        rowIcon.color = colorMap.actionButtonText;
                        rowText.color = colorMap.actionButtonText;
                    }
                    else
                    {
                        row.color = colorMap.keypadButton;
                        rowIcon.color = colorMap.text02;
                        rowText.color = colorMap.text01;
                    }
                }

                Component.onCompleted: {
                    fileListview.dragStarted.connect(reload);
                    fileListview.signalReloadAllRows.connect(reload);
                    reload();
                }

                Component.onDestruction: {
                    fileListview.dragStarted.disconnect(reload);
                    fileListview.signalReloadAllRows.disconnect(reload);
                }
            }
        }
    ]



    onUsbStickInsertedChanged: {
        logDebug("usbStickInserted=" + usbStickInserted + ", modelFolder.folder=" + modelFolder.folder + ", selectedFilePath=" + selectedFilePath);
        if (usbStickInserted)
        {
            if (selectedFilePath == rootPath)
            {
                // usbStick re-inserted.
                init();
            }
        }
        else
        {
            modelFolder.folder = rootPath;
            selectedFilePath = modelFolder.folder;
        }
    }

    Component.onCompleted: {
        appMain.colorMapChanged.connect(reload);
    }

    Component.onDestruction: {
        appMain.colorMapChanged.disconnect(reload);
    }

    function init()
    {
        modelFolder.folder = rootPath;
        selectedFilePath = modelFolder.folder;
        logDebug("FileExplorer Init: modelFolder.folder=" + modelFolder.folder);
        reload();
    }

    function reload()
    {
        // Corrent the path: e.g. "dir1/dir2/.." -> "dir1"
        //selectedFilePath = modelFolder.folder;

        var pathElements = selectedFilePath.split("/");
        if (pathElements[pathElements.length - 1] === "..")
        {
            selectedFilePath = "";
            for (var i = 0; i < pathElements.length - 2; i++)
            {
                if (i > 0)
                {
                    selectedFilePath += "/";
                }
                selectedFilePath += pathElements[i];
            }
        }


        if (modelFolder.count <= 1)
        {
            selectedFilePathText.text = "Please insert the USB stick";
        }
        else
        {
            selectedFilePathText.text = selectedFilePath.replace(rootPath, "USB:/");
        }

        fileListview.signalReloadAllRows();

        // Check file extension
        enableOkBtn = false;
        for (var filterIdx = 0; filterIdx < modelFolder.nameFilters.length; filterIdx++)
        {
            var myExtStart = selectedFilePath.lastIndexOf(".");
            if (myExtStart > 0)
            {
                var myExt = selectedFilePath.substring(myExtStart + 1);
                var filterExt = modelFolder.nameFilters[filterIdx].replace("*.", "");
                if (myExt === filterExt)
                {
                    enableOkBtn = true;
                    break;
                }
            }
        }
    }
}

