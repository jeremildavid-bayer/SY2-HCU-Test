#include <QSerialPortInfo>
#include <QThread>
#include <usb.h>
#include <QFile>
#include "Common/McuHardware.h"
#include "DataServices/Upgrade/DS_UpgradeData.h"
#include "UpgradeHwMcu.h"

#define STELLINITY2_FLASH_LINE_SIZE 256 // bytes to write/verify at a time (don't change this!!!)
#define CBUS_BIT_BANG               0x20
#define STELLINITY2_STR             "Stellinity2" // **CAVEATE** Must match the description string in FTDI usb/serial chip on the board

// Boot Mode Command Definitions
#define CMD_BITRATE_CAL             0x00
#define CMD_DEV_SELECT              0x10
#define CMD_DEV_INQ                 0x20
#define CMD_CLOCKMODE_INQ           0x21
#define CMD_FREQMULT_INQ            0x22
#define CMD_OPFREQ_INQ              0x23
#define CMD_CLOCKMODE               0x11
#define CMD_NEW_BITRATE             0x3F
#define CMD_PGM_TRANS               0x40
#define CMD_BOOT_INQ                0x4F
#define CMD_BITRATE_SET             0x55

// BootMode Response Definitions
#define RSP_BITRATE_ACK             0x00
#define ACK                         0x06
#define RSP_DEV_INQ                 0x30
#define RSP_CLOCKMODE_INQ           0x31
#define RSP_FREQMULT_INQ            0x32
#define RSP_OPFREQ_INQ              0x33
#define RSP_BITRATE_SET             0xE6
#define RSP_ERROR                   0xFF

// Rx63t Definitions for Erase response
#define ID_PROTECT_DISABLED         0x26
#define ID_PROTECT_ENABLED          0x16

// Programming Commands
#define SELECT_USER_MAT             0x43
#define PROGRAMME                   0x50
#define BOOT_PGM_INQ                0x4F

// 'n' byte command block header index definitions
#define BLK_COMMAND                 0
#define BLK_DATALENGTH              1
#define BLK_DATA                    2

// Device Inquiry Response packet index definitions
#define DEVCODELEN                  4       // device code length
#define DE_RESPONSE                 0       // contents should always be RSP_DEV_ENQ
#define DE_SIZE                     1
#define DE_NUMDEV                   2       // contents should always be 1 (device)
#define DE_NUMCHAR                  3
#define DE_DEVCODE                  4
#define DE_PRODNAME                 (DE_DEVCODE+DEVCODELEN)
#define PROG_BITRATE                1152    // 115200 baud
#define SALIENT_OP_FREQUENCY        1000    // 10MHz
#define STELLINITY2_OP_FREQUENCY    1200    // 12MHz
#define MULTIPLIER1                 8       // for 80MHz main (cpu) clock (96MHz for STELLINITY2)
#define MULTIPLIER2                 4       // for 40MHz peripheral clock (48MHz for STELLINITY2)
#define DELAY_TIME                  500     //Generic Delay time
#define MAX_NUM_RECORDS             4096
#define MAX_NUM_RECORDS_ADJUSTED    409600
#define MAX_BIT_RATE_CAL_RETRY      30

UpgradeHwMcu::UpgradeHwMcu(QObject *parent, EnvGlobal *env_, EnvLocal *envLocal_) :
    UpgradeHwBase(parent, env_, envLocal_, "UpgradeHwMcu", "UPGRADE_HW_MCU")
{
    eraseTimeoutMs = 25000;
    retryLimit = 2;
}

UpgradeHwMcu::~UpgradeHwMcu()
{
}

QString UpgradeHwMcu::connectDevice()
{
    QString err = "";

    if (ftdi_init(&curHandle) < 0)
    {
        err = "Failed to init ftdi handle";
        return err;
    }

    int errCode = ftdi_usb_open(&curHandle, FTDI_MCU_VENDOR_ID, FTDI_MCU_PRODUCT_ID);

    if (errCode != 0)
    {
        err = QString().asprintf("Failed to open usb, err=%d, %s\n", errCode, curHandle.error_str);
        return err;
    }

    return err;
}

