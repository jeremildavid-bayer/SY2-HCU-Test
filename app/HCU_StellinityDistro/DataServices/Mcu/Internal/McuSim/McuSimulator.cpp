#include "Apps/AppManager.h"
#include "McuSimulator.h"
#include "Syringe.h"
#include "StopCock.h"
#include "DataServices/Mcu/DS_McuData.h"
#include "Common/Environment.h"

#define STR_ERR_BAD_PARAMS                          "BAD_PARAMS"
#define INJECTION_PRESSURE_RANDOM_NOISE_KPA         50
#define INJECTION_PRESSURE_UPDATE_DELAY_SAMPLES     50

McuSimulator::McuSimulator(QObject *parent, EnvGlobal *envGlobal_, EnvLocal *env_) :
    QObject(parent),
    envGlobal(envGlobal_),
    env(env_)
{
    connect(envGlobal->appManager, &AppManager::signalAppStarted, this, [=]() {
        slotAppStarted();
    });


    //Init mcu simulator
    salineControlGroup.airDetected = hwData.bottleAirDetectedState[SYRINGE_IDX_SALINE];
    salineControlGroup.stopCock = new StopCock(this, envGlobal);
    salineControlGroup.syringe = new Syringe(this, env);

    contrast1ControlGroup.airDetected = hwData.bottleAirDetectedState[SYRINGE_IDX_CONTRAST1];
    contrast1ControlGroup.stopCock = new StopCock(this, envGlobal);
    contrast1ControlGroup.syringe = new Syringe(this, env);

    contrast2ControlGroup.airDetected = hwData.bottleAirDetectedState[SYRINGE_IDX_CONTRAST2];
    contrast2ControlGroup.stopCock = new StopCock(this, envGlobal);
    contrast2ControlGroup.syringe = new Syringe(this, env);

    mcuStateMachine = new McuStateMachine(this, envGlobal, env, &salineControlGroup, &contrast1ControlGroup, &contrast2ControlGroup);

    showVolume[SYRINGE_IDX_SALINE] = false;
    showVolume[SYRINGE_IDX_CONTRAST1] = false;
    showVolume[SYRINGE_IDX_CONTRAST2] = false;

    //Init some persistant data
    curDigest.alarmCode = 0;

    connect(this, SIGNAL(signalRequestReceived(QString)), this, SLOT(slotRequestReceived(QString)));

    //Set Default Hw Parameters
    initHwData();
    updateDigest();
}

McuSimulator::~McuSimulator()
{
    delete salineControlGroup.stopCock;
    delete salineControlGroup.syringe;
    delete contrast1ControlGroup.stopCock;
    delete contrast1ControlGroup.syringe;
    delete contrast2ControlGroup.stopCock;
    delete contrast2ControlGroup.syringe;
    delete mcuStateMachine;
}

void McuSimulator::slotAppStarted()
{
    connect(envGlobal->ds.mcuData, &DS_McuData::signalMcuDataChanged_SimulatorData, this, [=](DS_McuDef::SimulatorData data){
        updateHwStates(data);
    });
}

void McuSimulator::initHwData()
{
    hwData.resetMudsRequested = false;
    hwData.startOfDayRequested = false;
    hwData.isShuttingDown = false;
    hwData.mudsInserted = false;
    hwData.mudsLatched = false;
    hwData.adaptiveFlow = 0;
    hwData.temperature1 = 35.0;
    hwData.temperature2 = 35.0;
    hwData.sudsInserted = false;
    hwData.sudsBubbleDetected = false;
    hwData.primeBtnPressed = false;
    hwData.stopBtnPressed = false;
    hwData.batteryState = "FULL";
    hwData.isAcPowered = true;
    hwData.doorState = "OPEN";
    hwData.outletDoorState = "OPEN";
    hwData.wasteBinState = "LOW";
    hwData.bottleAirDetectedState[SYRINGE_IDX_SALINE] = DS_McuDef::BOTTLE_BUBBLE_DETECT_AIR;
    hwData.bottleAirDetectedState[SYRINGE_IDX_CONTRAST1] = DS_McuDef::BOTTLE_BUBBLE_DETECT_AIR;
    hwData.bottleAirDetectedState[SYRINGE_IDX_CONTRAST2] = DS_McuDef::BOTTLE_BUBBLE_DETECT_AIR;
    hwData.plungerStatus[SYRINGE_IDX_SALINE] = "DISENGAGED";
    hwData.plungerStatus[SYRINGE_IDX_CONTRAST1] = "DISENGAGED";
    hwData.plungerStatus[SYRINGE_IDX_CONTRAST2] = "DISENGAGED";
    hwData.stopcockPos[SYRINGE_IDX_SALINE] = "INJECT";
    hwData.stopcockPos[SYRINGE_IDX_CONTRAST1] = "INJECT";
    hwData.stopcockPos[SYRINGE_IDX_CONTRAST2] = "INJECT";
    for (int i = 0; i < ARRAY_LEN(hwData.ledColor); i++)
    {
        hwData.ledColor[i] = "OFF";
    }

    curDigest.temperature1 = 35.0;
    curDigest.temperature2 = 35.0;
    curDigest.heatMaintainerState = "ENABLED";
}

