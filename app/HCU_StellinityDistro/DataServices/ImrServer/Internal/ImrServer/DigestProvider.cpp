#include "Apps/AppManager.h"
#include "DigestProvider.h"
#include "Common/ImrParser.h"
#include "DataServices/Exam/DS_ExamData.h"
#include "DataServices/Exam/DS_ExamAction.h"
#include "DataServices/Device/DS_DeviceData.h"
#include "DataServices/System/DS_SystemData.h"
#include "DataServices/Mcu/DS_McuData.h"
#include "DataServices/Alert/DS_AlertAction.h"
#include "DataServices/Alert/DS_AlertData.h"
#include "DataServices/Workflow/DS_WorkflowData.h"
#include "DataServices/ImrServer/DS_ImrServerData.h"
#include "DataServices/ImrServer/DS_ImrServerAction.h"
#include "DataServices/Capabilities/DS_Capabilities.h"

DigestProvider::DigestProvider(QObject *parent, EnvGlobal *env_) :
    QObject(parent),
    env(env_)
{
    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });

    envLocal = new EnvLocal("DS_ImrServer-DigestProvider", "IMR_SERVER_DIGEST_PROVIDER", LOG_MID_SIZE_BYTES);

    digestId = 0;
    digestIdToSave = 0;
    digestIdSaved = -1;
    digestWrappedCount = 0;
    commitLocked = false;
    lastDigestIsReady = false;
}

DigestProvider::~DigestProvider()
{
    delete envLocal;
}

