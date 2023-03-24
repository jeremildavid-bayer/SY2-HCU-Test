#include "Apps/AppManager.h"
#include "DS_UpgradeData.h"

DS_UpgradeData::DS_UpgradeData(QObject *parent, EnvGlobal *env_) :
    QObject(parent),
    env(env_)
{
    envLocal = new EnvLocal("DS_Upgrade-Data", "UPGRADE_DATA");

    connect(env->appManager, &AppManager::signalAppStarted, this, [=]() {
        slotAppStarted();
    });

    m_DataLocked = false;
}

DS_UpgradeData::~DS_UpgradeData()
{
    delete envLocal;
}

void DS_UpgradeData::slotAppStarted()
{
}


