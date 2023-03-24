#ifndef IMR_SERVER_WEB_SOCKET_H
#define IMR_SERVER_WEB_SOCKET_H

#include "Common/Common.h"
#include "QtWebSockets/qwebsocketserver.h"
#include "QtWebSockets/qwebsocket.h"

class ImrServerWebSocket : public QObject
{
    Q_OBJECT
public:
    explicit ImrServerWebSocket(QObject *parent = NULL, EnvGlobal *env = NULL);
    ~ImrServerWebSocket();

    void txDataGroup(QString type);

private:
    EnvGlobal *env;
    EnvLocal *envLocal;
    QWebSocketServer *server;
    QList<QWebSocket *> clients;

    void start();
    void stop();

private slots:
    void slotClientConnected();
    void slotRxMessage(QString message);
    void slotRxBinMessage(QByteArray message);
    void socketDisconnected();
    void slotClosed();
    void slotAppInitialised();

};

#endif //IMR_SERVER_WEB_SOCKET_H
