import QtQuick 2.12
import "../../../Widgets"


AdminServiceToolTestTemplate {
    property bool barcodeReaderConnected: dsDevice.barcodeReaderConnected
    property var barcodeInfo: dsDevice.barcodeInfo

    activeScreenState: "Admin-Service-Tool-BarcodeReader"

    paramsTable: [
        AdminServiceToolTestTableRow {
            titleText: "Connect Reader"
            initToggledValue: barcodeReaderConnected
            valueText: "Connect"
            controlType: "BUTTON"
            onBtnClicked: {
                dsDevice.slotBarcodeReaderConnect();
            }
        },
        AdminServiceToolTestTableRow {
            titleText: "Disconnect Reader"
            initToggledValue: barcodeReaderConnected
            valueText: "Disconnect"
            controlType: "BUTTON"
            onBtnClicked: {
                dsDevice.slotBarcodeReaderDisconnect();
            }
        }
    ]

    Row {
        visible: !barcodeReaderConnected
        height: parent.height * 0.1
        width: parent.width * 0.5
        anchors.bottom: parent.bottom
        anchors.bottomMargin: parent.height * 0.02
        spacing: parent.width * 0.01

        WarningIcon {
            height: parent.height * 0.6
            anchors.verticalCenter: parent.verticalCenter
        }

        Text {
            height: parent.height
            font.pixelSize: height * 0.36
            font.family: fontRobotoBold.name
            verticalAlignment: Text.AlignVCenter
            color: colorMap.errText
            wrapMode: Text.Wrap
            text: "Barcode Reader is not configured properly"
        }
    }

    onTestStarted: {
        dsDevice.slotBarcodeReaderStart(0);
    }

    onTestStopped: {
        dsDevice.slotBarcodeReaderStop();
    }

    onBarcodeInfoChanged: {
        if (!visible)
        {
            return;
        }

        if (barcodeInfo === undefined)
        {
            return;
        }

        consoleText.append("BarcodePrefix: " + JSON.stringify(barcodeInfo, null, " "));
        trimVisibleLog();
        stopTest();
    }
}
