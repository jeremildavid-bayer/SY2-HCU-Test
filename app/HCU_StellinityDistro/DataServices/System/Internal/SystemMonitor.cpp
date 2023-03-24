#include "SystemMonitor.h"
#include "Apps/AppManager.h"
#include "Common/Util.h"
#include "Common/ImrParser.h"
#include "Common/Translator.h"
#include "DataServices/System/DS_SystemData.h"
#include "DataServices/Mcu/DS_McuData.h"
#include "DataServices/Cru/DS_CruData.h"
#include "DataServices/CfgGlobal/DS_CfgGlobal.h"
#include "DataServices/CfgLocal/DS_CfgLocal.h"
#include "DataServices/Capabilities/DS_Capabilities.h"
#include "DataServices/System/DS_SystemAction.h"
#include "DataServices/Exam/DS_ExamData.h"
#include "DataServices/Alert/DS_AlertAction.h"
#include "DataServices/Alert/DS_AlertData.h"
#include "DataServices/HardwareInfo/DS_HardwareInfo.h"
#include "DataServices/Device/DS_DeviceData.h"
#include "DataServices/Workflow/DS_WorkflowData.h"

SystemMonitor::SystemMonitor(QObject *parent, EnvGlobal *env_) :
    QObject(parent),
    env(env_)
{
    envLocal = new EnvLocal("DS_System-Monitor", "SYSTEM_MONITOR", LOG_MID_SIZE_BYTES);
    systemMonitorOs = new SystemMonitorOS(this, env);

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });
}

SystemMonitor::~SystemMonitor()
{
    delete systemMonitorOs;
    delete envLocal;
}

