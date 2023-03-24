#include <QThread>
#include "UpgradeHwStopcock.h"
#include "DataServices/Upgrade/DS_UpgradeData.h"

#define FTDI_TX_LINE_LIMIT_BYTES            60

// STM8 Addresses
#define STM8_PROGRAM_MEM_START_ADDRESS      0x8000

// STM8 Commands
#define STM8_CMD_SYNCH                      0x7F
#define STM8_CMD_ERASE_MEM                  0x43
#define STM8_CMD_ERASE_MEM_ALL              0xFF
#define STM8_CMD_WRITE_MEM                  0x31
#define STM8_CMD_GO                         0x21

//SY2_CBUS3_RESET_MASK -> low to reset
#define SY2_SC_CBUS_RESET_BITS              0xF0 | STELLINITY2_MUX_PROGRAM_SC | SY2_CBUS2_P00_MASK
#define SY2_SC_CBUS_BOOT_BITS               0xF0 | STELLINITY2_MUX_PROGRAM_SC | SY2_CBUS3_RESET_MASK
#define SY2_SC_CBUS_RUN_BITS                0xF0 | SY2_CBUS2_P00_MASK | SY2_CBUS3_RESET_MASK

#define STM8_BOOTLOADER_ADDRESS_16BIT       0x6000
#define STM8_RAM_ADDRESS                    0xa0

static const int e_w_routine_32k_1_3_size = 304;
static const quint8 e_w_routine_32k_1_3_data[] = {
    0x5f,0x3f,0x90,0x3f,0x96,0x72,0x09,0x00,0x8e,0x16,0xcd,0x60,0x65,0xb6,0x90,0xe7,
    0x00,0x5c,0x4c,0xb7,0x90,0xa1,0x21,0x26,0xf1,0xa6,0x20,0xb7,0x88,0x5f,0x3f,0x90,
    0xe6,0x00,0xa1,0x20,0x26,0x07,0x3f,0x8a,0xae,0x40,0x00,0x20,0x0c,0x3f,0x8a,0xae,
    0x00,0x80,0x42,0x58,0x58,0x58,0x1c,0x80,0x00,0x90,0x5f,0xcd,0x60,0x65,0x9e,0xb7,
    0x8b,0x9f,0xb7,0x8c,0xa6,0x20,0xc7,0x50,0x5b,0x43,0xc7,0x50,0x5c,0x4f,0x92,0xbd,
    0x00,0x8a,0x5c,0x9f,0xb7,0x8c,0x4f,0x92,0xbd,0x00,0x8a,0x5c,0x9f,0xb7,0x8c,0x4f,
    0x92,0xbd,0x00,0x8a,0x5c,0x9f,0xb7,0x8c,0x4f,0x92,0xbd,0x00,0x8a,0x72,0x00,0x50,
    0x5f,0x07,0x72,0x05,0x50,0x5f,0xfb,0x20,0x04,0x72,0x10,0x00,0x96,0x90,0xa3,0x00,
    0x07,0x27,0x0a,0x90,0x5c,0x1d,0x00,0x03,0x1c,0x00,0x80,0x20,0xae,0xb6,0x90,0xb1,
    0x88,0x27,0x1c,0x5f,0x3c,0x90,0xb6,0x90,0x97,0xcc,0x00,0xc0,0x9d,0x9d,0x9d,0x9d,
    0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x81,
    0xcd,0x60,0x65,0x5f,0x3f,0x97,0x72,0x0d,0x00,0x8e,0x18,0x72,0x00,0x00,0x94,0x0b,
    0xa6,0x01,0xc7,0x50,0x5b,0x43,0xc7,0x50,0x5c,0x20,0x08,0x35,0x81,0x50,0x5b,0x35,
    0x7e,0x50,0x5c,0x3f,0x94,0xf6,0x92,0xa7,0x00,0x8a,0x72,0x0c,0x00,0x8e,0x13,0x72,
    0x00,0x50,0x5f,0x07,0x72,0x05,0x50,0x5f,0xfb,0x20,0x04,0x72,0x10,0x00,0x97,0xcd,
    0x60,0x65,0x9f,0xb1,0x88,0x27,0x03,0x5c,0x20,0xdb,0x72,0x0d,0x00,0x8e,0x10,0x72,
    0x00,0x50,0x5f,0x07,0x72,0x05,0x50,0x5f,0xfb,0x20,0x24,0x72,0x10,0x00,0x97,0x20,
    0x1e,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,
    0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x81
};

