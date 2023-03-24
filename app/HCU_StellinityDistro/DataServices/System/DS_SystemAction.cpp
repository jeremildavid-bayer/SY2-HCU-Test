#include "Apps/AppManager.h"
#include "DS_SystemAction.h"
#include "DS_SystemData.h"
#include "Common/ImrParser.h"
#include "Common/Util.h"
#include "DataServices/CfgGlobal/DS_CfgGlobal.h"
#include "DataServices/CfgLocal/DS_CfgLocal.h"
#include "DataServices/Capabilities/DS_Capabilities.h"
#include "DataServices/Mcu/DS_McuData.h"
#include "DataServices/Mcu/DS_McuAction.h"
#include "DataServices/Mcu/DS_McuDef.h"
#include "DataServices/Cru/DS_CruData.h"
#include "DataServices/Alert/DS_AlertAction.h"
#include "DataServices/HardwareInfo/DS_HardwareInfo.h"

DS_SystemAction::DS_SystemAction(QObject *parent, EnvGlobal *env_) :
    ActionBase(parent, env_)
{
    envLocal = new EnvLocal("DS_System-Action", "SYSTEM_ACTION", LOG_MID_SIZE_BYTES);

    // Start System Monitor Thread
    monitor = new SystemMonitor(this, env);

    multipleScreenShotsParams.actStatus.state = DS_ACTION_STATE_UNKNOWN;
    lastPowerControlType = DS_McuDef::POWER_CONTROL_TYPE_NONE;

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_LinkState, this, [=](DS_McuDef::LinkState linkState) {
        if (linkState != DS_McuDef::LINK_STATE_CONNECTED)
        {
            if (lastPowerControlType != DS_McuDef::POWER_CONTROL_TYPE_NONE)
            {
                LOG_WARNING("MCU Link became %s while waiting for IsShuttingDown=true. Manually setting the flag..\n", ImrParser::ToImr_McuLinkState(linkState).CSTR());
                env->ds.mcuData->setIsShuttingDown(true);
            }
        }
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_IsShuttingDown, this, [=](bool isShuttingDown) {
        if (isShuttingDown)
        {
            if (lastPowerControlType == DS_McuDef::POWER_CONTROL_TYPE_NONE)
            {
                LOG_WARNING("signalDataChanged_IsShuttingDown(): Unexpected Power Off requested from MCU..\n");
                lastPowerControlType = DS_McuDef::POWER_CONTROL_TYPE_OFF;
            }

            LOG_INFO("signalDataChanged_IsShuttingDown(): Stopping all syringe actions before Shutting Down..\n");
            QString guid = Util::newGuid();

            env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus){
                if (curStatus.state == DS_ACTION_STATE_STARTED)
                {
                    env->actionMgr->onActionCompleted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus){
                        LOG_INFO("signalDataChanged_IsShuttingDown(): Syringe actions stopped (%s). Ready to Shut Down.\n", ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());
                        shutdown();
                    });
                }
                else
                {
                    LOG_WARNING("signalDataChanged_IsShuttingDown(): Syringe actions stop failed (%s). Shutting down anyway..\n", ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());
                    shutdown();
                }
            });

            env->ds.mcuAction->actStopAll(guid);
        }
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_StopBtnPressed, this, [=](bool pressed) {
        if (pressed)
        {
            QTimer::singleShot(STOP_BUTTON_LONG_PRESSED_INTERVAL_MS, this, [=] {
                if (env->ds.mcuData->getStopBtnPressed())
                {
                    actSaveUserData();
                }
            });
        }
    });
}

DS_SystemAction::~DS_SystemAction()
{
    procSetSystemDateTime.close();
    procSyncSystemDateTime.close();
    procSetScreenBrightness.close();
    procSetScreenSleepTime.close();
    procScreenWakeup.close();
    procLinkSetup.close();
    procSaveUserData.close();
    procScreenShot.close();
    procFactoryReset.close();
    delete monitor;
    delete envLocal;
}

void DS_SystemAction::slotAppInitialised()
{
    connect(env->ds.systemData, &DS_SystemData::signalDataChanged_StatePath, this, [=](DS_SystemDef::StatePath curStatePath, DS_SystemDef::StatePath prevStatePath) {
        if ( ( (prevStatePath == DS_SystemDef::STATE_PATH_IDLE) && (curStatePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING) ) ||
             ( (prevStatePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING) && (curStatePath == DS_SystemDef::STATE_PATH_IDLE) ) )
        {
            LOG_INFO("\n\n-----------------\nStatePath=%s\n\n", ImrParser::ToImr_StatePath(curStatePath).CSTR());
        }
    });

    connect(env->ds.capabilities, &DS_Capabilities::signalConfigChanged_Developer_DebugMode, this, [=](const Config::Item &cfg) {
        QString value = cfg.value.toString();

        //should I put on a timer - too complicate
        QProcess devModeProc;
        devModeProc.start(PATH_SET_DEV_ENV, QStringList() << value);
        LOG_INFO("%s %s", PATH_SET_DEV_ENV, value.CSTR());
        devModeProc.waitForFinished(-1);
        QString out = devModeProc.readAllStandardOutput();
        LOG_INFO("%s", out.CSTR());
    });

    // Init actions
    env->ds.systemAction->actSetStatePath(DS_SystemDef::STATE_PATH_ON_REACHABLE);
    checkLastShutdown();
    // Check RTC & Battery
    checkLastDatetime(false);
    // check Preventative Maintenance Reminder timer
    checkPreventativeMaintenanceReminder();

    // Mark Safe Shutdown File - Unknown (can be sw or hw)
    QFile fileBuf(PATH_SHUTDOWN_INFO_FILE);
    if (fileBuf.open(QFile::WriteOnly | QFile::Text))
    {
        fileBuf.write(Util::qDateTimeToUtcDateTimeStr(QDateTime::currentDateTimeUtc()).CSTR());
        fileBuf.write("\n");
        fileBuf.write("UNKNOWN");
        fileBuf.write("\n");
        fileBuf.close();
    }

    // initialize network
    initializeNetwork();
}

