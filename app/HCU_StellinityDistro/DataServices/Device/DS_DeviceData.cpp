#include "Apps/AppManager.h"
#include "DS_DeviceData.h"
#include "DataServices/Workflow/DS_WorkflowData.h"
#include "DataServices/Capabilities/DS_Capabilities.h"
#include "DataServices/CfgGlobal/DS_CfgGlobal.h"
#include "Common/ImrParser.h"

DS_DeviceData::DS_DeviceData(QObject *parent, EnvGlobal *env_) :
    QObject(parent),
    env(env_)
{
    envLocal = new EnvLocal("DS_Device-Data", "DEVICE_DATA", LOG_LRG_SIZE_BYTES);

    connect(env->appManager, &AppManager::signalAppStarted, this, [=]() {
        slotAppStarted();
    });

    m_DataLocked = false;
    m_MudsLineFluidSyringeIndex = SYRINGE_IDX_NONE;

    DS_DeviceDef::FluidSource item;

    // ---------------------------------------------------
    // Init LastFluidSources
    for (int srcIdx = 0; srcIdx < DS_DeviceDef::FLUID_SOURCE_IDX_MAX; srcIdx++)
    {
        m_LastFluidSources.append(item);
    }

    // ---------------------------------------------------
    // Init FluidSourceBottles
    for (int srcIdx = 0; srcIdx < SYRINGE_IDX_MAX; srcIdx++)
    {
        m_FluidSourceBottles.append(item);
    }

    // Init FluidSourceSyringes
    for (int srcIdx = 0; srcIdx < SYRINGE_IDX_MAX; srcIdx++)
    {
        m_FluidSourceSyringes.append(item);
    }

    m_FluidSourceMuds = item;
    m_FluidSourceSuds = item;
    m_FluidSourceWasteContainer = item;

    m_FluidSourceBottles[SYRINGE_IDX_SALINE].disposableType = DS_DeviceDef::FLUID_SOURCE_TYPE_SY2_BULK_BAG_BOTTLE;
    m_FluidSourceBottles[SYRINGE_IDX_CONTRAST1].disposableType = DS_DeviceDef::FLUID_SOURCE_TYPE_SY2_BULK_BAG_BOTTLE;
    m_FluidSourceBottles[SYRINGE_IDX_CONTRAST2].disposableType = DS_DeviceDef::FLUID_SOURCE_TYPE_SY2_BULK_BAG_BOTTLE;

    m_FluidSourceBottles[SYRINGE_IDX_SALINE].currentFluidKinds[0] = "Flush";
    m_FluidSourceBottles[SYRINGE_IDX_CONTRAST1].currentFluidKinds[0] = "Contrast";
    m_FluidSourceBottles[SYRINGE_IDX_CONTRAST2].currentFluidKinds[0] = "Contrast";

    m_FluidSourceSyringes[SYRINGE_IDX_SALINE].disposableType = DS_DeviceDef::FLUID_SOURCE_TYPE_SY2_MUDS_RESERVOIR;
    m_FluidSourceSyringes[SYRINGE_IDX_CONTRAST1].disposableType = DS_DeviceDef::FLUID_SOURCE_TYPE_SY2_MUDS_RESERVOIR;
    m_FluidSourceSyringes[SYRINGE_IDX_CONTRAST2].disposableType = DS_DeviceDef::FLUID_SOURCE_TYPE_SY2_MUDS_RESERVOIR;

    m_FluidSourceSyringes[SYRINGE_IDX_SALINE].currentFluidKinds[0] = "Flush";
    m_FluidSourceSyringes[SYRINGE_IDX_CONTRAST1].currentFluidKinds[0] = "Contrast";
    m_FluidSourceSyringes[SYRINGE_IDX_CONTRAST2].currentFluidKinds[0] = "Contrast";

    m_FluidSourceMuds.disposableType = DS_DeviceDef::FLUID_SOURCE_TYPE_SY2_MUDS_ASSEMBLY;
    m_FluidSourceSuds.disposableType = DS_DeviceDef::FLUID_SOURCE_TYPE_SY2_SUDS;
    m_FluidSourceWasteContainer.disposableType = DS_DeviceDef::FLUID_SOURCE_TYPE_SY2_WASTE_CONTAINER;

    m_IsFirstAutoPrimeCompleted = false;

    m_BarcodeReaderConnected = false;

    m_IsSudsUsed = false;

    SET_LAST_DATA(DataLocked)
    SET_LAST_DATA(IsFirstAutoPrimeCompleted)
    SET_LAST_DATA(FluidSourceBottles)
    SET_LAST_DATA(FluidSourceSyringes)
    SET_LAST_DATA(FluidSourceMuds)
    SET_LAST_DATA(FluidSourceSuds)
    SET_LAST_DATA(FluidSourceWasteContainer)
    SET_LAST_DATA(FluidOptions)
    SET_LAST_DATA(LastFluidSources)
    SET_LAST_DATA(AllowedContrastPackages)
    SET_LAST_DATA(AllowedSalinePackages)
    SET_LAST_DATA(BarcodeInfo)
    SET_LAST_DATA(BarcodeReaderConnected)
    SET_LAST_DATA(MudsLineFluidSyringeIndex)
}