static const int e_w_routine_32k_1_2_size = 304;
static const quint8 e_w_routine_32k_1_2_data[] = {
    0x5f,0x3f,0x90,0x3f,0x97,0x72,0x09,0x00,0x8e,0x16,0xcd,0x60,0x6d,0xb6,0x90,0xe7,
    0x00,0x5c,0x4c,0xb7,0x90,0xa1,0x21,0x26,0xf1,0xa6,0x20,0xb7,0x88,0x5f,0x3f,0x90,
    0xe6,0x00,0xa1,0x20,0x26,0x07,0x3f,0x8a,0xae,0x40,0x00,0x20,0x0c,0x3f,0x8a,0xae,
    0x00,0x80,0x42,0x58,0x58,0x58,0x1c,0x80,0x00,0x90,0x5f,0xcd,0x60,0x6d,0x9e,0xb7,
    0x8b,0x9f,0xb7,0x8c,0xa6,0x20,0xc7,0x50,0x5b,0x43,0xc7,0x50,0x5c,0x4f,0x92,0xbd,
    0x00,0x8a,0x5c,0x9f,0xb7,0x8c,0x4f,0x92,0xbd,0x00,0x8a,0x5c,0x9f,0xb7,0x8c,0x4f,
    0x92,0xbd,0x00,0x8a,0x5c,0x9f,0xb7,0x8c,0x4f,0x92,0xbd,0x00,0x8a,0x72,0x00,0x50,
    0x5f,0x07,0x72,0x05,0x50,0x5f,0xfb,0x20,0x04,0x72,0x10,0x00,0x97,0x90,0xa3,0x00,
    0x07,0x27,0x0a,0x90,0x5c,0x1d,0x00,0x03,0x1c,0x00,0x80,0x20,0xae,0xb6,0x90,0xb1,
    0x88,0x27,0x1c,0x5f,0x3c,0x90,0xb6,0x90,0x97,0xcc,0x00,0xc0,0x9d,0x9d,0x9d,0x9d,
    0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x81,
    0xcd,0x60,0x6d,0x5f,0x3f,0x98,0x72,0x0d,0x00,0x8e,0x18,0x72,0x00,0x00,0x94,0x0b,
    0xa6,0x01,0xc7,0x50,0x5b,0x43,0xc7,0x50,0x5c,0x20,0x08,0x35,0x81,0x50,0x5b,0x35,
    0x7e,0x50,0x5c,0x3f,0x94,0xcd,0x60,0x6d,0xf6,0x92,0xa7,0x00,0x8a,0x72,0x0c,0x00,
    0x8e,0x13,0x72,0x00,0x50,0x5f,0x07,0x72,0x05,0x50,0x5f,0xfb,0x20,0x04,0x72,0x10,
    0x00,0x98,0xcd,0x60,0x6d,0x9f,0xb1,0x88,0x27,0x03,0x5c,0x20,0xd8,0x72,0x0d,0x00,
    0x8e,0x10,0x72,0x00,0x50,0x5f,0x07,0x72,0x05,0x50,0x5f,0xfb,0x20,0x21,0x72,0x10,
    0x00,0x98,0x20,0x1b,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,
    0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x9d,0x81
};

static const quint8 firmwareBlankLine[] = {
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff
};

UpgradeHwStopcock::UpgradeHwStopcock(QObject *parent, EnvGlobal *env_, EnvLocal *envLocal_) :
    UpgradeHwBase(parent, env_, envLocal_, "UpgradeHwStopcock", "UPGRADE_HW_STOPCOCK")
{
    eraseTimeoutMs = 5000;
    retryLimit = 3;
}

UpgradeHwStopcock::~UpgradeHwStopcock()
{
}

QString UpgradeHwStopcock::connectDevice()
{
    QString err = "";

    if (ftdi_init(&curHandle) < 0)
    {
        err = "Failed to init ftdi handle";
        return err;
    }

    int errCode = ftdi_usb_open(&curHandle, FTDI_STOPCOCK_VENDOR_ID, FTDI_STOPCOCK_PRODUCT_ID);

    if (errCode != 0)
    {
        err = QString().asprintf("Failed to open usb, err=%d, %s\n", errCode, curHandle.error_str);
        return err;
    }

    return err;
}

QString UpgradeHwStopcock::disconnectDevice()
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