void SystemMonitor::slotAppInitialised()
{
    connect(env->ds.systemData, &DS_SystemData::signalDataChanged_StatePath, this, [=](DS_SystemDef::StatePath curStatePath, DS_SystemDef::StatePath prevStatePath) {
        if ( ( (prevStatePath == DS_SystemDef::STATE_PATH_IDLE) && (curStatePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING) ) ||
             ( (prevStatePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING) && (curStatePath == DS_SystemDef::STATE_PATH_IDLE) ) )
        {
            LOG_INFO("\n\n-----------------\nStatePath=%s\n\n", ImrParser::ToImr_StatePath(curStatePath).CSTR());
        }
    });

    connect(env->ds.capabilities, &DS_Capabilities::signalConfigChanged_DateTime_SystemDateTime, this, [=](const Config::Item &cfg){
        // dateTimeStr format is "YYYY/MM/DD hh:mm" and linux sudo date format is "MMDDhhmmYYYY"
        QString dateTimeStr = cfg.value.toString();
        if (dateTimeStr == "")
        {
            return;
        }

        QDateTime dateTime = QDateTime::fromString(dateTimeStr, "yyyy/MM/dd hh:mm");

        QString guid = Util::newGuid();
        env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
            if (curStatus.state == DS_ACTION_STATE_STARTED)
            {
                env->actionMgr->onActionCompleted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus status) {
                    if (status.state == DS_ACTION_STATE_COMPLETED)
                    {
                        LOG_INFO("DATE_TIME_SETUP: DateTime changed(%s)\n", dateTimeStr.CSTR());
                        QTimer::singleShot(1, [=] {
                            // Set config value to null. So the callback is always triggered even the value is not changed
                            Config::Item newCfgItem = cfg;
                            newCfgItem.value = "";
                            env->ds.capabilities->setDateTime_SystemDateTime(newCfgItem);
                        });
                    }
                    else
                    {
                        LOG_ERROR("DATE_TIME_SETUP: Action Failed Complete: %s\n", ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());
                    }
                });
            }
            else
            {
                LOG_ERROR("DATE_TIME_SETUP: Action Failed Start: %s\n", ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());
            }
        });
        env->ds.systemAction->actSetSystemDateTime(dateTime, guid);
    });

    connect(env->ds.cruData, &DS_CruData::signalDataChanged_CurrentUtcNowEpochSec, this, [=](const qint64 &cruTimeEpochSec) {
        DS_CruDef::CruLinkStatus linkStatus = env->ds.cruData->getCruLinkStatus();

        if ( (linkStatus.state == DS_CruDef::CRU_LINK_STATE_RECOVERING) ||
             (linkStatus.state == DS_CruDef::CRU_LINK_STATE_ACTIVE) )
        {
            //LOG_DEBUG_H("CurrentUtcNowEpochSecChanged: Date/Time Needs Sync. CRU=%s\n", QDateTime::fromSecsSinceEpoch(cruTimeEpochSec).toUTC().toString("yyyy/MM/dd hh:mm:ss").CSTR());

            // Check if time synch is required
            if (cruTimeEpochSec == 0)
            {
                QString err = QString().asprintf("Bad CRU Utc Time (%s)\n", QDateTime::fromSecsSinceEpoch(cruTimeEpochSec).toUTC().toString("yyyy/MM/dd hh:mm:ss").CSTR());
                LOG_ERROR("%s", err.CSTR());
                env->ds.alertAction->activate("HCUInternalSoftwareError", err);
                return;
            }

            qint64 hcuTimeEpochSec = QDateTime::currentDateTimeUtc().toSecsSinceEpoch();
            qint64 timeDiffSec = hcuTimeEpochSec - cruTimeEpochSec;

            if (abs(timeDiffSec) > CRU_TIME_SYNC_TOLERANCE_SEC)
            {
                LOG_WARNING("CurrentUtcNowEpochSecChanged: Date/Time Needs Sync. HCU=%s, CRU=%s, Diff=%lld seconds. Synchronise time now..\n",
                            QDateTime::fromSecsSinceEpoch(hcuTimeEpochSec).toUTC().toString("yyyy/MM/dd hh:mm:ss").CSTR(),
                            QDateTime::fromSecsSinceEpoch(cruTimeEpochSec).toUTC().toString("yyyy/MM/dd hh:mm:ss").CSTR(),
                            timeDiffSec);
                env->ds.systemAction->actSyncSystemDateTime();
            }
        }
    });

    connect(env->ds.cfgLocal, &DS_CfgLocal::signalConfigChanged_Settings_Display_ScreenBrightness, this, [=](const Config::Item &cfg){
        int level = cfg.value.toInt();

        QString guid = Util::newGuid();
        env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
            if (curStatus.state == DS_ACTION_STATE_STARTED)
            {
                env->actionMgr->onActionCompleted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
                    if (curStatus.state == DS_ACTION_STATE_COMPLETED)
                    {
                        LOG_INFO("SCREEN_BRIGHTNESS: Screen brightness changed (%d)\n", level);
                    }
                });
            }
        });

        env->ds.systemAction->actSetScreenBrightness(level, guid);
    });

    connect(env->ds.cfgLocal, &DS_CfgLocal::signalConfigChanged_Settings_Display_ScreenOffTimeoutMinutes, this, [=](const Config::Item &cfg) {
        int timeoutMinutes = cfg.value.toInt();

        QString guid = Util::newGuid();
        env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
            if (curStatus.state == DS_ACTION_STATE_STARTED)
            {
                env->actionMgr->onActionCompleted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
                    if (curStatus.state == DS_ACTION_STATE_COMPLETED)
                    {
                        LOG_INFO("SCREEN_OFF_TIMEOUT: Screen off timeout changed (%d minutes)\n", timeoutMinutes);
                    }
                    else
                    {
                        LOG_ERROR("SCREEN_OFF_TIMEOUT: Action Failed Complete: %s\n", ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());
                    }
                });
            }
            else
            {
                LOG_ERROR("SCREEN_OFF_TIMEOUT: Action Failed Start: %s\n", ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());
            }
        });

        env->ds.systemAction->actSetScreenSleepTime(timeoutMinutes, guid);
    });

    connect(env->ds.cfgLocal, &DS_CfgLocal::signalConfigChanged_Settings_SruLink_ConnectionType, this, [=](const Config::Item &newCfg, const Config::Item &prevCfg) {
        LOG_INFO("signalConfigChanged_Settings_SruLink_ConnectionType(): Link Type changed %s->%s\n", prevCfg.value.toString().CSTR(), newCfg.value.toString().CSTR());

        // Disconnect current interface first
        if (newCfg.value == prevCfg.value)
        {
            // no need to disconnect current interface
        }
        else if (prevCfg.value.toString() == _L("WirelessEthernet"))
        {
            env->ds.alertAction->activate("SRUNetworkingChanged", "wired");
        }
        else if (prevCfg.value.toString() == _L("WiredEthernet"))
        {
            env->ds.alertAction->activate("SRUNetworkingChanged", "wireless");
        }
        updateNetworkInterface();
    });


    connect(env->ds.cfgGlobal, &DS_CfgGlobal::signalConfigChanged_Settings_SruLink_WirelessCountryCode, this, [=] {
        QString connectionType = env->ds.cfgLocal->get_Settings_SruLink_ConnectionType();
        if (connectionType == _L("WirelessEthernet"))
        {
            updateNetworkInterface();
        }
    });

    connect(env->ds.cruData, &DS_CruData::signalDataChanged_WifiSsid, this, [=] {
        QString connectionType = env->ds.cfgLocal->get_Settings_SruLink_ConnectionType();
        if (connectionType == _L("WirelessEthernet"))
        {
            updateNetworkInterface();
        }
    });

    connect(env->ds.cruData, &DS_CruData::signalDataChanged_WifiPassword, this, [=] {
        QString connectionType = env->ds.cfgLocal->get_Settings_SruLink_ConnectionType();
        if (connectionType == _L("WirelessEthernet"))
        {
            updateNetworkInterface();
        }
    });


    connect(env->ds.cfgGlobal, &DS_CfgGlobal::signalConfigChanged_Service_TradeshowModeEnabled, this, [=] {
        bool tradeshowModeEnabled = env->ds.cfgGlobal->get_Service_TradeshowModeEnabled();
        if (tradeshowModeEnabled)
        {
            env->ds.alertAction->activate("TradeshowModeActive");
        }
        else
        {
            env->ds.alertAction->deactivate("TradeshowModeActive");
        }
    });


    connect(env->ds.capabilities, &DS_Capabilities::signalConfigChanged_Network_EthernetInterface, this, [=] {
        QString connectionType = env->ds.cfgLocal->get_Settings_SruLink_ConnectionType();
        if (connectionType == _L("WiredEthernet"))
        {
            updateNetworkInterface();
        }
    });

    connect(env->ds.capabilities, &DS_Capabilities::signalConfigChanged_Network_WifiInterface, this, [=] {
        QString connectionType = env->ds.cfgLocal->get_Settings_SruLink_ConnectionType();
        if (connectionType == _L("WirelessEthernet"))
        {
            updateNetworkInterface();
        }
    });

    connect(env->ds.hardwareInfo, &DS_HardwareInfo::signalConfigChanged_General_BaseBoardType, this, [=] {
        QString connectionType = env->ds.cfgLocal->get_Settings_SruLink_ConnectionType();
        QString baseBoardType = env->ds.hardwareInfo->getGeneral_BaseBoardType().value.toString();
        if ( (connectionType == "WirelessEthernet") &&
             ((baseBoardType == "OCS") || (baseBoardType == "NoBattery")) )
        {
            LOG_WARNING("LinkType=%s AND BaseBoardType=%s. Forcing the network type to WiredEthernet..\n", connectionType.CSTR(), baseBoardType.CSTR());
            env->ds.cfgLocal->set_Settings_SruLink_ConnectionType("WiredEthernet");
        }
    });

    connect(env->ds.systemData, &DS_SystemData::signalDataChanged_StatePath, this, [=](DS_SystemDef::StatePath curStatePath, DS_SystemDef::StatePath prevStatePath) {
        if (prevStatePath == curStatePath)
        {
            return;
        }

        LOG_INFO("\n");
        LOG_INFO("StatePath = %s -> %s\n\n", ImrParser::ToImr_StatePath(prevStatePath).CSTR(), ImrParser::ToImr_StatePath(curStatePath).CSTR());

        // Update Screen Sleep Time
        if (curStatePath == DS_SystemDef::STATE_PATH_READY_ARMED)
        {
            // ARMED: wake up and don't set the sleep time
            env->ds.systemAction->actScreenWakeup();
            env->ds.systemAction->actSetScreenSleepTime(0);
        }
        else if ( (prevStatePath == DS_SystemDef::STATE_PATH_BUSY_FINISHING) &&
                  (curStatePath == DS_SystemDef::STATE_PATH_IDLE) )
        {
            // Injection completed: set the sleep time
            env->ds.systemAction->actSetScreenSleepTimeToDefault();
        }
        else if ( (prevStatePath == DS_SystemDef::STATE_PATH_READY_ARMED) &&
                  (curStatePath == DS_SystemDef::STATE_PATH_IDLE) )
        {
            // Disarmed: set the sleep time
            env->ds.systemAction->actSetScreenSleepTimeToDefault();
        }

        // Handle Service/Error State
        if ( (curStatePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING) ||
             (curStatePath == DS_SystemDef::STATE_PATH_ERROR) )
        {
            LOG_WARNING("StatePath changed, Setting all Fluid Sources state to NOT busy..\n");
            DS_DeviceDef::FluidSource fluidSourceMuds = env->ds.deviceData->getFluidSourceMuds();
            fluidSourceMuds.isBusy = false;

            env->ds.deviceData->setFluidSourceMuds(fluidSourceMuds);

            DS_DeviceDef::FluidSource fluidSourceSuds = env->ds.deviceData->getFluidSourceSuds();
            fluidSourceSuds.isBusy = false;
            env->ds.deviceData->setFluidSourceSuds(fluidSourceSuds);

            DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
            for (int syringeIdx = 0; syringeIdx < fluidSourceSyringes.length(); syringeIdx++)
            {
                fluidSourceSyringes[syringeIdx].isBusy = false;
            }
            env->ds.deviceData->setFluidSourceSyringes(fluidSourceSyringes);

            DS_DeviceDef::FluidSources fluidSourceBottles = env->ds.deviceData->getFluidSourceBottles();
            for (int syringeIdx = 0; syringeIdx < fluidSourceSyringes.length(); syringeIdx++)
            {
                fluidSourceBottles[syringeIdx].isBusy = false;
            }
            env->ds.deviceData->setFluidSourceBottles(fluidSourceBottles);
        }
    });

    connect(&tmrRetryLinkSetup, &QTimer::timeout, this, [=] {
        if (env->state == EnvGlobal::STATE_EXITING)
        {
            return;
        }

        tmrRetryLinkSetup.stop();
        updateNetworkInterface();
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_PowerStatus, this, [=] {
        handleBatteryCritical();
    });

    connect(env->ds.workflowData, &DS_WorkflowData::signalDataChanged_ManualQualifiedDischargeStatus, this, [=] {
        handleBatteryCritical();
    });

    connect(env->ds.systemData, &DS_SystemData::signalDataChanged_HcuTemperatureParams, this, [=](DS_SystemDef::HcuTemperatureParams hcuTemperatureParams) {
        if (hcuTemperatureParams.cpuTemperatureCelcius >= HCU_TEMPERATURE_WARNING_ACTIVATE_CELCIUS)
        {
            LOG_WARNING("HCU Temperature is HIGH (%.1f°C)\n", hcuTemperatureParams.cpuTemperatureCelcius);
        }
        else
        {
            LOG_INFO("HCU Temperature is NORMAL (%.1f°C)\n", hcuTemperatureParams.cpuTemperatureCelcius);
        }

        if (hcuTemperatureParams.cpuTemperatureCelcius >= HCU_TEMPERATURE_WARNING_ACTIVATE_CELCIUS)
        {
            if (!env->ds.alertAction->isActivated("HCUTemperatureHigh"))
            {
                env->ds.alertAction->activate("HCUTemperatureHigh");
            }
        }
        else if (hcuTemperatureParams.cpuTemperatureCelcius < HCU_TEMPERATURE_WARNING_DEACTIVATE_CELCIUS)
        {

            if (env->ds.alertAction->isActivated("HCUTemperatureHigh"))
            {
                env->ds.alertAction->deactivate("HCUTemperatureHigh");
            }
        }
    });

    connect(env->ds.systemData, &DS_SystemData::signalDataChanged_DiskMainFreeSpaceMB, this, [=](double freeSpaceMB) {
        if (freeSpaceMB < LOW_DISK_SPACE_THRESHOLD_MB)
        {
            env->ds.alertAction->activate("HCULowDiskSpace", "MAIN");
        }
        else
        {
            env->ds.alertAction->deactivate("HCULowDiskSpace", "MAIN");
        }
    });

    connect(env->ds.systemData, &DS_SystemData::signalDataChanged_DiskUserFreeSpaceMB, this, [=](double freeSpaceMB) {
        if (freeSpaceMB < LOW_DISK_SPACE_THRESHOLD_MB)
        {
            env->ds.alertAction->activate("HCULowDiskSpace", "USER");
        }
        else
        {
            env->ds.alertAction->deactivate("HCULowDiskSpace", "USER");
        }
    });

    connect(env->ds.cfgGlobal, &DS_CfgGlobal::signalConfigChanged_Service_TradeshowModeEnabled, this, [=](const Config::Item &newCfg, const Config::Item &prevCfg) {
        if (newCfg.value.toBool() != prevCfg.value.toBool())
        {
            QString buildType = env->ds.systemData->getBuildType();

            bool tradeShowModeEnabled = env->ds.cfgGlobal->get_Service_TradeshowModeEnabled();
            env->humanUseAllowed = (buildType == BUILD_TYPE_REL) && (!tradeShowModeEnabled);
            if (!env->humanUseAllowed)
            {
                env->ds.alertAction->activate("NotForHumanUseSRU");
            }
            else
            {
                env->ds.alertAction->deactivate("NotForHumanUseSRU");
            }
        }
    });

    connect(env->ds.systemData, &DS_SystemData::signalDataChanged_StatePath, this, [=](DS_SystemDef::StatePath curStatePath, DS_SystemDef::StatePath prevStatePath) {
        // Only register screen wake up events after GUI is loaded. Once video finishes, the state changes to STATE_PATH_STARTUP_UNKNOWN and then shows the gui app
        // There will be other initializations that will eventually change the system state from STATE_PATH_STARTUP_UNKNOWN, we then register screen wake up events
        // REASON: during boot up and initializations, many events occur as initialization and screen wake up will be called.
        //         screen wake up sends ESC key command which kills the start up video too early
        if (prevStatePath == DS_SystemDef::STATE_PATH_STARTUP_UNKNOWN)
        {
            registerScreenWakeUpEvents();
        }
    });
}