void McuSimulator::setEnabled(bool enabled)
{
    isEnabled = enabled;
    if (enabled)
    {
        emit signalConnected();
    }
    else
    {
        emit signalDisconnected();
    }
}

void McuSimulator::slotRequestReceived(QString req)
{
    QStringList temp1 = req.split("@");

    if (isEnabled)
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
                if (temp1.at(1) != QLatin1String(""))
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

            handleAction(action, params, msgbuf);
        }
    }
}

void McuSimulator::sendMsg(QString req)
{
    emit signalRequestReceived(req);
}

void McuSimulator::sendResponse(QString originalMsg, QString reply)
{
    emit signalRxMsg(QString().sprintf("%s#%s\r", originalMsg.toStdString().c_str(), reply.toStdString().c_str()));
}

void McuSimulator::handleAction(QString action, QStringList params, QString msgIn)
{
    QTimer::singleShot(MCU_SIM_PROCESSING_TIME_MS, [=]{

        if (!envGlobal->distroIsRunning)
        {
            return;
        }

        if (action == QLatin1String("INFO"))
        {
            sendResponse(msgIn, "SUPER MARIO'S SIMULATOR");
        }
        else if (action == QLatin1String("VERSION"))
        {
            sendResponse(msgIn, QString().sprintf("SIM %s, SC", COMPATIBLE_MCU_VERSION));
        }
        else if (action == QLatin1String("DIGEST"))
        {
            updateDigest();
            sendResponse(msgIn, QString().sprintf("%016llX,%s,%d,%s,%d,%d,%s,%s,%s,%s,%s,%s,%s,%s,%s,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%s,%s,%s,%s,%d,%d,%s,%s,%s,%d,%d,%d,%d,%d,%f,%f,%s,%d"
                                                    ,curDigest.alarmCode
                                                    ,curDigest.injectState.toStdString().c_str()
                                                    ,curDigest.curPhaseIdx
                                                    ,curDigest.injectCompleteState.toStdString().c_str()
                                                    ,curDigest.pressure
                                                    ,curDigest.adaptiveFlow
                                                    ,curDigest.stopcock[SYRINGE_IDX_SALINE].toStdString().c_str()
                                                    ,curDigest.stopcock[SYRINGE_IDX_CONTRAST1].toStdString().c_str()
                                                    ,curDigest.stopcock[SYRINGE_IDX_CONTRAST2].toStdString().c_str()
                                                    ,curDigest.plunger[SYRINGE_IDX_SALINE].toStdString().c_str()
                                                    ,curDigest.plunger[SYRINGE_IDX_CONTRAST1].toStdString().c_str()
                                                    ,curDigest.plunger[SYRINGE_IDX_CONTRAST2].toStdString().c_str()
                                                    ,curDigest.syrAction[SYRINGE_IDX_SALINE].toStdString().c_str()
                                                    ,curDigest.syrAction[SYRINGE_IDX_CONTRAST1].toStdString().c_str()
                                                    ,curDigest.syrAction[SYRINGE_IDX_CONTRAST2].toStdString().c_str()
                                                    ,curDigest.vol[SYRINGE_IDX_SALINE]
                                                    ,curDigest.vol[SYRINGE_IDX_CONTRAST1]
                                                    ,curDigest.vol[SYRINGE_IDX_CONTRAST2]
                                                    ,curDigest.flow[SYRINGE_IDX_SALINE]
                                                    ,curDigest.flow[SYRINGE_IDX_CONTRAST1]
                                                    ,curDigest.flow[SYRINGE_IDX_CONTRAST2]
                                                    ,curDigest.battery.toStdString().c_str()
                                                    ,curDigest.isAcPowered ? "AC" : "BATTERY"
                                                    ,curDigest.door.toStdString().c_str()
                                                    ,curDigest.wasteBin.toStdString().c_str()
                                                    ,curDigest.mudsInserted ? 1 : 0
                                                    ,curDigest.mudsLatched ? 1 : 0
                                                    ,DS_McuDef::getBottleBubbleDetectorStateStr((DS_McuDef::BottleBubbleDetectorState)curDigest.inbubble[SYRINGE_IDX_SALINE]).toStdString().c_str()
                                                    ,DS_McuDef::getBottleBubbleDetectorStateStr((DS_McuDef::BottleBubbleDetectorState)curDigest.inbubble[SYRINGE_IDX_CONTRAST1]).toStdString().c_str()
                                                    ,DS_McuDef::getBottleBubbleDetectorStateStr((DS_McuDef::BottleBubbleDetectorState)curDigest.inbubble[SYRINGE_IDX_CONTRAST2]).toStdString().c_str()
                                                    ,curDigest.sudsInserted ? 1 : 0
                                                    ,curDigest.sudsBubbleDetected ? 1 : 0
                                                    ,curDigest.primeBtnPressed ? 1 : 0
                                                    ,curDigest.stopBtnPressed ? 1 : 0
                                                    ,curDigest.outletDoorState == "CLOSED" ? 1 : 0
                                                    ,curDigest.temperature1
                                                    ,curDigest.temperature2
                                                    ,curDigest.heatMaintainerState.toStdString().c_str()
                                                    ,curDigest.isShuttingDown ? 1 : 0 ));
        }
        else if (action == QLatin1String("CLEARALARMS"))
        {
            curDigest.alarmCode = 0;
            sendResponse(msgIn, "OK");
        }
        else if (action == QLatin1String("ARM"))
        {
            QString err;
            if ((err = mcuStateMachine->setActiveProtocol(params)) == QLatin1String("OK"))
            {
                err = checkMcuHardwareStates();
                if (err == "OK")
                {
                    err = mcuStateMachine->setInjectState("ARM");
                }
            }

            sendResponse(msgIn, err);
        }
        else if (action == QLatin1String("DISARM"))
        {
            QString err;
            err = mcuStateMachine->setInjectState("DISARM");

            sendResponse(msgIn, err);
        }
        else if (action == QLatin1String("INJECT"))
        {
            QString err;
            int jumpPhaseIdx = -1;
            if (params.length() == 2)
            {
                bool ok;
                jumpPhaseIdx = params.at(1).toInt(&ok);
                jumpPhaseIdx = ok ? jumpPhaseIdx : -1;
            }

            if (params.at(0) == "START")
            {
                if (!hwData.sudsInserted)
                {
                    err = "Patient Line Missing";
                    mcuStateMachine->setInjectState("DISARM");
                }
            }

            if (err == "")
            {
                err = mcuStateMachine->setInjectionControl(params.at(0),jumpPhaseIdx);
            }
            sendResponse(msgIn, err);
        }
        else if (action == QLatin1String("POWER"))
        {
            QString err = STR_ERR_BAD_PARAMS;
            if (params.at(0) == QLatin1String("REBOOT"))
            {
                err = "OK";
                hwData.isShuttingDown = true;
                QTimer::singleShot(1000, [=] {
                    LOG_DEBUG("'REBOOT' has been issued\n");
                });
            }
            else if (params.at(0) == QLatin1String("OFF"))
            {
                err = "OK";
                hwData.isShuttingDown = true;
                QTimer::singleShot(1000, [=] {
                    LOG_DEBUG("'OFF' has been issued\n");
                });
            }
            sendResponse(msgIn, err);
        }
        else if (action == QLatin1String("ADJFLOW"))
        {
            QString err = STR_ERR_BAD_PARAMS;
            if (params.length() == 1)
            {
                bool ok;
                double flow = params.at(0).toDouble(&ok);
                if (ok)
                {
                    err = mcuStateMachine->adjustFlow(flow);
                }
            }

            sendResponse(msgIn, err);
        }
        else if (action == QLatin1String("STOPCOCK"))
        {
            QString err = STR_ERR_BAD_PARAMS;
            if (params.length() == 3)
            {
                err = salineControlGroup.stopCock->setPosition(params.at(0));
                err = (err == QLatin1String("OK")) ? contrast1ControlGroup.stopCock->setPosition(params.at(1)) : err;
                err = (err == QLatin1String("OK")) ? contrast2ControlGroup.stopCock->setPosition(params.at(2)) : err;
            }

            sendResponse(msgIn, err);
        }
        else if (action == QLatin1String("LEDS"))
        {
            QString err = STR_ERR_BAD_PARAMS;
            if (params.length() >= 11)
            {
                for (int i = 0; i < params.length(); i++)
                {
                    if (params.at(i) != QLatin1String("NOCHANGE"))
                    {
                        hwData.ledColor[i] = params.at(i);
                    }
                }
                envGlobal->ds.mcuData->setSimulatorData(hwData);
                err = "OK";
            }
            sendResponse(msgIn, err);
        }
        else if (action == QLatin1String("ENGAGE"))
        {
            sendResponse(msgIn, "OK");
            int idx = params.at(0).toInt();
            mcuStateMachine->setSyringeActState(idx, "PROCESSING");

            QTimer::singleShot(MCU_SIM_SYRINGE_ACTION_DELAY_MS, [=] {
                bool ok;
                int syrIdx = params.at(0).toInt(&ok);
                if (ok)
                {
                    if ( (syrIdx >= 0) && (syrIdx <= 2) )
                    {
                        showVolume[idx] = true;
                        hwData.plungerStatus[syrIdx] = "ENGAGED";
                        mcuStateMachine->setSyringeActState(syrIdx, "COMPLETED");
                    }
                }
            });
        }
        else if (action == QLatin1String("ENGAGEALL"))
        {
            sendResponse(msgIn, "OK");
            mcuStateMachine->setSyringeActState(0, "PROCESSING");
            mcuStateMachine->setSyringeActState(1, "PROCESSING");
            mcuStateMachine->setSyringeActState(2, "PROCESSING");

            QTimer::singleShot(MCU_SIM_SYRINGE_ACTION_DELAY_MS, [=] {
                hwData.plungerStatus[SYRINGE_IDX_SALINE] = "ENGAGED";
                hwData.plungerStatus[SYRINGE_IDX_CONTRAST1] = "ENGAGED";
                hwData.plungerStatus[SYRINGE_IDX_CONTRAST2] = "ENGAGED";
                showVolume[SYRINGE_IDX_SALINE] = true;
                showVolume[SYRINGE_IDX_CONTRAST1] = true;
                showVolume[SYRINGE_IDX_CONTRAST2] = true;
                mcuStateMachine->setSyringeActState(0, "COMPLETED");
                mcuStateMachine->setSyringeActState(1, "COMPLETED");
                mcuStateMachine->setSyringeActState(2, "COMPLETED");
            });
        }
        else if (action == QLatin1String("DISENGAGE"))
        {
            sendResponse(msgIn, "OK");
            int idx = params.at(0).toInt();
            mcuStateMachine->setSyringeActState(idx, "PROCESSING");

            QTimer::singleShot(MCU_SIM_SYRINGE_ACTION_DELAY_MS, [=] {
                bool ok;
                int syrIdx = params.at(0).toInt(&ok);
                if (ok)
                {
                    if ( (syrIdx >= 0) && (syrIdx <= 2) )
                    {
                        showVolume[syrIdx] = false;
                        hwData.plungerStatus[syrIdx] = "DISENGAGED";
                        mcuStateMachine->setSyringeActState(syrIdx, "COMPLETED");
                    }
                }
            });
        }
        else if (action == QLatin1String("DISENGAGEALL"))
        {
            sendResponse(msgIn, "OK");
            mcuStateMachine->setSyringeActState(0, "PROCESSING");
            mcuStateMachine->setSyringeActState(1, "PROCESSING");
            mcuStateMachine->setSyringeActState(2, "PROCESSING");
            showVolume[SYRINGE_IDX_SALINE] = false;
            showVolume[SYRINGE_IDX_CONTRAST1] = false;
            showVolume[SYRINGE_IDX_CONTRAST2] = false;

            QTimer::singleShot(MCU_SIM_SYRINGE_ACTION_DELAY_MS, [=] {
                hwData.plungerStatus[SYRINGE_IDX_SALINE] = "DISENGAGED";
                hwData.plungerStatus[SYRINGE_IDX_CONTRAST1] = "DISENGAGED";
                hwData.plungerStatus[SYRINGE_IDX_CONTRAST2] = "DISENGAGED";
                mcuStateMachine->setSyringeActState(0, "COMPLETED");
                mcuStateMachine->setSyringeActState(1, "COMPLETED");
                mcuStateMachine->setSyringeActState(2, "COMPLETED");
            });
        }
        else if (action == QLatin1String("FINDPLUNGER"))
        {
            QString errStr = STR_ERR_BAD_PARAMS;
            if (params.length() > 0)
            {
                int idx = params.at(0).toInt();
                if ( (idx >= 0) && (idx < SYRINGE_IDX_MAX) )
                {
                    mcuStateMachine->setSyringeActState(idx, "PROCESSING");
                    errStr = "OK";

                    QTimer::singleShot(MCU_SIM_SYRINGE_ACTION_DELAY_MS, [=] {
                        showVolume[idx] = true;
                        mcuStateMachine->setSyringeActState(idx, "COMPLETED");
                    });
                }
            }
            sendResponse(msgIn, errStr);
        }
        else if (action == QLatin1String("FINDPLUNGERALL"))
        {
            sendResponse(msgIn, "OK");
            mcuStateMachine->setSyringeActState(0, "PROCESSING");
            mcuStateMachine->setSyringeActState(1, "PROCESSING");
            mcuStateMachine->setSyringeActState(2, "PROCESSING");

            QTimer::singleShot(MCU_SIM_SYRINGE_ACTION_DELAY_MS, [=] {
                showVolume[SYRINGE_IDX_SALINE] = true;
                showVolume[SYRINGE_IDX_CONTRAST1] = true;
                showVolume[SYRINGE_IDX_CONTRAST2] = true;
                mcuStateMachine->setSyringeActState(SYRINGE_IDX_SALINE, "COMPLETED");
                mcuStateMachine->setSyringeActState(SYRINGE_IDX_CONTRAST1, "COMPLETED");
                mcuStateMachine->setSyringeActState(SYRINGE_IDX_CONTRAST2, "COMPLETED");
            });
        }
        else if (action == QLatin1String("FILL"))
        {
            bool ok;
            QString err = STR_ERR_BAD_PARAMS;
            int syrIdx = params.at(0).toInt(&ok);
            if (ok)
            {
                err = isSyringeEngaged(syrIdx) ? mcuStateMachine->setSyringeAction(syrIdx, action, params.at(1).toDouble(), params.at(2).toDouble(), 0) : "NOT ENGAGED";
                hwData.bottleAirDetectedState[syrIdx] = DS_McuDef::BOTTLE_BUBBLE_DETECT_FLUID;
            }

            sendResponse(msgIn, err);
        }
        else if (action == QLatin1String("PRIME"))
        {
            bool ok;
            QString err = STR_ERR_BAD_PARAMS;
            int syrIdx = params.at(0).toInt(&ok);
            if (ok)
            {
                err = isSyringeEngaged(syrIdx) ? mcuStateMachine->setSyringeAction(syrIdx, action, params.at(1).toDouble(), params.at(2).toDouble(), 0) : "NOT ENGAGED";
            }

            sendResponse(msgIn, err);
        }
        else if (action == QLatin1String("PISTON"))
        {
            bool ok;
            QString err = STR_ERR_BAD_PARAMS;
            int syrIdx = params.at(0).toInt(&ok);
            if (ok)
            {
                double vol  = params.at(1).toDouble(&ok);
                double flow = params.at(2).toDouble(&ok);
                if (ok && (flow > 0))
                {
                    err = mcuStateMachine->setSyringeAction(syrIdx, "PUSH", vol, flow, 0);
                }
                else if (ok && (flow < 0))
                {
                    double posFlow = flow*(-1);
                    err = mcuStateMachine->setSyringeAction(syrIdx, "PULL", vol, posFlow, 0);
                }
            }
            sendResponse(msgIn, err);
        }
        else if (action == QLatin1String("PRIMEAUTO"))
        {
            bool ok;
            QString err = STR_ERR_BAD_PARAMS;
            int syrIdx = params.at(0).toInt(&ok);
            if (ok)
            {
                err = isSyringeEngaged(syrIdx) ?  mcuStateMachine->setSyringeAction(syrIdx, action, params.at(1).toDouble(), params.at(2).toDouble(), params.at(3).toDouble()) : "NOT ENGAGED";
            }

            sendResponse(msgIn, err);
        }
        else if (action == QLatin1String("PURGE"))
        {
            bool ok;
            QString err = STR_ERR_BAD_PARAMS;
            int syrIdx = params.at(0).toInt(&ok);
            if (ok)
            {
                err = mcuStateMachine->setSyringeAction(syrIdx, "PURGE", SYRINGE_VOLUME_MAX, MCU_SIM_PURGE_FLOW, 0);
            }
            sendResponse(msgIn, err);
        }
        else if (action == QLatin1String("PURGEALL"))
        {
            QString err = STR_ERR_BAD_PARAMS;
            err = mcuStateMachine->setSyringeAction(SYRINGE_IDX_SALINE, "PURGE", SYRINGE_VOLUME_MAX, MCU_SIM_PURGE_FLOW, 0);
            err = mcuStateMachine->setSyringeAction(SYRINGE_IDX_CONTRAST1, "PURGE", SYRINGE_VOLUME_MAX, MCU_SIM_PURGE_FLOW, 0);
            err = mcuStateMachine->setSyringeAction(SYRINGE_IDX_CONTRAST2, "PURGE", SYRINGE_VOLUME_MAX, MCU_SIM_PURGE_FLOW, 0);
            sendResponse(msgIn, err);
        }
        else if (action == QLatin1String("CALMOTOR"))
        {
             QTimer::singleShot(25000, [=]{
                sendResponse(msgIn, "OK");
             });
        }
        else if (action == QLatin1String("CALBUB"))
        {
            QTimer::singleShot(7500, [=]{
               sendResponse(msgIn, "OK");
            });
        }
        else if (action == QLatin1String("CALZEROPOS"))
        {
            sendResponse(msgIn, QLatin1String("OK"));
        }
        else if (action == QLatin1String("SYRSTOP"))
        {
            bool ok;
            QString err = STR_ERR_BAD_PARAMS;
            int syrIdx = params.at(0).toInt(&ok);
            if (ok)
            {
                err = mcuStateMachine->setSyringeAction(syrIdx, "SYRSTOP", 0, 0, 0);
            }
            sendResponse(msgIn, err);
        }
        else if (action == QLatin1String("STOPALL"))
        {
            QString err = STR_ERR_BAD_PARAMS;
            err = mcuStateMachine->setSyringeAction(SYRINGE_IDX_SALINE, "SYRSTOP", 0, 0, 0);
            err = mcuStateMachine->setSyringeAction(SYRINGE_IDX_CONTRAST1, "SYRSTOP", 0, 0, 0);
            err = mcuStateMachine->setSyringeAction(SYRINGE_IDX_CONTRAST2, "SYRSTOP", 0, 0, 0);
            sendResponse(msgIn, err);
        }
        else if (action == QLatin1String("RESETMUDS"))
        {
            QString err = mcuStateMachine->resetMuds() ? "OK" : "BAD_STATE";
            sendResponse(msgIn, err);
        }
        else if (action == QLatin1String("HEATER"))
        {
            double setTemp = params.at(0).toDouble();
            if (setTemp == 0.0)
            {
                // heat maintainer turned off
                curDigest.temperature1 = 0;
                curDigest.temperature2 = 0;
                curDigest.heatMaintainerState = "DISABLED";
            }
            else
            {
                curDigest.temperature1 = hwData.temperature1;
                curDigest.temperature2 = hwData.temperature2;
                curDigest.heatMaintainerState = "ENABLED";
            }
            sendResponse(msgIn, "OK");
        }
        else if (action == QLatin1String("SERVICEDIGEST"))
        {
            sendResponse(msgIn, "0,0,4.8,15.5,OFF,5.0,12.0,28.5,1.0,1.2,1.4,0,1.0,1,1,1,0,0,0,0,0,0");
        }
        else if (action == QLatin1String("CALSUDS"))
        {
            QTimer::singleShot(10000, [=]{
               sendResponse(msgIn, "OK");
            });
        }
        else if (action == QLatin1String("DOORLOCK"))
        {
            sendResponse(msgIn, "OK");
        }
        else
        {
            sendResponse(msgIn, "UNKNOWN COMMAND");
        }
    });
}

