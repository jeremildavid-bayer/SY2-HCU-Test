#include "Apps/AppManager.h"
#include "DS_ImrServerAction.h"
#include "DataServices/ImrServer/DS_ImrServerData.h"
#include "DataServices/Alert/DS_AlertData.h"

DS_ImrServerAction::DS_ImrServerAction(QObject *parent, EnvGlobal *env_) :
    ActionBase(parent, env_)
{
    envLocal = new EnvLocal("DS_ImrServer-Action", "IMR_SERVER_ACTION", LOG_MID_SIZE_BYTES);

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });

    serverWrapper = new ImrServerWrapper(this, env);
#ifdef ENABLE_IMR_WEB_SOCKET_SERVER
    webSocketServer = new ImrServerWebSocket(this, env);
#endif //ENABLE_IMR_WEB_SOCKET_SERVER
}

DS_ImrServerAction::~DS_ImrServerAction()
{
    delete envLocal;
    delete serverWrapper;
#ifdef ENABLE_IMR_WEB_SOCKET_SERVER
    delete webSocketServer;
#endif
}

void DS_ImrServerAction::slotAppInitialised()
{

}

DataServiceActionStatus DS_ImrServerAction::actUpdateDataGroup(QString type, QVariant data, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "actUpdateDataGroup");

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);

    // Update the Data Group
    QVariantMap dataGroup = env->ds.imrServerData->getDataGroup();

    if (type != "")
    {
        QVariantMap dataItem;
        dataItem.insert("UpdatedAt", Util::qDateTimeToUtcDateTimeStr(QDateTime::currentDateTimeUtc()));
        dataItem.insert("Data", data);
        dataGroup.insert(type, dataItem);
        env->ds.imrServerData->setDataGroup(dataGroup);
    }

#ifdef ENABLE_IMR_WEB_SOCKET_SERVER
    // Transmit to WebSocket
    webSocketServer->txDataGroup(type);
#endif //ENABLE_IMR_WEB_SOCKET_SERVER

    status.state = DS_ACTION_STATE_COMPLETED;
    actionStarted(status);
    return status;
}

