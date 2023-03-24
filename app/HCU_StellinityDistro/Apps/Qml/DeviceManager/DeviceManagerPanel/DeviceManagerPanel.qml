import QtQuick 2.12
import "../../Widgets"
import "../../Util.js" as Util

Item {
    property alias content: mainRect.children
    property string panelName: ""
    property string titleText: ""
    property string titleIconText: ""
    property string fluidColor: ""
    property string textDisposableState: "T_NotPresent"
    property bool disposableNeedsReplaced: false
    property bool disposableExpired: false
    property string textDisposableState2: ""
    
    property int infoRowHeight: height * 0.07
    property int infoRowSpacing: height * 0.01
    property int actionButtonSpacing: height * 0.05
    property int actionButtonIconWidth: actionButtonWidth * 0.18
    property int actionButtonTextMargin: actionButtonWidth * 0.05
    property int actionButtonWidth: width * 0.8
    property int actionButtonHeight: height * 0.115

    property double disposableInsertedEpochMs: -1
    property int maxUseDurationMs: -1
    property string elapsedTimeText: ""
    property var panelActiveAlerts: []

    visible: activePanelName === panelName
    anchors.fill: parent

    Timer {
        id: elapsedTimer
        interval: 1000
        onTriggered: {
            updateUseTime();
        }
    }

    Text {
        id: titleIcon
        anchors.top: parent.top
        anchors.topMargin: infoRowSpacing
        height: parent.height * 0.06
        width: parent.width * 0.1
        font.family: fontIcon.name
        text: titleIconText
        color: fluidColor
        font.pixelSize: height * 0.7
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    Item {
        id: rectHeader
        anchors.top: parent.top
        anchors.topMargin: infoRowSpacing
        anchors.bottom: parent.bottom
        anchors.left: titleIcon.right
        width: parent.width * 0.9

        Text {
            id: textTitle
            width: parent.width
            height: titleIcon.height
            font.family: fontRobotoBold.name
            text: translate(titleText)
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: titleIcon.font.pixelSize
            color: colorMap.text01
            elide: Text.ElideRight
        }

        Item
        {
            id: titleBottomSpacing
            anchors.top: textTitle.bottom
            height: infoRowSpacing * 2
            width: parent.width
        }

        Item {
            id: rowDisposableState
            anchors.top: titleBottomSpacing.bottom
            width: parent.width
            height: Math.max(iconDisposableWarning.height, txtDisposableState.height)
            clip: true
            WarningIcon {
                id: iconDisposableWarning
                width: disposableExpired ? (parent.width * 0.07) : 0
                height: disposableExpired ? (infoRowHeight * 0.5) : 0
                visible: disposableExpired
                horizontalAlignment: Text.AlignLeft
            }

            Text {
                id: txtDisposableState
                font.family: disposableExpired ? fontRobotoBold.name : fontRobotoLight.name
                anchors.left: iconDisposableWarning.right
                anchors.right: parent.right
                color: {
                    if (disposableExpired)
                    {
                        return colorMap.red;
                    }
                    else if (disposableNeedsReplaced)
                    {
                        return colorMap.deviceIconWarningState;
                    }
                    return colorMap.text02;
                }
                height: (text == "") ? 0 : contentHeight
                font.pixelSize: infoRowHeight * 0.48
                wrapMode: Text.Wrap
                text: {
                    var textBuf = translate(textDisposableState);
                    textBuf = textBuf.replace("\n\n", " ");
                    textBuf = textBuf.replace("\n", " ");
                    return textBuf;
                }
            }
        }

        Item {
            id: rowDisposableState2
            anchors.top: rowDisposableState.bottom
            anchors.topMargin: infoRowSpacing
            width: parent.width
            height: txtDisposableState2.height
            clip: true

            Text {
                id: txtDisposableState2
                font.family: fontRobotoLight.name
                anchors.left: parent.left
                anchors.right: parent.right
                color: colorMap.text02
                height: contentHeight
                font.pixelSize: txtDisposableState.font.pixelSize
                wrapMode: Text.Wrap
                text: {
                    var textBuf = translate(textDisposableState2);
                    textBuf = textBuf.replace("\n\n", " ");
                    textBuf = textBuf.replace("\n", " ");
                    return textBuf;
                }
            }
        }

        ListView {
            id: listViewAlert
            anchors.top: rowDisposableState2.bottom
            anchors.topMargin: infoRowSpacing
            width: parent.width
            height: contentHeight
            model: panelActiveAlerts.length
            interactive: false
            spacing: infoRowSpacing

            delegate: Item {
                clip: true
                height: {
                    var alertCodeName = panelActiveAlerts[index].CodeName;
                    if ( (alertCodeName === "MUDSUseLifeExceeded") ||
                         (alertCodeName === "BulkUseLifeExceeded") )
                    {
                        // Don't display this alert on the panel alert field - Just change the icon state
                        return 0;
                    }
                    return Math.max(iconActiveAlert.height, txtAlert.contentHeight);
                }
                width: ListView.view.width

                WarningIcon {
                    id: iconActiveAlert
                    width: parent.width * 0.07
                    height: infoRowHeight * 0.5
                    horizontalAlignment: Text.AlignLeft
                }

                Text {
                    id: txtAlert
                    anchors.left: iconActiveAlert.right
                    anchors.right: parent.right
                    text: {
                        var alertCodeName = panelActiveAlerts[index].CodeName;
                        var alertData = panelActiveAlerts[index].Data;
                        if (alertData !== "") {
                            alertData = ";" + alertData;
                        }
                        var alertText = "T_" + alertCodeName + "_UserDirection" + alertData;
                        alertText = translate(alertText);
                        alertText.replace("\n\n", " ");
                        alertText = alertText.replace("\n", " ");
                        return alertText;
                    }
                    font.pixelSize: txtDisposableState.font.pixelSize
                    font.family: fontRobotoBold.name
                    color: colorMap.errText
                    wrapMode: Text.Wrap
                }
            }
        }

        Item
        {
            id: alertListBottomSpacing
            anchors.top: listViewAlert.bottom
            height: (panelActiveAlerts.length === 0) ? 0 : infoRowSpacing
            width: parent.width
        }

        Item {
            id: mainRect
            anchors.top: alertListBottomSpacing.bottom
            anchors.topMargin: infoRowSpacing
            anchors.bottom: parent.bottom
            width: parent.width
        }
    }

    onVisibleChanged: {
        if (visible)
        {
            updateUseTime();
        }
        else
        {
            stopElapsedTimer();
        }
    }

    function initElapsedTimer(disposableInsertedEpochMsNew, maxUseDurationMsNew)
    {
        stopElapsedTimer();
        disposableInsertedEpochMs = disposableInsertedEpochMsNew;
        maxUseDurationMs = maxUseDurationMsNew;
        disposableExpired = false;
        disposableNeedsReplaced = false;
    }

    function startElapsedTimer()
    {
        updateUseTime();
    }

    function stopElapsedTimer()
    {
        elapsedTimer.stop();
        disposableExpired = false;
        disposableNeedsReplaced = false;
    }

    function updateUseTime()
    {
        if (disposableInsertedEpochMs === -1)
        {
            textDisposableState = "T_NotPresent";
            stopElapsedTimer();
            return;
        }

        var currentDate = new Date();

        if (maxUseDurationMs === -1)
        {
            var timePastMs = currentDate - disposableInsertedEpochMs;
            elapsedTimeText = Util.durationStrToHumanReadableFormatStr(Util.millisecToDurationStr(timePastMs));
            textDisposableState = "T_Installed_XXX_Ago;" + elapsedTimeText;
            elapsedTimer.start();
            disposableExpired = false;
        }
        else
        {
            var expiresIn = (disposableInsertedEpochMs + maxUseDurationMs) - currentDate;
            if (expiresIn < 0)
            {
                // Expired
                expiresIn = Math.abs(expiresIn);
                elapsedTimeText = Util.durationStrToHumanReadableFormatStr(Util.millisecToDurationStr(expiresIn));
                textDisposableState = "T_Expired_XXX_Ago;" + elapsedTimeText;
                disposableExpired = true;
            }
            else
            {
                elapsedTimeText = Util.durationStrToHumanReadableFormatStr(Util.millisecToDurationStr(expiresIn));
                textDisposableState = "T_DiscardIn_XXX_;" + elapsedTimeText;
                disposableExpired = false;
            }

            elapsedTimer.start();
        }
    }
}