QString UpgradeHwStopcock::startUpgradeInner(QFile &fileBuf)
{
    QString err;
    recordsWritten = 0;

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

    if ((err = writeBootloaderRoutines()) != "")
    {
        err = "Write Bootloader Err: " + err;
        endUpgrade();
        return err;
    }

    return "";
}

QString UpgradeHwStopcock::eraseFlash()
{
    QString err = "";
    int txLen, rxLen;

    //----------------- Start Flash Erase
    txLen = 2;
    txbuffer[0] = STM8_CMD_ERASE_MEM;
    txbuffer[1] = STM8_CMD_ERASE_MEM ^ 0xFF; //Send complement

    rxbuffer[0] = 0;
    rxLen = 1;

    if (ftdi_write_data(&curHandle, txbuffer, txLen) != txLen)
    {
        err = "Failed to send Erase command";
        LOG_ERROR("UPGRADE_HW_STOPCOCK: ERASE_FLASH: %s\n", err.CSTR());
        return err;
    }

    QThread::msleep(2);
    if (ftdi_read_data(&curHandle, rxbuffer, rxLen) != rxLen)
    {
        err = "Failed to read Erase response";
        LOG_ERROR("UPGRADE_HW_STOPCOCK: ERASE_FLASH: %s\n", err.CSTR());
        return err;
    }

    if (rxbuffer[0] != STM8_ACK)
    {
        err = QString().asprintf("Incorrect Erase Response(%02x != %02x)\n", rxbuffer[0], STM8_ACK);
        LOG_ERROR("UPGRADE_HW_STOPCOCK: ERASE_FLASH: %s\n", err.CSTR());
        return err;

    }
    sendAck();

    //------------------ Send command to erase all flash
    txLen = 2;
    txbuffer[0] = STM8_CMD_ERASE_MEM_ALL;
    txbuffer[1] = STM8_CMD_ERASE_MEM_ALL ^ 0xFF; //Complement

    rxbuffer[0] = 0;

    if (ftdi_write_data(&curHandle, txbuffer, txLen) != txLen)
    {
        err = "Failed to send Erase command";
        LOG_ERROR("UPGRADE_HW_STOPCOCK: ERASE_FLASH: %s\n", err.CSTR());
        return err;
    }

    return err;
}

QString UpgradeHwStopcock::verifyErasedFlash()
{
    QString err = "";
    int rxLen = 1;

    if (ftdi_read_data(&curHandle, rxbuffer, rxLen) != rxLen)
    {
        err = "Failed to read EraseAll Response";
        LOG_ERROR("UPGRADE_HW_STOPCOCK: ERASE_FLASH: %s\n", err.CSTR());
        return err;
    }

    if ( (rxbuffer[0] != STM8_ACK) &&
         (rxbuffer[0] != STM8_NACK) )
    {
        err = QString().asprintf("Incorrect EraseAll Response(%02x != %02x)\n", rxbuffer[0], STM8_ACK);
        LOG_ERROR("UPGRADE_HW_STOPCOCK: ERASE_FLASH: %s\n", err.CSTR());
        return err;
    }
    sendAck();

    return err;
}

QString UpgradeHwStopcock::initialiseUpgrade()
{
    char mask = SY2_SC_CBUS_RESET_BITS;
    char mode = CBUS_BIT_BANG;
    int txLen, rxLen;

    //------- FTDI Settings
    if (ftdi_set_baudrate(&curHandle, 115200) != 0)
    {
        return "Failed to set baudrate";
    }

    if (ftdi_set_line_property(&curHandle, BITS_8, STOP_BIT_1, NONE) != 0)
    {
        return "Failed to set line property";
    }

    if (ftdi_setflowctrl(&curHandle, SIO_DISABLE_FLOW_CTRL) != 0)
    {
        return "Failed to set flow ctrl";
    }

    if (ftdi_set_bitmode(&curHandle, mask, mode) != 0)
    {
        LOG_ERROR("UPGRADE_HW_STOPCOCK: Failed to set RESET bitmode\n");
        return "Failed to set RESET bit mode";
    }

    QThread::msleep(10);

    mask = SY2_SC_CBUS_BOOT_BITS;
    if (ftdi_set_bitmode(&curHandle, mask, mode) != 0)
    {
        LOG_ERROR("UPGRADE_HW_STOPCOCK: Failed to set BOOT bitmode\n");
        return "Failed to set BOOT bit mode";
    }

    QThread::msleep(50);

    // ------------ Send the synch command
    txLen = 1;
    txbuffer[0] = STM8_CMD_SYNCH;
    rxbuffer[0] = 0;
    rxLen = 1;

    if (ftdi_write_data(&curHandle, txbuffer, txLen) != txLen)
    {
        LOG_ERROR("UPGRADE_HW_STOPCOCK: Failed to send SYNCH Command\n");
        return "Failed to send SYNCH";
    }

    QThread::msleep(100);

    if (ftdi_read_data(&curHandle, rxbuffer, rxLen) != rxLen)
    {
        LOG_ERROR("UPGRADE_HW_STOPCOCK: Failed to read SYNCH Response\n");
        return "Failed to read SYNCH";
    }

    if (rxbuffer[0] != STM8_ACK)
    {
        LOG_ERROR("UPGRADE_HW_STOPCOCK: Incorrect SYNCH Response(%02x != %02x)\n", rxbuffer[0], STM8_ACK);
        sendAck();
        return "Failed to get expected SYNCH";
    }

    sendAck();
    return "";
}


