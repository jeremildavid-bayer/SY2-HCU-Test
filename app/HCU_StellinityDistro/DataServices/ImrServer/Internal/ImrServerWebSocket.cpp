#include "Apps/AppManager.h"
#include "ImrServerWebSocket.h"
#include "DataServices/ImrServer/DS_ImrServerData.h"
#include "DataServices/ImrServer/DS_ImrServerAction.h"
#include "DataServices/Capabilities/DS_Capabilities.h"
#include "DataServices/Alert/DS_AlertData.h"
#include "DataServices/CfgLocal/DS_CfgLocal.h"
#include "DataServices/CfgGlobal/DS_CfgGlobal.h"
#include "DataServices/HardwareInfo/DS_HardwareInfo.h"

ImrServerWebSocket::ImrServerWebSocket(QObject *parent, EnvGlobal *env_) :
    QObject(parent),
    env(env_)
{
    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });

    envLocal = new EnvLocal("DS_ImrServer-WebSocket", "IMR_SERVER_WEB_SOCKET");

    server = new QWebSocketServer("IMR WebSocket Server", QWebSocketServer::NonSecureMode, this);

    connect(server, &QWebSocketServer::newConnection, this, &ImrServerWebSocket::slotClientConnected);
    connect(server, &QWebSocketServer::closed, this, &ImrServerWebSocket::slotClosed);
}

ImrServerWebSocket::~ImrServerWebSocket()
{
    server->close();
    qDeleteAll(clients.begin(), clients.end());
    delete envLocal;
    delete server;
}

void ImrServerWebSocket::slotAppInitialised()
{
    connect(env->ds.capabilities, &DS_Capabilities::signalConfigChanged_General_WebApplicationEnabled, this, [=](const Config::Item &cfg) {
        bool webApplicationMode = cfg.value.toBool();

        if (webApplicationMode)
        {
            if (!server->isListening())
            {
                start();
            }
        }
        else
        {
            stop();
        }
    });

    connect(env->ds.alertData, &DS_AlertData::signalDataChanged_ActiveAlerts, this, [=](const QVariantList &alerts) {
        if (env->ds.capabilities->get_General_WebApplicationEnabled())
        {
            env->ds.imrServerAction->actUpdateDataGroup("ActiveAlerts", alerts);
        }
    });

    connect(env->ds.alertData, &DS_AlertData::signalDataChanged_ActiveSystemAlerts, this, [=](const QVariantList &alerts) {
        if (env->ds.capabilities->get_General_WebApplicationEnabled())
        {
            env->ds.imrServerAction->actUpdateDataGroup("ActiveSystemAlerts", alerts);
        }
    });

    connect(env->ds.alertData, &DS_AlertData::signalDataChanged_ActiveFluidSourceAlerts, this, [=](const QVariantMap &alertMap) {
        if (env->ds.capabilities->get_General_WebApplicationEnabled())
        {
            env->ds.imrServerAction->actUpdateDataGroup("ActiveFluidSourceAlerts", alertMap);
        }
    });

    connect(env->ds.cfgGlobal, &DS_CfgGlobal::signalConfigChanged, this, [=]() {
        if (env->ds.capabilities->get_General_WebApplicationEnabled())
        {
            QVariantMap configs = env->ds.cfgGlobal->getConfigs();
            env->ds.imrServerAction->actUpdateDataGroup("GlobalConfigs", configs);
        }
    });

    connect(env->ds.cfgLocal, &DS_CfgLocal::signalConfigChanged, this, [=]() {
        if (env->ds.capabilities->get_General_WebApplicationEnabled())
        {
            QVariantMap configs = env->ds.cfgLocal->getConfigs();
            env->ds.imrServerAction->actUpdateDataGroup("LocalConfigs", configs);
        }
    });

    connect(env->ds.hardwareInfo, &DS_HardwareInfo::signalConfigChanged, this, [=]() {
        if (env->ds.capabilities->get_General_WebApplicationEnabled())
        {
            QVariantMap configs = env->ds.hardwareInfo->getConfigs();
            env->ds.imrServerAction->actUpdateDataGroup("HwInfo", configs);
        }
    });

    connect(env->ds.capabilities, &DS_Capabilities::signalConfigChanged, this, [=]() {
        if (env->ds.capabilities->get_General_WebApplicationEnabled())
        {
            QVariantMap configs = env->ds.capabilities->getConfigs();
            env->ds.imrServerAction->actUpdateDataGroup("Capabilities", configs);
        }
    });

    // Sends first subscription message to client
    env->ds.imrServerAction->actUpdateDataGroup("ActiveAlerts", env->ds.alertData->getActiveAlerts());
    env->ds.imrServerAction->actUpdateDataGroup("ActiveSystemAlerts", env->ds.alertData->getActiveSystemAlerts());
    env->ds.imrServerAction->actUpdateDataGroup("ActiveFluidSourceAlerts", env->ds.alertData->getActiveFluidSourceAlerts());
    env->ds.imrServerAction->actUpdateDataGroup("GlobalConfigs", env->ds.cfgGlobal->getConfigs());
    env->ds.imrServerAction->actUpdateDataGroup("LocalConfigs", env->ds.cfgLocal->getConfigs());
    env->ds.imrServerAction->actUpdateDataGroup("HwInfo", env->ds.hardwareInfo->getConfigs());
    env->ds.imrServerAction->actUpdateDataGroup("Capabilities", env->ds.capabilities->getConfigs());
}