void SystemMonitor::handleBatteryCritical()
{
    DS_WorkflowDef::ManualQualifiedDischargeState manualQualifiedDischargeState = env->ds.workflowData->getManualQualifiedDischargeStatus().state;
    DS_McuDef::PowerStatus powerStatus = env->ds.mcuData->getPowerStatus();

    if ( (manualQualifiedDischargeState == DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_READY) &&
         (!powerStatus.isAcPowered) &&
         (powerStatus.batteryLevel == DS_McuDef::BATTERY_LEVEL_CRITICAL) )
    {
        LOG_WARNING("handleBatteryCritical(): Battery Level Critical: BatteryStatus=%s: System Shutting Down in %dms...\n",
                    Util::qVarientToJsonData(ImrParser::ToImr_PowerStatus(powerStatus)).CSTR(),
                    POWER_OFF_BATTERY_CRITICAL_WAIT_TIME_MS);
        QTimer::singleShot(POWER_OFF_BATTERY_CRITICAL_WAIT_TIME_MS, this, [=](){
            env->ds.systemAction->actShutdown(DS_McuDef::POWER_CONTROL_TYPE_OFF);
        });
    }
}

void SystemMonitor::updateNetworkInterface()
{
    if (env->state != EnvGlobal::STATE_RUNNING)
    {
        return;
    }

    // Start connect link
    QString guid = Util::newGuid();

    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            LOG_DEBUG("updateNetworkInterface(): Link Setup process started\n");

            env->actionMgr->onActionCompleted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
                if (curStatus.state == DS_ACTION_STATE_COMPLETED)
                {
                    LOG_DEBUG("LINK_SETUP: Link Setup Completed\n");
                }
                else
                {
                    LOG_ERROR("LINK_SETUP: Action Failed Complete: %s\n", ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());
                }
            });
        }
        else if (curStatus.state == DS_ACTION_STATE_BUSY)
        {
            LOG_INFO("updateNetworkInterface(): Link setup process is busy. Retrying soon..\n");
            tmrRetryLinkSetup.stop();
            tmrRetryLinkSetup.start(3000);
        }
        else
        {
            LOG_ERROR("updateNetworkInterface(): Action Failed Start: %s\n", ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());
        }
    });

    LOG_DEBUG("updateNetworkInterface(): Link Params Changed. Setting up CRU Link..\n");
    env->ds.systemAction->actNetworkInterfaceUp(guid);
}


