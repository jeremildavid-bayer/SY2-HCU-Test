#include "Apps/AppManager.h"
#include "SystemMonitorOS.h"
#include "Common/ImrParser.h"
#include "DataServices/Alert/DS_AlertAction.h"
#include "DataServices/Cru/DS_CruData.h"
#include "DataServices/System/DS_SystemData.h"
#include "DataServices/Capabilities/DS_Capabilities.h"
#include "DataServices/CfgLocal/DS_CfgLocal.h"

SystemMonitorOS::SystemMonitorOS(QObject *parent, EnvGlobal *env_) :
    QObject(parent),
    env(env_)
{
    lastCruLinkStateIsInactive = true; //Startup state of comms is down

    envLocal = new EnvLocal("DS_System-MonitorOS", "SYSTEM_MONITOR_OS", LOG_MID_SIZE_BYTES);

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });

    // Delete old unused-mounted directories
    (void)system(PATH_DELETE_ALL_USB_DIRS);
}

SystemMonitorOS::~SystemMonitorOS()
{
    procMonitorTimeShifted.close();
    procMonitorMainHdd.close();
    procMonitorUserHdd.close();
    procMonitorUsbStickInserted.close();
    procMonitorHcuTemperatureHw.close();
    procMonitorCpuMemUsage.close();
    procMonitorCruLink.close();

    delete envLocal;
}

