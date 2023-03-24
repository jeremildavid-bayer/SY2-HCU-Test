#include "Apps/AppManager.h"
#include "CruLink.h"
#include "Common/ImrParser.h"
#include "DataServices/Cru/DS_CruAction.h"
#include "DataServices/Cru/DS_CruData.h"
#include "DataServices/Exam/DS_ExamData.h"
#include "DataServices/Alert/DS_AlertAction.h"
#include "DataServices/System/DS_SystemAction.h"
#include "DataServices/CfgLocal/DS_CfgLocal.h"
#include "DataServices/CfgGlobal/DS_CfgGlobal.h"
#include "DataServices/HardwareInfo/DS_HardwareInfo.h"
#include "DataServices/Capabilities/DS_Capabilities.h"

CruLink::CruLink(QObject *parent, EnvGlobal *env_) :
    QObject(parent),
    env(env_)
{
    envLocal = new EnvLocal("DS_Cru-Link", "CRU_LINK", LOG_MID_SIZE_BYTES);

    connect(&tmrUpdateInjectionPlanData, SIGNAL(timeout()), SLOT(slotUpdateInjectionPlanData()));
    connect(env->appManager, &AppManager::signalAppStarted, this, [=]() {
        slotStart();
    });

    connect(env->ds.hardwareInfo, &DS_HardwareInfo::signalConfigChanged_General_SerialNumber, this, [=](const Config::Item &cfg) {
        QString serialNumber = cfg.value.toString();
        QString wifiSsid = Util::getCruWifiSsid(serialNumber);
        QString wifiPassword = Util::getCruWifiPassword(serialNumber);

        LOG_DEBUG("signalConfigChanged_General_SerialNumber(): SerialNumber=%s, Ssid=%s\n", serialNumber.CSTR(), wifiSsid.CSTR());
        env->ds.cruData->setWifiSsid(wifiSsid);
        env->ds.cruData->setWifiPassword(wifiPassword);
    });
}

CruLink::~CruLink()
{
    delete envLocal;
}

