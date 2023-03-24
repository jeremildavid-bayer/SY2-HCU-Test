import QtQuick 2.12

Item {
    // Data
    property bool sameContrasts: false

    // MUDS
    property var fluidSourceMuds
    property string mudsLineFluidSyringeIndex: "NONE"

    // SUDS
    property var fluidSourceSuds

    // SYRINGES
    property var fluidSourceSyringe1
    property var fluidSourceSyringe2
    property var fluidSourceSyringe3
    property var fluidSourceSyringePackages1: []
    property var fluidSourceSyringePackages2: []
    property var fluidSourceSyringePackages3: []

    // BOTTLES
    property var fluidSourceBottle1
    property var fluidSourceBottle2
    property var fluidSourceBottle3
    property var fluidSourceBottlePackages1: []
    property var fluidSourceBottlePackages2: []
    property var fluidSourceBottlePackages3: []
    property var salineSelectItems: []
    property var contrastSelectItems: []

    // OTHER FLUID SRCS
    property var fluidSourceWasteContainer

    // BarcodeData
    property var barcodeInfo
    property bool barcodeReaderConnected: false

    // Function from QML to CPP
    function slotStopcockFillPosition(syringeIdxStr)  { return dsDeviceCpp.slotStopcockFillPosition(syringeIdxStr); }
    function slotStopcockInjectPosition(syringeIdxStr)  { return dsDeviceCpp.slotStopcockInjectPosition(syringeIdxStr); }
    function slotStopcockClosePosition(syringeIdxStr)  { return dsDeviceCpp.slotStopcockClosePosition(syringeIdxStr); }
    function slotBarcodeReaderStart(sleepTimeoutMs) { return dsDeviceCpp.slotBarcodeReaderStart(sleepTimeoutMs); }
    function slotBarcodeReaderStop() { return dsDeviceCpp.slotBarcodeReaderStop(); }
    function slotBarcodeReaderConnect() { return dsDeviceCpp.slotBarcodeReaderConnect(); }
    function slotBarcodeReaderDisconnect() { return dsDeviceCpp.slotBarcodeReaderDisconnect(); }
    function slotBarcodeReaderSetBarcodeData(data)  { return dsDeviceCpp.slotBarcodeReaderSetBarcodeData(data); }
    function slotSyringeAirCheck(syringeIdxStr) { return dsDeviceCpp.slotSyringeAirCheck(syringeIdxStr); }
    function slotGetSyringeSodStartReady(syringeIdxStr)  { return dsDeviceCpp.slotGetSyringeSodStartReady(syringeIdxStr); }

}