void SystemMonitorOS::slotAppInitialised()
{
    if (env->ds.systemData->getBuildType() == _L(BUILD_TYPE_DEV))
    {
        // Create Upgrade Directory shortcut
        (void)system(QString().asprintf("sudo ln -s %s %s/MCU_SC_FIRMWARES", PATH_UPGRADE_DIR, PATH_USB).CSTR());
    }

    tmrMonitorTimeShifted.setSingleShot(true);
    tmrMonitorMainHdd.setSingleShot(true);
    tmrMonitorUserHdd.setSingleShot(true);
    tmrMonitorUsbStickInserted.setSingleShot(true);
    tmrMonitorHcuTemperatureHw.setSingleShot(true);
    tmrMonitorCpuMemUsage.setSingleShot(true);
    tmrMonitorCruLink.setSingleShot(true);

    connect(&tmrMonitorTimeShifted, &QTimer::timeout, this, [=] {
        tmrMonitorTimeShifted.stop();
        slotMonitorTimeShifted();
    });

    connect(&tmrMonitorMainHdd, &QTimer::timeout, this, [=] {
        tmrMonitorMainHdd.stop();
        LOG_DEBUG("procMonitorMainHdd Started\n");
        procMonitorMainHdd.start("sh", QStringList() << "-c" << QString().asprintf("df |grep IMAX_ACTIVE |awk '{print $4}'"));
    });
    connect(&procMonitorMainHdd, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, [=] { slotProcMonitorMainHddFinished(""); });
    connect(&procMonitorMainHdd, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::errorOccurred), this, [=](QProcess::ProcessError err) { slotProcMonitorMainHddFinished(QString().asprintf("%d", err)); });

    connect(&tmrMonitorUserHdd, &QTimer::timeout, this, [=] {
        tmrMonitorUserHdd.stop();
        LOG_DEBUG("procMonitorUserHdd Started\n");
        procMonitorUserHdd.start("sh", QStringList() << "-c" << QString().asprintf("df |grep %s |awk '{print $4}'", PATH_USER));
    });
    connect(&procMonitorUserHdd, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, [=] { slotProcMonitorUserHddFinished(""); });
    connect(&procMonitorUserHdd, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::errorOccurred), this, [=](QProcess::ProcessError err) { slotProcMonitorUserHddFinished(QString().asprintf("%d", err)); });

    connect(&tmrMonitorUsbStickInserted, &QTimer::timeout, this, [=] {
        tmrMonitorUsbStickInserted.stop();
        LOG_DEBUG("procMonitorUserHdd Started\n");
        procMonitorUsbStickInserted.start("sh", QStringList() << "-c" << QString().asprintf("lsblk |grep %s |awk {'print $7'}", PATH_USB));
    });
    connect(&procMonitorUsbStickInserted, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, [=] { slotProcMonitorUsbStickInsertedFinished(""); });
    connect(&procMonitorUsbStickInserted, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::errorOccurred), this, [=](QProcess::ProcessError err) { slotProcMonitorUsbStickInsertedFinished(QString().asprintf("%d", err)); });

    connect(&tmrMonitorHcuTemperatureHw, &QTimer::timeout, this, [=] {
        tmrMonitorHcuTemperatureHw.stop();
        LOG_DEBUG("procMonitorHcuTemperatureHw Started\n");
        procMonitorHcuTemperatureHw.start(PATH_GET_TEMPERATURE_PARAMS, QStringList());
    });
    connect(&procMonitorHcuTemperatureHw, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, [=] { slotProcMonitorHcuTemperatureHwFinished(""); });
    connect(&procMonitorHcuTemperatureHw, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::errorOccurred), this, [=](QProcess::ProcessError err) { slotProcMonitorHcuTemperatureHwFinished(QString().asprintf("%d", err)); });

    connect(&tmrMonitorCpuMemUsage, &QTimer::timeout, this, [=] {
        tmrMonitorCpuMemUsage.stop();
        LOG_DEBUG("procMonitorCpuMemUsage Started\n");
        procMonitorCpuMemUsage.start("/home/user/Imaxeon/script/get_cpu_mem_usage.sh", QStringList());
    });
    connect(&procMonitorCpuMemUsage, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, [=] { slotProcMonitorCpuMemUsageFinished(""); });
    connect(&procMonitorCpuMemUsage, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::errorOccurred), this, [=](QProcess::ProcessError err) { slotProcMonitorCpuMemUsageFinished(QString().asprintf("%d", err)); });

    connect(&tmrMonitorCruLink, &QTimer::timeout, this, [=] {
        tmrMonitorCruLink.stop();

        DS_SystemDef::NetworkSettingParams params = env->ds.systemData->getNetworkSettingParams();
        qint64 timeSinceNetworkSetupComplete = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch() - params.setupCompletedEpochMs;
        // timeSinceNetworkSetupComplete can be negative if hcu was in future prior to time sync with CRU
        if ((timeSinceNetworkSetupComplete > 0) && (timeSinceNetworkSetupComplete < SYSTEM_MONITOR_INTERVAL_MS_SETUP_CRU_LINK))
        {
            LOG_DEBUG("Link setup completed less than %lldms. Try Ping later..\n", timeSinceNetworkSetupComplete);
            tmrMonitorCruLink.start(SYSTEM_MONITOR_INTERVAL_MS_SETUP_CRU_LINK * 0.5);
            return;
        }

        if (env->ds.cruData->getCruLinkStatus().state != DS_CruDef::CRU_LINK_STATE_INACTIVE)
        {
            // Link is up, no need to repair
            tmrMonitorCruLink.start(SYSTEM_MONITOR_INTERVAL_MS_SETUP_CRU_LINK);
            return;
        }

        // Apply CRU Link Recover Control: If CRU is not responding, set the network again. (Sometimes this helps to reconnect)
        QString cruLinkType = env->ds.cfgLocal->get_Settings_SruLink_ConnectionType();
        QString cruIp = "";
        if (cruLinkType == _L("WirelessEthernet"))
        {
            cruIp = env->ds.cfgLocal->get_Hidden_CruIpWireless();
        }
        else if (cruLinkType == _L("WiredEthernet"))
        {
            cruIp = env->ds.cfgLocal->get_Hidden_CruIpWired();
        }
        else
        {
            LOG_ERROR("Unexpected CruLinkType(%s)\n", cruLinkType.CSTR());
            return;
        }
        LOG_DEBUG("procMonitorCruLink Started\n");
        procMonitorCruLink.start(PATH_PING_TEST, QStringList() << cruIp);
    });

    connect(&procMonitorCruLink, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, [=] { slotProcMonitorCruLinkFinished(""); });
    connect(&procMonitorCruLink, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::errorOccurred), this, [=](QProcess::ProcessError err) { slotProcMonitorCruLinkFinished(QString().asprintf("%d", err)); });

    connect(&tmrMonitorIwconfig, &QTimer::timeout, this, [=] {
        tmrMonitorIwconfig.stop();

        QString cruLinkType = env->ds.cfgLocal->get_Settings_SruLink_ConnectionType();
        if (cruLinkType != _L("WirelessEthernet"))
        {
            // Clear out iwconfig data and wait for the connection type to be wireless
            iwconfigParams.init();

            LOG_DEBUG("tmrMonitorIwconfig Link type is NOT configured for wireless. Try Iwconfig later..\n");
            tmrMonitorIwconfig.start(SYSTEM_MONITOR_INTERVAL_MS_IWCONFIG);
            return;
        }

        QString wifiInterface = env->ds.capabilities->get_Network_WifiInterface();

        LOG_DEBUG("procMonitorIwconfig Started\n");
        procMonitorIwconfig.start(PATH_IWCONFIG_TEST, QStringList() << wifiInterface);
    });
    connect(&procMonitorIwconfig, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, [=] { slotProcMonitorIwconfigFinished(""); });
    connect(&procMonitorIwconfig, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::errorOccurred), this, [=](QProcess::ProcessError err) { slotProcMonitorIwconfigFinished(QString().asprintf("%d", err)); });


    // Start timers
    tmrMonitorTimeShifted.start(1);
    tmrMonitorMainHdd.start(1);
    tmrMonitorUserHdd.start(1);
    tmrMonitorUsbStickInserted.start(1);
    tmrMonitorHcuTemperatureHw.start(1);
    tmrMonitorCpuMemUsage.start(1);
    tmrMonitorCruLink.start(1);
    tmrMonitorIwconfig.start(1);

    // Disable any new USB connections
    QString buildType = getenv("BUILD_TYPE");
    if ( (buildType == BUILD_TYPE_VNV) || (buildType == BUILD_TYPE_REL) )
    {
        LOG_INFO("Unbinding USB ports\n");
        (void)system(PATH_UNBIND_USB);
        LOG_INFO("Disabling new USB connections\n");
        (void)system(QString().asprintf("%s unauthorize", PATH_CONTROL_USB).CSTR());
    }
}

