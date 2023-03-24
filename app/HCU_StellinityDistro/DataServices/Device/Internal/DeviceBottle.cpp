#include "Apps/AppManager.h"
#include "DeviceBottle.h"
#include "Common/ImrParser.h"
#include "Common/Util.h"
#include "DataServices/Workflow/DS_WorkflowData.h"
#include "DataServices/Device/DS_DeviceData.h"
#include "DataServices/Device/DS_DeviceAction.h"
#include "DataServices/Mcu/DS_McuData.h"
#include "DataServices/Alert/DS_AlertAction.h"
#include "DataServices/CfgGlobal/DS_CfgGlobal.h"
#include "DataServices/CfgLocal/DS_CfgLocal.h"

DeviceBottle::DeviceBottle(QObject *parent, EnvGlobal *env_, EnvLocal *envLocal_, SyringeIdx location_) :
    ActionBase(parent, env_),
    location(location_)
{
    envLocal = envLocal_;

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });

    connect(&tmrUseLifeUpdate, SIGNAL(timeout()), this, SLOT(slotUpdateUsedTimeStatus()));
    connect(&tmrDebounceSpikeStateRead, &QTimer::timeout, this, [=] {
        tmrDebounceSpikeStateRead.stop();
        DS_McuDef::BottleBubbleDetectorState curState = env->ds.mcuData->getBottleBubbleStates()[location];
        if (curState != lastBubbleDetectorState)
        {
            QString curStateStr = ImrParser::ToImr_BottleBubbleDetectorState(curState);
            QString prevStateStr = ImrParser::ToImr_BottleBubbleDetectorState(lastBubbleDetectorState);

            LOG_DEBUG("tmrDebounceSpikeStateRead(): Spike State Changed from %s to %s\n", prevStateStr.CSTR(), curStateStr.CSTR());

            QString alertData;
            QString locationStr = ImrParser::ToImr_FluidSourceBottleIdx(location);
            switch (curState)
            {
            case DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_MISSING:
                alertData = QString().asprintf("%s;%s", locationStr.CSTR(), prevStateStr.CSTR());
                env->ds.alertAction->activate("SRURemovedSpikeHolder", alertData);
                break;
            case DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_AIR:
            case DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_FLUID:
                alertData = QString().asprintf("%s;%s", locationStr.CSTR(), curStateStr.CSTR());
                env->ds.alertAction->activate("SRUInsertedSpikeHolder", alertData);
                break;
            default:
                break;
            }

            if (isActive)
            {
                handleBubbleDetectorStateChanged();
            }

        }
    });

    isActive = false;
}

DeviceBottle::~DeviceBottle()
{
}

void DeviceBottle::slotAppInitialised()
{
    connect(env->ds.workflowData, &DS_WorkflowData::signalDataChanged_WorkflowState, this, [=](DS_WorkflowDef::WorkflowState workflowState, DS_WorkflowDef::WorkflowState workflowStatePrev) {
        if ( (workflowState != DS_WorkflowDef::STATE_INACTIVE) &&
             (workflowStatePrev == DS_WorkflowDef::STATE_INACTIVE) )
        {
            isActive = true;
            handleMudsInsertedChanged();
            handleBubbleDetectorStateChanged();
            setInletAirDetectorLed();
            slotUpdateUsedTimeStatus();

        }
        else if ( (workflowState == DS_WorkflowDef::STATE_INACTIVE) &&
                  (workflowStatePrev != DS_WorkflowDef::STATE_INACTIVE) )
        {
            isActive = false;
        }
    });


    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_MudsInserted, this, [=](bool inserted){
        if (!isActive)
        {
            return;
        }

        if (!inserted)
        {
            actUnload();
        }
        handleMudsInsertedChanged();
    });

    connect(env->ds.mcuData , &DS_McuData::signalDataChanged_BottleBubbleStates, this, [=](const DS_McuDef::BottleBubbleDetectorStates &states, const DS_McuDef::BottleBubbleDetectorStates &prevStates) {
        DS_McuDef::BottleBubbleDetectorState curState = states[location];
        DS_McuDef::BottleBubbleDetectorState prevState = prevStates[location];
        QString curStateStr = ImrParser::ToImr_BottleBubbleDetectorState(curState);
        QString prevStateStr = ImrParser::ToImr_BottleBubbleDetectorState(prevState);

        if (tmrDebounceSpikeStateRead.isActive())
        {
            LOG_DEBUG("signalDataChanged_BottleBubbleStates(): Spike State Changed from %s to %s to %s while debouncing in progress\n",
                      ImrParser::ToImr_BottleBubbleDetectorState(lastBubbleDetectorState).CSTR(),
                      prevStateStr.CSTR(),
                      curStateStr.CSTR());
            return;
        }

        // Update alerts
        if (curState != prevState)
        {
            LOG_DEBUG("signalDataChanged_BottleBubbleStates(): Spike State Changed from %s to %s. Start Debouncing..\n",
                      prevStateStr.CSTR(),
                      curStateStr.CSTR());
            lastBubbleDetectorState = prevState;
            tmrDebounceSpikeStateRead.stop();
            tmrDebounceSpikeStateRead.start(SPIKE_STATE_MONITOR_DEBOUNCE_INTERVAL_MS);
        }
    });

    connect(env->ds.deviceData, &DS_DeviceData::signalDataChanged_FluidSourceBottles, this, [=] {
        if (!isActive)
        {
            return;
        }
        setInletAirDetectorLed();
        slotUpdateUsedTimeStatus();
    });
}