QString UpgradeHwMcu::startUpgradeInner(QFile &fileBuf)
{
    QString err;

    // preset to erased FF's
    recordsWritten = 0;
    memset(image, 0xff, sizeof(image));

    if ((err = initialiseUpgrade()) != "")
    {
        err = "Init Err: " + err;
        endUpgrade();
        return err;
    }

    if ((err = loadImage(fileBuf)) != "")
    {
        err = "Load Err: " + err;
        endUpgrade();
        return err;
    }

    return "";
}

QString UpgradeHwMcu::setPortSettings(int baudRate)
{
    ftdi_disable_bitbang(&curHandle);
    if (ftdi_set_baudrate(&curHandle, baudRate) != 0)
    {
        return "Cannot set baudrate";
    }

    if (ftdi_set_line_property(&curHandle, BITS_8, STOP_BIT_1, NONE) != 0)
    {
        return "Cannot set line property";
    }

    if (ftdi_setflowctrl(&curHandle, SIO_DISABLE_FLOW_CTRL) != 0)
    {
        return "Cannot set flow control";
    }

    return "";
}

QString UpgradeHwMcu::initialiseUpgrade()
{
    QString err = "";
    quint8 mask;
    quint8 mode = CBUS_BIT_BANG;

    int bytesWritten;
    int bytesRead;

    mask = SY2_MCU_CBUS_RUN_BITS;

    LOG_DEBUG("UPGRADE_HW_MCU: initialiseUpgrade(): Started\n");

    if (ftdi_set_bitmode(&curHandle, mask, mode) != 0)
    {
        err = "Failed to set bit mode";
        goto bail;
    }

    setPortSettings(19200);

    if ((err = setToBootMode()) != "")
    {
        err = "Failed to set boot mode: " + err;
        goto bail;
    }

    if (setTransferBitRate() != "")
    {
        setPortSettings(19200);
        setToBootMode();
        if (setTransferBitRate() != "")
        {
            err = "Failed to set bit rate";
            goto bail;
        }
    }

    if (acknowledgeTransferBitRate() != "")
    {
        // Retrying..
        setPortSettings(19200);
        setToBootMode();
        if (setTransferBitRate() != "")
        {
            err = "Failed to set bit rate at postion2";
            goto bail;
        }
        if (acknowledgeTransferBitRate() != "")
        {
            err = "Failed twice to acknowledge bit rate";
            goto bail;
        }
    }

    ///////////////////////////////////////////// device inquiry
    // response should be:
    // 0x30 - response code
    // 13   - size
    // 01   - number of devices
    // 11   - number of device code + product name characters
    // '0'  - device code
    // 'w'
    // 'f'
    // '1'
    // 'R'  - product name
    // 'S'
    // 'F'
    // '7'
    // '0'
    // '8'
    // '6'
    // 0xD7 checksum

    ftdi_usb_purge_buffers(&curHandle);
    txbuffer[0] = CMD_DEV_INQ;

    bytesWritten = ftdi_write_data(&curHandle, txbuffer, 1);
    QThread::msleep(10);
    if ((err = getBootResponse(rxbuffer, RSP_DEV_INQ)) != "")
    {
        err = QString().asprintf("Failed to get boot response: BytesWritten=%d, Err=%s", bytesWritten, err.CSTR());
        goto bail;
    }
    ///////////////////////////////////////////// end device inquiry

    ///////////////////////////////////////////// device select
    txbuffer[BLK_COMMAND] = CMD_DEV_SELECT;
    txbuffer[BLK_DATALENGTH] = DEVCODELEN;

    // copy the device code to the transmit packet
    memcpy(&txbuffer[BLK_DATA], &rxbuffer[DE_DEVCODE], DEVCODELEN);
    sendBootCommand(txbuffer);

    // attempt to get 1 byte response
    //ftdi_setrts(&curHandle, SIO_SET_RTS_HIGH);
    QThread::msleep(30);
    bytesRead = ftdi_read_data(&curHandle, rxbuffer, 1);
    if (bytesRead <= 0)
    {
        err = "Failed to read data";
        goto bail;
    }

    if (rxbuffer[DE_RESPONSE] != ACK)
    {
        err = "Failed to get correct response";
        goto bail;
    }
    ///////////////////////////////////////////// end device select

    ///////////////////////////////////////////// clock mode inquiry
    txbuffer[0] = CMD_CLOCKMODE_INQ;
    bytesWritten = ftdi_write_data(&curHandle, txbuffer, 1);

    QThread::msleep(10);

    if ((err = getBootResponse(rxbuffer, RSP_CLOCKMODE_INQ)) != "")
    {
        err = QString().asprintf("Failed to get clockmode inquiry: BytesWritten=%d, Err=%s", bytesWritten, err.CSTR());
        goto bail;
    }

    ///////////////////////////////////////////// end clock mode inquiry

    ///////////////////////////////////////////// clock mode set
    txbuffer[BLK_COMMAND] = CMD_CLOCKMODE;
    txbuffer[BLK_DATALENGTH] = 1;
    txbuffer[BLK_DATA] = rxbuffer[BLK_DATA];
    sendBootCommand(txbuffer);

    QThread::msleep(15);
    // attempt to get 1 byte response
    bytesRead = ftdi_read_data(&curHandle, rxbuffer, 1);
    if (bytesRead <= 0)
    {
        err = "Failed to read data (clock mode set)";
        goto bail;
    }

    if (rxbuffer[DE_RESPONSE] != ACK)
    {
        err = "Failed to close usb handle";
        goto bail;
    }
    ///////////////////////////////////////////// end clock mode set

    ///////////////////////////////////////////// bit rate set
    txbuffer[BLK_COMMAND] = CMD_NEW_BITRATE;
    txbuffer[BLK_DATALENGTH] = 7; // the number of data bytes that follow
    txbuffer[BLK_DATA+0] = (PROG_BITRATE >> 8);   // high order byte
    txbuffer[BLK_DATA+1] = (PROG_BITRATE & 0xFF); // low order byte

    txbuffer[BLK_DATA+2] = (STELLINITY2_OP_FREQUENCY >> 8);   // high order byte
    txbuffer[BLK_DATA+3] = (STELLINITY2_OP_FREQUENCY & 0xFF); // low order byte break;

    txbuffer[BLK_DATA+4] = 2;                     // number of multipliers

    txbuffer[BLK_DATA+5] = MULTIPLIER1;           // cpu clock multiplier
    txbuffer[BLK_DATA+6] = MULTIPLIER2;           // peripheral clock multiplier

    sendBootCommand(txbuffer); // comms speed should be 115200 after this

    // close and re-open comms port at new speed
    if (ftdi_usb_close(&curHandle) != 0)
    {
        err = "Failed to close usb handle";
        goto bail;
    }

    LOG_DEBUG("UPGRADE_HW_MCU: initialiseUpgrade(): Waiting to re-open USB port..\n");

    QThread::msleep(1000); //Give the lib some time after close

    if (ftdi_usb_open(&curHandle, FTDI_MCU_VENDOR_ID, FTDI_MCU_PRODUCT_ID) != 0)
    {
        err = "Failed to open usb at new speed";
        goto bail;
    }

    if (ftdi_set_baudrate(&curHandle, 115200) != 0)
    {
        err = "Failed to set another baud rate";
        goto bail;
    }

    if (ftdi_set_line_property(&curHandle, BITS_8, STOP_BIT_1, NONE) != 0)
    {
        err = "Cannot set line property";
        goto bail;
    }

    if (ftdi_setflowctrl(&curHandle, SIO_DISABLE_FLOW_CTRL) != 0)
    {
        err = "Cannot set flow control";
        goto bail;
    }

    // send ACK to confirm the new speed
    txbuffer[BLK_COMMAND] = ACK;

    for (int retryIdx = 0; retryIdx < 10; retryIdx++)
    {
        bytesWritten = ftdi_write_data(&curHandle, txbuffer, 1);
        QThread::msleep(10);
        bytesRead = ftdi_read_data(&curHandle, rxbuffer, 1); // attempt to get 1 byte ACK response

        if ( (bytesWritten <= 0) || (bytesRead <= 0) )
        {
            err = QString().asprintf("Byte W/R Check failed. ByteW/R=%d,%d", bytesWritten, bytesRead);
            LOG_WARNING("UPGRADE_HW_MCU: initialiseUpgrade(): Try[%d]: %s\n", retryIdx, err.CSTR());
        }

        if (rxbuffer[DE_RESPONSE] != ACK)
        {
            err = QString().asprintf("Failed to get ACK: 0x%02x", rxbuffer[DE_RESPONSE]);
            LOG_WARNING("UPGRADE_HW_MCU: initialiseUpgrade(): Try[%d]: %s\n", retryIdx, err.CSTR());
        }
        else
        {
            LOG_DEBUG("UPGRADE_HW_MCU: initialiseUpgrade(): Ack Received\n");
            err = "";
            break;
        }
    }

bail:
    if (err != "")
    {
        LOG_ERROR("UPGRADE_HW_MCU: initialiseUpgrade(): %s\n", err.CSTR());
    }
    return err;
}

