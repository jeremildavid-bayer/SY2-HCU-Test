#ifndef CRU_LINK_MSG_COMM_H
#define CRU_LINK_MSG_COMM_H

#include <QNetworkAccessManager>
#include <QTimer>
#include "Common/Common.h"
#include "Common/HMAC.h"

class CruLinkMsgComm : public QObject
{
    Q_OBJECT
public:
    explicit CruLinkMsgComm(QObject *parent, EnvGlobal *env_, EnvLocal *envLocal_, QString guid, DataServiceActionStatus actStatus, int timeoutMs);
    ~CruLinkMsgComm();

    DataServiceActionStatus getActStatus() { return actStatus; }
    void start();

private:
    EnvGlobal *env;
    EnvLocal *envLocal;
    QString guid;
    DataServiceActionStatus actStatus;
    int timeoutMs;
    QNetworkAccessManager *nam;
    QTimer tmrRxTimeout;

private slots:
    void slotFinished(QNetworkReply *reply);
    void slotRxTimeout();
};

#endif // CRU_LINK_MSG_COMM_H
