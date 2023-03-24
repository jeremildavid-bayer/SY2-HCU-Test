import QtQuick 2.12
import QtQuick.Controls 2.2
import QtQuick.VirtualKeyboard 2.15
import QtQuick.VirtualKeyboard.Styles 2.15
import QtQuick.VirtualKeyboard.Settings 2.15
import "../../Util.js" as Util

Item {
    property int prevMainWindowY: 0
    property int newMainWindowY: appMainView.y
    property int keyboardWidth: inputPanel.width
    property int keyboardHeight: inputPanel.height
    property int animationMs: 250
    property var containerObject
    property var textObject
    property string currentValue: ""
    property string startValue: ""
    property int maxLen: 20
    property int minLen: 0
    property var keyPressedSoundCallback: function() { soundPlayer.playPressKey(); }
    property bool valueOk: (currentValue.length >= minLen) && (currentValue.length <= maxLen)
    property string cultureCode: dsCfgGlobal.cultureCode
    property var availableLocales: []

    signal signalClosed(bool modified)
    signal signalValueChanged(var val)

    id: keyboard
    width: appMainView.width
    height: appMainView.height

    Rectangle {
        id: textContainer
        visible: false
        x: parent.width * 0.1
        width: parent.width * 0.5
        y: parent.height * 0.3
        height: parent.height * 0.1
        color: colorMap.actionButtonBackground
        clip: true

        TextInput {
            id: textEdit
            color: valueOk ? colorMap.actionButtonText : colorMap.errText
            selectionColor: valueOk ? colorMap.actionButtonText : colorMap.errText
            selectedTextColor: colorMap.white01
            selectByMouse: true

            cursorDelegate: Rectangle {
                id: rectCursor
                color: colorMap.actionButtonText
                width: 5
                height: textEdit.height * 0.8

                SequentialAnimation {
                    id: animationCursorBlink
                    loops: Animation.Infinite
                    NumberAnimation { target: rectCursor; properties: "opacity"; to: 1; duration: 500 }
                    NumberAnimation { target: rectCursor; properties: "opacity"; to: 0; duration: 500 }
                }

                Component.onCompleted: {
                    textContainer.onVisibleChanged.connect(reload);
                    reload();
                }

                Component.onDestruction: {
                    textContainer.onVisibleChanged.disconnect(reload);
                }

                function reload()
                {
                    if (textContainer.visible)
                    {
                        animationCursorBlink.start();
                    }
                    else
                    {
                        animationCursorBlink.stop();
                    }
                }
            }

            onAccepted: {
                // When Enter key pressed
                textObject.text = textEdit.text;
                close(true);
            }

            Keys.onPressed: {
                keyPressedSoundCallback();
            }

            onTextChanged: {
                currentValue = text;
                signalValueChanged(currentValue);
            }
        }
    }

    InputPanel {
        id: inputPanel
        y: appMainWindow.height
        width: appMainWindow.width
        visible: false
        state: "INACTIVE"
        states: [
            State {
                name: "ACTIVE"
                PropertyChanges { target: inputPanel; y: appMainWindow.height - inputPanel.height; }
                PropertyChanges { target: appMainView; y: newMainWindowY; }
            },
            State {
                name: "INACTIVE"
                PropertyChanges { target: inputPanel; y: appMainWindow.height; }
                PropertyChanges { target: appMainView; y: 0; }
            }
        ]

        transitions: [
            Transition {
                from: "INACTIVE"
                to: "ACTIVE"
                ParallelAnimation {
                    NumberAnimation { target: inputPanel; properties: "y"; duration: animationMs; }
                    NumberAnimation { target: appMainView; properties: "y"; duration: animationMs; }
                }

                onRunningChanged: {
                    // make input panel visible when transition STARTs
                    if (inputPanel.state === "ACTIVE" && running) {
                        inputPanel.visible = true;
                    }
                }
            },
            Transition {
                from: "ACTIVE"
                to: "INACTIVE"
                ParallelAnimation {
                    NumberAnimation { target: inputPanel; properties: "y"; to: appMainWindow.height; duration: animationMs; }
                    NumberAnimation { target: appMainView; properties: "y"; to: 0; duration: animationMs; }
                }

                ScriptAction { script: {
                        newMainWindowY = 0;
                    }
                }

                onRunningChanged: {
                    // make input panel in-visible when transition ENDs
                    if (inputPanel.state === "INACTIVE" && !running) {
                        inputPanel.visible = false;
                    }
                }
            }
        ]

        onActiveChanged: {
            //logDebug("Keyboard: active=" + active);
            if (!active)
            {
                // Keyboard is not active anymore.
                timerSingleShot(50, function() {
                    if (!active)
                    {
                        // Make sure 'Accepted' event is handled before closing the keyboard
                        //logDebug("Keyboard: active=" + active + ", Force closing keyboard");
                        close(false);
                    }
                });
            }
        }
    }

    onCultureCodeChanged: {
        loadCurentLocale();
    }

    Component.onCompleted: {
        VirtualKeyboardSettings.styleName = "imax";
        appMain.screenStateChanged.connect(slotScreenStateChanged);
        appMain.signalUIStarted.connect(init);
    }

    Component.onDestruction: {
        appMain.screenStateChanged.disconnect(slotScreenStateChanged);
        appMain.signalUIStarted.disconnect(init);
    }

    function slotScreenStateChanged()
    {
        // Screen changed - close input pad
        close(false);
    }

    function init()
    {
        //VirtualKeyboardSettings.styleName = "imax";
        timerSingleShot(5, function() {
            availableLocales = Util.copyObject(VirtualKeyboardSettings.availableLocales);
            logDebug("Keyboard: init(): availableLocales=" + JSON.stringify(VirtualKeyboardSettings.availableLocales));
            loadCurentLocale();
        });
    }

    function loadCurentLocale()
    {
        if (availableLocales.length == 0)
        {
            return;
        }

        var locale = cultureCode;
        locale = locale.replace("-", "_");
        var localePrefix = locale.substr(0, 2);
        var localeKey = "";

        // Get localeKey from local string
        for (var localeIdx = 0; localeIdx < availableLocales.length; localeIdx++)
        {
            if (availableLocales[localeIdx] === locale)
            {
                localeKey = availableLocales[localeIdx];
                break;
            }
        }

        if (localeKey === "")
        {
            // for French, default is fr_FR if not found in the list
            if (localePrefix === "fr") {
                localeKey = "fr_FR";
            }
            else {
                // If localeKey not found, get localeKey from local prefix string
                for (localeIdx = 0; localeIdx < availableLocales.length; localeIdx++)
                {
                    if (availableLocales[localeIdx].indexOf(localePrefix) >= 0)
                    {
                        localeKey = availableLocales[localeIdx];
                        break;
                    }
                }
            }
        }

        var activeLocales = [];
        activeLocales.push("en_GB");
        if (localeKey !== "")
        {
            activeLocales.push(localeKey);
        }

        logDebug("Keyboard: loadCurentLocale(): cultureCode=" + cultureCode + ", locale=" + locale + ", localePrefix=" + localePrefix + ", activeLocales=" + JSON.stringify(activeLocales));
        VirtualKeyboardSettings.activeLocales = activeLocales;
    }

    function isOpen()
    {
        return inputPanel.state == "ACTIVE";
    }

    function isOpenFor(object)
    {
        return ( (isOpen()) && (textObject === object) );
    }

    function getShiftDistance(containerObj)
    {
        var containerPos = keyboard.mapFromItem(containerObj, 0, 0);
        var keyboardTopMargin = (appMainView.height * 0.05);

        // shiftig is needed if the keyboard will overlap with the text
        var needsShift = (inputPanel.height > (appMainWindow.height - containerPos.y - containerObj.height - keyboardTopMargin));

        var shiftDistance = (appMainView.y + appMainView.height - inputPanel.height) - (containerPos.y + containerObj.height) - keyboardTopMargin;

        if (inputPanel.state == "ACTIVE")
        {
            // also check if it needs to shift back down as the input area shrinks by deleting the text
            needsShift |= (newMainWindowY < shiftDistance);
        }

        if (needsShift)
        {
            return shiftDistance;
        }
        return 0;
    }

    function shiftMainWindow(containerObj)
    {
        var shiftDistance = getShiftDistance(containerObj);

        if (shiftDistance !== 0)
        {
            // main window needs to be shifted
            if (inputPanel.state !== "ACTIVE")
            {
                // if it is already active, don't update prevMainWindowY
                prevMainWindowY = appMainView.y;
            }
            newMainWindowY = shiftDistance;
        }
        else
        {
            if (inputPanel.state == "ACTIVE")
            {
                // Still active
            }
            else
            {
                prevMainWindowY = appMainView.y;
            }
            newMainWindowY = appMainView.y;
        }
    }

    function open(containerObj, textObj, minLength, maxLength)
    {
        if (widgetInputPad.isOpen())
        {
            widgetInputPad.close(false);
        }

        // Check if main window needs to be shifted
        appMainView.y = prevMainWindowY;
        maxLen = maxLength;
        minLen = minLength;
        containerObject = containerObj;
        textObject = textObj;
        shiftMainWindow(containerObject);

        textContainer.parent = containerObject.parent;
        textContainer.x = containerObject.x;
        textContainer.y = containerObject.y;
        textContainer.width = Qt.binding(function() { return containerObject.width; });
        textContainer.height = Qt.binding(function() { return containerObject.height; });
        textContainer.z = containerObject.z;
        textContainer.visible = true;
        containerObject.visible = false;

        textEdit.wrapMode = TextEdit.NoWrap;
        textEdit.x = textObject.x;
        textEdit.y = textObject.y;
        textEdit.width = Qt.binding(function() { return textObject.width; });
        textEdit.height = Qt.binding(function() { return textObject.height; });
        textEdit.maximumLength = maxLen;
        textEdit.horizontalAlignment = textObject.horizontalAlignment;
        textEdit.verticalAlignment = textObject.verticalAlignment;
        textEdit.font = textObject.font;
        textEdit.text = textObject.text;
        textEdit.select(textEdit.text.length, 0);
        textEdit.forceActiveFocus();

        // Initialise data
        startValue = textObj.text;
        currentValue = textObj.text;

        // Display Keyboard
        inputPanel.state = "ACTIVE";
    }

    function close(modified)
    {
        if (!isOpen())
        {
            return;
        }

        inputPanel.state = "INACTIVE";
        emitSignalClosed(modified);
        appMainView.forceActiveFocus();
        containerObject.visible = true;
        textContainer.visible = false;
        textContainer.parent = keyboard;
    }

    function setCurrentValue(newCurrentValue)
    {
        currentValue = newCurrentValue;
        textEdit.text = newCurrentValue;
        textEdit.cursorPosition = newCurrentValue.length;
    }

    function emitSignalClosed(modified)
    {
        signalClosed(modified);
    }

    function textSelect(start, end)
    {
        textEdit.select(start, end);
    }

    function textDeselect()
    {
        textEdit.deselect();
    }

    function textCursorPosition(newPos)
    {
        textEdit.cursorPosition = newPos;
    }

    function textSetWrapMode(mode)
    {
        textEdit.wrapMode = mode;
    }
}
