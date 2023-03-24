#include "Apps/AppManager.h"
#include "DS_DeviceAction.h"
#include "Common/ImrParser.h"
#include "DataServices/Device/DS_DeviceData.h"
#include "DataServices/Exam/DS_ExamData.h"
#include "DataServices/System/DS_SystemData.h"
#include "DataServices/Workflow/DS_WorkflowData.h"
#include "DataServices/Alert/DS_AlertAction.h"

DS_DeviceAction::DS_DeviceAction(QObject *parent, EnvGlobal *env_) :
    ActionBase(parent, env_)
{
    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });

    envLocal = new EnvLocal("DS_Device-Action", "DEVICE_ACTION", LOG_MID_SIZE_BYTES);
    monitor = new DeviceMonitor(this, env);

    // Initialise sub device modules
    envDeviceBottle = new EnvLocal("DS_Device-Bottle", "DEVICE_BOTTLE", LOG_LRG_SIZE_BYTES * 2);
    envDeviceSyringe = new EnvLocal("DS_Device-Syringe", "DEVICE_SYRINGE", LOG_MID_SIZE_BYTES);
    envDeviceStopcock = new EnvLocal("DS_Device-Stopcock", "DEVICE_STOPCOCK", LOG_MID_SIZE_BYTES);

    for (int syringeIdx = 0; syringeIdx < ARRAY_LEN(devBottles); syringeIdx++)
    {
        devBottles[syringeIdx] = new DeviceBottle(this, env, envDeviceBottle, (SyringeIdx)syringeIdx);
    }

    for (int syringeIdx = 0; syringeIdx < ARRAY_LEN(devSyringes); syringeIdx++)
    {
        devSyringes[syringeIdx] = new DeviceSyringe(this, env, envDeviceSyringe, (SyringeIdx)syringeIdx);
    }

    for (int syringeIdx = 0; syringeIdx < ARRAY_LEN(devStopcocks); syringeIdx++)
    {
        devStopcocks[syringeIdx] = new DeviceStopcock(this, env, envDeviceStopcock, (SyringeIdx)syringeIdx);
    }

    devLed = new DeviceLeds(this, env);
    devMuds = new DeviceMuds(this, env);
    devSuds = new DeviceSuds(this, env);
    devBarcodeReader = new DeviceBarcodeReader(this, env);
    devOutletAirDoor = new DeviceOutletAirDoor(this, env);
    devWasteContainer = new DeviceWasteContainer(this, env);
    devHeatMaintainer = new DeviceHeatMaintainer(this, env);
    devDoor = new DeviceDoor(this, env);
    devBattery = new DeviceBattery(this, env);

    deviceActive = false;
}

DS_DeviceAction::~DS_DeviceAction()
{
    for (int i = 0; i < ARRAY_LEN(devBottles); i++)
    {
        delete devBottles[i];
    }

    for (int i = 0; i < ARRAY_LEN(devSyringes); i++)
    {
        delete devSyringes[i];
    }

    for (int i = 0; i < ARRAY_LEN(devStopcocks); i++)
    {
        delete devStopcocks[i];
    }

    delete devLed;
    delete devMuds;
    delete devSuds;
    delete devBarcodeReader;
    delete devOutletAirDoor;
    delete devWasteContainer;
    delete devHeatMaintainer;
    delete devDoor;
    delete devBattery;
    delete monitor;
    delete envLocal;
    delete envDeviceBottle;
    delete envDeviceSyringe;
    delete envDeviceStopcock;
}

void DS_DeviceAction::slotAppInitialised()
{
    connect(env->ds.systemData, &DS_SystemData::signalDataChanged_StatePath, this, [=](DS_SystemDef::StatePath curStatePath, DS_SystemDef::StatePath prevStatePath) {
        if ( ( (prevStatePath == DS_SystemDef::STATE_PATH_IDLE) && (curStatePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING) ) ||
             ( (prevStatePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING) && (curStatePath == DS_SystemDef::STATE_PATH_IDLE) ) )
        {
            QString logMsg = QString().asprintf("\n\n-----------------\nStatePath=%s\n\n", ImrParser::ToImr_StatePath(curStatePath).CSTR());
            LOG_INFO("%s", logMsg.CSTR());
            envDeviceBottle->log->writeInfo(logMsg);
            envDeviceSyringe->log->writeInfo(logMsg);
            envDeviceStopcock->log->writeInfo(logMsg);
        }
    });
}