QString UpgradeHwMcu::sendBootCommand(quint8 *buffer)
{
    int size = buffer[BLK_DATALENGTH] + 2; // include the command & datalength bytes in size
    int bytesWritten;
    buffer[size] = McuHardware::calculateMcuUpgradeChecksum(buffer, size);
    bytesWritten = ftdi_write_data(&curHandle, buffer, size+1);
    if (bytesWritten > 0)
    {
        ////qDebug() << "sendBootCommand() -> bytesWritten: " << bytesWritten;
        ////qDebug() << buffer[0] << buffer[1] << buffer[2] << buffer[3] << buffer[4] << buffer[5] << buffer[6] << buffer[7] << buffer[8] << buffer[9];
        return "";
    }
    return "Failed to send boot command";
}

QString UpgradeHwMcu::getBootResponse(quint8 *buffer, quint8 responseCode)
{
    int bytesRead;
    bytesRead = ftdi_read_data(&curHandle, &buffer[0], 1);

    if (bytesRead <= 0)
    {
        return "No boot response received";
    }

    if (buffer[0] != responseCode)
    {
        return QString().asprintf("Incorrect responseCode: 0x%02x != 0x%02x", buffer[0], responseCode);
    }

    bytesRead = ftdi_read_data(&curHandle, &buffer[1], 1);

    if (bytesRead <= 0)
    {
        return "No response getting length byte";
    }

    QThread::msleep(100);
    bytesRead = ftdi_read_data(&curHandle, &buffer[2], buffer[1]+1);
    if (bytesRead < 0)
    {
        return "No response getting rest of boot response";
    }

   // //qDebug() << "bytesRead:" << bytesRead << "length byte:" << buffer[1]+1;
    if (bytesRead != (int)(buffer[1]+1))
    {
        return "Byte write check failed";
    }

    return "";
}

