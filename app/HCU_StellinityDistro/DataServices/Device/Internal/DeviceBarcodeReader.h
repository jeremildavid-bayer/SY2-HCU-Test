#ifndef DEVICE_BARCODE_READER_H
#define DEVICE_BARCODE_READER_H

#include <QObject>
#include <QTimer>
#include <QSerialPort>
#include "Common/Common.h"
#include "Common/HwCapabilities.h"
#include "Common/ActionBase.h"
#include "DataServices/Device/DS_DeviceDef.h"

#define CR8012_CONTINUOUS_MODE_ON                   "P(C4)f0\r"
#define CR8012_CONTINUOUS_MODE_OFF                  "Q(C4)ff\r"
#define CR8012_KEEP_AWAKE                           "P(39)1\r"
#define CR8012_SUFFIX_CARRIAGE_RETURN_LINE_FEED     "P(suffix)%0D%0A\r"
#define CR8012_GET_HW_INFO                          "I\r"
#define CR8012_CLEAR_ERRORS                         ")\r"
#define CR8012_DUMMY_WAKEUP                         ";>PA7\r" // this has some functionality, but we are using this to wake up the reader

#define CR8072_CONTINUOUS_MODE_ON                   "++++CDOPSMD1\r"
#define CR8072_CONTINUOUS_MODE_OFF                  "++++CDOPSMD0\r"
#define CR8072_GET_HW_INFO                          "++++RDRRGMD\r"

class DeviceBarcodeReader : public ActionBase
{
    Q_OBJECT
public:
    explicit DeviceBarcodeReader(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~DeviceBarcodeReader();

    DataServiceActionStatus actConnect(QString actGuid = "");
    DataServiceActionStatus actDisconnect(QString actGuid = "");
    DataServiceActionStatus actStart(int sleepTimeoutMs = BARCODE_READER_SLEEP_TIMEOUT_MS, QString actGuid = "");
    DataServiceActionStatus actStop(QString actGuid = "");
    DataServiceActionStatus actSetBarcodeData(QString data, QString actGuid = "");

private:
    QSerialPort *port;
    QTimer tmrReaderTimeout;
    QTimer tmrSleepTimeout;
    QString barcodeStrBuffer;
    bool isCheckingComm;

    QString barcodeHWVersion;

    QString getPortName();
    QString connectHw(QString portName);
    bool isPortAvailable(QString portName);
    bool handleBarcodeData(QString barcodeStr);
    QString extractGs1Data(QString barcodeStr, QString *barcodePrefix, qint64 *expDateEpochMs, QString *lotBatch);

private slots:
    void slotAppInitialised();
    void slotReaderTimeout();
    void slotSleepTimeout();
    void slotRxData();
};

#endif // DEVICE_BARCODE_READER_H
