#include "Apps/AppManager.h"
#include "DeviceSyringeFill.h"
#include "Common/ImrParser.h"
#include "DataServices/Workflow/DS_WorkflowAction.h"
#include "DataServices/Workflow/DS_WorkflowData.h"
#include "DataServices/Exam/DS_ExamData.h"
#include "DataServices/Device/DS_DeviceData.h"
#include "DataServices/Device/DS_DeviceAction.h"
#include "DataServices/Alert/DS_AlertAction.h"
#include "DataServices/Alert/DS_AlertData.h"
#include "DataServices/Mcu/DS_McuAction.h"
#include "DataServices/Mcu/DS_McuData.h"
#include "DataServices/Capabilities/DS_Capabilities.h"
#include "DataServices/CfgGlobal/DS_CfgGlobal.h"

DeviceSyringeFill::DeviceSyringeFill(QObject *parent, EnvGlobal *env_, EnvLocal *envLocal_, SyringeIdx location_) :
    ActionBaseExt(parent, env_),
    location(location_)
{
    envLocal = envLocal_;

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });

    state = STATE_READY;
}

DeviceSyringeFill::~DeviceSyringeFill()
{
}

void DeviceSyringeFill::slotAppInitialised()
{
    connect(env->ds.workflowData, &DS_WorkflowData::signalDataChanged_WorkflowState, this, [=](DS_WorkflowDef::WorkflowState workflowState, DS_WorkflowDef::WorkflowState workflowStatePrev) {
        if ( (workflowState != DS_WorkflowDef::STATE_INACTIVE) &&
             (workflowStatePrev == DS_WorkflowDef::STATE_INACTIVE) )
        {
            state = STATE_READY;
            processState();
        }
        else if ( (workflowState == DS_WorkflowDef::STATE_INACTIVE) &&
                  (workflowStatePrev != DS_WorkflowDef::STATE_INACTIVE) )
        {
            env->actionMgr->deleteActAll(actStatusBuf.guid);
            env->actionMgr->deleteActAll(guidSubAction);
            env->actionMgr->deleteActAll(guidMudsInsertWaiting);

            state = STATE_INACTIVE;
        }
    });
}

bool DeviceSyringeFill::isBusy()
{
    return state != STATE_READY;
}

DataServiceActionStatus DeviceSyringeFill::actFill(bool preAirCheckEnabled, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "Fill", QString().asprintf("%s;%s", ImrParser::ToImr_FluidSourceSyringeIdx(location).CSTR(), preAirCheckEnabled ? "AirCheckEnabled" : "AirCheckDisabled"));

    DS_DeviceDef::FluidSource fluidSourceMuds = env->ds.deviceData->getFluidSourceMuds();
    if (!fluidSourceMuds.isInstalled())
    {
        status.err = "MUDS not ready";
        status.state = DS_ACTION_STATE_INVALID_STATE;
        actionStarted(status);
        return status;
    }

    if (env->ds.alertAction->isActivated("MUDSUseLifeLimitEnforced"))
    {
        status.err = "T_ARMFAILED_MUDSUseLifeExceeded";
        status.state = DS_ACTION_STATE_INVALID_STATE;
        actionStarted(status);
        return status;
    }

    DS_DeviceDef::FluidSources fluidSourceBottles = env->ds.deviceData->getFluidSourceBottles();

    if (!fluidSourceBottles[location].isReady)
    {
        status.err = "Bottle Not Ready";
        status.state = DS_ACTION_STATE_INVALID_STATE;
        actionStarted(status);
        return status;
    }

    if (fluidSourceBottles[location].sourcePackages.length() == 0)
    {
        status.err = "Bottle Not Loaded";
        status.state = DS_ACTION_STATE_INVALID_STATE;
        actionStarted(status);
        return status;
    }

    if ( (preAirCheckEnabled) &&
         (fluidSourceBottles[location].needsReplaced) )
    {
        status.err = "Bottle is Empty";
        status.state = DS_ACTION_STATE_INVALID_STATE;
        actionStarted(status);
        return status;
    }

    if (state == STATE_WAITING_MUDS)
    {
        status.state = DS_ACTION_STATE_BUSY;
        status.err = "Waiting MUDS";
        actionStarted(status);
        return status;
    }

    if ( (state != STATE_READY) &&
         (state != STATE_FAILED) )
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "Fill Not Ready";
        actionStarted(status);
        return status;
    }

    if (!env->ds.deviceData->getIsBottleMandatoryFieldsFilled(location))
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "Mandatory Field Not Filled";
        actionStarted(status);
        return status;
    }

    DS_DeviceDef::FluidSource fluidSourceSyringe = env->ds.deviceData->getFluidSourceSyringes()[location];
    if (fluidSourceSyringe.currentVolumesTotal() >= SYRINGE_VOLUME_MAX)
    {
        // No more fill required
        LOG_INFO("FILL[%d]: Cannot fill anymore. Maximum limit(%.1f,%.1f) reached\n", location, fluidSourceSyringe.currentVolumesTotal(), SYRINGE_VOLUME_MAX);
        status.state = DS_ACTION_STATE_STARTED;
        actionStarted(status);
        actionCompleted(status);
        return status;
    }

    actStatusBuf = status;
    actStatusBuf.state = DS_ACTION_STATE_START_WAITING;
    actionStarted(actStatusBuf);
    setState(STATE_STARTED);
    return actStatusBuf;
}