QString UpgradeHwMcu::acknowledgeTransferBitRate()
{
    // Acknowledge the bitrate detection
    txbuffer[0] = CMD_BITRATE_SET;

    int bytesWritten = ftdi_write_data(&curHandle, &txbuffer[0], 1);
    if (bytesWritten <= 0)
    {
        return "Failed to write data during acknowledge bit rate";
    }

    QThread::msleep(10);
    // attempt to get 1 byte response
    int bytesRead = ftdi_read_data(&curHandle, &rxbuffer[0], 1);
    if (bytesRead <= 0)
    {
        return "Failed to read data during acknowledge bit rate";
    }

    if (rxbuffer[0] != RSP_BITRATE_SET)
    {
        return QString().asprintf("Failed to get correct response. %02x is not %02x\n", rxbuffer[0], RSP_BITRATE_SET);
    }

    return "";
}

void UpgradeHwMcu::setReadWriteTimeout(int readTimeout, int writeTimeout)
{
    curHandle.usb_read_timeout = readTimeout;
    curHandle.usb_write_timeout = writeTimeout;
}

QString UpgradeHwMcu::setTransferBitRate()
{
    int bytesread;
    QString err;

    if ((err = setPortSettings(9600)) != "")
    {
        err = "Set port failed: " + err;
        goto bail;
    }

    //setReadWriteTimeout(10, 5);

    txbuffer[0] = CMD_BITRATE_CAL;  // Speed detection
    rxbuffer[0] = RSP_ERROR;        // Default to error response

    for (int retryIdx = 0; retryIdx < 30; retryIdx++)
    {
        int bytesWritten = ftdi_write_data(&curHandle, txbuffer, 1);
        if (bytesWritten <= 0)
        {
            err = "Failed to write bytes";
            LOG_WARNING("UPGRADE_HW_MCU: Try[%d]: setTransferBitRate(): %s\n", retryIdx, err.CSTR());
        }

        // Added this to ensure there is time for the MCU to respond
        QThread::msleep(5);

        if ((bytesread = ftdi_read_data(&curHandle, rxbuffer, 1)) < 0)
        {
            err = "Failed to read bytes";
            LOG_WARNING("UPGRADE_HW_MCU: Try[%d]: setTransferBitRate(): %s\n", retryIdx, err.CSTR());
        }

        if (bytesread == 0)
        {
            err = "Failed to read bytes back";
            LOG_WARNING("UPGRADE_HW_MCU: Try[%d]: setTransferBitRate(): %s\n", retryIdx, err.CSTR());
        }
        else
        {
            //LOG_DEBUG("UPGRADE_HW_MCU: Bit rate cal bytes read(%d), rxbuffer[0](%02x) i(%d)\n", bytesread, rxbuffer[0], i);
            err = "";
            break; // received a byte
        }
    }

bail:
    if (err != "")
    {
        LOG_ERROR("UPGRADE_HW_MCU: setTransferBitRate(): %s\n", err.CSTR());
    }
    return "";
}

