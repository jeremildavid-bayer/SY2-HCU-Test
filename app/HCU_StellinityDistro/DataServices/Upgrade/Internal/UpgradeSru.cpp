#include <QDir>
#include "UpgradeSru.h"
#include "Apps/AppManager.h"
#include "Common/ImrParser.h"
#include "DataServices/Upgrade/DS_UpgradeData.h"
#include "DataServices/System/DS_SystemAction.h"
#include "DataServices/Mcu/DS_McuAction.h"
#include "DataServices/Mcu/DS_McuData.h"
#include "DataServices/Capabilities/DS_Capabilities.h"
#include "DataServices/CfgGlobal/DS_CfgGlobal.h"

UpgradeSru::UpgradeSru(QObject *parent, EnvGlobal *env_, EnvLocal *envLocal_) :
    ActionBaseExt(parent, env_)
{
    envLocal = envLocal_;
    upgradeHcu = new UpgradeHcu(this, env, envLocal);
    upgradeHwMcu = new UpgradeHwMcu(this, env, envLocal);
    upgradeHwStopcock = new UpgradeHwStopcock(this, env, envLocal);

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });
}

UpgradeSru::~UpgradeSru()
{
    delete upgradeHcu;
    delete upgradeHwMcu;
    delete upgradeHwStopcock;
}

void UpgradeSru::slotAppInitialised()
{
    setState(DS_UpgradeDef::STATE_READY);

    connect(&tmrProgressMonitor, &QTimer::timeout, this, [=] {
        if (env->state == EnvGlobal::STATE_EXITING)
        {
            return;
        }

        updateSruUpgradeProgress();
    });
}

DataServiceActionStatus UpgradeSru::actUpdateSelectedPackageInfo(QString pathPackage, QString actGuid)
{
    DS_UpgradeDef::UpgradeDigest digest = env->ds.upgradeData->getUpgradeDigest();
    if (digest.state != DS_UpgradeDef::STATE_READY)
    {
        DataServiceActionStatus status;
        status.guid = actGuid;
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = QString().asprintf("Invalid state, state=%d", digest.state);
        actionStarted(status);
        return status;
    }

    QFile packageFile(pathPackage);
    if (!packageFile.exists())
    {
        DataServiceActionStatus status;
        status.guid = actGuid;
        status.state = DS_ACTION_STATE_BAD_REQUEST;
        status.err = QString().asprintf("Cannot find SW Package");
        actionStarted(status);
        return status;
    }

    QString fileName = packageFile.fileName();
    if ( (!fileName.contains(".pkg")) &&
         (!fileName.contains(".gz")) &&
         (!fileName.contains(".mot")) &&
         (!fileName.contains(".hex")) )
    {
        DataServiceActionStatus status;
        status.guid = actGuid;
        status.state = DS_ACTION_STATE_BAD_REQUEST;
        status.err = QString().asprintf("Unsupported SW Package");
        actionStarted(status);
        return status;
    }

    actStatusBuf.guid = actGuid;
    actStatusBuf.err = "";
    actStatusBuf.state = DS_ACTION_STATE_STARTED;
    actionStarted(actStatusBuf);

    digest.sru.pathFile = "";
    digest.hcu.pathFile = "";
    digest.mcu.pathFile = "";
    digest.stopcock.pathFile = "";

    digest.sru.progress = 0;
    digest.hcu.progress = 0;
    digest.mcu.progress = 0;
    digest.stopcock.progress = 0;

    digest.sru.err = "";
    digest.hcu.err = "";
    digest.mcu.err = "";
    digest.stopcock.err = "";

    digest.sru.fileSizeKB = 0;
    digest.hcu.fileSizeKB = 0;
    digest.mcu.fileSizeKB = 0;
    digest.stopcock.fileSizeKB = 0;

    if (fileName.contains(".pkg"))
    {
        // SRU upgrade
        digest.sru.pathFile = pathPackage;
        env->ds.upgradeData->setUpgradeDigest(digest);

        // Needs to extract the package to find more information
        setState(DS_UpgradeDef::STATE_GET_PACKAGE_INFO_STARTED);
    }
    else
    {
        env->ds.upgradeData->setUpgradeDigest(digest);

        // Firmware upgrade
        actStatusBuf.err = updateFirmwareInfo(pathPackage);
        if (actStatusBuf.err == "")
        {
            actStatusBuf.state = DS_ACTION_STATE_COMPLETED;
        }
        else
        {
            actStatusBuf.state = DS_ACTION_STATE_INTERNAL_ERR;
        }
        LOG_INFO("UPGRADE_SRU: UpgradeDigest = %s\n", Util::qVarientToJsonData(ImrParser::ToImr_UpgradeDigest(digest), false).CSTR());
        actionCompleted(actStatusBuf);
    }
    return actStatusBuf;
}

