#include "Apps/AppManager.h"
#include "McuSim.h"
#include "McuSimSyringe.h"
#include "McuSimStopcock.h"
#include "DataServices/Mcu/DS_McuData.h"
#include "DataServices/Device/DS_DeviceData.h"
#include "DataServices/Alert/DS_AlertAction.h"
#include "DataServices/System/DS_SystemData.h"
#include "DataServices/Capabilities/DS_Capabilities.h"
#include "Common/Environment.h"
#include "Common/ImrParser.h"
#include <QRandomGenerator>

McuSim::McuSim(QObject *parent, EnvGlobal *env_) :
    QObject(parent),
    env(env_)
{
    envLocal = new EnvLocal("DS_Mcu-Sim", "MCU_SIM");
    isActive = false;

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });

    connect(env->appManager, &AppManager::signalAppStarted, this, [=]() {
        slotAppStarted();
    });

    //Init mcu simulator
    salineControlGroup.airDetected = false;
    salineControlGroup.stopCock = new McuSimStopcock(this, env);
    salineControlGroup.syringe = new McuSimSyringe(this, env, envLocal, SYRINGE_IDX_SALINE);

    contrast1ControlGroup.airDetected = false;
    contrast1ControlGroup.stopCock = new McuSimStopcock(this, env);
    contrast1ControlGroup.syringe = new McuSimSyringe(this, env, envLocal, SYRINGE_IDX_CONTRAST1);

    contrast2ControlGroup.airDetected = false;
    contrast2ControlGroup.stopCock = new McuSimStopcock(this, env);
    contrast2ControlGroup.syringe = new McuSimSyringe(this, env, envLocal, SYRINGE_IDX_CONTRAST2);

    mcuStateMachine = new McuSimStateMachine(this, env, envLocal, &salineControlGroup, &contrast1ControlGroup, &contrast2ControlGroup);

    connect(this, SIGNAL(signalRequestReceived(QString)), this, SLOT(slotRequestReceived(QString)));
    connect(&tmrReconnectMcu, SIGNAL(timeout()), SLOT(slotConnectStart()));

    updateDigest();
}

McuSim::~McuSim()
{
    delete salineControlGroup.stopCock;
    delete salineControlGroup.syringe;
    delete contrast1ControlGroup.stopCock;
    delete contrast1ControlGroup.syringe;
    delete contrast2ControlGroup.stopCock;
    delete contrast2ControlGroup.syringe;
    delete mcuStateMachine;
    delete envLocal;
}

void McuSim::slotAppInitialised()
{
    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_SimDigest, this, [=](const DS_McuDef::SimDigest &hwDigest, const DS_McuDef::SimDigest &prevHwDigest){
        updateHwStates(hwDigest, prevHwDigest);
    });
}

void McuSim::slotAppStarted()
{
    // Restore last MCU state
    DS_McuDef::SimDigest simDigest = env->ds.mcuData->getSimDigest();

    DS_DeviceDef::FluidSources lastFluidSources = env->ds.deviceData->getLastFluidSources();

    if ( (lastFluidSources[DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_1].disposableType == DS_DeviceDef::FLUID_SOURCE_TYPE_SY2_MUDS_RESERVOIR) &&
         (lastFluidSources[DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_2].disposableType == DS_DeviceDef::FLUID_SOURCE_TYPE_SY2_MUDS_RESERVOIR) &&
         (lastFluidSources[DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_3].disposableType == DS_DeviceDef::FLUID_SOURCE_TYPE_SY2_MUDS_RESERVOIR) )
    {
        salineControlGroup.syringe->setVol(lastFluidSources[DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_1].currentVolumesTotal());
        contrast1ControlGroup.syringe->setVol(lastFluidSources[DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_2].currentVolumesTotal());
        contrast2ControlGroup.syringe->setVol(lastFluidSources[DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_3].currentVolumesTotal());

        simDigest.vol[SYRINGE_IDX_SALINE] = lastFluidSources[DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_1].currentVolumesTotal();
        simDigest.vol[SYRINGE_IDX_CONTRAST1] = lastFluidSources[DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_2].currentVolumesTotal();
        simDigest.vol[SYRINGE_IDX_CONTRAST2] = lastFluidSources[DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_3].currentVolumesTotal();
        simDigest.plungerState[SYRINGE_IDX_SALINE] = lastFluidSources[DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_1].isInstalled() ? "ENGAGED" : "DISENGAGED";
        simDigest.plungerState[SYRINGE_IDX_CONTRAST1] = lastFluidSources[DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_2].isInstalled() ? "ENGAGED" : "DISENGAGED";
        simDigest.plungerState[SYRINGE_IDX_CONTRAST2] = lastFluidSources[DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_3].isInstalled() ? "ENGAGED" : "DISENGAGED";

        //simDigest.sudsInserted = lastFluidSources[DS_DeviceDef::FLUID_SOURCE_IDX_SUDS].isInstalled();
        simDigest.mudsPresent = lastFluidSources[DS_DeviceDef::FLUID_SOURCE_IDX_MUDS].isInstalled();
        simDigest.mudsLatched = lastFluidSources[DS_DeviceDef::FLUID_SOURCE_IDX_MUDS].isInstalled();

        simDigest.bottleAirDetectedState[SYRINGE_IDX_SALINE] = simDigest.mudsPresent ? DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_FLUID : DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_MISSING;
        simDigest.bottleAirDetectedState[SYRINGE_IDX_CONTRAST1] = simDigest.mudsPresent ? DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_FLUID : DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_MISSING;
        simDigest.bottleAirDetectedState[SYRINGE_IDX_CONTRAST2] = simDigest.mudsPresent ? DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_FLUID : DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_MISSING;
    }

    simDigest.outletDoorState = env->ds.alertAction->isActivated("OutletAirDoorLeftOpen") ? "OPEN" : "CLOSED";

    env->ds.mcuData->setSimDigest(simDigest);

    isActive = true;
    LOG_DEBUG("slotAppStarted()\n");
}

