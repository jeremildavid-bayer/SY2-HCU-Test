import QtQuick 2.12
import "../../../Widgets"

Item {
    property var motorModuleSerialNumbers: dsMcu.motorModuleSerialNumbers
    property string serialNumber: dsHardwareInfo.serialNumber
    property string mcuSerialNumber: dsMcu.mcuSerialNumber
    property string baseBoardType: dsHardwareInfo.baseBoardType
    property var bmsDigests: dsMcu.bmsDigests

    property var allConfigList: dsHardwareInfo.configTable
    property var configList: []
    property int col1x: 0
    property int col2x: width * 0.4
    property int col3x: width * 0.6
    property int tableItemMargin: width * 0.015

    id: hardwareInfoBoardPage
    anchors.fill: parent
    visible: false

    Rectangle {
        id: border1
        x: col2x
        y: 0
        width: 1
        height: parent.height
        color: colorMap.text02
    }

    Rectangle {
        id: border2
        x: col3x
        y: 0
        width: 1
        height: parent.height
        color: colorMap.text02
    }

    Item {
        id: headings
        clip: true
        width: parent.width
        height: parent.height * 0.1

        Text {
            id: labelType
            x: col1x + tableItemMargin
            width: col2x - x - tableItemMargin
            height: parent.height
            verticalAlignment: Text.AlignVCenter
            text: "Type"
            font.pixelSize: height * 0.45
            font.family: fontRobotoBold.name
            color: colorMap.text01
            wrapMode: Text.Wrap
        }

        Text {
            id: labelRevision
            x: col2x + tableItemMargin
            width: col3x - x - tableItemMargin
            height: parent.height
            verticalAlignment: Text.AlignVCenter
            text: "Revision"
            font.pixelSize: height * 0.45
            font.family: fontRobotoBold.name
            color: colorMap.text01
            wrapMode: Text.Wrap
        }

        Text {
            id: labelSerialNumber
            x: col3x + tableItemMargin
            width: parent.width - x - tableItemMargin
            height: parent.height
            verticalAlignment: Text.AlignVCenter
            text: "Serial / Batch #"
            font.pixelSize: height * 0.45
            font.family: fontRobotoBold.name
            color: colorMap.text01
            wrapMode: Text.Wrap
        }
    }

    Rectangle {
        id: border3
        anchors.top: headings.bottom
        width: parent.width
        height: 1
        color: colorMap.text02
    }

    ListView {
        property double lastContentY: 0

        id: listViewSettings
        width: parent.width
        anchors.top: border3.bottom
        anchors.bottom: parent.bottom
        cacheBuffer: Math.max(1, Math.max(contentHeight * 2, height * 10))
        clip: true
        delegate: AdminServiceHardwareInfoBoardListItem {}

        footer: Item {
            width: ListView.view ? ListView.view.width : 0
            height: (widgetKeyboard.keyboardHeight - actionBarHeight)
        }

        ScrollBar {}
        ListFade {}

        model: configList.length

        onModelChanged: {
            contentY = lastContentY;
        }

        onContentYChanged: {
            if (contentY != -0)
            {
                lastContentY = contentY;
            }
        }
    }

    onAllConfigListChanged: {
        reloadConfigItems();
        reload();
    }

    onSerialNumberChanged: {
        reloadConfigItems();
        reload();
    }

    onMotorModuleSerialNumbersChanged: {
        reloadConfigItems();
        reload();
    }

    onMcuSerialNumberChanged: {
        reloadConfigItems();
        reload();
    }

    onBaseBoardTypeChanged: {
        reloadConfigItems();
        reload();
    }

    onBmsDigestsChanged: {
        reloadConfigItems();
        reload();
    }

    Component.onCompleted: {
        appMain.screenStateChanged.connect(slotScreenStateChanged);
        slotScreenStateChanged();
    }

    Component.onDestruction: {
        appMain.screenStateChanged.disconnect(slotScreenStateChanged);
    }

    function slotScreenStateChanged()
    {
        visible = (appMain.screenState === "Admin-Service-HardwareInfo-Boards");
        reload();
    }

    function reload()
    {
        if (!visible)
        {
            return;
        }

        if (widgetInputPad.isOpen())
        {
            widgetInputPad.close(false);
        }

        if (widgetKeyboard.isOpen())
        {
            widgetKeyboard.close(false);
        }
    }

    function reloadConfigItems()
    {
        if (allConfigList === undefined)
        {
            configList = [];
            return;
        }

        var newConfigList = [];
        var newConfigMap = {};
        var prefixRevision = "Boards_Revision_";
        var prefixSerialNumber = "Boards_SerialNumber_";

        for (var i = 0; i < allConfigList.length; i++)
        {
            var configItem = allConfigList[i];
            var key;

            if (configItem.KeyName.indexOf(prefixRevision) === 0)
            {
                key = configItem.KeyName.substr(prefixRevision.length, configItem.KeyName.length - prefixRevision.length);
                if (newConfigMap[key] === undefined)
                {
                    newConfigMap[key] = {};
                }
                newConfigMap[key].Revision = configItem;
            }
            else if (configItem.KeyName.indexOf(prefixSerialNumber) === 0)
            {
                key = configItem.KeyName.substr(prefixSerialNumber.length, configItem.KeyName.length - prefixSerialNumber.length);
                if (newConfigMap[key] === undefined)
                {
                    newConfigMap[key] = {};
                }
                newConfigMap[key].SerialNumber = configItem;
            }
            else
            {
                continue;
            }

            if ( (newConfigMap[key].Revision !== undefined) &&
                 (newConfigMap[key].SerialNumber !== undefined) )
            {
                var configItemMap = {
                    Type: key,
                    Revision: newConfigMap[key].Revision,
                    SerialNumber: newConfigMap[key].SerialNumber,
                    SerialNumberReadOnly: false
                };

                // Set read only items
                if (key === "PA1302_MotorModuleS0")
                {
                    configItemMap.SerialNumber.Value = motorModuleSerialNumbers[0];
                    configItemMap.SerialNumberReadOnly = true;
                }
                else if (key === "PA1302_MotorModuleC1")
                {
                    configItemMap.SerialNumber.Value = motorModuleSerialNumbers[1];
                    configItemMap.SerialNumberReadOnly = true;
                }
                else if (key === "PA1302_MotorModuleC2")
                {
                    configItemMap.SerialNumber.Value = motorModuleSerialNumbers[2];
                    configItemMap.SerialNumberReadOnly = true;
                }
                else if (key === "PA1300_Main")
                {
                    configItemMap.SerialNumber.Value = mcuSerialNumber;
                    configItemMap.SerialNumberReadOnly = true;
                }
                else if (key === "PA1308_BatteryA")
                {
                    configItemMap.SerialNumber.Value = (bmsDigests[0] !== undefined) ? bmsDigests[0].SbsStatus.SerialNumber : "--"
                    configItemMap.SerialNumberReadOnly = true;
                }
                else if (key === "PA1308_BatteryB")
                {
                    configItemMap.SerialNumber.Value = (bmsDigests[1] !== undefined) ? bmsDigests[1].SbsStatus.SerialNumber : "--"
                    configItemMap.SerialNumberReadOnly = true;
                }

                newConfigList.push(configItemMap);
            }
        }

        //logDebug("newConfigList = " + JSON.stringify(newConfigList));
        configList = newConfigList;
    }

}