void ImrServerWebSocket::start()
{
    stop();

    if (env->ds.capabilities->get_General_WebApplicationEnabled())
    {
        int portNumber = env->ds.cfgLocal->get_Hidden_HcuPort() + 1;
        if (server->listen(QHostAddress::Any, portNumber))
        {
            LOG_INFO("Server listening on port %d...\n", portNumber);
        }
        else
        {
            LOG_ERROR("Server listening on port %d failed: %s\n", portNumber, server->errorString().CSTR());
        }
    }
}

void ImrServerWebSocket::stop()
{
    if (server->isListening())
    {
        // Close connection first. When closed, start() will be called
        server->close();
    }
}

void ImrServerWebSocket::slotClosed()
{
    if (env->state == EnvGlobal::STATE_EXITING)
    {
        return;
    }

    LOG_INFO("Server Closed.\n");

    if (env->ds.capabilities->get_General_WebApplicationEnabled())
    {
        LOG_INFO("Server closed unexpectedly. Starting server again..\n");
        start();
    }
}

void ImrServerWebSocket::slotClientConnected()
{
    QWebSocket *pSocket = server->nextPendingConnection();

    // check if client already connected
    foreach (QWebSocket *pClient, clients)
    {
        if (pClient != NULL)
        {
            if ( (pClient->peerAddress() == pSocket->peerAddress()) &&
                 (pClient->peerPort() == pSocket->peerPort()) )
            {
                // client already connected, ignore
                return;
            }
        }
    }

    connect(pSocket, &QWebSocket::textMessageReceived, this, &ImrServerWebSocket::slotRxMessage);
    connect(pSocket, &QWebSocket::binaryMessageReceived, this, &ImrServerWebSocket::slotRxBinMessage);
    connect(pSocket, &QWebSocket::disconnected, this, &ImrServerWebSocket::socketDisconnected);

    clients << pSocket;

    env->ds.imrServerData->setClientCount(clients.length());

    LOG_DEBUG("New Connection[%d]: %s%s:%u\n",
              (int)clients.length() - 1,
              pSocket->peerName().CSTR(),
              pSocket->peerAddress().toString().CSTR(),
              pSocket->peerPort());

    // Sends all data items to client
    env->ds.imrServerAction->actUpdateDataGroup("");
}

void ImrServerWebSocket::txDataGroup(QString type)
{
    QVariantMap dataGroup = env->ds.imrServerData->getDataGroup();
    QVariantMap dataGroup2;

    if (type == "")
    {
        // Send all
        dataGroup2 = dataGroup;
    }
    else if (dataGroup.contains(type))
    {
        dataGroup2.insert(type, dataGroup[type].toMap());
    }

    QVariantMap messageMap;
    messageMap.insert("type", "DataGroup");
    messageMap.insert("from", "HCU");
    messageMap.insert("data", dataGroup2);
    QString payload = Util::qVarientToJsonData(messageMap);

    //LOG_DEBUG("txDataGroup(): TX: %s\n", payload.CSTR());

    foreach (QWebSocket *pClient, clients)
    {
        if (pClient != NULL)
        {
            pClient->sendTextMessage(payload);
        }
    }
}

void ImrServerWebSocket::slotRxMessage(QString message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    LOG_DEBUG("Message received: %s\n", message.CSTR());

    if (pClient) {
        pClient->sendTextMessage(message);
    }
}

void ImrServerWebSocket::slotRxBinMessage(QByteArray message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());

    LOG_DEBUG("Binary Message received: %s\n", message.toHex().CSTR());

    if (pClient) {
        pClient->sendBinaryMessage(message);
    }
}

void ImrServerWebSocket::socketDisconnected()
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());

    if (pClient)
    {
        clients.removeAll(pClient);
        env->ds.imrServerData->setClientCount(clients.length());
        pClient->deleteLater();
        LOG_DEBUG("Socket Disconnected[%d]: %s%s:%u\n",
                  (int)clients.length(),
                  pClient->peerName().CSTR(),
                  pClient->peerAddress().toString().CSTR(),
                  pClient->peerPort());
    }
}
