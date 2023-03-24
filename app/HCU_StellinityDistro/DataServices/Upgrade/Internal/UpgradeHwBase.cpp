#include "Apps/AppManager.h"
#include "Common/ImrParser.h"
#include "DataServices/Upgrade/DS_UpgradeData.h"
#include "UpgradeHwBase.h"

UpgradeHwBase::UpgradeHwBase(QObject *parent, EnvGlobal *env_, EnvLocal *envLocal_, QString actionServiceName_, QString logPrefix_) :
    ActionBaseExt(parent, env_),
    logPrefix(logPrefix_),
    actionServiceName(actionServiceName_)
{
    envLocal = envLocal_;

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });
    retryLimit = 0;
}

UpgradeHwBase::~UpgradeHwBase()
{
}

void UpgradeHwBase::slotAppInitialised()
{
    setState(DS_UpgradeDef::UPGRADE_HW_STATE_READY);
}

DataServiceActionStatus UpgradeHwBase::actUpgrade(QString pathFile, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, actionServiceName + "Upgrade", QString().asprintf("%s", pathFile.CSTR()));

    if (state != DS_UpgradeDef::UPGRADE_HW_STATE_READY)
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = QString().asprintf("Invalid state, state=%d", state);
        actionStarted(status);
        return status;
    }

    firmwarePathFile = pathFile;
    retryCount = 1;
    actStatusBuf = status;
    actStatusBuf.state = DS_ACTION_STATE_STARTED;

    actionStarted(actStatusBuf);
    setState(DS_UpgradeDef::UPGRADE_HW_STATE_STARTED);

    return actStatusBuf;
}

void UpgradeHwBase::processState()
{
    switch (state)
    {
    case DS_UpgradeDef::UPGRADE_HW_STATE_READY:
        LOG_INFO("%s: UPGRADE_HW_STATE_READY\n", logPrefix.CSTR());
        break;
    case DS_UpgradeDef::UPGRADE_HW_STATE_STARTED:
        LOG_INFO("%s: UPGRADE_HW_STATE_STARTED\n", logPrefix.CSTR());
        {
            loadProgress = 0;
            eraseProgress = 0;

            QString err = startUpgrade(firmwarePathFile);
            if (err != "")
            {
                actStatusBuf.state = DS_ACTION_STATE_INTERNAL_ERR;
                actStatusBuf.err = err;
                setState(DS_UpgradeDef::UPGRADE_HW_STATE_FAILED);
            }
            else
            {
                setState(DS_UpgradeDef::UPGRADE_HW_STATE_ERASE_STARTED);
            }
        }
        break;
    case DS_UpgradeDef::UPGRADE_HW_STATE_ERASE_STARTED:
        LOG_INFO("%s: UPGRADE_HW_STATE_ERASE_STARTED\n", logPrefix.CSTR());
        {
            QString err = eraseFlash();
            if (err != "")
            {
                actStatusBuf.state = DS_ACTION_STATE_INTERNAL_ERR;
                actStatusBuf.err = err;
                setState(DS_UpgradeDef::UPGRADE_HW_STATE_FAILED);
            }
            else
            {
                updateUpgradeHwDigest();
                setState(DS_UpgradeDef::UPGRADE_HW_STATE_ERASING);
            }
        }
        break;
    case DS_UpgradeDef::UPGRADE_HW_STATE_ERASING:
        if (eraseProgress == 0)
        {
            LOG_INFO("%s: UPGRADE_HW_STATE_ERASING\n", logPrefix.CSTR());
        }
        if (eraseProgress >= 100)
        {
            setState(DS_UpgradeDef::UPGRADE_HW_STATE_VERIFYING_ERASED_FLASH);
        }
        else
        {
            QTimer::singleShot(eraseTimeoutMs / 100, this, [=] {
                eraseProgress++;
                updateUpgradeHwDigest();
                processState();
            });
        }
        break;
    case DS_UpgradeDef::UPGRADE_HW_STATE_VERIFYING_ERASED_FLASH:
        LOG_INFO("%s: UPGRADE_HW_STATE_VERIFYING_ERASED_FLASH\n", logPrefix.CSTR());
        {
            QString err = verifyErasedFlash();
            if (err != "")
            {
                actStatusBuf.state = DS_ACTION_STATE_INTERNAL_ERR;
                actStatusBuf.err = "Failed to erase flash";
                setState(DS_UpgradeDef::UPGRADE_HW_STATE_FAILED);
            }
            else
            {
                state = DS_UpgradeDef::UPGRADE_HW_STATE_PROGRAMMING_FLASH;
                setState(DS_UpgradeDef::UPGRADE_HW_STATE_PROGRAMMING_FLASH);
            }
            updateUpgradeHwDigest();
        }
        break;
    case DS_UpgradeDef::UPGRADE_HW_STATE_PROGRAMMING_FLASH:
        LOG_INFO("%s: UPGRADE_HW_STATE_PROGRAMMING_FLASH\n", logPrefix.CSTR());
        startProgramFlash();
        break;
    case DS_UpgradeDef::UPGRADE_HW_STATE_COMPLETED:
        LOG_INFO("%s: UPGRADE_HW_STATE_COMPLETED\n", logPrefix.CSTR());
        actStatusBuf.state = DS_ACTION_STATE_COMPLETED;
        actionCompleted(actStatusBuf);
        setState(DS_UpgradeDef::UPGRADE_HW_STATE_READY);
        break;
    case DS_UpgradeDef::UPGRADE_HW_STATE_FAILED:
        LOG_INFO("%s: UPGRADE_HW_STATE_FAILED\n", logPrefix.CSTR());
        if (retryCount >= retryLimit)
        {
            actionCompleted(actStatusBuf);
            setState(DS_UpgradeDef::UPGRADE_HW_STATE_READY);
        }
        else
        {
            LOG_WARNING("%s: Upgrade failed %d/%d. Retrying..\n", logPrefix.CSTR(), retryCount, retryLimit);
            retryCount++;
            QTimer::singleShot(100, this, [=] {
                setState(DS_UpgradeDef::UPGRADE_HW_STATE_STARTED);
            });
        }
        break;
    default:
        LOG_ERROR("%s: Bad state(%d)\n", logPrefix.CSTR(), state);
        break;
    }
}

QString UpgradeHwBase::startUpgrade(QString pathFile)
{
    QString err;
    QFile fileBuf(pathFile);
    if (!fileBuf.open(QFile::ReadWrite))
    {
        err = QString().asprintf("Failed to load image file. err=%s\n", fileBuf.errorString().CSTR());
        goto bail;
    }

    err = connectDevice();
    if (err != "")
    {
        goto bail;
    }

    loadProgress = 0;
    err = startUpgradeInner(fileBuf);

bail:
    fileBuf.close();
    updateUpgradeHwDigest();
    return err;
}

