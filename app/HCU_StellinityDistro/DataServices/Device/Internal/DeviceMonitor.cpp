#include "Apps/AppManager.h"
#include "DeviceMonitor.h"
#include "DataServices/Device/DS_DeviceData.h"
#include "DataServices/Device/DS_DeviceAction.h"
#include "DataServices/Exam/DS_ExamData.h"
#include "DataServices/CfgLocal/DS_CfgLocal.h"
#include "DataServices/System/DS_SystemData.h"
#include "DataServices/Mcu/DS_McuAction.h"
#include "Common/ImrParser.h"
#include "DataServices/Workflow/DS_WorkflowData.h"

DeviceMonitor::DeviceMonitor(QObject *parent, EnvGlobal *env_) :
    QObject(parent),
    env(env_)
{
    envLocal = new EnvLocal("DS_Device-Monitor", "DEVICE_MONITOR", LOG_MID_SIZE_BYTES);
    isActive = false;

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });
}

DeviceMonitor::~DeviceMonitor()
{
    delete envLocal;
}

void DeviceMonitor::slotAppInitialised()
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
            setAllowedSalinePackages();
            setAllowedContrastPackages();
        }
        else if ( (workflowState == DS_WorkflowDef::STATE_INACTIVE) &&
                  (workflowStatePrev != DS_WorkflowDef::STATE_INACTIVE) )
        {
            isActive = false;
        }
    });

    connect(env->ds.cfgLocal, &DS_CfgLocal::signalConfigChanged_Hidden_FluidOptions, this, [=](const Config::Item &cfg) {
        QString err;
        QVariantMap fluidOptionMap = cfg.value.toMap();
        DS_DeviceDef::FluidOptions fluidOptions = ImrParser::ToCpp_FluidOptions(fluidOptionMap, &err);

        if (err != "")
        {
            LOG_ERROR("Failed to parse FluidOptions. Err=%s\n", err.CSTR());
        }

        env->ds.deviceData->setFluidOptions(fluidOptions);
    });

    connect(env->ds.deviceData, &DS_DeviceData::signalDataChanged_FluidOptions, this, [=]() {
        if (!isActive)
        {
            return;
        }
        setAllowedSalinePackages();
        setAllowedContrastPackages();
    });

    connect(env->ds.deviceData, &DS_DeviceData::signalDataChanged_FluidSourceBottles, this, [=](const DS_DeviceDef::FluidSources &fluidSourceBottles, const DS_DeviceDef::FluidSources &prevFluidSourceBottles) {
        if (!isActive)
        {
            return;
        }

        if ( (fluidSourceBottles[SYRINGE_IDX_CONTRAST1].sourcePackages != prevFluidSourceBottles[SYRINGE_IDX_CONTRAST1].sourcePackages) ||
             (fluidSourceBottles[SYRINGE_IDX_CONTRAST2].sourcePackages != prevFluidSourceBottles[SYRINGE_IDX_CONTRAST2].sourcePackages) )
        {
            setAllowedContrastPackages();
        }
    });

    connect(env->ds.deviceData, &DS_DeviceData::signalDataChanged_FluidSourceSyringes, this, [=](const DS_DeviceDef::FluidSources &fluidSourceSyringes, const DS_DeviceDef::FluidSources &prevFluidSourceSyringes) {
        if (!isActive)
        {
            return;
        }
        if ( (fluidSourceSyringes[SYRINGE_IDX_CONTRAST1].sourcePackages != prevFluidSourceSyringes[SYRINGE_IDX_CONTRAST1].sourcePackages) ||
             (fluidSourceSyringes[SYRINGE_IDX_CONTRAST2].sourcePackages != prevFluidSourceSyringes[SYRINGE_IDX_CONTRAST2].sourcePackages) )
        {
            setAllowedContrastPackages();
        }
    });
}

void DeviceMonitor::setAllowedSalinePackages()
{
    DS_DeviceDef::FluidOptions fluidOptions = env->ds.deviceData->getFluidOptions();
    env->ds.deviceData->setAllowedSalinePackages(fluidOptions.salinePackages);
}

