import QtQuick 2.12
import "../Widgets"
import "../DeviceManager"
import "../ExamManager/ExamNavigationBar"
import "../Util.js" as Util

Rectangle {
    property var activeSystemAlerts: dsAlert.activeSystemAlerts
    property string statePath: dsSystem.statePath
    property string examScreenState: dsExam.examScreenState
    property string mcuLinkState: dsMcu.mcuLinkState
    property bool licenseEnabledPatientStudyContext: dsCru.licenseEnabledPatientStudyContext
    property string uiTheme: dsCfgLocal.uiTheme
    property int buttonWidth: width * 0.29
    property int buttonHeight: height * 0.7

    id: root
    visible: false
    color: colorMap.homeBackground

    Row {
        id: rowMainBtns
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: -(actionBar.height / 2)
        spacing: parent.width * 0.03

        Item {
            height: buttonHeight
            width: buttonWidth

            GenericButton {
                id: btnDeviceManager
                anchors.fill: parent
                enabled: isOperationBtnEnabled()
                color: colorMap.homeMenuBackground

                content: [
                    Text {
                        y: parent.height * 0.07
                        width: parent.width
                        height: parent.height * 0.06
                        text: translate("T_Injector")
                        color: colorMap.text01
                        font.family: fontRobotoLight.name
                        font.pixelSize: height
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                    },

                    Rectangle {
                        id: rectDeviceManagerIcon
                        anchors.fill: parent
                        color: btnDeviceManager.color
                        anchors.topMargin: parent.height * 0.18
                        anchors.bottomMargin: parent.height * 0.15
                        anchors.leftMargin: parent.width * 0.2
                        anchors.rightMargin: parent.width * 0.2
                    }
                ]

                onBtnClicked: {
                    appMain.setScreenState("DeviceManager-Muds");
                }
            }

            WarningIcon {
                id: itemActiveAlertIcon
                width: height
                height: parent.height * 0.068
                anchors.right: btnDeviceManager.right
                anchors.top: btnDeviceManager.top
                anchors.rightMargin: -width / 2
                anchors.topMargin: -height / 2
                visible: dsAlert.showDeviceAlertIcon
            }
        }

        GenericButton {
            id: btnExamManager
            enabled: isOperationBtnEnabled()
            height: buttonHeight
            width: buttonWidth
            color: colorMap.homeMenuBackground

            content: [
                Text {
                    y: parent.height * 0.08
                    width: parent.width
                    height: parent.height * 0.06
                    text: translate("T_Exam")
                    color: colorMap.text01
                    font.family: fontRobotoLight.name
                    font.pixelSize: height
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                },
                Image {
                    source: getImageSource()
                    sourceSize.width: parent.width * 0.8
                    sourceSize.height: parent.height * 0.4
                    height: sourceSize.height
                    width: sourceSize.width

                    anchors.horizontalCenter: parent.horizontalCenter
                    y: parent.height * 0.25

                    function getImageSource()
                    {
                        if (examScreenState == "ExamManager-PatientSelection")
                        {
                            return imageMap.homeExam1;
                        }
                        else if (examScreenState == "ExamManager-ProtocolSelection")
                        {
                            return imageMap.homeExam2;
                        }
                        else if (examScreenState == "ExamManager-ProtocolModification")
                        {
                            return imageMap.homeExam3;
                        }
                        else if (examScreenState == "ExamManager-SummaryConfirmation")
                        {
                            return imageMap.homeExam4;
                        }
                        else
                        {
                            if (licenseEnabledPatientStudyContext)
                            {
                                return imageMap.homeExam1;
                            }
                            else
                            {
                                return imageMap.homeExam2;
                            }
                        }
                    }
                },

                ExamNavigationBarGroup {
                    y: parent.height * 0.78
                    width: parent.width * 0.8
                    height: parent.height * 0.16
                    anchors.horizontalCenter: parent.horizontalCenter
                    minimised: true
                }
            ]

            onBtnClicked: {
                appMain.setScreenState("ExamManager-*");
            }
        }

        GenericButton {
            id: btnAdminManager
            height: buttonHeight
            width: buttonWidth
            color: colorMap.homeMenuBackground

            content: [
                Text {
                    y: parent.height * 0.08
                    width: parent.width
                    height: parent.height * 0.06
                    text: translate("T_Admin")
                    color: colorMap.text01
                    font.family: fontRobotoLight.name
                    font.pixelSize: height
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                },
                Image {
                    source: imageMap.homeAdmin
                    sourceSize.width: parent.width * 0.72
                    sourceSize.height: parent.height * 0.55
                    height: sourceSize.height
                    width: sourceSize.width
                    anchors.horizontalCenter: parent.horizontalCenter
                    y: parent.height * 0.28

                }
            ]

            onBtnClicked: {
                appMain.setScreenState("Admin-Select");
            }
        }
    }

    HomeActionBar {
        id: actionBar
        Component.onCompleted: {
            this.parent = frameActionBar;
        }
    }

    onUiThemeChanged: {
        var themeName;
        if (uiTheme == "twilight")
        {
            colorMap = themes.colorMapTwilight;
            themeName = "Twilight";
        }
        else
        {
            colorMap = themes.colorMapPurity;
            themeName = "Purity";
        }
        imageMap.setTheme(themeName);
    }

    onVisibleChanged: {
        if (visible)
        {
            deviceManagerIcon.visible = true;
            deviceManagerIcon.parent = rectDeviceManagerIcon;
            deviceManagerIcon.displaySourcePackageInfo = true;
            deviceManagerIcon.rootBackgroundColor = Qt.binding(function() { return rectDeviceManagerIcon.color; });
        }
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
        visible = (appMain.screenState === "Home");
        reload();
    }

    function reload()
    {
        if (!visible)
        {
            return;
        }
    }

    function isOperationBtnEnabled()
    {
        if (hcuBuildType == "PROD")
        {
            return false;
        }
        else if (statePath == "Error")
        {
            return false;
        }
        else if ( ( (hcuBuildType == "VNV") || (hcuBuildType == "REL") ) &&
                  (mcuLinkState !== "CONNECTED") )
        {
            return false;
        }
        return true;
    }
}
