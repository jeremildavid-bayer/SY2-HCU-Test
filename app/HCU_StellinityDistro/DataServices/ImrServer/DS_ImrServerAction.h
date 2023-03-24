#ifndef DS_IMR_SERVER_ACTION_H
#define DS_IMR_SERVER_ACTION_H

#include "DS_ImrServerDef.h"
#include "Common/Common.h"
#include "Common/ActionBase.h"
#include "Internal/ImrServerWrapper.h"
#include "Internal/ImrServerWebSocket.h"

class DS_ImrServerAction : public ActionBase
{
    Q_OBJECT

public:
    explicit DS_ImrServerAction(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~DS_ImrServerAction();

    DataServiceActionStatus actUpdateDataGroup(QString type, QVariant data = QVariant(), QString actGuid = "");

private:
    ImrServerWrapper *serverWrapper;
#ifdef ENABLE_IMR_WEB_SOCKET_SERVER
    ImrServerWebSocket *webSocketServer;
#endif //ENABLE_IMR_WEB_SOCKET_SERVER

private slots:
    void slotAppInitialised();
};

#endif // DS_IMR_SERVER_ACTION_H