void McuSim::activate(bool activateHw)
{
    tmrReconnectMcu.stop();
    if (activateHw)
    {
        slotConnectStart();
    }
    else
    {
        LOG_INFO("Disconnected\n");
        emit signalDisconnected();
    }
}

void McuSim::slotConnectStart()
{
    LOG_DEBUG("slotConnectStart()\n");
    tmrReconnectMcu.stop();
    if (isActive)
    {
        emit signalConnected();
    }
    else
    {
        tmrReconnectMcu.start(100);
    }
}

void McuSim::slotRequestReceived(QString req)
{
    //LOG_DEBUG("slotRequestReceived()\n");
    QStringList temp1 = req.split("@");

    if (isActive)
    {
        if (temp1.length() >= 1)
        {
            QString action = temp1.at(0);
            action.replace(">", "");
            action.replace("\r", "");
            action.replace("\n", "");
            QStringList params;
            if (temp1.length() > 1)
            {
                if (temp1.at(1) != _L(""))
                {
                    QString temp2 = temp1.at(1);
                    temp2.replace("\r", "").replace("\n", "");
                    params = temp2.split(",");
                }
            }
            else
            {
                params << "temp";
            }
            QString msgbuf = req;
            msgbuf.replace("\r", "").replace("\n", "");

            handleActionRequest(action, params, msgbuf);
        }
    }
}

void McuSim::sendMsg(QString req)
{
    //LOG_DEBUG("sendMsg()\n");
    emit signalRequestReceived(req);
}

void McuSim::sendResponse(QString originalMsg, QString reply)
{
    //LOG_DEBUG("sendResponse()\n");
    emit signalRxMsg(QString().asprintf("%s#%s\r", originalMsg.CSTR(), reply.CSTR()));
}