void DeviceBottle::handleMudsInsertedChanged()
{
    DS_DeviceDef::FluidSource fluidSourceBottle = env->ds.deviceData->getFluidSourceBottles()[location];
    bool isInstalled;

    if (!env->ds.mcuData->getMudsInserted())
    {
        isInstalled = false;
    }
    else
    {
        isInstalled = true;
    }

    if (isInstalled != fluidSourceBottle.isInstalled())
    {
        fluidSourceBottle.setIsInstalled(isInstalled);
    }

    setFluidSourceBottle(fluidSourceBottle);
}

void DeviceBottle::handleBubbleDetectorStateChanged()
{
    DS_McuDef::BottleBubbleDetectorState curState = env->ds.mcuData->getBottleBubbleStates()[location];
    DS_DeviceDef::FluidSource fluidSourceBottle = env->ds.deviceData->getFluidSourceBottles()[location];

    LOG_DEBUG("[%d]: BubbleState: %s\n", location, ImrParser::ToImr_BottleBubbleDetectorState(curState).CSTR());

    if (curState == DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_MISSING)
    {
        fluidSourceBottle.isReady = false;
    }
    else if (curState == DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_AIR)
    {
        fluidSourceBottle.isReady = true;
    }
    else if (curState == DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_FLUID)
    {
        fluidSourceBottle.isReady = true;
    }
    setFluidSourceBottle(fluidSourceBottle);
}

bool DeviceBottle::isReady()
{
    DS_DeviceDef::FluidSource fluidSourceBottle = env->ds.deviceData->getFluidSourceBottles()[location];
    return (fluidSourceBottle.isInstalled() && fluidSourceBottle.isReady);
}

DS_DeviceDef::FluidSource DeviceBottle::getFluidSourceSyringe()
{
    DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
    switch (location)
    {
    case SYRINGE_IDX_CONTRAST1:
        return fluidSourceSyringes[SYRINGE_IDX_CONTRAST1];
    case SYRINGE_IDX_CONTRAST2:
        return fluidSourceSyringes[SYRINGE_IDX_CONTRAST2];
    case SYRINGE_IDX_SALINE:
    default:
        return fluidSourceSyringes[SYRINGE_IDX_SALINE];
    }
}

void DeviceBottle::setFluidSourceBottle(const DS_DeviceDef::FluidSource &data)
{
    DS_DeviceDef::FluidSources fluidSourceBottles = env->ds.deviceData->getFluidSourceBottles();
    fluidSourceBottles[location] = data;
    env->ds.deviceData->setFluidSourceBottles(fluidSourceBottles);
}