DataServiceActionStatus UpgradeSru::actUpgrade(QString actGuid)
{
    DS_UpgradeDef::UpgradeDigest digest = env->ds.upgradeData->getUpgradeDigest();
    if (digest.state != DS_UpgradeDef::STATE_READY)
    {
        DataServiceActionStatus status;
        status.guid = actGuid;
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = QString().asprintf("Invalid state, state=%d", digest.state);
        actionStarted(status);
        return status;
    }

    digest.sru.progress = 0;
    digest.hcu.progress = 0;
    digest.mcu.progress = 0;
    digest.stopcock.progress = 0;

    digest.sru.err = "";
    digest.hcu.err = "";
    digest.mcu.err = "";
    digest.stopcock.err = "";

    env->ds.upgradeData->setUpgradeDigest(digest);
    actStatusBuf.guid = actGuid;
    actStatusBuf.err = "";
    actStatusBuf.state = DS_ACTION_STATE_STARTED;

    // reconnect MCU FTDI before starting upgrade
    (void)system(PATH_RECONNECT_MCU_FTDI);

    actionStarted(actStatusBuf);
    setState(DS_UpgradeDef::STATE_UPGRADE_STARTED);

    return actStatusBuf;
}

QString UpgradeSru::checkUpdateFile(const DS_UpgradeDef::UpgradeStatus &upgradeStatus)
{
    if (!QFile(upgradeStatus.pathFile).exists())
    {
        return "Unable to access file";
    }

    if (env->ds.systemData->getDiskUserFreeSpaceMB() < (upgradeStatus.fileSizeKB / 1024))
    {
        return "Not enough disk space";
    }
    return "";
}

QString UpgradeSru::updateFirmwareInfo(QString pathFirmware)
{
    QFile fwFile(pathFirmware);
    if (!fwFile.exists())
    {
        return "File not exist";
    }

    QString fileName = fwFile.fileName();
    if ( (!fileName.contains(".gz")) &&
         (!fileName.contains(".mot")) &&
         (!fileName.contains(".hex")) )
    {
        return "Unsupported file";
    }

    DS_UpgradeDef::UpgradeDigest digest = env->ds.upgradeData->getUpgradeDigest();
    DS_UpgradeDef::UpgradeStatus *upgradeStatus = NULL;

    if (fileName.contains(".gz"))
    {
        upgradeStatus = &digest.sru;
    }
    else if (fileName.contains(".mot"))
    {
        upgradeStatus = &digest.mcu;
    }
    else if (fileName.contains(".hex"))
    {
        upgradeStatus = &digest.stopcock;
    }
    upgradeStatus->pathFile = pathFirmware;
    upgradeStatus->fileSizeKB = fwFile.size() / 1024;
    digest.sru.fileSizeKB = upgradeStatus->fileSizeKB;
    env->ds.upgradeData->setUpgradeDigest(digest);
    return "";
}

