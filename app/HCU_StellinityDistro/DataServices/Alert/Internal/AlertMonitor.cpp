#include "Apps/AppManager.h"
#include "AlertMonitor.h"
#include "Common/ImrParser.h"
#include "DataServices/Alert/DS_AlertAction.h"
#include "DataServices/Alert/DS_AlertData.h"
#include "DataServices/Exam/DS_ExamAction.h"
#include "DataServices/System/DS_SystemAction.h"
#include "DataServices/Upgrade/DS_UpgradeData.h"
#include "DataServices/Mcu/DS_McuAction.h"
#include "DataServices/CfgGlobal/DS_CfgGlobal.h"

AlertMonitor::AlertMonitor(QObject *parent, EnvGlobal *env_) :
    QObject(parent),
    env(env_)
{
    envLocal = new EnvLocal("DS_Alert-Monitor", "ALERT_MONITOR");

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });

    tmrSaveLastAlertsWaiting.setSingleShot(true);
    connect(&tmrSaveLastAlertsWaiting, &QTimer::timeout, this, [=] {
        LOG_DEBUG("Saving last alerts..\n");
        env->ds.alertAction->saveLastAlerts();
    });
}

AlertMonitor::~AlertMonitor()
{
    delete envLocal;
}

void AlertMonitor::slotAppInitialised()
{
    connect(env->ds.alertData, &DS_AlertData::signalDataChanged_AllAlerts, this, [=] {
        if (!tmrSaveLastAlertsWaiting.isActive())
        {
            tmrSaveLastAlertsWaiting.start(ALERT_LOGGER_MONITOR_POLL_INTERVAL_MS);
        }
    });

    connect(env->ds.alertData, &DS_AlertData::signalDataChanged_ActiveAlerts, this, [=](const QVariantList &activeAlerts, const QVariantList &prevActiveAlerts) {
        handleActiveAlerts(activeAlerts, prevActiveAlerts);
        handleRemoteServicing();

        // enable USB when in Service mode
        if ( env->ds.alertAction->isActivated("ServiceModeRestartRequired", "", true) )
        {
            QString buildType = getenv("BUILD_TYPE");
            if ( (buildType == BUILD_TYPE_VNV) || (buildType == BUILD_TYPE_REL) )
            {
                // Enable new USB connections
                LOG_INFO("Enabling new USB connections\n");
                system(QString().asprintf("%s authorize", PATH_CONTROL_USB).CSTR());
            }
        }
    });

    connect(env->ds.alertData, &DS_AlertData::signalDataChanged_ActiveFatalAlerts, this, [=] {
        handleActiveFatalAlerts();
    });

    connect(env->ds.upgradeData, &DS_UpgradeData::signalDataChanged_UpgradeDigest, this, [=] {
        handleActiveFatalAlerts();
    });

    connect(env->ds.systemData, &DS_SystemData::signalDataChanged_StatePath, this, [=] {
        // handleActiveFatalAlerts() might change the statePath. So run it with timer.
        QTimer::singleShot(1, [=] { handleActiveFatalAlerts(); });
    });

    handleActiveAlerts(env->ds.alertData->getActiveAlerts(), env->ds.alertData->getActiveAlerts());
    handleActiveFatalAlerts();
}

