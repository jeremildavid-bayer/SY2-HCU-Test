#ifndef DEVICE_BOTTLE_H
#define DEVICE_BOTTLE_H

#include <QObject>
#include <QTimer>
#include "Common/Common.h"
#include "Common/HwCapabilities.h"
#include "Common/ActionBase.h"
#include "DataServices/Device/DS_DeviceDef.h"
#include "DataServices/Mcu/DS_McuDef.h"

class DeviceBottle : public ActionBase
{
    Q_OBJECT
public:
    explicit DeviceBottle(QObject *parent = 0, EnvGlobal *env_ = NULL, EnvLocal *envLocal_ = NULL, SyringeIdx location = SYRINGE_IDX_SALINE);
    ~DeviceBottle();

    DataServiceActionStatus actLoad(DS_DeviceDef::FluidPackage package, QString actGuid = "");
    DataServiceActionStatus actUnload(QString actGuid = "");
    bool isReady();

private:
    SyringeIdx location;
    bool isActive;
    QTimer tmrUseLifeUpdate;
    QTimer tmrDebounceSpikeStateRead;
    DS_McuDef::BottleBubbleDetectorState lastBubbleDetectorState;
    DS_DeviceDef::FluidSource getFluidSourceSyringe();
    void setFluidSourceBottle(const DS_DeviceDef::FluidSource &data);
    void setInletAirDetectorLed();
    void handleBubbleDetectorStateChanged();
    void handleMudsInsertedChanged();
    void checkSupplyUseTime();

private slots:
    void slotAppInitialised();
    void slotUpdateUsedTimeStatus();
};

#endif // DEVICE_BOTTLE_H