DataServiceActionStatus DeviceBottle::actLoad(DS_DeviceDef::FluidPackage package, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "Load", QString().asprintf("%s;%s", ImrParser::ToImr_FluidSourceBottleIdx(location).CSTR(), Util::qVarientToJsonData(ImrParser::ToImr_FluidPackage(package)).CSTR()));

    DS_DeviceDef::FluidSource fluidSourceBottle = env->ds.deviceData->getFluidSourceBottles()[location];

    if (location == SYRINGE_IDX_SALINE)
    {
        // Update saline max use duration from cfg
        int cfgMaxUseDurationMs = env->ds.cfgGlobal->get_Configuration_UseLife_SalineMaximumUseDuration() * 60 * 60 * 1000;
        package.maximumUseDurationMs =(cfgMaxUseDurationMs > 0) ? cfgMaxUseDurationMs : -1;
    }

    package.loadedAtEpochMs = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();

    if (fluidSourceBottle.currentVolumes.length() > 0)
    {
        fluidSourceBottle.currentVolumes.clear();
    }

    fluidSourceBottle.currentVolumes.append(package.volume);
    fluidSourceBottle.sourcePackages.clear();
    fluidSourceBottle.sourcePackages.append(package);
    setFluidSourceBottle(fluidSourceBottle);

    LOG_INFO("[%d]: ----------------------------------\n", (int)location);
    LOG_INFO("[%d]: New Bottle Loaded\n", (int)location);
    LOG_INFO("[%d]: Brand(%s), Concentration(%.1f %s), Vol(%.1fml), LotBatch(%s)\n",
             (int)location,
             package.brand.CSTR(),
             package.concentration,
             package.concentrationUnits.CSTR(),
             package.volume,
             package.lotBatch.CSTR());
    LOG_INFO("[%d]: MaxiumUseDuration(%s), LoadedAt(%s), ExpirationDate(%s)\n",
             (int)location,
             QDateTime::fromMSecsSinceEpoch(package.maximumUseDurationMs).toString("dd-hh:mm:ss").CSTR(),
             QDateTime::fromMSecsSinceEpoch(package.loadedAtEpochMs).toString("yyyy-MM-dd hh:mm:ss").CSTR(),
             QDateTime::fromMSecsSinceEpoch(package.expirationDateEpochMs).toString("yyyy-MM-dd hh:mm:ss").CSTR());
    LOG_INFO("[%d]:\n", (int)location);

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);

    status.state = DS_ACTION_STATE_COMPLETED;
    actionCompleted(status);

    return status;
}

DataServiceActionStatus DeviceBottle::actUnload(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "Unload", QString().asprintf("%d", location));

    DS_DeviceDef::FluidSource fluidSourceBottle = env->ds.deviceData->getFluidSourceBottles()[location];
    fluidSourceBottle.sourcePackages.clear();
    fluidSourceBottle.isBusy = false;
    fluidSourceBottle.needsReplaced = false;
    fluidSourceBottle.currentVolumes.clear();
    fluidSourceBottle.currentVolumes.append(0);

    LOG_INFO("[%d]: ----------------------------------\n", (int)location);
    LOG_INFO("[%d]: Bottle Unloaded\n", (int)location);
    LOG_INFO("[%d]:\n", (int)location);

    setFluidSourceBottle(fluidSourceBottle);

    if (actGuid == "")
    {
        actGuid = Util::newGuid();
    }

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);

    status.state = DS_ACTION_STATE_COMPLETED;
    actionCompleted(status);

    return status;
}

