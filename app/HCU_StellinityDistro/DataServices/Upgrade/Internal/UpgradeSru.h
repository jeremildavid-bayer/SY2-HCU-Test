#ifndef UPGRADE_SRU_H
#define UPGRADE_SRU_H

#include <QProcess>
#include "Common/Common.h"
#include "Common/ActionBaseExt.h"
#include "DataServices/Upgrade/DS_UpgradeDef.h"
#include "UpgradeHcu.h"
#include "UpgradeHwMcu.h"
#include "UpgradeHwStopcock.h"

class UpgradeSru : public ActionBaseExt
{
    Q_OBJECT
public:
    explicit UpgradeSru(QObject *parent = 0, EnvGlobal *env_ = NULL, EnvLocal *envLocal = NULL);
    ~UpgradeSru();

    DataServiceActionStatus actUpdateSelectedPackageInfo(QString pathPackage, QString actGuid = "");
    DataServiceActionStatus actUpgrade(QString actGuid = "");
private:
    
    UpgradeHcu *upgradeHcu;
    UpgradeHwMcu *upgradeHwMcu;
    UpgradeHwStopcock *upgradeHwStopcock;

    QProcess procSruUpgrade;
    QTimer tmrProgressMonitor;

    int getState();
    void setStateSynch(int newState);
    void processState();
    void updateSruUpgradeProgress();
    QString updateFirmwareInfo(QString pathFirmware);
    QString checkUpdateFile(const DS_UpgradeDef::UpgradeStatus &upgradeStatus);
    void updatePackageInfo(QString extractedInfo);
    void updateExtractedInfo();


private slots:
    void slotAppInitialised();
};

#endif // UPGRADE_SRU_H