void McuSim::handleActionRequest(QString action, QStringList params, QString msgIn)
{
    //LOG_DEBUG("handleActionRequest()\n");
    QTimer::singleShot(MCU_SIM_PROCESSING_TIME_MS, [=]{

        if (env->state == EnvGlobal::STATE_EXITING)
        {
            return;
        }

        QString errCodePrefix = "T_" + action + "FAILED_";
        QString err = errCodePrefix + "InvalidParameter";

        if (action == _L("INFO"))
        {
            sendResponse(msgIn, "Stl2CLI,0,0,0,0,UNKNOWN,MCU_S1,MM1,MM2,MM3");
        }
        else if (action == _L("VERSION"))
        {
            sendResponse(msgIn, QString().asprintf("MCU_SIM_%s,SC,%s", env->ds.systemData->getCompatibleMcuVersionPrefix().CSTR(), env->ds.systemData->getCompatibleMcuCommandVersion().CSTR()));
        }
        else if (action == _L("DIGEST"))
        {
            updateDigest();
            QString digestStr = ImrParser::ToImr_SimDigestStr(env->ds.mcuData->getSimDigest());
            sendResponse(msgIn, digestStr);
        }
        else if (action == _L("INJECTDIGEST"))
        {
            DS_McuDef::InjectDigest hwInjectDigest = env->ds.mcuData->getSimInjectDigest();
            QString hwInjectDigestStr = ImrParser::ToImr_InjectDigestStr(hwInjectDigest);
            sendResponse(msgIn, hwInjectDigestStr);
        }
        else if (action == _L("CLEARALARMS"))
        {
            DS_McuDef::SimDigest hwDigest = env->ds.mcuData->getSimDigest();
            hwDigest.alarmCode = 0x0;
            env->ds.mcuData->setSimDigest(hwDigest);
            sendResponse(msgIn, "OK");
        }
        else if (action == _L("ARM"))
        {
            if ((err = mcuStateMachine->setActiveProtocol(params, errCodePrefix)) != _L("OK"))
            {
                // set protocol failed
            }
            else if ((err = checkMcuArmCondition(errCodePrefix)) != _L("OK"))
            {
                // check arm condition failed
            }
            else if ((err = mcuStateMachine->setInjectState(action, errCodePrefix)) != _L("OK"))
            {
                // arm failed
            }
            else
            {
                DS_McuDef::InjectDigest hwInjectDigest = env->ds.mcuData->getSimInjectDigest();
                hwInjectDigest.adaptiveFlowState = DS_McuDef::ADAPTIVE_FLOW_STATE_OFF;
                env->ds.mcuData->setSimInjectDigest(hwInjectDigest);
            }
            sendResponse(msgIn, err);
        }
        else if (action == _L("DISARM"))
        {
            err = mcuStateMachine->setInjectState(action, errCodePrefix);
            sendResponse(msgIn, err);
        }
        else if (action == _L("INJECTSTART"))
        {
            if ((err = checkMcuInjectStartCondition(errCodePrefix)) != _L("OK"))
            {
                // check inject start condition
            }
            else
            {
                err = mcuStateMachine->injectStart(errCodePrefix);
            }
            sendResponse(msgIn, err);
        }
        else if (action == _L("INJECTSTOP"))
        {
            err = mcuStateMachine->injectStop(errCodePrefix);
            sendResponse(msgIn, err);
        }
        else if (action == _L("INJECTHOLD"))
        {
            err = mcuStateMachine->injectHold(errCodePrefix);
            sendResponse(msgIn, err);
        }
        else if (action == _L("INJECTJUMP"))
        {
            if (params.length() >= 1)
            {
                int jumpPhaseIdx = params.at(0).toInt();
                err = mcuStateMachine->injectJump(jumpPhaseIdx, errCodePrefix);
            }
            sendResponse(msgIn, err);
        }
        else if (action == _L("ADJFLOW"))
        {
            if (params.length() >= 1)
            {
                double flow = params.at(0).toDouble();
                err = mcuStateMachine->adjustFlow(flow, errCodePrefix);
            }

            sendResponse(msgIn, err);
        }
        else if (action == _L("STOPCOCK"))
        {
            if (params.length() >= 3)
            {
                if ((err = salineControlGroup.stopCock->setPosition(params.at(0), errCodePrefix)) != _L("OK"))
                {
                    // position1 failed
                }
                else if ((err = contrast1ControlGroup.stopCock->setPosition(params.at(1), errCodePrefix)) != _L("OK"))
                {
                    // position2 failed
                }
                else if ((err = contrast2ControlGroup.stopCock->setPosition(params.at(2), errCodePrefix)) != _L("OK"))
                {
                    // position3 failed
                }
            }
            sendResponse(msgIn, err);
        }
        else if (action == _L("LEDS"))
        {
            err = errCodePrefix + "InvalidColor";
            if (params.length() >= 11)
            {
                DS_McuDef::SimDigest hwDigest = env->ds.mcuData->getSimDigest();
                for (int i = 0; i < params.length(); i++)
                {
                    if (params.at(i) != _L("NOCHANGE"))
                    {
                        hwDigest.ledColor[i] = params.at(i);
                    }
                }
                env->ds.mcuData->setSimDigest(hwDigest);
                err = "OK";
            }
            sendResponse(msgIn, err);
        }
        else if (action == _L("ENGAGE"))
        {
            if (params.length() >= 1)
            {
                QString idxStr = params.at(0);
                if (idxStr == "all")
                {
                    err = "OK";
                    mcuStateMachine->setSyringeActState(0, "PROCESSING");
                    mcuStateMachine->setSyringeActState(1, "PROCESSING");
                    mcuStateMachine->setSyringeActState(2, "PROCESSING");

                    QTimer::singleShot(MCU_SIM_SYRINGE_ACTION_DELAY_MS, [=] {
                        DS_McuDef::SimDigest hwDigest = env->ds.mcuData->getSimDigest();
                        hwDigest.plungerState[SYRINGE_IDX_SALINE] = "ENGAGED";
                        hwDigest.plungerState[SYRINGE_IDX_CONTRAST1] = "ENGAGED";
                        hwDigest.plungerState[SYRINGE_IDX_CONTRAST2] = "ENGAGED";
                        env->ds.mcuData->setSimDigest(hwDigest);
                        mcuStateMachine->setSyringeActState(0, "COMPLETED");
                        mcuStateMachine->setSyringeActState(1, "COMPLETED");
                        mcuStateMachine->setSyringeActState(2, "COMPLETED");
                    });
                }
                else
                {
                    int syrIdx = params.at(0).toInt();
                    if ( (syrIdx >= SYRINGE_IDX_START) && (syrIdx < SYRINGE_IDX_MAX) )
                    {
                        err = "OK";
                        mcuStateMachine->setSyringeActState(syrIdx, "PROCESSING");

                        QTimer::singleShot(MCU_SIM_SYRINGE_ACTION_DELAY_MS, [=] {
                            DS_McuDef::SimDigest hwDigest = env->ds.mcuData->getSimDigest();
                            hwDigest.plungerState[syrIdx] = "ENGAGED";
                            env->ds.mcuData->setSimDigest(hwDigest);
                            mcuStateMachine->setSyringeActState(syrIdx, "COMPLETED");
                        });
                    }
                }
            }
            sendResponse(msgIn, err);
        }
        else if (action == _L("DISENGAGE"))
        {
            if (params.length() >= 1)
            {
                QString idxStr = params.at(0);
                if (idxStr == "all")
                {
                    err = "OK";
                    mcuStateMachine->setSyringeActState(0, "PROCESSING");
                    mcuStateMachine->setSyringeActState(1, "PROCESSING");
                    mcuStateMachine->setSyringeActState(2, "PROCESSING");
                    mcuStateMachine->resetMuds("");

                    QTimer::singleShot(MCU_SIM_SYRINGE_ACTION_DELAY_MS, [=] {
                        DS_McuDef::SimDigest hwDigest = env->ds.mcuData->getSimDigest();
                        hwDigest.plungerState[SYRINGE_IDX_SALINE] = "DISENGAGED";
                        hwDigest.plungerState[SYRINGE_IDX_CONTRAST1] = "DISENGAGED";
                        hwDigest.plungerState[SYRINGE_IDX_CONTRAST2] = "DISENGAGED";
                        env->ds.mcuData->setSimDigest(hwDigest);
                        mcuStateMachine->setSyringeActState(0, "COMPLETED");
                        mcuStateMachine->setSyringeActState(1, "COMPLETED");
                        mcuStateMachine->setSyringeActState(2, "COMPLETED");
                    });
                }
                else
                {
                    int syrIdx = idxStr.toInt();
                    if ( (syrIdx >= SYRINGE_IDX_START) && (syrIdx < SYRINGE_IDX_MAX) )
                    {
                        err = "OK";
                        mcuStateMachine->setSyringeActState(syrIdx, "PROCESSING");
                        mcuStateMachine->resetMuds("");

                        QTimer::singleShot(MCU_SIM_SYRINGE_ACTION_DELAY_MS, [=] {
                            DS_McuDef::SimDigest hwDigest = env->ds.mcuData->getSimDigest();
                            hwDigest.plungerState[syrIdx] = "DISENGAGED";
                            env->ds.mcuData->setSimDigest(hwDigest);
                            mcuStateMachine->setSyringeActState(syrIdx, "COMPLETED");
                        });
                    }
                }

            }
            sendResponse(msgIn, err);
        }
        else if (action == _L("FINDPLUNGER"))
        {
            if (params.length() >= 1)
            {
                QString idxStr = params.at(0);
                if (idxStr == "all")
                {
                    err = "OK";
                    mcuStateMachine->setSyringeActState(0, "PROCESSING");
                    mcuStateMachine->setSyringeActState(1, "PROCESSING");
                    mcuStateMachine->setSyringeActState(2, "PROCESSING");

                    QTimer::singleShot(MCU_SIM_SYRINGE_ACTION_DELAY_MS, [=] {
                        mcuStateMachine->setSyringeActState(SYRINGE_IDX_SALINE, "COMPLETED");
                        mcuStateMachine->setSyringeActState(SYRINGE_IDX_CONTRAST1, "COMPLETED");
                        mcuStateMachine->setSyringeActState(SYRINGE_IDX_CONTRAST2, "COMPLETED");
                    });
                }
                else
                {
                    int syrIdx = params.at(0).toInt();
                    if ( (syrIdx >= SYRINGE_IDX_START) && (syrIdx < SYRINGE_IDX_MAX) )
                    {
                        err = "OK";
                        mcuStateMachine->setSyringeActState(syrIdx, "PROCESSING");

                        QTimer::singleShot(MCU_SIM_SYRINGE_ACTION_DELAY_MS, [=] {
                            mcuStateMachine->setSyringeActState(syrIdx, "COMPLETED");
                        });
                    }
                }
            }
            sendResponse(msgIn, err);
        }
        else if (action == _L("FILL"))
        {
            if (params.length() >= 2)
            {
                int syrIdx = params.at(0).toInt();
                if ( (syrIdx >= SYRINGE_IDX_START) && (syrIdx < SYRINGE_IDX_MAX) )
                {
                    double fillVolume = params.at(1).toDouble();
                    DS_McuDef::SimDigest hwDigest = env->ds.mcuData->getSimDigest();
                    if (fillVolume == -1)
                    {
                        fillVolume = SYRINGE_VOLUME_MAX - hwDigest.vol[syrIdx];
                    }

                    double fillFlow = env->ds.capabilities->get_FluidControl_FillFlowRate() * 4;
                    err = isSyringeEngaged(syrIdx) ? mcuStateMachine->setSyringeAction(syrIdx, "FILL", errCodePrefix, fillVolume, fillFlow, 0) : "T_FILLFAILED_PlungerNotEngaged";
                }
            }

            sendResponse(msgIn, err);
        }
        else if (action == _L("PRIME"))
        {

            if (params.length() >= 4)
            {
                int syrIdx = params.at(0).toInt();
                double vol1 = params.at(1).toDouble();
                double vol2 = params.at(2).toDouble();
                double flow = params.at(3).toDouble();
                double airCheck = (vol2 > 0) ? 1 : 0;

                if ( (syrIdx >= SYRINGE_IDX_START) && (syrIdx < SYRINGE_IDX_MAX) )
                {
                    // Simulation: Faster prime
                    double simFlow = flow * 2;
                    err = isSyringeEngaged(syrIdx) ? mcuStateMachine->setSyringeAction(syrIdx, action, errCodePrefix, vol1, vol2, simFlow, airCheck) : "T_PRIMEFAILED_PlungerNotEngaged";
                }
            }
            sendResponse(msgIn, err);
        }
        else if (action == _L("PISTON"))
        {
            if (params.length() >= 1)
            {
                QString idxStr = params.at(0);
                double vol  = params.at(1).toDouble();
                double flow = params.at(2).toDouble();

                if (vol == SYRINGE_VOLUME_PRIME_ALL)
                {
                    vol = SYRINGE_VOLUME_MAX;
                }

                if (idxStr == "all")
                {
                    if ( (err = mcuStateMachine->setSyringeAction(SYRINGE_IDX_SALINE, action, errCodePrefix, vol, flow, 0)) != _L("OK") )
                    {
                        err = errCodePrefix + "SyringeNotReady1";
                    }

                    if ( (err == _L("OK")) &&
                         ((err = mcuStateMachine->setSyringeAction(SYRINGE_IDX_CONTRAST1, action, errCodePrefix, vol, flow, 0)) != _L("OK")) )
                    {
                        err = errCodePrefix + "SyringeNotReady2";
                    }

                    if ( (err == _L("OK")) &&
                         ((err = mcuStateMachine->setSyringeAction(SYRINGE_IDX_CONTRAST2, action, errCodePrefix, vol, flow, 0)) != _L("OK")) )
                    {
                        err = errCodePrefix + "SyringeNotReady3";
                    }
                }
                else if (idxStr == "contrasts")
                {
                    if ( (err = mcuStateMachine->setSyringeAction(SYRINGE_IDX_CONTRAST1, action, errCodePrefix, vol, flow, 0)) != _L("OK") )
                    {
                        err = errCodePrefix + "SyringeNotReady2";
                    }

                    if ( (err == _L("OK")) &&
                         ((err = mcuStateMachine->setSyringeAction(SYRINGE_IDX_CONTRAST2, action, errCodePrefix, vol, flow, 0)) != _L("OK")) )
                    {
                        err = errCodePrefix + "SyringeNotReady3";
                    }
                }
                else
                {
                    int syrIdx = params.at(0).toInt();
                    if ( (syrIdx >= SYRINGE_IDX_START) && (syrIdx < SYRINGE_IDX_MAX) )
                    {
                        err = mcuStateMachine->setSyringeAction(syrIdx, action, errCodePrefix, vol, flow, 0);
                    }
                }
            }
            sendResponse(msgIn, err);
        }
        else if (action == _L("PURGE"))
        {
            if (params.length() >= 1)
            {
                // Simulation: Faster purge
                double purgeFlow = SYRINGE_FLOW_PURGE_AIR * 8;

                QString idxStr = params.at(0);
                if (idxStr == "all")
                {
                    if (mcuStateMachine->setSyringeAction(SYRINGE_IDX_SALINE, action, errCodePrefix, SYRINGE_VOLUME_MAX, purgeFlow, 0) != _L("OK"))
                    {
                        err = errCodePrefix + "SyringeNotReady1";
                    }
                    else if (mcuStateMachine->setSyringeAction(SYRINGE_IDX_CONTRAST1, action, errCodePrefix, SYRINGE_VOLUME_MAX, purgeFlow, 0) != _L("OK"))
                    {
                        err = errCodePrefix + "SyringeNotReady2";
                    }
                    else if (mcuStateMachine->setSyringeAction(SYRINGE_IDX_CONTRAST2, action, errCodePrefix, SYRINGE_VOLUME_MAX, purgeFlow, 0) != _L("OK"))
                    {
                        err = errCodePrefix + "SyringeNotReady3";
                    }
                    else
                    {
                        err = "OK";
                    }
                }
                else
                {
                    int syrIdx = params.at(0).toInt();
                    if ( (syrIdx >= SYRINGE_IDX_START) && (syrIdx < SYRINGE_IDX_MAX) )
                    {
                        err = mcuStateMachine->setSyringeAction(syrIdx, action, errCodePrefix, SYRINGE_VOLUME_MAX, purgeFlow, 0);
                    }
                }
            }
            sendResponse(msgIn, err);
        }
        else if (action == _L("STOP"))
        {
            mcuStateMachine->setSyringeAction(SYRINGE_IDX_SALINE, action, errCodePrefix, 0, 0, 0);
            mcuStateMachine->setSyringeAction(SYRINGE_IDX_CONTRAST1, action, errCodePrefix, 0, 0, 0);
            mcuStateMachine->setSyringeAction(SYRINGE_IDX_CONTRAST2, action, errCodePrefix, 0, 0, 0);
            err = "OK";

            sendResponse(msgIn, err);
        }
        else if (action == _L("RESETMUDS"))
        {
            err = mcuStateMachine->resetMuds(errCodePrefix);
            sendResponse(msgIn, err);
        }
        else if (action == _L("POWER"))
        {
            if (params.length() >= 1)
            {
                if (params.at(0) == _L("REBOOT"))
                {
                    err = "OK";
                    DS_McuDef::SimDigest hwDigest = env->ds.mcuData->getSimDigest();
                    hwDigest.isShuttingDown = true;
                    env->ds.mcuData->setSimDigest(hwDigest);
                    QTimer::singleShot(1000, [=] {
                        LOG_INFO("'REBOOT' has been issued\n");
                    });
                }
                else if (params.at(0) == _L("OFF"))
                {
                    err = "OK";
                    DS_McuDef::SimDigest hwDigest = env->ds.mcuData->getSimDigest();
                    hwDigest.isShuttingDown = true;
                    env->ds.mcuData->setSimDigest(hwDigest);
                    QTimer::singleShot(1000, [=] {
                        LOG_INFO("'OFF' has been issued\n");
                    });
                }
            }
            sendResponse(msgIn, err);
        }
        else if (action == _L("CALMOTOR"))
        {
             QTimer::singleShot(15000, [=]{
                sendResponse(msgIn, "OK");
             });
        }
        else if (action == _L("CALMETER"))
        {
             QTimer::singleShot(2000, [=]{
                sendResponse(msgIn, "OK");
             });
        }
        else if (action == _L("CALBUB"))
        {
            QTimer::singleShot(7500, [=]{
               sendResponse(msgIn, "OK");
            });
        }
        else if (action == _L("CALPLUNGER"))
        {
             QTimer::singleShot(10000, [=]{
                sendResponse(msgIn, "OK");
             });
        }
        else if (action == _L("CALZEROPOS"))
        {
            sendResponse(msgIn, _L("OK"));
        }
        else if (action == _L("HEATER"))
        {
            if (params.length() >= 1)
            {
                double setTemp = params.at(0).toDouble();
                DS_McuDef::SimDigest hwDigest = env->ds.mcuData->getSimDigest();
                if (setTemp == 0.0)
                {
                    // heat maintainer turned off
                    hwDigest.heatMaintainerState = "DISABLED";
                }
                else
                {
                    hwDigest.heatMaintainerState = "ENABLED_ON";
                }
                env->ds.mcuData->setSimDigest(hwDigest);
            }
            sendResponse(msgIn, "OK");
        }
        else if (action == _L("HWDIGEST"))
        {
            sendResponse(msgIn, "0,0,4.8,15.5,OFF,5.0,12.0,28.5,1.0,1.2,1.4,0,1.0,1,1,1,0,0,0,0,0,0");
        }
        else if (action == _L("CALSUDS"))
        {
            QTimer::singleShot(10000, [=]{
                sendResponse(msgIn, "OK");
            });
        }
        else if (action == _L("DOORLOCK"))
        {
            sendResponse(msgIn, "OK");
        }
        else if (action == _L("GETPCALDATA"))
        {
            sendResponse(msgIn, "11,22,33,44");
        }        
        else if (action == _L("GETPCALCOEFF"))
        {
            // Note that these values were retrieved from a real injector in the attempt to make alert data "realistic"
            sendResponse(msgIn, "-208.915388,-69.690306,-7931.755200,-18.763528,-0.068280,-1280.600600,SET");
        }		  
        else if (action == _L("GETPLUNGERFRICTION"))
        {
            // Note that these values were retrieved from a real injector in the attempt to make alert data "realistic"
            sendResponse(msgIn, "-2787.879800,-2790,-2786,-2782.084400,-2783,-2781,-2745.555000,-2749,-2742,-2745.095000,-2746,-2744,-2803.460000,-2811,-2798,-2793.715000,-2794,-2793");
        }
        else if (action == _L("SETBASETYPE"))
        {
            QString baseType = params.at(0);
            if ( (baseType == "BATTERY") ||
                 (baseType == "NO_BATTERY") ||
                 (baseType == "OCS") )
            {
                sendResponse(msgIn, "OK");
            }
            else
            {
                sendResponse(msgIn, err);
            }
        }
        else if (action == _L("CALSLACK"))
        {
            if (params.length() >= 1)
            {
                int syrIdx = params.at(0).toInt();
                if ( (syrIdx >= SYRINGE_IDX_START) && (syrIdx < SYRINGE_IDX_MAX) )
                {
                    err = "OK";
                    mcuStateMachine->setSyringeActState(syrIdx, "PROCESSING");
                    QTimer::singleShot(MCU_SIM_SYRINGE_ACTION_DELAY_MS, [=] {
                        mcuStateMachine->setSyringeActState(syrIdx, "COMPLETED");
                    });
                }
            }
            sendResponse(msgIn, err);
        }
        else if (action == _L("SETSLACK"))
        {
            if (params.length() >= 2)
            {
                sendResponse(msgIn, "OK");
            }
            else
            {
                sendResponse(msgIn, err);
            }
        }
        else if (action == _L("CALSYRINGEAIRCHECK"))
        {
            if (params.length() >= 1)
            {
                int syrIdx = params.at(0).toInt();
                if ( (syrIdx >= SYRINGE_IDX_START) && (syrIdx < SYRINGE_IDX_MAX) )
                {
                    err = "OK";
                    mcuStateMachine->setSyringeActState(syrIdx, "PROCESSING");

                    QTimer::singleShot(MCU_SIM_SYRINGE_ACTION_DELAY_MS * 2, [=] {
                        mcuStateMachine->setSyringeActState(syrIdx, "COMPLETED");
                    });
                }
            }
            sendResponse(msgIn, err);
        }
        else if (action == _L("SYRINGEAIRCHECK"))
        {
            if (params.length() >= 1)
            {
                int syrIdx = params.at(0).toInt();
                if ( (syrIdx >= SYRINGE_IDX_START) && (syrIdx < SYRINGE_IDX_MAX) )
                {
                    err = "OK";
                    mcuStateMachine->setSyringeActState(syrIdx, "PROCESSING");
                    QTimer::singleShot(MCU_SIM_SYRINGE_ACTION_DELAY_MS * 1, [=] {
                        DS_McuDef::SimDigest simDigest = env->ds.mcuData->getSimDigest();
                        if (simDigest.syringesAirVolume > 0)
                        {
                            mcuStateMachine->setSyringeActState(syrIdx, "AIR_DETECTED");
                        }
                        else
                        {
                            mcuStateMachine->setSyringeActState(syrIdx, "COMPLETED");
                        }
                    });
                }
            }
            sendResponse(msgIn, err);
        }        
        else if (action == _L("GETSYRINGEAIRCHECKDATA"))
        {
            if (params.length() >= 1)
            {
                int syrIdx = params.at(0).toInt();
                if ( (syrIdx >= SYRINGE_IDX_START) && (syrIdx < SYRINGE_IDX_MAX) )
                {
                    // Use fake data for now
                    err = QString().asprintf("%d,%d", 1, 2);
                }
            }
            sendResponse(msgIn, err);
        }
        else if (action == _L("GETSYRINGEAIRCHECKCOEFF"))
        {
            if (params.length() >= 1)
            {
                int syrIdx = params.at(0).toInt();
                if ( (syrIdx >= SYRINGE_IDX_START) && (syrIdx < SYRINGE_IDX_MAX) )
                {
                    // Use fake data for now
                    err = QString().asprintf("%.1f,%.1f,%s", 1.1, 2.2, "sim");
                }
            }
            sendResponse(msgIn, err);
        }
        else if (action == _L("GETSYRINGEAIRVOL"))
        {
            if (params.length() >= 1)
            {
                int syrIdx = params.at(0).toInt();
                if ( (syrIdx >= SYRINGE_IDX_START) && (syrIdx < SYRINGE_IDX_MAX) )
                {
                    DS_McuDef::SimDigest hwDigest = env->ds.mcuData->getSimDigest();
                    err = QString().asprintf("%.1f,%.1f", hwDigest.syringesAirVolume, hwDigest.syringesAirVolume);
                }
            }
            sendResponse(msgIn, err);
        }
        else if (action == _L("SYRINGEVACUUM"))
        {
            if (params.length() >= 1)
            {
                int syrIdx = params.at(0).toInt();
                if ( (syrIdx >= SYRINGE_IDX_START) && (syrIdx < SYRINGE_IDX_MAX) )
                {
                    err = "OK";
                    mcuStateMachine->setSyringeActState(syrIdx, "PROCESSING");
                    QTimer::singleShot(MCU_SIM_SYRINGE_ACTION_DELAY_MS * 2, [=] {
                        mcuStateMachine->setSyringeActState(syrIdx, "COMPLETED");
                    });
                }
            }
            sendResponse(msgIn, err);
        }
        else if (action == _L("BMSDIGEST"))
        {
            // Fake BMS data for simulators
            QString fakeBMSDigest = "\
        ;FirmwareVersion:;0B 1E 9B 01 03 00 16 00 03 80 00 00 : 0B 1E 9B 01 03 00 16 00 03 80 00 00 ;\
        HardwareVersion:;02 AA 00 : 02 AA 00 ;\
        Temperature:;2956: 2956;\
        MaxError:;0: 0;\
        RelativeStateOfCharge:;99: 99;\
        RunTimeToEmpty:;65535: 65535;\
        AveTimeToEmpty:;65535: 65535;\
        AveTimeToFull:;3420: 3420;\
        BatteryStatus:;199: 199;\
        CycleCount:;0 : 0;\
        ManufactureDate:;19529: 19529;\
        SerialNumber:;0000: 0000;\
        HostFETControl:;0002: 0002;\
        CellVoltage10:;3333: 3333;\
        CellVoltage9:;3329: 3329;\
        CellVoltage8:;3335: 3335;\
        CellVoltage7:;3328: 3328;\
        CellVoltage6:;3329: 3329;\
        CellVoltage5:;3338: 3338;\
        CellVoltage4:;3338: 3338;\
        CellVoltage3:;3344: 3344;\
        CellVoltage2:;3328: 3328;\
        CellVoltage1:;3330: 3330;\
        StateOfHealth:;96: 96;\
        SafetyStatus:;04 00 00 00 00 : 04 00 00 00 00 ;\
        OperationStatus:;04 05 01 00 00 : 04 05 01 00 00 ;\
        ChargingStatus:;02 04 02 : 02 04 02 ;\
        GaugingStatus:;02 40 01 : 02 40 01 ;\
        ManufacturingStatus:;02 30 00 : 02 30 00 ;\
        PermanentFailStatus:;04 00 00 00 00 : 04 00 00 00 00 ;\
        DeviceName:;PACKA_REV0J : PACKB_REV0J";
            sendResponse(msgIn, fakeBMSDigest.CSTR());
        }
        else
        {
            sendResponse(msgIn, QString().asprintf("bad command(%s)", action.CSTR()));
        }
    });
}

