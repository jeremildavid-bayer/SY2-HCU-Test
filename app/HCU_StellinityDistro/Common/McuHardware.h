#ifndef MCU_HARDWARE_H
#define MCU_HARDWARE_H

#include <QFile>
#include <ftdi.h>
#include <QThread>
#include "Util.h"

// MuxB MuxA
// 0    0       Monitor
// 0    1       Program MCU
// 1    0       Program SC
// 1    1       Unused
#define STELLINITY2_MUX_MONITOR         0x00
#define STELLINITY2_MUX_PROGRAM_MCU     0x01
#define STELLINITY2_MUX_PROGRAM_SC      0x02
#define STELLINITY2_MUX_UNUSED          0x03

// MuxA:    0x01 (Use Table)
// MuxB:    0x02 (Use Table)
// P00:     0x04 (low to programme - MCU)
// Reset:   0x08 (high to reset! - MCU)

#define SY2_CBUS2_P00_MASK              0x04 //Low to program for MCU
#define SY2_CBUS3_RESET_MASK            0x08 //High to reset (toggle for stopcock board)//High to reset for MCU
#define RECMAX                          40 // max elements per S record
#define LINELEN                         ((RECMAX * 2) + 10)

// MCU Specific
#define SY2_MCU_CBUS_RESET_BITS     0xF0 | STELLINITY2_MUX_PROGRAM_MCU | SY2_CBUS3_RESET_MASK | SY2_CBUS2_P00_MASK
#define SY2_MCU_CBUS_BOOT_BITS      0xF0 | STELLINITY2_MUX_PROGRAM_MCU
#define SY2_MCU_CBUS_RUN_BITS       0xF0 | SY2_CBUS2_P00_MASK

// Stopcock Specific
//SY2_CBUS3_RESET_MASK -> low to reset
#define SY2_SC_CBUS_RESET_BITS              0xF0 | STELLINITY2_MUX_PROGRAM_SC | SY2_CBUS2_P00_MASK
#define SY2_SC_CBUS_BOOT_BITS               0xF0 | STELLINITY2_MUX_PROGRAM_SC | SY2_CBUS3_RESET_MASK
#define SY2_SC_CBUS_RUN_BITS                0xF0 | SY2_CBUS2_P00_MASK | SY2_CBUS3_RESET_MASK

#define CBUS_BIT_BANG                       0x20

typedef enum
{
    S19DATA = 0,
    BADADDRESS,
    TOOLONG,
    BADFORMAT,
    BADCRC,
    HEADERRECORD,
    COUNTRECORD,
    ENDRECORD
} S19STAT;

typedef enum
{
    HEX_DATA = 0,
    HEX_EOF,
    HEX_EXT_SEG_ADDR,
    HEX_START_SEG_ADDR,
    HEX_EXT_LINEAR_ADDR,
    HEX_START_LINEAR_ADDR,
    HEX_ERR_BAD_START_CODE,
    HEX_ERR_PARSE_FAIL
} HexFileRecordType;

typedef enum
{
    HW_DEVICE_TYPE_MCU = 0,
    HW_DEVICE_TYPE_SCB
} HardwareDeviceType;

class McuHardware
{
public:

    static QString resetDevice(HardwareDeviceType deviceType)
    {
        ftdi_context *ftdiHandle = ftdi_new();
        QString err = "";
        int vendorId;
        int productId;
        quint8 mask;
        quint8 mode = CBUS_BIT_BANG;
        int errCode = -1;

        switch (deviceType)
        {
        case HW_DEVICE_TYPE_SCB:
            vendorId = FTDI_MCU_VENDOR_ID;
            productId = FTDI_MCU_PRODUCT_ID;
            mask = SY2_SC_CBUS_RESET_BITS;
            break;
        case HW_DEVICE_TYPE_MCU:
        default:
            vendorId = FTDI_STOPCOCK_VENDOR_ID;
            productId = FTDI_STOPCOCK_PRODUCT_ID;
            mask = SY2_MCU_CBUS_RESET_BITS;
            break;
        }

        if (ftdiHandle == NULL)
        {
            return "Failed to init ftdi handle ";
        }

        errCode = ftdi_usb_open(ftdiHandle, vendorId, productId);
        if (errCode != 0)
        {
            err = QString().asprintf("Failed to open usb, err=%d, %s ", errCode, ftdiHandle->error_str);
            goto bail;
        }

        if (ftdi_set_bitmode(ftdiHandle, mask, mode) != 0)
        {
            err = "Cannot set bitmode (RESET BITS) ";
            goto bail;
        }

        QThread::msleep(100);

        switch (deviceType)
        {
        case HW_DEVICE_TYPE_SCB:
            mask = SY2_SC_CBUS_RUN_BITS;
            break;
        case HW_DEVICE_TYPE_MCU:
        default:
            mask = SY2_MCU_CBUS_RUN_BITS;
            break;
        }

        if (ftdi_set_bitmode(ftdiHandle, mask, mode) != 0)
        {
            err = "Cannot set bitmode (RUN BITS) ";
            //Continue and deinit handle
        }

     bail:
        if (errCode == 0)
        {
            ftdi_usb_reset(ftdiHandle);
            errCode = ftdi_usb_close(ftdiHandle);
            if (errCode != 0)
            {
                err += QString().asprintf("Failed to close usb, err(%d)\n", errCode);
                //Continue and deinit handle
            }
        }

        //char * errorororor = usb_strerror();
        ftdi_free(ftdiHandle);

        QThread::msleep(500);

        return err;
    }

