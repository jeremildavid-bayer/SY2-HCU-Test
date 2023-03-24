#ifndef UPGRADE_HW_MCU_H
#define UPGRADE_HW_MCU_H

#include <ftdi.h>
#include "Common/Common.h"
#include "DataServices/Upgrade/DS_UpgradeDef.h"
#include "UpgradeHwBase.h"

#define MAX_BUFF_SZ             256 // make sure is at least maximum line size for the device!!
#define BUFFERSZ                (1 + 4 + MAX_BUFF_SZ + 1) // command byte + address + data + xsum
#define IMAGE_SIZE              (512 * 1024)

class UpgradeHwMcu : public UpgradeHwBase
{
public:
    explicit UpgradeHwMcu(QObject *parent = 0, EnvGlobal *env_ = NULL, EnvLocal *envLocal = NULL);
    ~UpgradeHwMcu();

private:
    ftdi_context curHandle;
    quint8 txbuffer[BUFFERSZ];
    quint8 rxbuffer[BUFFERSZ];
    quint8 image[IMAGE_SIZE];
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
    QString setPortSettings(int baudRate);
    QString setToBootMode();
    QString releaseBootMode();
    QString startUpgradeInner(QFile &fileBuf);
    QString initialiseUpgrade();
    QString endUpgrade();
    QString loadImage(QFile &fileBuf);
    QString eraseFlash();
    QString verifyErasedFlash();
    QString startProgramFlash();
    QString acknowledgeTransferBitRate();
    QString setTransferBitRate();
    QString getBootResponse(quint8 *buffer, quint8 responseCode);
    QString sendBootCommand(quint8 *buffer);
    void setReadWriteTimeout(int readTimeout, int writeTimeout);
    void updateUpgradeHwDigest();
};

#endif // UPGRADE_HW_MCU_H
