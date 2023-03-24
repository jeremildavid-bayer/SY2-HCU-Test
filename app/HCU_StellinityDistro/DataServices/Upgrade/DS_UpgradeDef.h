#ifndef DS_UPGRADE_DEF_H
#define DS_UPGRADE_DEF_H

#include "Common/Common.h"

class DS_UpgradeDef
{
public:
    // ==============================================
    // Enumerations

    enum UpgradeState
    {
        STATE_UNKNOWN = 0,
        STATE_READY,
        STATE_GET_PACKAGE_INFO_STARTED,
        STATE_GET_PACKAGE_INFO_PROGRESS,
        STATE_GET_PACKAGE_INFO_DONE,
        STATE_GET_PACKAGE_INFO_FAILED,
        STATE_UPGRADE_STARTED,
        STATE_CHECK_PACKAGE_STARTED,
        STATE_CHECK_PACKAGE_DONE,
        STATE_CHECK_PACKAGE_FAILED,
        STATE_EXTRACT_PACKAGE_STARTED,
        STATE_EXTRACT_PACKAGE_PROGRESS,
        STATE_EXTRACT_PACKAGE_DONE,
        STATE_EXTRACT_PACKAGE_FAILED,
        STATE_INSTALL_STOPCOCK_STARTED,
        STATE_INSTALL_STOPCOCK_PROGRESS,
        STATE_INSTALL_STOPCOCK_DONE,
        STATE_INSTALL_STOPCOCK_FAILED,
        STATE_INSTALL_MCU_STARTED,
        STATE_INSTALL_MCU_PROGRESS,
        STATE_INSTALL_MCU_DONE,
        STATE_INSTALL_MCU_FAILED,
        STATE_INSTALL_HCU_STARTED,
        STATE_INSTALL_HCU_PROGRESS,
        STATE_INSTALL_HCU_DONE,
        STATE_INSTALL_HCU_FAILED,
        STATE_UPGRADE_DONE,
        STATE_UPGRADE_FAILED,
    };

    enum UpgradeHwState
    {
        UPGRADE_HW_STATE_READY = 0,
        UPGRADE_HW_STATE_STARTED,
        UPGRADE_HW_STATE_ERASE_STARTED,
        UPGRADE_HW_STATE_ERASING,
        UPGRADE_HW_STATE_VERIFYING_ERASED_FLASH,
        UPGRADE_HW_STATE_INITIALISING,
        UPGRADE_HW_STATE_PROGRAMMING_FLASH,
        UPGRADE_HW_STATE_COMPLETED,
        UPGRADE_HW_STATE_FAILED
    };

    enum UpgradeHwCompleteState
    {
        UPGRADE_HW_COMPLETE_STATE_OK = 0,
        UPGRADE_HW_COMPLETE_STATE_INIT_ERR,
        UPGRADE_HW_COMPLETE_STATE_ERASE_FLASH_ERR,
        UPGRADE_HW_COMPLETE_STATE_PROGRAM_ERR,
        UPGRADE_HW_COMPLETE_STATE_CONNECT_ERR,
        UPGRADE_HW_COMPLETE_STATE_IMAGE_LOAD_ERR,
        UPGRADE_HW_COMPLETE_STATE_TX_ERR,
        UPGRADE_HW_COMPLETE_STATE_NO_RX_DATA_ERR,
        UPGRADE_HW_COMPLETE_STATE_BAD_RX_DATA_ERR,
        UPGRADE_HW_COMPLETE_STATE_FILE_NOT_FOUND_ERR
    };

    enum UpgradeHwProgramState
    {
        UPGRADE_HW_PROGRAM_STATE_COMPLETE = 0,
        UPGRADE_HW_PROGRAM_STATE_IN_PROGRESS,
        UPGRADE_HW_PROGRAM_STATE_ERR_NO_RX_DATA,
        UPGRADE_HW_PROGRAM_STATE_ERR_BAD_RX_DATA,
        UPGRADE_HW_PROGRAM_STATE_ERR_TX_DATA
    };

    // ==============================================
    // Data Structures

    struct UpgradeStatus
    {
        QString err;
        QString pathFile;
        qint64 fileSizeKB;
        int progress;

        bool operator==(const UpgradeStatus &arg) const
        {
            bool equal = ( (err == arg.err) &&
                           (pathFile == arg.pathFile) &&
                           (fileSizeKB == arg.fileSizeKB) &&
                           (progress == arg.progress) );
            return equal;
        }

        bool operator!=(const UpgradeStatus &arg) const
        {
            return !operator==(arg);
        }
    };

    struct UpgradeDigest
    {
        UpgradeState state;
        UpgradeStatus sru;
        UpgradeStatus hcu;
        UpgradeStatus mcu;
        UpgradeStatus stopcock;

        UpgradeDigest()
        {
            state = DS_UpgradeDef::STATE_UNKNOWN;

            sru.err = "";
            sru.pathFile = "";
            sru.fileSizeKB = 0;
            sru.progress = 0;

            hcu.err = "";
            hcu.pathFile = "";
            hcu.fileSizeKB = 0;
            hcu.progress = 0;

            mcu.err = "";
            mcu.pathFile = "";
            mcu.fileSizeKB = 0;
            mcu.progress = 0;

            stopcock.err = "";
            stopcock.pathFile = "";
            stopcock.fileSizeKB = 0;
            stopcock.progress = 0;
        }

        bool operator==(const UpgradeDigest &arg)
        {
            bool equal = ( (state == arg.state) &&
                           (sru == arg.sru) &&
                           (hcu == arg.hcu) &&
                           (mcu == arg.mcu) &&
                           (stopcock == arg.stopcock) );
            return equal;
        }

        bool operator!=(const UpgradeDigest &arg)
        {
            return !operator==(arg);
        }
    };
};


Q_DECLARE_METATYPE(DS_UpgradeDef::UpgradeDigest);
#endif // DS_UPGRADE_DEF_H