void CruLink::slotStart()
{
    connect(env->ds.examData, &DS_ExamData::signalDataChanged_InjectionPlanTemplateGroups, this, [=]() {
        LOG_DEBUG("signalDataChanged_InjectionPlanTemplateGroups(): Polling to get new injection plan data..\n");
        tmrUpdateInjectionPlanData.stop();
        tmrUpdateInjectionPlanData.start(CRU_INJECTION_PLAN_DATA_POLL_MS);
    });

    connect(env->ds.cfgGlobal, &DS_CfgGlobal::signalConfigChanged, this, [=](){
        QString err;
        QVariantMap map = env->ds.cfgGlobal->getConfigs(&err);

        QString guid = Util::newGuid();
        env->actionMgr->onActionCompleted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus status){
            if (status.state != DS_ACTION_STATE_COMPLETED)
            {
                env->ds.cruAction->actGetConfigs();
            }
        });
        env->ds.cruAction->actPutConfigs(map, guid);

        if (err != "")
        {
            LOG_ERROR("Failed to send config change to CRU (err=%s)\n", err.CSTR());
        }
    });

    connect(env->ds.cruData, &DS_CruData::signalDataChanged_CruLinkStatus, this, [=](const DS_CruDef::CruLinkStatus &cruLinkStatus, const DS_CruDef::CruLinkStatus &prevCruLinkStatus) {
        LOG_INFO("\n");
        LOG_INFO("CRU Link State = %s -> %s\n\n", Util::qVarientToJsonData(ImrParser::ToImr_CruLinkStatus(prevCruLinkStatus)).CSTR(), Util::qVarientToJsonData(ImrParser::ToImr_CruLinkStatus(cruLinkStatus)).CSTR());

        if ((cruLinkStatus.state == DS_CruDef::CRU_LINK_STATE_ACTIVE)
                && (env->ds.cfgLocal->get_Hidden_PMLastPerformedAt() == 0))
        {
            int epochSecsNow = QDateTime::currentSecsSinceEpoch(); //todo the resolution is not enough must be qint64!
            env->ds.cfgLocal->set_Hidden_PMLastPerformedAt(epochSecsNow);
        }

        if ( (cruLinkStatus.state != prevCruLinkStatus.state) &&
             (cruLinkStatus.state == DS_CruDef::CRU_LINK_STATE_INACTIVE) )
        {
            // Link is deactivated
            env->ds.alertAction->deactivate("NotForHumanUseCRU");
            env->ds.alertAction->deactivate("RemoteServicingActive");
            env->ds.alertAction->deactivate("MaintenanceReminder");
            env->ds.alertAction->deactivate("SI2009InvalidConcentration");
            env->ds.alertAction->deactivate("SI2009CannotSwitchConcentrations");
            env->ds.alertAction->deactivate("SI2009CannotUpdateConcentration");
            env->ds.alertAction->deactivate("SI2009HoldPhaseEncountered");
            env->ds.alertAction->deactivate("SI2009NonLoadedConcentration");
            env->ds.alertAction->deactivate("ISI2IncompatibleVersion");
            env->ds.alertAction->deactivate("ISI2WrongUSBPort");
            env->ds.alertAction->deactivate("ISI2BoxUpgradeInProgress");
            env->ds.alertAction->deactivate("ISI2SiemensID2Denial");
            env->ds.alertAction->deactivate("SuiteNameNotAssigned");
            env->ds.alertAction->deactivate("PlatformClockDrifted");
            env->ds.alertAction->deactivate("CRUWirelessNetworkConflict");
            env->ds.alertAction->deactivate("P3TUnusableConcentration");
            env->ds.alertAction->deactivate("CRULowDiskSpace");
            env->ds.alertAction->deactivate("CRULowMemory");
            env->ds.alertAction->deactivate("ReportEmailFailed");
            env->ds.alertAction->deactivate("CRUConfigsReset");
            env->ds.alertAction->deactivate("LicensedFeatureExpiring");
            env->ds.alertAction->deactivate("LicensedFeatureAutoRenewed");
            env->ds.alertAction->deactivate("CRUUpgradeAbortedInPhase1");
            env->ds.alertAction->deactivate("CRUUpgradeAbortedInPhase2");
            env->ds.alertAction->deactivate("CRUUpgradeAbortedInPhase3");
            env->ds.alertAction->deactivate("CRUUpgradeCompleted");
            env->ds.alertAction->deactivate("CRUUpgradeDatabaseBackupInprogress");
            env->ds.alertAction->deactivate("CRUUpgradeDatabaseRestoreInprogress");
            env->ds.alertAction->deactivate("CRUUpgradeInProgress");
            env->ds.alertAction->deactivate("CRUUpgradePhase1Interrupted");
            env->ds.alertAction->deactivate("CRUUpgradePhase2Instructions");
            env->ds.alertAction->deactivate("CRUUpgradePhase3Interrupted");
            env->ds.alertAction->deactivate("CRUUpgradeRemoteNotSupported");
            env->ds.alertAction->deactivate("CRUFactoryDefaultInstructions");
            env->ds.alertAction->deactivate("CRUFactoryDefaultRestoreInProgress");
            env->ds.alertAction->deactivate("CRUFactoryDefaultRemoteNotSupported");
            env->ds.alertAction->deactivate("CRUFactoryDefaultAborted");
        }
    });

    reset();
}