void SystemMonitorOS::slotMonitorTimeShifted()
{
    qint64 curTimeEpochMs = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
    qint64 systemUpElapsedMs = curTimeEpochMs - env->systemUpEpochMs;
    qint64 timeShiftedMs = env->systemUpElapsedTimer.elapsed() - systemUpElapsedMs;

    /*LOG_DEBUG_H("slotMonitorTimeShifted(): systemUpElapsedTimer.elapsed=%llds, systemUpElapsedMs=%llds, timeShiftedMs=%lldms\n",
                env->systemUpElapsedTimer.elapsed() / 1000,
                systemUpElapsedMs / 1000,
                timeShiftedMs);*/

    if (labs(timeShiftedMs) > 1000)
    {
        LOG_WARNING("slotMonitorTimeShifted(): Time Shifted by %lldms\n", timeShiftedMs);
        env->systemUpEpochMs = curTimeEpochMs - env->systemUpElapsedTimer.elapsed();
        emit env->signalTimeShifted(timeShiftedMs);
    }

    tmrMonitorTimeShifted.start(SYSTEM_MONITOR_INTERVAL_MS_TIME_SHIFT);
}

void SystemMonitorOS::slotProcMonitorMainHddFinished(QString err)
{
    QString procOut;
    double curFreeSpaceMB;

    if (env->state == EnvGlobal::STATE_EXITING)
    {
        return;
    }

    if (err != "")
    {
        LOG_ERROR("slotProcMonitorMainHddFinished(): Exit with err=%s\n", err.CSTR());
        goto bail;
    }

    procOut = procMonitorMainHdd.readAll().trimmed();
    procMonitorMainHdd.close();

    LOG_DEBUG("slotProcMonitorMainHddFinished(): Proc(%s, arg=%s) returned out=%s\n", procMonitorMainHdd.program().CSTR(), procMonitorMainHdd.arguments().join(" ").CSTR(), procOut.CSTR());

    if (procOut == "")
    {
        LOG_ERROR("slotProcMonitorMainHddFinished(): Bad Process Output\n");
        goto bail;
    }

    curFreeSpaceMB = procOut.toInt() / 1024;
    if (curFreeSpaceMB != env->ds.systemData->getDiskMainFreeSpaceMB())
    {
        LOG_INFO("slotProcMonitorMainHddFinished(): Setting Free Space to %.1fMB\n", curFreeSpaceMB);
        env->ds.systemData->setDiskMainFreeSpaceMB(curFreeSpaceMB);
    }

bail:
    tmrMonitorMainHdd.start(SYSTEM_MONITOR_INTERVAL_MS_MAIN_HDD);
}

