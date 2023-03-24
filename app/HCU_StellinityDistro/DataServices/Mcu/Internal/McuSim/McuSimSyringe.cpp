#include <QDateTime>
#include "McuSimSyringe.h"
#include "DataServices/Mcu/DS_McuData.h"

McuSimSyringe::McuSimSyringe(QObject *parent, EnvGlobal *env_, EnvLocal *envLocal_, SyringeIdx location_) :
    QObject(parent),
    env(env_),
    envLocal(envLocal_),
    location(location_)
{
    curInfo.engaged = false;
    curInfo.flow = 0;
    curInfo.volume = MCU_SIM_FRESH_MUDS_VOLUME;
    curInfo.state = SYR_STATE_IDLE;
    curInfo.actionDoneState = "COMPLETED";
    curInfo.curAction = "NONE";
    paused = false;
    injectedVol = 0;
    targetInjectVol = 0;

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_SimDigest, this, [=](const DS_McuDef::SimDigest &simData, const DS_McuDef::SimDigest &prevSimDigest) {
        if ( (simData.sudsInserted != prevSimDigest.sudsInserted) ||
             (simData.mudsPresent != prevSimDigest.mudsPresent) ||
             (simData.mudsLatched != prevSimDigest.mudsLatched) )
        {
            slotUpdateStatus();
        }
    });
}

McuSimSyringe::~McuSimSyringe()
{
}

bool McuSimSyringe::adjustFlow(double flow)
{
    curInfo.flow = flow;
    return true;
}

double McuSimSyringe::getInjectedVol()
{
    return injectedVol;
}

double McuSimSyringe::calculateTargetVolume(double vol, bool isFill)
{
    double target;

    if (isFill)
    {
        target = curInfo.volume + vol;
        target = qMin(target, SYRINGE_VOLUME_MAX);
    }
    else
    {
        target = curInfo.volume - vol;
        target = qMax(target, 0.0);
    }

    return target;
}

QString McuSimSyringe::startAction(QString action, QString errCodePrefix, double vol, double flow, bool airCheck)
{
    QString err = "OK";

    if (action == _L("NONE"))
    {
        return err;
    }
    else if (action == _L("STOP"))
    {
        if (curInfo.state != SYR_STATE_IDLE)
        {
            curInfo.state = SYR_STATE_ABORTING;
            curInfo.actionDoneState = "USER_ABORT";
        }
        else
        {
            curInfo.actionDoneState = "COMPLETED";
        }
        return err;
    }
    else if (action == _L("PRIME"))
    {
        if (curInfo.state == SYR_STATE_IDLE)
        {
            curInfo.curAction = action;
            curInfo.flow = flow;
            curInfo.airCheck = airCheck;
            if (vol == SYRINGE_VOLUME_PRIME_ALL)
            {
                vol = SYRINGE_VOLUME_MAX;
            }
            targetInfo.volume = calculateTargetVolume(vol, false);
            curInfo.state = SYR_STATE_INJECTING;
        }
        else
        {
            err = errCodePrefix + "InvalidState";
        }
    }
    else if (action == _L("ENGAGE"))
    {
        if (curInfo.state == SYR_STATE_IDLE)
        {
            curInfo.curAction = action;
            curInfo.state = SYR_STATE_ENGAGING;
        }
        else
        {
            err = errCodePrefix + "InvalidState";
        }
    }
    else if (action == _L("DISENGAGE"))
    {
        curInfo.curAction = action;
        curInfo.state = SYR_STATE_ENGAGING;
    }
    else if (action == _L("FILL"))
    {
        if (curInfo.state == SYR_STATE_IDLE)
        {
            curInfo.curAction = action;
            curInfo.flow = flow;
            targetInfo.volume = calculateTargetVolume(vol, true);
            curInfo.state = SYR_STATE_FILLING;
            LOG_INFO("MCU_SIM_SYR[%d]:SYRINE_ACTION_TYPE_FILL Requested\n", location);
        }
        else
        {
            err = errCodePrefix + "InvalidState";
        }
    }
    else if (action == _L("PURGE"))
    {
        if (curInfo.state == SYR_STATE_IDLE)
        {
            curInfo.curAction = action;
            curInfo.flow = flow;
            targetInfo.volume = 0;
            curInfo.state = SYR_STATE_INJECTING;
        }
        else
        {
            err = errCodePrefix + "InvalidState";
        }
    }
    else if (action == _L("INJECT"))
    {
        if (curInfo.state == SYR_STATE_IDLE)
        {
            //Reset injected vol
            injectedVol = 0;
            targetInjectVol = vol;

            curInfo.curAction = action;
            curInfo.flow = flow;
            targetInfo.volume = calculateTargetVolume(vol, false);
            curInfo.state = SYR_STATE_INJECTING;
            LOG_INFO("MCU_SIM_SYR[%d]: SYRINE_ACTION_TYPE_INJECT Requested: flow=%.1fml/s, vol=%.1fml\n", location, curInfo.flow, targetInfo.volume);
        }
        else
        {
            err = errCodePrefix + "InvalidState";
            LOG_ERROR("MCU_SIM_SYR[%d]: syringe is busy(state=%d), cannot set new inject parameters\n", location, curInfo.state);
        }
    }
    else if (action == _L("PISTON"))
    {
        if (curInfo.state == SYR_STATE_IDLE)
        {
            curInfo.curAction = action;
            curInfo.flow = ::abs(flow);
            targetInfo.volume = calculateTargetVolume(vol, flow < 0);
            curInfo.state = (flow >= 0) ? SYR_STATE_INJECTING : SYR_STATE_FILLING;
        }
        else
        {
            err = errCodePrefix + "InvalidState";
        }
    }
    else
    {
        LOG_ERROR("MCU_SIM_SYR[%d]: Unexpected action(%s)\n", location, action.CSTR());
    }

    curInfo.actionDoneState = "PROCESSING";
    slotUpdateStatus();
    return err;
}