DS_DeviceData::~DS_DeviceData()
{
    delete envLocal;
}

void DS_DeviceData::slotAppStarted()
{
    EMIT_DATA_CHANGED_SIGNAL(DataLocked)
    EMIT_DATA_CHANGED_SIGNAL(IsFirstAutoPrimeCompleted)
    //EMIT_DATA_CHANGED_SIGNAL(FluidSourceBottles)
    //EMIT_DATA_CHANGED_SIGNAL(FluidSourceSyringes)
    //EMIT_DATA_CHANGED_SIGNAL(FluidSourceMuds)
    //EMIT_DATA_CHANGED_SIGNAL(FluidSourceSuds)
    //EMIT_DATA_CHANGED_SIGNAL(FluidSourceWasteContainer)
    //EMIT_DATA_CHANGED_SIGNAL(FluidOptions)
    EMIT_DATA_CHANGED_SIGNAL(LastFluidSources)
    EMIT_DATA_CHANGED_SIGNAL(AllowedContrastPackages)
    EMIT_DATA_CHANGED_SIGNAL(AllowedSalinePackages)
}

bool DS_DeviceData::isSameContrastsLoaded() const
{
    static bool wasSameContrastsLoaded = false;
    bool sameContrastsLoaded = false;
    static QString prevReason = "";
    QString reason;

    DS_DeviceDef::FluidPackages contrastPackagesLoaded1 = m_FluidSourceBottles[SYRINGE_IDX_CONTRAST1].sourcePackages;
    DS_DeviceDef::FluidPackages contrastPackagesLoaded2 = m_FluidSourceBottles[SYRINGE_IDX_CONTRAST2].sourcePackages;

    if ( (contrastPackagesLoaded1.length() > 0) &&
         (contrastPackagesLoaded2.length() > 0) &&
         (contrastPackagesLoaded1.first().isSameFluidPackage(contrastPackagesLoaded2.first())) )
    {
        reason = "Same Packages Loaded";
        sameContrastsLoaded = true;
    }
    else
    {
        reason = QString().asprintf("PackagesLoaded[BC1]=%s\nPackagesLoaded[BC2]=%s\n",
                                   Util::qVarientToJsonData(ImrParser::ToImr_FluidPackages(contrastPackagesLoaded1)).CSTR(),
                                   Util::qVarientToJsonData(ImrParser::ToImr_FluidPackages(contrastPackagesLoaded2)).CSTR());
        sameContrastsLoaded = false;
    }

    if (reason != prevReason)
    {
        LOG_INFO("isSameContrastsLoaded(): %s Contrasts Loaded: Reason=%s\n", sameContrastsLoaded ? "SAME" : "DIFFERENT", reason.CSTR());
        prevReason = reason;
    }

    if (sameContrastsLoaded != wasSameContrastsLoaded)
    {
        wasSameContrastsLoaded = sameContrastsLoaded;
    }

    return sameContrastsLoaded;
}