void UpgradeSru::updatePackageInfo(QString extractionInfo)
{
    DS_UpgradeDef::UpgradeDigest digest = env->ds.upgradeData->getUpgradeDigest();
    QStringList lines = extractionInfo.split("\n");
    foreach (QString line, lines)
    {
        line.replace("\r", "");
        if (line.contains("<SIZE>"))
        {
            digest.sru.fileSizeKB = line.replace("<SIZE>", "").toInt() * 1024;
        }
    }
    env->ds.upgradeData->setUpgradeDigest(digest);
}

void UpgradeSru::updateExtractedInfo()
{
    DS_UpgradeDef::UpgradeDigest digest = env->ds.upgradeData->getUpgradeDigest();
    QStringList extractedFileNames = QDir(PATH_TEMP_UPGRADE).entryList();
    QString pathPrefix = QString(PATH_TEMP_UPGRADE) + "/";
    foreach (QString fileName, extractedFileNames)
    {
        if (fileName.contains(".gz"))
        {
            digest.hcu.pathFile = pathPrefix + fileName;
        }
        else if (fileName.contains(".mot"))
        {
            QString curVersion = env->ds.mcuData->getMcuVersion();
            QString curFileName = "MCU_" + curVersion.replace(".", "_") + ".mot";
            if (curFileName == fileName)
            {
                LOG_WARNING("UPGRADE_SRU: MCU is already loaded with same firmware(%s). Skipping MCU upgrade.\n", fileName.CSTR());
            }
            else
            {
                digest.mcu.pathFile = pathPrefix + fileName;
            }
        }
        else if (fileName.contains(".hex"))
        {
            QString curVersion = env->ds.mcuData->getStopcockVersion();
            QString curFileName = "SC_" + curVersion.replace(".", "_") + ".hex";
            if (curFileName == fileName)
            {
                LOG_WARNING("UPGRADE_SRU: STOPCOCK is already loaded with same firmware(%s). Skipping STOPCOCK upgrade.\n", fileName.CSTR());
            }
            else
            {
                digest.stopcock.pathFile = pathPrefix + fileName;
            }
        }
    }

    env->ds.upgradeData->setUpgradeDigest(digest);
    LOG_INFO("UPGRADE_SRU: UpgradeDigest = %s\n", Util::qVarientToJsonData(ImrParser::ToImr_UpgradeDigest(digest), false).CSTR());
}

void UpgradeSru::updateSruUpgradeProgress()
{
    DS_UpgradeDef::UpgradeDigest digest = env->ds.upgradeData->getUpgradeDigest();

    if (digest.state == DS_UpgradeDef::STATE_UPGRADE_DONE)
    {
        digest.sru.progress = 100;
    }
    else if (digest.sru.pathFile != "")
    {
        // Derive progress from each individual upgrades

        // Get extraction progress
        int extractedProgress = 0;
        if (digest.state == DS_UpgradeDef::STATE_EXTRACT_PACKAGE_PROGRESS)
        {
            qint64 extractedKB = 0;
            QStringList extractedFileNames = QDir(PATH_TEMP_UPGRADE).entryList(QStringList() << "*.gz");
            if (extractedFileNames.length() > 0)
            {
                QFile fileBuf(QString(PATH_TEMP_UPGRADE) + "/" + extractedFileNames[0]);
                extractedKB = fileBuf.size() / 1024;
            }

            if (digest.sru.fileSizeKB > 0)
            {
                extractedProgress = qMin((qint64)99, (extractedKB * 100) / digest.sru.fileSizeKB);
            }
        }
        else if (digest.state > DS_UpgradeDef::STATE_EXTRACT_PACKAGE_DONE)
        {
            extractedProgress = 100;
        }

        int curProgress = (extractedProgress * 0.3) + (digest.stopcock.progress * 0.2) + (digest.mcu.progress * 0.2) + (digest.hcu.progress * 0.3);
        if (curProgress > 99)
        {
            LOG_WARNING("UPGRADE_SRU: Unexpected progress values(%d,%d,%d,%d)\n", extractedProgress, digest.stopcock.progress, digest.mcu.progress, digest.hcu.progress);
        }
        curProgress = qMin(99, curProgress);
        digest.sru.progress = qMax(digest.sru.progress, curProgress);
    }
    else if (digest.hcu.pathFile != "")
    {
        digest.sru.progress = qMax(digest.sru.progress, digest.hcu.progress);
    }
    else if (digest.mcu.pathFile != "")
    {
        digest.sru.progress = qMax(digest.sru.progress, digest.mcu.progress);
    }
    else if (digest.stopcock.pathFile != "")
    {
        digest.sru.progress = qMax(digest.sru.progress, digest.stopcock.progress);
    }
    env->ds.upgradeData->setUpgradeDigest(digest);
}

