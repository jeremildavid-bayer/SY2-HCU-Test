import QtQuick 2.12

Item {
    property int numPadX: appMainView.width * 0.73
    property int numPadY: titleBarHeight
    property int numPadW: appMainView.width - numPadX
    property int numPadH: appMainView.height - titleBarHeight - actionBarHeight
    property int animationMs: 250
    property bool modified: false
    property var modalObjects: []
    property var startValue
    property var endValue
    property var currentValue
    property string screenState: appMain.screenState
    property bool blurBackgroundVisible: false
    property var backgroundWidget
    property var curBackgroundWidget

    property int prevMainWindowX: 0
    property int newMainWindowX: 0

    property var keyPressedSoundCallback: function() { soundPlayer.playPressKey(); }
    property var textContainerWidget
    property var textWidget
    property string origTextContainerWidgetColor: ""
    property string origTextWidgetColor: ""
    property int origTextContainerWidgetRadius: 0
    property bool origTextWidgetIsBold: false

    property bool valueOk

    signal signalClosed(bool modified);
    signal signalValueChanged(var val)

    id: root
    state: "INACTIVE"
    visible: false

    Rectangle {
        id: mainRect
        x: numPadX + numPadW
        y: numPadY
        width: numPadW
        height: numPadH
        color: colorMap.subPanelBackground
        clip: true

        MouseArea {
            // Reserved mouse area to prevent close due to click outside
            anchors.fill: parent
        }

        Item {
            // content
            y: dsCfgLocal.screenH * 0.08 // Space for elapsedTimeFrame
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width * 0.71
            height: parent.height - y

            InputPadInteger {
                id: integerPad
                visible: false
            }

            InputPadFloat {
                id: floatPad
                visible: false
            }

            InputPadDuration {
                id: durationPad
                visible: false
            }

            InputPadDualRatio {
                id: dualRatioPad
                visible: false
            }

            InputPadText {
                id: textPad
                visible: false
            }

            InputPadSelectList {
                id: selectListPad
                visible: false
            }

            InputPadMultiSelectList {
                id: multiSelectListPad
                visible: false
            }

            InputPadDateTime {
                id: dateTimePad
                visible: false
            }
        }
    }

    states: [
        State { name: "ACTIVE" },
        State { name: "INACTIVE" }
    ]

    transitions: [
        Transition {
            from: "INACTIVE"
            to: "ACTIVE"
            SequentialAnimation {
                ScriptAction {
                    script: {
                        if (blurBackgroundVisible)
                        {
                            curBackgroundWidget = getBackgroundWidget();

                            if (curBackgroundWidget !== undefined)
                            {
                                curBackgroundWidget.open([root], true, slotOutsideClicked);
                            }
                        }
                        root.visible = true;
                    }
                }

                ParallelAnimation {
                    NumberAnimation { target: mainRect; properties: "x"; to: numPadX; duration: animationMs; }
                    NumberAnimation { target: appMainView; properties: "x"; to: newMainWindowX; duration: animationMs }
                }

                ScriptAction {
                    script: {
                        if (newMainWindowX != 0)
                        {
                            updateModalObjects();
                        }
                    }
                }
            }
        },
        Transition {
            from: "ACTIVE"
            to: "INACTIVE"
            SequentialAnimation {
                ScriptAction { script: {
                        if (newMainWindowX != 0)
                        {
                            restoreModalObjects();
                        }
                    }
                }

                ParallelAnimation {
                    NumberAnimation { target: mainRect; properties: "x"; to: numPadX + numPadW; duration: animationMs; }
                    NumberAnimation { target: appMainView; properties: "x"; to: prevMainWindowX; duration: animationMs }
                }

                ScriptAction { script: {
                        root.visible = false;
                        if (newMainWindowX == 0)
                        {
                            restoreModalObjects();
                        }
                        newMainWindowX = 0;
                    }
                }
            }
        }
    ]

    SequentialAnimation {
        property QtObject newPad: null
        property QtObject oldPad: null

        id: animationShowPad

        ScriptAction { script: {
                animationShowPad.newPad.x = mainRect.width;
                animationShowPad.newPad.visible = true;
            }
        }

        ParallelAnimation {
            NumberAnimation { target: animationShowPad.newPad; properties: "x"; to: 0; duration: animationMs; }
            NumberAnimation { target: animationShowPad.oldPad; properties: "x"; to: -mainRect.width; duration: animationMs; }
        }

        ScriptAction { script: {
                animationShowPad.oldPad.visible = false;
            }
        }
    }

    Component.onCompleted: {
        appMain.screenStateChanged.connect(slotScreenStateChanged);
    }

    Component.onDestruction: {
        appMain.screenStateChanged.disconnect(slotScreenStateChanged);
    }

    function slotScreenStateChanged()
    {
        // Screen changed - close input pad
        close(false);
    }

    function open(targetPad, newTextContainerWidget, newTextWidget)
    {
        textContainerWidget = newTextContainerWidget;
        textWidget = newTextWidget;

        if (textContainerWidget !== undefined)
        {
            origTextContainerWidgetColor = textContainerWidget.color;
            textContainerWidget.color = colorMap.actionButtonBackground;
            origTextContainerWidgetRadius = textContainerWidget.radius;
            textContainerWidget.radius = 0;

            if (textWidget !== undefined)
            {
                origTextWidgetColor = textWidget.color;
                origTextWidgetIsBold = textWidget.font.bold;
                textWidget.color = colorMap.actionButtonText;
                textWidget.font.bold = true;
            }
        }

        if (widgetKeyboard.isOpen())
        {
            widgetKeyboard.close(false);
        }

        if (state == "ACTIVE")
        {
            signalClosed(true);
            if (targetPad !== animationShowPad.newPad)
            {
                animationShowPad.complete();
                animationShowPad.oldPad = animationShowPad.newPad;
                animationShowPad.newPad = targetPad;
                animationShowPad.start();
            }
        }
        else
        {
            integerPad.visible = false;
            floatPad.visible = false;
            durationPad.visible = false;
            dualRatioPad.visible = false;
            multiSelectListPad.visible = false;
            selectListPad.visible = false;
            textPad.visible = false;
            dateTimePad.visible = false;

            animationShowPad.newPad = targetPad;
            animationShowPad.newPad.x = 0;
            animationShowPad.newPad.visible = true;
        }

        startValue = textWidget.text;
        currentValue = startValue;
        endValue = currentValue;
        modified = false;
        state = "ACTIVE";
    }

    function getBackgroundWidget()
    {
        if (backgroundWidget !== undefined)
        {
            return backgroundWidget;
        }
        else if ( (screenState == "OFF") ||
                  (screenState == "Startup") )
        {
            return startupPageBlurBackground;
        }
        else
        {
            return blurBackground;
        }
    }

    function openIntegerPad(newTextContainerWidget, newTextWidget, unitStr, minValue, maxValue)
    {
        integerPad.unitStr = unitStr;
        integerPad.decimalPointLimit = Math.ceil(maxValue).toString().length;
        integerPad.minValue = minValue;
        integerPad.maxValue = maxValue;

        open(integerPad, newTextContainerWidget, newTextWidget);
    }

    function openFloatPad(newTextContainerWidget, newTextWidget, unitStr, floatPoint, minValue, maxValue)
    {
        floatPad.unitStr = unitStr;
        floatPad.floatPointLimit = floatPoint;
        floatPad.decimalPointLimit = Math.ceil(maxValue).toString().length;
        floatPad.minValue = minValue;
        floatPad.maxValue = maxValue;

        open(floatPad, newTextContainerWidget, newTextWidget);
    }

    function openTextPad(newTextContainerWidget, newTextWidget, minLength, maxLength)
    {
        valueOk = true;
        textPad.minLength = minLength;
        textPad.maxLength = maxLength;
        open(textPad, newTextContainerWidget, newTextWidget);
    }

    function openDurationPad(newTextContainerWidget, newTextWidget, minMs, maxMs)
    {
        valueOk = true;
        durationPad.minMs = minMs;
        durationPad.maxMs = maxMs;
        open(durationPad, newTextContainerWidget, newTextWidget);
    }

    function openDualRatioPad(newTextContainerWidget, newTextWidget)
    {
        valueOk = true;
        dualRatioPad.init(newTextWidget.text);
        open(dualRatioPad, newTextContainerWidget, newTextWidget);
    }

    function openSelectListPad(newTextContainerWidget, newTextWidget, selectList, selectUnits, customListItems)
    {
        valueOk = true;
        selectListPad.init(selectList, selectUnits, customListItems);
        open(selectListPad, newTextContainerWidget, newTextWidget);
    }

    function openMultiSelectListPad(newTextContainerWidget, newTextWidget, selectList, enableKeyboardButton)
    {
        valueOk = true;
        multiSelectListPad.init(selectList, enableKeyboardButton);
        open(multiSelectListPad, newTextContainerWidget, newTextWidget);
    }

    function resetMultiSelectListPadOptions(newSelectList)
    {
        multiSelectListPad.resetSelectOptions(newSelectList);
    }

    function openDateTimePad(newTextContainerWidget, newTextWidget, dateTimeFormat, minDateInclusive, maxDateInclusive)
    {
        if ((minDateInclusive !== undefined) && (maxDateInclusive !== undefined))
        {
            dateTimePad.minDateInclusive = minDateInclusive;
            dateTimePad.maxDateInclusive = maxDateInclusive;
        }
        else
        {
            dateTimePad.minDateInclusive = undefined;
            dateTimePad.maxDateInclusive = undefined;
        }

        dateTimePad.init(dateTimeFormat);
        open(dateTimePad, newTextContainerWidget, newTextWidget);
    }

    function isOpen()
    {
        return state == "ACTIVE";
    }

    function isOpenFor(object)
    {
        return ( (isOpen()) && (textWidget === object) );
    }

    function close(modified)
    {
        if (state == "INACTIVE")
        {
            return;
        }

        if (blurBackgroundVisible)
        {
            if (curBackgroundWidget !== undefined)
            {
                curBackgroundWidget.close([root]);
            }
            blurBackgroundVisible = false;
        }

        // modified should be true when
        // value actually changed AND is OK
        if ((endValue === currentValue) || !valueOk)
        {
            modified = false;
        }
        else
        {
            endValue = currentValue;
        }

        if ( (!valueOk) ||
             (!modified) )
        {
            // Undo change
            currentValue = startValue;
            signalValueChanged(currentValue);
        }

        state = "INACTIVE";
        emitSignalClosed(modified);
    }

    function setCurrentValue(newCurrentValue)
    {
        currentValue = newCurrentValue;
        textWidget.text = newCurrentValue;
    }

    function emitSignalClosed(modified)
    {
        if (textContainerWidget !== undefined )
        {
            textContainerWidget.color = origTextContainerWidgetColor;
            textContainerWidget.radius = origTextContainerWidgetRadius;

            if (textWidget !== undefined)
            {
                textWidget.text = currentValue.toString();
                textWidget.color = origTextWidgetColor;
                textWidget.font.bold = origTextWidgetIsBold;
            }
        }

        textContainerWidget = undefined;
        textWidget = undefined;
        signalClosed(modified);
    }

    function setBackgroundSlide()
    {
        if (state == "INACTIVE")
        {
            if (newMainWindowX == 0)
            {
                newMainWindowX = appMainView.x - numPadW;
            }
        }
    }

    function setBackgroundSlideWithObject(object)
    {
        if (state == "INACTIVE")
        {
            if (newMainWindowX == 0)
            {
                var objectPos = root.mapFromItem(object, 0, 0);

                if ((objectPos.x + object.width) > numPadX)
                {
                    var shiftWindowXBy = objectPos.x + (object.width * 1.1) - numPadX;
                    newMainWindowX = appMainView.x - shiftWindowXBy;
                }
            }
        }
    }

    function setModalObjects(newModalObjects)
    {
        if (modalObjects.length > 0)
        {
            // modalObjects exist already
            return;
        }

        var shiftWindowXBy = 0;

        for (var i = 0; i < newModalObjects.length; i++)
        {
            var object = newModalObjects[i];

            // set new MainWindowX - make sure all modal objects are visible when input pad is activated
            var objectPos = root.mapFromItem(object, 0, 0);

            if ((objectPos.x + object.width) > numPadX)
            {
                var curShiftWindowXBy = objectPos.x + (object.width * 1.1) - numPadX;
                shiftWindowXBy = Math.max(shiftWindowXBy, curShiftWindowXBy);
            }

            // Save current property for object - to restore at the end
            var modalObject = { object: object,
                                parent: object.parent,
                                x0: object.x,
                                y0: object.y,
                                w0: object.width,
                                h0: object.height };

            modalObjects.push(modalObject);
        }

        newMainWindowX = appMainView.x - shiftWindowXBy;

        if (newMainWindowX == 0)
        {
            updateModalObjects();
        }
    }

    function updateModalObjects()
    {
        for (var key in modalObjects)
        {
            //logDebug("key=" + JSON.stringify(key));
            var modalObject = modalObjects[key];
            var object = modalObject.object;
            var pos = root.parent.mapFromItem(object, 0, 0);
            object.parent = root;
            object.x = pos.x;
            object.y = pos.y;
            object.width = modalObject.w0;
            object.height = modalObject.h0;
        }
    }

    function restoreModalObjects()
    {
        // Restore all modalObjects
        while (modalObjects.length > 0)
        {
            modalObjects[0].object.parent = modalObjects[0].parent;
            modalObjects[0].object.x = modalObjects[0].x0;
            modalObjects[0].object.y = modalObjects[0].y0;
            modalObjects.shift();
        }
    }

    function slotOutsideClicked()
    {
        close(false);
    }
}

