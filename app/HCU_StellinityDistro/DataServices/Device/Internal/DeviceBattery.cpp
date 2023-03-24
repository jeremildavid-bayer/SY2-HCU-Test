#include "Apps/AppManager.h"
#include "DeviceBattery.h"
#include "DataServices/Workflow/DS_WorkflowData.h"
#include "DataServices/Mcu/DS_McuAction.h"
#include "DataServices/Mcu/DS_McuData.h"
#include "DataServices/Alert/DS_AlertAction.h"
#include "DataServices/Alert/DS_AlertData.h"
#include "DataServices/HardwareInfo/DS_HardwareInfo.h"
#include "DataServices/Capabilities/DS_Capabilities.h"
#include "DataServices/CfgLocal/DS_CfgLocal.h"

DeviceBattery::DeviceBattery(QObject *parent, EnvGlobal *env_) :
    ActionBase(parent, env_)
{
    envLocal = new EnvLocal("DS_Device-Battery", "DEVICE_BATTERY", LOG_LRG_SIZE_BYTES * 2);
    batteriesUnsealed = false;
    bmsCommFaultA = false;
    bmsCommFaultB = false;

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });

    connect(&tmrBMSDigestMonitor, &QTimer::timeout, this, [=] {
        env->ds.mcuAction->actBmsDigest();
    });
}

DeviceBattery::~DeviceBattery()
{
    delete envLocal;
}