void CruLink::slotUpdateInjectionPlanData()
{
    tmrUpdateInjectionPlanData.stop();

    if ( (state == STATE_IDLE) ||
         (state == STATE_CONNECTING) ||
         (state == STATE_REQ_PROFILE_STARTED) ||
         (state == STATE_REQ_PROFILE_PROGRESS) )
    {
        return;
    }

    LOG_DEBUG("slotUpdateInjectionPlanData()\n");

    if (state == STATE_HEART_BEATING)
    {
        DS_ExamDef::InjectionPlanTemplateGroups groups = env->ds.examData->getInjectionPlanTemplateGroups();
        for (int groupIdx = 0; groupIdx < groups.length(); groupIdx++)
        {
            for (int planIdx = 0; planIdx < groups[groupIdx].planDigests.length(); planIdx++)
            {
                QString groupName = groups[groupIdx].name;
                QString planTemplateGuid = groups[groupIdx].planDigests[planIdx].guid;

                if (groups[groupIdx].planDigests[planIdx].state == DS_ExamDef::INJECTION_PLAN_DIGEST_STATE_INITIALISING)
                {
                    // Plan data is not initialised yet. Update required.
                    QString guid = Util::newGuid();

                    env->actionMgr->onActionCompleted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus status) {
                        DS_ExamDef::InjectionPlanTemplateGroups groups = env->ds.examData->getInjectionPlanTemplateGroups();

                        if ( (groupIdx < groups.length()) &&
                             (planIdx < groups[groupIdx].planDigests.length()) )
                        {
                            DS_ExamDef::InjectionPlanDigest *injectionPlanDigest = &groups[groupIdx].planDigests[planIdx];
                            if (status.state == DS_ACTION_STATE_COMPLETED)
                            {
                                // Heart beating is ok
                                LOG_INFO("slotUpdateInjectionPlanData(): InjectionPlan for group(%s:%s): template guid(%s): GET OK\n", groupName.CSTR(), groups[groupIdx].planDigests[planIdx].name.CSTR(), planTemplateGuid.CSTR());
                                injectionPlanDigest->state = DS_ExamDef::INJECTION_PLAN_DIGEST_STATE_READY;
                            }
                            else
                            {
                                LOG_INFO("slotUpdateInjectionPlanData(): InjectionPlan for group(%s:%s): template guid(%s): GET FAILED: Err=%s\n", groupName.CSTR(), groups[groupIdx].planDigests[planIdx].name.CSTR(), planTemplateGuid.CSTR(), ImrParser::ToImr_DataServiceActionStatusStr(status).CSTR());
                                injectionPlanDigest->state = DS_ExamDef::INJECTION_PLAN_DIGEST_STATE_BAD_DATA;
                            }

                            LOG_DEBUG("slotUpdateInjectionPlanData(): Setting new InjectionPlanTemplateGroups...\n");
                            env->ds.examData->setInjectionPlanTemplateGroups(groups);
                            LOG_DEBUG("slotUpdateInjectionPlanData(): Setting new InjectionPlanTemplateGroups...Done\n");
                        }
                        else
                        {
                            LOG_ERROR("slotUpdateInjectionPlanData(): Failed to set planDigest. Bad Indexes: groupIdx=%d, planIdx=%d\n", groupIdx, planIdx);
                        }

                        /*tmrUpdateInjectionPlanData.stop();
                        tmrUpdateInjectionPlanData.start(CRU_INJECTION_PLAN_DATA_POLL_MS);*/
                    });

                    LOG_INFO("slotUpdateInjectionPlanData(): InjectionPlan for group(%s:%s): template guid(%s): GETTING..\n", groupName.CSTR(), groups[groupIdx].planDigests[planIdx].name.CSTR(), planTemplateGuid.CSTR());
                    env->ds.cruAction->actGetInjectPlan(planTemplateGuid, guid);
                    //goto bail;
                    return;
                }
            }
        }
    }

//bail:
    // TODO: Remove below when confirmed. The timer should only be triggered when InjectionPlanTemplateGroups changed
    /*if (!tmrUpdateInjectionPlanData.isActive())
    {
        tmrUpdateInjectionPlanData.start(CRU_INJECTION_PLAN_DATA_POLL_MS * 10 * 3); // 3 second wait
    }*/
}

void CruLink::reset()
{
    LOG_DEBUG("reset(): Resetting..\n");
    setState(STATE_CONNECTING, 500);
}

void CruLink::setState(State state_, int waitTimeMs)
{
    state = state_;
    QTimer::singleShot(waitTimeMs, this, [=] {
        // Give little time to process data changed as there might be SetData during state transition
        processState();
    });
}