void McuSim::updateHwStates(DS_McuDef::SimDigest hwDigest, DS_McuDef::SimDigest prevhwDigest)
{
    //LOG_DEBUG("updateHwStates()\n");
    if ( (!hwDigest.sudsInserted) && (prevhwDigest.sudsInserted) )
    {
        mcuStateMachine->triggerSudsRemoved();
    }
    else if ( (hwDigest.sudsInserted) && (!prevhwDigest.sudsInserted) )
    {
        mcuStateMachine->triggerSudsInserted();
    }

    if ( (!hwDigest.mudsLatched) && (prevhwDigest.mudsLatched) )
    {
        mcuStateMachine->triggerMudsUnlatched();
    }

    if ( (hwDigest.mudsPresent != prevhwDigest.mudsPresent) &&
         (!hwDigest.mudsPresent) )
    {
        hwDigest.plungerState[SYRINGE_IDX_SALINE] = "DISENGAGED";
        hwDigest.plungerState[SYRINGE_IDX_CONTRAST1] = "DISENGAGED";
        hwDigest.plungerState[SYRINGE_IDX_CONTRAST2] = "DISENGAGED";
    }

    if (hwDigest.sudsBubbleDetected)
    {
        mcuStateMachine->triggerAirDetected();
    }

    if ( (!hwDigest.isAcPowered) &&
         (hwDigest.batteryState == _L("CRITICAL")) )
    {
        mcuStateMachine->triggerBatteryCritical();
    }

    if (hwDigest.bottleAirDetectedState[SYRINGE_IDX_SALINE] == DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_AIR)
    {
        mcuStateMachine->triggerSalineAirDetector();
    }

    if (hwDigest.bottleAirDetectedState[SYRINGE_IDX_CONTRAST1] == DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_AIR)
    {
        mcuStateMachine->triggerContrast1AirDetector();
    }

    if (hwDigest.bottleAirDetectedState[SYRINGE_IDX_CONTRAST2] == DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_AIR)
    {
        mcuStateMachine->triggerContrast2AirDetector();
    }

    if (hwDigest.stopBtnPressed)
    {
        mcuStateMachine->triggerStopButtonClicked();
    }

    if (hwDigest.startOfDayRequested)
    {
        hwDigest.mudsPresent = true;
        hwDigest.mudsLatched = true;
        hwDigest.outletDoorState = "CLOSED";
        hwDigest.plungerState[SYRINGE_IDX_SALINE] = "ENGAGED";
        hwDigest.plungerState[SYRINGE_IDX_CONTRAST1] = "ENGAGED";
        hwDigest.plungerState[SYRINGE_IDX_CONTRAST2] = "ENGAGED";
        hwDigest.bottleAirDetectedState[SYRINGE_IDX_SALINE] = DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_FLUID;
        hwDigest.bottleAirDetectedState[SYRINGE_IDX_CONTRAST1] = DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_FLUID;
        hwDigest.bottleAirDetectedState[SYRINGE_IDX_CONTRAST2] = DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_FLUID;
        salineControlGroup.syringe->setVol(SYRINGE_VOLUME_MAX);
        salineControlGroup.stopCock->setPosition("INJECT");
        contrast1ControlGroup.syringe->setVol(SYRINGE_VOLUME_MAX);
        contrast1ControlGroup.stopCock->setPosition("FILL");
        contrast2ControlGroup.syringe->setVol(SYRINGE_VOLUME_MAX);
        contrast2ControlGroup.stopCock->setPosition("FILL");
        hwDigest.startOfDayRequested = false;
    }

    env->ds.mcuData->setSimDigest(hwDigest);
}