DataServiceActionStatus DS_DeviceAction::actMudsInit(QString actGuid)
{
    return devMuds->actMudsInit(actGuid);
}

DataServiceActionStatus DS_DeviceAction::actMudsSodStart(QString actGuid)
{
    return devMuds->actMudsSodStart(actGuid);
}

DataServiceActionStatus DS_DeviceAction::actMudsSodAbort(QString actGuid)
{
    return devMuds->actMudsSodAbort(actGuid);
}

DataServiceActionStatus DS_DeviceAction::actMudsFindPlungerAll(QString actGuid)
{
    return devMuds->actMudsFindPlungerAll(actGuid);
}

DataServiceActionStatus DS_DeviceAction::actMudsPurgeAirAll(QString actGuid)
{
    return devMuds->actMudsPurgeAirAll(actGuid);
}

DataServiceActionStatus DS_DeviceAction::actMudsPurgeFluid(QList<bool> purgeSyringe, QString actGuid)
{
    return devMuds->actMudsPurgeFluid(purgeSyringe, actGuid);
}

DataServiceActionStatus DS_DeviceAction::actMudsEngageAll(QString actGuid)
{
    return devMuds->actMudsEngageAll(actGuid);
}

DataServiceActionStatus DS_DeviceAction::actMudsDisengageAll(QString actGuid)
{
    // Force unload of bottles during disengage
    for (int i = 0; i < ARRAY_LEN(devBottles); i++)
    {
        devBottles[i]->actUnload();
    }
    return devMuds->actMudsDisengageAll(DISENGAGE_ACTION_TRIALS_LIMIT, actGuid);
}

DataServiceActionStatus DS_DeviceAction::actMudsPreloadPrime(const DS_McuDef::InjectionProtocol &injectionProtocol, QString actGuid)
{
    return devMuds->actMudsPreload(injectionProtocol, actGuid);
}

DataServiceActionStatus DS_DeviceAction::actBottleLoad(SyringeIdx bottleIdx, DS_DeviceDef::FluidPackage package, QString actGuid)
{
    return devBottles[bottleIdx]->actLoad(package, actGuid);
}

DataServiceActionStatus DS_DeviceAction::actBottleUnload(SyringeIdx bottleIdx, QString actGuid)
{
    return devBottles[bottleIdx]->actUnload(actGuid);
}

DataServiceActionStatus DS_DeviceAction::actBottlesUnload(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "BottlesUnload");

    devBottles[SYRINGE_IDX_SALINE]->actUnload();
    devBottles[SYRINGE_IDX_CONTRAST1]->actUnload();
    devBottles[SYRINGE_IDX_CONTRAST2]->actUnload();

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);
    status.state = DS_ACTION_STATE_COMPLETED;
    actionCompleted(status);

    return status;
}

DataServiceActionStatus DS_DeviceAction::actSyringeFill(SyringeIdx syringeIdx, bool preAirCheckEnabled, QString actGuid)
{
    return devSyringes[syringeIdx]->actFill(preAirCheckEnabled, actGuid);
}

DataServiceActionStatus DS_DeviceAction::actSyringePrime(SyringeIdx syringeIdx, DS_DeviceDef::SudsPrimeType sudsPrimeType, QString actGuid)
{
    return devSyringes[syringeIdx]->actPrime(sudsPrimeType, actGuid);
}

DataServiceActionStatus DS_DeviceAction::actSyringePrime(const DS_McuDef::ActPrimeParams &primeParams, QString actGuid)
{
    return devSyringes[primeParams.idx]->actPrime(primeParams, actGuid);
}

DataServiceActionStatus DS_DeviceAction::actSyringeSodStart(SyringeIdx syringeIdx, QString actGuid)
{
    return devSyringes[syringeIdx]->actSodStart(actGuid);
}

