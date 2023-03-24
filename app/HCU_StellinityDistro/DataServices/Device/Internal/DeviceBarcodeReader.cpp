#include <QSerialPortInfo>
#include "Apps/AppManager.h"
#include "DeviceBarcodeReader.h"
#include "DataServices/Workflow/DS_WorkflowData.h"
#include "DataServices/Device/DS_DeviceData.h"
#include "DataServices/Alert/DS_AlertAction.h"
#include "Common/Util.h"

DeviceBarcodeReader::DeviceBarcodeReader(QObject *parent, EnvGlobal *env_) :
    ActionBase(parent, env_)
{
    envLocal = new EnvLocal("DS_Device-BarcodeReader", "DEVICE_BARCODE_READER", LOG_MID_SIZE_BYTES);
    isCheckingComm = false;

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });

    connect(&tmrReaderTimeout, SIGNAL(timeout()), this, SLOT(slotReaderTimeout()));
    connect(&tmrSleepTimeout, SIGNAL(timeout()), this, SLOT(slotSleepTimeout()));

    port = new QSerialPort(this);
    connect(port, SIGNAL(readyRead()), this, SLOT(slotRxData()));

    barcodeHWVersion = "";
}

DeviceBarcodeReader::~DeviceBarcodeReader()
{
    tmrReaderTimeout.stop();
    tmrSleepTimeout.stop();
    if (port->isOpen())
    {
        port->close();
    }
    delete port;
    delete envLocal;
}

void DeviceBarcodeReader::slotAppInitialised()
{
    connect(env->ds.systemData, &DS_SystemData::signalDataChanged_StatePath, this, [=](DS_SystemDef::StatePath curStatePath, DS_SystemDef::StatePath prevStatePath) {
        if ( ( (prevStatePath == DS_SystemDef::STATE_PATH_IDLE) && (curStatePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING) ) ||
             ( (prevStatePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING) && (curStatePath == DS_SystemDef::STATE_PATH_IDLE) ) )
        {
            LOG_INFO("\n\n-----------------\nStatePath=%s\n\n", ImrParser::ToImr_StatePath(curStatePath).CSTR());
        }
    });

    //Init COM port
    port->setBaudRate(QSerialPort::Baud115200);
    port->setDataBits(QSerialPort::Data8);
    port->setFlowControl(QSerialPort::NoFlowControl);
    port->setParity(QSerialPort::NoParity);
    port->setStopBits(QSerialPort::OneStop);

    actConnect();

    // actConnect will take some time to figure out HW version. Stop after some seconds
    QTimer::singleShot(BARCODE_READER_TEST_MSG_RX_TIMEOUT_MS, this, [=] {
        actStop();
    });
}

DataServiceActionStatus DeviceBarcodeReader::actConnect(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "Connect");

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);

    QString portName = getPortName();
    if (portName != "")
    {
        status.err = connectHw(portName);
        if (status.err == "")
        {
            status.state = DS_ACTION_STATE_COMPLETED;
        }
        else
        {
            status.state = DS_ACTION_STATE_INTERNAL_ERR;
        }
    }
    else
    {
        status.err = "Port Not Found";
        status.state = DS_ACTION_STATE_INTERNAL_ERR;
    }

    actionCompleted(status);
    return status;
}

DataServiceActionStatus DeviceBarcodeReader::actDisconnect(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "Disconnect");

    actionStarted(status);

    tmrReaderTimeout.stop();
    tmrSleepTimeout.stop();

    if ( (port != NULL) &&
         (port->isOpen()) )
    {
        DataServiceActionStatus actStopStatus = actStop();
        if (actStopStatus.state != DS_ACTION_STATE_COMPLETED)
        {
            status.state = DS_ACTION_STATE_INTERNAL_ERR;
            status.err = "Stop Failed";
            actionCompleted(status);
            return status;
        }

        port->close();
        env->ds.deviceData->setBarcodeReaderConnected(false);
        LOG_INFO("actDisconnect(): Disconnected\n");
    }
    else
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "Port Not Open";
    }

    actionCompleted(status);
    return status;
}