DataServiceActionStatus DS_SystemAction::actRunSystemProcess(QProcess &proc, QString program, QStringList args, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "RunSystemProcess", QString().asprintf("%s;%s", program.CSTR(), args.join(" ").CSTR()));

    //mask wifi password
    if (program == PATH_SET_NETWORK )
    {
        status.arg.replace(env->ds.cruData->getWifiPassword(), ANONYMIZED_STR);
    }

    if (proc.isOpen())
    {
        status.state = DS_ACTION_STATE_BUSY;
        status.err = "Process Busy";
        actionStarted(status);
        return status;
    }

    QString guid1 = Util::newGuid();
    QString guid2 = Util::newGuid();

    QMetaObject::Connection conn = connect(&proc, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, [=] {
        //LOG_DEBUG("ACT_RUN_SYSTEM_PROCESS: proc=%s, arg=%s: Process completed\n", program.CSTR(), args.join(" ").CSTR());
        env->actionMgr->deleteActCompleted(guid1);
        env->actionMgr->deleteActCompleted(guid2);
        DataServiceActionStatus curStatus = status;
        curStatus.state = DS_ACTION_STATE_COMPLETED;
        actionCompleted(curStatus);
    });

    QMetaObject::Connection conn2 = connect(&proc, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::errorOccurred), this, [=](QProcess::ProcessError err) {
        //LOG_ERROR("ACT_RUN_SYSTEM_PROCESS: proc=%s, arg=%s: Process error(%d)\n", program.CSTR(), args.join(" ").CSTR(), err);
        env->actionMgr->deleteActCompleted(guid1);
        env->actionMgr->deleteActCompleted(guid2);
        DataServiceActionStatus curStatus = status;
        curStatus.state = DS_ACTION_STATE_INTERNAL_ERR;
        curStatus.err = QString().asprintf("ProcErr%d", err);
        actionCompleted(curStatus);
    });

    env->actionMgr->createActCompleted(guid1, conn, QString(__PRETTY_FUNCTION__) + ": actRunSystemProcess: ProcFinished");
    env->actionMgr->createActCompleted(guid2, conn2, QString(__PRETTY_FUNCTION__) + ": actRunSystemProcess: ProcErrorOccurred");

    //LOG_INFO("ACT_RUN_SYSTEM_PROCESS: Process starting: proc=%s, arg=%s\n", program.CSTR(), args.join(" ").CSTR());

    proc.start(program, args);

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);
    return status;
}

