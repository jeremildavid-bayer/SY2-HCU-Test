#ifndef QML_DEVICE_H
#define QML_DEVICE_H

#include "Common/Common.h"
#include "DataServices/Device/DS_DeviceDef.h"

class QML_Device : public QObject
{
    Q_OBJECT
public:
    explicit QML_Device(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~QML_Device();

    Q_INVOKABLE void slotStopcockFillPosition(QString syringeIdxStr);
    Q_INVOKABLE void slotStopcockInjectPosition(QString syringeIdxStr);
    Q_INVOKABLE void slotStopcockClosePosition(QString syringeIdxStr);
    Q_INVOKABLE void slotBarcodeReaderStart(int sleepTimeoutMs);
    Q_INVOKABLE void slotBarcodeReaderStop();
    Q_INVOKABLE void slotBarcodeReaderConnect();
    Q_INVOKABLE void slotBarcodeReaderDisconnect();
    Q_INVOKABLE void slotBarcodeReaderSetBarcodeData(QString data);
    Q_INVOKABLE void slotSyringeAirCheck(QString syringeIdxStr);
    Q_INVOKABLE bool slotGetSyringeSodStartReady(QString syringeIdxStr);

private:
    EnvGlobal *env;
    EnvLocal *envLocal;
    QObject *qmlSrc;
};

#endif // QML_DEVICE_H