DataServiceActionStatus DeviceBarcodeReader::actStart(int sleepTimeoutMs, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "Start", QString().asprintf("%d", sleepTimeoutMs));

    actionStarted(status);

    if (port == NULL)
    {
        status.err = "Port Not Initialised";
        status.state = DS_ACTION_STATE_INVALID_STATE;
        actionCompleted(status);
        return status;
    }

    if (port->isOpen())
    {
        if (barcodeHWVersion == "8012")
        {
            port->write(CR8012_CONTINUOUS_MODE_ON); // Continuous mode on
            port->waitForBytesWritten(BARCODE_READER_BYTES_WRITTEN_TIMEOUT_MS);

            port->write(CR8012_KEEP_AWAKE); // Trigger to read with keep awake setting
            port->waitForBytesWritten(BARCODE_READER_BYTES_WRITTEN_TIMEOUT_MS);

            port->write(CR8012_SUFFIX_CARRIAGE_RETURN_LINE_FEED); // Suffix - Carriage Return Line Feed
            port->waitForBytesWritten(BARCODE_READER_BYTES_WRITTEN_TIMEOUT_MS);

            port->write(CR8012_CLEAR_ERRORS); // Clear any errors
            port->waitForBytesWritten(BARCODE_READER_BYTES_WRITTEN_TIMEOUT_MS);

            barcodeStrBuffer.clear();
        }
        else if (barcodeHWVersion == "8072")
        {
            port->write(CR8072_CONTINUOUS_MODE_ON);
            port->waitForBytesWritten(BARCODE_READER_BYTES_WRITTEN_TIMEOUT_MS);
            barcodeStrBuffer.clear();
        }

        tmrSleepTimeout.stop();
        if (sleepTimeoutMs > 0)
        {
            tmrSleepTimeout.start(sleepTimeoutMs);
        }
    }
    else
    {
        status.err = "Port Not Open";
        status.state = DS_ACTION_STATE_INVALID_STATE;
    }

    actionCompleted(status);
    return status;
}

DataServiceActionStatus DeviceBarcodeReader::actStop(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "Stop");

    actionStarted(status);

    tmrSleepTimeout.stop();
    if (port == NULL)
    {
        status.err = "Port Not Initialised";
        status.state = DS_ACTION_STATE_INVALID_STATE;
        actionCompleted(status);
        return status;
    }

    if (port->isOpen())
    {
        LOG_INFO("actStop(): Turning off\n");

        if (barcodeHWVersion == "8012")
        {
            port->write(CR8012_CONTINUOUS_MODE_OFF); // Continuous mode off
            port->waitForBytesWritten(BARCODE_READER_BYTES_WRITTEN_TIMEOUT_MS);

            port->write(CR8012_CONTINUOUS_MODE_OFF); // Continuous mode off
            port->waitForBytesWritten(BARCODE_READER_BYTES_WRITTEN_TIMEOUT_MS);
            barcodeStrBuffer.clear();

            // Do a comm check when the reader is told to stop
            port->write(CR8012_GET_HW_INFO); //Get info string
            port->waitForBytesWritten(BARCODE_READER_BYTES_WRITTEN_TIMEOUT_MS);
            port->waitForReadyRead(BARCODE_READER_BYTES_WRITTEN_TIMEOUT_MS);
        }
        else if (barcodeHWVersion == "8072")
        {
            port->write(CR8072_CONTINUOUS_MODE_OFF);
            port->waitForBytesWritten(BARCODE_READER_BYTES_WRITTEN_TIMEOUT_MS);

            barcodeStrBuffer.clear();
            port->write(CR8072_GET_HW_INFO); //Get info string
            port->waitForBytesWritten(BARCODE_READER_BYTES_WRITTEN_TIMEOUT_MS);
            port->waitForReadyRead(BARCODE_READER_BYTES_WRITTEN_TIMEOUT_MS);
        }
        isCheckingComm = true;
        tmrReaderTimeout.start(BARCODE_READER_TEST_MSG_RX_TIMEOUT_MS);
    }
    else
    {
        status.err = "Port Not Open";
        status.state = DS_ACTION_STATE_INVALID_STATE;
    }

    actionCompleted(status);
    return status;
}

DataServiceActionStatus DeviceBarcodeReader::actSetBarcodeData(QString data, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "SetBarcodeData", data);

    actionStarted(status);

    handleBarcodeData(data);

    actionCompleted(status);
    return status;
}


