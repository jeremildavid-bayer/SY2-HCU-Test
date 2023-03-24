#include "Apps/AppManager.h"
#include "DeviceWasteContainer.h"
#include "DataServices/Workflow/DS_WorkflowData.h"
#include "DataServices/Device/DS_DeviceData.h"
#include "DataServices/Mcu/DS_McuData.h"
#include "DataServices/Mcu/Internal/McuAlarm.h"
#include "DataServices/Alert/DS_AlertAction.h"
#include "Common/ImrParser.h"

DeviceWasteContainer::DeviceWasteContainer(QObject *parent, EnvGlobal *env_) :
    ActionBase(parent, env_)
{
    envLocal = new EnvLocal("DS_Device-WasteContainer", "DEVICE_WC");

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });
}

DeviceWasteContainer::~DeviceWasteContainer()
{
    delete envLocal;
}

void DeviceWasteContainer::slotAppInitialised()
{
    connect(env->ds.systemData, &DS_SystemData::signalDataChanged_StatePath, this, [=](DS_SystemDef::StatePath curStatePath, DS_SystemDef::StatePath prevStatePath) {
        if ( ( (prevStatePath == DS_SystemDef::STATE_PATH_IDLE) && (curStatePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING) ) ||
             ( (prevStatePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING) && (curStatePath == DS_SystemDef::STATE_PATH_IDLE) ) )
        {
            LOG_INFO("\n\n-----------------\nStatePath=%s\n\n", ImrParser::ToImr_StatePath(curStatePath).CSTR());
        }
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_WasteBinState, this, [=](DS_McuDef::WasteBinState state, DS_McuDef::WasteBinState prevState) {
        updateAlerts(state, prevState);
        updateFluidSource();
    });
}

void DeviceWasteContainer::updateAlerts(DS_McuDef::WasteBinState state, DS_McuDef::WasteBinState prevState)
{
    static DS_McuDef::WasteBinState lastKnownState = DS_McuDef::WASTE_BIN_STATE_UNKNOWN;

    // Filter the comm down state
    if (prevState == DS_McuDef::WASTE_BIN_STATE_COMM_DOWN)
    {
        prevState = lastKnownState;
    }
    else
    {
        lastKnownState = prevState;
    }

    if (state == prevState)
    {
        return;
    }

    if ( (state == DS_McuDef::WASTE_BIN_STATE_UNKNOWN) ||
         (state == DS_McuDef::WASTE_BIN_STATE_COMM_DOWN) )
    {
        return;
    }

    LOG_INFO("updateAlerts(): WC State changed from %s to %s\n", ImrParser::ToImr_WasteBinState(prevState).CSTR(), ImrParser::ToImr_WasteBinState(state).CSTR());

    if (state == DS_McuDef::WASTE_BIN_STATE_MISSING)
    {
        if ( (prevState == DS_McuDef::WASTE_BIN_STATE_LOW_FILLED) ||
             (prevState == DS_McuDef::WASTE_BIN_STATE_HIGH_FILLED) ||
             (prevState == DS_McuDef::WASTE_BIN_STATE_FULLY_FILLED) )
        {
            // WC is removed
            DS_DeviceDef::WasteContainerLevel prevLevel = getLevelFromState(prevState);
            env->ds.alertAction->activate("SRURemovedWasteContainer", QString().asprintf("%d", prevLevel));
        }

        env->ds.alertAction->activate("WasteContainerMissing");
        env->ds.alertAction->deactivate("WasteContainerFull");
    }
    else if ( (state == DS_McuDef::WASTE_BIN_STATE_LOW_FILLED) ||
              (state == DS_McuDef::WASTE_BIN_STATE_HIGH_FILLED) ||
              (state == DS_McuDef::WASTE_BIN_STATE_FULLY_FILLED) )
    {
        env->ds.alertAction->deactivate("WasteContainerMissing");

        if (state == DS_McuDef::WASTE_BIN_STATE_FULLY_FILLED)
        {
            env->ds.alertAction->activate("WasteContainerFull");
        }
        else
        {
            env->ds.alertAction->deactivate("WasteContainerFull");
        }

        if (prevState == DS_McuDef::WASTE_BIN_STATE_MISSING)
        {
            // WC is inserted
            env->ds.alertAction->activate("SRUInsertedWasteContainer");
        }
    }
}


void DeviceWasteContainer::updateFluidSource()
{
    // update fluid source
    DS_McuDef::WasteBinState state = env->ds.mcuData->getWasteBinState();
    DS_DeviceDef::WasteContainerLevel level = getLevelFromState(state);
    DS_DeviceDef::FluidSource fluidSourceWasteContainer = env->ds.deviceData->getFluidSourceWasteContainer();
    bool newInstalledState = false;

    switch (state)
    {
    case DS_McuDef::WASTE_BIN_STATE_LOW_FILLED:
    case DS_McuDef::WASTE_BIN_STATE_HIGH_FILLED:
        newInstalledState = true;
        fluidSourceWasteContainer.isReady = true;
        fluidSourceWasteContainer.needsReplaced = false;
        break;
    case DS_McuDef::WASTE_BIN_STATE_FULLY_FILLED:
        newInstalledState = true;
        fluidSourceWasteContainer.isReady = false;
        fluidSourceWasteContainer.needsReplaced = true;
        break;
    case DS_McuDef::WASTE_BIN_STATE_MISSING:
    default:
        break;
    }

    if (fluidSourceWasteContainer.isInstalled() != newInstalledState)
    {
        fluidSourceWasteContainer.setIsInstalled(newInstalledState);
    }

    fluidSourceWasteContainer.currentVolumes[0] = level;
    env->ds.deviceData->setFluidSourceWasteContainer(fluidSourceWasteContainer);
}

DS_DeviceDef::WasteContainerLevel DeviceWasteContainer::getLevelFromState(DS_McuDef::WasteBinState state)
{
    switch (state)
    {
    case DS_McuDef::WASTE_BIN_STATE_COMM_DOWN:
        return DS_DeviceDef::WASTE_CONTAINER_LEVEL_UNKNOWN_COMM_DOWN;
    case DS_McuDef::WASTE_BIN_STATE_FULLY_FILLED:
        return DS_DeviceDef::WASTE_CONTAINER_LEVEL_FULL;
    case DS_McuDef::WASTE_BIN_STATE_HIGH_FILLED:
        return DS_DeviceDef::WASTE_CONTAINER_LEVEL_HIGH;
    default:
        return DS_DeviceDef::WASTE_CONTAINER_LEVEL_LOW;
    }
}
