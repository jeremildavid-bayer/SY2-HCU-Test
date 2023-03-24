#ifndef UPGRADE_HW_BASE_H
#define UPGRADE_HW_BASE_H

#include "Common/ActionBaseExt.h"
#include "DataServices/Upgrade/DS_UpgradeDef.h"

class UpgradeHwBase : public ActionBaseExt
{
    Q_OBJECT
public:
    explicit UpgradeHwBase(QObject *parent = 0, EnvGlobal *env_ = NULL, EnvLocal *envLocal = NULL, QString actionServiceName = "", QString logPrefix = "");
    ~UpgradeHwBase();

    DataServiceActionStatus actUpgrade(QString firmwarePathFile, QString actGuid = "");

protected:
    QString firmwarePathFile;
    int loadProgress;
    int eraseProgress;
    int eraseTimeoutMs;
    int retryCount;
    int retryLimit;

private:
    QString logPrefix;
    QString actionServiceName;

    void processState();
    QString startUpgrade(QString pathFirmwareFile);
    virtual QString connectDevice() = 0;
    virtual QString startUpgradeInner(QFile &fileBuf) = 0;
    virtual void updateUpgradeHwDigest() = 0;
    virtual QString eraseFlash() = 0;
    virtual QString verifyErasedFlash() = 0;
    virtual QString startProgramFlash() = 0;

private slots:
    void slotAppInitialised();
};

#endif // UPGRADE_HW_BASE_H