QString DeviceBarcodeReader::connectHw(QString portName)
{
    QString err = "";

    if (port)
    {
        if (isPortAvailable(portName))
        {
            port->setPortName(portName);
            if (port->isOpen())
            {
                LOG_DEBUG("connectHw(): Already Connected\n");
            }
            else if (port->open(QSerialPort::ReadWrite))
            {
                LOG_INFO("connectHw(): Connected to port '%s'\n", portName.CSTR());
                isCheckingComm = true;
                // Detect which HW type it is by querying info strnig for both versions

                // HW CR8012 type
                port->write(CR8012_DUMMY_WAKEUP); // wake up barcode reader.. otherwise it doesn't respond
                port->waitForBytesWritten(BARCODE_READER_BYTES_WRITTEN_TIMEOUT_MS);
                port->waitForReadyRead(BARCODE_READER_BYTES_WRITTEN_TIMEOUT_MS);

                port->write(CR8012_GET_HW_INFO); // get info string
                port->waitForBytesWritten(BARCODE_READER_BYTES_WRITTEN_TIMEOUT_MS);
                port->waitForReadyRead(BARCODE_READER_TEST_MSG_RX_TIMEOUT_MS);

                // HW CR8072 type
                port->write(CR8072_GET_HW_INFO); // get info string
                port->waitForBytesWritten(BARCODE_READER_BYTES_WRITTEN_TIMEOUT_MS);
                port->waitForReadyRead(BARCODE_READER_BYTES_WRITTEN_TIMEOUT_MS);

                tmrReaderTimeout.start(BARCODE_READER_TEST_MSG_RX_TIMEOUT_MS);
            }
            else
            {
                LOG_ERROR("connectHw(): Connect Failed: Port='%s'\n", portName.CSTR());
                err = "Connect Failed";
            }
        }
        else
        {
            LOG_ERROR("connectHw(): Port Not Found: Port='%s'\n", portName.CSTR());
            err = "Port Not Found";
        }
    }

    return err;
}

QString DeviceBarcodeReader::getPortName()
{
    QString portName = "";
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    for (int i = 0; i < ports.size(); i++)
    {
        QString tempPortName = ports.at(i).portName();
        if (tempPortName.contains(BARCODE_READER_PORT_NAME_PART))
        {
            portName = tempPortName;
            LOG_DEBUG("getPortName(): Port Found: '%s'\n", portName.CSTR());
            break;
        }
    }
    return portName;
}

bool DeviceBarcodeReader::isPortAvailable(QString portName)
{
    bool ret = false;
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    for (int i = 0; i < ports.size(); i++)
    {
        QString tempPortName = ports.at(i).portName();
        if (tempPortName.contains(portName))
        {
            ret = true;
            break;
        }
    }
    return ret;
}

void DeviceBarcodeReader::slotReaderTimeout()
{
    LOG_ERROR("slotReaderTimeout(): barcode reader response timeout.\n");
    tmrReaderTimeout.stop();
    env->ds.alertAction->activate("BarcodeReaderFault");
    isCheckingComm = false;
    // version check may have appended to this buffer. Clear if it timed out
    barcodeStrBuffer.clear();
}

void DeviceBarcodeReader::slotSleepTimeout()
{
    LOG_INFO("slotSleepTimeout(): Barcode Sleep timeout. Barcode will turn off\n");
    actStop();
}