void CruLink::processState()
{
    switch (state)
    {
    case STATE_IDLE:
        LOG_INFO("STATE_IDLE\n");
        break;
    case STATE_CONNECTING:
        LOG_INFO("STATE_CONNECTING\n");
        {
            // Reset CRU-State
            DS_CruDef::CruLinkStatus linkStatus = env->ds.cruData->getCruLinkStatus();
            linkStatus.state = DS_CruDef::CRU_LINK_STATE_INACTIVE;
            linkStatus.quality = DS_CruDef::CRU_LINK_QUALITY_POOR;
            linkStatus.signalLevel = "";
            env->ds.cruData->setCruLinkStatus(linkStatus);

            rxRetryCount = 0;
            setState(STATE_INCOMING_REQUEST_WAITING);
        }
        break;
    case STATE_INCOMING_REQUEST_WAITING:
        LOG_INFO("STATE_INCOMING_REQUEST_WAITING\n");
        {
            DS_CruDef::CruLinkStatus linkStatus = env->ds.cruData->getCruLinkStatus();
            if (linkStatus.state == DS_CruDef::CRU_LINK_STATE_RECOVERING)
            {
                LOG_INFO("STATE_INCOMING_REQUEST_WAITING: Link is recovering..\n");
                setState(STATE_REQ_PROFILE_STARTED);
            }
            else
            {
                setState(state, CRU_RECONNECT_TIMEOUT_MS);
            }
        }
        break;
    case STATE_REQ_PROFILE_STARTED:
        LOG_INFO("STATE_REQ_PROFILE_STARTED\n");
        {
            QString guid = Util::newGuid();

            env->actionMgr->onActionCompleted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus status) {
                if (status.state == DS_ACTION_STATE_COMPLETED)
                {
                    rxRetryCount = 0;
                    setState(STATE_REQ_PROFILE_DONE);
                }
                else
                {
                    LOG_WARNING("Failed to get profile, err=%s\n", status.err.CSTR());

                    if (rxRetryCount++ >= CRU_RX_RETRY_LIMIT)
                    {
                        state = STATE_CONNECTING;
                    }
                    else
                    {
                        state = STATE_REQ_PROFILE_STARTED;
                    }
                    // First CRU message request is not received. It is likely the CRU is not there.
                    // Retry with bigger interval
                    setState(state, CRU_RECONNECT_TIMEOUT_MS * 2);
                }
            });

            env->ds.cruAction->actGetProfile(guid);
            setState(STATE_REQ_PROFILE_PROGRESS);
        }
        break;
    case STATE_REQ_PROFILE_PROGRESS:
        LOG_INFO("STATE_REQ_PROFILE_PROGRESS\n");
        break;
    case STATE_REQ_PROFILE_DONE:
        LOG_INFO("STATE_REQ_PROFILE_DONE\n");
        setState(STATE_HEART_BEATING);
        break;
    case STATE_HEART_BEATING:
        {
            LOG_INFO("STATE_HEART_BEATING\n");
            QString guid = Util::newGuid();

            env->actionMgr->onActionCompleted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus status) {
                if (status.state == DS_ACTION_STATE_COMPLETED)
                {
                    rxRetryCount = 0;
                    setState(state, CRU_LINK_HEART_BEAT_INTERVAL_MS);
                }
                else
                {
                    if (rxRetryCount++ >= CRU_RX_RETRY_LIMIT)
                    {
                        LOG_ERROR("Failed to get digest for %d trials, err=%s. Link will be in recovery mode.\n", CRU_RX_RETRY_LIMIT, status.err.CSTR());
                        state = STATE_CONNECTING;
                    }
                    else
                    {
                        LOG_WARNING("Failed to get digest, err=%s. Retrying to get digest..\n", status.err.CSTR());
                        state = STATE_HEART_BEATING;
                    }
                    setState(state, CRU_WAIT_FOR_NEXT_RETRY_HEART_BEAT);
                }
            });

            env->ds.cruAction->actDigest(guid);
        }
        break;
    default:
        LOG_ERROR("STATE_UNKNOWN_%d\n", state);
        break;
    }
}