void DigestProvider::slotAppInitialised()
{
    loadLastDigest();

    connect(&tmrReleaseLastDigest, &QTimer::timeout, this, [=] {
        if (env->state == EnvGlobal::STATE_EXITING)
        {
            return;
        }

        QMutexLocker locker(&mutexSingleton);
        tmrReleaseLastDigest.stop();
        lastDigestIsReady = true;

        if (!tmrDigestSave.isActive())
        {
            //LOG_DEBUG("tmrReleaseLastDigestTimeout: lastDigestIsReady=true. Restarting the tmrDigestSave..\n");
            tmrDigestSave.start(1);
        }
    });

    connect(&tmrDigestSave, &QTimer::timeout, this, [=] {
        //test QMutexLocker locker(&mutexSingleton);
        tmrDigestSave.stop();

        // New digest available
        if (digestIdToSave <= digestIdSaved)
        {
            // current digest already saved
            //LOG_DEBUG("tmrDigestSave(): digestIdToSave is changed from %d to %d\n", digestIdToSave, digestIdSaved + 1);
            digestIdToSave = digestIdSaved + 1;
        }

        QVariantList digestList = getDigests(digestIdToSave, 2);
        if (digestList.length() > 0)
        {
            QVariantMap newDigest = digestList.first().toMap();
            quint32 lastDigestId = getLastDigestId();

            // Save digest
            saveDigest(newDigest);

            // Send digest via web-socket
            if (env->ds.capabilities->get_General_WebApplicationEnabled())
            {
                QVariantMap digestDelta;
                getDigestDeltaMap(digestDelta, digestIdToSave - 1);
                env->ds.imrServerAction->actUpdateDataGroup("DigestDelta", digestDelta);
            }

            digestIdSaved = digestIdToSave;
            //LOG_DEBUG("tmrDigestSaveTimeout(): Digest saved: DigestIdSaved=%d, LastDigestId=%d\n", digestIdToSave, lastDigestId);

            if (digestIdToSave < lastDigestId)
            {
                LOG_DEBUG("tmrDigestSaveTimeout: DigestIdToSave=%d, digestId=%d: MgetLastDigestIdore digests should be saved. digestIdToSave incremented\n", digestIdToSave, lastDigestId);
                digestIdToSave++;
            }
            else
            {
                //LOG_DEBUG("tmrDigestSaveTimeout: digestIdToSave=%d, digestId=%d: Current digest is still in progress after save.\n", digestIdToSave, getLastDigestId());
            }

            if (digestList.length() > 1)
            {
                // More digests exist. Start the timer again to save more digests
                //LOG_DEBUG("tmrDigestSaveTimeout: More digests exist. Start the timer again to save more digests..\n");
                tmrDigestSave.start(IMR_MIN_DIGEST_INTERVAL_MS * 0.5);
            }
        }
    });

    // Debugging feature used to investigate communication timings
    int spontaneousDigestMillis = env->ds.capabilities->get_Network_SpontaneousDigestInterval();
    if (spontaneousDigestMillis > 0)
    {
        // Ensure that this feature is not enabled in a human use build
        QString hcuBuildType = env->ds.systemData->getBuildType();
        if (hcuBuildType != "REL")
        {
            // Retrieve the on/off interval periods
            int spontaneousDigestOnMins = env->ds.capabilities->get_Network_SpontaneousDigestOnInterval();
            int spontaneousDigestOffMins = env->ds.capabilities->get_Network_SpontaneousDigestOffInterval();

            // Convert the interval periods to counts
            int spontaneousDigestOnCount = spontaneousDigestOnMins * 60 * ((int) 1000 / spontaneousDigestMillis);
            int spontaneousDigestOffCount = spontaneousDigestOffMins * 60 * ((int) 1000 / spontaneousDigestMillis);

            LOG_DEBUG("slotAppInitialised: spontaneous digest interval is %dms, with ON (%dmins) count of %d and OFF (%dmins) count of %d. Starting timer.\n",
                      spontaneousDigestMillis, spontaneousDigestOnMins, spontaneousDigestOnCount, spontaneousDigestOffMins, spontaneousDigestOffCount);

            // Start a timer for the configured interval
            connect(&tmrSpontaneousDigest, &QTimer::timeout, this, [=] {
                tmrSpontaneousDigest.stop();

                if (spontaneousDigestsAdded >= spontaneousDigestOnCount)
                {
                    LOG_DEBUG("tmrSpontaneousDigest: added spontaneous digest count reached max: %d. Turning feature OFF for %d counts.\n", spontaneousDigestOnCount, spontaneousDigestOffCount);
                    spontaneousDigestsAdded = 0;
                    spontaneousDigestOn = false;
                }
                else if (spontaneousDigestsSkipped >= spontaneousDigestOffCount)
                {
                    LOG_DEBUG("tmrSpontaneousDigest: skipped spontaneous digest count reached max: %d. Turning feature ON for %d counts.\n", spontaneousDigestOffCount, spontaneousDigestOnCount);
                    spontaneousDigestsSkipped = 0;
                    spontaneousDigestOn = true;
                }

                if (spontaneousDigestOn)
                {
                    QMutexLocker locker(&mutexSingleton);
                    Digest digest = getLastDigest();
                    addNewDigest(digest);
                    spontaneousDigestsAdded += 1;
                    //LOG_DEBUG("tmrSpontaneousDigest: added spontaneous digest #%d out of %d.\n", spontaneousDigestsAdded, SpontaneousDigestOnInterval);
                }
                else
                {
                    spontaneousDigestsSkipped += 1;
                    //LOG_DEBUG("tmrSpontaneousDigest: skipped spontaneous digest #%d out of %d.\n", spontaneousDigestsSkipped, SpontaneousDigestOffInterval);
                }

                tmrSpontaneousDigest.start(spontaneousDigestMillis);
            });

            tmrSpontaneousDigest.start(spontaneousDigestMillis);
        }
        else
        {
            LOG_DEBUG("slotAppInitialised: spontaneous digest interval is %dms, but HCU Build Type is %s. Not enabling feature!\n", spontaneousDigestMillis, hcuBuildType.CSTR());
        }
    }

    connect(env->ds.systemData, &DS_SystemData::signalDataChanged_DigestLocked, this, [=](bool locked) {
       QMutexLocker locker(&mutexSingleton);
       if (locked)
       {
           // Add write-buffer digest while locked
           Digest digest = getLastDigest();
           addNewDigest(digest);
       }
       commitLocked = locked;
    });

    connect(env->ds.alertData, &DS_AlertData::signalDataChanged_AllAlerts, this, [=] {
        QMutexLocker locker(&mutexSingleton);
        Digest digest = getLastDigest();
        QDateTime curTime = QDateTime::currentDateTimeUtc();
        digest.setAlertsUpdatedAt(Util::qDateTimeToUtcDateTimeStr(curTime));
        addNewDigest(digest);
    });

    connect(env->ds.mcuData , &DS_McuData::signalDataChanged_Pressure, this, [=](int val) {
        QMutexLocker locker(&mutexSingleton);
        Digest digest = getLastDigest();
        digest.setCurrentPressure(val);
        addNewDigest(digest);
    });

    connect(env->ds.examData, &DS_ExamData::signalDataChanged_ExecutedSteps, this, [=](const DS_ExamDef::ExecutedSteps &val) {
        QMutexLocker locker(&mutexSingleton);
        QVariantList valImr = ImrParser::ToImr_ExecutedSteps(val);
        Digest digest = getLastDigest();
        digest.setStepProgress(valImr);
        addNewDigest(digest);
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_PowerStatus, this, [=](const DS_McuDef::PowerStatus &val) {
        QMutexLocker locker(&mutexSingleton);
        QVariantMap valImr = ImrParser::ToImr_PowerStatus(val);
        Digest digest = getLastDigest();
        digest.setPowerStatus(valImr);
        addNewDigest(digest);
    });

    connect(env->ds.examData, &DS_ExamData::signalDataChanged_IsAirCheckNeeded, this, [=](bool val) {
        QMutexLocker locker(&mutexSingleton);
        bool valImr = val;
        Digest digest = getLastDigest();
        digest.setIsAirCheckNeeded(valImr);
        addNewDigest(digest);
    });

    connect(env->ds.examData, &DS_ExamData::signalDataChanged_InjectionPlan, this, [=](const DS_ExamDef::InjectionPlan &val) {
        QMutexLocker locker(&mutexSingleton);
        QVariantMap valImr = ImrParser::ToImr_InjectionPlan(val);
        Digest digest = getLastDigest();
        digest.setInjectionPlan(valImr);
        addNewDigest(digest);
    });

    connect(env->ds.examData, &DS_ExamData::signalDataChanged_ScannerInterlocks, this, [=](const DS_ExamDef::ScannerInterlocks &val) {
        QMutexLocker locker(&mutexSingleton);
        QVariantMap valImr = ImrParser::ToImr_ScannerInterlocks(val);
        Digest digest = getLastDigest();
        digest.setScannerInterlocks(valImr);
        addNewDigest(digest);
    });

    connect(env->ds.systemData, &DS_SystemData::signalDataChanged_StatePath, this, [=](DS_SystemDef::StatePath val) {
        QMutexLocker locker(&mutexSingleton);
        QString valImr = ImrParser::ToImr_StatePath(val);
        Digest digest = getLastDigest();
        digest.setStatePath(valImr);
        addNewDigest(digest);
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_HeatMaintainerStatus, this, [=](const DS_McuDef::HeatMaintainerStatus &val) {
        QMutexLocker locker(&mutexSingleton);
        QVariantMap valImr = ImrParser::ToImr_HeatMaintainerStatus(val);
        Digest digest = getLastDigest();
        digest.setHeatMaintainer(valImr);
        addNewDigest(digest);
    });

    connect(env->ds.examData, &DS_ExamData::signalDataChanged_ExamProgressState, this, [=](DS_ExamDef::ExamProgressState val) {
        QMutexLocker locker(&mutexSingleton);
        QString valImr = ImrParser::ToImr_ExamProgressState(val);

        // Modify Internal ExamProgressState to Global ExamProgressState
        if ( (valImr == _L("Prepared")) ||
             (valImr == _L("Completed")) )
        {
            valImr = "Idle";
        }
        else if (valImr == _L("Started"))
        {
            valImr = "ProtocolModification";
        }

        Digest digest = getLastDigest();
        digest.setCurrentExamProgressState(valImr);
        addNewDigest(digest);
    });

    connect(env->ds.examData, &DS_ExamData::signalDataChanged_ExamGuid, this, [=](QString val) {
        QMutexLocker locker(&mutexSingleton);
        QString valImr = val;
        Digest digest = getLastDigest();
        digest.setCurrentExamGuid(valImr);
        addNewDigest(digest);
    });

    connect(env->ds.examData, &DS_ExamData::signalDataChanged_ExamStartedAtEpochMs, this, [=](qint64 val) {
        QMutexLocker locker(&mutexSingleton);
        qint64 valImr = val;
        Digest digest = getLastDigest();
        digest.setCurrentExamStartedAt(Util::qDateTimeToUtcDateTimeStr(QDateTime::fromMSecsSinceEpoch(valImr)));
        addNewDigest(digest);
    });

    connect(env->ds.deviceData, &DS_DeviceData::signalDataChanged_FluidSourceSyringes, this, [=](const DS_DeviceDef::FluidSources &val) {
        QMutexLocker locker(&mutexSingleton);
        QVariantMap valImr = ImrParser::ToImr_FluidSourceSyringes(val);
        Digest digest = getLastDigest();
        digest.setFluidSourcesFromPartial(valImr);
        addNewDigest(digest);
    });

    connect(env->ds.deviceData, &DS_DeviceData::signalDataChanged_FluidSourceBottles, this, [=](const DS_DeviceDef::FluidSources &val) {
        QMutexLocker locker(&mutexSingleton);
        QVariantMap valImr = ImrParser::ToImr_FluidSourceBottles(val);
        Digest digest = getLastDigest();
        digest.setFluidSourcesFromPartial(valImr);
        addNewDigest(digest);
    });

    connect(env->ds.deviceData, &DS_DeviceData::signalDataChanged_FluidSourceMuds, this, [=](const DS_DeviceDef::FluidSource &val) {
        QMutexLocker locker(&mutexSingleton);
        QString err;
        QVariantMap valImr = ImrParser::ToImr_FluidSource(val, DS_DeviceDef::FLUID_SOURCE_IDX_MUDS, &err);
        Digest digest = getLastDigest();
        digest.setFluidSourcesFromPartial(valImr);
        addNewDigest(digest);
    });

    connect(env->ds.deviceData, &DS_DeviceData::signalDataChanged_FluidSourceSuds, this, [=](const DS_DeviceDef::FluidSource &val) {
        QMutexLocker locker(&mutexSingleton);
        QString err;
        QVariantMap valImr = ImrParser::ToImr_FluidSource(val, DS_DeviceDef::FLUID_SOURCE_IDX_SUDS, &err);
        Digest digest = getLastDigest();
        digest.setFluidSourcesFromPartial(valImr);
        addNewDigest(digest);
    });

    connect(env->ds.deviceData, &DS_DeviceData::signalDataChanged_FluidSourceWasteContainer, this, [=](const DS_DeviceDef::FluidSource &val) {
        QMutexLocker locker(&mutexSingleton);
        QString err;
        QVariantMap valImr = ImrParser::ToImr_FluidSource(val, DS_DeviceDef::FLUID_SOURCE_IDX_WASTE_CONTAINER, &err);

        Digest digest = getLastDigest();
        digest.setFluidSourcesFromPartial(valImr);
        addNewDigest(digest);
    });

    connect(env->ds.systemData, &DS_SystemData::signalDataChanged_StatePath, this, [=](DS_SystemDef::StatePath curStatePath, DS_SystemDef::StatePath prevStatePath) {
       if ( (prevStatePath == DS_SystemDef::STATE_PATH_IDLE) &&
            (curStatePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING) )
        {
            LOG_INFO("Service mode entered. Save last fluid source data..\n");

            QMutexLocker locker(&mutexSingleton);
            Digest digest = getLastDigest();
            LOG_INFO("Last digest = %s\n", digest.serialize().CSTR());
            restoreLastFluidSources(digest);
        }
    });

    connect(env->ds.systemData, &DS_SystemData::signalDataChanged_IwconfigParams, this, [=](const DS_SystemDef::IwconfigParams &val) {
        QMutexLocker locker(&mutexSingleton);
        QVariantMap valImr = ImrParser::ToImr_Iwconfig(val);
        Digest digest = getLastDigest();
        digest.setIwconfig(valImr);
        addNewDigest(digest);
    });
}

Digest DigestProvider::getLastDigest()
{
    Digest digest;
    digest.setId(-1);
    //pri QMutexLocker locker(&mutexSingleton);

    //mutexListDigest.lock();
    if (listDigest.length() > 0)
    {
        digest = listDigest.last();
    }
    else
    {
        digest.setId(0);
    }
    //mutexListDigest.unlock();

    return digest;
}

void DigestProvider::addNewDigest(Digest &newDigest)
{
    int newDigestId = newDigest.getId();

    if (newDigestId == -1)
    {
        LOG_ERROR("addNewDigest(): newDigestId=%d: Failed to add digest. Digest.Id=-1\n", newDigestId);
        return;
    }

    //pri QMutexLocker locker(&mutexSingleton);
    //mutexListDigest.lock();

    QDateTime curTime = QDateTime::currentDateTimeUtc();
    bool digestAddRequired = false;

    newDigest.setEventAt(Util::qDateTimeToUtcDateTimeStr(curTime));

    if (listDigest.length() == 0)
    {
        LOG_DEBUG("addNewDigest(): newDigestId=%d: Empty digest list. Digest should be added.\n", newDigestId);
        digestAddRequired = true;
    }
    else
    {
        const Digest &lastDigest = listDigest[listDigest.length() - 1];
        QDateTime lastDigestTime = Util::utcDateTimeStrToQDateTime(lastDigest.getEventAt());

        if (newDigest == listDigest.last())
        {
            // No change in digest
            goto bail;
        }
        else if (newDigest.getStatePath() != lastDigest.getStatePath())
        {
            LOG_DEBUG("addNewDigest(): newDigestId=%d: StatePath changed(%s->%s). Digest should be added.\n", newDigestId, lastDigest.getStatePath().CSTR(), newDigest.getStatePath().CSTR());
            digestAddRequired = true;
        }
        else if (newDigest.getCurrentExamProgressState() != lastDigest.getCurrentExamProgressState())
        {
            LOG_DEBUG("addNewDigest(): newDigestId=%d: ExamProgress changed(%s->%s). Digest should be added.\n", newDigestId, lastDigest.getCurrentExamProgressState().CSTR(), newDigest.getCurrentExamProgressState().CSTR());
            digestAddRequired = true;
        }
        else if (commitLocked)
        {
            LOG_DEBUG("addNewDigest(): newDigestId=%d: Digest commit locked. Digest should NOT be added.\n", newDigestId);
            digestAddRequired = false;
        }
        else if (curTime.currentMSecsSinceEpoch() - lastDigestTime.currentMSecsSinceEpoch() >= IMR_MIN_DIGEST_INTERVAL_MS)
        {
            LOG_DEBUG("addNewDigest(): newDigestId=%d: Last digest is old enough(>=%dms). Digest should be added.\n", newDigestId, IMR_MIN_DIGEST_INTERVAL_MS);
            digestAddRequired = true;
        }
    }

    // Assign digest ID
    tmrReleaseLastDigest.stop();
    if ( (digestAddRequired) || (listDigest.length() == 0) )
    {
        if ( (lastDigestIsReady) || (listDigest.length() == 0) )
        {
            incrementDigestId();
            newDigest.setId(digestId);
            //LOG_DEBUG("addNewDigest(): newDigestId=%d: DigestAddRequired: New Digest added. (digestId=%d)\n", newDigestId, digestId);
        }
        else
        {
            newDigest.setId(digestId);
            listDigest.pop_back();
            //LOG_DEBUG("addNewDigest(): newDigestId=%d: DigestAddRequired: Last Digest Not Ready: Last digest replaced. (digestId=%d)\n", newDigestId, digestId);
        }
        lastDigestIsReady = true;
    }
    else
    {
        // New Digest cannot be added
        if (lastDigestIsReady)
        {
            // Avoid 'ready' digest content change. Force to add new digest
            incrementDigestId();
            newDigest.setId(digestId);
            //LOG_INFO("addNewDigest(): newDigestId=%d: DigestAddNotRequired: New Digest added. (digestId=%d)\n", newDigestId, digestId);
        }
        else
        {
            newDigest.setId(digestId);
            listDigest.pop_back();
            //LOG_INFO("addNewDigest(): newDigestId=%d: DigestAddNotRequired: Last Digest Not Ready: Last digest replaced. (digestId=%d)\n", newDigestId, digestId);
        }

        // Digest Add Not Required. Set last digest NOT Ready.
        lastDigestIsReady = false;
        tmrReleaseLastDigest.start(IMR_MIN_DIGEST_INTERVAL_MS);
    }

    // Update listDigest with newDigest
    listDigest.append(newDigest);
    if (listDigest.length() > IMR_MAX_DIGESTS)
    {
        //LOG_DEBUG("Digest list reached limit(%d). Deleting oldest one\n", IMR_MAX_DIGESTS);
        listDigest.pop_front();
    }

    tmrDigestSave.stop();
    tmrDigestSave.start(IMR_MIN_DIGEST_INTERVAL_MS * 0.5);

bail:
    //mutexListDigest.unlock();
    return;
}

quint32 DigestProvider::getLastDigestId()
{
    QMutexLocker locker(&mutexSingleton);

    if (lastDigestIsReady)
    {
        return digestId;
    }
    else
    {
        return digestId - 1;
    }
}

void DigestProvider::incrementDigestId()
{
    //pri QMutexLocker locker(&mutexSingleton);
    digestId++;
    if (digestId == 0)
    {
        digestId = 1;
        digestIdToSave = 1;
        digestWrappedCount++;
    }
}

int DigestProvider::getDigestWrappedCount()
{
    QMutexLocker locker(&mutexSingleton);
    return digestWrappedCount;
}

QVariantList DigestProvider::getDigests(int fromId, int sizeLimit, bool includeNotReady, bool omitAlerts)
{
    if (fromId == 0)
    {
        fromId = 1;
    }

    QVariantList digests;
    int fromPt = -1;
    //mutexListDigest.lock();
    QMutexLocker locker(&mutexSingleton);

    if (listDigest.length() > 0)
    {
        int startId = listDigest.first().getId();
        int endId = listDigest.last().getId();
        if (startId > endId)
        {
            // Digest ID wrapped. Find Position manually
            for (int digestIdx = 0; digestIdx < listDigest.length(); digestIdx++)
            {
                if (listDigest[digestIdx].getId() == fromId)
                {
                    fromPt = digestIdx;
                    break;
                }
            }
        }
        else if (fromId >= startId)
        {
            fromPt = fromId - startId;
        }
        else
        {
            // No digest found
        }
    }

    if (fromPt != -1)
    {
        for (int digestIdx = fromPt; digestIdx < listDigest.length(); digestIdx++)
        {
            if ( (!includeNotReady) &&
                 (digestIdx == listDigest.length() - 1) &&
                 (!lastDigestIsReady) )
            {
                // current digest is not ready. Don't output the current digest
            }
            else
            {
                digests.append(listDigest[digestIdx].getParams());
            }

            if (digests.length() == sizeLimit)
            {
                break;
            }
        }
    }
    // Because QMap is in used, and QMap is not thread-safe.
    // Don't release the mutex here until everything has been jsonised.

    if (digests.length() > 0)
    {
        // Get digests with alerts

        // For all digests except last, each digest shall contain alerts that have happend to current digest createdAt time
        for (int digestIdx = 0; digestIdx < digests.length() - 1; digestIdx++)
        {
            QVariantMap digest = digests[digestIdx].toMap();
            QDateTime alertTimeOffset = Util::utcDateTimeStrToQDateTime(digest["EventAt"].toString());
            digest[_L("Alerts")] = omitAlerts ? QVariantList() : env->ds.alertAction->getAlertsFromTimeOffset(alertTimeOffset);
            digests[digestIdx] = digest;
        }

        // Set last digest
        QVariantList allAlerts = env->ds.alertData->getAllAlerts();
        QVariantMap lastDigest = digests.last().toMap();
        lastDigest[_L("Alerts")] = omitAlerts ? QVariantList() : allAlerts;
        digests[digests.length() - 1] = lastDigest;
    }

    return digests;
}

void DigestProvider::getDigestDeltaMap(QVariantMap &deltaMap, const QVariantMap &map1, const QVariantMap &map2)
{
    QMap<QString, QVariant>::const_iterator fieldIter = map2.begin();

    while (fieldIter != map2.end())
    {
        QString fieldKey = fieldIter.key();
        if (map1.contains(fieldKey))
        {
            if (map1[fieldKey] != map2[fieldKey])
            {
                int dataType1 = map1[fieldKey].userType();
                int dataType2 = map2[fieldKey].userType();

                if ( (dataType1 == QMetaType::QVariantMap) &&
                     (dataType2 == QMetaType::QVariantMap) )
                {
                    // Map date type. Perform delta for child maps
                    QVariantMap mapBuf;
                    getDigestDeltaMap(mapBuf, map1[fieldKey].toMap(), map2[fieldKey].toMap());
                    deltaMap.insert(fieldKey, mapBuf);
                }
                else
                {
                    // Data type is different. Overwirte with new one
                    //LOG_DEBUG("getDigestDeltaMap(): map1[%s] != map2[%s]: \n  {%s} / {%s},\n  {%s} / {%s}\n", fieldKey.CSTR(), fieldKey.CSTR(), Util::qVarientToJsonData(map1[fieldKey]).CSTR(),  map1[fieldKey].toString().CSTR(), Util::qVarientToJsonData(map2[fieldKey]).CSTR(), map2[fieldKey].toString().CSTR());
                    deltaMap.insert(fieldKey, map2[fieldKey]);
                }
            }
        }
        else
        {
            deltaMap.insert(fieldKey, map2[fieldKey]);
        }

        fieldIter++;
    }
}

void DigestProvider::getDigestDeltaMap(QVariantMap &deltaMap, int digestId)
{
    QVariantList digests = getDigests(digestId, 2);

    if (digests.length() != 2)
    {
        LOG_DEBUG("getDigestDeltaMap(): No digests (digestId=%d) found: len=%d\n", (int)digestId, digests.length());
        return;
    }

    QVariantMap digestMap1 = digests[0].toMap();
    QVariantMap digestMap2 = digests[1].toMap();
    QByteArray newDigestStr = Util::qVarientToJsonData(digestMap2, true);
    newDigestStr = newDigestStr.replace(" ", "");
    newDigestStr = newDigestStr.toLower();
    uint16_t newDigestCrc = Util::crc16(newDigestStr);

    bool logHcuDigestEnabled = env->ds.capabilities->get_Logging_ImrServerHcuDigest();
    if (logHcuDigestEnabled)
    {
        LOG_DEBUG("getDigestDeltaMap(): DigestId=%d, DigestCrc16=%d\nDigest=%s\n\n", digestId, newDigestCrc, newDigestStr.CSTR());
    }
    deltaMap.insert("crc16", newDigestCrc);
    getDigestDeltaMap(deltaMap, digestMap1, digestMap2);
}

void DigestProvider::saveDigest(const QVariantMap &digestMap)
{
    DS_WorkflowDef::MudsSodStatus mudsSodStatus = env->ds.workflowData->getMudsSodStatus();

    if (!mudsSodStatus.identified)
    {
        // MUDS is not identified yet. Save digest later.
        return;
    }

    LOG_DEBUG("saveDigest(): Saving Digest (DigestID=%d)..\n", digestMap["DigestNumber"].toInt());
    //LOG_DEBUG("Saving Digest %s\n", newDigest.serialize().CSTR());

    //pri QMutexLocker locker(&mutexSingleton);
    QFile fileBuf(PATH_LAST_DIGEST);
    if (fileBuf.open(QFile::WriteOnly | QFile::Text))
    {
        fileBuf.write(Util::qVarientToJsonData(digestMap));
        fileBuf.close();
    }
}

void DigestProvider::loadLastDigest()
{
    bool lastDigestFound = false;

    //pri QMutexLocker locker(&mutexSingleton);
    QFile fileBuf(PATH_LAST_DIGEST);
    if (fileBuf.open(QFile::ReadOnly | QFile::Text))
    {
        QJsonParseError parseErr;
        QString jsonStr = fileBuf.readAll();
        fileBuf.close();

        QJsonDocument document = QJsonDocument::fromJson(jsonStr.toUtf8(), &parseErr);

        if (jsonStr == "")
        {
            LOG_WARNING("loadLastDigest(): Empty LastDigest\n");
        }
        else if (parseErr.error != QJsonParseError::NoError)
        {
            QString err = QString().asprintf("Failed to open last digest. ParseErr=%s\n", parseErr.errorString().CSTR());
            LOG_ERROR("loadLastDigest(): %s", err.CSTR());
            env->ds.alertAction->activate("HCUInternalSoftwareError", err);
        }
        else
        {
            QVariantMap digestMap = document.toVariant().toMap();
            Digest digest;
            digest.setParams(digestMap);
            digest.setStatePath("OffUnreachable");

            // Put last digest to first digest
            addNewDigest(digest);

            LOG_INFO("loadLastDigest(): LastDigest = %s\n", digest.serialize().CSTR());

            restoreLastFluidSources(digest);
            restoreLastExam(digest);
            lastDigestFound = true;
        }
    }

    if (!lastDigestFound)
    {
        // No Last Digest found
        Digest digest;
        digest.setAlertsUpdatedAt(Util::qDateTimeToUtcDateTimeStr(QDateTime::fromMSecsSinceEpoch(0)));
        addNewDigest(digest);
    }
}

void DigestProvider::restoreLastFluidSources(const Digest &digest)
{
    const QVariantMap fluidSources = digest.getFluidSources();
    QString err;
    DS_DeviceDef::FluidSources lastFluidSources = ImrParser::ToCpp_FluidSources(fluidSources, &err);
    if (err != "")
    {
        LOG_ERROR("Failed to restore last fluid sources. Parse error=%s\n", err.CSTR());
    }
    else
    {
        LOG_DEBUG("RestoreLastFluidSources: Last Fluid Sources:\n%s\n", Util::qVarientToJsonData(fluidSources, false).CSTR());
    }
    env->ds.deviceData->setLastFluidSources(lastFluidSources);
}

void DigestProvider::restoreLastExam(const Digest &digest)
{
    QString err;

    QString examGuid = digest.getCurrentExamGuid();

    qint64 examStartedAtEpochMs = Util::utcDateTimeStrToQDateTime(digest.getCurrentExamStartedAt()).toMSecsSinceEpoch();

    DS_ExamDef::ExamProgressState examProgressState = ImrParser::ToCpp_ExamProgressState(digest.getCurrentExamProgressState());
    DS_ExamDef::InjectionPlan injectionPlan = ImrParser::ToCpp_InjectionPlan(digest.getInjectionPlan(), &err);
    DS_ExamDef::ExecutedSteps executedSteps = env->ds.examData->getExecutedSteps();

    if (err != "")
    {
        LOG_ERROR("restoreLastExam(): Failed to restore last exam: Parse Error (InjectionPlan: %s)\n", err.CSTR());
        env->ds.examAction->actExamReset();
        return;
    }

    // Clear Preloaded State
    for (int stepIdx = executedSteps.length(); stepIdx < injectionPlan.steps.length(); stepIdx++)
    {
        injectionPlan.steps[stepIdx].isPreloaded = false;
    }

    if (examGuid == EMPTY_GUID)
    {
        LOG_INFO("restoreLastExam(): No exam was in progress before last shut down\n");
        // No exam was in progress before last shut down: can still restore plan and progress state
        env->ds.examData->setInjectionPlan(injectionPlan);
        env->ds.examData->setExamProgressState(examProgressState);
        return;
    }

    DS_ExamDef::ExecutedSteps executedStep = ImrParser::ToCpp_ExecutedSteps(digest.getStepProgress(), &err);
    if (err != "")
    {
        LOG_ERROR("restoreLastExam(): Failed to restore last exam: Parse Error (ExecutedSteps: %s)\n", err.CSTR());
        env->ds.examAction->actExamReset();
        return;
    }

    env->ds.examAction->actExamRestore(examGuid, examStartedAtEpochMs, examProgressState, injectionPlan, executedStep);
}

