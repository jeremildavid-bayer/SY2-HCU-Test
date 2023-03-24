import QtQuick 2.12
import "../../Util.js" as Util

Item {
    property string examScreenState: dsExam.examScreenState
    property int itemWidth: root.width * 0.25
    property int itemHeight: root.height
    property int itemCount: dsCru.licenseEnabledPatientStudyContext ? 4 : 3
    property bool minimised: false
    property int selectedAnimation: 200

    signal signalNavPatient()
    signal signalNavProtocol()
    signal signalNavInjection()
    signal signalNavSummary()

    id: root

    Rectangle {
        id: selectedBorder
        width: itemWidth
        height: itemHeight
        color: "transparent"
        border.color: colorMap.navBtnSelectedBorder
        border.width: width * 0.025
        radius: width * 0.06

        Behavior on x {
            NumberAnimation {
                duration: selectedAnimation
            }
        }
    }

    Row {
        width: parent.width
        height: parent.height
        spacing: (width - (itemCount * itemWidth)) / (itemCount - 1)

        ExamNavigationBarItem {
            id: btnPatientSelect
            visibleCondition: dsCru.licenseEnabledPatientStudyContext
            width: itemWidth
            height: itemHeight
            label: "T_Patient"
            onBtnClicked: {
                signalNavPatient();
            }
            onXChanged: {
                reload();
            }
        }

        ExamNavigationBarItem {
            id: btnProtocol
            width: itemWidth
            height: itemHeight
            label: "T_Protocols"
            onBtnClicked: {
                signalNavProtocol();
            }
            onXChanged: {
                reload();
            }
        }

        ExamNavigationBarItem {
            id: btnInjection
            width: itemWidth
            height: itemHeight
            label: "T_Injection"
            onBtnClicked: {
                signalNavInjection();
            }
            onXChanged: {
                reload();
            }
        }

        ExamNavigationBarItem {
            id: btnSummary
            width: itemWidth
            height: itemHeight
            label: "T_Summary"
            onBtnClicked: {
                signalNavSummary();
            }
            onXChanged: {
                reload();
            }
        }
    }

    onExamScreenStateChanged: {
        reload();
    }

    onVisibleChanged: {
        reload();
    }

    Component.onCompleted: {
        appMain.screenStateChanged.connect(slotScreenStateChanged);
    }

    Component.onDestruction: {
        appMain.screenStateChanged.disconnect(slotScreenStateChanged);
    }

    function slotScreenStateChanged()
    {
        visible = ( (appMain.screenState === "Home") ||
                    (appMain.screenState === "ExamManager-PatientSelection") ||
                    (appMain.screenState === "ExamManager-ProtocolSelection") ||
                    (appMain.screenState === "ExamManager-ProtocolModification") ||
                    (appMain.screenState === "ExamManager-SummaryConfirmation") );
    }

    function reload()
    {
        if (!visible)
        {
            return;
        }

        reloadNavigation();
    }

    function reloadNavigation()
    {
        if ( (examScreenState == "Home") ||
             (examScreenState == "OFF") )
        {
            btnPatientSelect.btnState = "DISABLED";
            btnProtocol.btnState = "DISABLED";
            btnInjection.btnState = "DISABLED";
            btnSummary.btnState = "DISABLED";
            selectedBorder.visible = false;
        }
        else if (examScreenState == "ExamManager-PatientSelection")
        {
            btnPatientSelect.btnState = "SELECTED";
            btnProtocol.btnState = "ENABLED";
            btnInjection.btnState = "ENABLED";
            btnSummary.btnState = "ENABLED";
            selectedBorder.visible = true;
            selectedBorder.x = selectedBorder.parent.mapFromItem(btnPatientSelect, 0, 0).x;
        }
        else if (examScreenState == "ExamManager-ProtocolSelection")
        {
            btnPatientSelect.btnState = "ENABLED";
            btnProtocol.btnState = "SELECTED";
            btnInjection.btnState = "ENABLED";
            btnSummary.btnState = "ENABLED";
            selectedBorder.visible = true;
            selectedBorder.x = selectedBorder.parent.mapFromItem(btnProtocol, 0, 0).x;
        }
        else if ( (examScreenState == "ExamManager-ProtocolModification") ||
                  (examScreenState == "ExamManager-InjectionExecution") )
        {
            btnPatientSelect.btnState = "ENABLED";
            btnProtocol.btnState = "ENABLED";
            btnInjection.btnState = "SELECTED";
            btnSummary.btnState = "ENABLED";
            selectedBorder.visible = true;
            selectedBorder.x = selectedBorder.parent.mapFromItem(btnInjection, 0, 0).x;
        }
        else if (examScreenState == "ExamManager-SummaryConfirmation")
        {
            btnPatientSelect.btnState = "ENABLED";
            btnProtocol.btnState = "ENABLED";
            btnInjection.btnState = "ENABLED";
            btnSummary.btnState = "SELECTED";
            selectedBorder.visible = true;
            selectedBorder.x = selectedBorder.parent.mapFromItem(btnSummary, 0, 0).x;
        }
    }
}
