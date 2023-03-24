#include "McuLink.h"
#include "Apps/AppManager.h"
#include "Common/Util.h"
#include "Common/ImrParser.h"
#include "DataServices/Mcu/DS_McuAction.h"
#include "DataServices/Mcu/DS_McuData.h"
#include "DataServices/Alert/DS_AlertAction.h"
#include "DataServices/Capabilities/DS_Capabilities.h"
#include "DataServices/CfgGlobal/DS_CfgGlobal.h"

McuLink::McuLink(QObject *parent, EnvGlobal *env_) :
    QObject(parent),
    env(env_)
{
    envLocal = new EnvLocal("DS_Mcu-Link", "MCU_LINK", LOG_LRG_SIZE_BYTES * 4);

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });

    msgHandler = new McuMsgHandler(this, env, envLocal);
    simulator = new McuSim(this, env);
    hardware = new McuHw(this, env, envLocal);
    isSimulationMode = false;  //assume will be overided on loaded

    // Setup Heartbeat
    connect(&tmrCommHeartBeat, SIGNAL(timeout()), SLOT(slotHeartBeatTimeout()));
    connect(&tmrHwDigestMonitor, SIGNAL(timeout()), SLOT(slotHwDigestMonitor()));
    connect(&tmrInjectDigestMonitor, SIGNAL(timeout()), SLOT(slotInjectDigestMonitor()));
    connect(&tmrRxTimeout, SIGNAL(timeout()), SLOT(slotRxTimeout()));
    connect(&tmrTxMsgImmediate, SIGNAL(timeout()), SLOT(slotTxMsgImmediate()));

    connect(this, SIGNAL(signalTxMsg(QString,QString,int)), SLOT(slotTxMsg(QString,QString,int)));

    connect(this, SIGNAL(signalLinkConnect()), SLOT(slotLinkConnect()));
    connect(this, SIGNAL(signalLinkDisconnect()), SLOT(slotLinkDisconnect()));
}

McuLink::~McuLink()
{
    hardware->deleteLater();
    simulator->deleteLater();
    msgHandler->deleteLater();
    delete envLocal;
}