DataServiceActionStatus DS_SystemAction::actSetStatePath(DS_SystemDef::StatePath newStatePath, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "SetStatePath", QString().asprintf("%s", ImrParser::ToImr_StatePath(newStatePath).CSTR()));
    DS_SystemDef::StatePath curStatePath = env->ds.systemData->getStatePath();

    // Check State Path Transition
    LOG_INFO("StatePath change requested = %s -> %s\n", ImrParser::ToImr_StatePath(curStatePath).CSTR(), ImrParser::ToImr_StatePath(newStatePath).CSTR());
    bool statePathOk = true;

    if (newStatePath == curStatePath)
    {
        // State path not changed
    }
    else if (newStatePath == DS_SystemDef::STATE_PATH_ERROR)
    {
        // Every path can go to error state
    }
    else
    {
        statePathOk = false;

        switch (curStatePath)
        {
        case DS_SystemDef::STATE_PATH_OFF_UNREACHABLE:
            if (newStatePath == DS_SystemDef::STATE_PATH_ON_REACHABLE)
            {
                statePathOk = true;
            }
            break;
        case DS_SystemDef::STATE_PATH_ON_REACHABLE:
            if ( (newStatePath == DS_SystemDef::STATE_PATH_STARTUP_UNKNOWN) ||
                 (newStatePath == DS_SystemDef::STATE_PATH_SERVICING) )
            {
                statePathOk = true;
            }
            break;
        case DS_SystemDef::STATE_PATH_STARTUP_UNKNOWN:
            if (newStatePath == DS_SystemDef::STATE_PATH_IDLE)
            {
                statePathOk = true;
            }
            break;
        case DS_SystemDef::STATE_PATH_IDLE:
            if ( (newStatePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING) ||
                 (newStatePath == DS_SystemDef::STATE_PATH_READY_ARMED) ||
                 (newStatePath == DS_SystemDef::STATE_PATH_BUSY_WAITING) )
            {
                statePathOk = true;
            }
            break;
        case DS_SystemDef::STATE_PATH_READY_ARMED:
            if ( (newStatePath == DS_SystemDef::STATE_PATH_IDLE) ||
                 (newStatePath == DS_SystemDef::STATE_PATH_BUSY_WAITING) ||
                 (newStatePath == DS_SystemDef::STATE_PATH_EXECUTING) ||
                 (newStatePath == DS_SystemDef::STATE_PATH_BUSY_FINISHING) )
            {
                statePathOk = true;
            }
            break;
        case DS_SystemDef::STATE_PATH_EXECUTING:
            if ( (newStatePath == DS_SystemDef::STATE_PATH_BUSY_HOLDING) ||
                 (newStatePath == DS_SystemDef::STATE_PATH_BUSY_WAITING) ||
                 (newStatePath == DS_SystemDef::STATE_PATH_BUSY_FINISHING) )
            {
                statePathOk = true;
            }
            break;
        case DS_SystemDef::STATE_PATH_BUSY_HOLDING:
            if ( (newStatePath == DS_SystemDef::STATE_PATH_IDLE) ||
                 (newStatePath == DS_SystemDef::STATE_PATH_BUSY_WAITING) ||
                 (newStatePath == DS_SystemDef::STATE_PATH_EXECUTING) ||
                 (newStatePath == DS_SystemDef::STATE_PATH_BUSY_FINISHING) )
            {
                statePathOk = true;
            }
            break;
        case DS_SystemDef::STATE_PATH_BUSY_FINISHING:
            if ( (newStatePath == DS_SystemDef::STATE_PATH_IDLE) ||
                 (newStatePath == DS_SystemDef::STATE_PATH_BUSY_HOLDING) )
            {
                statePathOk = true;
            }
            break;
        case DS_SystemDef::STATE_PATH_BUSY_WAITING:
            if ( (newStatePath == DS_SystemDef::STATE_PATH_IDLE) ||
                 (newStatePath == DS_SystemDef::STATE_PATH_READY_ARMED) )
            {
                statePathOk = true;
            }
            break;
        case DS_SystemDef::STATE_PATH_BUSY_SERVICING:
            if ( (newStatePath == DS_SystemDef::STATE_PATH_IDLE) ||
                 (newStatePath == DS_SystemDef::STATE_PATH_SERVICING) )
            {
                statePathOk = true;
            }
            break;
        case DS_SystemDef::STATE_PATH_SERVICING:
            if ( (newStatePath == DS_SystemDef::STATE_PATH_ON_REACHABLE) ||
                 (newStatePath == DS_SystemDef::STATE_PATH_ERROR) )
            {
                statePathOk = true;
            }
            break;
        case DS_SystemDef::STATE_PATH_ERROR:
            if (newStatePath == DS_SystemDef::STATE_PATH_SERVICING)
            {
                statePathOk = true;
            }
            break;
        default:
            break;
        }
    }

    if (statePathOk)
    {
        status.state = DS_ACTION_STATE_STARTED;
        actionStarted(status);

        if (newStatePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING)
        {
            LOG_INFO("actSetStatePath(): Stopping all syringe actions before entering to Sevice Mode..\n");
            QString guid = Util::newGuid();

            env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus){
                if (curStatus.state == DS_ACTION_STATE_STARTED)
                {
                    env->actionMgr->onActionCompleted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus){
                        LOG_INFO("actSetStatePath(): Syringe actions stopped (%s) Entering Service mode..\n", ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());
                        env->ds.systemData->setStatePath(newStatePath);
                        curStatus.state = DS_ACTION_STATE_COMPLETED;
                        actionCompleted(curStatus, &status);
                    });
                }
                else
                {
                    LOG_INFO("actSetStatePath(): Syringe actions stop failed (%s) Entering Service mode..\n", ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());
                    env->ds.systemData->setStatePath(newStatePath);
                    curStatus.state = DS_ACTION_STATE_COMPLETED;
                    actionCompleted(curStatus, &status);
                }
            });

            env->ds.mcuAction->actStopAll(guid);
            return status;
        }
        env->ds.systemData->setStatePath(newStatePath);
        status.state = DS_ACTION_STATE_COMPLETED;
    }
    else
    {
        LOG_ERROR("actSetStatePath(): Bad StatePath Transition: %s -> %s\n", ImrParser::ToImr_StatePath(curStatePath).CSTR(), ImrParser::ToImr_StatePath(newStatePath).CSTR());
        status.state = DS_ACTION_STATE_BAD_REQUEST;
        status.err = "Bad State Path";
    }

    actionCompleted(status);

    return status;
}

DataServiceActionStatus DS_SystemAction::actScreenWakeup(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "ScreenWakeup");

    QString guid = Util::newGuid();

    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);

        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            env->actionMgr->onActionCompleted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
                if (curStatus.state != DS_ACTION_STATE_COMPLETED)
                {
                    curStatus.err = procScreenWakeup.errorString();
                }
                procScreenWakeup.close();
                actionCompleted(curStatus, &status);
            });
        }
    });

    return actRunSystemProcess(procScreenWakeup, PATH_WAKE_UP, QStringList(), guid);
}

DataServiceActionStatus DS_SystemAction::actSetScreenSleepTime(int sleepTimeMinutes, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "SetScreenSleepTime", QString().asprintf("%d", sleepTimeMinutes));

    // NOTE: if sleepTimeMinutes == 0, sleep is turned off
    QString guid = Util::newGuid();

    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);
        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            env->actionMgr->onActionCompleted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
                if (curStatus.state != DS_ACTION_STATE_COMPLETED)
                {
                    curStatus.err = procSetScreenSleepTime.errorString();
                }
                procSetScreenSleepTime.close();
                actionCompleted(curStatus, &status);
            });

        }
    });

    // Wait for previous process to finish first
    procSetScreenSleepTime.waitForFinished(-1);

    return actRunSystemProcess(procSetScreenSleepTime, PATH_SET_SLEEP_TIME, QStringList() << QString().asprintf("%d", sleepTimeMinutes * 60), guid);
}

DataServiceActionStatus DS_SystemAction::actSetScreenSleepTimeToDefault(QString actGuid)
{
    int timeoutMinutes = env->ds.cfgLocal->get_Settings_Display_ScreenOffTimeoutMinutes();
    return env->ds.systemAction->actSetScreenSleepTime(timeoutMinutes, actGuid);
}

DataServiceActionStatus DS_SystemAction::actSetScreenBrightness(int level, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "SetScreenBrightness", QString().asprintf("%d", level));

    QString guid = Util::newGuid();

    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);
        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            env->actionMgr->onActionCompleted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
                if (curStatus.state != DS_ACTION_STATE_COMPLETED)
                {
                    curStatus.err = procSetScreenBrightness.errorString();
                }
                procSetScreenBrightness.close();
                actionCompleted(curStatus, &status);
            });

        }
    });

   return actRunSystemProcess(procSetScreenBrightness, PATH_SET_BRIGHTNESS, QStringList() << QString().asprintf("%d", level), guid);
}