bool McuSimSyringe::stopAction(QString actionDone)
{
    startAction("STOP", "T_SYRSTOP_FAILED");
    curInfo.actionDoneState = actionDone;
    return true;
}

void McuSimSyringe::pause()
{
    targetInfo.flow = curInfo.flow;
    curInfo.flow = 0;
    paused = true;
}

void McuSimSyringe::resume()
{
    curInfo.flow = targetInfo.flow;
    paused = false;
}

bool McuSimSyringe::resetSyringe()
{
    if (curInfo.state == SYR_STATE_IDLE)
    {
        curInfo.volume = MCU_SIM_FRESH_MUDS_VOLUME;
        targetInfo.volume = curInfo.volume;
        return true;
    }

    return false;
}

void McuSimSyringe::slotUpdateStatus()
{
    bool actionInProgress = false;
    DS_McuDef::SimDigest simData = env->ds.mcuData->getSimDigest();

    if (curInfo.state == SYR_STATE_ABORTING)
    {
        setSyringeActionDone(curInfo.actionDoneState, curInfo.curAction);
        return;
    }

    if (curInfo.curAction == _L("PISTON"))
    {
        calculateVolume(curInfo.state == SYR_STATE_FILLING);
        if (isVolumeAtTarget(curInfo.state == SYR_STATE_FILLING))
        {
            curInfo.volume = targetInfo.volume;
            setSyringeActionDone("COMPLETED", curInfo.curAction);
        }
        else
        {
            actionInProgress = true;
        }
    }
    else if (curInfo.curAction == _L("FILL"))
    {
        calculateVolume(true);
        if (isVolumeAtTarget(true))
        {
            curInfo.volume = targetInfo.volume;
            setSyringeActionDone("COMPLETED", curInfo.curAction);
        }
        else if (!simData.mudsLatched)
        {
            setSyringeActionDone("MUDS_REMOVED", curInfo.curAction);
        }
        else
        {
            actionInProgress = true;
        }
    }
    else if (curInfo.curAction == _L("INJECT"))
    {
        calculateVolume(false);
        injectedVol = targetInjectVol - (curInfo.volume - targetInfo.volume);

        if (isVolumeAtTarget(false))
        {
            curInfo.volume = targetInfo.volume;
            injectedVol = targetInjectVol - (curInfo.volume - targetInfo.volume);
            setSyringeActionDone("COMPLETED", curInfo.curAction);
        }
        else
        {
            actionInProgress = true;
        }
    }
    else if (curInfo.curAction == _L("PURGE"))
    {
        calculateVolume(false);
        if (isVolumeAtTarget(false))
        {
            curInfo.volume = targetInfo.volume;
            setSyringeActionDone("COMPLETED", curInfo.curAction);
        }
        else if (simData.sudsInserted)
        {
            setSyringeActionDone("SUDS_INSERTED", curInfo.curAction);
        }
        else if (!simData.mudsLatched)
        {
            setSyringeActionDone("MUDS_REMOVED", curInfo.curAction);
        }
        else
        {
            actionInProgress = true;
        }
    }
    else if (curInfo.curAction == _L("PRIME"))
    {
        calculateVolume(false);
        if (isVolumeAtTarget(false))
        {
            curInfo.volume = targetInfo.volume;
            setSyringeActionDone("COMPLETED", curInfo.curAction);
        }
        else if (curInfo.volume == 0)
        {
            setSyringeActionDone("INSUFFICIENT_FLUID", curInfo.curAction);
        }
        else if (!simData.sudsInserted)
        {
            setSyringeActionDone("SUDS_REMOVED", curInfo.curAction);
        }
        else if (!simData.mudsLatched)
        {
            setSyringeActionDone("MUDS_REMOVED", curInfo.curAction);
        }
        else
        {
            actionInProgress = true;
        }
    }
    else if (curInfo.curAction == _L("ENGAGE"))
    {
        curInfo.engaged = true;
        setSyringeActionDone("COMPLETED", curInfo.curAction);
    }
    else if (curInfo.curAction == _L("DISENGAGE"))
    {
        curInfo.engaged = false;
        setSyringeActionDone("COMPLETED", curInfo.curAction);
    }
    else if (curInfo.curAction == _L("STOP"))
    {
        setSyringeActionDone("USER_ABORT", curInfo.curAction);
    }
    else if (curInfo.curAction == _L("NONE"))
    {

    }
    else
    {
        LOG_ERROR("MCU_SIM_SYR[%d]: Invalid Action(%s)\n", location, curInfo.curAction.CSTR());
    }

    if (actionInProgress)
    {
        QTimer::singleShot(MCU_SIM_SYRINGE_STATE_UPDATE_INTERVAL_MS, this, SLOT(slotUpdateStatus()));
    }
}

