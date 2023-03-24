#include "Apps/AppManager.h"
#include "McuMonitor.h"
#include "DataServices/Mcu/DS_McuAction.h"
#include "DataServices/Mcu/DS_McuData.h"
#include "DataServices/System/DS_SystemData.h"
#include "DataServices/Alert/DS_AlertAction.h"
#include "DataServices/HardwareInfo/DS_HardwareInfo.h"
#include "Common/ImrParser.h"

McuMonitor::McuMonitor(QObject *parent, EnvGlobal *env_) :
    QObject(parent),
    env(env_)
{
    envLocal = new EnvLocal("DS_Mcu-Monitor", "MCU_MONITOR");

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });
}

McuMonitor::~McuMonitor()
{
    tmrMcuCommMonitor.stop();
    delete envLocal;
}

void McuMonitor::slotAppInitialised()
{
    connect(env->ds.systemData, &DS_SystemData::signalDataChanged_StatePath, this, [=](DS_SystemDef::StatePath curStatePath, DS_SystemDef::StatePath prevStatePath) {
        if ( ( (prevStatePath == DS_SystemDef::STATE_PATH_IDLE) && (curStatePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING) ) ||
             ( (prevStatePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING) && (curStatePath == DS_SystemDef::STATE_PATH_IDLE) ) )
        {
            LOG_INFO("\n\n-----------------\nStatePath=%s\n\n", ImrParser::ToImr_StatePath(curStatePath).CSTR());
        }
    });

    connect(&tmrMcuCommMonitor, &QTimer::timeout, this, [=] {
        if (env->state == EnvGlobal::STATE_EXITING)
        {
            return;
        }

        DS_McuDef::LinkState linkState = env->ds.mcuData->getLinkState();
        if (linkState != DS_McuDef::LINK_STATE_CONNECTED)
        {
            LOG_ERROR("MCU comm has been down for %dms\n", MCU_COMM_CONNECT_TIMEOUT_MS);
            env->ds.alertAction->activate("HCULostCommWithMCU");
        }
    });
    tmrMcuCommMonitor.start(MCU_COMM_CONNECT_TIMEOUT_MS);

    connect(env->ds.systemData, &DS_SystemData::signalDataChanged_StatePath, this, [=](DS_SystemDef::StatePath curStatePath, DS_SystemDef::StatePath prevStatePath) {
        if ( (prevStatePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING) &&
             (curStatePath == DS_SystemDef::STATE_PATH_IDLE) )
        {
            LOG_INFO("Service mode exit. Re-emiting MCU data..\n");

            env->ds.mcuAction->actRestoreLastMcuDigest();

            DS_McuDef::LinkState mcuLinkState = env->ds.mcuData->getLinkState();
            if (mcuLinkState == DS_McuDef::LINK_STATE_CONNECTED)
            {
                // If MCU still connected, notify for all data changed.
                // NOTE: If MCU is not connected, do nothing. Notification shall be performed when the MCU link is up again.
                env->ds.mcuData->emitAllDataChanged();
            }
        }
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_LinkState, this, [=](DS_McuDef::LinkState linkState, DS_McuDef::LinkState prevLinkState) {
        if (prevLinkState == DS_McuDef::LINK_STATE_UNKNOWN)
        {
            return;
        }

        if (linkState == DS_McuDef::LINK_STATE_CONNECTED)
        {
            checkMcuVersion();

            tmrMcuCommMonitor.stop();
            env->ds.alertAction->deactivate("HCULostCommWithMCU");

            // Emit All MCU Data Signals after all devices are activated
            QTimer::singleShot(MCU_TX_POLL_MS, this, [=](){
                env->ds.mcuData->emitAllDataChanged();
            });
        }
        else if ( (prevLinkState == DS_McuDef::LINK_STATE_CONNECTED) &&
                  (linkState == DS_McuDef::LINK_STATE_DISCONNECTED) )
        {
            env->ds.alertAction->activate("HCULostCommWithMCU");
        }
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_OutletDoorState, this, [=](DS_McuDef::OutletDoorState state, DS_McuDef::OutletDoorState prevState) {
        if (env->ds.systemData->getStatePath() == DS_SystemDef::STATE_PATH_BUSY_SERVICING)
        {
            return;
        }

        if ( (state == prevState) ||
             (prevState == DS_McuDef::OUTLET_DOOR_STATE_UNKNOWN) )
        {
            // no transition
        }
        else if (state == DS_McuDef::OUTLET_DOOR_STATE_OPEN)
        {
            env->ds.alertAction->activate("SRUOutletAirDoorOpened");
        }
        else if (state == DS_McuDef::OUTLET_DOOR_STATE_CLOSED)
        {
            env->ds.alertAction->activate("SRUOutletAirDoorClosed");
        }
    });

    connect(env->ds.hardwareInfo, &DS_HardwareInfo::signalConfigChanged_General_BaseBoardType, this, [=] {
        if (env->ds.mcuData->getLinkState() == DS_McuDef::LINK_STATE_CONNECTED)
        {
            env->ds.mcuAction->actSetBaseType();
        }
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_BMSDigests, this, [=](const DS_McuDef::BMSDigests &bmsDigests) {
       // Get Battery Capacity
        quint16 chargeA = bmsDigests[0].sbsStatus.relativeStateOfCharge;
        quint16 chargeB = bmsDigests[1].sbsStatus.relativeStateOfCharge;
        quint16 capacityLevel = 0;

        if ( (chargeA > 0) && (chargeB > 0) )
        {
            capacityLevel = (chargeA + chargeB) / 2;
        }
        else if (chargeA > 0)
        {
            capacityLevel = chargeA;
        }
        else if (chargeB > 0)
        {
            capacityLevel = chargeB;
        }

        // Battery % translated to  0-100% range from 15-95% range using linear regression
        // Please refer to https://bayer.codefactori.com/jira/browse/S2SRUSW-3053 for more details
        DS_McuDef::PowerStatus powerStatus = env->ds.mcuData->getPowerStatus();
        double newBatteryCharge = ::round((1.25 * capacityLevel) - 18.75);
        newBatteryCharge = qMax(0.0, newBatteryCharge);
        newBatteryCharge = qMin(100.0, newBatteryCharge);

        if (newBatteryCharge != powerStatus.batteryCharge)
        {
            LOG_INFO("signalDataChanged_BMSDigests(): chargeA(%u%%), chargeB(%u%%): Battery Charge is changed from %.1f%% to %.1f%%\n",
                     chargeA, chargeB,
                     powerStatus.batteryCharge, newBatteryCharge);
            powerStatus.batteryCharge = newBatteryCharge;
            env->ds.mcuData->setPowerStatus(powerStatus);
        }
    });
}

void McuMonitor::checkMcuVersion()
{
    DS_McuDef::LinkState mcuLinkState = env->ds.mcuData->getLinkState();

    if (mcuLinkState != DS_McuDef::LINK_STATE_CONNECTED)
    {
        // MCU is not connected yet
        LOG_INFO("Failed to verify MCU version. MCU link state=%s\n", ImrParser::ToImr_McuLinkState(mcuLinkState).CSTR());
        return;
    }

    QString mcuVersion = env->ds.mcuData->getMcuVersion();
    QString expectedMcuVersionPrefix = env->ds.systemData->getCompatibleMcuVersionPrefix();

    if (mcuVersion.contains(expectedMcuVersionPrefix))
    {
        LOG_INFO("CheckMcuVersion(): Expected MCU Version Found: %s\n", mcuVersion.CSTR());
        env->ds.alertAction->deactivate("IncompatibleMCUConnected");
    }
    else
    {
        LOG_ERROR("CheckMcuVersion(): MCU Version Mismatch. Expected(%s*), Actual(%s)\n", expectedMcuVersionPrefix.CSTR(), mcuVersion.CSTR());
        env->ds.alertAction->activate("IncompatibleMCUConnected");
    }

    QString mcuCommandVersion = env->ds.mcuData->getMcuCommandVersion();
    QString expectedMcuCommandVersion = env->ds.systemData->getCompatibleMcuCommandVersion();

    if (mcuCommandVersion != expectedMcuCommandVersion)
    {
        QString err = QString().asprintf("MCU Command Version Mismatch. Expected(%s*), Actual(%s)\n", expectedMcuCommandVersion.CSTR(), mcuCommandVersion.CSTR());
        LOG_ERROR("CheckMcuVersion(): %s", err.CSTR());
        env->ds.alertAction->activate("HCUInternalSoftwareError", err);
    }
}