void SystemMonitorOS::slotProcMonitorUserHddFinished(QString err)
{
    QString procOut;
    double curFreeSpaceMB;

    if (env->state == EnvGlobal::STATE_EXITING)
    {
        return;
    }

    if (err != "")
    {
        LOG_ERROR("slotProcMonitorUserHddFinished(): Exit with err=%s\n", err.CSTR());
        goto bail;
    }

    procOut = procMonitorUserHdd.readAll().trimmed();
    procMonitorUserHdd.close();

    LOG_DEBUG("slotProcMonitorUserHddFinished(): Proc(%s, arg=%s) returned out=%s\n", procMonitorUserHdd.program().CSTR(), procMonitorUserHdd.arguments().join(" ").CSTR(), procOut.CSTR());

    if (procOut == "")
    {
        LOG_ERROR("slotProcMonitorUserHddFinished(): Bad Process Output\n");
        goto bail;
    }

    curFreeSpaceMB = procOut.toInt() / 1024;
    if (curFreeSpaceMB != env->ds.systemData->getDiskUserFreeSpaceMB())
    {
        LOG_INFO("slotProcMonitorUserHddFinished(): Setting Free Space to %.1fMB\n", curFreeSpaceMB);
        env->ds.systemData->setDiskUserFreeSpaceMB(curFreeSpaceMB);
    }

bail:
    tmrMonitorUserHdd.start(SYSTEM_MONITOR_INTERVAL_MS_USER_HDD);
}

void SystemMonitorOS::slotProcMonitorUsbStickInsertedFinished(QString err)
{
    QString procOut;
    static QString lastUsbDirList = "";
    QString curUsbDirList;

    if (env->state == EnvGlobal::STATE_EXITING)
    {
        return;
    }

    if (err != "")
    {
        LOG_ERROR("slotProcMonitorUsbStickInsertedFinished(): Exit with err=%s\n", err.CSTR());
        goto bail;
    }

    procOut = procMonitorUsbStickInserted.readAll().trimmed();
    procMonitorUsbStickInserted.close();

    LOG_DEBUG("slotProcMonitorUsbStickInsertedFinished(): Proc(%s, arg=%s) returned out=%s\n", procMonitorUsbStickInserted.program().CSTR(), procMonitorUsbStickInserted.arguments().join(" ").CSTR(), procOut.CSTR());


    if (procOut == "")
    {
        // No USB stick found
    }

    curUsbDirList = procOut;

    if (curUsbDirList != lastUsbDirList)
    {
        LOG_INFO("slotProcMonitorUsbStickInsertedFinished(): Setting USB dir list to %s\n", curUsbDirList.CSTR());
        env->ds.systemData->setUsbStickInserted(curUsbDirList.length() > 0);
        lastUsbDirList = curUsbDirList;
    }

bail:
    tmrMonitorUsbStickInserted.start(SYSTEM_MONITOR_INTERVAL_MS_USB_STICK_INSERTED);
}