DataServiceActionStatus DS_SystemAction::actScreenShot(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "ScreenShot");

    QString guid = Util::newGuid();

    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);
        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            env->actionMgr->onActionCompleted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
                if (curStatus.state != DS_ACTION_STATE_COMPLETED)
                {
                    curStatus.err = procScreenShot.errorString();
                }
                procScreenShot.close();
                actionCompleted(curStatus, &status);
            });

        }
    });

    QStringList args = QStringList() << QString().asprintf("[%s]", env->ds.cfgGlobal->get_Settings_General_CultureCode().CSTR())
                                     << QString().asprintf("%d", env->ds.cfgLocal->get_Hidden_ScreenX())
                                     << QString().asprintf("%d", env->ds.cfgLocal->get_Hidden_ScreenY())
                                     << QString().asprintf("%d", env->ds.cfgLocal->get_Hidden_ScreenW())
                                     << QString().asprintf("%d", env->ds.cfgLocal->get_Hidden_ScreenH());

    return actRunSystemProcess(procScreenShot, PATH_SAVE_SCREENSHOT, args, guid);
}

DataServiceActionStatus DS_SystemAction::actMultipleScreenShots(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "MultipleScreenShots");

    if (multipleScreenShotsParams.actStatus.state == DS_ACTION_STATE_STARTED)
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        actionStarted(status);
        return status;
    }

    multipleScreenShotsParams.actStatus = status;
    multipleScreenShotsParams.actStatus.state = DS_ACTION_STATE_STARTED;
    actionStarted(multipleScreenShotsParams.actStatus);

    multipleScreenShotsParams.cultureCodeCfg = env->ds.cfgGlobal->getItem_Settings_General_CultureCode();
    multipleScreenShotsParams.origCultureCode = multipleScreenShotsParams.cultureCodeCfg.value.toString();
    multipleScreenShotsParams.cultureCodeIndex = 0;

    actMultipleScreenShotsInner();

    return multipleScreenShotsParams.actStatus;
}

void DS_SystemAction::actMultipleScreenShotsInner()
{
    if (multipleScreenShotsParams.cultureCodeIndex >= multipleScreenShotsParams.cultureCodeCfg.validList.length())
    {
        // all screenshots taken
        multipleScreenShotsParams.actStatus.state = DS_ACTION_STATE_COMPLETED;

        // restore config
        multipleScreenShotsParams.cultureCodeCfg.value = multipleScreenShotsParams.origCultureCode;
        env->ds.cfgGlobal->setSettings_General_CultureCode(multipleScreenShotsParams.cultureCodeCfg);

        actionCompleted(multipleScreenShotsParams.actStatus);
        return;
    }

    QVariant newCfgValue = multipleScreenShotsParams.cultureCodeCfg.validList[multipleScreenShotsParams.cultureCodeIndex];
    multipleScreenShotsParams.cultureCodeCfg.value = newCfgValue.toString();
    env->ds.cfgGlobal->setSettings_General_CultureCode(multipleScreenShotsParams.cultureCodeCfg);

    QString guid = Util::newGuid();

    env->actionMgr->onActionCompleted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus ) {
        multipleScreenShotsParams.cultureCodeIndex++;
        QTimer::singleShot(20, this, [=] {
            actMultipleScreenShotsInner();
        });
    });

    actScreenShot(guid);
}

DataServiceActionStatus DS_SystemAction::actSetSystemDateTime(QDateTime dateTime, QString actGuid)
{
    // Convert CRU time to UTC
    int utcOffsetSeconds = env->ds.cfgLocal->get_Hidden_CurrentUtcOffsetMinutes() * -60;
    QDateTime dateTimeUtc = dateTime.addSecs(utcOffsetSeconds);

    LOG_INFO("DateTime change is requested (dateTime=%s, dateTimeUTC=%s, utcOffsetSeconds=%d)\n",
             dateTime.toString("yyyy/MM/dd hh:mm").CSTR(),
             dateTimeUtc.toString("yyyy/MM/dd hh:mm").CSTR(),
             utcOffsetSeconds);

    DataServiceActionStatus status = actionInit(actGuid, "SetSystemDateTime", QString().asprintf("%s", dateTimeUtc.toString("yyyy/MM/dd hh:mm").CSTR()));

    QString guid = Util::newGuid();

    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);
        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            env->actionMgr->onActionCompleted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
                if (curStatus.state != DS_ACTION_STATE_COMPLETED)
                {
                    curStatus.err = procSetSystemDateTime.errorString();
                }
                procSetSystemDateTime.close();
                actionCompleted(curStatus, &status);
            });
        }
    });

   return actRunSystemProcess(procSetSystemDateTime, PATH_SET_DATETIME, QStringList() << dateTimeUtc.toString("yyyy-MM-dd hh:mm:00"), guid);
}

DataServiceActionStatus DS_SystemAction::actSyncSystemDateTime(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "SyncSystemDateTime");

    QString guid = Util::newGuid();

    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        curStatus.guid = actGuid;
        actionStarted(curStatus);
        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            env->actionMgr->onActionCompleted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
                QString stdErr = procSyncSystemDateTime.readAllStandardError();
                QString stdOut = procSyncSystemDateTime.readAllStandardOutput();

                LOG_DEBUG("actSyncSystemDateTime(): procSyncSystemDateTime: stdOut=%s, stdErr=%s\n", stdOut.CSTR(), stdErr.CSTR());
                if (curStatus.state != DS_ACTION_STATE_COMPLETED)
                {
                    curStatus.err = procSyncSystemDateTime.errorString();
                }
                procSyncSystemDateTime.close();
                actionCompleted(curStatus, &status);

                // Update Datetime for RTC check.
                checkLastDatetime(false);
            });
        }
    });

   return actRunSystemProcess(procSyncSystemDateTime, PATH_SYNC_DATETIME, QStringList(), guid);
}