void McuSimulator::updateHwStates(DS_McuDef::SimulatorData newHwData)
{
    if ( (!newHwData.sudsInserted) && (hwData.sudsInserted) )
    {
        mcuStateMachine->triggerSudsRemoved();
    }

    if ( (newHwData.mudsInserted != hwData.mudsInserted) && (!newHwData.mudsInserted) )
    {
        newHwData.plungerStatus[SYRINGE_IDX_SALINE] = "DISENGAGED";
        newHwData.plungerStatus[SYRINGE_IDX_CONTRAST1] = "DISENGAGED";
        newHwData.plungerStatus[SYRINGE_IDX_CONTRAST2] = "DISENGAGED";
        showVolume[SYRINGE_IDX_SALINE] = false;
        showVolume[SYRINGE_IDX_CONTRAST1] = false;
        showVolume[SYRINGE_IDX_CONTRAST2] = false;
    }

    hwData = newHwData;
    if (hwData.sudsBubbleDetected)
    {
        mcuStateMachine->triggerAirDetected();
        hwData.sudsBubbleDetected = false;
    }

    if (hwData.bottleAirDetectedState[SYRINGE_IDX_SALINE] == DS_McuDef::BOTTLE_BUBBLE_DETECT_AIR)
    {
        mcuStateMachine->triggerSalineAirDetector();
    }

    if (hwData.bottleAirDetectedState[SYRINGE_IDX_CONTRAST1] == DS_McuDef::BOTTLE_BUBBLE_DETECT_AIR)
    {
        mcuStateMachine->triggerContrast1AirDetector();
    }

    if (hwData.bottleAirDetectedState[SYRINGE_IDX_CONTRAST2] == DS_McuDef::BOTTLE_BUBBLE_DETECT_AIR)
    {
        mcuStateMachine->triggerContrast2AirDetector();
    }

    if (hwData.resetMudsRequested)
    {
        mcuStateMachine->resetMuds();
        hwData.resetMudsRequested = false;
    }

    if (hwData.stopBtnPressed)
    {
        mcuStateMachine->stopButtonClicked();
    }

    if (hwData.startOfDayRequested)
    {
        hwData.mudsInserted = true;
        hwData.mudsLatched = true;
        hwData.sudsInserted = true;
        hwData.plungerStatus[SYRINGE_IDX_SALINE] = "ENGAGED";
        hwData.plungerStatus[SYRINGE_IDX_CONTRAST1] = "ENGAGED";
        hwData.plungerStatus[SYRINGE_IDX_CONTRAST2] = "ENGAGED";
        showVolume[SYRINGE_IDX_SALINE] = true;
        showVolume[SYRINGE_IDX_CONTRAST1] = true;
        showVolume[SYRINGE_IDX_CONTRAST2] = true;
        salineControlGroup.syringe->setVol(SYRINGE_VOLUME_MAX);
        salineControlGroup.stopCock->setPosition("INJECT");
        contrast1ControlGroup.syringe->setVol(SYRINGE_VOLUME_MAX);
        contrast1ControlGroup.stopCock->setPosition("FILL");
        contrast2ControlGroup.syringe->setVol(SYRINGE_VOLUME_MAX);
        contrast2ControlGroup.stopCock->setPosition("FILL");
        hwData.outletDoorState = "CLOSED";

        hwData.startOfDayRequested = false;
    }
}

