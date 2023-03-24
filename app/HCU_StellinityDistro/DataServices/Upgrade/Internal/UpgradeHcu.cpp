#include "UpgradeHcu.h"
#include "Apps/AppManager.h"
#include "DataServices/Upgrade/DS_UpgradeData.h"
#include "DataServices/System/DS_SystemAction.h"

UpgradeHcu::UpgradeHcu(QObject *parent, EnvGlobal *env_, EnvLocal *envLocal_) :
    ActionBaseExt(parent, env_)
{
    envLocal = envLocal_;
    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });
}

UpgradeHcu::~UpgradeHcu()
{
}

void UpgradeHcu::slotAppInitialised()
{
    setState(STATE_READY);

    connect(&procUpgrade, &QProcess::readyReadStandardError, this, [=] {
        if (getState() != STATE_PROGRESS)
        {
            return;
        }
        QString allStdErrStr = procUpgrade.readAllStandardError();
        QStringList stdErrs = allStdErrStr.split(" ");
        foreach (QString stdErr, stdErrs)
        {
            if (stdErr.contains("%"))
            {
                stdErr.replace("%", "");
                DS_UpgradeDef::UpgradeDigest digest = env->ds.upgradeData->getUpgradeDigest();
                digest.hcu.progress = qMin(99, stdErr.toInt());
                env->ds.upgradeData->setUpgradeDigest(digest);
                break;
            }
        }
    });
}

DataServiceActionStatus UpgradeHcu::actUpgrade(QString actGuid)
{
    if (state != STATE_READY)
    {
        DataServiceActionStatus status;
        status.guid = actGuid;
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = QString().asprintf("Invalid state, state=%d", state);
        actionStarted(status);
        return status;
    }

    actStatusBuf.guid = actGuid;
    actStatusBuf.err = "";
    actStatusBuf.state = DS_ACTION_STATE_STARTED;

    actionStarted(actStatusBuf);
    setState(STATE_STARTED);

    return actStatusBuf;
}

void UpgradeHcu::processState()
{
    DS_UpgradeDef::UpgradeDigest digest = env->ds.upgradeData->getUpgradeDigest();

    switch (state)
    {
    case STATE_READY:
        LOG_INFO("UPGRADE_HCU: STATE_READY\n");
        procUpgrade.close();
        break;
    case STATE_STARTED:
        LOG_INFO("UPGRADE_HCU: STATE_STARTED\n");
        {
            QString actGuid = Util::newGuid();
            env->actionMgr->onActionStarted(actGuid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
                if (curStatus.state == DS_ACTION_STATE_STARTED)
                {
                    setState(STATE_PROGRESS);
                }
                else
                {
                    actStatusBuf.err = curStatus.err;
                    actStatusBuf.state = curStatus.state;
                    setState(STATE_FAILED);
                }
            });

            env->actionMgr->onActionCompleted(actGuid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
                if (curStatus.state == DS_ACTION_STATE_COMPLETED)
                {
                    setState(STATE_DONE);
                }
                else
                {
                   actStatusBuf.err = curStatus.err;
                   actStatusBuf.state = curStatus.state;
                   setState(STATE_FAILED);
                }
            });
            env->ds.systemAction->actRunSystemProcess(procUpgrade, PATH_UPGRADE_LOAD_IMAGE, QStringList() << PATH_TEMP_UPGRADE << digest.hcu.pathFile, actGuid);
        }
        break;
    case STATE_PROGRESS:
        LOG_INFO("UPGRADE_HCU: STATE_PROGRESS\n");
        break;
    case STATE_DONE:
        LOG_INFO("UPGRADE_HCU: STATE_DONE\n");
        {
            QString stdOut = procUpgrade.readAllStandardOutput();
            QString stdErr = procUpgrade.readAllStandardError();

            LOG_INFO("UPGRADE_HCU: Install process completed: proc=%s, arg=%s, out=%s err=%s\n",
                     procUpgrade.program().CSTR(), procUpgrade.arguments().join(" ").CSTR(),
                     stdOut.CSTR(), stdErr.CSTR());

            if ( (stdErr.contains("Writing image done")) ||
                 (stdErr.contains("No space left on device")) ||
                 (stdOut.contains("Writing image done")) ||
                 (stdOut.contains("No space left on device")) ||
                 (stdOut.contains("Upgrade completed")) )
            {
                digest.hcu.progress = 100;
                env->ds.upgradeData->setUpgradeDigest(digest);
                actStatusBuf.state = DS_ACTION_STATE_COMPLETED;
                actionCompleted(actStatusBuf);
                setState(STATE_READY);
            }
            else
            {
                actStatusBuf.err = stdOut + "," + stdErr;
                actStatusBuf.state = DS_ACTION_STATE_INTERNAL_ERR;
                setState(STATE_FAILED);
            }
        }
        break;
    case STATE_FAILED:
        LOG_ERROR("UPGRADE_HCU: STATE_FAILED\n");
        digest.hcu.err = actStatusBuf.err;
        env->ds.upgradeData->setUpgradeDigest(digest);
        actionCompleted(actStatusBuf);
        setState(STATE_READY);
        break;
    default:
        LOG_ERROR("UPGRADE_HCU: Unknown State(%d)\n", state);
        break;
    }
}