DataServiceActionStatus DS_SystemAction::actNetworkInterfaceUp(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "NetworkInterfaceUp");

    // Reset CRU-State
    QString cruLinkType = env->ds.cfgLocal->get_Settings_SruLink_ConnectionType();
    DS_CruDef::CruLinkStatus linkStatus;
    linkStatus.type = ImrParser::ToCpp_CruLinkType(cruLinkType);
    linkStatus.state = DS_CruDef::CRU_LINK_STATE_INACTIVE;
    linkStatus.quality = DS_CruDef::CRU_LINK_QUALITY_POOR;
    linkStatus.signalLevel = "";
    env->ds.cruData->setCruLinkStatus(linkStatus);

    if (procLinkSetup.isOpen())
    {
        status.state = DS_ACTION_STATE_BUSY;
        status.err = "Link Setup In Progress";
        actionStarted(status);
        return status;
    }

    // Initialise Network Params
    DS_SystemDef::NetworkSettingParams params;
    actGetNetworkSettingParams(params);
    env->ds.systemData->setNetworkSettingParams(params);

    // Check Conditions
    if (params.activeIface == "")
    {
        status.state = DS_ACTION_STATE_BAD_REQUEST;
        status.err = "Interface is not ready";
        actionStarted(status);
        return status;
    }
    else if ( (params.isWifiType) &&
              (params.ssid == "") )
    {
        status.state = DS_ACTION_STATE_BAD_REQUEST;
        status.err = "Bad SSID value";
        actionStarted(status);
        return status;
    }
    else if ( (!params.isDhcpMode) &&
              (params.localIp == "") )
    {
        status.state = DS_ACTION_STATE_BAD_REQUEST;
        status.err = "Bad IP Address value";
        actionStarted(status);
        return status;
    }

    // Deactivate inactive interface
    env->ds.systemAction->actNetworkInterfaceDown(params.inactiveIface);

    // Setup network
    QString paramStr = QString().asprintf("%s/%s/%s/%s/%s",
                                         params.localIp.CSTR(),
                                         params.netmask.CSTR(),
                                         params.ssid.CSTR(),
                                         params.pwd.CSTR(),
                                         params.countryCode.CSTR());

    QStringList lstParams = QStringList() << "connect" << params.activeIface << paramStr;

    QString guid = Util::newGuid();

    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);
        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            env->actionMgr->onActionCompleted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
                if (curStatus.state == DS_ACTION_STATE_COMPLETED)
                {
                    DS_SystemDef::NetworkSettingParams params = env->ds.systemData->getNetworkSettingParams();
                    params.setupCompletedEpochMs = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
                    env->ds.systemData->setNetworkSettingParams(params);

                    curStatus = handleSetNetworkProcess(curStatus);
                }

                actionCompleted(curStatus, &status);
            });
        }
    });

    params.setupCompletedEpochMs = -1;
    env->ds.systemData->setNetworkSettingParams(params);

    return actRunSystemProcess(procLinkSetup, PATH_SET_NETWORK, lstParams, guid);
}

DataServiceActionStatus DS_SystemAction::actNetworkInterfaceDown(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "NetworkInterfaceDown");

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);

    DS_SystemDef::NetworkSettingParams params = env->ds.systemData->getNetworkSettingParams();

    // TODO: use set_network script
    LOG_DEBUG("ACT_NETWORK_INTERFACE_DOWN: Ifconfig down %s ..\n", params.inactiveIface.CSTR());
    QProcess proc;
    proc.start("sh", QStringList() << "-c" << QString().asprintf("sudo ifconfig %s down", params.inactiveIface.CSTR()));
    proc.waitForFinished(-1);
    QString procOut = proc.readAllStandardOutput().trimmed();

    LOG_INFO("ACT_NETWORK_INTERFACE_DOWN: Proc(%s, arg=%s) returned out=%s\n",
             proc.program().CSTR(),
             proc.arguments().join(" ").CSTR(),
             procOut.CSTR());


    status.state = DS_ACTION_STATE_COMPLETED;
    actionCompleted(status);

    return status;
}

DataServiceActionStatus DS_SystemAction::handleSetNetworkProcess(DataServiceActionStatus status)
{
    QString stdErr = procLinkSetup.readAllStandardError();
    QString stdOut = procLinkSetup.readAllStandardOutput();

    QString logStdOut = stdOut;
    
    LOG_INFO("HANDLE_SET_NETWORK_PROCESS: Process Completed: %s, arg=%s, out=%s\n", procLinkSetup.program().CSTR(), procLinkSetup.arguments().join(" ").replace(env->ds.cruData->getWifiPassword(), ANONYMIZED_STR).CSTR(), logStdOut.replace(env->ds.cruData->getWifiPassword(), ANONYMIZED_STR).CSTR());
    procLinkSetup.close();

    if ( (stdOut.contains("NETWORK SETUP COMPLETED")) &&
         (stdErr == "") )
    {
        // Network setup ok
        env->ds.alertAction->deactivate("HCUNetworkSetupFailed");
    }
    else
    {
        QString oldAlertGuid = env->ds.alertAction->getActiveAlertGuid("HCUNetworkSetupFailed", "", true);
        if (oldAlertGuid != EMPTY_GUID)
        {
            QVariantMap oldAlert = env->ds.alertAction->getFromGuid(oldAlertGuid);
            if (oldAlert.contains("Data"))
            {
                QString oldData = oldAlert["Data"].toString();
                if (oldData != stdErr)
                {
                    // Deactivate last HCUNetworkSetupFailed alert as the data is changed
                    env->ds.alertAction->deactivate("HCUNetworkSetupFailed");
                }
            }
        }

        LOG_ERROR("HANDLE_SET_NETWORK_PROCESS: Failed to set network (err=%s)\n", stdErr.CSTR());
        status.err = stdErr;

        if (!env->ds.alertAction->isActivated("HCUNetworkSetupFailed", "", true))
        {
            env->ds.alertAction->activate("HCUNetworkSetupFailed", stdErr);
        }
    }
    return status;
}

