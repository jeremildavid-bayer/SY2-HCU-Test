#ifndef DS_UPGRADE_ACTION_H
#define DS_UPGRADE_ACTION_H

#include "Common/Common.h"
#include "Common/ActionBase.h"
#include "DS_UpgradeDef.h"
#include "Internal/UpgradeSru.h"


class DS_UpgradeAction : public ActionBase
{
    Q_OBJECT

public:
    explicit DS_UpgradeAction(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~DS_UpgradeAction();

    DataServiceActionStatus actUpdateSelectedPackageInfo(QString pathPackage, QString actGuid = "");
    DataServiceActionStatus actUpgrade(QString actGuid = "");


private:
    UpgradeSru *upgradeSru;

private slots:
    void slotAppInitialised();
};

#endif // DS_UPGRADE_ACTION_H
