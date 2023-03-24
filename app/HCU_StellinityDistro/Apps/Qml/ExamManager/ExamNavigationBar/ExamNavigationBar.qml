import QtQuick 2.12
import "../../Widgets"
import "../../DeviceManager"
import "../../Util.js" as Util

Rectangle {
    property string examProgressState: dsExam.examProgressState
    property string examScreenState: dsExam.examScreenState
    property string screenState: appMain.screenState
    property var executedSteps: dsExam.executedSteps
    property var executingStep: dsExam.executingStep
    property var plan: dsExam.plan
    property alias btnDeviceManager: btnDeviceManager
    property int animationMs: 400
    property var examAdvanceInfo: dsExam.examAdvanceInfo
    property bool licenseEnabledPatientStudyContext: dsCru.licenseEnabledPatientStudyContext
    property var cruLinkStatus: dsCru.cruLinkStatus
    property bool isExamStarted: dsExam.isExamStarted
    property int refreshButtonDebounceTimerMs: dsCapabilities.defaultButtonDebounceTimerMs

    // external modules disabling the button. each module is responsible for incrementing/decrementing this counter correctly
    property int disableNavNextButtonCount: 0

    onDisableNavNextButtonCountChanged:
    {
        if (disableNavNextButtonCount < 0)
        {
            logError("ExamNavigationBar: disableNavNextButtonCount is less than 0!")
            disableNavNextButtonCount = 0;
        }
    }

    signal signalNavNext();
    signal signalNavPatient();
    signal signalNavProtocol();
    signal signalNavInjection();
    signal signalNavSummary();

    id: root
    width: parent.width
    height: parent.height
    color: colorMap.actionBarBackground

    GenericButton {
        id: btnDeviceManager
        x: parent.width * 0.02
        y: parent.height * 0.1
        width: parent.width * 0.0854
        height: parent.height * 0.82

        disabledColor: "transparent"

        Item {
            id: rectDeviceManagerIcon
            anchors.fill: parent
        }

        onBtnClicked: {
            appMain.setScreenState("DeviceManager-Muds");
        }
    }

    WarningIcon {
        id: itemActiveAlertIcon
        height: parent.height * 0.15
        width: height
        anchors.right: btnDeviceManager.right
        anchors.rightMargin: -width / 2
        anchors.top: btnDeviceManager.top
        anchors.topMargin: -height / 2
        visible: dsAlert.showDeviceAlertIcon
    }

    ExamNavigationBarGroup {
        anchors.centerIn: parent
        width: Util.getPixelH(800)
        height: Util.getPixelV(203)

        onSignalNavPatient: {
            root.signalNavPatient();
        }

        onSignalNavProtocol: {
            root.signalNavProtocol();
        }

        onSignalNavInjection: {
            root.signalNavInjection();
        }

        onSignalNavSummary: {
            root.signalNavSummary();
        }
    }

    GenericButton {
        property bool isNavigationBtnType: {
            return (textBtnNext.text !== translate("T_Refresh"));
        }

        id: btnNext
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width * 0.2
        height: parent.height * 0.6
        x: btnNext.isNavigationBtnType ? (parent.width - width + radius) : (parent.width - width - (radius * 4))
        color: btnNext.isNavigationBtnType ? colorMap.actionButtonBackground : colorMap.keypadButton
        enabled: isEnabled()

        Text {
            id: textBtnNext
            anchors.left: parent.left
            anchors.right: iconNextBtnText.left
            padding: parent.width * 0.01
            height: parent.height
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.family: fontRobotoBold.name
            font.pixelSize: parent.height * 0.27
            minimumPixelSize: font.pixelSize * 0.24
            fontSizeMode: Text.Fit
            wrapMode: Text.Wrap
            color: btnNext.isNavigationBtnType ? colorMap.actionButtonText : colorMap.text01
            text: {
                var nextBtnText = "";
                if (screenState === "ExamManager-PatientSelection")
                {
                    if ( (cruLinkStatus.State === "Active") &&
                         (!patientSelect.isEntrySelected()) )
                    {
                        nextBtnText = "T_Refresh";
                    }
                    else if ( (!isExamStarted) &&
                              (cruLinkStatus.State === "Active") )
                    {
                        nextBtnText = "T_StartExam";
                    }
                    else
                    {
                        nextBtnText = "T_Next";
                    }
                }
                else if (screenState === "ExamManager-ProtocolSelection")
                {
                    nextBtnText = "T_Select";
                }
                else if (screenState === "ExamManager-ProtocolModification")
                {
                    nextBtnText = (executingStep === undefined) ? "T_Finish" : "T_Arm";
                }
                else if (screenState === "ExamManager-SummaryConfirmation")
                {
                    nextBtnText = isExamStarted ? "T_EndExam" : "T_EndExamNotStarted";
                }
                return translate(nextBtnText);
            }
        }

        Text {
            id: iconNextBtnText
            width: contentWidth
            anchors.right: parent.right
            anchors.rightMargin: width * 0.5
            height: parent.height
            verticalAlignment: Text.AlignVCenter
            color: btnNext.isNavigationBtnType ? colorMap.actionButtonText : colorMap.text01
            font.family: fontIcon.name
            font.pixelSize:  height * 0.3
            text: btnNext.isNavigationBtnType ? "\ue908" : ""
        }

        pressedSoundCallback: function(){ soundPlayer.playNext(); }
        onBtnClicked: {
            logDebug("ExamNavigationBar: btnNext(" + textBtnNext.text + ") Clicked");
            signalNavNext();

            // if it is refresh button, disable it shortly to avoid spamming
            if (!btnNext.isNavigationBtnType)
            {
                btnNext.enabled = false;
                // disabling for 500 ms
                timerSingleShot(refreshButtonDebounceTimerMs, function() {
                    btnNext.enabled = Qt.binding(isEnabled);
                });
            }
        }

        function isEnabled()
        {
            if ( (widgetInputPad.isOpen()) ||
                 (widgetKeyboard.isOpen()) )
            {
                return false;
            }
            else if (disableNavNextButtonCount > 0)
            {
                return false;

            }
            else if (screenState === "ExamManager-PatientSelection")
            {
            }
            else if (screenState === "ExamManager-ProtocolSelection")
            {
                if (executedSteps.length > 0)
                {
                    return false;
                }
            }
            else if (screenState === "ExamManager-SummaryConfirmation")
            {
                if ( (licenseEnabledPatientStudyContext) &&
                     (cruLinkStatus.State === "Active") &&
                     (isExamStarted) &&
                     (executedSteps.length > 0) &&
                     (!examAdvanceInfo.MandatoryFieldsEntered) )
                {
                    // Mandatory fields needs to be filled while CRU link is active AND PatientStudyContext is enabled
                    // AND at least one injection was executed, or the 'End Exam' button is disabled.
                    return false;
                }
            }
            return true;
        }
    }

    SequentialAnimation {
        id: animationActivated
        NumberAnimation { target: root; properties: 'y'; from: height; to: 0; duration: animationMs; easing.type: Easing.InOutQuart }
    }

    onPlanChanged: {
        reload();
    }

    onExamProgressStateChanged: {
        if (examScreenState == "Home")
        {
            var examStarted = false;
            if (dsCru.licenseEnabledPatientStudyContext)
            {
                examStarted = (examProgressState == "PatientSelection");
            }
            else
            {
                examStarted = (examProgressState == "ProtocolSelection") || (examProgressState == "ProtocolModification");
            }

            if (examStarted)
            {
                animationActivated.start();
            }
        }
    }

    onVisibleChanged: {
        reload();
    }

    onScreenStateChanged: {
        visible = ( (screenState === "ExamManager-PatientSelection") ||
                    (screenState === "ExamManager-ProtocolSelection") ||
                    (screenState === "ExamManager-ProtocolModification") ||
                    (screenState === "ExamManager-SummaryConfirmation") );
    }

    function reload()
    {
        if (!visible)
        {
            return;
        }

        deviceManagerIcon.visible = true;
        deviceManagerIcon.parent = rectDeviceManagerIcon;
        deviceManagerIcon.displaySourcePackageInfo = false;

        btnNext.reset();
    }
}