int UpgradeSru::getState()
{
    DS_UpgradeDef::UpgradeDigest digest = env->ds.upgradeData->getUpgradeDigest();
    return digest.state;
}

void UpgradeSru::setStateSynch(int newState)
{
    DS_UpgradeDef::UpgradeDigest digest = env->ds.upgradeData->getUpgradeDigest();
    digest.state = (DS_UpgradeDef::UpgradeState)newState;
    env->ds.upgradeData->setUpgradeDigest(digest);
}

void UpgradeSru::processState()
{
    DS_UpgradeDef::UpgradeDigest digest = env->ds.upgradeData->getUpgradeDigest();

    switch (getState())
    {
    case DS_UpgradeDef::STATE_READY:
        LOG_INFO("UPGRADE_SRU: STATE_READY\n");
        procSruUpgrade.close();
        tmrProgressMonitor.stop();
        break;
    case DS_UpgradeDef::STATE_GET_PACKAGE_INFO_STARTED:
        LOG_INFO("UPGRADE_SRU: STATE_GET_PACKAGE_INFO_STARTED\n");
        handleSubAction(DS_UpgradeDef::STATE_GET_PACKAGE_INFO_PROGRESS, DS_UpgradeDef::STATE_GET_PACKAGE_INFO_DONE, DS_UpgradeDef::STATE_GET_PACKAGE_INFO_FAILED);
        procSruUpgrade.close();
        env->ds.systemAction->actRunSystemProcess(procSruUpgrade, PATH_UPGRADE_CHECK_FILE, QStringList() << digest.sru.pathFile, guidSubAction);
        break;
    case DS_UpgradeDef::STATE_GET_PACKAGE_INFO_PROGRESS:
        LOG_INFO("UPGRADE_SRU: STATE_GET_PACKAGE_INFO_PROGRESS\n");
        break;
    case DS_UpgradeDef::STATE_GET_PACKAGE_INFO_DONE:
        LOG_INFO("UPGRADE_SRU: STATE_GET_PACKAGE_INFO_DONE\n");
        {
            QString extractionInfo = procSruUpgrade.readAll();
            if (extractionInfo.contains("INFO FILE read done"))
            {
                updatePackageInfo(extractionInfo);
                DS_UpgradeDef::UpgradeDigest digest = env->ds.upgradeData->getUpgradeDigest();
                LOG_INFO("UPGRADE_SRU: UpgradeDigest = %s\n", Util::qVarientToJsonData(ImrParser::ToImr_UpgradeDigest(digest), false).CSTR());
                actionCompleted(actStatusBuf);
                setState(DS_UpgradeDef::STATE_READY);
            }
            else
            {
                actStatusBuf.err = QString().asprintf("Get Info Failed - %s", extractionInfo.CSTR());
                actStatusBuf.state = DS_ACTION_STATE_INTERNAL_ERR;
                setState(DS_UpgradeDef::STATE_GET_PACKAGE_INFO_FAILED);
            }
        }
        break;
    case DS_UpgradeDef::STATE_GET_PACKAGE_INFO_FAILED:
        LOG_ERROR("UPGRADE_SRU: STATE_GET_PACKAGE_INFO_FAILED\n");
        actionCompleted(actStatusBuf);
        setState(DS_UpgradeDef::STATE_UPGRADE_FAILED);
        break;
    case DS_UpgradeDef::STATE_UPGRADE_STARTED:
        LOG_INFO("UPGRADE_SRU: STATE_UPGRADE_STARTED\n");
        digest.sru.progress = 1;
        env->ds.upgradeData->setUpgradeDigest(digest);
        env->ds.mcuAction->actLinkDisconnect();
        if (!tmrProgressMonitor.isActive())
        {
            tmrProgressMonitor.start(1000);
        }
        setState(DS_UpgradeDef::STATE_CHECK_PACKAGE_STARTED);
        break;
    case DS_UpgradeDef::STATE_CHECK_PACKAGE_STARTED:
        LOG_INFO("UPGRADE_SRU: STATE_CHECK_PACKAGE_STARTED\n");
        {
            if (digest.sru.pathFile == "")
            {
                QTimer::singleShot(MCU_ACTION_TIMEOUT_MS, this, [=] {
                    // Give a little time to prepare link state
                    // Individual upgrade
                    setState(DS_UpgradeDef::STATE_INSTALL_STOPCOCK_STARTED);
                });
            }
            else
            {
                // Package upgrade
                actStatusBuf.err = checkUpdateFile(digest.sru);
                if (actStatusBuf.err == "")
                {
                    setState(DS_UpgradeDef::STATE_CHECK_PACKAGE_DONE);
                }
                else
                {
                    actStatusBuf.state = DS_ACTION_STATE_INVALID_STATE;
                    setState(DS_UpgradeDef::STATE_CHECK_PACKAGE_FAILED);
                }
            }
        }
        break;
    case DS_UpgradeDef::STATE_CHECK_PACKAGE_DONE:
        LOG_INFO("UPGRADE_SRU: STATE_CHECK_PACKAGE_DONE\n");
        setState(DS_UpgradeDef::STATE_EXTRACT_PACKAGE_STARTED);
        break;
    case DS_UpgradeDef::STATE_CHECK_PACKAGE_FAILED:
        LOG_ERROR("UPGRADE_SRU: STATE_CHECK_PACKAGE_FAILED\n");
        setState(DS_UpgradeDef::STATE_UPGRADE_FAILED);
        break;
    case DS_UpgradeDef::STATE_EXTRACT_PACKAGE_STARTED:
        LOG_INFO("UPGRADE_SRU: STATE_EXTRACT_PACKAGE_STARTED\n");
        handleSubAction(DS_UpgradeDef::STATE_EXTRACT_PACKAGE_PROGRESS, DS_UpgradeDef::STATE_EXTRACT_PACKAGE_DONE, DS_UpgradeDef::STATE_EXTRACT_PACKAGE_FAILED);
        procSruUpgrade.close();
        env->ds.systemAction->actRunSystemProcess(procSruUpgrade, PATH_UPGRADE_EXTRACT, QStringList() << digest.sru.pathFile << PATH_TEMP_UPGRADE, guidSubAction);
        break;
    case DS_UpgradeDef::STATE_EXTRACT_PACKAGE_PROGRESS:
        LOG_INFO("UPGRADE_SRU: STATE_EXTRACT_PACKAGE_PROGRESS\n");
        break;
    case DS_UpgradeDef::STATE_EXTRACT_PACKAGE_DONE:
        LOG_INFO("UPGRADE_SRU: STATE_EXTRACT_PACKAGE_DONE\n");
        {
            QString extractionInfo = procSruUpgrade.readAll();
            if (extractionInfo.contains("Extraction Done"))
            {
                updateExtractedInfo();
                setState(DS_UpgradeDef::STATE_INSTALL_STOPCOCK_STARTED);
            }
            else
            {
                actStatusBuf.err = QString().asprintf("Extraction Failed - %s", extractionInfo.CSTR());
                actStatusBuf.state = DS_ACTION_STATE_INTERNAL_ERR;
                setState(DS_UpgradeDef::STATE_EXTRACT_PACKAGE_FAILED);
            }
        }
        break;
    case DS_UpgradeDef::STATE_EXTRACT_PACKAGE_FAILED:
        LOG_ERROR("UPGRADE_SRU: STATE_EXTRACT_PACKAGE_FAILED\n");
        setState(DS_UpgradeDef::STATE_UPGRADE_FAILED);
        break;
    case DS_UpgradeDef::STATE_INSTALL_STOPCOCK_STARTED:
        LOG_INFO("UPGRADE_SRU: STATE_INSTALL_STOPCOCK_STARTED\n");
        if ( (digest.stopcock.pathFile == "") ||
             (env->ds.capabilities->get_Developer_McuSimulationEnabled()) )
        {
            digest.stopcock.progress = 100;
            env->ds.upgradeData->setUpgradeDigest(digest);
            setState(DS_UpgradeDef::STATE_INSTALL_STOPCOCK_DONE);
        }
        else
        {
            actStatusBuf.err = checkUpdateFile(digest.stopcock);
            if (actStatusBuf.err == "")
            {
                handleSubAction(DS_UpgradeDef::STATE_INSTALL_STOPCOCK_PROGRESS, DS_UpgradeDef::STATE_INSTALL_STOPCOCK_DONE, DS_UpgradeDef::STATE_INSTALL_STOPCOCK_FAILED);
                upgradeHwStopcock->actUpgrade(digest.stopcock.pathFile, guidSubAction);
            }
            else
            {
                actStatusBuf.state = DS_ACTION_STATE_INVALID_STATE;
                setState(DS_UpgradeDef::STATE_INSTALL_STOPCOCK_FAILED);
            }
        }
        break;
    case DS_UpgradeDef::STATE_INSTALL_STOPCOCK_PROGRESS:
        LOG_INFO("UPGRADE_SRU: STATE_INSTALL_STOPCOCK_PROGRESS\n");
        break;
    case DS_UpgradeDef::STATE_INSTALL_STOPCOCK_DONE:
        LOG_INFO("UPGRADE_SRU: STATE_INSTALL_STOPCOCK_DONE\n");
        setState(DS_UpgradeDef::STATE_INSTALL_MCU_STARTED);
        break;
    case DS_UpgradeDef::STATE_INSTALL_STOPCOCK_FAILED:
        LOG_ERROR("UPGRADE_SRU: STATE_INSTALL_STOPCOCK_FAILED\n");
        actStatusBuf.err = "Stopcock Upgrade Failed - " + actStatusBuf.err;
        setState(DS_UpgradeDef::STATE_UPGRADE_FAILED);
        break;
    case DS_UpgradeDef::STATE_INSTALL_MCU_STARTED:
        LOG_INFO("UPGRADE_SRU: STATE_INSTALL_MCU_STARTED\n");
        if ( (digest.mcu.pathFile == "") || (env->ds.capabilities->get_Developer_McuSimulationEnabled()) )
        {
            digest.mcu.progress = 100;
            env->ds.upgradeData->setUpgradeDigest(digest);
            setState(DS_UpgradeDef::STATE_INSTALL_MCU_DONE);
        }
        else
        {
            actStatusBuf.err = checkUpdateFile(digest.mcu);
            if (actStatusBuf.err == "")
            {
                handleSubAction(DS_UpgradeDef::STATE_INSTALL_MCU_PROGRESS, DS_UpgradeDef::STATE_INSTALL_MCU_DONE, DS_UpgradeDef::STATE_INSTALL_MCU_FAILED);
                upgradeHwMcu->actUpgrade(digest.mcu.pathFile, guidSubAction);
            }
            else
            {
                actStatusBuf.state = DS_ACTION_STATE_INVALID_STATE;
                setState(DS_UpgradeDef::STATE_INSTALL_MCU_FAILED);
            }
        }
        break;
    case DS_UpgradeDef::STATE_INSTALL_MCU_PROGRESS:
        LOG_INFO("UPGRADE_SRU: STATE_INSTALL_MCU_PROGRESS\n");
        break;
    case DS_UpgradeDef::STATE_INSTALL_MCU_DONE:
        LOG_INFO("UPGRADE_SRU: STATE_INSTALL_MCU_DONE\n");
        setState(DS_UpgradeDef::STATE_INSTALL_HCU_STARTED);
        break;
    case DS_UpgradeDef::STATE_INSTALL_MCU_FAILED:
        LOG_ERROR("UPGRADE_SRU: STATE_INSTALL_MCU_FAILED\n");
        actStatusBuf.err = "MCU Upgrade Failed - " + actStatusBuf.err;
        setState(DS_UpgradeDef::STATE_UPGRADE_FAILED);
        break;
    case DS_UpgradeDef::STATE_INSTALL_HCU_STARTED:
        LOG_INFO("UPGRADE_SRU: STATE_INSTALL_HCU_STARTED\n");
        if (digest.hcu.pathFile == "")
        {
            digest.hcu.progress = 100;
            env->ds.upgradeData->setUpgradeDigest(digest);
            setState(DS_UpgradeDef::STATE_INSTALL_HCU_DONE);
        }
        else
        {
            actStatusBuf.err = checkUpdateFile(digest.hcu);
            if (actStatusBuf.err == "")
            {
                handleSubAction(DS_UpgradeDef::STATE_INSTALL_HCU_PROGRESS, DS_UpgradeDef::STATE_INSTALL_HCU_DONE, DS_UpgradeDef::STATE_INSTALL_HCU_FAILED);
                upgradeHcu->actUpgrade(guidSubAction);
            }
            else
            {
                actStatusBuf.state = DS_ACTION_STATE_INVALID_STATE;
                setState(DS_UpgradeDef::STATE_INSTALL_HCU_FAILED);
            }
        }
        break;
    case DS_UpgradeDef::STATE_INSTALL_HCU_PROGRESS:
        LOG_INFO("UPGRADE_SRU: STATE_INSTALL_HCU_PROGRESS\n");
        break;
    case DS_UpgradeDef::STATE_INSTALL_HCU_DONE:
        LOG_INFO("UPGRADE_SRU: STATE_INSTALL_HCU_DONE\n");
        setState(DS_UpgradeDef::STATE_UPGRADE_DONE);
        break;
    case DS_UpgradeDef::STATE_INSTALL_HCU_FAILED:
        LOG_ERROR("UPGRADE_SRU: STATE_INSTALL_HCU_FAILED\n");
        actStatusBuf.err = "HCU Upgrade Failed - " + actStatusBuf.err;
        setState(DS_UpgradeDef::STATE_UPGRADE_FAILED);
        break;
    case DS_UpgradeDef::STATE_UPGRADE_DONE:
        LOG_INFO("UPGRADE_SRU: STATE_UPGRADE_DONE\n");
        {
            updateSruUpgradeProgress();
            env->ds.mcuAction->actResetMcuHw();
            env->ds.systemAction->actSetStatePath(DS_SystemDef::STATE_PATH_SERVICING);
            actionCompleted(actStatusBuf);

            // Clear ProductSoftwareVersion
            Config::Item cfgProductSoftwareVersion = env->ds.cfgGlobal->getItem_Hidden_ProductSoftwareVersion();
            cfgProductSoftwareVersion.value = "";
            env->ds.cfgGlobal->setHidden_ProductSoftwareVersion(cfgProductSoftwareVersion);

            setState(DS_UpgradeDef::STATE_READY);
        }
        break;
    case DS_UpgradeDef::STATE_UPGRADE_FAILED:
        LOG_ERROR("UPGRADE_SRU:STATE_UPGRADE_FAILED \n");
        digest.sru.err = actStatusBuf.err;
        env->ds.upgradeData->setUpgradeDigest(digest);
        actionCompleted(actStatusBuf);
        env->ds.mcuAction->actLinkConnect();
        setState(DS_UpgradeDef::STATE_READY);
        break;
    default:
        LOG_ERROR("UPGRADE_SRU: Unknown State(%d)\n", digest.state);
        break;
    }
}
