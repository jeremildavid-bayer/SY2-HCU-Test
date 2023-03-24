import QtQuick 2.12
import "../../Widgets"

GenericButton {
    property int maxWidth: parent.width * 0.62
    property string prevSearchStr: ""
    property int totalRows: 0
    property int searchStrFound: 0
    property var cruLinkStatus: dsCru.cruLinkStatus
    property string screenState: appMain.screenState

    id: root
    radius: 0
    visible: {
        if ( (screenState === "ExamManager-PatientSelection") &&
             (!examPatientSelect.isEntrySelected()) &&
             (cruLinkStatus.State === "Active") )
        {
            return true;
        }
        return false;
    }

    state: "IDLE" // IDLE, EDITING, SET

    color: {
        if (state == "EDITING")
        {
            return colorMap.actionButtonBackground;
        }
        else if (state == "SET")
        {
            return colorMap.keypadButton
        }
        else
        {
            return "transparent";
        }
    }

    width: {
        var widthTotal;

        if ( (state == "EDITING") ||
             (state == "SET") )
        {
            widthTotal = btnDelete.x + btnDelete.width;
        }
        else
        {
            // IDLE, no search text set
            widthTotal = labelSearch.x + labelSearch.width;
        }
        return Math.min(widthTotal, maxWidth);
    }

    Rectangle {
        id: textEditContainer
        color: "transparent"
        width: parent.width
        height: parent.height
    }

    Text {
        id: iconSearch
        width: contentWidth
        height: parent.height
        anchors.left: parent.left
        anchors.leftMargin: maxWidth * 0.02
        font.family: fontIcon.name
        font.pixelSize: parent.height * 0.35
        verticalAlignment: Text.AlignVCenter
        color: {
            if (root.state == "EDITING")
            {
                return colorMap.actionButtonText;
            }
            else
            {
                return colorMap.text01;
            }
        }
        text: "\ue944"
        z: textEditContainer.z + 1
    }

    Text {
        id: labelSearch
        visible: root.state == "IDLE"
        anchors.left: iconSearch.right
        anchors.leftMargin: maxWidth * 0.015
        width: contentWidth + (maxWidth * 0.03)
        height: parent.height
        font.family: fontRobotoLight.name
        font.pixelSize: parent.height * 0.32
        verticalAlignment: Text.AlignVCenter
        text: translate("T_Search")
        color: colorMap.text01
    }

    Text {
        id: textSearch
        visible: (root.state == "EDITING") || (root.state == "SET")
        anchors.left: iconSearch.right
        anchors.leftMargin: maxWidth * 0.015
        width: contentWidth + (maxWidth * 0.03)
        height: parent.height
        font.family: fontRobotoBold.name
        font.pixelSize: parent.height * 0.32
        verticalAlignment: Text.AlignVCenter
        text: searchStr
        color: colorMap.text01
    }

    Text {
        id: textSearchCount
        visible: (root.state == "EDITING") || (root.state == "SET")
        anchors.left: textSearch.right
        anchors.leftMargin: -maxWidth * 0.01
        width: contentWidth
        height: parent.height
        font.family: fontRobotoLight.name
        font.pixelSize: parent.height * 0.26
        verticalAlignment: Text.AlignVCenter
        text: "(" + searchStrFound + " of " + totalRows + ")"
        color: (root.state == "EDITING") ? colorMap.actionButtonText : colorMap.text01
        z: textEditContainer.z + 1
    }

    GenericIconButton {
        id: btnDelete
        visible: (root.state == "EDITING") || (root.state == "SET")
        radius: 0
        anchors.left: textSearchCount.right
        width: height
        height: parent.height
        iconColor: (root.state == "EDITING") ? colorMap.actionButtonText : colorMap.text01
        iconText: "\ue939"
        iconFontPixelSize: parent.height * 0.3
        z: textEditContainer.z + 1
        onBtnClicked: {
            logDebug("ExamPatientSelectSearch: Delete Btn Clicked");
            if (root.state == "EDITING")
            {
                widgetKeyboard.setCurrentValue("");
                widgetKeyboard.close(true);
            }
            else if (root.state == "SET")
            {
                examPatientSelect.searchStr = "";
                slotKeyboardClosed(true);
            }
        }
    }

    onBtnClicked: {
        logDebug("ExamPatientSelectSearch: Search Btn Clicked");
        widgetKeyboard.open(textEditContainer, textSearch, 0, 200);
        widgetKeyboard.signalClosed.connect(slotKeyboardClosed);
        widgetKeyboard.signalValueChanged.connect(slotKeyboardValueChanged);
        root.state = "EDITING";
        prevSearchStr = searchStr;
    }

    Component.onCompleted: {
        titleBar.addCustomWidget(this);
    }

    Component.onDestruction: {
        widgetKeyboard.signalClosed.disconnect(slotKeyboardClosed);
        widgetKeyboard.signalValueChanged.disconnect(slotKeyboardValueChanged);
    }

    function slotKeyboardValueChanged(newValue)
    {
        if (root.state == "EDITING")
        {
            if ( (newValue.length >= searchStr.length) &&
                 (root.width >= maxWidth) )
            {
                logDebug("ExamPatientSelectSearch: Search Text length exceeded limit. TextLen=" + newValue.length);
                widgetKeyboard.setCurrentValue(searchStr);
            }
            else
            {
                //logDebug("ExamPatientSelectSearch: Search Text: " + newValue);
                examPatientSelect.searchStr = newValue;
            }
        }
    }

    function slotKeyboardClosed(modified)
    {
        if (!modified)
        {
            examPatientSelect.searchStr = prevSearchStr;
        }

        if (searchStr == "")
        {
            root.state = "IDLE";
        }
        else
        {
            root.state = "SET";
        }
        textSearch.text = Qt.binding(function() { return searchStr; });
        widgetKeyboard.signalClosed.disconnect(slotKeyboardClosed);
        widgetKeyboard.signalValueChanged.disconnect(slotKeyboardValueChanged);
    }

}