DataServiceActionStatus DS_DeviceAction::actSyringeSodAbort(SyringeIdx syringeIdx, QString actGuid)
{
    return devSyringes[syringeIdx]->actSodAbort(actGuid);
}

DataServiceActionStatus DS_DeviceAction::actSyringeAirCheck(SyringeIdx syringeIdx, QString actGuid)
{
    return devSyringes[syringeIdx]->actAirCheck(actGuid);
}

DataServiceActionStatus DS_DeviceAction::actSudsPrimeInit(SyringeIdx syringeIdx, QString actGuid)
{
    return devSuds->actPrimeInit(syringeIdx, actGuid);
}

DataServiceActionStatus DS_DeviceAction::actSudsPrimeUpdateState(SyringeIdx syringeIdx, QString actGuid)
{
    return devSuds->actPrimeUpdateState(syringeIdx, actGuid);
}

DataServiceActionStatus DS_DeviceAction::actSudsAutoPrime(QString actGuid)
{
    return devSyringes[SYRINGE_IDX_SALINE]->actPrime(DS_DeviceDef::SUDS_PRIME_TYPE_AUTO, actGuid);
}

DataServiceActionStatus DS_DeviceAction::actSudsManualPrime(QString actGuid)
{
    return devSyringes[SYRINGE_IDX_SALINE]->actPrime(DS_DeviceDef::SUDS_PRIME_TYPE_MANUAL, actGuid);
}

DataServiceActionStatus DS_DeviceAction::actSudsSetActiveContrastSyringe(SyringeIdx syringeIdx, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "actSudsSetActiveContrastSyringe");

    devSuds->actSetActiveContrastLocation(syringeIdx);

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);
    status.state = DS_ACTION_STATE_COMPLETED;
    actionCompleted(status);

    return status;
}

DataServiceActionStatus DS_DeviceAction::actSudsSetNeedsReplace(bool needed, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "actSudsSetNeedsReplace");

    devSuds->actSetNeedsReplace(needed);

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);
    status.state = DS_ACTION_STATE_COMPLETED;
    actionCompleted(status);

    return status;
}

DataServiceActionStatus DS_DeviceAction::actSudsSetNeedsPrime(bool needed, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "SudsSetNeedsPrime", QString().asprintf("%s", needed ? "Needed" : "notNeeded"));

    if (needed)
    {
        devSuds->actSetStateNotPrimed();
    }
    else
    {
        devSuds->actSetStatePrimed();
    }

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);
    status.state = DS_ACTION_STATE_COMPLETED;
    actionCompleted(status);

    return status;
}

DataServiceActionStatus DS_DeviceAction::actStopcock(DS_McuDef::StopcockPos pos1, DS_McuDef::StopcockPos pos2, DS_McuDef::StopcockPos pos3, int retryLimit, QString actGuid)
{
    DS_McuDef::ActStopcockParams params;
    params.posAll[SYRINGE_IDX_SALINE] = pos1;
    params.posAll[SYRINGE_IDX_CONTRAST1] = pos2;
    params.posAll[SYRINGE_IDX_CONTRAST2] = pos3;
    return actStopcock(params, retryLimit, actGuid);
}

DataServiceActionStatus DS_DeviceAction::actStopcock(SyringeIdx syringeIdx, DS_McuDef::StopcockPos pos, int retryLimit, QString actGuid)
{
    DS_McuDef::ActStopcockParams params;
    for (int scIdx = 0; scIdx < params.posAll.length(); scIdx++)
    {
        if (scIdx == syringeIdx)
        {
            params.posAll[scIdx] = pos;
        }
        else
        {
            params.posAll[scIdx] = DS_McuDef::STOPCOCK_POS_NONE;
        }
    }
    return actStopcock(params, retryLimit, actGuid);
}

