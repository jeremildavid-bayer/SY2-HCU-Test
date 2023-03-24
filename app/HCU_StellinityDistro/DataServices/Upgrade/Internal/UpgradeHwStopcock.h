#ifndef UPGRADE_HW_STOPCOCK_H
#define UPGRADE_HW_STOPCOCK_H

#include <ftdi.h>
#include "Common/Common.h"
#include "DataServices/Upgrade/DS_UpgradeDef.h"
#include "UpgradeHwBase.h"
#include "Common/McuHardware.h"

#define MAX_BUFF_SZ             256 // make sure is at least maximum line size for the device!!
#define BUFFERSZ                (1 + 4 + MAX_BUFF_SZ + 1) // command byte + address + data + xsum
#define STM8_ACK                0x79
#define STM8_NACK               0x1F

class UpgradeHwStopcock : public UpgradeHwBase
{
public:
    explicit UpgradeHwStopcock(QObject *parent = 0, EnvGlobal *env_ = NULL, EnvLocal *envLocal = NULL);
    ~UpgradeHwStopcock();

private:
    ftdi_context curHandle;
    quint8 txbuffer[BUFFERSZ];
    quint8 rxbuffer[BUFFERSZ];
    QList<QByteArray> lstData;
    QList<QByteArray> lstAddresses;
    bool isBoot;

    int recordsWritten;
    int totalRecords;
    quint32 flashStart;
    quint32 flashEnd;
    quint32 currentAddress;
    quint32 maxAddress;

    DS_UpgradeDef::UpgradeHwProgramState programImage();
    QString connectDevice();
    QString disconnectDevice();
    QString startUpgradeInner(QFile &fileBuf);
    QString initialiseUpgrade();
    QString endUpgrade();
    QString loadImage(QFile &fileBuf);
    QString eraseFlash();
    QString verifyErasedFlash();
    QString startProgramFlash();
    void updateUpgradeHwDigest();
    HexFileRecordType loadImageFile(char * buffp, quint32 AddrMin, quint32 AddrMax, quint32 *pMaxAddr);
    QString writeBootloaderRoutines();
    QString sendGo();
    QString sendAck(quint8 ack = STM8_ACK);
    QString getBootloaderVersion();
    QString resetHw();
    DS_UpgradeDef::UpgradeHwProgramState sendWriteCommand();
    DS_UpgradeDef::UpgradeHwProgramState sendAddress(quint8 *address);
};

#endif // UPGRADE_HW_STOPCOCK_H
