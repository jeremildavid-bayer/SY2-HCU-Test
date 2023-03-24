#include "Apps/AppManager.h"
#include "DeviceHeatMaintainer.h"
#include "DataServices/Workflow/DS_WorkflowData.h"
#include "DataServices/Mcu/DS_McuData.h"
#include "DataServices/Mcu/Internal/McuAlarm.h"
#include "DataServices/CfgGlobal/DS_CfgGlobal.h"
#include "DataServices/Alert/DS_AlertAction.h"
#include "DataServices/Mcu/DS_McuAction.h"
#include "Common/ImrParser.h"

DeviceHeatMaintainer::DeviceHeatMaintainer(QObject *parent, EnvGlobal *env_) :
    ActionBase(parent, env_)
{
    envLocal = new EnvLocal("DS_Device-HeatMaintainer", "DEVICE_HEAT_MAINTAINER");
    isActive = false;

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=] {
        slotAppInitialised();
    });

    connect(&tmrTemperatureMonitor, &QTimer::timeout, this, [=] {
        if (env->state != EnvGlobal::STATE_RUNNING)
        {
            return;
        }

        if (!env->ds.cfgGlobal->get_Settings_Injector_HeatMaintainerEnabled())
        {
            // Heat Maintainer is disabled. Ignore temperature changed
            return;
        }

        // Temperature Related Alerts
        DS_McuDef::TemperatureReadings temperatureReadings = env->ds.mcuData->getHeatMaintainerStatus().temperatureReadings;

        bool temperatureChanged = false;

        for (int heatMaintainerIdx = 0; heatMaintainerIdx < temperatureReadings.length(); heatMaintainerIdx++)
        {
            if (::fabs(temperatureReadings[heatMaintainerIdx] - lastTemperatureReadings[heatMaintainerIdx]) >= 1.0)
            {
                temperatureChanged = true;
                break;
            }
        }

        if (temperatureChanged)
        {
            LOG_INFO("Temperature changed (%s -> %s)\n",
                     Util::qVarientToJsonData(ImrParser::ToImr_TemperatureReadings(lastTemperatureReadings)).CSTR(),
                     Util::qVarientToJsonData(ImrParser::ToImr_TemperatureReadings(temperatureReadings)).CSTR());

            // Report alert
            QString alertData;
            for (int temperatureIdx = 0; temperatureIdx < temperatureReadings.length(); temperatureIdx++)
            {
                if (temperatureIdx > 0)
                {
                    alertData += ";";
                }
                alertData += QString().asprintf("%.1f", temperatureReadings[temperatureIdx]);
            }
            LOG_INFO("Temperature is changed(%s) from last time\n", alertData.CSTR());
            env->ds.alertAction->activate("HeatMaintainerTemperatureChanged", alertData);
        }

        lastTemperatureReadings = temperatureReadings;
    });
}

DeviceHeatMaintainer::~DeviceHeatMaintainer()
{
    delete envLocal;
}

void DeviceHeatMaintainer::slotAppInitialised()
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
            handleHeatMaintainerStatusChanged();
        }
        else if ( (workflowState == DS_WorkflowDef::STATE_INACTIVE) &&
                  (workflowStatePrev != DS_WorkflowDef::STATE_INACTIVE) )
        {
            isActive = false;
        }
    });

    connect(env->ds.cfgGlobal, &DS_CfgGlobal::signalConfigChanged_Settings_Injector_HeatMaintainerEnabled, this, [=](const Config::Item &cfg, const Config::Item &prevCfg) {
        if (env->state != EnvGlobal::STATE_RUNNING)
        {
            return;
        }

        // Config can be changed from the service menu
        bool heatMaintainerEnabled = cfg.value.toBool();
        bool heatMaintainerEnabledPrev = prevCfg.value.toBool();

        DS_McuDef::LinkState mcuLinkState = env->ds.mcuData->getLinkState();
        if (mcuLinkState == DS_McuDef::LINK_STATE_CONNECTED)
        {
            env->ds.mcuAction->actHeatMaintainerOn(heatMaintainerEnabled ? DEFAULT_HEATER_TEMPERATURE : 0);
        }

        if (heatMaintainerEnabled != heatMaintainerEnabledPrev)
        {
            // Cfg is changed, Update alert
            env->ds.alertAction->activate("SRUHeatMaintainerChanged", heatMaintainerEnabled ? "Enabled" : "Disabled");
        }
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_HeatMaintainerStatus, this, [=] {
        if (!isActive)
        {
            return;
        }

        handleHeatMaintainerStatusChanged();
    });
}

void DeviceHeatMaintainer::handleHeatMaintainerStatusChanged()
{
    DS_McuDef::HeatMaintainerStatus heatMaintainerStatus = env->ds.mcuData->getHeatMaintainerStatus();

    if (!tmrTemperatureMonitor.isActive())
    {
        // Temperature monitor is not started yet. Capture current one
        lastTemperatureReadings = heatMaintainerStatus.temperatureReadings;
        tmrTemperatureMonitor.start(HEAT_MAINTAINER_TEMPERATURE_MONITOR_INTERVAL_MS);
        LOG_INFO("Temperature monitor timer is started (%dsec interval)\n", HEAT_MAINTAINER_TEMPERATURE_MONITOR_INTERVAL_MS / 1000);
    }

    if (heatMaintainerStatus.state == DS_McuDef::HEAT_MAINTAINER_STATE_CUTOFF)
    {
        if (!env->ds.alertAction->isActivated("HeatMaintainerCutoff"))
        {
            env->ds.alertAction->activate("HeatMaintainerCutoff");
        }
    }
    else
    {
        if (env->ds.alertAction->isActivated("HeatMaintainerCutoff"))
        {
            env->ds.alertAction->deactivate("HeatMaintainerCutoff");
        }
    }

    // On/Off alert handling
    if (heatMaintainerStatus.state == DS_McuDef::HEAT_MAINTAINER_STATE_ENABLED_ON)
    {
        if (env->ds.alertAction->isActivated("HeatMaintainerIsOff"))
        {
            env->ds.alertAction->deactivate("HeatMaintainerIsOff");
        }
    }
    else
    {
        if (!env->ds.alertAction->isActivated("HeatMaintainerIsOff"))
        {
            env->ds.alertAction->activate("HeatMaintainerIsOff");
        }

    }
}