DataServiceActionStatus DS_SystemAction::actGetNetworkSettingParams(DS_SystemDef::NetworkSettingParams &params, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "GetNetworkSettingParams");

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);

    params.init();

    params.localIp = env->ds.cfgLocal->get_Hidden_HcuIp();
    params.netmask = "255.255.255.0";
    params.localPort = env->ds.cfgLocal->get_Hidden_HcuPort();
    params.routerIp = env->ds.cfgLocal->get_Hidden_CruRouterIp();

    QString linkType = env->ds.cfgLocal->get_Settings_SruLink_ConnectionType();
    QString baseBoardType = env->ds.hardwareInfo->getGeneral_BaseBoardType().value.toString();

    if ( (linkType == "WirelessEthernet") &&
         ((baseBoardType == "OCS") || (baseBoardType == "NoBattery")) )
    {
        LOG_WARNING("actGetNetworkSettingParams(): LinkType=%s AND BaseBoardType=%s. Forcing the network type to WiredEthernet..\n", linkType.CSTR(), baseBoardType.CSTR());
        linkType = "WiredEthernet";
    }

    if (linkType == "WirelessEthernet")
    {
        params.isWifiType = true;
        params.activeIface = env->ds.capabilities->get_Network_WifiInterface();
        params.inactiveIface = env->ds.capabilities->get_Network_EthernetInterface();

        params.isDhcpMode = false;
        params.ssid = env->ds.cruData->getWifiSsid();
        params.pwd = env->ds.cruData->getWifiPassword();
        params.serverIp = env->ds.cfgLocal->get_Hidden_CruIpWireless();

        // Set Country Code: AA-x
        // AA: 2 alphas country code as per https://en.wikipedia.org/wiki/ISO_3166-1_alpha-2,
        // x: base 10 numeric country code
        QString countryCode = env->ds.cfgGlobal->get_Settings_SruLink_WirelessCountryCode();
        QStringList countryCodeList = countryCode.split("-");
        QString countryCodeNum = "";

        if (countryCodeList.length() > 1)
        {
            //the wifi driver setting can be either "OpenSource" or "Silex"
            QString wifi_driver = env->ds.capabilities->get_Developer_WifiDriver();
            if (wifi_driver == "Silex")
            {// Silex uses numeric country code
                QString countryCodeNumBuf = countryCodeList[1];

                // Remove leading 0 for countryCode number
                for (int i = 0; i < countryCodeNumBuf.length(); i++)
                {
                    if ( (countryCodeNumBuf[i] != '0') ||
                         (countryCodeNum != "") )
                    {
                        countryCodeNum += countryCodeNumBuf[i];
                    }
                }
            }
            else if (wifi_driver == "OpenSource")
            {// Open source/iw uses ISO_3166-1_alpha-2 country code
                countryCodeNum = countryCodeList[0];
            }
            else
            {// not expect here unless there is a new driver option
                LOG_ERROR("TODO new wifi driver option %s \n", wifi_driver.CSTR());
                countryCodeNum = countryCodeList[0];
            }
        }
        params.countryCode = countryCodeNum;
    }
    else
    {
        params.isWifiType = false;
        params.activeIface = env->ds.capabilities->get_Network_EthernetInterface();
        params.inactiveIface = env->ds.capabilities->get_Network_WifiInterface();

        params.isDhcpMode = false;
        params.serverIp = env->ds.cfgLocal->get_Hidden_CruIpWired();
    }

    QString networkSettingParams = Util::qVarientToJsonData(ImrParser::ToImr_NetworkSettingParams(params));
    LOG_DEBUG("actGetNetworkSettingParams(): Params = %s\n", networkSettingParams.replace(params.pwd, ANONYMIZED_STR).CSTR());

    status.state = DS_ACTION_STATE_COMPLETED;
    actionCompleted(status);

    return status;
}

DataServiceActionStatus DS_SystemAction::actSaveUserData(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "SaveUserData");

    // Get MCU Calibration Data First
    LOG_INFO("actSaveUserData(): Getting MCU Calibration data..\n");
    QString guidCalDigest = Util::newGuid();
    env->actionMgr->onActionStarted(guidCalDigest, __PRETTY_FUNCTION__, [=](DataServiceActionStatus) {
        LOG_INFO("actSaveUserData(): Saving User Data..\n");

        // Save User Data
        QString guid = Util::newGuid();
        env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
            actionStarted(curStatus, &status);
            if (curStatus.state == DS_ACTION_STATE_STARTED)
            {
                env->actionMgr->onActionCompleted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
                    if (curStatus.state != DS_ACTION_STATE_COMPLETED)
                    {
                        curStatus.err = procSaveUserData.errorString();
                    }
                    procSaveUserData.close();
                    actionCompleted(curStatus, &status);
                });
            }
        });
        actRunSystemProcess(procSaveUserData, PATH_SAVE_USER_DATA, QStringList(), guid);
    });
    env->ds.mcuAction->actCalDigest(guidCalDigest);
    status.state = DS_ACTION_STATE_START_WAITING;
    return status;
}