void DeviceBarcodeReader::slotRxData()
{
    QString data = QString(port->readAll());
    LOG_DEBUG("slotRxData(): RX=[%s]\n", data.CSTR());

    if (isCheckingComm)
    {
        LOG_DEBUG("DETECTING BARCODE HW TYPE...\n");
        tmrReaderTimeout.stop();

        barcodeStrBuffer.append(data);

        // checking for CR8012 type. This is return from "I\r"
        // CR8012 "I/r" return looks like: ap/i12621262none0020689694A0600000080006001700730002	cd(16.1.35)
        // where i1262 from above indicates the firmware version. However other firmware versions also have been identified (eg, i1306)
        // the version string format is completely different to CR8072 so we use very generic check
        // that's why we are not specifically checking for version in string eg)
        //           if (barcodeStrBuffer.contains("i1306"))
        if (barcodeStrBuffer.contains("cd("))
        {
            barcodeHWVersion = "8012";
            isCheckingComm = false;
            // There is no carriage return. We store to buffer until we know the HW version
            barcodeStrBuffer.clear();
        }
        // checking for CR8072 type. This is return from "++++RDRRGMD\r"
        else if (barcodeStrBuffer.contains("CR8072"))
        {
            barcodeHWVersion = "8072";
            isCheckingComm = false;
            // There is no carriage return. We store to buffer until we know the HW version
            barcodeStrBuffer.clear();
        }

        if (barcodeHWVersion != "")
        {
            env->ds.deviceData->setBarcodeReaderConnected(true);
            env->ds.alertAction->deactivate("BarcodeReaderFault");
            LOG_DEBUG("BARCODE HW DETECTED : %s\n", barcodeHWVersion.CSTR());
        }
        return;
    }

    if (!data.contains("\r"))
    {
        barcodeStrBuffer.append(data);
    }
    else
    {
        barcodeStrBuffer.append(data.left(data.indexOf('\r')));
        barcodeStrBuffer.replace("\n", "");
        barcodeStrBuffer.replace("\r", "");

        if ( (barcodeStrBuffer.contains("failed", Qt::CaseInsensitive)) ||
             (barcodeStrBuffer.length() == 0) )
        {
            port->flush();
            actDisconnect();
            actConnect();
        }
        else
        {
            handleBarcodeData(barcodeStrBuffer);
            actStop();
        }
        barcodeStrBuffer.clear();
    }
}

//===============================================
// Barcode Parsing Functions
//===============================================
bool DeviceBarcodeReader::handleBarcodeData(QString barcodeStr)
{
    DS_DeviceDef::BarcodeInfo barcodeInfo;
    DS_DeviceDef::FluidOptions fluidOptions = env->ds.deviceData->getFluidOptions();
    qint64 expDateEpochMs;
    QString tempLotBatch;

    barcodeInfo.scannedAtEpochMs = QDateTime::currentMSecsSinceEpoch();
    barcodeInfo.isFluidPackageOk = false;
    barcodeInfo.isKnownBarcode = false;

    QString extractErr = extractGs1Data(barcodeStr, &barcodeInfo.barcodePrefix, &expDateEpochMs, &tempLotBatch);

    LOG_DEBUG("handleBarcodeData(): barcodeStr=%s\n", barcodeStr.CSTR());

    if (extractErr != "")
    {
        LOG_DEBUG("handleBarcodeData(): Non GS1 Barcode Scanned\n");
        barcodeInfo.barcodePrefix = barcodeStr;

        if (barcodeInfo.barcodePrefix.length() > BARCODE_PREFIX_LEN_MAX)
        {
            barcodeInfo.err = QString().asprintf("Barcode Prefix length(%d) > %d", (int)barcodeInfo.barcodePrefix.length(), BARCODE_PREFIX_LEN_MAX);
            LOG_WARNING("handleBarcodeData(): Invalid Barcode: %s\n", barcodeInfo.err.CSTR());
            goto bail;
        }
    }

    // Scan Known barcodes
    for (int barcodePrefixIdx = 0; barcodePrefixIdx < fluidOptions.knownBarcodes.length(); barcodePrefixIdx++)
    {
        if (fluidOptions.knownBarcodes[barcodePrefixIdx].barcode == barcodeInfo.barcodePrefix)
        {
            LOG_INFO("handleBarcodeData(): BarcodePrefix found from fluidOptions.knownBarcodes[%d].barcode\n", barcodePrefixIdx);
            barcodeInfo.isFluidPackageOk = true;
            barcodeInfo.isKnownBarcode = true;
            barcodeInfo.fluidPackage = fluidOptions.knownBarcodes[barcodePrefixIdx].fluidPackage;
            break;
        }
    }

    // Scan System Fluid Options
    for (int familyIdx = 0; familyIdx < fluidOptions.contrastFamilies.length(); familyIdx++)
    {
        DS_DeviceDef::FluidFamily fluidFamily = fluidOptions.contrastFamilies[familyIdx];
        for (int fluidPackageIdx = 0; fluidPackageIdx < fluidFamily.fluidPackages.length(); fluidPackageIdx++)
        {
            DS_DeviceDef::FluidPackage tempPackage = fluidFamily.fluidPackages[fluidPackageIdx];
            for (int barcodePrefixIdx = 0; barcodePrefixIdx < tempPackage.barcodePrefixes.length(); barcodePrefixIdx++)
            {
                if (tempPackage.barcodePrefixes[barcodePrefixIdx] == barcodeInfo.barcodePrefix)
                {
                    LOG_INFO("handleBarcodeData(): BarcodePrefix found from fluidOptions.contrastFamilies[%d].fluidPackages[%d].barcodePrefixes[%d]\n", familyIdx, fluidPackageIdx, barcodePrefixIdx);
                    barcodeInfo.isFluidPackageOk = true;
                    barcodeInfo.fluidPackage = tempPackage;
                    break;
                }
            }
        }
    }

    //LOG_DEBUG("---------------- fluidOptions=%s\n", Util::qVarientToJsonData(ImrParser::ToImr_FluidOptions(fluidOptions)).CSTR());



bail:
    barcodeInfo.fluidPackage.lotBatch = tempLotBatch;
    barcodeInfo.fluidPackage.expirationDateEpochMs = expDateEpochMs;

    LOG_INFO("handleBarcodeData(): BarcodeInfo=%s\n", Util::qVarientToJsonData(ImrParser::ToImr_BarcodeInfo(barcodeInfo)).CSTR());

    env->ds.deviceData->setBarcodeInfo(barcodeInfo);
    return barcodeInfo.isFluidPackageOk;
}