DataServiceActionStatus DS_DeviceAction::actStopcock(DS_McuDef::ActStopcockParams params, int retryLimit, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "Stopcock", QString().asprintf("%s;%d", Util::qVarientToJsonData(ImrParser::ToImr_StopcockPosAll(params.posAll)).CSTR(), retryLimit));
    status.state = DS_ACTION_STATE_START_WAITING;

    if (actGuid == "")
    {
        actGuid = Util::newGuid();
    }

    StopcockActionStatus scActionStatus;
    stopcockActionStatusMap.insert(actGuid, scActionStatus);

    for (int locationIdx = 0; locationIdx < params.posAll.length(); locationIdx++)
    {
        if ( (params.posAll[locationIdx] == DS_McuDef::STOPCOCK_POS_FILL) ||
             (params.posAll[locationIdx] == DS_McuDef::STOPCOCK_POS_INJECT) ||
             (params.posAll[locationIdx] == DS_McuDef::STOPCOCK_POS_CLOSED) )
        {
            stopcockActionStatusMap[actGuid].started[locationIdx] = false;
            stopcockActionStatusMap[actGuid].completed[locationIdx] = false;
            stopcockActionStatusMap[actGuid].actGuid[locationIdx] = Util::newGuid();

            env->actionMgr->onActionStarted(stopcockActionStatusMap[actGuid].actGuid[locationIdx], __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
                if (!stopcockActionStatusMap.contains(actGuid))
                {
                    // Action start failed by other stopcock
                    LOG_WARNING("ACT_STOPCOCK: Action failed by other stopcock\n");
                    return;
                }

                if ( (curStatus.state != DS_ACTION_STATE_STARTED) &&
                     (curStatus.state != DS_ACTION_STATE_START_WAITING) )
                {
                    actionStarted(curStatus, &status);
                    stopcockActionStatusMap.remove(actGuid);
                    return;
                }

                stopcockActionStatusMap[actGuid].started[locationIdx] = true;

                // Check all actions started
                bool allStopcockActionsStarted = true;
                for (int scIdx = 0; scIdx < ARRAY_LEN(stopcockActionStatusMap[actGuid].started); scIdx++)
                {
                    if (!stopcockActionStatusMap[actGuid].started[scIdx])
                    {
                        allStopcockActionsStarted = false;
                        break;
                    }
                }

                if (allStopcockActionsStarted)
                {
                    // All actions stared, emit signal
                    actionStarted(curStatus, &status);
                }
            });

            env->actionMgr->onActionCompleted(stopcockActionStatusMap[actGuid].actGuid[locationIdx], __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
                if (!stopcockActionStatusMap.contains(actGuid))
                {
                    LOG_WARNING("ACT_STOPCOCK: Action failed by other stopcock\n");
                    return;
                }

                if (curStatus.state != DS_ACTION_STATE_COMPLETED)
                {
                    //LOG_WARNING("ACT_STOPCOCK: Failed to complete stopcock action, location=%d\n", locationIdx);
                    actionCompleted(curStatus, &status);
                    stopcockActionStatusMap.remove(actGuid);
                    return;
                }

                stopcockActionStatusMap[actGuid].completed[locationIdx] = true;

                bool allStopcockActionsCompleted = true;
                for (int scIdx = 0; scIdx < ARRAY_LEN(stopcockActionStatusMap[actGuid].completed); scIdx++)
                {
                    if (!stopcockActionStatusMap[actGuid].completed[scIdx])
                    {
                        allStopcockActionsCompleted = false;
                        break;
                    }
                }

                if (allStopcockActionsCompleted)
                {
                    // All actions completed, emit signal
                    //LOG_DEBUG("ACT_STOPCOCK: Success Complete stopcock action, actGuid=%s\n", actGuid.CSTR());
                    actionCompleted(curStatus, &status);
                    stopcockActionStatusMap.remove(actGuid);
                }
            });
        }
        else
        {
            stopcockActionStatusMap[actGuid].started[locationIdx] = true;
            stopcockActionStatusMap[actGuid].completed[locationIdx] = true;
        }
    }


    // Start stopcock actions
    bool allStopcockActionsCompleted = true;
    for (int locationIdx = 0; locationIdx < ARRAY_LEN(stopcockActionStatusMap[actGuid].completed); locationIdx++)
    {
        if (!stopcockActionStatusMap[actGuid].completed[locationIdx])
        {
            //LOG_DEBUG("ACT_STOPCOCK: Starting DevStopcocks[%d]: guid=%s, pos=%s\n", locationIdx, stopcockActionStatusMap[actGuid].actGuid[locationIdx].CSTR(), ImrParser::ToImr_StopcockPos(params.posAll[locationIdx]).CSTR());
            allStopcockActionsCompleted = false;
            devStopcocks[locationIdx]->actStopcock(params.posAll[locationIdx], retryLimit, stopcockActionStatusMap[actGuid].actGuid[locationIdx]);
        }
    }

    if (allStopcockActionsCompleted)
    {
        // All actions completed, emit signal
        status.state = DS_ACTION_STATE_STARTED;
        actionStarted(status);
        status.state = DS_ACTION_STATE_COMPLETED;
        actionCompleted(status);
        stopcockActionStatusMap.remove(actGuid);
    }

    return status;
}

