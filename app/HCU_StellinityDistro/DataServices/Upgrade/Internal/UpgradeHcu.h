#ifndef UPGRADE_HCU_H
#define UPGRADE_HCU_H

#include <QProcess>
#include "Common/Common.h"
#include "Common/ActionBaseExt.h"
#include "DataServices/Upgrade/DS_UpgradeDef.h"

class UpgradeHcu : public ActionBaseExt
{
    Q_OBJECT
public:
    explicit UpgradeHcu(QObject *parent = 0, EnvGlobal *env_ = NULL, EnvLocal *envLocal = NULL);
    ~UpgradeHcu();

    DataServiceActionStatus actUpgrade(QString actGuid = "");
private:
    enum State
    {
        STATE_UNKNOWN = 0,
        STATE_READY,
        STATE_STARTED,
        STATE_PROGRESS,
        STATE_DONE,
        STATE_FAILED
    };

    QProcess procUpgrade;

    void processState();

private slots:
    void slotAppInitialised();
};

#endif // UPGRADE_HCU_H