DataServiceActionStatus DS_SystemAction::actFactoryReset(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "FactoryReset");

    QString guid = Util::newGuid();

    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);
        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            env->actionMgr->onActionCompleted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
                if (curStatus.state != DS_ACTION_STATE_COMPLETED)
                {
                    curStatus.err = procFactoryReset.errorString();
                }
                procFactoryReset.close();
                actionCompleted(curStatus, &status);
            });
        }
    });
    return actRunSystemProcess(procFactoryReset, PATH_FACTORY_RESET, QStringList(), guid);
}

DataServiceActionStatus DS_SystemAction::actShutdown(DS_McuDef::PowerControlType powerControlType, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "Shutdown", ImrParser::ToImr_PowerControlType(powerControlType));
    QString guid = Util::newGuid();
    lastPowerControlType = powerControlType;

    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);

        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            LOG_DEBUG("ACT_SHUT_DOWN: Shutdown is accepted in MCU. MCU will set system power state soon.\n");
            curStatus.state = DS_ACTION_STATE_COMPLETED;
        }
        else if (curStatus.err == "DISCONNECTED")
        {
            LOG_WARNING("ACT_SHUT_DOWN: MCU is not connected. HCU shutdown is continued anyway..\n");
            curStatus.state = DS_ACTION_STATE_INVALID_STATE;
            env->ds.mcuData->setIsShuttingDown(true);
        }
        else
        {
            LOG_ERROR("ACT_SHUT_DOWN: HCU shutdown is rejected.\n");
        }
        actionCompleted(curStatus);
    });

    checkLastDatetime(true);
    env->ds.mcuAction->actPower(lastPowerControlType, guid);
    status.state = DS_ACTION_STATE_START_WAITING;
    return status;
}

DataServiceActionStatus DS_SystemAction::actSafeExit(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "SafeExit");

    env->state = EnvGlobal::STATE_EXITING;

    // Mark Safe Shutdown File
    QFile fileBuf(PATH_SHUTDOWN_INFO_FILE);
    if (fileBuf.open(QFile::WriteOnly | QFile::Text))
    {
        fileBuf.write(Util::qDateTimeToUtcDateTimeStr(QDateTime::currentDateTimeUtc()).CSTR());
        fileBuf.write("\n");
        fileBuf.write("NORMAL");
        fileBuf.write("\n");
        fileBuf.close();
    }

    env->ds.alertAction->activate("HCUApplicationStopped");

    int exitDistroWaitTimeMs = qMax(POWER_OFF_WAIT_TIME_MS / 2, 100);

    QTimer::singleShot(exitDistroWaitTimeMs, this, [=]() {
        // Gives time for the alert to be saved to file before initiating shutdown process
        emit env->signalExitDistro();
        env->ds.alertAction->saveLastAlerts();
    });

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);
    status.state = DS_ACTION_STATE_COMPLETED;
    actionCompleted(status);

    return status;
}

QVariantMap DS_SystemAction::readJsonFile(QString fileName, bool alertEnable = false)
{
    QFile fileBuf(fileName);
    QJsonDocument document;
    QJsonParseError parseErr;
    QVariantMap jsonMap;

    if (fileBuf.exists())
    {
        if (fileBuf.open(QFile::ReadOnly | QFile::Text))
        {
            QString lastStatusFile = fileBuf.readAll();
            fileBuf.close();

            document = QJsonDocument::fromJson(lastStatusFile.toUtf8(), &parseErr);
            if (document.isNull() || document.isObject())
                jsonMap = document.object().toVariantMap();
        }
        if (alertEnable && (parseErr.error != QJsonParseError::NoError))
        {
            QString err = QString().asprintf("Could (%s) a bad file.\n", fileName.CSTR());
            LOG_ERROR("%s", err.CSTR());
            env->ds.alertAction->activate("HCUInternalSoftwareError", err);
        }
    }

    return jsonMap;
}

void DS_SystemAction::writeJsonFile(QString fileName, QVariantMap jObj, bool alertEnable = false)
{
    QFile fileBuf(fileName);

    if (fileBuf.open(QFile::WriteOnly | QFile::Text))
    {
        QByteArray jsonStr = Util::qVarientToJsonData(jObj, false);
        fileBuf.write(jsonStr);
        fileBuf.close();
    }
    else {
        if (alertEnable)
        {
            QString err = QString().asprintf("Could not write file (%s).\n", fileName.CSTR());
            LOG_ERROR("%s", err.CSTR());
            env->ds.alertAction->activate("HCUInternalSoftwareError", err);
        }
    }
}


void DS_SystemAction::checkLastDatetime(bool suppressPopup)
{
    QString lastGoodDateTime = "LAST_GOOD_DATETIME";
    bool updateTime = false;
    QDateTime   currDatetime = QDateTime::currentDateTimeUtc();
    QFile fileBuf(PATH_LAST_STATUS_FILE);
    if (!fileBuf.exists())//If the file doesn't exist yet, we don't have a last date time. We must update to our current date time.
    {
        updateTime = true;
    }

    try
    {
        QVariantMap jsonObject = readJsonFile(PATH_LAST_STATUS_FILE);

        if (!updateTime)
        {
            QDateTime lastDatetime = currDatetime;
            if (jsonObject.contains(lastGoodDateTime))
            {
                QDateTime dateTime = Util::utcDateTimeStrToQDateTime(jsonObject[lastGoodDateTime].toString());
                lastDatetime = dateTime;
            }
            else
            {
                updateTime = true;
                jsonObject.insert(lastGoodDateTime, currDatetime);
            }

            if ((currDatetime.daysTo(lastDatetime) > HCU_CLOCK_BATTERY_DEAD_MAX_DAYS) && !suppressPopup)
            {
                QString err = QString().asprintf("Last good date/time is (%s). Please check RTC battery.\n", lastDatetime.toString().CSTR());
                LOG_ERROR("%s", err.CSTR());
                QString data = Util::qDateTimeToUtcDateTimeStr(lastDatetime) + ";" + Util::qDateTimeToUtcDateTimeStr(currDatetime);
                env->ds.alertAction->activate("HCUClockBatteryDead", data);
            }

            if (currDatetime > lastDatetime)
            {
                updateTime = true;
            }
        }

        if (updateTime)
        {
            QString dateTimeString = Util::qDateTimeToUtcDateTimeStr(currDatetime);
            jsonObject[lastGoodDateTime] = dateTimeString;
            writeJsonFile(PATH_LAST_STATUS_FILE, jsonObject, true);
        }
    }
    catch (std::exception &e) {
        LOG_INFO("checkLastDatetime(): \n%s\n", e.what());
    }
}