void SystemMonitorOS::slotProcMonitorHcuTemperatureHwFinished(QString err)
{
    QString procOut;
    QString fanSpeedStr;
    QStringList stdOutList;
    DS_SystemDef::HcuTemperatureParams hcuTemperatureParams;
    DS_SystemDef::HcuTemperatureParams envHcuTemperatureParams;

    if (env->state == EnvGlobal::STATE_EXITING)
    {
        return;
    }

    if (err != "")
    {
        LOG_ERROR("slotProcMonitorHcuTemperatureHwFinished(): Exit with err=%s\n", err.CSTR());
        goto bail;
    }

    procOut = procMonitorHcuTemperatureHw.readAll().trimmed();
    procMonitorHcuTemperatureHw.close();

    LOG_DEBUG("slotProcMonitorHcuTemperatureHwFinished(): Proc(%s, arg=%s) returned out=\n%s\n", procMonitorHcuTemperatureHw.program().CSTR(), procMonitorHcuTemperatureHw.arguments().join(" ").CSTR(), procOut.CSTR());

    if (procOut == "")
    {
        LOG_ERROR("slotProcMonitorHcuTemperatureHwFinished(): Bad Process Output\n");
        goto bail;
    }

    stdOutList = procOut.split("\n");
    for (int i = 0; i < stdOutList.length(); i++)
    {
        QString curLine = stdOutList[i];
        if (curLine.contains("°C"))
        {
            // Get temperature value
            QStringList strList = curLine.split(" ");
            for (int j = 0; j < strList.length(); j++)
            {
                if (strList[j].contains("°C"))
                {
                    QString temperatureCelciusStr = strList[j];
                    temperatureCelciusStr.replace("°C", "");
                    temperatureCelciusStr.replace("+", "");
                    hcuTemperatureParams.cpuTemperatureCelcius = temperatureCelciusStr.toDouble();
                    break;
                }
            }
            break;
        }
    }

    fanSpeedStr = stdOutList[stdOutList.length() - 1];
    if (fanSpeedStr.contains("rpm"))
    {
        fanSpeedStr.replace(" rpm","");
        hcuTemperatureParams.fanSpeed = fanSpeedStr.toInt();

    }

    envHcuTemperatureParams = env->ds.systemData->getHcuTemperatureParams();

    if (hcuTemperatureParams.cpuTemperatureCelcius != envHcuTemperatureParams.cpuTemperatureCelcius)
    {
        LOG_INFO("slotProcMonitorHcuTemperatureHwFinished(): Monitored CPU Temperature = %.1f°C\n", hcuTemperatureParams.cpuTemperatureCelcius);
        env->ds.systemData->setHcuTemperatureParams(hcuTemperatureParams);
    }
    else if (hcuTemperatureParams.fanSpeed != envHcuTemperatureParams.fanSpeed)
    {
        env->ds.systemData->setHcuTemperatureParams(hcuTemperatureParams);
    }

bail:
    tmrMonitorHcuTemperatureHw.start(SYSTEM_MONITOR_INTERVAL_MS_HCU_TEMPERATURE_HW);
}

void SystemMonitorOS::slotProcMonitorCpuMemUsageFinished(QString err)
{
    QString procOut;

    if (env->state == EnvGlobal::STATE_EXITING)
    {
        return;
    }

    if ( (err != "") &&
         (err != "0") )
    {
        LOG_ERROR("slotProcMonitorCpuMemUsageFinished(): Exit with err=%s\n", err.CSTR());
        goto bail;
    }

    procOut = procMonitorCpuMemUsage.readAll().trimmed();
    procMonitorCpuMemUsage.close();

    LOG_DEBUG("slotProcMonitorCpuMemUsageFinished(): Proc(%s, arg=%s) returned out=\n%s\n", procMonitorCpuMemUsage.program().CSTR(), procMonitorCpuMemUsage.arguments().join(" ").CSTR(), procOut.CSTR());

bail:
    tmrMonitorCpuMemUsage.start(SYSTEM_MONITOR_INTERVAL_MS_CPU_MEM_USAGE);
}