QString UpgradeHwStopcock::loadImage(QFile &fileBuf)
{
    char lineBuf[LINELEN + 1];

    flashStart = STM8_PROGRAM_MEM_START_ADDRESS;
    flashEnd = 0xFFFFFFFF;
    totalRecords = 0;
    currentAddress = flashStart;
    lstAddresses.clear();
    lstData.clear();

    while (fileBuf.readLine(lineBuf, sizeof(lineBuf)) != -1)
    {
        HexFileRecordType recType = loadImageFile(lineBuf, flashStart, flashEnd, &maxAddress);
        if (recType == HEX_EOF)
        {
            //continue process
            break;
        }
        else if (recType == HEX_ERR_BAD_START_CODE)
        {
            return "Failed to read first";
        }
        else if (recType == HEX_ERR_PARSE_FAIL)
        {
            return "Failed parse";
        }
    }

    totalRecords = lstData.length();
    return "";
}

HexFileRecordType UpgradeHwStopcock::loadImageFile(char * buffp, quint32 AddrMin, quint32 AddrMax, quint32 *pMaxAddr)
{
    quint8 datalen;
    QByteArray buf;
    QByteArray bufAddr;

    //Start Code
    if (buffp[0] != ':')
    {
        return HEX_ERR_BAD_START_CODE;
    }

    datalen = (quint8)(Util::tohex(buffp[1]) << 4) + (quint8)(Util::tohex(buffp[2]));

    quint8 highByte = (quint8)(Util::tohex(buffp[3]) << 4) + (quint8)(Util::tohex(buffp[4]));
    quint8 lowByte = (quint8)(Util::tohex(buffp[5]) << 4) + (quint8)(Util::tohex(buffp[6]));

    bufAddr.append('\0');
    bufAddr.append('\0');
    bufAddr.append(highByte);
    bufAddr.append(lowByte);
    lstAddresses.append(bufAddr);

    // Record type (1byte)
    quint8 rectype = 0;
    rectype = (quint8)(Util::tohex(buffp[7]) << 4) + (quint8)(Util::tohex(buffp[8]));

    switch (rectype)
    {
    case 0:
        //DATA - continue
        break;
    case 1:
        return HEX_EOF;
    default:
        return HEX_ERR_PARSE_FAIL;
    }

    // Actual data
    int currentData = 9;
    for (int i = 0; i < datalen; i++)
    {
        buf.append((quint8)(Util::tohex(buffp[currentData]) << 4) + (quint8)(Util::tohex(buffp[currentData + 1])));
        currentData += 2;
    }

    // Add to list
    lstData.append(buf);

    return HEX_DATA;
}

