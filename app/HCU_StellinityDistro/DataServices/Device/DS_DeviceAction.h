#ifndef DS_DEVICE_ACTION_H
#define DS_DEVICE_ACTION_H

#include "Common/Common.h"
#include "Common/ActionBase.h"
#include "DS_DeviceDef.h"
#include "Internal/DeviceMonitor.h"
#include "Internal/DeviceBottle.h"
#include "Internal/DeviceSyringe.h"
#include "Internal/DeviceLeds.h"
#include "Internal/DeviceMuds.h"
#include "Internal/DeviceStopcock.h"
#include "Internal/DeviceSuds.h"
#include "Internal/DeviceBarcodeReader.h"
#include "Internal/DeviceOutletAirDoor.h"
#include "Internal/DeviceWasteContainer.h"
#include "Internal/DeviceHeatMaintainer.h"
#include "Internal/DeviceDoor.h"
#include "Internal/DeviceBattery.h"

class DS_DeviceAction : public ActionBase
{
    Q_OBJECT

public:
    explicit DS_DeviceAction(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~DS_DeviceAction();

    DataServiceActionStatus actMudsInit(QString actGuid = "");
    DataServiceActionStatus actMudsSodStart(QString actGuid = "");
    DataServiceActionStatus actMudsSodAbort(QString actGuid = "");
    DataServiceActionStatus actMudsFindPlungerAll(QString actGuid = "");
    DataServiceActionStatus actMudsPurgeAirAll(QString actGuid = "");
    DataServiceActionStatus actMudsPurgeFluid(QList<bool> purgeSyringe, QString actGuid = "");
    DataServiceActionStatus actMudsEngageAll(QString actGuid = "");
    DataServiceActionStatus actMudsDisengageAll(QString actGuid = "");
    DataServiceActionStatus actMudsPreloadPrime(const DS_McuDef::InjectionProtocol &injectionProtocol, QString actGuid = "");

    DataServiceActionStatus actBottleLoad(SyringeIdx bottleIdx, DS_DeviceDef::FluidPackage package, QString actGuid = "");
    DataServiceActionStatus actBottleUnload(SyringeIdx bottleIdx, QString actGuid = "");
    DataServiceActionStatus actBottlesUnload(QString actGuid = "");

    DataServiceActionStatus actSyringeFill(SyringeIdx syringeIdx, bool preAirCheckEnabled = false, QString actGuid = "");
    DataServiceActionStatus actSyringePrime(SyringeIdx syringeIdx, DS_DeviceDef::SudsPrimeType sudsPrimeType = DS_DeviceDef::SUDS_PRIME_TYPE_AUTO, QString actGuid = "");
    DataServiceActionStatus actSyringePrime(const DS_McuDef::ActPrimeParams &primeParams, QString actGuid = "");
    DataServiceActionStatus actSyringeSodStart(SyringeIdx syringeIdx, QString actGuid = "");
    DataServiceActionStatus actSyringeSodAbort(SyringeIdx syringeIdx, QString actGuid = "");
    DataServiceActionStatus actSyringeAirCheck(SyringeIdx syringeIdx, QString actGuid = "");

    DataServiceActionStatus actSudsPrimeInit(SyringeIdx syringeIdx = SYRINGE_IDX_SALINE, QString actGuid = "");
    DataServiceActionStatus actSudsPrimeUpdateState(SyringeIdx syringeIdx = SYRINGE_IDX_SALINE, QString actGuid = "");
    DataServiceActionStatus actSudsAutoPrime(QString actGuid = "");
    DataServiceActionStatus actSudsManualPrime(QString actGuid = "");
    DataServiceActionStatus actSudsSetNeedsReplace(bool needed = true, QString actGuid = "");
    DataServiceActionStatus actSudsSetNeedsPrime(bool needed = true, QString actGuid = "");
    DataServiceActionStatus actSudsSetActiveContrastSyringe(SyringeIdx syringeIdx, QString actGuid = "");

    DataServiceActionStatus actStopcock(DS_McuDef::StopcockPos pos1, DS_McuDef::StopcockPos pos2, DS_McuDef::StopcockPos pos3, int retryLimit = STOPCOCK_ACTION_TRIALS_LIMIT, QString actGuid = "");
    DataServiceActionStatus actStopcock(SyringeIdx location, DS_McuDef::StopcockPos pos, int retryLimit = STOPCOCK_ACTION_TRIALS_LIMIT, QString actGuid = "");
    DataServiceActionStatus actStopcock(DS_McuDef::ActStopcockParams params, int retryLimit = STOPCOCK_ACTION_TRIALS_LIMIT, QString actGuid = "");

    DataServiceActionStatus actBarcodeReaderConnect(QString actGuid = "");
    DataServiceActionStatus actBarcodeReaderDisconnect(QString actGuid = "");
    DataServiceActionStatus actBarcodeReaderStart(int sleepTimeoutMs = BARCODE_READER_SLEEP_TIMEOUT_MS, QString actGuid = "");
    DataServiceActionStatus actBarcodeReaderStop(QString actGuid = "");
    DataServiceActionStatus actBarcodeReaderSetBarcodeData(QString data, QString actGuid = "");

    DataServiceActionStatus actLeds(LedIndex index, DS_McuDef::ActLedParams param, QString actGuid = "");
    DataServiceActionStatus actCopyLeds(LedIndex from, LedIndex to, QString actGuid = "");

    DataServiceActionStatus actDeactivateReservoirAlertsOnEmptied(SyringeIdx syringeIdx, QString actGuid = "");


private:
    struct StopcockActionStatus
    {
        QString actGuid[SYRINGE_IDX_MAX];
        bool started[SYRINGE_IDX_MAX];
        bool completed[SYRINGE_IDX_MAX];
    };

    EnvLocal *envDeviceBottle;
    EnvLocal *envDeviceSyringe;
    EnvLocal *envDeviceStopcock;

    bool deviceActive;

    DeviceMonitor *monitor;
    DeviceBottle *devBottles[SYRINGE_IDX_MAX];
    DeviceSyringe *devSyringes[SYRINGE_IDX_MAX];
    DeviceStopcock *devStopcocks[SYRINGE_IDX_MAX];

    QMap<QString, StopcockActionStatus> stopcockActionStatusMap;
    DeviceLeds *devLed;
    DeviceMuds *devMuds;
    DeviceSuds *devSuds;
    DeviceBarcodeReader *devBarcodeReader;
    DeviceOutletAirDoor *devOutletAirDoor;
    DeviceWasteContainer *devWasteContainer;
    DeviceHeatMaintainer *devHeatMaintainer;
    DeviceDoor *devDoor;
    DeviceBattery *devBattery;

private slots:
    void slotAppInitialised();
};

#endif // DS_DEVICE_ACTION_H
