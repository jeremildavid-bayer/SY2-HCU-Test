#include <QSerialPortInfo>
#include <QProcess>
#include "McuHw.h"

McuHw::McuHw(QObject *parent, EnvGlobal *env_, EnvLocal *envLocal_) :
    QObject(parent),
    env(env_),
    envLocal(envLocal_)
{
    port = new QSerialPort(this);
    connect(port, SIGNAL(readyRead()), this, SLOT(slotPortRx()));
    connect(port, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(slotPortErr(QSerialPort::SerialPortError)));
    connect(port, SIGNAL(bytesWritten(qint64)), this, SLOT(slotPortTx(qint64)));

    connect(&tmrReconnectMcuHw, SIGNAL(timeout()), SLOT(slotConnectStart()));

    isActivated = false;
    tmrReconnectMcuHw.start(MCU_COMM_RECONNECT_TIMEOUT_MS);
}

McuHw::~McuHw()
{
    if (port->isOpen())
    {
        port->close();
    }
    port->deleteLater();
}

void McuHw::activate(bool activateHw)
{
    if (isActivated != activateHw)
    {
        isActivated = activateHw;
    }

    tmrReconnectMcuHw.stop();

    if (isActivated)
    {
        tmrReconnectMcuHw.start(MCU_COMM_RECONNECT_TIMEOUT_MS);
    }
    else
    {
        emit signalDisconnected();

        if (port->isOpen())
        {
            port->close();
        }
    }
}

void McuHw::sendMsg(QString msg)
{
    if (port->isOpen())
    {
        //LOG_DEBUG("MCU_HW: Writing to port [%s]\n", msg.CSTR());
        port->write(msg.CSTR(), msg.length());
    }
    else
    {
        LOG_WARNING("MCU_HW: QSerialPort is closed, cannot sendMsg()\n");
    }
}


void McuHw::slotConnectStart()
{
    if (!isActivated)
    {
        tmrReconnectMcuHw.stop();
        return;
    }

    if (port->isOpen())
    {
        LOG_DEBUG("MCU_HW: Close requested\n");
        port->close();
        emit signalDisconnected();
    }

    QString serialPort = "";
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    for (int i = 0; i < ports.size(); i++)
    {
        QString portName = ports.at(i).portName();
        if (portName.contains("ACM"))
        {
            QProcess procGetPortName;
            QString cmd = QString().asprintf("udevadm info -q path -n %s -a |grep 'ATTRS{product}'", portName.CSTR());
            procGetPortName.start("sh", QStringList() << "-c" << cmd);
            procGetPortName.waitForFinished(-1);
            QString out = procGetPortName.readAllStandardOutput();
            if (out.contains(MCU_FTDI_DESCRIPTION))
            {
                serialPort = portName;
                break;
            }
        }
    }

    if (serialPort == "")
    {
        LOG_WARNING("MCU_HW: Unabled to find serial port with description(%s)\n", MCU_FTDI_DESCRIPTION);
        emit signalDisconnected();
    }
    else
    {
        LOG_INFO("MCU_HW: connecting to serial port (%s)..\n", serialPort.CSTR());

        port->setPortName(serialPort);
        port->setBaudRate(QSerialPort::Baud115200);
        port->setFlowControl(QSerialPort::NoFlowControl);
        port->setParity(QSerialPort::NoParity);
        port->setDataBits(QSerialPort::Data8);
        port->setStopBits(QSerialPort::OneStop);

        if (port->open(QIODevice::ReadWrite))
        {
            port->clear();

            LOG_INFO("MCU_HW: Connected\n");
            tmrReconnectMcuHw.stop();

            emit signalConnected();
        }
        else
        {
            LOG_ERROR("MCU_HW: Failed to open port(err=%s)\n", port->errorString().CSTR());
            emit signalDisconnected();
        }
    }
}

void McuHw::slotPortRx()
{
    if (port->isOpen())
    {
        QString rxMsg = port->readAll();
        emit signalRxMsg(rxMsg);
    }
}

void McuHw::slotPortErr(QSerialPort::SerialPortError err)
{//Handle QSerialPort::errorOccurred signal
    tmrReconnectMcuHw.stop();
    QString errorMsg = port->errorString();

    if (err == QSerialPort::NoError)
    {
        return;
    }

    emit signalDisconnected();

    if ( (errorMsg.contains("Resource")) ||
         (errorMsg.contains("Serial Error")) ||
         (errorMsg.contains("Device or resource busy")) ||
         (errorMsg.contains("Unknown error")) )
    {
        LOG_ERROR("MCU_HW: Socket Error(%s)\n", errorMsg.CSTR());
        port->clearError();
        if (port->isOpen())
        {
            port->close();
        }

        tmrReconnectMcuHw.start(MCU_COMM_RECONNECT_TIMEOUT_MS);
    }
    else
    {
        LOG_ERROR("MCU_HW: Serial Port Error(%s)\n", errorMsg.CSTR());
    }
}

void McuHw::slotPortTx(qint64 bytes)
{
    if (port->isOpen())
    {
        emit signalTxMsgWritten();
    }
}