void SystemMonitorOS::slotProcMonitorCruLinkFinished(QString err)
{
    QString procOut;

    if (env->state == EnvGlobal::STATE_EXITING)
    {
        return;
    }

    if (err != "")
    {
        LOG_ERROR("slotProcMonitorCruLinkFinished(): Exit with err=%s\n", err.CSTR());
        goto bail;
    }

    procOut = procMonitorCruLink.readAll().trimmed();
    procMonitorCruLink.close();

    LOG_DEBUG("slotProcMonitorCruLinkFinished(): Proc(%s, arg=%s) returned out=%s\n", procMonitorCruLink.program().CSTR(), procMonitorCruLink.arguments().join(" ").CSTR(), procOut.CSTR());

    if (procOut == "")
    {
        LOG_ERROR("slotProcMonitorCruLinkFinished(): Bad Process Output\n");
        goto bail;
    }

    if (procOut.contains("ping failed"))
    {
        LOG_INFO("slotProcMonitorCruLinkFinished(): CRU link has been off for a while, setting network\n");
        Config::Item cfgConnectionType = env->ds.cfgLocal->getItem_Settings_SruLink_ConnectionType();
        emit env->ds.cfgLocal->signalConfigChanged_Settings_SruLink_ConnectionType(cfgConnectionType, cfgConnectionType);
    }

bail:
    tmrMonitorCruLink.start(SYSTEM_MONITOR_INTERVAL_MS_SETUP_CRU_LINK);
}