void McuLink::slotAppInitialised()
{
    connect(env->ds.systemData, &DS_SystemData::signalDataChanged_StatePath, this, [=](DS_SystemDef::StatePath curStatePath, DS_SystemDef::StatePath prevStatePath) {
        if ( ( (prevStatePath == DS_SystemDef::STATE_PATH_IDLE) && (curStatePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING) ) ||
             ( (prevStatePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING) && (curStatePath == DS_SystemDef::STATE_PATH_IDLE) ) )
        {
            LOG_INFO("\n\n-----------------\nStatePath=%s\n\n", ImrParser::ToImr_StatePath(curStatePath).CSTR());
        }
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_HwDigestMonitorEnabled, this, [=](bool HwDigestMonitorEnabled) {
        if (HwDigestMonitorEnabled)
        {
            if (!tmrHwDigestMonitor.isActive())
            {
                tmrHwDigestMonitor.start(MCU_SERVICE_MONITOR_INTERVAL_MS);
            }
        }
        else
        {
            tmrHwDigestMonitor.stop();
        }
    });

    connect(env->ds.systemData, &DS_SystemData::signalDataChanged_StatePath, this, [=](DS_SystemDef::StatePath statePath) {
        if ( (statePath == DS_SystemDef::STATE_PATH_EXECUTING) ||
             (statePath == DS_SystemDef::STATE_PATH_BUSY_HOLDING) ||
             (statePath == DS_SystemDef::STATE_PATH_BUSY_FINISHING) )
        {
            int heartBeatIntervalMs = MCU_LINK_HEART_BEAT_INTERVAL_MS * 0.5;
            if ( (!tmrInjectDigestMonitor.isActive()) ||
                 (tmrInjectDigestMonitor.interval() != heartBeatIntervalMs) )
            {
                tmrInjectDigestMonitor.stop();
                tmrInjectDigestMonitor.start(heartBeatIntervalMs);
            }
        }
        else
        {
            tmrInjectDigestMonitor.stop();
        }
    });

    connect(env->ds.capabilities, &DS_Capabilities::signalConfigChanged_Developer_McuSimulationEnabled, this, [=] {
        slotLinkConnect();
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_LinkState, this, [=](DS_McuDef::LinkState curLinkState, DS_McuDef::LinkState prevLinkState) {
        LOG_INFO("\n");
        LOG_INFO("MCU Link State = %s -> %s\n\n", ImrParser::ToImr_McuLinkState(prevLinkState).CSTR(), ImrParser::ToImr_McuLinkState(curLinkState).CSTR());

        if ( (curLinkState == DS_McuDef::LINK_STATE_RECOVERING) &&
             (prevLinkState != DS_McuDef::LINK_STATE_RECOVERING) )
        {
            // Start MCU heart beat
            if (MCU_LINK_HEART_BEAT_INTERVAL_MS > 0)
            {
                tmrCommHeartBeat.stop();
                tmrCommHeartBeat.start(MCU_LINK_HEART_BEAT_INTERVAL_MS);
            }
        }

        if ( (curLinkState == DS_McuDef::LINK_STATE_CONNECTED) &&
             (prevLinkState != DS_McuDef::LINK_STATE_CONNECTED) )
        {
            mcuLinkInitProcess();
        }
    });
}

void McuLink::slotLinkConnect()
{
    slotLinkDisconnected();

    isSimulationMode = env->ds.capabilities->get_Developer_McuSimulationEnabled();
    env->ds.mcuData->setLinkState(DS_McuDef::LINK_STATE_CONNECTING);

    disconnect(hardware, SIGNAL(signalConnected()), this, SLOT(slotLinkConnected()));
    disconnect(hardware, SIGNAL(signalDisconnected()), this, SLOT(slotLinkDisconnected()));
    disconnect(hardware, SIGNAL(signalRxTimeout()), this, SLOT(slotRxTimeout()));
    disconnect(hardware, SIGNAL(signalRxMsg(QString)), this, SLOT(slotRxMsg(QString)));
    disconnect(hardware, SIGNAL(signalTxMsgWritten()), this, SLOT(slotTxMsgWritten()));
    disconnect(simulator, SIGNAL(signalConnected()), this, SLOT(slotLinkConnected()));
    disconnect(simulator, SIGNAL(signalDisconnected()), this, SLOT(slotLinkDisconnected()));
    disconnect(simulator, SIGNAL(signalRxTimeout()), this, SLOT(slotRxTimeout()));
    disconnect(simulator, SIGNAL(signalRxMsg(QString)), this, SLOT(slotRxMsg(QString)));

    if (isSimulationMode)
    {
        LOG_INFO("Simulation mode is enabled\n");
        hardware->activate(false);
        connect(simulator, SIGNAL(signalConnected()), this, SLOT(slotLinkConnected()));
        connect(simulator, SIGNAL(signalDisconnected()), this, SLOT(slotLinkDisconnected()));
        connect(simulator, SIGNAL(signalRxTimeout()), this, SLOT(slotRxTimeout()));
        connect(simulator, SIGNAL(signalRxMsg(QString)), this, SLOT(slotRxMsg(QString)));
        simulator->activate(true);
    }
    else
    {
        simulator->activate(false);
        connect(hardware, SIGNAL(signalConnected()), this, SLOT(slotLinkConnected()));
        connect(hardware, SIGNAL(signalDisconnected()), this, SLOT(slotLinkDisconnected()));
        connect(hardware, SIGNAL(signalRxTimeout()), this, SLOT(slotRxTimeout()));
        connect(hardware, SIGNAL(signalRxMsg(QString)), this, SLOT(slotRxMsg(QString)));
        connect(hardware, SIGNAL(signalTxMsgWritten()), this, SLOT(slotTxMsgWritten()));
        hardware->activate(true);
    }
}

void McuLink::slotLinkDisconnect()
{
    if (isSimulationMode)
    {
        simulator->activate(false);
    }
    else
    {
        hardware->activate(false);
    }
}

void McuLink::mcuLinkInitProcess()
{
    LOG_INFO("MCU Link Init Process Started..\n");

    // MCU link is connected and active. Put all 'init commands' here
    env->ds.mcuAction->actInfo();
    env->ds.mcuAction->actSetBaseType();
    env->ds.mcuAction->actSetHwTypes();

    // Enable / disable the heat maintainer on startup
    bool heatMaintainerEnabled = env->ds.cfgGlobal->get_Settings_Injector_HeatMaintainerEnabled();
    env->ds.mcuAction->actHeatMaintainerOn(heatMaintainerEnabled ? DEFAULT_HEATER_TEMPERATURE : 0);
}

void McuLink::slotTxMsg(QString guid, QString msg, int timeoutMs)
{
    MsgInfo msgInfo;
    msgInfo.guid = guid;
    msgInfo.msg = msg;
    msgInfo.timeoutMs = timeoutMs;
    listTxMsg.append(msgInfo);

    if (!tmrTxMsgImmediate.isActive())
    {
        tmrTxMsgImmediate.start(MCU_TX_POLL_MS);
    }
}

void McuLink::slotTxMsgImmediate()
{
    tmrTxMsgImmediate.stop();

    if (isWaitingRx)
    {
        // Waiting for current reply
        LOG_DEBUG("Waiting for current reply..\n");
    }
    else
    {
        if (listTxMsg.length() > 0)
        {
            curMsgSent = listTxMsg.first();
            listTxMsg.pop_front();

            isWaitingRx = true;

            LOG_INFO("TX: %s \n", curMsgSent.msg.CSTR());
            tmrRxTimeout.start(curMsgSent.timeoutMs);

            if (isSimulationMode)
            {
                simulator->sendMsg(curMsgSent.msg);
            }
            else
            {
                hardware->sendMsg(curMsgSent.msg);
            }
        }
    }
}

void McuLink::slotHwDigestMonitor()
{
    if (!isWaitingRx)
    {
        env->ds.mcuAction->actHwDigest();
    }
}

void McuLink::slotInjectDigestMonitor()
{
    env->ds.mcuAction->actInjectDigest();
}

void McuLink::slotHeartBeatTimeout()
{
    tmrCommHeartBeat.stop();

    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus){
        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            // Link is up
        }
        else
        {
            LOG_ERROR("ACTION_STATUS: %s\n", ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());
        }
    });
    env->ds.mcuAction->actDigest(guid);

    if (listTxMsg.length() > 5)
    {
        LOG_WARNING("Too many TX items(%d) in the queue\n", (int)listTxMsg.length());
    }
}