bool McuSimSyringe::isVolumeAtTarget(bool isFill)
{
    if (isFill)
    {
        return (targetInfo.volume <= curInfo.volume);
    }

    return (targetInfo.volume >= curInfo.volume);
}

void McuSimSyringe::calculateVolume(bool isFill)
{
    if (paused)
    {
        return;
    }

    if (isFill)
    {
        curInfo.volume += (curInfo.flow / MCU_SIM_SYRINGE_VOLUME_UPDATE_INTERVAL_MS);
    }
    else
    {
        curInfo.volume -= (curInfo.flow / MCU_SIM_SYRINGE_VOLUME_UPDATE_INTERVAL_MS);
        if (Util::isFloatVarGreaterThan(curInfo.volume, MCU_SIM_FRESH_MUDS_VOLUME, 1))
        {
            // Sanity Check. If volume
            curInfo.volume = 0;
        }
    }
}

McuSimSyringe::Status McuSimSyringe::getInfo()
{
    return curInfo;
}

void McuSimSyringe::setVol(double vol)
{
    curInfo.volume = vol;
}

QString McuSimSyringe::setSyringeActionDone(QString result, QString)
{
    targetInfo.volume = curInfo.volume;
    curInfo.flow = 0;
    curInfo.curAction = "NONE";
    curInfo.state = SYR_STATE_IDLE;
    curInfo.actionDoneState = result;

    emit signalSyringeActionDone(curInfo.actionDoneState);
    return result;
}


