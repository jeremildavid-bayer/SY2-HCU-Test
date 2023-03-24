#include "Apps/AppManager.h"
#include "DS_UpgradeAction.h"

DS_UpgradeAction::DS_UpgradeAction(QObject *parent, EnvGlobal *env_) :
    ActionBase(parent, env_)
{
    envLocal = new EnvLocal("DS_Upgrade-Action", "UPGRADE_ACTION");
    upgradeSru = new UpgradeSru(this, env, envLocal);

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });
}

DS_UpgradeAction::~DS_UpgradeAction()
{
    delete upgradeSru;
    delete envLocal;
}

void DS_UpgradeAction::slotAppInitialised()
{
}

DataServiceActionStatus DS_UpgradeAction::actUpdateSelectedPackageInfo(QString pathPackage, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "UpdateSelectedPackageInfo", QString().asprintf("%s", pathPackage.CSTR()));

    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);
        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            env->actionMgr->onActionCompleted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
                actionCompleted(curStatus, &status);
            });
        }
    });

    return upgradeSru->actUpdateSelectedPackageInfo(pathPackage, guid);
}

DataServiceActionStatus DS_UpgradeAction::actUpgrade(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "Upgrade");
    QString guid = Util::newGuid();

    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);
        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            env->actionMgr->onActionCompleted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
                actionCompleted(curStatus, &status);
            });
        }
    });

    return upgradeSru->actUpgrade(guid);
}