    static quint8 calculateMcuUpgradeChecksum(quint8 *buffer, int size)
    {
        quint8 xsum = 0;

        while (size--)
        {
            xsum += *buffer++;
        }
        return (quint8)(0x100 - xsum);
    }

    static quint8 calculateStopcockUpgradeChecksum(quint8 *buffer, int size)
    {
        quint8 xsum = *buffer++;
        size--;

        while (size--)
        {
            xsum = xsum ^ *buffer++;
        }

        return xsum;
    }

    // Read S19 record into sFormatBuf here
    static S19STAT processS19rec(char *buffp, quint8 *Timage, quint32 AddrMin, quint32 AddrMax, quint32 *pmaxAddress)
    {
        bool status = false;
        int checksum;
        char *bp;
        unsigned long reclen;
        unsigned long recaddr;
        unsigned long data;
        int dataoffset;

        if (buffp[0] != 'S')
        {
            return BADFORMAT;  /* invalid record ??? */
        }

        if (buffp[1] == '0')
            return HEADERRECORD;

        if (buffp[1] == '5')
            return COUNTRECORD;

        if ((buffp[1] == '9') || (buffp[1] == '8') || (buffp[1] == '7'))
            return ENDRECORD;

        if (buffp[1] == '1')
            dataoffset = 8;
        else
            if (buffp[1] == '2')
                dataoffset = 10;
            else
                if (buffp[1] == '3')
                    dataoffset = 12;
                else
                {
                    return BADFORMAT;  /* invalid record ??? */
                }

        // we have an Sx record here
        if (!Util::asc2hex2(&buffp[2], &reclen))
        {
            return BADFORMAT;
        }

        checksum = (int)reclen;

        if (reclen > RECMAX)
            return TOOLONG;

        switch (dataoffset)
        {
        case 8: // 16 bit address
            status = Util::asc2hex4(&buffp[4], &recaddr);
            reclen -= 3L;
            break;

        case 10: // 24 bit address
            status = Util::asc2hex6(&buffp[4], &recaddr);
            reclen -= 4L;
            break;

        case 12: // 32 bit address
            status = Util::asc2hex8(&buffp[4], &recaddr);
            reclen -= 5L;
            break;
        }

        if (!status)
        {
            return BADFORMAT;
        }

        if (recaddr)
        {
            unsigned long temp = (unsigned long)AddrMax + 1;
            //unsigned long temp2 = AddrMin;
            if ((recaddr < AddrMin) || ((recaddr + reclen) > temp))
            {
                return BADADDRESS;
            }
        }

        *pmaxAddress = recaddr + reclen - 1;
        checksum += (int)recaddr & 0xff;
        checksum += (int)(recaddr >> 8) & 0xff;
        checksum += (int)(recaddr >> 16) & 0xff;
        checksum += (int)(recaddr >> 24) & 0xff;

        bp = &buffp[dataoffset];

        // copy the data
        while (reclen--)
        {
            if (!Util::asc2hex2(bp, &data))
            {
                return BADFORMAT;
            }

            checksum += (int)data;
            Timage[recaddr - AddrMin] = (quint8)data;
            recaddr++;
            bp += 2;
        }

        /* read and compare checksum here
       */
        if (!Util::asc2hex2(bp, &data))
        {
            return BADFORMAT;
        }

        if ((~checksum & 0xff) != (int)data)
            return BADCRC;

        return S19DATA;
    }

    static S19STAT getSRecords(QFile &fileBuf, quint8 *LoadBuff, quint32 AddrMin, quint32 AddrMax, quint32 *pmaxAddress)
    {
        S19STAT stat;
        char sFormatBuf[LINELEN + 1];

        *pmaxAddress = AddrMin;
        quint32 MAddr = AddrMin;

        while (fileBuf.readLine(sFormatBuf, sizeof(sFormatBuf)) != -1)
        {
            switch (stat = processS19rec(sFormatBuf, LoadBuff, AddrMin, AddrMax, &MAddr))
            {
            case S19DATA:
            case HEADERRECORD:
            case COUNTRECORD:
                break;
            case BADADDRESS:
            case TOOLONG:
            case BADFORMAT:
            case BADCRC:
                //LOG_DEBUG("ERR: getSRecords(): %d\n", stat);
            case ENDRECORD:
                return stat;
            }

            // keep track of the maximum address
            if (MAddr > *pmaxAddress)
            {
                *pmaxAddress = MAddr;
            }
        }

        return ENDRECORD;
    }
};



#endif // MCU_HARDWARE_H