void DeviceBottle::slotUpdateUsedTimeStatus()
{
    tmrUseLifeUpdate.stop();

    DS_DeviceDef::FluidSource fluidSourceBottle = env->ds.deviceData->getFluidSourceBottles()[location];

    if ( (!fluidSourceBottle.isInstalled()) ||
         (fluidSourceBottle.needsReplaced) ||
         (fluidSourceBottle.sourcePackages.length() == 0) ||
         (fluidSourceBottle.sourcePackages[0].maximumUseDurationMs == -1) )
    {
        env->ds.alertAction->deactivate("BulkUseLifeExceeded", ImrParser::ToImr_FluidSourceBottleIdx((SyringeIdx)location));
        return;
    }

    qint64 maxUseTime = fluidSourceBottle.sourcePackages[0].loadedAtEpochMs + fluidSourceBottle.sourcePackages[0].maximumUseDurationMs;
    if (QDateTime::currentMSecsSinceEpoch() > maxUseTime)
    {
        LOG_INFO("[%d]: Package Use Life Exceeded. MaxUseTime=%lldhours\n", location, maxUseTime / 1000 / 3600);
        env->ds.alertAction->activate("BulkUseLifeExceeded", ImrParser::ToImr_FluidSourceBottleIdx((SyringeIdx)location));
    }
    else
    {
        if (env->ds.alertAction->isActivated("BulkUseLifeExceeded", ImrParser::ToImr_FluidSourceBottleIdx((SyringeIdx)location)))
        {
            env->ds.alertAction->deactivate("BulkUseLifeExceeded", ImrParser::ToImr_FluidSourceBottleIdx((SyringeIdx)location));
        }
        tmrUseLifeUpdate.start(FLUID_SOURCE_USE_LIFE_CHECK_INTERVAL_MS);
    }
}

void DeviceBottle::setInletAirDetectorLed()
{
    static QString lastLedChangeReason = "";

    LedIndex ledIndex = LED_IDX_UNKNOWN;
    switch (location)
    {
    case SYRINGE_IDX_SALINE:
        ledIndex = LED_IDX_SALINE;
        break;
    case SYRINGE_IDX_CONTRAST1:
        ledIndex = LED_IDX_CONTRAST1;
        break;
    case SYRINGE_IDX_CONTRAST2:
        ledIndex = LED_IDX_CONTRAST2;
        break;
    default:
        break;
    }

    if (ledIndex == LED_IDX_UNKNOWN)
    {
        LOG_ERROR("[%d]: setInletAirDetectorLed(): Bad Led Index\n", location);
        return;
    }

    DS_DeviceDef::FluidSource fluidSrcBottle = env->ds.deviceData->getFluidSourceBottles()[location];
    DS_McuDef::ActLedParams params;
    params.isFlashing = false;

    SyringeIdx colorIdx;
    switch (location)
    {
    case SYRINGE_IDX_CONTRAST1:
        colorIdx = SYRINGE_IDX_CONTRAST1;
        break;
    case SYRINGE_IDX_CONTRAST2:
        colorIdx = env->ds.deviceData->isSameContrastsLoaded() ? SYRINGE_IDX_CONTRAST1 : SYRINGE_IDX_CONTRAST2;
        break;
    case SYRINGE_IDX_SALINE:
    default:
        colorIdx = SYRINGE_IDX_SALINE;
        break;
    }

    QString ledChangeReason;

    if (!fluidSrcBottle.isInstalled())
    {
        ledChangeReason = "MUDS is not present or not latched";
        params.setColorOff();
    }
    else if (!fluidSrcBottle.isReady)
    {
        ledChangeReason = "Spike is not inserted";
        params.setColorWhite();
        params.isFlashing = true;
    }
    else if (fluidSrcBottle.isBusy)
    {
        ledChangeReason = "Currently filling";
        params.setColorFluid(colorIdx);
        params.isFlashing = true;
    }
    else if (fluidSrcBottle.needsReplaced)
    {
        ledChangeReason = "Bottle needs replaced";
        params.setColorOrange();
    }
    else if (fluidSrcBottle.sourcePackages.length() == 0)
    {
        ledChangeReason = "Spike is inserted with no fluid loaded";
        params.setColorWhite();
    }
    else
    {
        ledChangeReason = "Fluid loaded, fluid supply not empty";
        params.setColorFluid(colorIdx);
    }

    if (ledChangeReason != lastLedChangeReason)
    {
        LOG_DEBUG("[%d]: setInletAirDetectorLed(): Reason=%s, Params[%s]=%s\n",
                  location,
                  ledChangeReason.CSTR(),
                  ImrParser::ToImr_LedIndex(ledIndex).CSTR(),
                  Util::qVarientToJsonData(ImrParser::ToImr_ActLedParams(params)).CSTR());
        lastLedChangeReason = ledChangeReason;
    }
    env->ds.deviceAction->actLeds(ledIndex, params);
}