QString UpgradeHwStopcock::writeBootloaderRoutines()
{
    int currentWriteAddress = STM8_RAM_ADDRESS;
    int txLen, rxLen;
    int flashLineSize = FTDI_TX_LINE_LIMIT_BYTES;
    int bytesSent = 0;

    getBootloaderVersion();

    for (int i = 0; i < 10; i++)
    {
        if (currentWriteAddress >= (int)(e_w_routine_32k_1_2_size + STM8_RAM_ADDRESS) )
        {
            //complete
            break;
        }

        // ================== Send Write Command
        txLen = 2;
        txbuffer[0] = STM8_CMD_WRITE_MEM;
        txbuffer[1] = STM8_CMD_WRITE_MEM ^ 0xFF;

        rxbuffer[0] = 0;
        rxLen = 1;

        if (ftdi_write_data(&curHandle, txbuffer, txLen) != txLen)
        {
            return "Failed to send WRITE Command";
        }

        QThread::msleep(1);
        if (ftdi_read_data(&curHandle, rxbuffer, rxLen) != rxLen)
        {
            return "BLR Failed to read WRITE Response";
        }

        if (rxbuffer[0] != STM8_ACK)
        {
            return QString().asprintf("BLR Incorrect WRITE Response(%02x != %02x)", rxbuffer[0], STM8_ACK);
        }
        sendAck();

        // ================== Send Address
        txLen = 5;
        Util::writeAddress32bit(txbuffer, currentWriteAddress);
        txbuffer[4] = McuHardware::calculateStopcockUpgradeChecksum(&txbuffer[0], 4);
        rxbuffer[0] = 0;
        rxLen = 1;

        if (ftdi_write_data(&curHandle, txbuffer, txLen) != txLen)
        {
            return "BLR Failed to send WRITE-Address Command";
        }

        QThread::msleep(2);

        if (ftdi_read_data(&curHandle, rxbuffer, rxLen) != rxLen)
        {
            return "BLR Failed to read WRITE-Address Response";
        }

        if (rxbuffer[0] != STM8_ACK)
        {
            return QString().asprintf("BLR Incorrect WRITE-Address Response(%02x != %02x)\n", rxbuffer[0], STM8_ACK);
        }
        sendAck();

        // ================== Send DATA
        if ( (bytesSent + flashLineSize) >= e_w_routine_32k_1_2_size)
        {
            txLen = e_w_routine_32k_1_2_size - bytesSent;
        }
        else
        {
            txLen = flashLineSize;
        }

        txbuffer[0] = txLen - 1;
        memcpy(&txbuffer[1], &e_w_routine_32k_1_2_data[bytesSent], txLen);
        txbuffer[txLen+1] = McuHardware::calculateStopcockUpgradeChecksum(txbuffer, txLen+1);

        rxbuffer[0] = 0;
        rxLen = 1;

        txLen += 2; //For checksum and number of bytes
        int bytesWritten = ftdi_write_data(&curHandle, &txbuffer[0], txLen);
        if (bytesWritten != txLen)
        {
            return QString().asprintf("BLR Failed to send WRITE-DATA Command %d != %d\n", txLen, bytesWritten);
        }

        QThread::msleep(20);

        if (ftdi_read_data(&curHandle, rxbuffer, rxLen) != rxLen)
        {
            return "BLR Failed to read WRITE-DATA Response";
        }

        if (rxbuffer[0] != STM8_ACK)
        {
            return QString().asprintf("BLR Incorrect WRITE-DATA Response(%02x != %02x)\n", rxbuffer[0], STM8_ACK);
        }
        sendAck();
        QThread::msleep(5);

        //Update bytes sent and address
        txLen -= 2; //Remove checksum and number of bytes to update other params
        bytesSent += txLen;
        currentWriteAddress += txLen;
    }

    return "";
}

QString UpgradeHwStopcock::sendGo()
{
    QString err = "";
    int txLen, rxLen;

    txLen = 2;
    txbuffer[0] = STM8_CMD_GO;
    txbuffer[1] = STM8_CMD_GO ^ 0xFF;

    rxbuffer[0] = 0;
    rxLen = 1;

    if (ftdi_write_data(&curHandle, txbuffer, txLen) != txLen)
    {
        err = "Failed to send GO command";
        LOG_ERROR("UPGRADE_HW_STOPCOCK: SendGo: %s\n", err.CSTR());
        return err;
    }

    QThread::msleep(3);

    if (ftdi_read_data(&curHandle, rxbuffer, rxLen) != rxLen)
    {
        err = "Failed to read GO command";
        LOG_ERROR("UPGRADE_HW_STOPCOCK: SendGo: %s\n", err.CSTR());
        return err;
    }

    if (rxbuffer[0] != STM8_ACK)
    {
        err = QString().asprintf("Incorrect GO Response(%02x != %02x)\n", rxbuffer[0], STM8_ACK);
        LOG_ERROR("UPGRADE_HW_STOPCOCK: SendGo: %s\n", err.CSTR());
        return err;
    }
    sendAck();

    txLen = 5;
    txbuffer[0] = 0x00;
    txbuffer[1] = 0x00;
    txbuffer[2] = 0x80;
    txbuffer[3] = 0x00;
    txbuffer[4] = McuHardware::calculateStopcockUpgradeChecksum(txbuffer, 4);

    rxbuffer[0] = 0;
    rxLen = 1;

    if (ftdi_write_data(&curHandle, txbuffer, txLen) != txLen)
    {
        err = "Failed to send GO command";
        LOG_ERROR("UPGRADE_HW_STOPCOCK: SendGo: %s\n", err.CSTR());
        return err;
    }

    QThread::msleep(5);

    if (ftdi_read_data(&curHandle, rxbuffer, rxLen) != rxLen)
    {
        err = "Failed to read GO command";
        LOG_ERROR("UPGRADE_HW_STOPCOCK: SendGo: %s\n", err.CSTR());
        return err;
    }

    if (rxbuffer[0] != STM8_ACK)
    {
        err = QString().asprintf("Incorrect GO Response(%02x != %02x)\n", rxbuffer[0], STM8_ACK);
        LOG_ERROR("UPGRADE_HW_STOPCOCK: SendGo: %s\n", err.CSTR());
        return err;
    }
    sendAck();

    return "";
}