DataServiceActionStatus DS_DeviceAction::actBarcodeReaderConnect(QString actGuid)
{
    return devBarcodeReader->actConnect(actGuid);
}

DataServiceActionStatus DS_DeviceAction::actBarcodeReaderDisconnect(QString actGuid)
{
    return devBarcodeReader->actDisconnect(actGuid);
}

DataServiceActionStatus DS_DeviceAction::actBarcodeReaderStart(int sleepTimeoutMs, QString actGuid)
{
    return devBarcodeReader->actStart(sleepTimeoutMs, actGuid);
}

DataServiceActionStatus DS_DeviceAction::actBarcodeReaderStop(QString actGuid)
{
    return devBarcodeReader->actStop(actGuid);
}

DataServiceActionStatus DS_DeviceAction::actBarcodeReaderSetBarcodeData(QString data, QString actGuid)
{
    return devBarcodeReader->actSetBarcodeData(data, actGuid);
}

DataServiceActionStatus DS_DeviceAction::actLeds(LedIndex index, DS_McuDef::ActLedParams param, QString actGuid)
{
    return devLed->actLeds(index, param, actGuid);
}

DataServiceActionStatus DS_DeviceAction::actCopyLeds(LedIndex from, LedIndex to, QString actGuid)
{
    return devLed->actCopyLeds(from, to, actGuid);
}

DataServiceActionStatus DS_DeviceAction::actDeactivateReservoirAlertsOnEmptied(SyringeIdx syringeIdx, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "DeactivateReservoirAlertsOnEmptied", ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdx).CSTR());

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);

    env->ds.alertAction->deactivateFromSyringeIdx("ReservoirAirCheckCalFailed", syringeIdx);
    env->ds.alertAction->deactivateFromSyringeIdx("ReservoirAirCheckFailed", syringeIdx);
    env->ds.alertAction->deactivateFromSyringeIdx("InsufficientVolumeForCalSlack", syringeIdx);
    env->ds.alertAction->deactivateFromSyringeIdx("InsufficientVolumeForMUDSPrime", syringeIdx);
    env->ds.alertAction->deactivateFromSyringeIdx("InsufficientVolumeForReservoirAirCheck", syringeIdx);
    env->ds.alertAction->deactivateFromSyringeIdx("InsufficientVolumeForReservoirAirRecovery", syringeIdx);
    env->ds.alertAction->deactivateFromSyringeIdx("InsufficientVolumeForSUDSPrime", syringeIdx);
    env->ds.alertAction->deactivateFromSyringeIdx("ReservoirAirDetected", syringeIdx);
    env->ds.alertAction->deactivateFromSyringeIdx("ReservoirAirRecoveryFailed", syringeIdx);
    env->ds.alertAction->deactivateFromSyringeIdx("FillFailed", syringeIdx);
    env->ds.alertAction->deactivateFromSyringeIdx("SRUClearingInletFluid", syringeIdx);

    status.state = DS_ACTION_STATE_COMPLETED;
    actionCompleted(status);

    return status;
}


