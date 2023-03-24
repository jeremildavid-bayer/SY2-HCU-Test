import QtQuick 2.12
import QtQuick.Controls 2.12
import "../../../Widgets"

AdminServiceToolTestTemplate {
    property string bmsDigestOutput: dsMcu.bmsDigestOutput
    property string bmsCommandOutput: dsMcu.bmsCommandOutput

    property int delayBetweenBmsCommandsMs: 500

    property var bmsManufacturingStatusList: adminBMS.bmsManufacturingStatusList

    property var bmsCommandsList: [
        { commandName: "ShutdownMode", command: "0010", },
        { commandName: "SleepMode", command: "0011", },
        { commandName: "DeviceReset", command: "0012", },
        { commandName: "PCHG_FET_TOGGLE", command: "001E", },
        { commandName: "CHG_FET_TOGGLE", command: "001F", },
        { commandName: "DCHG_FET_TOGGLE", command: "0020", },
        {
            commandName: "FET_EN",
            description: "AllFetAction",
            command: "0022",
            defaultValue: "true",
            currentValue: undefined,
            location: bmsManufacturingStatusList,
        },
        {
            commandName: "LIFETIME_EN",
            description: "LifetimeDataCollection",
            command: "0023",
            defaultValue: "true",
            currentValue: undefined,
            location: bmsManufacturingStatusList,
        },
        { commandName: "LIFETIME_RESET", command: "0028", },
        { commandName: "LIFETIME_FLUSH", command: "002E", },
        {
            commandName: "PF_EN",
            description: "PermanentFailure",
            command: "0024",
            defaultValue: "true",
            currentValue: undefined,
            location: bmsManufacturingStatusList,
        },
        { commandName: "PF_CLEAR", command: "0029", },
        {
            commandName: "BBR_EN",
            description: "BlackBoxReorder",
            command: "0025",
            defaultValue: "true",
            currentValue: undefined,
            location: bmsManufacturingStatusList,
        },
        { commandName: "BBR_CLEAR", command: "002A", },
        {
            commandName: "SAFE_EN",
            description: "SafeAction",
            command: "0026",
            defaultValue: "true",
            currentValue: undefined,
            location: bmsManufacturingStatusList,
        },
        {
            commandName: "LED_EN",
            description: "LedDisplay",
            command: "0027",
            defaultValue: "true",
            currentValue: undefined,
            location: bmsManufacturingStatusList,
        },
        { commandName: "SealDevice", command: "0030", },
        { commandName: "Unseal", command: "UNSEAL", },
        { commandName: "FullAccess", command: "FULLACCESS", },
    ]

    titleWidthPercent: 0.6
    paramsTableRowWidth: width * 0.6
    activeScreenState: "Admin-Service-Tool-BMSConsole"
    btnStart.visible: false

    paramsTable: [
        AdminServiceToolTestTableRow {
            titleText: "BMS Digest"
            valueText: "Send"
            controlType: "BUTTON"
            onBtnClicked: {
                dsMcu.slotBmsDigest();
            }
        },

        Repeater {
            model: bmsCommandsList.length
            AdminServiceToolTestTableRow {
                titleText: (bmsCommandsList[index].commandName + " [" + bmsCommandsList[index].command +  "]")
                controlType: "OBJECT"
                customContent: [
                    AdminServiceToolBMSConsoleTableCell {
                        onBtnClicked: (batteryIdx) => {
                            var context = "Send " + (bmsCommandsList[index].commandName + " [" + bmsCommandsList[index].command +  "]") + " to battery [" + ((batteryIdx === 0)? "A":"B") + "] ?";
                            var bmsCommandArgumentList = [ batteryIdx, bmsCommandsList[index].command ];
                            openConfirmationPopup(context, sendBmsCommand, bmsCommandArgumentList);
                        }
                    }
                ]
            }
        },

        AdminServiceToolTestTableRow {
            titleText: "RESET TO DEFAULTS"
            controlType: "OBJECT"
            customContent: [
                AdminServiceToolBMSConsoleTableCell {
                    onBtnClicked: {
                        var context = "Reset battery [" + ((batteryIdx===0)?"A":"B") + "] to default ?";
                        var bmsCommandArgumentList = [ batteryIdx ];
                        openConfirmationPopup(context, resetBatteryToDefault, bmsCommandArgumentList);
                    }
                }
            ]
        }
    ]

    onBmsDigestOutputChanged: {
        consoleText.append(new Date().toTimeString("hh:mm:ss") + " - ==========================\nBMS DIGEST:" + bmsDigestOutput + "\n\n");
        trimVisibleLog();
    }

    onBmsCommandOutputChanged: {
        consoleText.append(new Date().toTimeString("hh:mm:ss") + " - " + bmsCommandOutput);
        trimVisibleLog();
    }

    function fillCurrentBMSValues(index)
    {
        for (var key in bmsCommandsList)
        {
            if (bmsCommandsList.hasOwnProperty(key))
            {
                var command = bmsCommandsList[key];

                // only interested in commands that have default value
                if ((command.defaultValue !== undefined) && (command.location !== undefined))
                {
                    // clear current value first
                    command.currentValue = undefined;

                    var searchString = (command.description !== undefined) ? command.description : command.commandName;
                    for (var bmsItemKey in command.location)
                    {
                        if (command.location.hasOwnProperty(bmsItemKey))
                        {
                            var bmsItem = command.location[bmsItemKey];
                            if (bmsItem.Name.indexOf(searchString) >= 0)
                            {
                                command.currentValue = ((index === 0) ? bmsItem.Val1 : bmsItem.Val2);
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    function resetBatteryToDefault(batteryIdx)
    {
        consoleText.text = "";
        consoleText.cursorPosition = 0;
        consoleText.append("Resetting battery [" + ((batteryIdx === 0)?"A":"B") + "] to defaults...");

        // unseal first
        sendBmsCommand(batteryIdx, "UNSEAL");

        fillCurrentBMSValues(batteryIdx);

        // get sub array of what flags needs to reset
        var filteredList = bmsCommandsList.filter(function(item){
            return ((item.defaultValue !== undefined) &&
                    (item.defaultValue.toUpperCase() !== item.currentValue.toUpperCase()));
        });

        var i = 0, num = filteredList.length;

        // don't burst bms commands. using timer of 500ms between each command
        function f() {
            sendBmsCommand(batteryIdx, filteredList[i].command);
            i++;
            if (i < num) {
                timerSingleShot(delayBetweenBmsCommandsMs, f);
            }
            else
            {
                timerSingleShot(delayBetweenBmsCommandsMs, function() {
                    consoleText.append("Setting defaults done!");
                });
            }
        }

        if (num > 0)
        {
            f();
        }
        else
        {
            consoleText.append("Setting defaults done!");
        }

    }

    function sendBmsCommand(index, data)
    {
        var indexStr = (index === 0) ? "A" : "B";
        dsMcu.bmsCommandOutput = "TX: " + indexStr + "[" + data + "]..";
        dsMcu.slotBmsCommand(index, data);
    }

    function openConfirmationPopup(context, confirmFunction, confirmFunctionArgumentsList)
    {
        popupManager.popupActionConfirmation.titleText = "BMS Command Confirmation";
        popupManager.popupActionConfirmation.contentText = context;
        popupManager.popupActionConfirmation.okFunction = confirmFunction;
        popupManager.popupActionConfirmation.okFunctionArgumentsList = confirmFunctionArgumentsList;
        popupManager.popupActionConfirmation.open();
    }
}