void AlertMonitor::handleActiveAlerts(const QVariantList &activeAlerts, const QVariantList &prevActiveAlerts)
{
    if (env->state == EnvGlobal::STATE_EXITING)
    {
        return;
    }

    QVariantList activeSystemAlerts;
    QVariantList activeFatalAlerts;

    QVariantMap activeFluidSourceAlerts;
    QVariantList fluidSourceMudsAlerts, fluidSourceSudsAlerts, fluidSourceWcAlerts;
    QVariantList fluidSourceSyringe1Alerts, fluidSourceSyringe2Alerts, fluidSourceSyringe3Alerts;
    QVariantList fluidSourceBottle1Alerts, fluidSourceBottle2Alerts, fluidSourceBottle3Alerts;

    foreach (QVariant alert, activeAlerts)
    {
        QString codeName = alert.toMap()[_L("CodeName")].toString();
        QString severity = alert.toMap()[_L("Severity")].toString();
        QString data = alert.toMap()[_L("Data")].toString();
        QString systemAlert = alert.toMap()[_L("SystemAlert")].toString();

        // Build Fatal Alerts
        if (severity == _L("Fatal"))
        {
            activeFatalAlerts.append(alert);
        }

        // Build System Alerts
        if ((severity == _L("Fatal")) || (systemAlert == _L("true")))
        {
            activeSystemAlerts.append(alert);
        }

        // Update Fluid
        if ( (codeName == _L("HeatMaintainerPowerFault")) ||
             (codeName == _L("UsedMUDSDetected")) ||
             (codeName == _L("MUDSUseLifeExceeded")) ||
             (codeName == _L("MUDSUseLifeLimitEnforced")) ||
             (codeName == _L("HeatMaintainerCutoff")) ||
             (codeName == _L("MUDSRestraintUnlatched")) ||
             (codeName == _L("MUDSEjectedRemovalRequired")) ||
             (codeName == _L("OutletAirDetected")) ||
             (codeName == _L("OutletAirDoorLeftOpen")) ||
             (codeName == _L("StopcockUnintendedMotionDetected")) )
        {
            fluidSourceMudsAlerts.append(alert);
            fluidSourceSyringe1Alerts.append(alert);
            fluidSourceSyringe2Alerts.append(alert);
            fluidSourceSyringe3Alerts.append(alert);
        }

        else if ( (codeName == _L("PlungerEngagementFault")) ||
                  (codeName == _L("MotorPositionFault")) ||
                  (codeName == _L("PlungerNotDetected")) ||
                  (codeName == _L("StopcockEngagementFault")) ||
                  (codeName == _L("MudsPrimeFailed")) ||
                  (codeName == _L("InsufficientVolumeForCalSlack")) ||
                  (codeName == _L("InsufficientVolumeForMUDSPrime")) ||
                  (codeName == _L("InsufficientVolumeForSUDSPrime")) ||
                  (codeName == _L("InsufficientVolumeForReservoirAirCheck")) ||
                  (codeName == _L("InsufficientVolumeForReservoirAirRecovery")) ||
                  (codeName == _L("CalSlackFailed")) ||
                  (codeName == _L("ReservoirAirCheckCalFailed")) ||
                  (codeName == _L("ReservoirAirCheckFailed")) ||
                  (codeName == _L("ReservoirAirDetected")) ||
                  (codeName == _L("ReservoirAirRecoveryFailed")) )
        {

             if (data.indexOf("RS0") >= 0)
             {
                 fluidSourceSyringe1Alerts.append(alert);
                 fluidSourceMudsAlerts.append(alert);
             }
             else if (data.indexOf("RC1") >= 0)
             {
                 fluidSourceSyringe2Alerts.append(alert);
                 fluidSourceMudsAlerts.append(alert);
             }
             else if (data.indexOf("RC2") >= 0)
             {
                 fluidSourceSyringe3Alerts.append(alert);
                 fluidSourceMudsAlerts.append(alert);
             }
        }
        else if ( (codeName == _L("SUDSChangeRequired")) ||
                  (codeName == _L("SUDSSensorFault")) ||
                  // intentionally excluded from Suds alert since we don't want to show alertIcon due to this. This is only used internally
                  //(codeName == _L("EnsureNoPatientConnected")) ||
                  (codeName == _L("AutoPrimeFailed")) ||
                  (codeName == _L("RepreloadReprimeRequired")) )
        {
            fluidSourceSudsAlerts.append(alert);
        }
        else if (codeName == _L("BulkUseLifeExceeded"))
        {
            if (data.indexOf("BS0") >= 0)
            {
                fluidSourceBottle1Alerts.append(alert);
            }
            else if (data.indexOf("BC1") >= 0)
            {
                fluidSourceBottle2Alerts.append(alert);
            }
            else if (data.indexOf("BC2") >= 0)
            {
                fluidSourceBottle3Alerts.append(alert);
            }
        }
        else if (codeName == _L("ContrastBottleCountReached"))
        {
            fluidSourceBottle2Alerts.append(alert);
            fluidSourceBottle3Alerts.append(alert);
        }
        else if ( (codeName == _L("WasteContainerMissing")) ||
                  (codeName == _L("WasteContainerFull")) )
        {
            fluidSourceWcAlerts.append(alert);
        }
    }

    // Set Fatal Alerts
    env->ds.alertData->setActiveFatalAlerts(activeFatalAlerts);

    // Set System Alerts
    QVariantList mergedActiveSystemAlerts = getMergedSystemAlerts(activeSystemAlerts);
    env->ds.alertData->setActiveSystemAlerts(mergedActiveSystemAlerts);

    // Set FluidSource Alerts
    QVariantList mergedFluidSourceMudsAlerts = env->ds.alertAction->getMergedAlerts(fluidSourceMudsAlerts);
    activeFluidSourceAlerts.insert("ML", mergedFluidSourceMudsAlerts);
    activeFluidSourceAlerts.insert("PL", fluidSourceSudsAlerts);
    activeFluidSourceAlerts.insert("WC", fluidSourceWcAlerts);
    activeFluidSourceAlerts.insert("BS0", fluidSourceBottle1Alerts);
    activeFluidSourceAlerts.insert("BC1", fluidSourceBottle2Alerts);
    activeFluidSourceAlerts.insert("BC2", fluidSourceBottle3Alerts);
    activeFluidSourceAlerts.insert("RS0", fluidSourceSyringe1Alerts);
    activeFluidSourceAlerts.insert("RC1", fluidSourceSyringe2Alerts);
    activeFluidSourceAlerts.insert("RC2", fluidSourceSyringe3Alerts);
    env->ds.alertData->setActiveFluidSourceAlerts(activeFluidSourceAlerts);
}