void DeviceBattery::slotAppInitialised()
{
    connect(env->ds.systemData, &DS_SystemData::signalDataChanged_StatePath, this, [=](DS_SystemDef::StatePath curStatePath, DS_SystemDef::StatePath prevStatePath) {
        if ( ( (prevStatePath == DS_SystemDef::STATE_PATH_IDLE) && (curStatePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING) ) ||
             ( (prevStatePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING) && (curStatePath == DS_SystemDef::STATE_PATH_IDLE) ) )
        {
            LOG_INFO("\n\n-----------------\nStatePath=%s\n\n", ImrParser::ToImr_StatePath(curStatePath).CSTR());
            updateBMSDigestPolling();
        }
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_LinkState, this, [=](DS_McuDef::LinkState linkState) {
        if (linkState == DS_McuDef::LINK_STATE_CONNECTED)
        {
            // Wait for a while to MCU initialise the BMS.
            QTimer::singleShot(POWER_BMS_INIT_DURATION_MS, this, [=] {
                env->ds.mcuAction->actBmsDigest();
                updateBMSDigestPolling();
            });
        }
    });

    connect(env->ds.capabilities, &DS_Capabilities::signalConfigChanged_BMS_DigestPollingInterval, this, [=] {
        updateBMSDigestPolling();
    });

    connect(env->ds.workflowData, &DS_WorkflowData::signalDataChanged_AutomaticQualifiedDischargeStatus, this, [=] {
        updateBMSDigestPolling();
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_PowerStatus, this, [=](const DS_McuDef::PowerStatus &powerStatus, const DS_McuDef::PowerStatus &prevPowerStatus_) {
        static DS_McuDef::PowerStatus lastKnownPowerStatus;

        DS_McuDef::PowerStatus prevPowerStatus = prevPowerStatus_;

        QString baseBoardType = env->ds.hardwareInfo->getGeneral_BaseBoardType().value.toString();
        bool prevPowerStatusOk = false;

        // Filter 'Battery Unknown State'. Note: The battery level is reported to be 'unknown' during startup.
        if ( (baseBoardType == "OCS") || (baseBoardType == "NoBattery") )
        {
            prevPowerStatusOk = true;
        }
        else if ( (baseBoardType == "Battery") &&
                  (prevPowerStatus.batteryLevel != DS_McuDef::BATTERY_LEVEL_NO_BATTERY) &&
                  (prevPowerStatus.batteryLevel != DS_McuDef::BATTERY_LEVEL_UNKNOWN) )
        {
            prevPowerStatusOk = true;
        }

        if (prevPowerStatusOk)
        {
            lastKnownPowerStatus = prevPowerStatus;
        }
        else
        {
            prevPowerStatus = lastKnownPowerStatus;
        }

        if ( (baseBoardType == "Battery") &&
             ( (powerStatus.batteryLevel == DS_McuDef::BATTERY_LEVEL_NO_BATTERY) ||
               (powerStatus.batteryLevel == DS_McuDef::BATTERY_LEVEL_UNKNOWN) ) )
        {
            // Battery level unknown. The level should be ready soon.
            return;
        }

        LOG_INFO("signalDataChanged_PowerStatus: PowerStatus changed from %s to %s\n", Util::qVarientToJsonData(ImrParser::ToImr_PowerStatus(prevPowerStatus)).CSTR(), Util::qVarientToJsonData(ImrParser::ToImr_PowerStatus(powerStatus)).CSTR());

        if (!powerStatus.isAcPowered)
        {
            switch (powerStatus.batteryLevel)
            {
            case DS_McuDef::BATTERY_LEVEL_NO_BATTERY:
                break;
            case DS_McuDef::BATTERY_LEVEL_FULL:
            case DS_McuDef::BATTERY_LEVEL_HIGH:
            case DS_McuDef::BATTERY_LEVEL_MEDIUM:
            case DS_McuDef::BATTERY_LEVEL_LOW:
                env->ds.alertAction->deactivate("InjectorBatteryIsFlat");
                env->ds.alertAction->deactivate("InjectorBatteryIsDead");
                env->ds.alertAction->deactivate("InjectorBatteryIsCritical");
                break;
            case DS_McuDef::BATTERY_LEVEL_FLAT:
                env->ds.alertAction->activate("InjectorBatteryIsFlat");
                env->ds.alertAction->deactivate("InjectorBatteryIsDead");
                env->ds.alertAction->deactivate("InjectorBatteryIsCritical");
                break;
            case DS_McuDef::BATTERY_LEVEL_DEAD:
                env->ds.alertAction->activate("InjectorBatteryIsDead");
                env->ds.alertAction->deactivate("InjectorBatteryIsFlat");
                env->ds.alertAction->deactivate("InjectorBatteryIsCritical");
                break;
            case DS_McuDef::BATTERY_LEVEL_CRITICAL:
                env->ds.alertAction->activate("InjectorBatteryIsCritical");
                env->ds.alertAction->deactivate("InjectorBatteryIsFlat");
                env->ds.alertAction->deactivate("InjectorBatteryIsDead");
                break;
            case DS_McuDef::BATTERY_LEVEL_UNKNOWN:
            default:
                LOG_ERROR("Unknown battery level(%d)\n", powerStatus.batteryLevel);
                break;
            }
        }
        else
        {
            env->ds.alertAction->deactivate("InjectorBatteryIsFlat");
            env->ds.alertAction->deactivate("InjectorBatteryIsDead");
            env->ds.alertAction->deactivate("InjectorBatteryIsCritical");
        }

        if ( (powerStatus.isAcPowered != prevPowerStatus.isAcPowered) ||
             (powerStatus.batteryLevel != prevPowerStatus.batteryLevel) )
        {
            QString alertData = QString().asprintf("%s;%s", ImrParser::ToImr_BatteryLevel(powerStatus.batteryLevel).CSTR(), powerStatus.isAcPowered ? "true" : "false");
            env->ds.alertAction->activate("BatteryLevelChanged", alertData);
        }
    });

    connect(env->ds.alertData, &DS_AlertData::signalDataChanged_ActiveAlerts, this, [=]() {
        // This logic is to deactivate any InjectorBatteryManagementFaults/InjectorBatteryManagementErrors if BMS comm down.
        bmsCommFaultA = env->ds.alertAction->isActivated("GeneralI2CFault", "ALARM_I2C_GENERAL_BATTERY_MANAGEMENT_SYSTEM_A", false);
        bmsCommFaultB = env->ds.alertAction->isActivated("GeneralI2CFault", "ALARM_I2C_GENERAL_BATTERY_MANAGEMENT_SYSTEM_B", false);

        if (!(bmsCommFaultA || bmsCommFaultB))
        {
            // No I2C Fault
            return;
        }

        if (!(env->ds.alertAction->isActivated("InjectorBatteryManagementFault", "", true)))
        {
            // No battery fault
            return;
        }

        if (!(env->ds.alertAction->isActivated("InjectorBatteryManagementError", "", true)))
        {
            // No battery error
            return;
        }

        DS_McuDef::BMSDigests bmsDigests = env->ds.mcuData->getBMSDigests();

        for (int batteryIdx = 0; batteryIdx < bmsDigests.length(); batteryIdx++)
        {
            QString batteryType = (batteryIdx == 0) ? "A" : "B";
            bool bmsCommFault = (batteryIdx == 0) ? bmsCommFaultA : bmsCommFaultB;

            if (bmsCommFault)
            {
                // InjectorBatteryManagementFault alerts
                QString batteryPermanentFailure = QString().asprintf("%s;PermanentFailure", batteryType.CSTR());
                QString batteryOverTemperature = QString().asprintf("%s;OverTemperature", batteryType.CSTR());
                QString batteryUnderTemperature = QString().asprintf("%s;UnderTemperature", batteryType.CSTR());
                if (deactivateInjectorBatteryManagementFaultAlert(batteryPermanentFailure))
                {
                    LOG_WARNING("signalDataChanged_ActiveAlerts(): Battery[%s]: I2C comm down: Deactivating InjectorBatteryManagementFault;%s", batteryType.CSTR(), batteryPermanentFailure.CSTR());
                }
                if (deactivateInjectorBatteryManagementFaultAlert(batteryOverTemperature))
                {
                    LOG_WARNING("signalDataChanged_ActiveAlerts(): Battery[%s]: I2C comm down: Deactivating InjectorBatteryManagementFault;%s", batteryType.CSTR(), batteryOverTemperature.CSTR());
                }
                if (deactivateInjectorBatteryManagementFaultAlert(batteryUnderTemperature))
                {
                    LOG_WARNING("signalDataChanged_ActiveAlerts(): Battery[%s]: I2C comm down: Deactivating InjectorBatteryManagementFault;%s", batteryType.CSTR(), batteryUnderTemperature.CSTR());
                }

                // InjectorBatteryManagementError alerts
                QString batteryCycleCountHigh = QString().asprintf("%s;CycleCountHigh", batteryType.CSTR());
                QString batteryStateOfHealthLow = QString().asprintf("%s;StateOfHealthLow", batteryType.CSTR());
                if (deactivateInjectorBatteryManagementErrorAlert(batteryCycleCountHigh))
                {
                    LOG_WARNING("signalDataChanged_ActiveAlerts(): Battery[%s]: I2C comm down: Deactivating InjectorBatteryManagementError;%s", batteryType.CSTR(), batteryCycleCountHigh.CSTR());
                }
                if (deactivateInjectorBatteryManagementErrorAlert(batteryStateOfHealthLow))
                {
                    LOG_WARNING("signalDataChanged_ActiveAlerts(): Battery[%s]: I2C comm down: Deactivating InjectorBatteryManagementError;%s", batteryType.CSTR(), batteryStateOfHealthLow.CSTR());
                }
            }
        }
    });

    connect(env->ds.hardwareInfo, &DS_HardwareInfo::signalConfigChanged_General_BaseBoardType, this, [=](const Config::Item &cfg) {
        QString baseBoardType = cfg.value.toString();
        if (baseBoardType != "Battery")
        {
            if (env->ds.alertAction->isActivated("InjectorBatteryManagementFault", "", true))
            {
                env->ds.alertAction->deactivate("InjectorBatteryManagementFault");
            }

            if (env->ds.alertAction->isActivated("InjectorBatteryManagementError", "", true))
            {
                env->ds.alertAction->deactivate("InjectorBatteryManagementError");
            }
        }
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_BMSDigests, this, [=](const DS_McuDef::BMSDigests &bmsDigests, const DS_McuDef::BMSDigests &prevBmsDigests) {
        unsealBatteries(bmsDigests); // this will only unseal it once very first time
        updateBatteryManagementFaultAlerts(bmsDigests, prevBmsDigests);
        repairBMSPermanentFailureStatus(bmsDigests);
    });
}

void DeviceBattery::updateBMSDigestPolling()
{
    DS_McuDef::LinkState mcuLinkState = env->ds.mcuData->getLinkState();
    tmrBMSDigestMonitor.stop();

    if (mcuLinkState != DS_McuDef::LINK_STATE_CONNECTED)
    {
        return;
    }

    DS_SystemDef::StatePath statePath = env->ds.systemData->getStatePath();
    DS_WorkflowDef::AutomaticQualifiedDischargeStatus status = env->ds.workflowData->getAutomaticQualifiedDischargeStatus();

    int pollingMs;
    if (statePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING)
    {
        // Servicing mode. Poll BMS digest frequently.
        pollingMs = POWER_BMS_DIGEST_UPDATE_DURING_SERVICE_INTERVAL_MS;
    }
    else if (status.state == DS_WorkflowDef::AQD_STATE_DISCHARGE_PROGRESS)
    {
        // while AQD is in progress, check every minute
        pollingMs = 60 * 1000;
    }
    else
    {
        pollingMs = env->ds.capabilities->get_BMS_DigestPollingInterval() * 60 * 1000;
    }
    if (tmrBMSDigestMonitor.interval() != pollingMs)
    {
        LOG_INFO("updateBMSDigestPolling(): Polling with %dms interval..\n", pollingMs);
    }
    tmrBMSDigestMonitor.start(pollingMs);
}

void DeviceBattery::unsealBatteries(const DS_McuDef::BMSDigests &bmsDigests)
{
    if (batteriesUnsealed)
    {
        return;
    }

    for (int batteryIdx = 0; batteryIdx < bmsDigests.length(); batteryIdx++)
    {
        QString batteryType = (batteryIdx == 0) ? "A" : "B";
        QString securityMode = bmsDigests[batteryIdx].operationStatus.securityMode;

        LOG_DEBUG("unsealBatteries(): Battery[%s]: Battery security mode is %s\n", batteryType.CSTR(), securityMode.CSTR());

        if (securityMode == "SEALED")
        {
            LOG_WARNING("unsealBatteries(): Battery[%s]: Battery security mode is %s. Setting to UNSEALED..\n", batteryType.CSTR(), securityMode.CSTR());
            env->ds.mcuAction->actBmsCommand(batteryIdx, "UNSEAL");
        }
    }
    batteriesUnsealed = true;
}

void DeviceBattery::updateTemperatureAlerts(const DS_McuDef::BMSDigests &bmsDigests, const DS_McuDef::BMSDigests &prevBmsDigests)
{
    QString alertData;

    for (int batteryIdx = 0; batteryIdx < bmsDigests.length(); batteryIdx++)
    {
        bool bmsCommFault = (batteryIdx == 0) ? bmsCommFaultA : bmsCommFaultB;
        QString batteryType = (batteryIdx == 0) ? "A" : "B";
        qint16 batteryTemperature = bmsDigests[batteryIdx].sbsStatus.temperature;
        qint16 prevBatteryTemperature = prevBmsDigests[batteryIdx].sbsStatus.temperature;

        if (bmsCommFault)
        {
            continue;
        }

        if ((batteryTemperature != prevBatteryTemperature))
        {
            LOG_INFO("signalDataChanged_BMSDigests(): Battery[%s]: Temperature Changed from %d°C to %d°C\n", batteryType.CSTR(), prevBatteryTemperature, batteryTemperature);
        }

        int temperatureLimitLow = env->ds.capabilities->get_BMS_TemperatureLimitLow();
        int temperatureLimitHigh = env->ds.capabilities->get_BMS_TemperatureLimitHigh();

        if (batteryTemperature > temperatureLimitHigh)
        {
            // Temperature High
            alertData = QString().asprintf("%s;%s", batteryType.CSTR(), "OverTemperature");
            if (activateInjectorBatteryManagementFaultAlert(alertData))
            {
                env->ds.mcuAction->actChgFetCtl(batteryIdx, false);
            }
        }
        else if (batteryTemperature < temperatureLimitLow)
        {
            // Temperature LOW
            alertData = QString().asprintf("%s;%s", batteryType.CSTR(), "UnderTemperature");
            if (activateInjectorBatteryManagementFaultAlert(alertData))
            {
                env->ds.mcuAction->actChgFetCtl(batteryIdx, false);
            }
        }
        else
        {
            // Temperature OK
            alertData = QString().asprintf("%s;%s", batteryType.CSTR(), "OverTemperature");
            if (deactivateInjectorBatteryManagementFaultAlert(alertData))
            {
                env->ds.mcuAction->actChgFetCtl(batteryIdx, true);
            }

            alertData = QString().asprintf("%s;%s", batteryType.CSTR(), "UnderTemperature");
            if (deactivateInjectorBatteryManagementFaultAlert(alertData))
            {
                env->ds.mcuAction->actChgFetCtl(batteryIdx, true);
            }
        }
    }
}

void DeviceBattery::updateCycleCountAlerts(const DS_McuDef::BMSDigests &bmsDigests, const DS_McuDef::BMSDigests &prevBmsDigests)
{
    QString alertData;
    for (int batteryIdx = 0; batteryIdx < bmsDigests.length(); batteryIdx++)
    {
        bool bmsCommFault = (batteryIdx == 0) ? bmsCommFaultA : bmsCommFaultB;
        QString batteryType = (batteryIdx == 0) ? "A" : "B";

        // alert is A;CycleCountHigh or B;CycleCountHigh
        alertData = QString().asprintf("%s;%s", batteryType.CSTR(), "CycleCountHigh");

        if (bmsCommFault)
        {
            continue;
        }

        qint16 batteryCycleCount = bmsDigests[batteryIdx].sbsStatus.cycleCount;
        int cycleCountHighLimit = env->ds.capabilities->get_BMS_CycleCountLimit();

        if (batteryCycleCount >= cycleCountHighLimit)
        {
            activateInjectorBatteryManagementErrorAlert(alertData);
        }
        else
        {
            deactivateInjectorBatteryManagementErrorAlert(alertData);
        }
    }
}

void DeviceBattery::updateStateOfHealthAlerts(const DS_McuDef::BMSDigests &bmsDigests, const DS_McuDef::BMSDigests &prevBmsDigests)
{
    QString alertData;
    for (int batteryIdx = 0; batteryIdx < bmsDigests.length(); batteryIdx++)
    {
        bool bmsCommFault = (batteryIdx == 0) ? bmsCommFaultA : bmsCommFaultB;
        QString batteryType = (batteryIdx == 0) ? "A" : "B";
        qint16 batteryStateOfHealth = bmsDigests[batteryIdx].sbsStatus.stateOfHealth;
        int stateOfHealthLowLimit = env->ds.capabilities->get_BMS_StateOfHealthLowLimit();

        // alert is "A;StateOfHealthLow" or "B;StateOfHealthLow"
        alertData = QString().asprintf("%s;%s", batteryType.CSTR(), "StateOfHealthLow");

        if (bmsCommFault)
        {
            return;
        }

        if (batteryStateOfHealth <= stateOfHealthLowLimit)
        {
            activateInjectorBatteryManagementErrorAlert(alertData);
        }
        else
        {
            deactivateInjectorBatteryManagementErrorAlert(alertData);
        }
    }
}

void DeviceBattery::updatePermanentFailureAlerts(const DS_McuDef::BMSDigests &bmsDigests, const DS_McuDef::BMSDigests &prevBmsDigests)
{
    QString alertData;
    for (int batteryIdx = 0; batteryIdx < bmsDigests.length(); batteryIdx++)
    {
        bool bmsCommFault = (batteryIdx == 0) ? bmsCommFaultA : bmsCommFaultB;
        QString batteryType = (batteryIdx == 0) ? "A" : "B";

        // alert is "A;PermanentFailure" or "B;PermanentFailure"
        alertData = QString().asprintf("%s;%s", batteryType.CSTR(), "PermanentFailure");

        if (bmsCommFault)
        {
            continue;
        }

        // Check if the target battery has any permanent failures
        // pfStatusBytes is of a type QString. Example "00 00 00 00". Any non-zero represents a permanent failure, but due to the string also containing spaces, we ignore spaces as well
        bool batteryPermanentFailures = (bmsDigests[batteryIdx].pfStatusBytes.contains(QRegularExpression("[^0 ]")) && (bmsDigests[batteryIdx].pfStatusBytes != "--"));

        if (batteryPermanentFailures)
        {
            activateInjectorBatteryManagementFaultAlert(alertData);
        }
        else
        {
            deactivateInjectorBatteryManagementFaultAlert(alertData);
        }
    }
}

bool DeviceBattery::activateInjectorBatteryManagementFaultAlert(QString alertData)
{
    if (!env->ds.alertAction->isActivated("InjectorBatteryManagementFault", alertData, false))
    {
        env->ds.alertAction->activate("InjectorBatteryManagementFault", alertData);
        return true;
    }
    return false;
}

bool DeviceBattery::deactivateInjectorBatteryManagementFaultAlert(QString alertData)
{
    if (env->ds.alertAction->isActivated("InjectorBatteryManagementFault", alertData, false))
    {
        env->ds.alertAction->deactivate("InjectorBatteryManagementFault", alertData);
        return true;
    }
    return false;
}

bool DeviceBattery::activateInjectorBatteryManagementErrorAlert(QString alertData)
{
    if (!env->ds.alertAction->isActivated("InjectorBatteryManagementError", alertData, false))
    {
        env->ds.alertAction->activate("InjectorBatteryManagementError", alertData);
        return true;
    }
    return false;
}

bool DeviceBattery::deactivateInjectorBatteryManagementErrorAlert(QString alertData)
{
    if (env->ds.alertAction->isActivated("InjectorBatteryManagementError", alertData, false))
    {
        env->ds.alertAction->deactivate("InjectorBatteryManagementError", alertData);
        return true;
    }
    return false;
}

void DeviceBattery::updateBatteryManagementFaultAlerts(const DS_McuDef::BMSDigests &bmsDigests, const DS_McuDef::BMSDigests &prevBmsDigests)
{
    updateTemperatureAlerts(bmsDigests, prevBmsDigests);
    updateCycleCountAlerts(bmsDigests, prevBmsDigests);
    updateStateOfHealthAlerts(bmsDigests, prevBmsDigests);
    updatePermanentFailureAlerts(bmsDigests, prevBmsDigests);
}

void DeviceBattery::repairBMSPermanentFailureStatus(const DS_McuDef::BMSDigests &bmsDigests)
{
    DS_SystemDef::StatePath statePath = env->ds.systemData->getStatePath();
    if (statePath != DS_SystemDef::STATE_PATH_IDLE)
    {
        // Unexpected StatePath
        return;
    }

    int pfResetTrialLimit = env->ds.capabilities->get_BMS_PFResetTrialLimit();

    for (int batteryIdx = 0; batteryIdx < bmsDigests.length(); batteryIdx++)
    {
        DS_McuDef::BMSDigestParamsPfStatus pfStatus = bmsDigests[batteryIdx].pfStatus;

        // Check if CFETF bit is the only set bit
        if ( (pfStatus.dataFlashWearoutFailure) ||
             (pfStatus.instructionFlashCehcksumFailure) ||
             (pfStatus.safetyOvertemperatureFETFailure) ||
             (pfStatus.openThermistorTS3Failure) ||
             (pfStatus.openThermistorTS2Failure) ||
             (pfStatus.openThermistorTS1Failure) ||
             (pfStatus.companionAfeXReadyFailure) ||
             (pfStatus.companionAfeOveride) ||
             (pfStatus.afeCommunicationFailure) ||
             (pfStatus.afeRegisterFailure) ||
             (pfStatus.dischargeFETFailure) ||
             (!pfStatus.chargeFETFailure) ||
             (pfStatus.voltageImbalanceWhilePackRestFailure) ||
             (pfStatus.safetyOvertemperatureCellFailure) ||
             (pfStatus.safetyOvercurrentInDischarge) ||
             (pfStatus.safetyOvercurrentInCharge) ||
             (pfStatus.safetyCellOvervoltageFailure) ||
             (pfStatus.safetyCellUndervoltageFailure) )
        {
            // PermanentFailStatus has NOT just the single CFETF bit
            LOG_DEBUG("repairBMSPermanentFailureStatus(): [%d]: PermanentFailStatus has NOT just the single CFETF bit, pfStatus=%s\n", batteryIdx, Util::qVarientToJsonData(ImrParser::ToImr_BMSDigestParamsPfStatus(pfStatus)).CSTR());
            continue;
        }

        DS_McuDef::BMSDigestParamsSBSStatus sbsStatus = bmsDigests[batteryIdx].sbsStatus;

        // Get current and last battery id
        QString batteryId = QString().asprintf("%04x-%04x", sbsStatus.serialNumber, sbsStatus.manufactureDate);
        QString lastBatteryId =
            (batteryIdx == 0)
            ?
            env->ds.cfgLocal->get_Hidden_BMSLastBatteryIdA()
            :
            env->ds.cfgLocal->get_Hidden_BMSLastBatteryIdB();

        // Check if battery id is changed
        //LOG_DEBUG("repairBMSPermanentFailureStatus(): [%d]: batteryId=%s\n", batteryIdx, batteryId.CSTR());
        if (batteryId != lastBatteryId)
        {
            LOG_WARNING("repairBMSPermanentFailureStatus(): [%d], Battery ID changed from %s to %s\n", batteryIdx, lastBatteryId.CSTR(), batteryId.CSTR());
            if (batteryIdx == 0)
            {
                env->ds.cfgLocal->set_Hidden_BMSLastBatteryIdA(batteryId);
            }
            else
            {
                env->ds.cfgLocal->set_Hidden_BMSLastBatteryIdB(batteryId);
            }
        }

        // Get current cycle count
        quint16 cycleCount = bmsDigests[batteryIdx].sbsStatus.cycleCount;

        // Get last reset cycle count parameters
        int lastPfResetCycleCount = (batteryIdx == 0) ? env->ds.cfgLocal->get_Hidden_BMSLastPFResetCycleCountA() : env->ds.cfgLocal->get_Hidden_BMSLastPFResetCycleCountB();

        int pfResetCount = (batteryIdx == 0) ? env->ds.cfgLocal->get_Hidden_BMSPFResetCountA() : env->ds.cfgLocal->get_Hidden_BMSPFResetCountB();
        //LOG_DEBUG("repairBMSPermanentFailureStatus(): [%d]: cycleCount=%d\n", batteryIdx, pfResetCount);

        if (lastPfResetCycleCount != cycleCount)
        {
            LOG_WARNING("repairBMSPermanentFailureStatus(): [%d]: Pf Reset Cycle Count changed from %u to %u\n", batteryIdx, lastPfResetCycleCount, cycleCount);
        }

        if ( (batteryId != lastBatteryId) ||
             (lastPfResetCycleCount != cycleCount) )
        {
            // CycleCount is changed OR Battery is changed.
            // Reset Cycle Count Parameters

            LOG_INFO("repairBMSPermanentFailureStatus(): [%d]: Resetting Cycle Count Parameters\n", batteryIdx);
            lastPfResetCycleCount = cycleCount;
            pfResetCount = 0;

            if (batteryIdx == 0)
            {
                env->ds.cfgLocal->set_Hidden_BMSLastPFResetCycleCountA(lastPfResetCycleCount);
                env->ds.cfgLocal->set_Hidden_BMSPFResetCountA(pfResetCount);
            }
            else
            {
                env->ds.cfgLocal->set_Hidden_BMSLastPFResetCycleCountB(lastPfResetCycleCount);
                env->ds.cfgLocal->set_Hidden_BMSPFResetCountB(pfResetCount);
            }
        }

        if (pfResetCount >= pfResetTrialLimit)
        {
            // PFReset Count reached limit. Do not try to clear register anymore.
            LOG_WARNING("repairBMSPermanentFailureStatus(): [%d]: PFReset Count reached limit (%d >= %d). Not clearing PFSTatus register..\n", batteryIdx, pfResetCount, pfResetTrialLimit);
            continue;
        }

        LOG_INFO("repairBMSPermanentFailureStatus(): [%d]: lastPfResetCycleCount=%d, pfResetCount=%d: Clearing PFSTatus Register..\n", batteryIdx, lastPfResetCycleCount, pfResetCount);

        // Clear PFStatus Register
        env->ds.mcuAction->actBmsCommand(batteryIdx, "0029");

        // Update PFResetCount
        pfResetCount++;
        if (batteryIdx == 0)
        {
            env->ds.cfgLocal->set_Hidden_BMSPFResetCountA(pfResetCount);
        }
        else
        {
            env->ds.cfgLocal->set_Hidden_BMSPFResetCountB(pfResetCount);
        }

        // Raise Alert
        LOG_INFO("repairBMSPermanentFailureStatus(): [%d]: Raising InjectorBatteryPFReset alert..\n", batteryIdx);
        QString alertData = QString().asprintf("%s;%s", (batteryIdx == 0) ? "A" : "B", Util::qVarientToJsonData(ImrParser::ToImr_BMSDigest(bmsDigests[batteryIdx])).CSTR());
        env->ds.alertAction->activate("InjectorBatteryPFReset", alertData);
    }
}