QString UpgradeHwMcu::endUpgrade()
{
    releaseBootMode();
    disconnectDevice();
    return "";
}

QString UpgradeHwMcu::setToBootMode()
{
    quint8 mask;
    quint8 mode = CBUS_BIT_BANG;

    // put the chip into reset
    mask = SY2_MCU_CBUS_RESET_BITS;

    if (ftdi_set_bitmode(&curHandle, mask, mode) != 0)
    {
        return "Cannot set bitmode";
    }

    QThread::msleep(DELAY_TIME);

    // release reset into boot mode
    mask = SY2_MCU_CBUS_BOOT_BITS;

    if (ftdi_set_bitmode(&curHandle, mask, mode) != 0)
    {
        return "Cannot set bitmode";
    }

    isBoot = true;
    QThread::msleep(DELAY_TIME);

    return "";
}

QString UpgradeHwMcu::releaseBootMode()
{
    quint8 mode, mask;

    if (isBoot)
    {
        mode = CBUS_BIT_BANG;
        mask = SY2_MCU_CBUS_RESET_BITS;
        ftdi_set_bitmode(&curHandle, mask, mode);

        QThread::msleep(100);

        mask = SY2_MCU_CBUS_RUN_BITS;
        ftdi_set_bitmode(&curHandle, mask, mode);
        isBoot = false;

        // Giving MCU time to reboot and restore comm
        QThread::msleep(2000);
    }

    return "";
}

QString UpgradeHwMcu::disconnectDevice()
{
    QString err = "";
    int errCode = ftdi_usb_close(&curHandle);
    if (errCode != 0)
    {
        err = QString().asprintf("Failed to close usb, err(%d)\n", errCode);
    }

    ftdi_deinit(&curHandle);
    return err;
}

QString UpgradeHwMcu::loadImage(QFile &fileBuf)
{
    flashStart = 0xFFF80000;
    flashEnd   = 0xFFFFFFFF;
    totalRecords = flashEnd - flashStart;
    currentAddress = flashStart;

    S19STAT s19Stat = McuHardware::getSRecords(fileBuf, image, flashStart, flashEnd, &maxAddress);

    if (s19Stat == ENDRECORD)
    {
        return "";
    }

    return QString().asprintf("Failed to get SRecords (stat=%d)", s19Stat);
}

QString UpgradeHwMcu::eraseFlash()
{
    QString err = "";
    txbuffer[0] = CMD_PGM_TRANS;

    if (ftdi_write_data(&curHandle, &txbuffer[0], 1) <= 0)
    {
        err = "Failed to write data";
        LOG_ERROR("UPGRADE_HW_MCU: ERASE_FLASH: %s\n", err.CSTR());
    }

    return err;
}