void DS_SystemAction::checkLastShutdown()
{
    QFile fileBuf(PATH_SHUTDOWN_INFO_FILE);
    if (fileBuf.exists())
    {
        if (fileBuf.open(QFile::ReadOnly | QFile::Text))
        {
            QString shutDownInfoFileRead = fileBuf.readAll();

            LOG_INFO("checkLastShutdown(): ShutdownInfoFile:\n%s\n", shutDownInfoFileRead.CSTR());

            if (shutDownInfoFileRead.contains(_L("NORMAL")))
            {
                // Normal exit observed
            }
            else
            {
                QString shutDownReason = shutDownInfoFileRead.contains(_L("SW")) ? "SW" : "HW";
                QString buildType = env->ds.systemData->getBuildType();

                if ( (buildType == BUILD_TYPE_SQA) ||
                     (buildType == BUILD_TYPE_VNV) ||
                     (buildType == BUILD_TYPE_REL) )
                {
                    env->ds.alertAction->activate("HCUApplicationCrashed", shutDownReason);
                }
            }
            fileBuf.close();
        }
    }
    else
    {
        QString err = QString().asprintf("Failed to open ShutdownInfoFile (path=%s).\n", PATH_SHUTDOWN_INFO_FILE);
        LOG_ERROR("%s", err.CSTR());
        env->ds.alertAction->activate("HCUInternalSoftwareError", err);
    }
}

void DS_SystemAction::checkPreventativeMaintenanceReminder()
{
    int epochSecsLastPM = env->ds.cfgLocal->get_Hidden_PMLastPerformedAt();

    // only check if PM last is set (set by first CRU connection)
    if (epochSecsLastPM != 0)
    {
        QDateTime lastPMPerformed = QDateTime::fromSecsSinceEpoch(epochSecsLastPM);
        QDateTime now = QDateTime::currentDateTimeUtc();
        qint64 daysPastLastPM = lastPMPerformed.daysTo(now);

        if (daysPastLastPM > 365)
        {
            int epochSecLastReminder = env->ds.cfgLocal->get_Hidden_PMLastReminderAt();
            if (epochSecLastReminder == 0)
            {
                // it's not set. Set to NOW
                int value = QDateTime::currentSecsSinceEpoch();
                env->ds.cfgLocal->set_Hidden_PMLastReminderAt(value);
            }

            QDateTime lastReminderDate = QDateTime::fromSecsSinceEpoch(env->ds.cfgLocal->get_Hidden_PMLastReminderAt());
            qint64 daysPastLastReminder = lastReminderDate.daysTo(now);
            int reminderFreqDays = env->ds.cfgLocal->get_Settings_Injector_PMReminderFrequency().toInt();
            if ((reminderFreqDays != -1) && (daysPastLastReminder >= reminderFreqDays))
            {
                QString daysInString = QString::number(daysPastLastPM);
                env->ds.alertAction->activate("PreventativeMaintenanceReminder", daysInString);
            }
        }
    }
}


void DS_SystemAction::initializeNetwork()
{
    QString curEthernetName = env->ds.capabilities->get_Network_EthernetInterface();
    procCheckEthernetInterfaceName.start(PATH_CHECK_ETHERNET_INTERFACE_NAME, QStringList() << curEthernetName);
    connect(&procCheckEthernetInterfaceName, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [=](int exitCode, QProcess::ExitStatus) {
        if (exitCode)
        {
            QString newEthernetName;
            if (curEthernetName == "enp3s0")
            {
                newEthernetName = QString().asprintf("%s","eno1");
            }
            else if (curEthernetName == "eno1")
            {
                newEthernetName = QString().asprintf("%s","enp3s0");
            }
            LOG_INFO("initializeNetwork(): ExitStatus = %d. Renaming ethernet interface from %s to %s. \n", exitCode, curEthernetName.CSTR(), newEthernetName.CSTR());
            env->ds.capabilities->set_Network_EthernetInterface(newEthernetName);
        }

        // Initialise network interface params
        DS_SystemDef::NetworkSettingParams params;
        actGetNetworkSettingParams(params);
        env->ds.systemData->setNetworkSettingParams(params);

        procCheckEthernetInterfaceName.close();
    });
}

void DS_SystemAction::shutdown()
{
    LOG_INFO("\n");
    LOG_INFO("\n");
    LOG_INFO("==================================\n");
    LOG_INFO("%s requested from MCU. Executing in %.1fsec..\n", ImrParser::ToImr_PowerControlType(lastPowerControlType).CSTR(), POWER_OFF_WAIT_TIME_MS / 1000.0f);
    actSafeExit();

    QTimer::singleShot(POWER_OFF_WAIT_TIME_MS, this, [=](){
        if (lastPowerControlType == DS_McuDef::POWER_CONTROL_TYPE_OFF)
        {
            LOG_INFO("Shutting down now\n");
            (void)system(PATH_SYSTEM_SHUTDOWN);
        }
        else
        {
            LOG_INFO("Rebooting now\n");
            (void)system(PATH_SYSTEM_REBOOT);
        }
    });
}
