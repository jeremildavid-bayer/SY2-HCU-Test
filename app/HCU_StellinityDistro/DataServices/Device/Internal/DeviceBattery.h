#ifndef DEVICE_BATTERY_H
#define DEVICE_BATTERY_H

#include <QObject>
#include "Common/Common.h"
#include "Common/ActionBase.h"

class DeviceBattery : public ActionBase
{
    Q_OBJECT
public:
    explicit DeviceBattery(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~DeviceBattery();

private:
    QTimer tmrBMSDigestMonitor;
    bool batteriesUnsealed;
    bool bmsCommFaultA;
    bool bmsCommFaultB;
    void updateBMSDigestPolling();
    void updateBatteryManagementFaultAlerts(const DS_McuDef::BMSDigests &bmsDigests, const DS_McuDef::BMSDigests &prevBmsDigests);
    void updateTemperatureAlerts(const DS_McuDef::BMSDigests &bmsDigests, const DS_McuDef::BMSDigests &prevBmsDigests);
    void updateCycleCountAlerts(const DS_McuDef::BMSDigests &bmsDigests, const DS_McuDef::BMSDigests &prevBmsDigests);
    void updateStateOfHealthAlerts(const DS_McuDef::BMSDigests &bmsDigests, const DS_McuDef::BMSDigests &prevBmsDigests);
    void updatePermanentFailureAlerts(const DS_McuDef::BMSDigests &bmsDigests, const DS_McuDef::BMSDigests &prevBmsDigests);

    // InjectorBatteryManagementFault
    bool activateInjectorBatteryManagementFaultAlert(QString alertData);
    bool deactivateInjectorBatteryManagementFaultAlert(QString alertData);

    // InjectorBatteryManagementError
    bool activateInjectorBatteryManagementErrorAlert(QString alertData);
    bool deactivateInjectorBatteryManagementErrorAlert(QString alertData);
    void unsealBatteries(const DS_McuDef::BMSDigests &bmsDigest);
    void repairBMSPermanentFailureStatus(const DS_McuDef::BMSDigests &bmsDigests);

private slots:
    void slotAppInitialised();

};

#endif // DEVICE_BATTERY_H