QString DeviceBarcodeReader::extractGs1Data(QString barcodeStr, QString *barcodePrefix, qint64 *expDateEpochMs, QString *lotBatch)
{
    //01[14chars]17[(expDate) 6 chars]10[(lotBatch)variable length to end]
    //17 may not always exist (ExpDate format = YYMMdd)
    //10 may not always exist but has to come after 17

    QString ret = "";

    *barcodePrefix = "";
    *expDateEpochMs = -1;
    *lotBatch = "";

    if (barcodeStr.startsWith("01"))
    {
        //Extract Barcode Prefix
        barcodeStr.remove(0, BARCODE_IDENTIFIER_LEN);
        *barcodePrefix = barcodeStr.left(BARCODE_PREFIX_DATA_LEN);
        barcodeStr.remove(0, BARCODE_PREFIX_DATA_LEN);

        for (int i  = 0; i < BARCODE_NUMBER_OF_FIELDS; i++)
        {
            // Extract exp date
            if (barcodeStr.startsWith("17"))
            {
                QDateTime expDate;
                QDate date;
                barcodeStr.remove(0, BARCODE_IDENTIFIER_LEN);
                QString barcodeExpDateStr = barcodeStr.left(BARCODE_EXPIRATION_DATE_LEN);
                int year = barcodeExpDateStr.left(2).toInt() + 2000; //20yy
                int month = barcodeExpDateStr.mid(2, 2).toInt();
                int day = barcodeExpDateStr.mid(4, 2).toInt();
                if (day == 0)
                {
                    day = QDate(year, month, 1).daysInMonth();
                }
                date.setDate(year, month, day);
                expDate.setDate(date);
                *expDateEpochMs = expDate.toMSecsSinceEpoch();
                barcodeStr.remove(0, BARCODE_EXPIRATION_DATE_LEN);
            }

            // Extract Lot/batch
            if (barcodeStr.startsWith("10"))
            {
                barcodeStr.remove(0, BARCODE_IDENTIFIER_LEN);

                if (barcodeStr.contains(BARCODE_LOT_BATCH_DELIMETER))
                {
                    *lotBatch = barcodeStr.left(barcodeStr.indexOf(BARCODE_LOT_BATCH_DELIMETER));
                    barcodeStr.remove(0, barcodeStr.indexOf(BARCODE_LOT_BATCH_DELIMETER));
                    barcodeStr.remove(BARCODE_LOT_BATCH_DELIMETER);
                }
                else
                {
                    *lotBatch = barcodeStr;
                }
            }
        }

        LOG_DEBUG("extractGs1Data(): Scanned GS1 barcode. Prefix='%s', ExpDate='%s', lotBatch='%s'\n",
                  barcodePrefix->CSTR(),
                  QDateTime::fromMSecsSinceEpoch(*expDateEpochMs).toString("yyyy-MM-dd").CSTR(),
                  lotBatch->CSTR());
    }
    else
    {
        ret = "GS1 Barcode Identifier Not Found";
    }

    return ret;
}