bool DS_DeviceData::isRefillReady(SyringeIdx syringeIdx) const
{
    QString syringeIdxStr = ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdx);
    DS_WorkflowDef::MudsSodStatus mudsSodStatus = env->ds.workflowData->getMudsSodStatus();

    if ( (mudsSodStatus.syringeSodStatusAll[syringeIdx].primed) &&
         (!m_FluidSourceBottles[syringeIdx].needsReplaced) &&
         (m_FluidSourceBottles[syringeIdx].isReady) &&
         (m_FluidSourceBottles[syringeIdx].isInstalled()) &&
         (m_FluidSourceBottles[syringeIdx].sourcePackages.length() > 0) )
    {
        LOG_DEBUG("isRefillReady(%s): Refill Ready\n", syringeIdxStr.CSTR());
        return true;
    }

    if (!mudsSodStatus.syringeSodStatusAll[syringeIdx].primed)
    {
        LOG_WARNING("isRefillReady(%s): Refill Not Ready: Syringe not primed yet.\n", syringeIdxStr.CSTR());
    }

    if ( (m_FluidSourceBottles[syringeIdx].needsReplaced) ||
         (!m_FluidSourceBottles[syringeIdx].isReady) ||
         (!m_FluidSourceBottles[syringeIdx].isInstalled()) ||
         (m_FluidSourceBottles[syringeIdx].sourcePackages.length() == 0) )
    {
        LOG_WARNING("isRefillReady(%s): Refill Not Ready.\nFluidSourceBottle=%s\n",
                    syringeIdxStr.CSTR(), Util::qVarientToJsonData(ImrParser::ToImr_FluidSource(m_FluidSourceBottles[syringeIdx]), false).CSTR());
    }

    return false;
}

bool DS_DeviceData::getFluidSourceSyringesBusy(SyringeIdx ignoredSyringe) const
{
    for (int syringeIdx = 0; syringeIdx < m_FluidSourceSyringes.length(); syringeIdx++)
    {
        if (syringeIdx == ignoredSyringe)
        {
            continue;
        }

        if (m_FluidSourceSyringes[syringeIdx].isBusy)
        {
            return true;
        }
    }
    return false;
}

bool DS_DeviceData::getSyringeSodStartReady(SyringeIdx syringeIdx) const
{
    DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
    DS_WorkflowDef::MudsSodStatus mudsSodStatus = env->ds.workflowData->getMudsSodStatus();

    if (fluidSourceSyringes[syringeIdx].sourcePackages.length() == 0)
    {
        // no fluid is loaded
        return false;
    }

    double curVolume = fluidSourceSyringes[syringeIdx].currentVolumesTotal();

    if (!mudsSodStatus.syringeSodStatusAll[syringeIdx].calSlackDone)
    {
        // CalSlack is not done
        if (curVolume < getSyringeCalSlackVolRequired(syringeIdx))
        {
            // Insufficient volume for syringe calslack
            return false;
        }
    }
    else
    {
        // CalSlack is done
        if (curVolume < getSyringePrimeVolRequired(syringeIdx))
        {
            // Insufficient volume for syringe prime
            return false;
        }
    }

    return true;
}

double DS_DeviceData::getSyringeCalSlackVolRequired(SyringeIdx syringeIdx) const
{
    DS_WorkflowDef::MudsSodStatus mudsSodStatus = env->ds.workflowData->getMudsSodStatus();
    double syringeCalSlackVolRequired = env->ds.capabilities->get_FluidControl_CalSyringeSlackRequiredVol();
    double calSlackVolRequired = 0;

    if (!mudsSodStatus.syringeSodStatusAll[syringeIdx].calSlackDone)
    {
        calSlackVolRequired += syringeCalSlackVolRequired;
    }
    return calSlackVolRequired;
}