void DeviceSyringeFill::processState()
{
    DS_DeviceDef::FluidSource fluidSourceSyringe = env->ds.deviceData->getFluidSourceSyringes()[location];
    DS_DeviceDef::FluidSource fluidSourceMuds = env->ds.deviceData->getFluidSourceMuds();
    DS_DeviceDef::FluidSource fluidSourceBottle = env->ds.deviceData->getFluidSourceBottles()[location];

    switch (state)
    {
    case STATE_INACTIVE:
        LOG_INFO("FILL[%d]: STATE_INACTIVE\n", location);
        break;
    case STATE_READY:
        LOG_INFO("FILL[%d]: STATE_READY\n", location);
        break;
    case STATE_STARTED:
        LOG_INFO("FILL[%d]: STATE_STARTED\n", location);
        setState(STATE_WAITING_MUDS);
        break;
    case STATE_WAITING_MUDS:
        LOG_INFO("FILL[%d]: STATE_WAITING_MUDS\n", location);
        {
            // MUDS Idle callback connection
            env->actionMgr->deleteActAll(guidMudsInsertWaiting);
            guidMudsInsertWaiting = Util::newGuid();
            QMetaObject::Connection conn = connect(env->ds.deviceData, &DS_DeviceData::signalDataChanged_FluidSourceMuds, this, [=](const DS_DeviceDef::FluidSource &fluidSourceMuds) {
                if (state != STATE_WAITING_MUDS)
                {
                    // unexpected callback
                    env->actionMgr->deleteActCompleted(guidMudsInsertWaiting);
                    return;
                }

                if (fluidSourceMuds.isBusy)
                {
                    // wait until MUDS is idle
                }
                else
                {
                    setState(STATE_WAITING_MUDS);
                }
            });
            env->actionMgr->createActCompleted(guidMudsInsertWaiting, conn, QString(__PRETTY_FUNCTION__) + ": STATE_WAITING_MUDS: FluidSourceMuds");


            // Check for starting condition
            if (fluidSourceMuds.isBusy)
            {
                LOG_DEBUG("FILL[%d]: STATE_WAITING_MUDS: MUDS is busy\n", location);
            }
            else
            {
                actStatusBuf.state = DS_ACTION_STATE_STARTED;
                actionStarted(actStatusBuf);
                setState(STATE_STOPCOCK_FILL_SETTING);
            }
        }
        break;
    case STATE_STOPCOCK_FILL_SETTING:
        LOG_INFO("FILL[%d]: STATE_STOPCOCK_FILL_SETTING\n", location);
        env->actionMgr->deleteActAll(guidMudsInsertWaiting);
        fluidSourceBottle.isBusy = true;
        fluidSourceBottle.needsReplaced = false;
        env->ds.deviceData->setFluidSourceBottle(location, fluidSourceBottle);

        fluidSourceSyringe.isBusy = true;
        env->ds.deviceData->setFluidSourceSyringe(location, fluidSourceSyringe);

        handleSubAction(STATE_STOPCOCK_FILL_PROGRESS, STATE_STOPCOCK_FILL_DONE, STATE_STOPCOCK_FILL_FAILED);
        env->ds.deviceAction->actStopcock(location, DS_McuDef::STOPCOCK_POS_FILL, STOPCOCK_ACTION_TRIALS_LIMIT, guidSubAction);
        break;
    case STATE_STOPCOCK_FILL_PROGRESS:
        LOG_INFO("FILL[%d]: STATE_STOPCOCK_FILL_PROGRESS\n", location);
        break;
    case STATE_STOPCOCK_FILL_FAILED:
        LOG_ERROR("FILL[%d]: STATE_STOPCOCK_FILL_FAILED\n", location);
        setState(STATE_FILL_FAILED);
        break;
    case STATE_STOPCOCK_FILL_DONE:
        LOG_INFO("FILL[%d]: STATE_STOPCOCK_FILL_DONE\n", location);
        setState(STATE_FILL_STARTED);
        break;
    case STATE_FILL_STARTED:
        LOG_INFO("FILL[%d]: STATE_FILL_STARTED\n", location);
        {
            // Add fluid source package
            if (fluidSourceBottle.sourcePackages.length() <= 0)
            {
                LOG_ERROR("FILL[%d]: Failed to get bottle source package information\n", location);
            }
            else
            {
                DS_DeviceDef::FluidPackage newSourcePackage = fluidSourceBottle.sourcePackages[0];

                if ( (fluidSourceSyringe.sourcePackages.length() > 0) &&
                     (fluidSourceSyringe.sourcePackages.first().loadedAtEpochMs == newSourcePackage.loadedAtEpochMs) )
                {
                    // If current Source Package is same as new one, replace with new one
                    LOG_INFO("FILL[%d]: Refilling\n", location);
                    fluidSourceSyringe.sourcePackages.removeFirst();
                }
                else
                {
                    // AlertData format is { <Location>: <FluidPackage> }
                    QVariantMap jsonMap;
                    jsonMap.insert(ImrParser::ToImr_FluidSourceBottleIdx(location), ImrParser::ToImr_FluidPackage(newSourcePackage));
                    QString alertData = Util::qVarientToJsonData(jsonMap);
                    env->ds.alertAction->activate("SRUBulkSupplyLoaded", alertData);
                    LOG_INFO("FILL[%d]: Filling %s(%.1f %s)\n", location, newSourcePackage.brand.CSTR(), newSourcePackage.concentration, newSourcePackage.concentrationUnits.CSTR());
                }

                fluidSourceSyringe.sourcePackages.push_front(newSourcePackage);
            }

            env->ds.deviceData->setFluidSourceSyringe(location, fluidSourceSyringe);

            // Determine Fill Type
            DS_McuDef::ActFillParams params;
            params.type = DS_McuDef::FILL_TYPE_SOD;
            params.flow = env->ds.capabilities->get_FluidControl_FillFlowRate();
            params.idx = location;
            params.vol = SYRINGE_VOLUME_FILL_ALL;

            LOG_INFO("FILL[%d]: Type=%s: Filling %.1fml with %.1fml/s\n", location, ImrParser::ToImr_FillType(params.type).CSTR(), params.vol, params.flow);

            env->ds.alertAction->deactivateFromSyringeIdx("FillFailed", location);
            env->ds.alertAction->activate("SRUFillingMUDS", ImrParser::ToImr_FluidSourceSyringeIdx(location));

            handleSubAction(STATE_FILL_PROGRESS, STATE_FILL_DONE, STATE_FILL_FAILED);
            env->ds.mcuAction->actFill(params, guidSubAction);
        }
        break;
    case STATE_FILL_PROGRESS:
        LOG_INFO("FILL[%d]: STATE_FILL_PROGRESS\n", location);
        break;
    case STATE_FILL_DONE:
        LOG_INFO("FILL[%d]: STATE_FILL_DONE\n", location);
        env->ds.alertAction->deactivate("SRUFillingMUDS", ImrParser::ToImr_FluidSourceSyringeIdx(location));
        setState(STATE_STOPCOCK_CLOSE_SETTING);
        break;
    case STATE_FILL_FAILED:
        LOG_ERROR("FILL[%d]: STATE_FILL_FAILED: Status=%s\n", location, ImrParser::ToImr_DataServiceActionStatusStr(actStatusBuf).CSTR());
        env->ds.alertAction->deactivate("SRUFillingMUDS", ImrParser::ToImr_FluidSourceSyringeIdx(location));

        if (actStatusBuf.reply.contains("T_FILLFAILED_"))
        {
            LOG_WARNING("FILL[%d]: Fill request is rejected. Reason=%s\n", location, actStatusBuf.err.CSTR());
            actStatusBuf.err = actStatusBuf.reply;
            actStatusBuf.err.replace("T_FILLFAILED_", "");
            actStatusBuf.state = DS_ACTION_STATE_INVALID_STATE;
            QString alertData = QString().asprintf("%s;%s", ImrParser::ToImr_FluidSourceSyringeIdx(location).CSTR(), actStatusBuf.err.CSTR());
            env->ds.alertAction->activate("FillFailed", alertData);
        }
        else if (actStatusBuf.err.contains("AIR_DETECTED"))
        {
            LOG_WARNING("FILL[%d]: Air is detected\n", location);
            DS_DeviceDef::FluidSource fluidSourceBottle = env->ds.deviceData->getFluidSourceBottles()[location];
            fluidSourceBottle.needsReplaced = true;
            env->ds.deviceData->setFluidSourceBottle(location, fluidSourceBottle);
        }
        setState(STATE_STOPCOCK_CLOSE_SETTING);
        break;			
    case STATE_STOPCOCK_CLOSE_SETTING:
        LOG_INFO("FILL[%d]: STATE_STOPCOCK_CLOSE_SETTING\n", location);
        handleSubAction(STATE_STOPCOCK_CLOSE_PROGRESS, STATE_STOPCOCK_CLOSE_DONE, STATE_STOPCOCK_CLOSE_FAILED);
        env->ds.deviceAction->actStopcock(location, DS_McuDef::STOPCOCK_POS_CLOSED, STOPCOCK_ACTION_TRIALS_LIMIT, guidSubAction);
        break;
    case STATE_STOPCOCK_CLOSE_PROGRESS:
        LOG_INFO("FILL[%d]: STATE_STOPCOCK_CLOSE_PROGRESS\n", location);
        break;
    case STATE_STOPCOCK_CLOSE_DONE:
        LOG_INFO("FILL[%d]: STATE_STOPCOCK_CLOSE_DONE\n", location);
        setState(STATE_SYRINGE_AIR_CHECK_STARTED);
        break;
    case STATE_STOPCOCK_CLOSE_FAILED:
        LOG_ERROR("FILL[%d]: STATE_STOPCOCK_CLOSE_FAILED\n", location);
        setState(STATE_FAILED);
        break;
    case STATE_SYRINGE_AIR_CHECK_STARTED:
        LOG_INFO("FILL[%d]: STATE_SYRINGE_AIR_CHECK_STARTED\n", location);
        {
            // Filling fluid is done. Set bottle state to NOT busy.
            fluidSourceBottle.isBusy = false;
            env->ds.deviceData->setFluidSourceBottle(location, fluidSourceBottle);

            DS_WorkflowDef::MudsSodStatus mudsSodStatus = env->ds.workflowData->getMudsSodStatus();
            if (!mudsSodStatus.syringeSodStatusAll[location].isCompleted())
            {
                LOG_WARNING("FILL[%d]: STATE_SYRINGE_AIR_CHECK_STARTED: SOD is not completed. Air check skipped..\n", location);
                setState(STATE_DONE);
            }
            else if (actStatusBuf.err.contains("INVALID_STATE"))
            {
                LOG_WARNING("FILL[%d]: STATE_SYRINGE_AIR_CHECK_STARTED: Fill didn't start due to Invalid State. Air check skipped..\n", location);
                setState(STATE_DONE);
                return;
            }
            else
            {
                handleSubAction(STATE_SYRINGE_AIR_CHECK_PROGRESS, STATE_SYRINGE_AIR_CHECK_DONE, STATE_SYRINGE_AIR_CHECK_FAILED);
                env->ds.deviceAction->actSyringeAirCheck(location, guidSubAction);
            }
        }
        break;
    case STATE_SYRINGE_AIR_CHECK_PROGRESS:
        LOG_INFO("FILL[%d]: STATE_SYRINGE_AIR_CHECK_PROGRESS\n", location);
        break;
    case STATE_SYRINGE_AIR_CHECK_FAILED:
        LOG_ERROR("FILL[%d]: STATE_SYRINGE_AIR_CHECK_FAILED\n", location);
        LOG_WARNING("FILL[%d]: Fill is completed but air detected in one of the syringes. Syringe Air Recovery is expected to be started soon..\n", location);
        setState(STATE_DONE);
        break;
    case STATE_SYRINGE_AIR_CHECK_DONE:
        LOG_INFO("FILL[%d]: STATE_SYRINGE_AIR_CHECK_DONE\n", location);
        setState(STATE_DONE);
        break;
    case STATE_DONE:
        LOG_INFO("FILL[%d]: STATE_DONE\n", location);
		setState(STATE_READY);

        fluidSourceBottle.isBusy = false;
        env->ds.deviceData->setFluidSourceBottle(location, fluidSourceBottle);

        fluidSourceSyringe.isBusy = false;
        env->ds.deviceData->setFluidSourceSyringe(location, fluidSourceSyringe);

        if (actStatusBuf.err == "")
		{
			actStatusBuf.state = DS_ACTION_STATE_COMPLETED;
        }

        // Fill stopped/aborted
        if (actStatusBuf.state == DS_ACTION_STATE_ALLSTOP_BTN_ABORT)
        {
            env->ds.alertAction->activate("InterruptedAllStopPressed", QString().asprintf("Fill;%s", ImrParser::ToImr_FluidSourceSyringeIdx(location).CSTR()));
        }
        actionCompleted(actStatusBuf);
        break;
    case STATE_FAILED:
        LOG_ERROR("FILL[%d]: STATE_FAILED\n", location);
        setState(STATE_DONE);
        break;
    default:
        LOG_ERROR("FILL[%d]: Unknown State(%d)\n", location, state);
        break;
    }
}