void McuLink::slotRxTimeout()
{
    tmrRxTimeout.stop();

    LOG_ERROR("RX Timeout (%dms). CurMsg.guid=%s, msg=%s , rxMsgQueue=%s\n", curMsgSent.timeoutMs, curMsgSent.guid.CSTR(), curMsgSent.msg.CSTR(), rxMsgQueue.CSTR());
    slotLinkDisconnect();
}

void McuLink::slotLinkConnected()
{
    LOG_INFO("Connected.\n");

    listTxMsg.clear();
    isWaitingRx = false;

    rxMsgQueue = "";
    lastDigestRxReply = "";
    lastInjectDigestRxReply = "";
    lastHwDigestRxReply = "";

    // Set link status
    env->ds.mcuData->setLinkState(DS_McuDef::LINK_STATE_CONNECTING);

    // Init data before link is up
    env->ds.mcuData->setMcuVersion("");
    env->ds.mcuData->setStopcockVersion("");
    env->ds.mcuData->setMcuSerialNumber("");

    QStringList motorModuleSerialNumbers;
    for (int i = 0; i < SYRINGE_IDX_MAX; i++)
    {
        motorModuleSerialNumbers.append("unknown");
    }
    env->ds.mcuData->setMotorModuleSerialNumbers(motorModuleSerialNumbers);

    // Get version: Why two times? Because we want to flush previous junk bytes
    env->ds.mcuAction->actVersion();
    env->ds.mcuAction->actVersion();
}

void McuLink::slotLinkDisconnected()
{
    LOG_WARNING("Disconnected.\n");

    tmrCommHeartBeat.stop();
    env->ds.mcuData->setLinkState(DS_McuDef::LINK_STATE_DISCONNECTED);
}

