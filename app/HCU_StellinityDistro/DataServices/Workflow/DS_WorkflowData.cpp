#include "Apps/AppManager.h"
#include "DS_WorkflowData.h"
#include "DataServices/CfgLocal/DS_CfgLocal.h"
#include "DataServices/Capabilities/DS_Capabilities.h"
#include "Common/ImrParser.h"

DS_WorkflowData::DS_WorkflowData(QObject *parent, EnvGlobal *env_) :
    QObject(parent),
    env(env_)
{
    envLocal = new EnvLocal("DS_Workflow-Data", "WORKFLOW_DATA");

    connect(env->appManager, &AppManager::signalAppStarted, this, [=]() {
        slotAppStarted();
    });

    m_DataLocked = false;
    m_WorkflowState = DS_WorkflowDef::STATE_INACTIVE;
    m_EndOfDayPurgeState = DS_WorkflowDef::END_OF_DAY_PURGE_STATE_INACTIVE;
    m_AutoEmptyState = DS_WorkflowDef::AUTO_EMPTY_STATE_INACTIVE;
    m_MudsEjectState = DS_WorkflowDef::MUDS_EJECT_STATE_INACTIVE;
    m_FluidRemovalState = DS_WorkflowDef::FLUID_REMOVAL_STATE_INACTIVE;
    m_SyringeAirRecoveryState = DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_INACTIVE;
    m_SudsAirRecoveryState = DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_INACTIVE;
    m_MudsSodState = DS_WorkflowDef::MUDS_SOD_STATE_INACTIVE;
    m_SodErrorState = DS_WorkflowDef::SOD_ERROR_STATE_NONE;
    m_PreloadProtocolState = DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_INACTIVE;

    m_UseExtendedAutoPrimeVolume  = false;

    QString errStr;
    QVariantMap mudsSodStatus = env->ds.cfgLocal->get_Hidden_MudsSodStatus();
    m_MudsSodStatus = ImrParser::ToCpp_MudsSodStatus(mudsSodStatus, &errStr);
    m_MudsSodStatus.identified = false;

    m_ContrastAvailable1 = false;
    m_ContrastAvailable2 = false;

    // get from localcfg
    QList<QVariant> cfgLocal_SyringesUsedInLastExam = env->ds.cfgLocal->get_Hidden_SyringesUsedInLastExam();
    for (auto value : cfgLocal_SyringesUsedInLastExam)
    {
        m_SyringesUsedInLastExam.append(value.toBool());
    }

    if (errStr != "")
    {
        LOG_ERROR("Failed to get MudsSodStatus. Err=%s\n", errStr.CSTR());
    }

    SET_LAST_DATA(DataLocked)
    SET_LAST_DATA(WorkflowState)
    SET_LAST_DATA(WorkflowErrorStatus)
    SET_LAST_DATA(SodErrorState)
    SET_LAST_DATA(FluidRemovalState)
    SET_LAST_DATA(EndOfDayPurgeState)
    SET_LAST_DATA(AutoEmptyState)
    SET_LAST_DATA(MudsEjectState)
    SET_LAST_DATA(MudsSodStatus)
    SET_LAST_DATA(SyringeAirRecoveryState)
    SET_LAST_DATA(SudsAirRecoveryState)
    SET_LAST_DATA(MudsSodState)
    SET_LAST_DATA(PreloadProtocolState)
    SET_LAST_DATA(UseExtendedAutoPrimeVolume)
    SET_LAST_DATA(ContrastAvailable1)
    SET_LAST_DATA(ContrastAvailable2)
    SET_LAST_DATA(SyringesUsedInLastExam)
}

DS_WorkflowData::~DS_WorkflowData()
{
    delete envLocal;
}

void DS_WorkflowData::slotAppStarted()
{
    //EMIT_DATA_CHANGED_SIGNAL(DataLocked)
    EMIT_DATA_CHANGED_SIGNAL(WorkflowState)
    //EMIT_DATA_CHANGED_SIGNAL(SodStatus)
    //EMIT_DATA_CHANGED_SIGNAL(WorkflowErrorStatus)
    //EMIT_DATA_CHANGED_SIGNAL(FluidRemovalState)
    //EMIT_DATA_CHANGED_SIGNAL(SyringeAirRecoveryState)
    //EMIT_DATA_CHANGED_SIGNAL(SudsAirRecoveryState)
    //EMIT_DATA_CHANGED_SIGNAL(MudsSodState)
    EMIT_DATA_CHANGED_SIGNAL(MudsSodStatus)
}

double DS_WorkflowData::getAutoPrimeVolume()
{
    return (getUseExtendedAutoPrimeVolume()
            ?
            env->ds.capabilities->get_Prime_ExtendedAutoPrimeVol()
            :
            env->ds.capabilities->get_Prime_AutoPrimeVol());
}

void DS_WorkflowData::resetSyringesUsedInLastExam()
{
    QList<bool> syringesUsed;
    for (int i = (int)SYRINGE_IDX_START; i < (int)SYRINGE_IDX_MAX; i++)
    {
        syringesUsed.append(false);
    }

    m_SyringesUsedInLastExam = syringesUsed;
    EMIT_DATA_CHANGED_SIGNAL(SyringesUsedInLastExam)
}