void AlertMonitor::handleActiveFatalAlerts()
{
    QString buildType = env->ds.systemData->getBuildType();

    if ( (buildType == _L(BUILD_TYPE_PROD)) ||
         (buildType == _L(BUILD_TYPE_DEV)) ||
         (buildType == _L(BUILD_TYPE_SQA)) )
    {
        return;
    }

    bool tradeShowModeEnabled = env->ds.cfgGlobal->get_Service_TradeshowModeEnabled();

    if (tradeShowModeEnabled)
    {
        return;
    }

    DS_SystemDef::StatePath statePath = env->ds.systemData->getStatePath();

    if (statePath == DS_SystemDef::STATE_PATH_ERROR)
    {
        return;
    }

    DS_UpgradeDef::UpgradeDigest upgradeDigest = env->ds.upgradeData->getUpgradeDigest();
    if (upgradeDigest.state != DS_UpgradeDef::STATE_READY)
    {
        LOG_INFO("Fatal alert occurred during upgrade. Will set to Error state later.\n");
    }

    QVariantList activeFatalAlerts = env->ds.alertData->getActiveFatalAlerts();

    if (activeFatalAlerts.length() == 0)
    {
        // No active fatal alerts
        return;
    }

    // Enter Error state if required
    if (statePath == DS_SystemDef::STATE_PATH_STARTUP_UNKNOWN)
    {
        LOG_INFO("Fatal alert occurred. Will set to Error state later.\n");
    }
    else if ( (statePath == DS_SystemDef::STATE_PATH_EXECUTING) ||
              (statePath == DS_SystemDef::STATE_PATH_BUSY_HOLDING) ||
              (statePath == DS_SystemDef::STATE_PATH_BUSY_FINISHING) )
    {
        LOG_WARNING("Fatal alert occurred. Injection shall be stopped soon.\n");
    }
    else
    {
        env->ds.mcuAction->actStopAll();

        LOG_WARNING("Fatal alert occurred: %s\n", Util::qVarientToJsonData(activeFatalAlerts).CSTR());
        LOG_WARNING("Setting statePath to %s\n", ImrParser::ToImr_StatePath(DS_SystemDef::STATE_PATH_ERROR).CSTR());
        env->ds.systemAction->actSetStatePath(DS_SystemDef::STATE_PATH_ERROR);
    }
}

