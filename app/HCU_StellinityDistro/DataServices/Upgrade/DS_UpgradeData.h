#ifndef DS_UPGRADE_DATA_H
#define DS_UPGRADE_DATA_H

#include "Common/Common.h"
#include "DS_UpgradeDef.h"
#include "DataServices/DataServicesMacros.h"

class DS_UpgradeData : public QObject
{
    Q_OBJECT

public:
    explicit DS_UpgradeData(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~DS_UpgradeData();

    static void registerDataTypesForThread()
    {
        qRegisterMetaType<DS_UpgradeDef::UpgradeDigest>("DS_UpgradeDef::UpgradeDigest");
    }

signals:
    // Define OUTGOING Signals (i.e. WHEN DATA CHANGED)
    CREATE_DATA_CHANGED_SIGNAL(bool, DataLocked)
    CREATE_DATA_CHANGED_SIGNAL(DS_UpgradeDef::UpgradeDigest, UpgradeDigest)

private:
    // Creates GET/SET Properties
    CREATE_DATA_MEMBERS(bool, DataLocked)
    CREATE_DATA_MEMBERS(DS_UpgradeDef::UpgradeDigest, UpgradeDigest)

    EnvGlobal *env;
    EnvLocal *envLocal;

private slots:
    void slotAppStarted();

};

#endif // DS_UPGRADE_DATA_H
