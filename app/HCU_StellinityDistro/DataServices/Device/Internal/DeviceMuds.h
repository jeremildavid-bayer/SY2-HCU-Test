#ifndef DEVICE_MUDS_H
#define DEVICE_MUDS_H

#include "Common/ActionBase.h"
#include "DataServices/Device/DS_DeviceDef.h"
#include "DeviceMudsDisengage.h"
#include "DeviceMudsPreload.h"
#include "DeviceMudsSod.h"

class DeviceMuds : public ActionBase
{
    Q_OBJECT
public:
    explicit DeviceMuds(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~DeviceMuds();

    DataServiceActionStatus actMudsInit(QString actGuid = "");
    DataServiceActionStatus actMudsFindPlungerAll(QString actGuid = "");
    DataServiceActionStatus actMudsPurgeAirAll(QString actGuid = "");
    DataServiceActionStatus actMudsPurgeFluid(QList<bool> purgeSyringe, QString actGuid = "");
    DataServiceActionStatus actMudsEngageAll(QString actGuid = "");
    DataServiceActionStatus actMudsDisengageAll(int retryLimit = DISENGAGE_ACTION_TRIALS_LIMIT, QString actGuid = "");
    DataServiceActionStatus actMudsPreload(const DS_McuDef::InjectionProtocol &injectionProtocol, QString actGuid = "");
    DataServiceActionStatus actMudsSodStart(QString actGuid = "");
    DataServiceActionStatus actMudsSodAbort(QString actGuid = "");

private:
    bool isActive;

    DeviceMudsDisengage *deviceMudsDisengage;
    DeviceMudsSod *deviceMudsSod;
    DeviceMudsPreload * deviceMudsPreload;

    QTimer tmrUseLifeUpdate;

    bool isNewMudsDetected();
    bool isLastUsedMudsDetected();
    void setFluidSourceMudsStatus();
    void updateMudsAlerts();
    void updateInsufficientVolumeAlerts();
    void updateSyringeInstalledState();
    void updateSyringeIsReadyState();
    void handleActiveAlertsChanged();
    void handleSyringeVolsChanged();
    void handleInjectionProgressChanged();

    void configureContrastBottleCountReachedAlert();
    void updateContrastBottleCountReachedAlert(int bottleCount);

private slots:
    void slotAppInitialised();
    void slotUpdateUsedTimeStatus();
};

#endif // DEVICE_MUDS_H