QVariantList AlertMonitor::getMergedSystemAlerts(const QVariantList &alerts)
{
    QVariantList mergedAlerts;

    for (int i = 0 ; i < alerts.length(); i++)
    {
        QVariantMap alert = alerts[i].toMap();
        bool merged = false;
        for (int j = 0; j < mergedAlerts.length(); j++)
        {
            QVariantMap baseAlert = mergedAlerts[j].toMap();
            if (baseAlert[_L("CodeName")].toString() == alert[_L("CodeName")].toString())
            {
                // found match, append to base
                QString mergedData = baseAlert[_L("Data")].toString() + ", " + alert[_L("Data")].toString();
                baseAlert[_L("Data")] = mergedData;
                mergedAlerts[j] = baseAlert;
                merged = true;
                break;
            }
        }
        if (!merged)
        {
            mergedAlerts.append(alert);
        }
    }

    return mergedAlerts;
}

void AlertMonitor::handleRemoteServicing()
{
    DS_SystemDef::StatePath statePath = env->ds.systemData->getStatePath();

    if ( (env->ds.alertAction->isActivated("RemoteServicingEnabled")) &&
         ( (statePath == DS_SystemDef::STATE_PATH_READY_ARMED) || (statePath == DS_SystemDef::STATE_PATH_EXECUTING) ) )
    {
         LOG_WARNING("handleRemoteServicing(): Alert 'RemoteServicingEnabled' is activated while statePath=%s. Deactivating alert..\n", ImrParser::ToImr_StatePath(statePath).CSTR());
         env->ds.alertAction->deactivate("RemoteServicingEnabled");
    }

    if (env->ds.alertAction->isActivated("RemoteServicingRestartRequired"))
    {
        if (statePath == DS_SystemDef::STATE_PATH_SERVICING)
        {
            // StatePath set ok
        }
        else if (statePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING)
        {
            LOG_INFO("handleRemoteServicing(): Alert 'RemoteServicingRestartRequired' activated: Setting statePath from %s->%s..\n",
                     ImrParser::ToImr_StatePath(statePath).CSTR(),
                     ImrParser::ToImr_StatePath(DS_SystemDef::STATE_PATH_SERVICING).CSTR());
            env->ds.systemAction->actSetStatePath(DS_SystemDef::STATE_PATH_SERVICING);
        }
        else
        {
            LOG_INFO("handleRemoteServicing(): Alert 'RemoteServicingRestartRequired' activated: Setting statePath from %s->%s..\n",
                     ImrParser::ToImr_StatePath(statePath).CSTR(),
                     ImrParser::ToImr_StatePath(DS_SystemDef::STATE_PATH_BUSY_SERVICING).CSTR());

            // Set statePath to BusyServicing THEN Servicing
            QString guid = Util::newGuid();
            env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
                if (curStatus.state == DS_ACTION_STATE_STARTED)
                {
                    env->actionMgr->onActionCompleted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus status) {
                        if (status.state == DS_ACTION_STATE_COMPLETED)
                        {
                            DS_SystemDef::StatePath curStatePath = env->ds.systemData->getStatePath();
                            if (curStatePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING)
                            {
                                handleRemoteServicing();
                            }
                        }
                        else
                        {
                            LOG_WARNING("handleRemoteServicing(): Alert 'RemoteServicingRestartRequired' activated: Failed to set statePath, Err Status=%s\n", ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());
                        }
                    });
                }
                else
                {
                    LOG_WARNING("handleRemoteServicing(): Alert 'RemoteServicingRestartRequired' activated: Failed to set statePath, Err Status=%s\n", ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());
                }
            });

            env->ds.systemAction->actSetStatePath(DS_SystemDef::STATE_PATH_BUSY_SERVICING, guid);
        }
    }
}
