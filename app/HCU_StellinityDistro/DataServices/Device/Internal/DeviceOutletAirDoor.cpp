#include "Apps/AppManager.h"
#include "DeviceOutletAirDoor.h"
#include "DataServices/Workflow/DS_WorkflowData.h"
#include "DataServices/Device/DS_DeviceAction.h"
#include "DataServices/Mcu/DS_McuData.h"
#include "DataServices/Mcu/DS_McuAction.h"
#include "DataServices/Alert/DS_AlertAction.h"
#include "Common/Util.h"

DeviceOutletAirDoor::DeviceOutletAirDoor(QObject *parent, EnvGlobal *env_) :
    ActionBase(parent, env_)
{
    envLocal = new EnvLocal("DS_Device-OutletAirDoor", "DEVICE_OUTLET_AIR_DOOR");
    isActive = false;

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });
}

DeviceOutletAirDoor::~DeviceOutletAirDoor()
{
    delete envLocal;
}

void DeviceOutletAirDoor::slotAppInitialised()
{
    connect(env->ds.systemData, &DS_SystemData::signalDataChanged_StatePath, this, [=](DS_SystemDef::StatePath curStatePath, DS_SystemDef::StatePath prevStatePath) {
        if ( ( (prevStatePath == DS_SystemDef::STATE_PATH_IDLE) && (curStatePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING) ) ||
             ( (prevStatePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING) && (curStatePath == DS_SystemDef::STATE_PATH_IDLE) ) )
        {
            LOG_INFO("\n\n-----------------\nStatePath=%s\n\n", ImrParser::ToImr_StatePath(curStatePath).CSTR());
        }
    });

    connect(env->ds.workflowData, &DS_WorkflowData::signalDataChanged_WorkflowState, this, [=](DS_WorkflowDef::WorkflowState workflowState, DS_WorkflowDef::WorkflowState workflowStatePrev) {
        if ( (workflowState != DS_WorkflowDef::STATE_INACTIVE) &&
             (workflowStatePrev == DS_WorkflowDef::STATE_INACTIVE) )
        {
            isActive = true;
            setAlert();
            setLed();
        }
        else if ( (workflowState == DS_WorkflowDef::STATE_INACTIVE) &&
                  (workflowStatePrev != DS_WorkflowDef::STATE_INACTIVE) )
        {
            isActive = false;
        }
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_LinkState, this, [=] {
        if (!isActive)
        {
            return;
        }
        setLed();
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_OutletDoorState, this, [=] {
        if (!isActive)
        {
            return;
        }

        setLed();
        setAlert();
    });

    connect(env->ds.mcuData , &DS_McuData::signalDataChanged_DoorState, this, [=] {
        if (!isActive)
        {
            return;
        }

        setLed();
        setAlert();
    });

    connect(env->ds.mcuData , &DS_McuData::signalDataChanged_MudsInserted, this, [=] {
        if (!isActive)
        {
            return;
        }
        setLed();
        setAlert();
    });

    connect(env->ds.workflowData, &DS_WorkflowData::signalDataChanged_MudsSodStatus, this, [=] {
        if (!isActive)
        {
            return;
        }
        setLed();
    });

    connect(env->ds.mcuData , &DS_McuData::signalDataChanged_SudsInserted, this, [=] {
        if (!isActive)
        {
            return;
        }

        setAlert();
    });
}

void DeviceOutletAirDoor::setAlert()
{
    DS_McuDef::DoorState doorState = env->ds.mcuData->getDoorState();
    DS_McuDef::OutletDoorState outletDoorState = env->ds.mcuData->getOutletDoorState();
    bool mudsInserted = env->ds.mcuData->getMudsInserted();
    bool sudsInserted = env->ds.mcuData->getSudsInserted();

    if ( (outletDoorState == DS_McuDef::OUTLET_DOOR_STATE_OPEN) &&
         (mudsInserted) &&
         ( (doorState == DS_McuDef::DOOR_CLOSED) || (sudsInserted) ) )
    {
        if (!env->ds.alertAction->isActivated("OutletAirDoorLeftOpen"))
        {
            env->ds.alertAction->activate("OutletAirDoorLeftOpen");
        }
    }
    else
    {
        if (env->ds.alertAction->isActivated("OutletAirDoorLeftOpen"))
        {
            env->ds.alertAction->deactivate("OutletAirDoorLeftOpen");
        }
    }
}

void DeviceOutletAirDoor::setLed()
{
    static QString lastLedChangeReason = "";

    DS_McuDef::LinkState mcuLinkState = env->ds.mcuData->getLinkState();
    DS_McuDef::DoorState doorState = env->ds.mcuData->getDoorState();
    DS_McuDef::OutletDoorState outletDoorState = env->ds.mcuData->getOutletDoorState();
    bool mudsInserted = env->ds.mcuData->getMudsInserted();
    DS_WorkflowDef::MudsSodStatus mudsSodStatus = env->ds.workflowData->getMudsSodStatus();

    DS_McuDef::ActLedParams params;
    params.isFlashing = false;

    QString ledChangeReason;

    if (mcuLinkState != DS_McuDef::LINK_STATE_CONNECTED)
    {
        ledChangeReason = "MCU is not connected: LED off";
        params.setColorOff();
    }
    else if (doorState == DS_McuDef::DOOR_CLOSED)
    {
        ledChangeReason = "Main door is closed: LED off";
        params.setColorOff();
    }
    else if (!mudsInserted)
    {
        ledChangeReason = "MUDS is not inserted";
        params.setColorOff();
    }
    else if ( (!mudsSodStatus.syringeSodStatusAll[SYRINGE_IDX_SALINE].plungerEngaged) ||
              (!mudsSodStatus.syringeSodStatusAll[SYRINGE_IDX_CONTRAST1].plungerEngaged) ||
              (!mudsSodStatus.syringeSodStatusAll[SYRINGE_IDX_CONTRAST2].plungerEngaged) )
    {
        ledChangeReason = "Pistons not engaged: LED off";
        params.setColorOff();
    }
    else if (outletDoorState == DS_McuDef::OUTLET_DOOR_STATE_OPEN)
    {
        ledChangeReason = "Outlet Door is open: flashing orange";
        params.setColorOrange();
        params.isFlashing = true;
    }
    else
    {
        ledChangeReason = "Outlet Door is closed: solid green";
        params.setColorGreen();
    }

    if (ledChangeReason != lastLedChangeReason)
    {
        LOG_DEBUG("setLed(): Reason=%s, Params[%s]=%s\n",
                  ledChangeReason.CSTR(),
                  ImrParser::ToImr_LedIndex(LED_IDX_AIR_DOOR).CSTR(),
                  Util::qVarientToJsonData(ImrParser::ToImr_ActLedParams(params)).CSTR());
        lastLedChangeReason = ledChangeReason;
    }

    env->ds.deviceAction->actLeds(LED_IDX_AIR_DOOR, params);
}
