import QtQuick 2.12
import "../../Widgets"
import "../../Widgets/ConfigTable"
import "../../Widgets/Popup"
import "../../Util.js" as Util

Item {
    property string accessCode: dsCfgGlobal.accessCode
    property string servicePasscode: dsCfgGlobal.servicePasscode
    property var globalConfigs: dsCfgGlobal.configTable
    property var localConfigs: dsCfgLocal.configTable
    property var allConfigs
    property int prevPageIdx: 0
    property int frameMargin: dsCfgLocal.screenW * 0.02
    property int leftFrameWidth: (dsCfgLocal.screenW * 0.21) + (frameMargin * 2)
    property int listViewItemHeight: parent.height * 0.12
    property int cellMargin: width * 0.015
    property int currentUtcOffsetMinutes: dsCfgLocal.currentUtcOffsetMinutes
    property bool licenseEnabledPatientStudyContext: dsCru.licenseEnabledPatientStudyContext
    property bool licenseEnabledWorklistSelection: dsCru.licenseEnabledWorklistSelection
    property string baseBoardType: dsHardwareInfo.baseBoardType

    anchors.fill: parent
    visible: false

    ListView {
        id: listView
        width: leftFrameWidth
        height: parent.height
        clip: true

        ScrollBar {}
        ListFade {}

        model: ListModel {
            ListElement { title: "General" }
            ListElement { title: "SruLink" }
            ListElement { title: "Sound" }
            ListElement { title: "Display" }
            ListElement { title: "Injector" }
            //ListElement { title: "Admin" }
        }

        delegate: GenericButton {
            width: ListView.view.width
            height: listViewItemHeight
            radius: 0
            color: (listView.currentIndex === index) ? colorMap.text02 : "transparent"

            Text {
                text: translate("T_SettingsMenuItem_" + title)
                anchors.fill: parent
                anchors.leftMargin: cellMargin
                anchors.rightMargin: cellMargin
                font.family: (listView.currentIndex === index) ? fontRobotoBold.name : fontRobotoLight.name
                font.pixelSize: height * 0.4
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
                color: (listView.currentIndex === index) ? colorMap.mainBackground : colorMap.text01
                wrapMode: Text.Wrap
            }

            onBtnClicked: {
                selectPage(index);
            }

            Component.onCompleted: {
                listView.dragStarted.connect(reset);
            }

            Component.onDestruction: {
                listView.dragStarted.disconnect(reset);
            }
        }
    }

    Rectangle {
        // separator line
        id: rectSeparator
        anchors.left: listView.right
        width: frameMargin / 6
        height: parent.height
        color: colorMap.titleBarBackground
    }

    Item {
        id: mainRect
        anchors.left: rectSeparator.right
        anchors.leftMargin: frameMargin
        width: parent.width - x - frameMargin
        height: parent.height

        ConfigTable {
            allConfigList: allConfigs
            configKeyName: "Settings_General_"
            visibleScreenState: "Admin-Settings-General"
            onSignalConfigChanged: (configItem) => {
                slotConfigChanged(configItem);
            }
            customConfigRows: {
                "Settings_General_DateFormat": {
                    "getTextFromConfigValue": function(configValue) {
                        var dateFormat = configValue;
                        var dateFormatLocale = translate("T_ConfigItem_Settings_General_DateFormat_" + dateFormat);
                        var cfgVal = Util.getCurrentDateTimeStr(dateFormatLocale, "", currentUtcOffsetMinutes);
                        return cfgVal.trim();
                    },
                    "getValidList": function(validList) {
                        var newValidList = [];
                        for (var listIdx = 0; listIdx < validList.length; listIdx++)
                        {
                            var dateFormat = validList[listIdx];
                            var dateFormatLocale = translate("T_ConfigItem_Settings_General_DateFormat_" + dateFormat);
                            var cfgVal = Util.getCurrentDateTimeStr(dateFormatLocale, "", currentUtcOffsetMinutes);
                            newValidList.push(cfgVal.trim());
                        }
                        return newValidList;
                    },
                    "updateFromRealDateTime": true
                },
                "Settings_General_TimeFormat": {
                    "getTextFromConfigValue": function(configValue) {
                        var timeFormat = configValue;
                        var timeFormatLocale = translate("T_ConfigItem_Settings_General_TimeFormat_" + timeFormat);
                        var cfgVal = Util.getCurrentDateTimeStr("", timeFormatLocale, currentUtcOffsetMinutes);
                        return cfgVal.trim();
                    },
                    "getValidList": function(validList) {
                        var newValidList = [];
                        for (var listIdx = 0; listIdx < validList.length; listIdx++)
                        {
                            var timeFormat = validList[listIdx];
                            var timeFormatLocale = translate("T_ConfigItem_Settings_General_TimeFormat_" + timeFormat);
                            var cfgVal = Util.getCurrentDateTimeStr("", timeFormatLocale, currentUtcOffsetMinutes);
                            newValidList.push(cfgVal.trim());
                        }
                        return newValidList;
                    },
                    "updateFromRealDateTime": true
                }
            }
        }

        ConfigTable {
            allConfigList: allConfigs
            configKeyName: "Settings_SruLink_"
            visibleScreenState: "Admin-Settings-SruLink"
            onSignalConfigChanged: (configItem) => {
                slotConfigChanged(configItem);
            }
            customConfigRows: {
                "Settings_SruLink_WirelessCountryCode": {
                    "getVisibleState": function() {
                        for (var cfgIdx = 0; cfgIdx < allConfigList.length; cfgIdx++)
                        {
                            if (allConfigList[cfgIdx].KeyName === "Settings_SruLink_ConnectionType")
                            {
                                if (allConfigList[cfgIdx].Value === "WirelessEthernet")
                                {
                                    return true;
                                }
                            }
                        }
                        return false;
                    }
                }
            }
        }

        ConfigTable {
            allConfigList: allConfigs
            configKeyName: "Settings_Sound_"
            visibleScreenState: "Admin-Settings-Sound"
            onSignalConfigChanged: (configItem) => {
                slotConfigChanged(configItem);
            }
            customConfigRows: {
                "Settings_Sound_NormalAudioLevel": {
                    "sliderCtrlLabelStart": "\ue981"
                },
                "Settings_Sound_InjectionAudioLevel": {
                    "sliderCtrlLabelStart": "\ue981"
                },
                "Settings_Sound_SudsPrimedAudioLevel": {
                    "sliderCtrlLabelStart": "\ue981"
                }
            }
        }

        ConfigTable {
            allConfigList: allConfigs
            configKeyName: "Settings_Display_"
            visibleScreenState: "Admin-Settings-Display"
            onSignalConfigChanged: (configItem) => {
                slotConfigChanged(configItem);
            }
            customConfigRows: {
                "Settings_Display_ScreenOffTimeoutMinutes": {
                    "sliderCtrlLabelStart": "\ue964"
                }
            }
        }

        ConfigTable {
            allConfigList: allConfigs
            configKeyName: "Settings_Injector_"
            visibleScreenState: "Admin-Settings-Injector"
            onSignalConfigChanged: (configItem) => {
                slotConfigChanged(configItem);
            }
        }
    }

    onLocalConfigsChanged: {
        setAllConfigs();
    }

    onGlobalConfigsChanged: {
        setAllConfigs();
    }

    onBaseBoardTypeChanged: {
        showHideNetworkMenu();
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
        var prevVisible = (appMain.screenStatePrev.indexOf("Admin-Setting") >= 0);
        visible = (appMain.screenState.indexOf("Admin-Setting") >= 0);
        if ( (visible) && (!prevVisible))
        {
            // Network menu needs to be correctly hidden/visible depending on baseboard type
            // Once it is correctly hidden/shown initially, changing baseboard type will update this.
            showHideNetworkMenu();

            selectPage(0);
        }
        reload();
    }

    function reload()
    {
        if (!visible)
        {
            widgetInputPad.close();
            return;
        }
    }

    function setAllConfigs()
    {
        if ( (localConfigs === undefined) &&
             (globalConfigs === undefined) )
        {
            allConfigs = undefined;
        }
        else if ( (localConfigs !== undefined) &&
                  (globalConfigs === undefined) )
        {
            allConfigs = localConfigs;
        }
        else if ( (globalConfigs !== undefined) &&
                  (localConfigs === undefined) )
        {
            allConfigs = globalConfigs;
        }
        else
        {
            // Merge Config
            allConfigs = localConfigs.concat(globalConfigs);
        }
    }

    function selectPage(pageIdx)
    {
        if (pageIdx !== (listView.model.count - 1))
        {
            prevPageIdx = pageIdx
        }

        listView.currentIndex = pageIdx;

        var selectedPage = listView.model.get(listView.currentIndex);

        if (selectedPage.title === "General")
        {
            appMain.setScreenState("Admin-Settings-General");
        }
        if (selectedPage.title === "SruLink")
        {
            appMain.setScreenState("Admin-Settings-SruLink");
        }
        if (selectedPage.title === "Sound")
        {
            appMain.setScreenState("Admin-Settings-Sound");
        }
        if (selectedPage.title === "Display")
        {
            appMain.setScreenState("Admin-Settings-Display");
        }
        else if (selectedPage.title === "Injector")
        {
            appMain.setScreenState("Admin-Settings-Injector");
        }

        /* TODO: Is this used?
        else if (listView.currentIndex === (listView.model.count - 1))
        {
            appMain.setScreenState("Admin-Settings-Advanced");
        }
        */

    }

    function slotConfigChanged(configItem)
    {
        var cfgIdx;
        var configTable = dsCfgGlobal.configTable;

        for (cfgIdx = 0; cfgIdx < configTable.length; cfgIdx++)
        {
            //logDebug("configItem.KeyName=" + configItem.KeyName + ", configTable.KeyName=" + configTable.KeyName);
            if (configTable[cfgIdx].KeyName === configItem.KeyName)
            {
                dsCfgGlobal.slotConfigChanged(configItem);
                break;
            }
        }

        configTable = dsCfgLocal.configTable;
        for (cfgIdx = 0; cfgIdx < configTable.length; cfgIdx++)
        {
            //logDebug("configItem.KeyName=" + configItem.KeyName + ", configTable.KeyName=" + configTable.KeyName);
            if (configTable[cfgIdx].KeyName === configItem.KeyName)
            {
                dsCfgLocal.slotConfigChanged(configItem);
                break;
            }
        }
    }

    function slotInputPadValChanged(newValue)
    {
        popupManager.popupEnterPassCode.setValue(newValue);
    }

    function slotInputPadClosed(modified)
    {
        widgetInputPad.signalValueChanged.disconnect(slotInputPadValChanged);
        widgetInputPad.signalClosed.disconnect(slotInputPadClosed);

        //var accessCodeOk = false;

        if (modified)
        {
            if ( (widgetInputPad.currentValue === accessCode) ||
                 (widgetInputPad.currentValue === servicePasscode) )
            {
                accessCodeOk = true;
            }
            else
            {
                logError("Bad access code is entered");
                soundPlayer.playError();
            }
        }

        popupManager.popupEnterPassCode.textWidget.text = "";
        popupManager.popupEnterPassCode.close();
    }

    function showHideNetworkMenu()
    {
        if (allConfigs !== undefined)
        {
            if ((baseBoardType === undefined) || (baseBoardType !== "Battery"))
            {
                // remove SruLink
                for (var i = 0; i < listView.model.count; i++)
                {
                    if (listView.model.get(i).title === "SruLink")
                    {
                        listView.model.remove(i);
                        return;
                    }
                }
            }
            else
            {
                // add SruLink networking if it's not there. Add it after "General"
                for (i = listView.model.count - 1; i >= 0; i--)
                {
                    if (listView.model.get(i).title === "SruLink")
                    {
                        return;
                    }
                    if (listView.model.get(i).title === "General")
                    {
                        listView.model.insert(i+1, {title: "SruLink"});
                        return;
                    }
                }
            }
        }
    }
}
