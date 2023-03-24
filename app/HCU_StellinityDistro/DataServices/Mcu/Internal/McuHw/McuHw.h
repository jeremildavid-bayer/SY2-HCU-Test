#ifndef MCU_HW_H
#define MCU_HW_H

#include <QObject>
#include <QSerialPort>
#include <QTimer>
#include "Common/Common.h"

class McuHw : public QObject
{
    Q_OBJECT
public:
    explicit McuHw(QObject *parent = 0, EnvGlobal *env_ = NULL, EnvLocal *envLocal_ = NULL);
    ~McuHw();
    void activate(bool activateHw);
    void sendMsg(QString msg);
private:
    EnvGlobal *env;
    EnvLocal *envLocal;

    QSerialPort *port;
    QTimer tmrReconnectMcuHw;
    QTimer tmrRxTimeout;
    bool isActivated;

signals:
    void signalConnected();
    void signalDisconnected();
    void signalRxTimeout();
    void signalRxMsg(QString msg);
    void signalTxMsgWritten();

private slots:
    void slotPortRx();
    void slotPortErr(QSerialPort::SerialPortError err);
    void slotConnectStart();
    void slotPortTx(qint64 bytes);

};

#endif // MCU_HW_H