QString UpgradeHwStopcock::sendAck(quint8 ack)
{
    QString err = "";
    int txLen = 1;
    txbuffer[0] = ack;

    if (ftdi_write_data(&curHandle, txbuffer, txLen) != txLen)
    {
        err = "Failed to send ACK Command";
        LOG_ERROR("UPGRADE_HW_STOPCOCK: SendAck: %s\n", err.CSTR());
        return err;
    }

    return err;
}

QString UpgradeHwStopcock::getBootloaderVersion()
{
    QString err = "";
    int txLen, rxLen;
    txLen = 2;
    txbuffer[0] = 0x00;
    txbuffer[1] = 0xFF;

    rxbuffer[0] = 0;
    rxLen = 1;

    if (ftdi_write_data(&curHandle, txbuffer, txLen) != txLen)
    {
        err = "Failed to send GET Command";
        LOG_ERROR("UPGRADE_HW_STOPCOCK: GetBootloaderVersion: %s\n", err.CSTR());
        return err;
    }

    QThread::msleep(5);

    if (ftdi_read_data(&curHandle, rxbuffer, rxLen) != rxLen)
    {
        err = "Failed to read GET Response";
        LOG_ERROR("UPGRADE_HW_STOPCOCK: GetBootloaderVersion: %s\n", err.CSTR());
        return err;
    }

    if (rxbuffer[0] != STM8_ACK)
    {
        err = QString().asprintf("Incorrect GET Response(%02x != %02x)\n", rxbuffer[0], STM8_ACK);
        LOG_ERROR("UPGRADE_HW_STOPCOCK: GetBootloaderVersion: %s\n", err.CSTR());
        return err;

    }

    sendAck();

    // NOTE: Sending ACK for many times: This is due to the nature of the STM8 comm
    int sendAckLimit = 8;
    for (int i = 0; i < sendAckLimit; i++)
    {
        QThread::msleep(1);
        if (ftdi_read_data(&curHandle, rxbuffer, rxLen) != rxLen)
        {
            err = "Failed to read GET Response";
            LOG_ERROR("UPGRADE_HW_STOPCOCK: GetBootloaderVersion: ACK[%d]: %s\n", i, err.CSTR());
            return err;
        }
        sendAck(rxbuffer[0]);
        //LOG_DEBUG("UPGRADE_HW_STOPCOCK: %02x\n", rxbuffer[0]);
    }
    return err;
}

QString UpgradeHwStopcock::endUpgrade()
{
    if (sendGo() == "")
    {
       disconnectDevice();
       QThread::msleep(1000);
       resetHw();
       return "";
    }
    return "Failed SendGo";
}

QString UpgradeHwStopcock::resetHw()
{
    QString err = "";
    if (connectDevice() != "")
    {
        err = "Failed to connect to device";
        return err;
    }

    char mask = SY2_SC_CBUS_RESET_BITS;
    char mode = CBUS_BIT_BANG;

    if (ftdi_set_bitmode(&curHandle, mask, mode) != 0)
    {
        err = "Failed to set RESET bitmode";
        return err;
    }

    QThread::msleep(20);

    mask = SY2_SC_CBUS_RUN_BITS;
    if (ftdi_set_bitmode(&curHandle, mask, mode) != 0)
    {
        err = "Failed to set RUN bitmode";
        return err;
    }

    disconnectDevice();
    QThread::msleep(2000);
    return "";
}

