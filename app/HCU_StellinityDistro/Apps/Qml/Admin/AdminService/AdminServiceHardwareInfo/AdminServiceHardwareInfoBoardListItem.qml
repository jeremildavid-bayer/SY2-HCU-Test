import QtQuick 2.12
import "../../../Widgets"
import "../../../Util.js" as Util

Item {
    property int rowHeight: ListView.view.height * 0.15
    property int rowIndex: index
    property var rowData: configList[rowIndex]
    property string editMode: "NONE" // "NONE", "REVISION", "SERIAL_NUMBER

    width: ListView.view.width
    height: rowHeight

    Text {
        text: rowData.Type
        x: col1x + tableItemMargin
        width: col2x - x - tableItemMargin
        height: parent.height
        font.pixelSize: height * 0.3
        font.family: fontRobotoLight.name
        color: colorMap.text01
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.Wrap
        clip: true
    }

    GenericButton {
        id: btnRevision
        x: col2x + tableItemMargin
        width: col3x - x - tableItemMargin
        anchors.verticalCenter: parent.verticalCenter
        height: parent.height * 0.85
        color: colorMap.editFieldBackground

        content: [
            Text {
                id: textRevision
                text: getRevisionText()
                anchors.fill: parent
                anchors.margins: parent.height * 0.2
                font.pixelSize: height * 0.6
                font.family: fontRobotoBold.name
                verticalAlignment: Text.AlignVCenter
                color: colorMap.text01
                elide: Text.ElideRight
            }
        ]
        onBtnClicked: {
            startEdit("REVISION");
        }

        Component.onCompleted: {
            listViewSettings.dragStarted.connect(reset);
        }

        Component.onDestruction: {
            listViewSettings.dragStarted.disconnect(reset);
        }
    }

    GenericButton {
        id: btnSerialNumber
        x: col3x + tableItemMargin
        width: parent.width - x - tableItemMargin
        anchors.verticalCenter: parent.verticalCenter
        height: parent.height * 0.85
        color: colorMap.editFieldBackground
        enabled: !rowData.SerialNumberReadOnly

        content: [
            Text {
                id: textSerialNumber
                text: getSerialNumberText()
                anchors.fill: parent
                anchors.margins: parent.height * 0.2
                font.pixelSize: height * 0.6
                font.family: fontRobotoBold.name
                verticalAlignment: Text.AlignVCenter
                color: colorMap.text01
                elide: Text.ElideRight
            }
        ]

        onBtnClicked: {
            startEdit("SERIAL_NUMBER");
        }

        Component.onCompleted: {
            listViewSettings.dragStarted.connect(reset);
        }

        Component.onDestruction: {
            listViewSettings.dragStarted.disconnect(reset);
        }
    }

    Component.onCompleted: {
    }

    Component.onDestruction: {
        widgetKeyboard.signalClosed.disconnect(slotKeyboardClosed);
        widgetKeyboard.signalValueChanged.disconnect(slotKeyboardValueChanged);
    }

    function startEdit(newEditMode)
    {
        if (widgetKeyboard.isOpen())
        {
            widgetKeyboard.close(true);
        }

        editMode = newEditMode;

        var btnEdit;

        if (editMode === "REVISION")
        {
            btnEdit = btnRevision;
        }
        else if (editMode === "SERIAL_NUMBER")
        {
            btnEdit = btnSerialNumber;
        }
        else
        {
            return;
        }

        // Scroll list to let edit button stay out from keyboard
        var scrollDistance = widgetKeyboard.getShiftDistance(btnEdit);

        if (scrollDistance !== 0)
        {
            // Manually scroll the listview to locate the edit field on top of the keyboard
            scrollDistance -= ListView.view.y;
            tmrKeyboardOpen.triggeredOnStart = false;
            listViewSettings.flick(0, scrollDistance * 2.7);
        }
        else
        {
            tmrKeyboardOpen.triggeredOnStart = true;
        }

        tmrKeyboardOpen.stop();
        tmrKeyboardOpen.start();
    }

    Timer {
        id: tmrKeyboardOpen
        interval: 10
        onTriggered: {
            if (listViewSettings.flicking)
            {
                // List is in dragging state to move the edit button out from the keyboard area
                tmrKeyboardOpen.start();
            }
            else
            {
                var btnEdit, textEdit, cfg;

                if (editMode === "REVISION")
                {
                    btnEdit = btnRevision;
                    textEdit = textRevision;
                    cfg = rowData.Revision;
                }
                else if (editMode === "SERIAL_NUMBER")
                {
                    btnEdit = btnSerialNumber;
                    textEdit = textSerialNumber;
                    cfg = rowData.SerialNumber;
                }

                widgetKeyboard.open(btnEdit, textEdit, cfg.ValidRange.LowerLimitInclusive, cfg.ValidRange.UpperLimitInclusive);
                widgetKeyboard.signalClosed.connect(slotKeyboardClosed);
                widgetKeyboard.signalValueChanged.connect(slotKeyboardValueChanged);
            }
        }
    }

    function slotKeyboardValueChanged(newValue)
    {
        if (editMode == "REVISION")
        {
            if (widgetKeyboard.currentValue.length > 0)
            {
                // make sure first char is always upper case
                var valStr = newValue.substring(0, 1).toUpperCase() + newValue.substring(1, newValue.length);
                widgetKeyboard.setCurrentValue(valStr);
            }
        }
    }

    function slotKeyboardClosed(modified)
    {
        widgetKeyboard.signalClosed.disconnect(slotKeyboardClosed);
        widgetKeyboard.signalValueChanged.disconnect(slotKeyboardValueChanged);

        if (editMode == "REVISION")
        {
            textRevision.text = Qt.binding(function() { return getRevisionText(); });
        }
        else if (editMode == "SERIAL_NUMBER")
        {
            textSerialNumber.text = Qt.binding(function() { return getSerialNumberText(); });
        }
        else
        {
            return;
        }

        if (modified)
        {
            var oldConfig, newConfig;
            if (editMode == "REVISION")
            {
                oldConfig = Util.copyObject(rowData.Revision);
                newConfig = rowData.Revision;
            }
            else if (editMode == "SERIAL_NUMBER")
            {
                oldConfig = Util.copyObject(rowData.SerialNumber);
                newConfig = rowData.SerialNumber;
            }

            newConfig.Value = (widgetKeyboard.currentValue === "--") ? "" : widgetKeyboard.currentValue;

            if (newConfig.Value !== oldConfig.Value)
            {
                logDebug("Saving Board Info: " + editMode + " : value changed: " + oldConfig.Value + " -> " + newConfig.Value);
                dsHardwareInfo.slotConfigChanged(newConfig);
            }
            else
            {
                logDebug("Saving Board Info: " + editMode + " : value not changed: " + newConfig.Value);
            }

            editMode = "NONE";
        }
    }

    function getRevisionText()
    {
        var textVal = rowData.Revision.Value;

        if (textVal === "")
        {
            return "--";
        }
        return textVal;
    }

    function getSerialNumberText()
    {
        var textVal = rowData.SerialNumber.Value;

        if (textVal === "")
        {
            return "--";
        }
        return textVal;
    }
}
