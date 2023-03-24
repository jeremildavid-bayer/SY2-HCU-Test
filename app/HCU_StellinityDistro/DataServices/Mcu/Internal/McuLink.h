#ifndef MCU_LINK_H
#define MCU_LINK_H

#include <QObject>
#include <QTimer>
#include <QMutex>
#include <QDateTime>
#include "Common/Common.h"
#include "McuMsgHandler.h"
#include "McuHw/McuHw.h"
#include "McuSim/McuSim.h"

class McuLink : public QObject
{
    Q_OBJECT
public:
    explicit McuLink(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~McuLink();
    McuMsgHandler *msgHandler;

private:
    struct MsgInfo
    {
        QString msg;
        QString guid;
        int timeoutMs;
    };

    EnvGlobal *env;
    EnvLocal *envLocal;

    McuSim *simulator;
    McuHw *hardware;

    QTimer tmrCommHeartBeat;
    QTimer tmrHwDigestMonitor;
    QTimer tmrInjectDigestMonitor;
    QTimer tmrRxTimeout;
    QTimer tmrTxMsgImmediate;

    bool isSimulationMode;
    QList<MsgInfo> listTxMsg;

    bool isWaitingRx;
    MsgInfo curMsgSent;
    QString rxMsgQueue;
    QDateTime lastMsgSentTimestamp;

    QString lastDigestRxReply;
    QString lastInjectDigestRxReply;
    QString lastHwDigestRxReply;

    void mcuLinkInitProcess();

private slots:
    void slotAppInitialised();
    void slotLinkConnect();
    void slotLinkDisconnect();
    void slotLinkConnected();
    void slotLinkDisconnected();
    void slotRxMsg(QString msg);
    void slotHeartBeatTimeout();
    void slotHwDigestMonitor();
    void slotInjectDigestMonitor();
    void slotRxTimeout();
    void slotTxMsg(QString guid, QString msg, int timeoutMs);
    void slotTxMsgImmediate();
    void slotTxMsgWritten();

signals:
    void signalLinkConnect();
    void signalLinkDisconnect();
    void signalRxMsg(QString guid, QString rxMsg);
    void signalTxMsg(QString guid, QString msg, int timeoutMs);
};

#endif // MCU_LINK_H