void McuSim::updateDigest()
{
    //LOG_DEBUG("updateDigest()\n");
    DS_McuDef::SimDigest hwDigest = env->ds.mcuData->getSimDigest();

    hwDigest.digestIdx++;

    McuSimSyringe::Status syringeInfo1 = salineControlGroup.syringe->getInfo();
    McuSimSyringe::Status syringeInfo2 = contrast1ControlGroup.syringe->getInfo();
    McuSimSyringe::Status syringeInfo3 = contrast2ControlGroup.syringe->getInfo();

    hwDigest.flow[SYRINGE_IDX_SALINE] = syringeInfo1.flow;
    hwDigest.vol[SYRINGE_IDX_SALINE] = syringeInfo1.volume;

    hwDigest.flow[SYRINGE_IDX_CONTRAST1] = syringeInfo2.flow;
    hwDigest.vol[SYRINGE_IDX_CONTRAST1] = syringeInfo2.volume;

    hwDigest.flow[SYRINGE_IDX_CONTRAST2] = syringeInfo3.flow;
    hwDigest.vol[SYRINGE_IDX_CONTRAST2] = syringeInfo3.volume;

    hwDigest.injectCompleteStatus = ImrParser::ToImr_InjectionCompleteStatus(mcuStateMachine->getInjectionCompleteStatus());
    hwDigest.injectState = ImrParser::ToImr_InjectorState(mcuStateMachine->getInjectorState());

    if (mcuStateMachine->getInjectorState() == DS_McuDef::INJECTOR_STATE_DELIVERING)
    {
        if ( (hwDigest.pressure == 0) ||
             (hwDigest.digestIdx % MCU_SIM_INJECTION_PRESSURE_UPDATE_DELAY_SAMPLES == 0) )
        {
            quint32 seed = QRandomGenerator::global()->generate();
            QRandomGenerator randomGenerator(seed);
            hwDigest.pressure = (randomGenerator.generate() % MCU_SIM_INJECTION_PRESSURE_RANDOM_NOISE_KPA) + ((PRESSURE_LIMIT_KPA_MIN + PRESSURE_LIMIT_KPA_MAX) / 2);
        }
    }
    else
    {
        hwDigest.pressure = 0;
    }

    hwDigest.stopcockPos[SYRINGE_IDX_SALINE] = salineControlGroup.stopCock->getPosition();
    hwDigest.stopcockPos[SYRINGE_IDX_CONTRAST1] = contrast1ControlGroup.stopCock->getPosition();
    hwDigest.stopcockPos[SYRINGE_IDX_CONTRAST2] = contrast2ControlGroup.stopCock->getPosition();

    hwDigest.syrAction[SYRINGE_IDX_SALINE] = mcuStateMachine->getSyrActState(SYRINGE_IDX_SALINE);
    hwDigest.syrAction[SYRINGE_IDX_CONTRAST1] = mcuStateMachine->getSyrActState(SYRINGE_IDX_CONTRAST1);
    hwDigest.syrAction[SYRINGE_IDX_CONTRAST2] = mcuStateMachine->getSyrActState(SYRINGE_IDX_CONTRAST2);

    if ( (hwDigest.heatMaintainerState == _L("ENABLED_ON")) ||
         (hwDigest.heatMaintainerState == _L("CUTOFF")) )
    {
        if ( (hwDigest.temperature1 < HEATER_CUTOFF_TEMPERATURE) &&
             (hwDigest.temperature2 < HEATER_CUTOFF_TEMPERATURE) )
        {
            hwDigest.heatMaintainerState = "ENABLED_ON";
        }
        else
        {
            hwDigest.heatMaintainerState = "CUTOFF";
        }
    }

    env->ds.mcuData->setSimDigest(hwDigest);
}