void SystemMonitor::registerScreenWakeUpEvents()
{
    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_StopBtnPressed, this, [=](bool pressed) {
        if (pressed)
        {
            env->ds.systemAction->actScreenWakeup();
        }
    });

    connect(env->ds.alertData, &DS_AlertData::signalDataChanged_ActiveSystemAlerts, this, [=](const QVariantList &activeSystemAlerts) {
        env->ds.systemAction->actScreenWakeup();
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_PrimeBtnPressed, this, [=](bool pressed) {
        if (pressed)
        {
            env->ds.systemAction->actScreenWakeup();
        }
    });

    // DOOR
    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_DoorBtnPressed, this, [=](bool pressed) {
        if (pressed)
        {
            env->ds.systemAction->actScreenWakeup();
        }
    });
    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_DoorState, this, [=](DS_McuDef::DoorState state) {
        env->ds.systemAction->actScreenWakeup();
    });
    // DOOR end

    // Shutdown
    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_IsShuttingDown, this, [=](bool isShuttingDown) {
        if (isShuttingDown) {
            env->ds.systemAction->actScreenWakeup();
        }
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_SudsInserted, this, [=](bool inserted) {
        env->ds.systemAction->actScreenWakeup();
    });

    // MUDS
    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_MudsInserted, this, [=](bool inserted) {
        env->ds.systemAction->actScreenWakeup();
    });
    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_MudsPresent, this, [=](bool isPresent) {
        env->ds.systemAction->actScreenWakeup();
    });
    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_MudsLatched, this, [=](bool latched) {
        env->ds.systemAction->actScreenWakeup();
    });
    // MUDS end

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_OutletDoorState, this, [=](DS_McuDef::OutletDoorState state) {
        env->ds.systemAction->actScreenWakeup();
    });
}