double DS_DeviceData::getSyringePrimeVolRequired(SyringeIdx syringeIdx) const
{
    DS_WorkflowDef::MudsSodStatus mudsSodStatus = env->ds.workflowData->getMudsSodStatus();
    double syringeMudsPrimeVolRequired = env->ds.capabilities->get_Prime_SyringePrimeVol() * SOD_SYRINGE_PRIME_TRIALS_LIMIT;
    double syringeCalAirCheckVolRequired = env->ds.capabilities->get_AirCheck_SyringeAirCheckRequiredVol();

    double primeVolRequired = 0;

    if (!mudsSodStatus.syringeSodStatusAll[syringeIdx].primed)
    {
        primeVolRequired += syringeMudsPrimeVolRequired;
    }

    if (!mudsSodStatus.syringeSodStatusAll[syringeIdx].airCheckCalibrated)
    {
        primeVolRequired += syringeCalAirCheckVolRequired;
    }

    return primeVolRequired;
}

void DS_DeviceData::setFluidSourceSyringe(SyringeIdx syringeIdx, const DS_DeviceDef::FluidSource &fluidSource)
{
    DS_DeviceDef::FluidSources fluidSourceSyringes = getFluidSourceSyringes();
    fluidSourceSyringes[syringeIdx] = fluidSource;
    setFluidSourceSyringes(fluidSourceSyringes);
}

void DS_DeviceData::setFluidSourceBottle(SyringeIdx syringeIdx, const DS_DeviceDef::FluidSource &fluidSource)
{
    DS_DeviceDef::FluidSources fluidSourceBottles = getFluidSourceBottles();
    fluidSourceBottles[syringeIdx] = fluidSource;
    setFluidSourceBottles(fluidSourceBottles);
}

bool DS_DeviceData::isFluidSourceWasteContainerReady() const
{
    double wcLevel = m_FluidSourceWasteContainer.currentVolumesTotal();
    if ( (m_FluidSourceWasteContainer.isInstalled()) &&
         ( (wcLevel == DS_DeviceDef::WASTE_CONTAINER_LEVEL_LOW) ||
           (wcLevel == DS_DeviceDef::WASTE_CONTAINER_LEVEL_HIGH) ) )
    {
        return true;
    }
    else if (wcLevel == DS_DeviceDef::WASTE_CONTAINER_LEVEL_UNKNOWN_COMM_DOWN)
    {
        // WC comm down. Skip WC check
        LOG_WARNING("isFluidSourceWasteContainerReady(): WC check is ignored.\n");
        return true;
    }
    return false;
}

bool DS_DeviceData::getIsBottleMandatoryFieldsFilled(SyringeIdx syringeIdx) const
{
    if (m_FluidSourceBottles[syringeIdx].sourcePackages.length() == 0)
    {
        return true;
    }

    const DS_DeviceDef::FluidPackage *fluidPackage = &m_FluidSourceBottles[syringeIdx].sourcePackages[0];

    QJsonDocument document = QJsonDocument::fromJson(env->ds.cfgGlobal->get_Configuration_Exam_MandatoryFields().toUtf8());
    QVariantMap mandatoryFields = document.toVariant().toMap();

    QString mandatoryFieldBatch = QString().asprintf("%sLotBatch", (syringeIdx == SYRINGE_IDX_SALINE) ? "Saline" : "Contrast");
    if ( (mandatoryFields.contains(mandatoryFieldBatch)) &&
         (mandatoryFields[mandatoryFieldBatch].toBool()) )
    {
        // LotBatch field is mandatory
        if (fluidPackage->lotBatch == "")
        {
            return false;
        }
    }

    QString mandatoryFieldExpiration = QString().asprintf("%sExpiration", (syringeIdx == SYRINGE_IDX_SALINE) ? "Saline" : "Contrast");

    if ( (mandatoryFields.contains(mandatoryFieldExpiration)) &&
         (mandatoryFields[mandatoryFieldExpiration].toBool()) )
    {
        // Expiration field is mandatory
        if (fluidPackage->expirationDateEpochMs == -1)
        {
            return false;
        }
    }

    return true;
}