QString UpgradeHwStopcock::startProgramFlash()
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
    case DS_UpgradeDef::UPGRADE_HW_PROGRAM_STATE_ERR_BAD_RX_DATA:
        endUpgrade();
        digest.stopcock.err = "Bad RX Data";
        env->ds.upgradeData->setUpgradeDigest(digest);
        updateUpgradeHwDigest();
        setState(DS_UpgradeDef::UPGRADE_HW_STATE_FAILED);
        break;
    case DS_UpgradeDef::UPGRADE_HW_PROGRAM_STATE_ERR_NO_RX_DATA:
        endUpgrade();
        digest.stopcock.err = "No RX Data";
        env->ds.upgradeData->setUpgradeDigest(digest);
        updateUpgradeHwDigest();
        setState(DS_UpgradeDef::UPGRADE_HW_STATE_FAILED);
        break;
    case DS_UpgradeDef::UPGRADE_HW_PROGRAM_STATE_ERR_TX_DATA:
        endUpgrade();
        digest.stopcock.err = "TX Error";
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

void UpgradeHwStopcock::updateUpgradeHwDigest()
{
    DS_UpgradeDef::UpgradeDigest digest = env->ds.upgradeData->getUpgradeDigest();
    digest.stopcock.progress = (eraseProgress * 0.1) + (loadProgress * 0.9);
    digest.stopcock.progress = (digest.stopcock.progress * retryCount) / retryLimit;

    if (digest.stopcock.progress > 100)
    {
        LOG_ERROR("UPGRADE_HW_STOPCOCK: Bad Progress(%d), eraseProgress=%d, loadProgress=%d, retryCount=%d\n",
                  digest.stopcock.progress, eraseProgress, loadProgress, retryCount);
        digest.stopcock.progress = 100;
    }

    env->ds.upgradeData->setUpgradeDigest(digest);
}

DS_UpgradeDef::UpgradeHwProgramState UpgradeHwStopcock::programImage()
{
    int txLen, rxLen;
    DS_UpgradeDef::UpgradeHwProgramState state;

    if (lstData.length() == 0)
    {
        return DS_UpgradeDef::UPGRADE_HW_PROGRAM_STATE_COMPLETE;
    }

    //Check if the line of data is "blank"
    quint8 highByte = lstAddresses.first().at(2);
    quint8 lowByte = lstAddresses.first().at(3);
    quint16 address = (highByte << 8) + lowByte;

    QByteArray tmpLine = lstData.first();
    if (address < STM8_PROGRAM_MEM_START_ADDRESS)
    {
        if (memcmp(firmwareBlankLine, tmpLine, 16) == 0)
        {
            // Skip this line of data
            lstAddresses.pop_front();
            lstData.pop_front();
            return DS_UpgradeDef::UPGRADE_HW_PROGRAM_STATE_IN_PROGRESS;
        }
    }

    state = sendWriteCommand();
    if (state != DS_UpgradeDef::UPGRADE_HW_PROGRAM_STATE_IN_PROGRESS)
    {
        return state;
    }

    state = sendAddress((quint8 *)lstAddresses.front().data());
    if (state != DS_UpgradeDef::UPGRADE_HW_PROGRAM_STATE_IN_PROGRESS)
    {
        return state;
    }
    lstAddresses.pop_front();

    //Send actual Data
    txbuffer[0] = tmpLine.length()-1;
    rxbuffer[0] = 0;

    txLen = tmpLine.length();

    memcpy(&txbuffer[1], tmpLine,txLen);
    txbuffer[txLen+1] = McuHardware::calculateStopcockUpgradeChecksum(txbuffer, txLen+1);
    rxbuffer[0] = 0;
    rxLen = 1;
    txLen +=2;

    if (ftdi_write_data(&curHandle, txbuffer, txLen) != txLen)
    {
        LOG_ERROR("UPGRADE_HW_STOPCOCK: Failed to send 'Data and checksum'\n");
        return DS_UpgradeDef::UPGRADE_HW_PROGRAM_STATE_ERR_TX_DATA;
    }

    QThread::msleep(150);

    if (ftdi_read_data(&curHandle, rxbuffer, rxLen) != rxLen)
    {
        LOG_ERROR("UPGRADE_HW_STOPCOCK: Failed to read 'Data and checksum' response\n");
        return DS_UpgradeDef::UPGRADE_HW_PROGRAM_STATE_ERR_NO_RX_DATA;
    }

    if (rxbuffer[0] != STM8_ACK)
    {
        LOG_ERROR("UPGRADE_HW_STOPCOCK: Incorrect 'Data and checksum' Response(%02x) != %02x\n", rxbuffer[0], STM8_ACK);
        LOG_ERROR("UPGRADE_HW_STOPCOCK: records left = %d\n", (int)lstData.length());
        return DS_UpgradeDef::UPGRADE_HW_PROGRAM_STATE_ERR_BAD_RX_DATA;
    }
    sendAck();

    txLen -=2;
    currentAddress += txLen;
    lstData.pop_front();
    loadProgress = (int)(100 * ((double)(totalRecords - lstData.length()) / (double)totalRecords));

    if (loadProgress > 100)
    {
        LOG_WARNING("UPGRADE_HW_STOPCOCK: Bad Load Progress(%d), totalRecords=%d, lstData.length=%d\n",
                    loadProgress, totalRecords, (int)lstData.length());
        loadProgress = 100;
    }
    updateUpgradeHwDigest();

    return DS_UpgradeDef::UPGRADE_HW_PROGRAM_STATE_IN_PROGRESS;
}