QString UpgradeHwMcu::verifyErasedFlash()
{
    QString err = "";
    int bytesWritten;
    int bytesRead;
    quint8 mask = SY2_MCU_CBUS_BOOT_BITS;
    quint8 mode;

    bytesRead = ftdi_read_data(&curHandle, &rxbuffer[DE_RESPONSE], 2);
    if (bytesRead <= 0)
    {
        err = "Failed to read data";
        goto bail;
    }

    if (rxbuffer[DE_RESPONSE] != ID_PROTECT_DISABLED)
    {
        // ID protection must be off for this implementation
        err = QString().asprintf("Incorrect response (%02x, %02x) should be (%05x)\n", rxbuffer[DE_RESPONSE], rxbuffer[1], ID_PROTECT_DISABLED);
        goto bail;
    }

    //------------------------------------------------------------

    ///////////////////////////////////////////// USER MAT Select
    txbuffer[0] = SELECT_USER_MAT;

    mask = SY2_MCU_CBUS_BOOT_BITS;
    mode = CBUS_BIT_BANG;
    mask = SY2_MCU_CBUS_BOOT_BITS;
    ftdi_set_bitmode(&curHandle, mask, mode);

    bytesWritten = ftdi_write_data(&curHandle, txbuffer, 1);
    if (bytesWritten <= 0)
    {
        err = "Failed to write data";
        goto bail;
    }
    bytesWritten = ftdi_write_data(&curHandle, txbuffer, 1);

    QThread::msleep(10);

    bytesRead = ftdi_read_data(&curHandle, rxbuffer, 1);
    if (bytesRead <= 0)
    {
        err =  QString().asprintf("Failed Verify Erase Flash: BytesWritten=%d, BytesRead=%d", bytesWritten, bytesRead);
        goto bail;
    }

    if (rxbuffer[DE_RESPONSE] != ACK)
    {
        //Retry
        for (int retryIdx = 0; retryIdx < 10; retryIdx++)
        {
            ftdi_write_data(&curHandle, txbuffer, 1);
            QThread::msleep(5);
            ftdi_read_data(&curHandle, &rxbuffer[0], 1);

            if (rxbuffer[DE_RESPONSE] == ACK)
            {
                err = "";
                break;
            }
            else
            {
                err = QString().asprintf("Incorrect response (%d) should be ACK(%d)\n", rxbuffer[DE_RESPONSE], ACK);
                LOG_WARNING("UPGRADE_HW_MCU: Try[%d]: verifyErasedFlash(): %s\n", retryIdx, err.CSTR());
            }
        }
    }
    ///////////////////////////////////////////// end USER MAT Select

bail:
    if (err != "")
    {
        LOG_ERROR("UPGRADE_HW_MCU: verifyErasedFlash(): %s\n", err.CSTR());
    }
    return err;
}

QString UpgradeHwMcu::startProgramFlash()
{
    DS_UpgradeDef::UpgradeDigest digest = env->ds.upgradeData->getUpgradeDigest();
    DS_UpgradeDef::UpgradeHwProgramState programState = programImage();

    switch (programState)
    {
    case DS_UpgradeDef::UPGRADE_HW_PROGRAM_STATE_IN_PROGRESS:
        QTimer::singleShot(PROCESS_STATE_TRANSITION_DELAY_MS, [=] {
            startProgramFlash();
        });
        break;
    case DS_UpgradeDef::UPGRADE_HW_PROGRAM_STATE_ERR_NO_RX_DATA:
        endUpgrade();
        digest.mcu.err = "No RX Data";
        env->ds.upgradeData->setUpgradeDigest(digest);
        updateUpgradeHwDigest();
        setState(DS_UpgradeDef::UPGRADE_HW_STATE_FAILED);
        break;
    case DS_UpgradeDef::UPGRADE_HW_PROGRAM_STATE_ERR_BAD_RX_DATA:
        endUpgrade();
        digest.mcu.err = "Bad RX Data";
        env->ds.upgradeData->setUpgradeDigest(digest);
        updateUpgradeHwDigest();
        setState(DS_UpgradeDef::UPGRADE_HW_STATE_FAILED);
        break;
    case DS_UpgradeDef::UPGRADE_HW_PROGRAM_STATE_COMPLETE:
        endUpgrade();
        loadProgress = 100;
        retryCount = retryLimit;
        updateUpgradeHwDigest();
        setState(DS_UpgradeDef::UPGRADE_HW_STATE_COMPLETED);
        break;
    default:
        break;
    }
    return "";
}