bool McuSim::isValidSyringeIdx(int idx)
{
    //LOG_DEBUG("isValidSyringeIdx()\n");
    if ( (idx >= SYRINGE_IDX_SALINE) && (idx < SYRINGE_IDX_MAX))
    {
        return true;
    }
    return false;
}

bool McuSim::isSyringeEngaged(int idx)
{
    //LOG_DEBUG("isSyringeEngaged()\n");
    if ((isValidSyringeIdx(idx)))
    {
        DS_McuDef::SimDigest hwDigest = env->ds.mcuData->getSimDigest();
        return (hwDigest.plungerState[idx] == _L("ENGAGED"));
    }
    return false;
}

QString McuSim::checkMcuArmCondition(QString errCodePrefix)
{
    LOG_DEBUG("checkMcuArmCondition()\n");
    QString err = "OK";
    DS_McuDef::SimDigest hwDigest = env->ds.mcuData->getSimDigest();
    if ( (hwDigest.plungerState[SYRINGE_IDX_SALINE] == _L("DISENGAGED")) ||
         (hwDigest.plungerState[SYRINGE_IDX_CONTRAST1] == _L("DISENGAGED")) ||
         (hwDigest.plungerState[SYRINGE_IDX_CONTRAST2] == _L("DISENGAGED")) )
    {
        err = errCodePrefix + "PlungerNotEngaged";
    }
    else if ( (hwDigest.temperature1 >= HEATER_CUTOFF_TEMPERATURE) ||
              (hwDigest.temperature2 >= HEATER_CUTOFF_TEMPERATURE) )
    {
        err = errCodePrefix + "OverTemperature";
    }
    else if  ( (!hwDigest.isAcPowered) &&
               ( (hwDigest.batteryState == _L("DEAD")) || (hwDigest.batteryState == _L("CRITICAL"))) )
    {
        err = errCodePrefix + "BatteryLevelDead";
    }
    else if ( (!hwDigest.mudsPresent) ||
              (!hwDigest.mudsLatched) )
    {
        err = errCodePrefix + "MUDSNotInserted";
    }
    else if (hwDigest.outletDoorState == _L("OPEN"))
    {
        err = errCodePrefix + "OutletAirDoorOpen";
    }
    else if (!hwDigest.sudsInserted)
    {
        err = errCodePrefix + "PatientLineNotConnected";
    }
    else if (hwDigest.sudsBubbleDetected)
    {
        err = errCodePrefix + "PatientLineAirDetected";
    }
    else if (hwDigest.stopBtnPressed)
    {
        err = errCodePrefix + "StopButtonPressed";
    }

    return err;
}

QString McuSim::checkMcuInjectStartCondition(QString errCodePrefix)
{
    QString err = "OK";
    DS_McuDef::SimDigest hwDigest = env->ds.mcuData->getSimDigest();
    if (hwDigest.stopBtnPressed)
    {
        err = errCodePrefix + "StopButtonPressed";
    }

    return err;
}