void McuLink::slotRxMsg(QString msgRx)
{
    bool replyReceived = false;

    rxMsgQueue += msgRx;

    while (rxMsgQueue.length() > 0)
    {
        while ( (rxMsgQueue.length() > 0) &&
                (rxMsgQueue[0] != MSG_START_CHAR) )
        {
            rxMsgQueue.remove(0, 1);
        }

        if (rxMsgQueue.length() == 0)
        {
            return;
        }

        int idxStart = 0;
        int idxEnd = rxMsgQueue.indexOf(MSG_END_CHAR, idxStart);
        if (idxEnd < 0)
        {
            // message end not found.
            return;
        }

        // Prepare message
        QString msg = rxMsgQueue.mid(idxStart, idxEnd - idxStart + 1);
        rxMsgQueue = rxMsgQueue.right(rxMsgQueue.length() - msg.length());

        // Parse message
        QString id, body, reply;
        DS_McuDef::parseMsg(msg, id, body, reply);

        if (reply == "")
        {
            LOG_ERROR("Bad message, Failed to get reply. (msg=%s)\n", msg.CSTR());
            continue;
        }

        bool writeLog = true;
        bool replyNeedHandled = true;

        if (id == _L("DIGEST"))
        {
            if (lastDigestRxReply == reply)
            {
                // Commented out logging "SAME_AS_PREV" to safe log space
                //LOG_INFO("RX: [%s][%s][SAME_AS_PREV]\n", id.CSTR(), body.CSTR());
                writeLog = false;
                replyNeedHandled = false;
            }
            else
            {
                lastDigestRxReply = reply;
            }
        }
        else if (id == _L("INJECTDIGEST"))
        {
            if (lastInjectDigestRxReply == reply)
            {
                // Commented out logging "SAME_AS_PREV" to safe log space
                //LOG_INFO("RX: [%s][%s][SAME_AS_PREV]\n", id.CSTR(), body.CSTR());
                writeLog = false;
                replyNeedHandled = false;
            }
            else
            {
                lastInjectDigestRxReply = reply;
            }
        }
        else if (id == _L("HWDIGEST"))
        {
            if (lastHwDigestRxReply == reply)
            {
                // Commented out logging "SAME_AS_PREV" to safe log space
                //LOG_INFO("RX: [%s][%s][SAME_AS_PREV]\n", id.CSTR(), body.CSTR());
                writeLog = false;
                replyNeedHandled = false;
            }
            else
            {
                lastHwDigestRxReply = reply;
            }
        }

        if (writeLog)
        {
            LOG_INFO("RX: [%s][%s][%s]\n", id.CSTR(), body.CSTR(), reply.CSTR());
        }

        replyReceived = true;

        if (id == _L("VERSION"))
        {
            if (env->ds.mcuData->getLinkState() == DS_McuDef::LINK_STATE_CONNECTING)
            {
                env->ds.mcuData->setLinkState(DS_McuDef::LINK_STATE_RECOVERING);
            }
        }
        else if (id == _L("DIGEST"))
        {
            env->ds.mcuData->setLinkState(DS_McuDef::LINK_STATE_CONNECTED);
        }

        if ( (id == _L("VERSION")) ||
             (id == _L("INFO")) ||
             (id == _L("DIGEST")) ||
             (id == _L("HWDIGEST")) ||
             (id == _L("BMSDIGEST")) ||
             (id == _L("INJECTDIGEST")) ||
             (id == _L("GETSYRINGEAIRCHECKDATA")) ||
             (id == _L("GETSYRINGEAIRCHECKCOEFF")) ||
             (id == _L("GETSYRINGEAIRVOL")) ||
             (id == _L("GETPCALCOEFF")) ||
             (id == _L("GETPLUNGERFRICTION")) )
        {
            if (replyNeedHandled)
            {
                //Digest save lastMsgSentTimestamp to body for diagnostic event time calculation
                if (id == _L("DIGEST")) {
                    body = lastMsgSentTimestamp.toString(MCU_TIMESTAMP_DIGESTSENT_FORMAT);
                }
                QString err = msgHandler->handleMsg(id, body, reply);
                if (err != "")
                {
                    LOG_ERROR("Failed to parse message(msg=%s:%s, err=%s)\n", id.CSTR(), reply.CSTR(), err.CSTR());
                }
            }
        }

        tmrRxTimeout.stop();
        emit signalRxMsg(curMsgSent.guid, msg);
        curMsgSent.guid = "REPLY_RECEIVED";

        if (id == _L("DIGEST"))
        {
            DS_SystemDef::StatePath statePath = env->ds.systemData->getStatePath();
            int heartBeatIntervalMs = MCU_LINK_HEART_BEAT_INTERVAL_MS;

            if ( (statePath == DS_SystemDef::STATE_PATH_READY_ARMED) ||
                 (statePath == DS_SystemDef::STATE_PATH_EXECUTING) ||
                 (statePath == DS_SystemDef::STATE_PATH_BUSY_HOLDING) ||
                 (statePath == DS_SystemDef::STATE_PATH_BUSY_FINISHING) )
            {
                heartBeatIntervalMs = MCU_LINK_HEART_BEAT_INTERVAL_MS * 0.5;
            }

            // Continue MCU heart beat
            tmrCommHeartBeat.stop();
            tmrCommHeartBeat.start(heartBeatIntervalMs);
        }
        break;
    }

    if (replyReceived)
    {
        isWaitingRx = false;
        if (!tmrTxMsgImmediate.isActive())
        {
            tmrTxMsgImmediate.start(MCU_TX_POLL_MS);
        }
    }
}

void McuLink::slotTxMsgWritten()
{
    lastMsgSentTimestamp = QDateTime::currentDateTime();
}