void McuSimulator::updateDigest()
{
    static uint32_t digestIdx = 0;

    digestIdx++;

    //curDigest.alarmCode = 0x00000000;
    curDigest.adaptiveFlow = hwData.adaptiveFlow;
    curDigest.battery = hwData.batteryState;
    curDigest.isAcPowered = hwData.isAcPowered;
    curDigest.door = hwData.doorState;
    curDigest.outletDoorState = hwData.outletDoorState;
    curDigest.curPhaseIdx = mcuStateMachine->getCurPhase();
    curDigest.flow[SYRINGE_IDX_SALINE] = salineControlGroup.syringe->getInfo().flow;
    curDigest.flow[SYRINGE_IDX_CONTRAST1] = contrast1ControlGroup.syringe->getInfo().flow;
    curDigest.flow[SYRINGE_IDX_CONTRAST2] = contrast2ControlGroup.syringe->getInfo().flow;
    curDigest.vol[SYRINGE_IDX_SALINE] = showVolume[SYRINGE_IDX_SALINE] ? salineControlGroup.syringe->getInfo().volume : -1;
    curDigest.vol[SYRINGE_IDX_CONTRAST1] = showVolume[SYRINGE_IDX_CONTRAST1] ? contrast1ControlGroup.syringe->getInfo().volume : -1;
    curDigest.vol[SYRINGE_IDX_CONTRAST2] = showVolume[SYRINGE_IDX_CONTRAST2] ? contrast2ControlGroup.syringe->getInfo().volume : -1;
    curDigest.injectCompleteState = mcuStateMachine->getInjectCompleteState();
    curDigest.injectState = mcuStateMachine->getInjectProgressState();
    curDigest.mudsInserted = hwData.mudsInserted;
    curDigest.mudsLatched = hwData.mudsLatched;
    curDigest.plunger[SYRINGE_IDX_SALINE] = hwData.plungerStatus[SYRINGE_IDX_SALINE];
    curDigest.plunger[SYRINGE_IDX_CONTRAST1] = hwData.plungerStatus[SYRINGE_IDX_CONTRAST1];
    curDigest.plunger[SYRINGE_IDX_CONTRAST2] = hwData.plungerStatus[SYRINGE_IDX_CONTRAST2];
    curDigest.temperature1 = hwData.temperature1;
    curDigest.temperature2 = hwData.temperature2;


    if (curDigest.injectState == "DELIVERING")
    {
        if ( (curDigest.pressure == 0) ||
             (digestIdx % INJECTION_PRESSURE_UPDATE_DELAY_SAMPLES == 0) )
        {
            qsrand(qrand());
            curDigest.pressure = (qrand() % INJECTION_PRESSURE_RANDOM_NOISE_KPA) + ((PRESSURE_LIMIT_KPA_MIN + PRESSURE_LIMIT_KPA_MAX) / 2);
        }
    }
    else if (curDigest.injectState == "HOLDING")
    {
        curDigest.pressure = 0;
    }
    else
    {
        curDigest.pressure = 0;
    }

    curDigest.primeBtnPressed = hwData.primeBtnPressed;
    curDigest.stopBtnPressed = hwData.stopBtnPressed;
    curDigest.stopcock[SYRINGE_IDX_SALINE] = salineControlGroup.stopCock->getPosition();
    curDigest.stopcock[SYRINGE_IDX_CONTRAST1] = contrast1ControlGroup.stopCock->getPosition();
    curDigest.stopcock[SYRINGE_IDX_CONTRAST2] = contrast2ControlGroup.stopCock->getPosition();
    curDigest.sudsInserted = hwData.sudsInserted;
    curDigest.sudsBubbleDetected = hwData.sudsBubbleDetected;
    curDigest.syrAction[SYRINGE_IDX_SALINE] = mcuStateMachine->getSyrActState(SYRINGE_IDX_SALINE);
    curDigest.syrAction[SYRINGE_IDX_CONTRAST1] = mcuStateMachine->getSyrActState(SYRINGE_IDX_CONTRAST1);
    curDigest.syrAction[SYRINGE_IDX_CONTRAST2] = mcuStateMachine->getSyrActState(SYRINGE_IDX_CONTRAST2);
    curDigest.wasteBin = hwData.wasteBinState;
    curDigest.inbubble[SYRINGE_IDX_SALINE] = hwData.bottleAirDetectedState[SYRINGE_IDX_SALINE];
    curDigest.inbubble[SYRINGE_IDX_CONTRAST1] = hwData.bottleAirDetectedState[SYRINGE_IDX_CONTRAST1];
    curDigest.inbubble[SYRINGE_IDX_CONTRAST2] = hwData.bottleAirDetectedState[SYRINGE_IDX_CONTRAST2];
    curDigest.isShuttingDown = hwData.isShuttingDown;
    envGlobal->ds.mcuData->setSimulatorData(hwData);
}