DS_UpgradeDef::UpgradeHwProgramState UpgradeHwMcu::programImage()
{
    int bytesWritten;
    int bytesRead;
    int i;
    int flashLineSize;

    if (currentAddress >= flashEnd)
    {
        // all done! no send end-programme
        txbuffer[0] = PROGRAMME;
        txbuffer[1] = 0xFF;
        txbuffer[2] = 0xFF;
        txbuffer[3] = 0xFF;
        txbuffer[4] = 0xFF;
        txbuffer[5] = McuHardware::calculateMcuUpgradeChecksum(txbuffer, 5);

        bytesWritten = ftdi_write_data(&curHandle, txbuffer, 6);

        if (bytesWritten < 0)
        {
            //LOG_ERROR("Failed to write bytes\n");
        }

        QThread::msleep(40);
        bytesRead = ftdi_read_data(&curHandle, rxbuffer, 1);

        if (bytesRead == 0)
        {
            LOG_ERROR("UPGRADE_HW_MCU: programImage(): No RX Data\n");
            return DS_UpgradeDef::UPGRADE_HW_PROGRAM_STATE_ERR_NO_RX_DATA;
        }

        if (rxbuffer[DE_RESPONSE] != ACK)
        {
            LOG_ERROR("UPGRADE_HW_MCU: programImage(): Rx data should be 0x06 no %02x\n", rxbuffer[1]);
            return DS_UpgradeDef::UPGRADE_HW_PROGRAM_STATE_ERR_BAD_RX_DATA;
        }

        loadProgress = 100;
        updateUpgradeHwDigest();
        return DS_UpgradeDef::UPGRADE_HW_PROGRAM_STATE_COMPLETE;
    }

    flashLineSize = STELLINITY2_FLASH_LINE_SIZE;

    for (i = 0; i < flashLineSize; i++)
    {
        if (image[currentAddress - flashStart + i] != 0xff)
        {
            break;
        }
    }

    if (i == flashLineSize)
    {
        // this record is all FF - skip it
        currentAddress += flashLineSize;
        recordsWritten += flashLineSize;
        loadProgress = (int)(100 * ((double)(recordsWritten) / (double)totalRecords));
        updateUpgradeHwDigest();
        return DS_UpgradeDef::UPGRADE_HW_PROGRAM_STATE_IN_PROGRESS;
    }

    txbuffer[0] = PROGRAMME;
    Util::putLong(&txbuffer[1], (unsigned long)currentAddress);
    memcpy(&txbuffer[5], &image[currentAddress - flashStart], flashLineSize);
    txbuffer[1 + 4 + flashLineSize] = McuHardware::calculateMcuUpgradeChecksum(txbuffer, (1 + 4 + flashLineSize));

    int sectionSize = 60;
    int totalSize = (1 + 4 + flashLineSize + 1);

    ftdi_usb_purge_rx_buffer(&curHandle);

    for (int k = 0; k < 10; k++)
    {
        if (totalSize < sectionSize)
        {
            bytesWritten = ftdi_write_data(&curHandle, &txbuffer[(k*sectionSize)], totalSize);
            QThread::msleep(10);
            break;
        }
        bytesWritten = ftdi_write_data(&curHandle, &txbuffer[(k*sectionSize)], sectionSize);
        QThread::msleep(10);
        totalSize -= sectionSize;
    }

    bytesRead = ftdi_read_data(&curHandle, rxbuffer, 2);

    // attempt to get 1 byte ACK response
    if (bytesRead <= 0)
    {
        LOG_ERROR("UPGRADE_HW_MCU: programImage(): No RX Data\n");
        updateUpgradeHwDigest();
        return DS_UpgradeDef::UPGRADE_HW_PROGRAM_STATE_ERR_NO_RX_DATA;
    }

    if (rxbuffer[DE_RESPONSE] != ACK)
    {
        LOG_ERROR("UPGRADE_HW_MCU: Rx data[0] should be 0x06 not %02x\n", rxbuffer[0]);
        updateUpgradeHwDigest();
        return DS_UpgradeDef::UPGRADE_HW_PROGRAM_STATE_ERR_BAD_RX_DATA;
    }

    currentAddress += flashLineSize;
    recordsWritten += flashLineSize;
    loadProgress = (int)(100 * ((double)(recordsWritten) / (double)totalRecords));
    updateUpgradeHwDigest();

    if (currentAddress == 0)
    {
        // last address!
        currentAddress = 0xFFFFFFFF;
    }

    // programming incomplete
    return DS_UpgradeDef::UPGRADE_HW_PROGRAM_STATE_IN_PROGRESS;
}

void UpgradeHwMcu::updateUpgradeHwDigest()
{
    DS_UpgradeDef::UpgradeDigest digest = env->ds.upgradeData->getUpgradeDigest();
    digest.mcu.progress = (eraseProgress * 0.5) + (loadProgress * 0.5);
    digest.mcu.progress = (digest.mcu.progress * retryCount) / retryLimit;
    env->ds.upgradeData->setUpgradeDigest(digest);
}