void DeviceMonitor::setAllowedContrastPackages()
{
    DS_DeviceDef::FluidSources fluidSourceBottles = env->ds.deviceData->getFluidSourceBottles();
    DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
    DS_DeviceDef::FluidOptions fluidOptions = env->ds.deviceData->getFluidOptions();
    DS_DeviceDef::FluidPackages allowedContrastPackages;

    bool contrastLoadedBottle1 = !fluidSourceBottles[SYRINGE_IDX_CONTRAST1].sourcePackages.isEmpty();
    bool contrastLoadedBottle2 = !fluidSourceBottles[SYRINGE_IDX_CONTRAST2].sourcePackages.isEmpty();
    bool contrastFilledSyringe1 = !fluidSourceSyringes[SYRINGE_IDX_CONTRAST1].sourcePackages.isEmpty();
    bool contrastFilledSyringe2 = !fluidSourceSyringes[SYRINGE_IDX_CONTRAST2].sourcePackages.isEmpty();

    if ( (!contrastLoadedBottle1) &&
         (!contrastLoadedBottle2) &&
         (!contrastFilledSyringe1) &&
         (!contrastFilledSyringe2) )
    {
        // No contrast is loaded to any bottle or syringe. All contrast packages are allowed.
        allowedContrastPackages = fluidOptions.getAllContrastPackages();
    }
    else
    {
        // Get AllowedContrastPackages for Contrast1/2
        for (int syringeIdx = 0; syringeIdx < SYRINGE_IDX_MAX; syringeIdx++)
        {
            bool contrastLoadedBottle, contrastFilledSyringe;

            QString bottleName = ImrParser::ToImr_FluidSourceBottleIdx((SyringeIdx)syringeIdx);
            QString syringeName = ImrParser::ToImr_FluidSourceSyringeIdx((SyringeIdx)syringeIdx);

            if (syringeIdx == SYRINGE_IDX_CONTRAST1)
            {
                contrastLoadedBottle = contrastLoadedBottle1;
                contrastFilledSyringe = contrastFilledSyringe1;
            }
            else if (syringeIdx == SYRINGE_IDX_CONTRAST2)
            {
                contrastLoadedBottle = contrastLoadedBottle2;
                contrastFilledSyringe = contrastFilledSyringe2;
            }
            else
            {
                continue;
            }

            if ( (contrastLoadedBottle) ||
                 (contrastFilledSyringe) )
            {
                DS_DeviceDef::FluidPackages allowedContrastPackagesBuf;
                const DS_DeviceDef::FluidPackages &bottleFluidPackages = fluidSourceBottles[syringeIdx].sourcePackages;
                const DS_DeviceDef::FluidPackages &syringeFluidPackages = fluidSourceSyringes[syringeIdx].sourcePackages;
                const DS_DeviceDef::FluidPackage *loadedFluidPackage;

                // Get AllowedContrastPackages from bottle
                if (bottleFluidPackages.length() > 0)
                {
                    loadedFluidPackage = &bottleFluidPackages[0];
                    LOG_DEBUG("setAllowedContrastPackages(): %s: LoadedPackage=%s\n", bottleName.CSTR(), Util::qVarientToJsonData(ImrParser::ToImr_FluidPackage(*loadedFluidPackage)).CSTR());
                }
                else if (syringeFluidPackages.length() > 0)
                {
                    loadedFluidPackage = &syringeFluidPackages[0];
                    LOG_DEBUG("setAllowedContrastPackages(): %s: LoadedPackage=%s\n", syringeName.CSTR(), Util::qVarientToJsonData(ImrParser::ToImr_FluidPackage(*loadedFluidPackage)).CSTR());
                }
                else
                {
                    LOG_ERROR("setAllowedContrastPackages(): %s: No sourcePackages found from bottle or syringe\n", bottleName.CSTR());
                    continue;
                }

                allowedContrastPackagesBuf = fluidOptions.getAllowedContrastPackages(*loadedFluidPackage);

                if (allowedContrastPackagesBuf.length() == 0)
                {
                    LOG_WARNING("setAllowedContrastPackages(): Currently loaded source package is NotFound or NotCompatible.\n");
                }

                // Remove duplicated packages
                for (int packageIdx = 0; packageIdx < allowedContrastPackages.length(); packageIdx++)
                {
                    for (int packageIdx2 = 0; packageIdx2 < allowedContrastPackagesBuf.length(); packageIdx2++)
                    {
                        if ( (allowedContrastPackages[packageIdx].brand.toLower() == allowedContrastPackagesBuf[packageIdx2].brand.toLower()) &&
                             (allowedContrastPackages[packageIdx].concentration == allowedContrastPackagesBuf[packageIdx2].concentration) &&
                             (allowedContrastPackages[packageIdx].volume == allowedContrastPackagesBuf[packageIdx2].volume) )
                        {
                            /*LOG_DEBUG("setAllowedContrastPackages(): Removing duplicate package: brand=%s, conc=%.0f, volume=%.1fml\n",
                                      allowedContrastPackages[packageIdx].brand.CSTR(),
                                      allowedContrastPackages[packageIdx].concentration,
                                      allowedContrastPackages[packageIdx].volume);*/
                            allowedContrastPackagesBuf.removeAt(packageIdx2);
                            packageIdx2--;
                        }
                    }
                }

                allowedContrastPackages += allowedContrastPackagesBuf;
            }
        }
    }

    //LOG_DEBUG("fluidOptions = %s\n", Util::qVarientToJsonData(ImrParser::ToImr_FluidOptions(fluidOptions), false).CSTR());
    if (env->ds.deviceData->getAllowedContrastPackages() != allowedContrastPackages)
    {
        LOG_DEBUG("setAllowedContrastPackages(): allowedContrastPackages = %s\n", Util::qVarientToJsonData(ImrParser::ToImr_FluidPackages(allowedContrastPackages), false).CSTR());
    }
    env->ds.deviceData->setAllowedContrastPackages(allowedContrastPackages);
}

