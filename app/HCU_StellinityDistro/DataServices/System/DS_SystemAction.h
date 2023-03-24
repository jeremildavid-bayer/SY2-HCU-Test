#ifndef DS_SYSTEM_ACTION_H
#define DS_SYSTEM_ACTION_H

#include <QProcess>
#include <QTimer>
#include <QJsonObject>

#include "Common/Common.h"
#include "Common/ActionBase.h"
#include "DS_SystemDef.h"
#include "Internal/SystemMonitor.h"

class DS_SystemAction : public ActionBase
{
    Q_OBJECT
public:
    explicit DS_SystemAction(QObject *parent = 0, EnvGlobal *env = NULL);
    ~DS_SystemAction();

    // Action Interfaces
    DataServiceActionStatus actRunSystemProcess(QProcess &proc, QString program, QStringList args, QString actGuid = "");
    DataServiceActionStatus actSetStatePath(DS_SystemDef::StatePath statePath, QString actGuid = "");
    DataServiceActionStatus actScreenWakeup(QString actGuid = "");
    DataServiceActionStatus actSetScreenSleepTime(int sleepTimeMinutes, QString actGuid = "");
    DataServiceActionStatus actSetScreenSleepTimeToDefault(QString actGuid = "");
    DataServiceActionStatus actSetScreenBrightness(int level, QString actGuid = "");
    DataServiceActionStatus actScreenShot(QString actGuid = "");
    DataServiceActionStatus actMultipleScreenShots(QString actGuid = "");
    DataServiceActionStatus actSetSystemDateTime(QDateTime newDateTime, QString actGuid = "");
    DataServiceActionStatus actSyncSystemDateTime(QString actGuid = "");
    DataServiceActionStatus actNetworkInterfaceUp(QString actGuid = "");
    DataServiceActionStatus actNetworkInterfaceDown(QString actGuid = "");
    DataServiceActionStatus actGetNetworkSettingParams(DS_SystemDef::NetworkSettingParams &params, QString actGuid = "");
    DataServiceActionStatus actShutdown(DS_McuDef::PowerControlType powerControlType, QString actGuid = "");
    DataServiceActionStatus actSafeExit(QString actGuid = "");
    DataServiceActionStatus actSaveUserData(QString actGuid = "");
    DataServiceActionStatus actFactoryReset(QString actGuid = "");


private:
    struct MultipleScreenShotsParams
    {
        Config::Item cultureCodeCfg;
        DataServiceActionStatus actStatus;
        QVariant origCultureCode;
        int cultureCodeIndex;
    };

    QProcess procSetSystemDateTime;
    QProcess procSyncSystemDateTime;
    QProcess procSetScreenBrightness;
    QProcess procSetScreenSleepTime;
    QProcess procScreenWakeup;
    QProcess procLinkSetup;
    QProcess procSaveUserData;
    QProcess procScreenShot;
    QProcess procFactoryReset;
    QProcess procCheckEthernetInterfaceName;

    MultipleScreenShotsParams multipleScreenShotsParams;
    SystemMonitor *monitor;
    DS_McuDef::PowerControlType lastPowerControlType;

    QVariantMap readJsonFile(QString fileName, bool alertEnable);
    void writeJsonFile(QString fileName, QVariantMap jObj, bool alertEnable);
    void checkLastDatetime(bool suppressPopup);
    void checkLastShutdown();
    void checkPreventativeMaintenanceReminder();
    void actMultipleScreenShotsInner();
    void shutdown();
    DataServiceActionStatus handleSetNetworkProcess(DataServiceActionStatus status);
    void initializeNetwork();
private slots:
    void slotAppInitialised();
};

#endif // DS_SYSTEM_ACTION_H