DS_UpgradeDef::UpgradeHwProgramState UpgradeHwStopcock::sendWriteCommand()
{
    int txLen, rxLen;

    txLen = 2;
    txbuffer[0] = STM8_CMD_WRITE_MEM;
    txbuffer[1] = STM8_CMD_WRITE_MEM ^ 0xFF;

    rxbuffer[0] = 0;
    rxLen = 1;

    if (ftdi_write_data(&curHandle, txbuffer, txLen) != txLen)
    {
        LOG_ERROR("UPGRADE_HW_STOPCOCK: Failed to send write command\n");
        return DS_UpgradeDef::UPGRADE_HW_PROGRAM_STATE_ERR_TX_DATA;
    }

    QThread::msleep(3);

    if (ftdi_read_data(&curHandle, rxbuffer, rxLen) != rxLen)
    {
        LOG_ERROR("UPGRADE_HW_STOPCOCK: Failed to read write response\n");
        return DS_UpgradeDef::UPGRADE_HW_PROGRAM_STATE_ERR_NO_RX_DATA;
    }

    if (rxbuffer[0] != STM8_ACK)
    {
        LOG_ERROR("UPGRADE_HW_STOPCOCK: Incorrect write Response(%02x != %02x)\n", rxbuffer[0], STM8_ACK);
        return DS_UpgradeDef::UPGRADE_HW_PROGRAM_STATE_ERR_BAD_RX_DATA;
    }
    sendAck();

    return DS_UpgradeDef::UPGRADE_HW_PROGRAM_STATE_IN_PROGRESS;
}

DS_UpgradeDef::UpgradeHwProgramState UpgradeHwStopcock::sendAddress(quint8 *address)
{
    int txLen, rxLen;
    txLen = 5;
    txbuffer[0] = address[0];
    txbuffer[1] = address[1];
    txbuffer[2] = address[2];
    txbuffer[3] = address[3];
    txbuffer[4] = McuHardware::calculateStopcockUpgradeChecksum(txbuffer, 4);
    rxbuffer[0] = 0;
    rxLen = 1;

    if (ftdi_write_data(&curHandle, txbuffer, txLen) != txLen)
    {
        LOG_ERROR("UPGRADE_HW_STOPCOCK: Failed to send write address\n");
        return DS_UpgradeDef::UPGRADE_HW_PROGRAM_STATE_ERR_TX_DATA;
    }

    QThread::msleep(50);

    if (ftdi_read_data(&curHandle, rxbuffer, rxLen) != rxLen)
    {
        LOG_ERROR("UPGRADE_HW_STOPCOCK: Failed to read write address response\n");
        return DS_UpgradeDef::UPGRADE_HW_PROGRAM_STATE_ERR_NO_RX_DATA;
    }

    if (rxbuffer[0] != STM8_ACK)
    {
        LOG_ERROR("UPGRADE_HW_STOPCOCK: Incorrect write address Response(%02x != %02x)\n", rxbuffer[0], STM8_ACK);
        return DS_UpgradeDef::UPGRADE_HW_PROGRAM_STATE_ERR_BAD_RX_DATA;
    }
    sendAck();

    return DS_UpgradeDef::UPGRADE_HW_PROGRAM_STATE_IN_PROGRESS;
}