bool McuSimulator::isValidSyringeIdx(int idx)
{
    if ( (idx >= SYRINGE_IDX_SALINE) && (idx < SYRINGE_IDX_MAX))
    {
        return true;
    }
    return false;
}

bool McuSimulator::isSyringeEngaged(int idx)
{
    if ((isValidSyringeIdx(idx)))
    {
        return (hwData.plungerStatus[idx] == QLatin1String("ENGAGED"));
    }
    return false;
}

QString McuSimulator::checkMcuHardwareStates()
{
    QString err = "OK";

    if (hwData.plungerStatus[SYRINGE_IDX_SALINE] == "DISENGAGED")
    {
        err = "T_ARMFAILED_PlungerNotEngaged";
    }
    else if (hwData.plungerStatus[SYRINGE_IDX_CONTRAST1] == "DISENGAGED")
    {
        err = "T_ARMFAILED_PlungerNotEngaged";
    }
    else if (hwData.plungerStatus[SYRINGE_IDX_CONTRAST2] == "DISENGAGED")
    {
        err = "T_ARMFAILED_PlungerNotEngaged";
    }
    else if (hwData.temperature1 >= 45)
    {
        err = "T_ARMFAILED_OverTemperature";
    }
    else if (hwData.temperature2 >= 45)
    {
        err = "T_ARMFAILED_OverTemperature";
    }
    else if (!hwData.isAcPowered && (hwData.batteryState == "DEAD"))
    {
        err = "T_ARMFAILED_BatteryLevelDead";
    }
    else if (!hwData.isAcPowered && (hwData.batteryState == "CRITICAL"))
    {
        err = "T_ARMFAILED_BatteryLevelDead";
    }
    //TODO: Door open and outlet air door open conditions

    return err;
}