void SystemMonitorOS::slotProcMonitorIwconfigFinished(QString err)
{
    QString procOut;
    QStringList stdOutList;    

    if (env->state == EnvGlobal::STATE_EXITING)
    {
        return;
    }

    if ( (err != "") &&
         (err != "0") )
    {
        LOG_ERROR("slotProcMonitorIwconfigFinished(): Exit with err=%s\n", err.CSTR());
        goto bail;
    }

    procOut = procMonitorIwconfig.readAll().trimmed();
    procMonitorIwconfig.close();

    LOG_DEBUG("slotProcMonitorIwconfigFinished(): Proc(%s, arg=%s) returned out=\n%s\n", procMonitorIwconfig.program().CSTR(), procMonitorIwconfig.arguments().join(" ").CSTR(), procOut.CSTR());

    if (procOut == "")
    {
        LOG_ERROR("slotProcMonitorIwconfigFinished(): Bad Process Output\n");
        goto bail;
    }

    stdOutList = procOut.split("\n");
    for (int i = 0; i < stdOutList.length(); i++)
    {
        QString curLine = stdOutList[i];
        if (curLine.contains("Access Point") && curLine.contains("Not-Associated"))
        {
            // No access point is associated with the SRU, ignore iwconfig data
            iwconfigParams.linkQualities.append(0);
            iwconfigParams.signalLevels.append(0);
            iwconfigParams.noiseLevels.append(0);
            break;
        }
        else if (curLine.contains("Link Quality"))
        {
            int value;
            // Get specific values
            QStringList strList = curLine.split("  ");
            for (int j = 0; j < strList.length(); j++)
            {
                const QString &line = strList[j];

                if (line.contains("Link Quality="))
                {
                    //" Link Quality=94/97" - extract number between '=' and '/'
                    if (Util::extractIntFromString(&value, line, '=', '/'))
                    {
                        iwconfigParams.linkQualities.append(value);
                    }
                    else
                    {
                        LOG_ERROR("slotProcMonitorIwconfigFinished() parse error '%s' ", line.CSTR());
                    }
                }
                else if (line.contains("Signal level="))
                {
                    //" Signal level=10 dBm" - extract number between '=' and ' '
                    if (Util::extractIntFromString(&value, line, '=', ' '))
                    {
                        iwconfigParams.signalLevels.append(value);
                    }
                    else
                    {
                        LOG_ERROR("slotProcMonitorIwconfigFinished() parse error '%s' ", line.CSTR());
                    }
                }
                else if (line.contains("Noise level="))
                {
                    //" Noise level==10 dBm" - extract number between '=' and ' '
                    if (Util::extractIntFromString(&value, line, '=', ' '))
                    {
                        iwconfigParams.noiseLevels.append(value);
                    }
                    else
                    {
                        LOG_ERROR("slotProcMonitorIwconfigFinished() parse error '%s' ", line.CSTR());
                    }

                    break; // This is the last item that we need...
                }
            }
            break;
        }
    }

    if (iwconfigParams.linkQualities.size() >= SYSTEM_MONITOR_SAMPLE_COUNT_IWCONFIG)
    {
        // We have enough data points (~60s worth), so let's compute the stats
        int p = (int) (SYSTEM_MONITOR_INTERVAL_MS_IWCONFIG / 1000);
        int w = (int) (p * iwconfigParams.linkQualities.size());
        iwconfigParams.computeStats(p, w);

        // Update the system data if it has changed (note, this will cause a new digest to be generated!)
        DS_SystemDef::IwconfigParams iwconfigParamsLast = env->ds.systemData->getIwconfigParams();
        if (iwconfigParams != iwconfigParamsLast)
        {
            if (abs(iwconfigParams.signalLevelAvg - iwconfigParamsLast.signalLevelAvg) > SYSTEM_MONITOR_ALERT_THRESHOLD_IWCONFIG)
            {
                LOG_INFO("Wifi Signal Strength (%d) has changed %ddBm from last avg check (%d)\n", iwconfigParams.signalLevelAvg, SYSTEM_MONITOR_ALERT_THRESHOLD_IWCONFIG, iwconfigParamsLast.signalLevelAvg);
                QVariantMap alertData = ImrParser::ToImr_Iwconfig(iwconfigParams);
                env->ds.alertAction->activate("WifiSignalLevelChanged", Util::qVarientToJsonData(alertData));
            }

            LOG_INFO("slotProcMonitorIwconfigFinished(): Monitored avg iwconfig params = %d/94 %d dBm %d dBm\n", iwconfigParams.linkQualityAvg, iwconfigParams.signalLevelAvg, iwconfigParams.noiseLevelAvg);
            env->ds.systemData->setIwconfigParams(iwconfigParams);
        }

        // Append to our history list, but cap it...
        if (iwconfigParamsHistory.length() >= SYSTEM_MONITOR_HISTORY_COUNT_IWCONFIG)
        {
            iwconfigParamsHistory.removeFirst();
        }        
        iwconfigParamsHistory.append(iwconfigParams);

        // Determine if the link state has changed since we last checked, and is now Inactive
        bool cruLinkStateIsInactive = env->ds.cruData->getCruLinkStatus().state == DS_CruDef::CRU_LINK_STATE_INACTIVE;
        if (cruLinkStateIsInactive)
        {
            if (!lastCruLinkStateIsInactive)
            {
                // If we have enough samples in our history, raise an alert with this data since comms are down.
                // Checking for this count acts as a debounce; we must have been active for at
                // least 6 mins (iwconfigParamsHistory length of 6) before raising this alert.
                if (iwconfigParamsHistory.length() >= SYSTEM_MONITOR_HISTORY_COUNT_IWCONFIG)
                {
                    QString alertData = QString().asprintf("%s;",ImrParser::ToImr_StatePath(env->ds.systemData->getStatePath()).CSTR());
                    foreach (DS_SystemDef::IwconfigParams paramas, iwconfigParamsHistory)
                    {
                        alertData += QString().asprintf("%s;", Util::qVarientToJsonData(ImrParser::ToImr_DoubleList(paramas.signalLevels)).CSTR());
                    }
                    env->ds.alertAction->activate("SRUCommLossDiagnostics", alertData);
                }
                LOG_INFO("slotProcMonitorIwconfigFinished(): CruLinkState is now Inactive, and iwconfigParamsHistory count is %d\n", (int)iwconfigParamsHistory.length());
            }

            // While comms are inactive, clear any history that we may have
            iwconfigParamsHistory.clear();
        }
        lastCruLinkStateIsInactive = cruLinkStateIsInactive;

        // Clear out iwconfig data to gather the next window sample set
        iwconfigParams.init();
    }


bail:
    tmrMonitorIwconfig.start(SYSTEM_MONITOR_INTERVAL_MS_IWCONFIG);
}


