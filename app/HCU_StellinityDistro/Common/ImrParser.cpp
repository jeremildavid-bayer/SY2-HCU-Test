#include "ImrParser.h"
#include "Common/Util.h"
#include "Common/Common.h"
#include "Common/HwCapabilities.h"

DS_DeviceDef::FluidSourceIdx ImrParser::ToCpp_FluidSourceIdx(QString from)
{
    if (from == _L("BS0"))
    {
        return DS_DeviceDef::FLUID_SOURCE_IDX_BOTTLE_1;
    }
    else if (from == _L("BC1"))
    {
        return DS_DeviceDef::FLUID_SOURCE_IDX_BOTTLE_2;
    }
    else if (from == _L("BC2"))
    {
        return DS_DeviceDef::FLUID_SOURCE_IDX_BOTTLE_3;
    }
    else if (from == _L("RS0"))
    {
        return DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_1;
    }
    else if (from == _L("RC1"))
    {
        return DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_2;
    }
    else if (from == _L("RC2"))
    {
        return DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_3;
    }
    else if (from == _L("ML"))
    {
        return DS_DeviceDef::FLUID_SOURCE_IDX_MUDS;
    }
    else if (from == _L("PL"))
    {
        return DS_DeviceDef::FLUID_SOURCE_IDX_SUDS;
    }
    else if (from == _L("WC"))
    {
        return DS_DeviceDef::FLUID_SOURCE_IDX_WASTE_CONTAINER;
    }
    else
    {
        return DS_DeviceDef::FLUID_SOURCE_IDX_MAX;
    }
}

SyringeIdx ImrParser::ToCpp_FluidSourceSyringeIdx(QString from)
{
    if (from == _L("RS0"))
    {
        return SYRINGE_IDX_SALINE;
    }
    else if (from == _L("RC1"))
    {
        return SYRINGE_IDX_CONTRAST1;
    }
    else if (from == _L("RC2"))
    {
        return SYRINGE_IDX_CONTRAST2;
    }
    return SYRINGE_IDX_NONE;
}

DS_SystemDef::StatePath ImrParser::ToCpp_StatePath(QString from)
{
    if (from == _L("OnReachable"))
    {
        return DS_SystemDef::STATE_PATH_ON_REACHABLE;
    }
    else if (from == _L("StartupUnknown"))
    {
        return DS_SystemDef::STATE_PATH_STARTUP_UNKNOWN;
    }
    else if (from == _L("Idle"))
    {
        return DS_SystemDef::STATE_PATH_IDLE;
    }
    else if (from == _L("Ready/Armed"))
    {
        return DS_SystemDef::STATE_PATH_READY_ARMED;
    }
    else if (from == _L("Executing"))
    {
        return DS_SystemDef::STATE_PATH_EXECUTING;
    }
    else if (from == _L("Busy/Waiting"))
    {
        return DS_SystemDef::STATE_PATH_BUSY_WAITING;
    }
    else if (from == _L("Busy/Finishing"))
    {
        return DS_SystemDef::STATE_PATH_BUSY_FINISHING;
    }
    else if (from == _L("Busy/Holding"))
    {
        return DS_SystemDef::STATE_PATH_BUSY_HOLDING;
    }
    else if (from == _L("Busy/Servicing"))
    {
        return DS_SystemDef::STATE_PATH_BUSY_SERVICING;
    }
    else if (from == _L("Error"))
    {
        return DS_SystemDef::STATE_PATH_ERROR;
    }
    else if (from == _L("Servicing"))
    {
        return DS_SystemDef::STATE_PATH_SERVICING;
    }
    else
    {
        return DS_SystemDef::STATE_PATH_OFF_UNREACHABLE;
    }
}

DS_ExamDef::InjectionPlanDigestState ImrParser::ToCpp_InjectionPlanDigestState(QString from)
{
    if (from == _L("Initialising"))
    {
        return DS_ExamDef::INJECTION_PLAN_DIGEST_STATE_INITIALISING;
    }
    else if (from == _L("BadData"))
    {
        return DS_ExamDef::INJECTION_PLAN_DIGEST_STATE_BAD_DATA;
    }
    else if (from == _L("Ready"))
    {
        return DS_ExamDef::INJECTION_PLAN_DIGEST_STATE_READY;
    }
    else
    {
        return DS_ExamDef::INJECTION_PLAN_DIGEST_STATE_UNKNOWN;
    }
}

DS_ExamDef::InjectionPersonalizationInputSource ImrParser::ToCpp_InjectionPersonalizationInputSource(QString from)
{
    if (from == _L("None"))
    {
        return DS_ExamDef::INJECTION_PLAN_CUSTOM_PARAM_SOURCE_NONE;
    }
    else if (from == _L("Entry"))
    {
        return DS_ExamDef::INJECTION_PLAN_CUSTOM_PARAM_SOURCE_ENTRY;
    }
    else if (from == _L("Current"))
    {
        return DS_ExamDef::INJECTION_PLAN_CUSTOM_PARAM_SOURCE_CURRENT;
    }
    else
    {
        return DS_ExamDef::INJECTION_PLAN_CUSTOM_PARAM_SOURCE_DEFAULT;
    }
}

DS_ExamDef::InjectionPersonalizationNoticeType ImrParser::ToCpp_InjectionPersonalizationNoticeType(QString from)
{
    if (from == _L("MaximumIodineLoadLimitApplied"))
    {
        return DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_MAXIMUM_IODINE_LOAD_LIMIT_APPLIED;
    }
    else if (from == _L("MaximumContrastVolumeLimitApplied"))
    {
        return DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_MAXIMUM_CONTRAST_VOLUME_LIMIT_APPLIED;
    }
    else if (from == _L("MaximumSalineVolumeLimitApplied"))
    {
        return DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_MAXIMUM_SALINE_VOLUME_LIMIT_APPLIED;
    }
    else if (from == _L("MaximumFlowRateLimitApplied"))
    {
        return DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_MAXIMUM_FLOW_RATE_LIMIT_APPLIED;
    }
    else if (from == _L("MinimumIodineLoadLimitApplied"))
    {
        return DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_MINIMUM_IODINE_LOAD_LIMIT_APPLIED;
    }
    else if (from == _L("MinimumFlowRateLimitApplied"))
    {
        return DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_MINIMUM_FLOW_RATE_LIMIT_APPLIED;
    }
    else if (from == _L("MinimumContrastVolumeLimitApplied"))
    {
        return DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_MINIMUM_CONTRAST_VOLUME_LIMIT_APPLIED;
    }
	else if (from == _L("MaximumIodineLoadLimitIdrDecreased"))
    {
        return DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_MAXIMUM_IODINE_LOAD_LIMIT_IDR_DECREASED;
    }
    else if (from == _L("MaximumFlowRateLimitIdrDecreased"))
    {
        return DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_MAXIMUM_FLOW_RATE_LIMIT_IDR_DECREASED;
    }
    else if (from == _L("MinimumIodineLoadLimitIdrIncreased"))
    {
        return DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_MINIMUM_IODINE_LOAD_LIMIT_IDR_INCREASED;
    }
    else if (from == _L("MinimumFlowRateLimitIdrIncreased"))
    {
        return DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_MINIMUM_FLOW_RATE_LIMIT_IDR_INCREASED;
    }
    else if (from == _L("MinMaxIodineLoadFlowRateConflict"))
    {
        return DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_MIN_MAX_IODINE_LOAD_FLOW_RATE_CONFLICT;
    }
    else if (from == _L("EGFREquationNotValidWithIDMS"))
    {
        return DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_EGFR_EQUATION_NOT_VALID_WITH_IDMS;
    }
    else if (from == _L("EGFREquationOnlyValidWithIDMS"))
    {
        return DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_EGFR_EQUATION_ONLY_VALID_WITH_IDMS;
    }
    else if (from == _L("EGFRMeasurementOlderThanLimit"))
    {
        return DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_EGFR_MEASUREMENT_OLDER_THAN_LIMIT;
    }
    else if (from == _L("InjectionVolumesDecreasedByKruleset"))
    {
        return DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_INJECTION_VOLUMES_DECREASED_BY_KRULESET;
    }
    else if (from == _L("InjectionVolumesIncreasedByKruleset"))
    {
        return DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_INJECTION_VOLUMES_INCREASED_BY_KRULESET;
    }
    else if (from == _L("InjectionVolumesUnaffectedByKruleset"))
    {
        return DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_INJECTION_VOLUMES_UNAFFECTED_BY_KRULESET;
    }
    else if (from == _L("KvpNotConfiguredInKruleset"))
    {
        return DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_KVP_NOT_CONFIGURED_IN_KRULESET;
    }
    else if (from == _L("FlowrateReducedByCatheterTypeLimit"))
    {
        return DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_FLOW_RATE_REDUCED_BY_CATHETER_TYPE_LIMIT;
    }
    else if (from == _L("FlowrateReducedByCatheterPlacementLimit"))
    {
        return DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_FLOW_RATE_REDUCED_BY_CATHETER_PLACEMENT_LIMIT;
    }
    else if (from == _L("FlowRateExceedsCatheterTypeLimit"))
    {
        return DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_FLOW_RATE_EXCEEDS_CATHETER_TYPE_LIMIT;
    }
    else if (from == _L("FlowRateExceedsCatheterPlacementLimit"))
    {
        return DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_FLOW_RATE_EXCEEDS_CATHETER_PLACEMENT_LIMIT;
    }
    else if (from == _L("PressureLimitReducedByCatheterTypeLimit"))
    {
        return DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_PRESSURE_LIMIT_REDUCED_BY_CATHETER_TYPE_LIMIT;
    }
    else if (from == _L("PressureLimitExceedsCatheterTypeLimit"))
    {
        return DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_PRESSURE_LIMIT_EXCEEDS_CATHETER_TYPE_LIMIT;
    }
    else if (from == _L("ContrastVolumeDecreasedByInjectorContext"))
    {
        return DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_CONTRAST_VOLUME_DECREASED_BY_INJECTOR_CONTEXT;
    }
    else if (from == _L("ContrastVolumeIncreasedByInjectorContext"))
    {
        return DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_CONTRAST_VOLUME_INCREASED_BY_INJECTOR_CONTEXT;
    }
    else if (from == _L("SalineVolumeDecreasedByInjectorContext"))
    {
        return DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_SALINE_VOLUME_DECREASED_BY_INJECTOR_CONTEXT;
    }
    else if (from == _L("SalineVolumeIncreasedByInjectorContext"))
    {
        return DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_SALINE_VOLUME_INCREASED_BY_INJECTOR_CONTEXT;
    }
    else if (from == _L("FlowRateIncreasedByInjectorContextAndDurationMaintained"))
    {
        return DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_FLOW_RATE_INCREASED_BY_INJECTOR_CONTEXT_AND_DURATION_MAINTAINED;
    }
    else if (from == _L("FlowRateIncreasedByInjectorContextAndVolumeMaintained"))
    {
        return DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_FLOW_RATE_INCREASED_BY_INJECTOR_CONTEXT_AND_VOLUME_MAINTAINED;
    }
    else if (from == _L("FlowRateDecreasedByInjectorContextAndDurationMaintained"))
    {
        return DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_FLOW_RATE_DECREASED_BY_INJECTOR_CONTEXT_AND_DURATION_MAINTAINED;
    }
    else if (from == _L("FlowRateDecreasedByInjectorContextAndVolumeMaintained"))
    {
        return DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_FLOW_RATE_DECREASED_BY_INJECTOR_CONTEXT_AND_VOLUME_MAINTAINED;
    }
	else if (from == _L("ContrastVolumeDecreasedByInjectorContextAndDurationMaintained"))
    {
        return DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_CONTRAST_VOLUME_DECREASED_BY_INJECTOR_CONTEXT_AND_DURATION_MAINTAINED;
    }
	else if (from == _L("ContrastVolumeDecreasedByInjectorContext"))
    {
        return DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_CONTRAST_VOLUME_DECREASED_BY_INJECTOR_CONTEXT;
    }
	else if (from == _L("ContrastVolumeIncreasedByInjectorContextAndDurationMaintained"))
    {
        return DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_CONTRAST_VOLUME_INCREASED_BY_INJECTOR_CONTEXT_AND_DURATION_MAINTAINED;
    }
	else if (from == _L("ContrastVolumeIncreasedByInjectorContext"))
    {
        return DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_CONTRAST_VOLUME_INCREASED_BY_INJECTOR_CONTEXT;
    }
    else if (from == _L("SalineVolumeDecreasedByInjectorContextAndDurationMaintained"))
    {
        return DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_SALINE_VOLUME_DECREASED_BY_INJECTOR_CONTEXT_AND_DURATION_MAINTAINED;
    }
	else if (from == _L("SalineVolumeDecreasedByInjectorContext"))
    {
        return DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_SALINE_VOLUME_DECREASED_BY_INJECTOR_CONTEXT;
    }
    else if (from == _L("SalineVolumeIncreasedByInjectorContextAndDurationMaintained"))
    {
        return DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_SALINE_VOLUME_INCREASED_BY_INJECTOR_CONTEXT_AND_DURATION_MAINTAINED;
    }
	else if (from == _L("SalineVolumeIncreasedByInjectorContext"))
    {
        return DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_SALINE_VOLUME_INCREASED_BY_INJECTOR_CONTEXT;
    }
    else
    {
        return DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_UNKNOWN;
    }
}

DS_ExamDef::InjectionPersonalizationNoticeImportance ImrParser::ToCpp_InjectionPersonalizationNoticeImportance(QString from)
{
    if (from == _L("Info"))
    {
        return DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_SEVERITY_INFO;
    }
    else if (from == _L("Warning"))
    {
        return DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_SEVERITY_WARNING;
    }
    else if (from == _L("Error"))
    {
        return DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_SEVERITY_ERROR;
    }
    else
    {
        return DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_SEVERITY_UNKNOWN;
    }
}

DS_ExamDef::InjectionPhaseType ImrParser::ToCpp_InjectionPhaseType(QString from)
{
    if (from == _L("Delay"))
    {
        return DS_ExamDef::INJECTION_PHASE_TYPE_DELAY;
    }
    else if (from == _L("Fluid"))
    {
        return DS_ExamDef::INJECTION_PHASE_TYPE_FLUID;
    }
    else if (from == _L("Unknown"))
    {
        return DS_ExamDef::INJECTION_PHASE_TYPE_UNKNOWN;
    }
    else
    {
        return DS_ExamDef::INJECTION_PHASE_TYPE_INVALID;
    }
}

DS_CruDef::CruLicensedFeature ImrParser::ToCpp_CruLicensedFeature(QString from)
{
    if (from == _L("PatientStudyContext"))
    {
        return DS_CruDef::CRU_LICENSED_FEATURE_PATIENT;
    }
    else if (from == _L("WorklistSelection"))
    {
        return DS_CruDef::CRU_LICENSED_FEATURE_WORKLIST;
    }
    else if (from == _L("Unknown"))
    {
        return DS_CruDef::CRU_LICENSED_FEATURE_UNKNOWN;
    }
    else
    {
        return DS_CruDef::CRU_LICENSED_FEATURE_INVALID;
    }
}

DS_CruDef::CruLinkType ImrParser::ToCpp_CruLinkType(QString from)
{
    if (from == _L("WiredEthernet"))
    {
        return DS_CruDef::CRU_LINK_TYPE_WIRED;
    }
    else if (from == _L("WirelessEthernet"))
    {
        return DS_CruDef::CRU_LINK_TYPE_WIRELESS;
    }
    else if (from == _L("Unknown"))
    {
        return DS_CruDef::CRU_LINK_TYPE_UNKNOWN;
    }
    else
    {
        return DS_CruDef::CRU_LINK_TYPE_INVALID;
    }
}

DS_CruDef::CruLinkState ImrParser::ToCpp_CruLinkState(QString from)
{
    if (from == _L("Inactive"))
    {
        return DS_CruDef::CRU_LINK_STATE_INACTIVE;
    }
    else if (from == _L("Recovering"))
    {
        return DS_CruDef::CRU_LINK_STATE_RECOVERING;
    }
    else if (from == _L("Active"))
    {
        return DS_CruDef::CRU_LINK_STATE_ACTIVE;
    }
    else if (from == _L("Unknown"))
    {
        return DS_CruDef::CRU_LINK_STATE_UNKNOWN;
    }
    else
    {
        return DS_CruDef::CRU_LINK_STATE_INVALID;
    }
}

DS_CruDef::CruLinkQuality ImrParser::ToCpp_CruLinkQuality(QString from)
{
    if (from == _L("Poor"))
    {
        return DS_CruDef::CRU_LINK_QUALITY_POOR;
    }
    else if (from == _L("Fair"))
    {
        return DS_CruDef::CRU_LINK_QUALITY_FAIR;
    }
    else if (from == _L("Good"))
    {
        return DS_CruDef::CRU_LINK_QUALITY_GOOD;
    }
    else if (from == _L("Excellent"))
    {
        return DS_CruDef::CRU_LINK_QUALITY_EXCELLENT;
    }
    else if (from == _L("Unknown"))
    {
        return DS_CruDef::CRU_LINK_QUALITY_UNKNOWN;
    }
    else
    {
        return DS_CruDef::CRU_LINK_QUALITY_INVALID;
    }
}

DS_McuDef::InjectionPhaseType ImrParser::ToCpp_McuInjectionPhaseType(QString type)
{
    if (type == _L("PAUSE"))
    {
        return DS_McuDef::INJECTION_PHASE_TYPE_PAUSE;
    }
    else if (type == _L("CONTRAST1"))
    {
        return DS_McuDef::INJECTION_PHASE_TYPE_CONTRAST1;
    }
    else if (type == _L("CONTRAST2"))
    {
        return DS_McuDef::INJECTION_PHASE_TYPE_CONTRAST2;
    }
    else if (type == _L("SALINE"))
    {
        return DS_McuDef::INJECTION_PHASE_TYPE_SALINE;
    }
    else if (type == _L("DUAL1"))
    {
        return DS_McuDef::INJECTION_PHASE_TYPE_DUAL1;
    }
    else if (type == _L("DUAL2"))
    {
        return DS_McuDef::INJECTION_PHASE_TYPE_DUAL2;
    }
    else
    {
        return DS_McuDef::INJECTION_PHASE_TYPE_NONE;
    }
}

DS_McuDef::InjectorState ImrParser::ToCpp_InjectorState(QString from)
{
    if (from == _L("READY_START"))
    {
        return DS_McuDef::INJECTOR_STATE_READY_START;
    }
    else if (from == _L("DELIVERING"))
    {
        return DS_McuDef::INJECTOR_STATE_DELIVERING;
    }
    else if (from == _L("HOLDING"))
    {
        return DS_McuDef::INJECTOR_STATE_HOLDING;
    }
    else if (from == _L("PHASE_PAUSED"))
    {
        return DS_McuDef::INJECTOR_STATE_PHASE_PAUSED;
    }
    else if (from == _L("COMPLETING"))
    {
        return DS_McuDef::INJECTOR_STATE_COMPLETING;
    }
    else if (from == _L("COMPLETED"))
    {
        return DS_McuDef::INJECTOR_STATE_COMPLETED;
    }

    return DS_McuDef::INJECTOR_STATE_IDLE;
}

DS_McuDef::InjectionCompleteReason ImrParser::ToCpp_InjectionCompleteReason(QString from)
{
    if (from == _L("COMPLETED_NORMAL"))
    {
        return DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_NORMAL;
    }
    else if (from == _L("COMPLETED_ALLSTOP_ABORT"))
    {
        return DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_ALLSTOP_ABORT;
    }
    else if (from == _L("COMPLETED_USER_ABORT"))
    {
        return DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_USER_ABORT;
    }
    else if (from == _L("COMPLETED_HOLD_TIMEOUT"))
    {
        return DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_HOLD_TIMEOUT;
    }
    else if (from.contains(_L("COMPLETED_ALARM_ABORT")))
    {
        return DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_ALARM_ABORT;
    }
    else if (from == _L("COMPLETED_OVER_PRESSURE"))
    {
        return DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_OVER_PRESSURE;
    }
    else if (from == _L("COMPLETED_OVER_CURRENT"))
    {
        return DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_OVER_CURRENT;
    }
    else if (from == _L("COMPLETED_AIR_DETECTED"))
    {
        return DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_AIR_DETECTED;
    }
    else if (from == _L("COMPLETED_SUDS_MISSING"))
    {
        return DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_SUDS_MISSING;
    }
    else if (from == _L("COMPLETED_MOTOR_STALL_S0"))
    {
        return DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_MOTOR_STALL_1;
    }
    else if (from == _L("COMPLETED_MOTOR_STALL_C1"))
    {
        return DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_MOTOR_STALL_2;
    }
    else if (from == _L("COMPLETED_MOTOR_STALL_C2"))
    {
        return DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_MOTOR_STALL_3;
    }
    else if (from == _L("COMPLETED_MOTOR_SLIP_S0"))
    {
        return DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_MOTOR_SLIP_1;
    }
    else if (from == _L("COMPLETED_MOTOR_SLIP_C1"))
    {
        return DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_MOTOR_SLIP_2;
    }
    else if (from == _L("COMPLETED_MOTOR_SLIP_C2"))
    {
        return DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_MOTOR_SLIP_3;
    }
    else if (from == _L("COMPLETED_HCU_COMM_DOWN"))
    {
        return DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_HCU_COMM_DOWN;
    }
    else if (from == _L("COMPLETED_OVER_TEMPERATURE"))
    {
        return DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_OVER_TEMPERATURE;
    }
    else if (from == _L("COMPLETED_BATTERY_CRITICAL"))
    {
        return DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_BATTERY_CRITICAL;
    }
    else if (from == _L("COMPLETED_STOPCOCK_COMM_ERROR"))
    {
        return DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_STOPCOCK_COMM_ERROR;
    }
    else if (from == _L("COMPLETED_MUDS_UNLATCHED"))
    {
        return DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_MUDS_UNLATCHED;
    }
    else if (from == _L("DISARMED_SUDS_MISSING"))
    {
        return DS_McuDef::INJECTION_COMPLETE_REASON_DISARMED_SUDS_MISSING;
    }
    else if (from == _L("DISARMED_HCU_ABORT"))
    {
        return DS_McuDef::INJECTION_COMPLETE_REASON_DISARMED_HCU_ABORT;
    }
    else if (from == _L("DISARMED_STOP_BUTTON_ABORT"))
    {
        return DS_McuDef::INJECTION_COMPLETE_REASON_DISARMED_STOP_BUTTON_ABORT;
    }
    else if (from == _L("DISARMED_ARM_TIMEOUT"))
    {
        return DS_McuDef::INJECTION_COMPLETE_REASON_DISARMED_ARM_TIMEOUT;
    }
    else if (from == _L("DISARMED_MUDS_UNLATCHED"))
    {
        return DS_McuDef::INJECTION_COMPLETE_REASON_DISARMED_MUDS_UNLATCHED;
    }
    return DS_McuDef::INJECTION_COMPLETE_REASON_UNKNOWN;
}

DS_McuDef::StopcockPos ImrParser::ToCpp_StopcockPos(QString from)
{
    if (from == _L("SC_FILL"))
    {
        return DS_McuDef::STOPCOCK_POS_FILL;
    }
    else if (from == _L("SC_INJECT"))
    {
        return DS_McuDef::STOPCOCK_POS_INJECT;
    }
    else if (from == _L("SC_CLOSED"))
    {
        return DS_McuDef::STOPCOCK_POS_CLOSED;
    }
    else if (from == _L("SC_MOVING"))
    {
        return DS_McuDef::STOPCOCK_POS_MOVING;
    }
    else if (from == _L("SC_DISENGAGED"))
    {
        return DS_McuDef::STOPCOCK_POS_DISENGAGED;
    }
    return DS_McuDef::STOPCOCK_POS_UNKNOWN;
}

DS_McuDef::PlungerState ImrParser::ToCpp_PlungerState(QString from)
{
    if (from == _L("ENGAGED"))
    {
        return DS_McuDef::PLUNGER_STATE_ENGAGED;
    }
    else if (from == _L("DISENGAGED"))
    {
        return DS_McuDef::PLUNGER_STATE_DISENGAGED;
    }
    else if (from == _L("LOCKFAILED"))
    {
        return DS_McuDef::PLUNGER_STATE_LOCKFAILED;
    }
    return DS_McuDef::PLUNGER_STATE_UNKNOWN;
}

DS_McuDef::SyringeState ImrParser::ToCpp_SyringeState(QString from)
{
    if (from == _L("PROCESSING"))
    {
        return DS_McuDef::SYRINGE_STATE_PROCESSING;
    }
    else if (from == _L("COMPLETED"))
    {
        return DS_McuDef::SYRINGE_STATE_COMPLETED;
    }
    else if (from == _L("USER_ABORT"))
    {
        return DS_McuDef::SYRINGE_STATE_USER_ABORT;
    }
    else if (from == _L("AIR_DETECTED"))
    {
        return DS_McuDef::SYRINGE_STATE_AIR_DETECTED;
    }
    else if (from == _L("LOST_PLUNGER"))
    {
        return DS_McuDef::SYRINGE_STATE_LOST_PLUNGER;
    }
    else if (from == _L("PLUNGER_ENGAGE_FAULT"))
    {
        return DS_McuDef::SYRINGE_STATE_PLUNGER_ENGAGE_FAULT;
    }
    else if (from == _L("PLUNGER_DISENGAGE_FAULT"))
    {
        return DS_McuDef::SYRINGE_STATE_PLUNGER_DISENGAGE_FAULT;
    }
    else if (from == _L("MOTOR_FAIL"))
    {
        return DS_McuDef::SYRINGE_STATE_MOTOR_FAIL;
    }
    else if (from == _L("OVER_PRESSURE"))
    {
        return DS_McuDef::SYRINGE_STATE_OVER_PRESSURE;
    }
    else if (from == _L("OVER_CURRENT"))
    {
        return DS_McuDef::SYRINGE_STATE_OVER_CURRENT;
    }
    else if (from == _L("SUDS_REMOVED"))
    {
        return DS_McuDef::SYRINGE_STATE_SUDS_REMOVED;
    }
    else if (from == _L("SUDS_INSERTED"))
    {
        return DS_McuDef::SYRINGE_STATE_SUDS_INSERTED;
    }
    else if (from == _L("MUDS_REMOVED"))
    {
        return DS_McuDef::SYRINGE_STATE_MUDS_REMOVED;
    }
    else if (from == _L("INSUFFICIENT_FLUID"))
    {
        return DS_McuDef::SYRINGE_STATE_INSUFFICIENT_FLUID;
    }
    else if (from == _L("TIMEOUT"))
    {
        return DS_McuDef::SYRINGE_STATE_TIMEOUT;
    }
    else if (from == _L("HOME_SENSOR_MISSING"))
    {
        return DS_McuDef::SYRINGE_STATE_HOME_SENSOR_MISSING;
    }
    else if (from == _L("INVALID_STATE"))
    {
        return DS_McuDef::SYRINGE_STATE_INVALID_STATE;
    }
    else if (from == _L("BAD_DATA"))
    {
        return DS_McuDef::SYRINGE_STATE_BAD_DATA;
    }
    else if (from == _L("FRAM_FAULT"))
    {
        return DS_McuDef::SYRINGE_STATE_FRAM_FAULT;
    }
    else if (from == _L("STOP_PENDING"))
    {
        return DS_McuDef::SYRINGE_STATE_STOP_PENDING;
    }
    else if (from == _L("SPIKE_MISSING"))
    {
        return DS_McuDef::SYRINGE_STATE_SPIKE_MISSING;
    }
    else if (from == _L("BAD_STOPCOCK_POSITION"))
    {
        return DS_McuDef::SYRINGE_STATE_BAD_STOPCOCK_POSITION;
    }
    return DS_McuDef::SYRINGE_STATE_UNKNOWN;
}

DS_McuDef::BatteryLevel ImrParser::ToCpp_McuBatteryLevel(QString from)
{
    if (from == _L("BT_NO_BATTERY"))
    {
        return DS_McuDef::BATTERY_LEVEL_NO_BATTERY;
    }
    else if (from == _L("BT_FULL"))
    {
        return DS_McuDef::BATTERY_LEVEL_FULL;
    }
    else if (from == _L("BT_HIGH"))
    {
        return DS_McuDef::BATTERY_LEVEL_HIGH;
    }
    else if (from == _L("BT_MEDIUM"))
    {
        return DS_McuDef::BATTERY_LEVEL_MEDIUM;
    }
    else if (from == _L("BT_LOW"))
    {
        return DS_McuDef::BATTERY_LEVEL_LOW;
    }
    else if (from == _L("BT_FLAT"))
    {
        return DS_McuDef::BATTERY_LEVEL_FLAT;
    }
    else if (from == _L("BT_DEAD"))
    {
        return DS_McuDef::BATTERY_LEVEL_DEAD;
    }
    else if (from == _L("BT_CRITICAL"))
    {
        return DS_McuDef::BATTERY_LEVEL_CRITICAL;
    }

    return DS_McuDef::BATTERY_LEVEL_UNKNOWN;
}

DS_McuDef::DoorState ImrParser::ToCpp_DoorState(QString from)
{
    if (from == _L("DOOR_CLOSED"))
    {
        return DS_McuDef::DOOR_CLOSED;
    }
    else if (from == _L("DOOR_OPEN"))
    {
        return DS_McuDef::DOOR_OPEN;
    }
    return DS_McuDef::DOOR_UNKNOWN;
}

DS_McuDef::WasteBinState ImrParser::ToCpp_WasteBinState(QString from)
{
    if (from == _L("WC_MISSING"))
    {
        return DS_McuDef::WASTE_BIN_STATE_MISSING;
    }
    else if (from == _L("WC_LOW"))
    {
        return DS_McuDef::WASTE_BIN_STATE_LOW_FILLED;
    }
    else if (from == _L("WC_HIGH"))
    {
        return DS_McuDef::WASTE_BIN_STATE_HIGH_FILLED;
    }
    else if (from == _L("WC_FULL"))
    {
        return DS_McuDef::WASTE_BIN_STATE_FULLY_FILLED;
    }
    else if (from == _L("WC_COMM_DOWN"))
    {
        return DS_McuDef::WASTE_BIN_STATE_COMM_DOWN;
    }

    return DS_McuDef::WASTE_BIN_STATE_UNKNOWN;
}

bool ImrParser::ToCpp_AdaptiveFlowActive(QString from)
{
    if (from == _L("AF_ON"))
    {
        return true;
    }
    return false;
}

bool ImrParser::ToCpp_MudsPresent(QString from)
{
    if (from == _L("MUDS_PRESENT"))
    {
        return true;
    }
    return false;
}

bool ImrParser::ToCpp_MudsLatched(QString from)
{
    if (from == _L("LATCHED"))
    {
        return true;
    }
    return false;
}

bool ImrParser::ToCpp_SudsInserted(QString from)
{
    if (from == _L("SUDS_PRESENT"))
    {
        return true;
    }
    return false;
}

bool ImrParser::ToCpp_SudsbubbleDetected(QString from)
{
    if (from == _L("SUDS_AIR"))
    {
        return true;
    }
    return false;
}

bool ImrParser::ToCpp_PrimeBtnPressed(QString from)
{
    if (from == _L("PR_BTN_PRESSED"))
    {
        return true;
    }
    return false;
}

bool ImrParser::ToCpp_StopBtnPressed(QString from)
{
    if (from == _L("S_BTN_PRESSED"))
    {
        return true;
    }
    return false;
}

bool ImrParser::ToCpp_DoorBtnPressed(QString from)
{
    if (from == _L("D_BTN_PRESSED"))
    {
        return true;
    }
    return false;
}

DS_McuDef::BottleBubbleDetectorState ImrParser::ToCpp_BubbleState(QString from)
{
    if (from == _L("INLET_AIR"))
    {
        return DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_AIR;
    }
    else if (from == _L("INLET_FLUID"))
    {
        return DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_FLUID;
    }
    else if (from == _L("INLET_MISSING"))
    {
        return DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_MISSING;
    }

    return DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_UNKNOWN;
}

DS_McuDef::OutletDoorState ImrParser::ToCpp_OutletDoorState(QString from)
{
    if (from == _L("O_DOOR_OPEN"))
    {
        return DS_McuDef::OUTLET_DOOR_STATE_OPEN;
    }
    else if (from == _L("O_DOOR_CLOSED"))
    {
        return DS_McuDef::OUTLET_DOOR_STATE_CLOSED;
    }
    return DS_McuDef::OUTLET_DOOR_STATE_UNKNOWN;
}

DS_McuDef::HeatMaintainerState ImrParser::ToCpp_HeatMaintainerState(QString from)
{
    if (from == _L("ENABLED_ON"))
    {
        return DS_McuDef::HEAT_MAINTAINER_STATE_ENABLED_ON;
    }
    else if (from == _L("ENABLED_OFF"))
    {
        return DS_McuDef::HEAT_MAINTAINER_STATE_ENABLED_OFF;
    }
    else if (from == _L("DISABLED"))
    {
        return DS_McuDef::HEAT_MAINTAINER_STATE_DISABLED;
    }
    else if (from == _L("CUTOFF"))
    {
        return DS_McuDef::HEAT_MAINTAINER_STATE_CUTOFF;
    }

    return DS_McuDef::HEAT_MAINTAINER_STATE_UNKNOWN;
}

bool ImrParser::ToCpp_IsShutdown(QString from)
{
    if (from == _L("SHUTDOWN"))
    {
        return true;
    }
    return false;
}

DS_ExamDef::AdaptiveFlowState ImrParser::ToCpp_AdaptiveFlowState(QString from)
{
    if (from == _L("ActiveCritical"))
    {
        return DS_ExamDef::ADAPTIVE_FLOW_STATE_CRITICAL;
    }
    else if (from == _L("Active"))
    {
        return DS_ExamDef::ADAPTIVE_FLOW_STATE_ACTIVE;
    }
    return DS_ExamDef::ADAPTIVE_FLOW_STATE_OFF;
}

DS_McuDef::AdaptiveFlowState ImrParser::ToCpp_McuAdaptiveFlowState(QString from)
{
    if (from == _L("AF_CRITICAL"))
    {
        return DS_McuDef::ADAPTIVE_FLOW_STATE_CRITICAL;
    }
    else if (from == _L("AF_ACTIVE"))
    {
        return DS_McuDef::ADAPTIVE_FLOW_STATE_ACTIVE;
    }
    return DS_McuDef::ADAPTIVE_FLOW_STATE_OFF;
}

DS_TestDef::TestType ImrParser::ToCpp_TestType(QString from)
{
    if (from == _L("Piston"))
    {
        return DS_TestDef::TEST_TYPE_PISTON;
    }
    else if (from == _L("Stopcock"))
    {
        return DS_TestDef::TEST_TYPE_STOPCOCK;
    }
    else if (from == _L("Network"))
    {
        return DS_TestDef::TEST_TYPE_NETWORK;
    }
    else if (from == _L("ContinuousExams"))
    {
        return DS_TestDef::TEST_TYPE_CONTINUOUS_EXAMS;
    }
    return DS_TestDef::TEST_TYPE_UNKNOWN;
}

DS_McuDef::PressureLimitSensitivityType ImrParser::ToCpp_PressureLimitSensitivityType(QString from)
{
    if (from == _L("Unknown"))
    {
        return DS_McuDef::PRESSURE_LIMIT_SENSITIVITY_TYPE_UNKNOWN;
    }
    else if (from == _L("Low"))
    {
        return DS_McuDef::PRESSURE_LIMIT_SENSITIVITY_TYPE_LOW;
    }
    else if (from == _L("Medium"))
    {
        return DS_McuDef::PRESSURE_LIMIT_SENSITIVITY_TYPE_MEDIUM;
    }
    else if (from == _L("Default"))
    {
        return DS_McuDef::PRESSURE_LIMIT_SENSITIVITY_TYPE_DEFAULT;
    }
    else if (from == _L("High"))
    {
        return DS_McuDef::PRESSURE_LIMIT_SENSITIVITY_TYPE_HIGH;
    }
    return DS_McuDef::PRESSURE_LIMIT_SENSITIVITY_TYPE_INVALID;
}

DS_ExamDef::StepTerminationReason ImrParser::ToCpp_StepTerminationReason(QString from)
{
    if (from == _L("Unknown"))
    {
        return DS_ExamDef::STEP_TERMINATION_REASON_UNKNOWN;
    }
    else if (from == _L("Normal"))
    {
        return DS_ExamDef::STEP_TERMINATION_REASON_NORMAL;
    }
    else if (from == _L("AbortedByRequest"))
    {
        return DS_ExamDef::STEP_TERMINATION_REASON_ABORTED_BY_REQUEST;
    }
    else if (from == _L("AbortedByOverPressure"))
    {
        return DS_ExamDef::STEP_TERMINATION_REASON_ABORTED_BY_OVER_PRESSURE;
    }
    else if (from == _L("AbortedByOtherInternal"))
    {
        return DS_ExamDef::STEP_TERMINATION_REASON_ABORTED_BY_OTHER_INTERNAL;
    }
    else if (from == _L("InjectorCommLoss"))
    {
        return DS_ExamDef::STEP_TERMINATION_REASON_ABORTED_BY_INJECTOR_COMM_LOSS;
    }
    else if (from == _L("InjectorCriticalError"))
    {
        return DS_ExamDef::STEP_TERMINATION_REASON_ABORTED_BY_INJECTOR_CRITICAL_ERROR;
    }
    else if (from == _L("AbortedByStalling"))
    {
        return DS_ExamDef::STEP_TERMINATION_REASON_ABORTED_BY_STALLING;
    }
    else if (from == _L("AbortedByHoldTimeout"))
    {
        return DS_ExamDef::STEP_TERMINATION_REASON_ABORTED_BY_HOLD_TIMEOUT;
    }
    else if (from == _L("AbortedByScanner"))
    {
        return DS_ExamDef::STEP_TERMINATION_REASON_ABORTED_BY_SCANNER;
    }
    else if (from == _L("AbortedByAirDetection"))
    {
        return DS_ExamDef::STEP_TERMINATION_REASON_ABORTED_BY_AIR_DETECTION;
    }
    else if (from == _L("AbortedByDisposableRemoval"))
    {
        return DS_ExamDef::STEP_TERMINATION_REASON_ABORTED_BY_DISPOSABLE_REMOVAL;
    }
    else if (from == _L("AbortedByCriticalBatteryLevel"))
    {
        return DS_ExamDef::STEP_TERMINATION_REASON_ABORTED_BY_CRITICAL_BATTERY_LEVEL;
    }
    else if (from == _L("AbortedByHardwareFault"))
    {
        return DS_ExamDef::STEP_TERMINATION_REASON_ABORTED_BY_HARDWARE_FAULT;
    }
    else
    {
        return DS_ExamDef::STEP_TERMINATION_REASON_INVALID;
    }
}

DS_ExamDef::InjectionPhaseProgressState ImrParser::ToCpp_InjectionPhaseProgressState(QString from)
{
    if (from == _L("Unknown"))
    {
        return DS_ExamDef::INJECTION_PHASE_PROGRESS_STATE_UNKNOWN;
    }
    else if (from == _L("Progress"))
    {
        return DS_ExamDef::INJECTION_PHASE_PROGRESS_STATE_PROGRESS;
    }
    else if (from == _L("Jumped"))
    {
        return DS_ExamDef::INJECTION_PHASE_PROGRESS_STATE_JUMPED;
    }
    else if (from == _L("Aborted"))
    {
        return DS_ExamDef::INJECTION_PHASE_PROGRESS_STATE_ABORTED;
    }
    else if (from == _L("Completed"))
    {
        return DS_ExamDef::INJECTION_PHASE_PROGRESS_STATE_COMPLETED;
    }
    else
    {
        return DS_ExamDef::INJECTION_PHASE_PROGRESS_STATE_INVALID;
    }
}

DS_ExamDef::TimedReminderDisplayHint ImrParser::ToCpp_TimedReminderDisplayHint(QString from)
{
    if (from == _L("Unknown"))
    {
        return DS_ExamDef::TIMED_REMINDER_DISPLAY_HINT_UNKNOWN;
    }
    else if (from == _L("CountUp"))
    {
        return DS_ExamDef::TIMED_REMINDER_DISPLAY_HINT_COUNT_UP;
    }
    else if (from == _L("CountDown"))
    {
        return DS_ExamDef::TIMED_REMINDER_DISPLAY_HINT_COUNT_DOWN;
    }
    else if (from == _L("Visual"))
    {
        return DS_ExamDef::TIMED_REMINDER_DISPLAY_HINT_VISUAL;
    }
    else if (from == _L("Audible"))
    {
        return DS_ExamDef::TIMED_REMINDER_DISPLAY_HINT_AUDIBLE;
    }
    else if (from == _L("ForegroundCount"))
    {
        return DS_ExamDef::TIMED_REMINDER_DISPLAY_HINT_FOREGROUND_COUNT;
    }
    else if (from == _L("BackgroundCount"))
    {
        return DS_ExamDef::TIMED_REMINDER_DISPLAY_HINT_BACKGROUND_COUNT;
    }
    else
    {
        return DS_ExamDef::TIMED_REMINDER_DISPLAY_HINT_INVALID;
    }
}

DS_ExamDef::ScannerInterfaceStatus ImrParser::ToCpp_ScannerInterfaceStatus(QString from)
{
    if (from == _L("DisabledTemporarily"))
    {
        return DS_ExamDef::SCANNER_INTERFACE_STATUS_DISABLED_TEMPORARILY;
    }
    else if (from == _L("DisabledPermanently"))
    {
        return DS_ExamDef::SCANNER_INTERFACE_STATUS_DISABLED_PERMANENTLY;
    }
    else if (from == _L("Enabled"))
    {
        return DS_ExamDef::SCANNER_INTERFACE_STATUS_ENABLED;
    }
    else if (from == _L("Active"))
    {
        return DS_ExamDef::SCANNER_INTERFACE_STATUS_ACTIVE;
    }
    else if (from == _L("Waiting"))
    {
        return DS_ExamDef::SCANNER_INTERFACE_STATUS_WAITING;
    }
    else
    {
        return DS_ExamDef::SCANNER_INTERFACE_STATUS_UNKNOWN;
    }
}

DS_ExamDef::ScannerInterfaceMode ImrParser::ToCpp_ScannerInterfaceMode(QString from)
{
    if (from == _L("Monitor"))
    {
        return DS_ExamDef::SCANNER_INTERFACE_MODE_MONITOR;
    }
    else if (from == _L("Tracking"))
    {
        return DS_ExamDef::SCANNER_INTERFACE_MODE_TRACKING;
    }
    else if (from == _L("Control"))
    {
        return DS_ExamDef::SCANNER_INTERFACE_MODE_CONTROL;
    }
    else if (from == _L("Synchronization"))
    {
        return DS_ExamDef::SCANNER_INTERFACE_MODE_SYNCHRONIZATION;
    }
    else
    {
        return DS_ExamDef::SCANNER_INTERFACE_MODE_UNKNOWN;
    }
}

DS_ExamDef::ExamProgressState ImrParser::ToCpp_ExamProgressState(QString from)
{
    if (from == _L("Idle"))
    {
        return DS_ExamDef::EXAM_PROGRESS_STATE_IDLE;
    }
    else if (from == _L("Prepared"))
    {
        return DS_ExamDef::EXAM_PROGRESS_STATE_PREPARED;
    }
    else if (from == _L("PatientSelection"))
    {
        return DS_ExamDef::EXAM_PROGRESS_STATE_PATIENT_SELECTION;
    }
    else if (from == _L("ProtocolSelection"))
    {
        return DS_ExamDef::EXAM_PROGRESS_STATE_PROTOCOL_SELECTION;
    }
    else if (from == _L("ProtocolModification"))
    {
        return DS_ExamDef::EXAM_PROGRESS_STATE_PROTOCOL_MODIFICATION;
    }
    else if (from == _L("Started"))
    {
        return DS_ExamDef::EXAM_PROGRESS_STATE_STARTED;
    }
    else if (from == _L("InjectionExecution"))
    {
        return DS_ExamDef::EXAM_PROGRESS_STATE_INJECTION_EXECUTION;
    }
    else if (from == _L("SummaryConfirmation"))
    {
        return DS_ExamDef::EXAM_PROGRESS_STATE_SUMMARY_CONFIRMATION;
    }
    else if (from == _L("Completing"))
    {
        return DS_ExamDef::EXAM_PROGRESS_STATE_COMPLETING;
    }
    else if (from == _L("Completed"))
    {
        return DS_ExamDef::EXAM_PROGRESS_STATE_COMPLETED;
    }
    else
    {
        return DS_ExamDef::EXAM_PROGRESS_STATE_INVALID;
    }
}

DS_DeviceDef::FluidSourceDisposableType ImrParser::ToCpp_FluidSourceDisposableType(QString from)
{
    if (from == _L("Unknown"))
    {
        return DS_DeviceDef::FLUID_SOURCE_TYPE_UNKNOWN;
    }
    else if (from == _L("FLSII"))
    {
        return DS_DeviceDef::FLUID_SOURCE_TYPE_FLS_II;
    }
    else if (from == _L("PFA100"))
    {
        return DS_DeviceDef::FLUID_SOURCE_TYPE_PFA_100;
    }
    else if (from == _L("PFA150"))
    {
        return DS_DeviceDef::FLUID_SOURCE_TYPE_PFA_150;
    }
    else if (from == _L("PFABracco"))
    {
        return DS_DeviceDef::FLUID_SOURCE_TYPE_PFA_BRACO;
    }
    else if (from == _L("LEXAN"))
    {
        return DS_DeviceDef::FLUID_SOURCE_TYPE_LEXAN;
    }
    else if (from == _L("MRXP_Lexan_Saline"))
    {
        return DS_DeviceDef::FLUID_SOURCE_TYPE_MRXP_LEXAN_SALINE;
    }
    else if (from == _L("MRXP_Lexan_Contrast"))
    {
        return DS_DeviceDef::FLUID_SOURCE_TYPE_MRXP_LEXAN_CONTRAST;
    }
    else if (from == _L("MRXP_QWIKFIT_115"))
    {
        return DS_DeviceDef::FLUID_SOURCE_TYPE_MRXP_QWIKFIT_115;
    }
    else if (from == _L("MRXP_QWIKFIT_65"))
    {
        return DS_DeviceDef::FLUID_SOURCE_TYPE_MRXP_QWIKFIT_65;
    }
    else if (from == _L("Salient_FLS"))
    {
        return DS_DeviceDef::FLUID_SOURCE_TYPE_SALINET_FLS;
    }
    else if (from == _L("SY2_WasteContainer"))
    {
        return DS_DeviceDef::FLUID_SOURCE_TYPE_SY2_WASTE_CONTAINER;
    }
    else if (from == _L("SY2_SUDS"))
    {
        return DS_DeviceDef::FLUID_SOURCE_TYPE_SY2_SUDS;
    }
    else if (from == _L("SY2_MUDS_Assembly"))
    {
        return DS_DeviceDef::FLUID_SOURCE_TYPE_SY2_MUDS_ASSEMBLY;
    }
    else if (from == _L("SY2_MUDS_Reservoir"))
    {
        return DS_DeviceDef::FLUID_SOURCE_TYPE_SY2_MUDS_RESERVOIR;
    }
    else if (from == _L("SY2_BulkBagBottle"))
    {
        return DS_DeviceDef::FLUID_SOURCE_TYPE_SY2_BULK_BAG_BOTTLE;
    }
    else
    {
        return DS_DeviceDef::FLUID_SOURCE_TYPE_INVALID;
    }
}

DS_DeviceDef::FluidSources ImrParser::ToCpp_FluidSources(const QVariantMap &from, QString *err)
{
    QString errStr = "";
    DS_DeviceDef::FluidSources ret;

    for (int srcIdx = DS_DeviceDef::FLUID_SOURCE_IDX_START; srcIdx < DS_DeviceDef::FLUID_SOURCE_IDX_MAX; srcIdx++)
    {
        QString fluidKey = ToImr_FluidSourceIdx((DS_DeviceDef::FluidSourceIdx)srcIdx);
        if (!from.contains(fluidKey))
        {
            errStr = "Missing Fluid Source Key " + fluidKey;
        }
        DS_DeviceDef::FluidSource fluidSource = ToCpp_FluidSource(from[fluidKey].toMap(), &errStr);

        if (errStr != "")
        {
            errStr = "Fluid Source Parse Error: " + fluidKey + ": " + errStr;
        }

        ret.append(fluidSource);
    }

    if (err != NULL)
    {
        // TODO: For everywhere, set append error string so all errors can be reported
        // E.g.
        //  if (errStr != "")
        //  {
        //    errStr = ", " + errStr;
        //  }
        //  *err += errStr;
        *err = errStr;
    }
    return ret;
}

DS_DeviceDef::FluidSource ImrParser::ToCpp_FluidSource(const QVariantMap &from, QString *err)
{
    QString errStr = "";
    DS_DeviceDef::FluidSource ret;

    if (from.isEmpty())
    {
        ret.setIsInstalled(false);
        return ret;
    }

    if (from.contains(_L("InstalledAt")))
    {
        ret.installedAtEpochMs = Util::utcDateTimeStrToQDateTime(from[_L("InstalledAt")].toString()).toMSecsSinceEpoch();
    }
    else
    {
        ret.setIsInstalled(false);
        errStr = "Missing field (InstalledAt)";
    }

    if (from.contains(_L("IsReady")))
    {
        ret.isReady = from[_L("IsReady")].toBool();
    }
    else
    {
        errStr = "Missing field (IsReady)";
    }

    if (from.contains(_L("IsBusy")))
    {
        ret.isBusy = from[_L("IsBusy")].toBool();
    }
    else
    {
        errStr = "Missing field (IsBusy)";
    }

    if (from.contains(_L("CurrentVolumes")))
    {
        QVariantList currentVolumes = from[_L("CurrentVolumes")].toList();
        ret.currentVolumes.clear();
        for (int i = 0; i < currentVolumes.length(); i++)
        {
            ret.currentVolumes.append(currentVolumes[i].toDouble());
        }
    }
    else
    {
        errStr = "Missing field (CurrentVolumes)";
    }

    if (from.contains(_L("CurrentFluidKinds")))
    {
        QVariantList currentFluidKinds = from[_L("CurrentFluidKinds")].toList();
        ret.currentFluidKinds.clear();
        for (int i = 0; i < currentFluidKinds.length(); i++)
        {
            ret.currentFluidKinds.append(currentFluidKinds[i].toString());
        }
    }
    else
    {
        errStr = "Missing field (CurrentFluidKinds)";
    }

    if (from.contains(_L("SourcePackages")))
    {
        ret.sourcePackages = ToCpp_FluidPackages(from[_L("SourcePackages")].toList(), &errStr);
        if (errStr != _L(""))
        {
            errStr = "Failed to parse SourcePackages: " + errStr;
        }
    }
    else
    {
        errStr = "Missing field (SourcePackages)";
    }

    if (from.contains(_L("DisposableType")))
    {
        ret.disposableType = ToCpp_FluidSourceDisposableType(from[_L("DisposableType")].toString());
    }
    else
    {
        errStr = "Missing field (DisposableType)";
    }

    if (from.contains(_L("NeedsReplaced")))
    {
        ret.needsReplaced = from[_L("NeedsReplaced")].toBool();
    }
    else
    {
        errStr = "Missing field (NeedsReplaced)";
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

DS_DeviceDef::FluidPackages ImrParser::ToCpp_FluidPackages(const QVariantList &from, QString *err)
{
    QString errStr = "";
    DS_DeviceDef::FluidPackages ret;

    for (int packageIdx = 0; packageIdx < from.length(); packageIdx++)
    {
        DS_DeviceDef::FluidPackage fluidPackage = ToCpp_FluidPackage(from[packageIdx].toMap(), &errStr);
        if (errStr != "")
        {
            errStr = QString().asprintf("Failed to parse FluidPackages[%d]: %s", packageIdx, errStr.CSTR());
        }

        ret.append(fluidPackage);
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

DS_DeviceDef::FluidOptions ImrParser::ToCpp_FluidOptions(const QVariantMap &from, QString *err)
{
    QString errStr = "";
    DS_DeviceDef::FluidOptions ret;

    if (from.contains(_L("ContrastFamilies")))
    {
        ret.contrastFamilies = ToCpp_FluidFamilies(from[_L("ContrastFamilies")].toList(), &errStr);
        if (errStr != _L(""))
        {
            errStr = "ContrastFamilies: " + errStr;
        }
    }
    else
    {
        errStr = "Missing field (ContrastFamilies)";
    }

    if (from.contains(_L("SalinePackages")))
    {
        ret.salinePackages = ToCpp_FluidPackages(from[_L("SalinePackages")].toList(), &errStr);
        if (errStr != "")
        {
            errStr = "SalinePackages: " + errStr;
        }
    }
    else
    {
        errStr = "Missing field (SalinePackages)";
    }

    if (from.contains(_L("KnownBarcodes")))
    {
        ret.knownBarcodes = ToCpp_KnownBarcodes(from[_L("KnownBarcodes")].toMap(), &errStr);
        if (errStr != "")
        {
            errStr = "KnownBarcodes: " + errStr;
        }
    }
    else
    {
        errStr = "Missing field (KnownBarcodes)";
    }

    if (from.contains(_L("ChangedAt")))
    {
        ret.changedAtEpochMs = Util::utcDateTimeStrToQDateTime(from[_L("ChangedAt")].toString()).toMSecsSinceEpoch();
    }
    else
    {
        errStr = "Missing field (ChangedAt)";
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

DS_DeviceDef::FluidFamilies ImrParser::ToCpp_FluidFamilies(const QVariantList &from, QString *err)
{
    QString errStr = "";
    DS_DeviceDef::FluidFamilies ret;

    for (int familyIdx = 0; familyIdx < from.length(); familyIdx++)
    {
        DS_DeviceDef::FluidFamily contrastFamily = ToCpp_ContrastFamily(from[familyIdx].toMap(), &errStr);
        if (errStr != "")
        {
            errStr = QString().asprintf("FluidFamily[%d]: ", familyIdx) + errStr;
        }
        ret.append(contrastFamily);
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

DS_DeviceDef::FluidFamily ImrParser::ToCpp_ContrastFamily(const QVariantMap &from, QString *err)
{
    QString errStr = "";
    DS_DeviceDef::FluidFamily ret;

    if (from.contains(_L("Name")))
    {
        ret.name = from[_L("Name")].toString();
    }
    else
    {
        errStr = "Missing field (Name)";
    }

    if (from.contains(_L("FluidPackages")))
    {
        ret.fluidPackages = ToCpp_FluidPackages(from[_L("FluidPackages")].toList(), &errStr);

        if (errStr != _L(""))
        {
            errStr = "FluidPackages: " + errStr;
        }
    }
    else
    {
        errStr = "Missing field (FluidPackages)";
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

DS_DeviceDef::FluidPackage ImrParser::ToCpp_FluidPackage(const QVariantMap &from, QString *err)
{
    QString errStr = "";
    DS_DeviceDef::FluidPackage ret;

    if (from.contains(_L("Brand")))
    {
        ret.brand = from[_L("Brand")].toString();
    }
    else
    {
        errStr = "Missing field (Brand)";
    }

    if (from.contains(_L("Concentration")))
    {
        ret.concentration = from[_L("Concentration")].toDouble();
    }
    else
    {
        errStr = "Missing field (Concentration)";
    }

    if (from.contains(_L("ConcentrationUnits")))
    {
        ret.concentrationUnits = from[_L("ConcentrationUnits")].toString();
    }
    else
    {
        errStr = "Missing field (ConcentrationUnits)";
    }

    ret.volume = 0;
    if (from.contains(_L("Volume")))
    {
        ret.volume = from[_L("Volume")].toDouble();
    }
    else
    {
        errStr = "Missing field (Volume)";
    }

    ret.maximumUseDurationMs = -1;
    if (from.contains(_L("MaximumUseDuration")))
    {
        if (!from[_L("MaximumUseDuration")].isNull())
        {
            ret.maximumUseDurationMs = Util::durationStrToMillisec(from[_L("MaximumUseDuration")].toString());
        }

        if (ret.maximumUseDurationMs == 0)
        {
            // NOTE: Qt parser transforms the null object to 0. Make sure it is set to null.
            ret.maximumUseDurationMs = -1;
        }
    }
    else
    {
        errStr = "Missing field (MaximumUseDuration)";
    }

    ret.loadedAtEpochMs = -1;
    if (from.contains(_L("LoadedAt")))
    {
        if (!from[_L("LoadedAt")].isNull())
        {
            ret.loadedAtEpochMs = Util::utcDateTimeStrToQDateTime(from[_L("LoadedAt")].toString()).toMSecsSinceEpoch();
        }

        if (ret.loadedAtEpochMs == 0)
        {
            // NOTE: Qt parser transforms the null object to 0. Make sure it is set to null.
            ret.loadedAtEpochMs = -1;
        }
    }
    else
    {
        errStr = "Missing field (LoadedAt)";
    }

    ret.lotBatch = "";
    if (from.contains(_L("LotBatch")))
    {
        if (!from[_L("LotBatch")].isNull())
        {
            ret.lotBatch = from[_L("LotBatch")].toString();
        }
    }
    else
    {
        errStr = "Missing field (LotBatch)";
    }

    ret.expirationDateEpochMs = -1;
    if (from.contains(_L("ExpirationDate")))
    {
        if (!from[_L("ExpirationDate")].isNull())
        {
            ret.expirationDateEpochMs = Util::utcDateTimeStrToQDateTime(from[_L("ExpirationDate")].toString()).toMSecsSinceEpoch();
        }

        if (ret.expirationDateEpochMs == 0)
        {
            // NOTE: Qt parser transforms the null object to 0. Make sure it is set to null.
            ret.expirationDateEpochMs = -1;
        }
    }
    else
    {
        errStr = "Missing field (ExpirationDate)";
    }

    if (from.contains(_L("CompatibilityFamily")))
    {
        ret.compatibilityFamily = from[_L("CompatibilityFamily")].toString();
    }
    else
    {
        errStr = "Missing field (CompatibilityFamily)";
    }

    if (from.contains(_L("BarcodePrefixes")))
    {
        ret.barcodePrefixes = from[_L("BarcodePrefixes")].toStringList();
    }
    else
    {
        errStr = "Missing field (BarcodePrefixes)";
    }

    if (from.contains(_L("FluidKind")))
    {
        ret.fluidKind = from[_L("FluidKind")].toString();
    }
    else
    {
        errStr = "Missing field (FluidKind)";
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

DS_DeviceDef::KnownBarcodes ImrParser::ToCpp_KnownBarcodes(const QVariantMap &from, QString *err)
{
    QString errStr = "";
    DS_DeviceDef::KnownBarcodes ret;

    QMap<QString, QVariant>::const_iterator i = from.begin();

    while (i != from.end())
    {
        QVariantMap knownBarcodeMap = i.value().toMap();
        QString key = i.key();

        DS_DeviceDef::KnownBarcode knownBarcode;
        knownBarcode.barcode = key;
        knownBarcode.fluidPackage = ToCpp_FluidPackage(knownBarcodeMap, &errStr);

        if (errStr != "")
        {
            errStr = QString().asprintf("KnownBarcdes[%s]: ", key.CSTR()) + errStr;
        }

        ret.append(knownBarcode);
        i++;
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

Config::ValidRange ImrParser::ToCpp_ConfigItemValidRange(const QVariantMap &from, QString *err)
{
    QString errStr = "";
    Config::ValidRange ret;

    if (from.isEmpty())
    {
        ret.lowerLimitInclusive = 0;
        ret.upperLimitInclusive = 0;
        ret.resolutionIncrement = 0;
    }
    else
    {
        if (from.contains(_L("LowerLimitInclusive")))
        {
            if (from[_L("LowerLimitInclusive")].isNull())
            {
                ret.lowerLimitInclusive = 0;
            }
            else
            {
                ret.lowerLimitInclusive = from[_L("LowerLimitInclusive")];
            }
        }
        else
        {
            errStr = "Missing field (LowerLimitInclusive)";
        }

        if (from.contains(_L("UpperLimitInclusive")))
        {
            if (from[_L("UpperLimitInclusive")].isNull())
            {
                ret.upperLimitInclusive = 0;
            }
            else
            {
                ret.upperLimitInclusive = from[_L("UpperLimitInclusive")];
            }
        }
        else
        {
            errStr = "Missing field (UpperLimitInclusive)";
        }

        if (from.contains(_L("ResolutionIncrement")))
        {
            if (from[_L("ResolutionIncrement")].isNull())
            {
                ret.resolutionIncrement = 0;
            }
            else
            {
                ret.resolutionIncrement = from[_L("ResolutionIncrement")];
            }
        }
        else
        {
            errStr = "Missing field (ResolutionIncrement)";
        }
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

Config::Item ImrParser::ToCpp_ConfigItem(const QVariantMap &from, QString *err)
{
    QString errStr = "";
    Config::Item ret;

    ret.changedAtEpochMs = -1;

    if (from.contains(_L("ChangedAt")))
    {
        if ( (!from[_L("ChangedAt")].isNull()) &&
             (from[_L("ChangedAt")].isValid()) )
        {
            QString changedAtStr = from[_L("ChangedAt")].toString();
            QDateTime changedAtDateTime = Util::utcDateTimeStrToQDateTime(changedAtStr);
            ret.changedAtEpochMs = changedAtDateTime.toMSecsSinceEpoch();
        }
    }

    if (from.contains(_L("DisplayIndex")))
    {
        ret.displayIndex = from[_L("DisplayIndex")].toInt();
    }
    else
    {
        errStr = "Missing field (DisplayIndex)";
    }


    if (from.contains(_L("DefaultValue")))
    {
        ret.defaultValue = from[_L("DefaultValue")];
    }
    else
    {
        errStr = "Missing field (DefaultValue)";
    }

    if (from.contains(_L("KeyName")))
    {
        ret.keyName = from[_L("KeyName")].toString();
    }
    else
    {
        errStr = "Missing field (KeyName)";
    }

    if (from.contains(_L("Value")))
    {
        ret.value = from[_L("Value")];
    }
    else
    {
        errStr = "Missing field (Value)";
    }

    if (from.contains(_L("Units")))
    {
        ret.units = from[_L("Units")].toString();
    }
    else
    {
        errStr = "Missing field (Units)";
    }

    if (from.contains(_L("ValidDataType")))
    {
        ret.validDataType = from[_L("ValidDataType")].toString();
    }
    else
    {
        errStr = "Missing field (ValidDataType)";
    }

    if (from.contains(_L("ValidList")))
    {
        ret.validList = from[_L("ValidList")].toList();
    }
    else
    {
        errStr = "Missing field (ValidList)";
    }

    if (from.contains(_L("ValidRange")))
    {
        ret.validRange = ToCpp_ConfigItemValidRange(from[_L("ValidRange")].toMap());
    }
    else
    {
        errStr = "Missing field (ValidRange)";
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

DS_ExamDef::InjectionPersonalizationInput ImrParser::ToCpp_InjectionPersonalizationInput(const QVariantMap &from, QString *err)
{
    QString errStr = "";
    DS_ExamDef::InjectionPersonalizationInput ret;

    if (from.contains(_L("Name")))
    {
        ret.name = from[_L("Name")].toString();
    }
    else
    {
        errStr = "Missing field (Name)";
    }

    if (from.contains(_L("Value")))
    {
        ret.value = from[_L("Value")];
    }
    else
    {
        errStr = "Missing field (Value)";
    }

    if (from.contains(_L("Units")))
    {
        ret.units = from[_L("Units")].toString();
    }
    else
    {
        errStr = "Missing field (Units)";
    }

    if (from.contains(_L("Source")))
    {
        ret.source = ToCpp_InjectionPersonalizationInputSource(from[_L("Source")].toString());
    }
    else
    {
        errStr = "Missing field (Source)";
    }


    if (from.contains(_L("ValidDataType")))
    {
        ret.validDataType = from[_L("ValidDataType")].toString();
    }
    else
    {
        errStr = "Missing field (ValidDataType)";
    }

    if (from.contains(_L("ValidList")))
    {
        ret.validList = from[_L("ValidList")].toList();
    }
    else
    {
        errStr = "Missing field (ValidList)";
    }

    if (from.contains(_L("ValidRange")))
    {
        ret.validRange = ToCpp_ConfigItemValidRange(from[_L("ValidRange")].toMap());
    }
    else
    {
        errStr = "Missing field (ValidRange)";
    }

    if (from.contains(_L("IsEnabled")))
    {
        ret.isEnabled = from[_L("IsEnabled")].toBool();
    }
    else
    {
        errStr = "Missing field (IsEnabled)";
    }

    if (from.contains(_L("IsValueVisible")))
    {
        ret.isValueVisible = from[_L("IsValueVisible")].toBool();
    }
    else
    {
        errStr = "Missing field (IsValueVisible)";
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

DS_ExamDef::InjectionPersonalizationInputs ImrParser::ToCpp_InjectionPersonalizationInputs(const QVariantList &from, QString *err)
{
    QString errStr = "";
    DS_ExamDef::InjectionPersonalizationInputs ret;

    for (int paramIdx = 0; paramIdx < from.length(); paramIdx++)
    {
        DS_ExamDef::InjectionPersonalizationInput param = ToCpp_InjectionPersonalizationInput(from[paramIdx].toMap(), &errStr);

        if (errStr != "")
        {
            errStr = QString().asprintf("InjectionPersonalizationInputs[%d]: ", paramIdx) + errStr;
        }

        ret.append(param);
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

DS_ExamDef::InjectionPersonalizationNotice ImrParser::ToCpp_InjectionPersonalizationNotice(const QVariantMap &from, QString *err)
{
    QString errStr = "";
    DS_ExamDef::InjectionPersonalizationNotice ret;

    if (from.contains(_L("Name")))
    {
        ret.name = ToCpp_InjectionPersonalizationNoticeType(from[_L("Name")].toString());
    }
    else
    {
        errStr = "Missing field (Name)";
    }

    if (from.contains(_L("Importance")))
    {
        ret.importance = ToCpp_InjectionPersonalizationNoticeImportance(from[_L("Importance")].toString());
    }
    else
    {
        errStr = "Missing field (Importance)";
    }

    if (from.contains(_L("Values")))
    {
        ret.values = from[_L("Values")].toList();
    }
    else
    {
        errStr = "Missing field (Values)";
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

DS_ExamDef::InjectionPersonalizationNotices ImrParser::ToCpp_InjectionPersonalizationNotices(const QVariantList &from, QString *err)
{
    QString errStr = "";
    DS_ExamDef::InjectionPersonalizationNotices ret;

    for (int noticeIdx = 0; noticeIdx < from.length(); noticeIdx++)
    {
        DS_ExamDef::InjectionPersonalizationNotice notice = ToCpp_InjectionPersonalizationNotice(from[noticeIdx].toMap(),&errStr);

        if (errStr != "")
        {
            errStr = QString().asprintf("ToCpp_InjectionPersonalizationNotices[%d]: ", noticeIdx) + errStr;
        }

        ret.append(notice);
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

DS_ExamDef::SharingInformation ImrParser::ToCpp_SharingInformation(const QVariantMap &from, QString *err)
{
    QString errStr = "";
    DS_ExamDef::SharingInformation ret;

    if (from.contains(_L("Logo")))
    {
        ret.logo = from[_L("Logo")].toString();
    }
    else
    {
        errStr = "Missing field (Logo)";
    }

    if (from.contains(_L("Url")))
    {
        ret.url = from[_L("Url")].toString();
    }
    else
    {
        errStr = "Missing field (Url)";
    }

    if (from.contains(_L("Credits")))
    {
        ret.credits = from[_L("Credits")].toString();
    }
    else
    {
        errStr = "Missing field (Credits)";
    }

    if (from.contains(_L("Version")))
    {
        ret.version = from[_L("Version")].toString();
    }
    else
    {
        errStr = "Missing field (Version)";
    }

    if (from.contains(_L("Passcode")))
    {
        ret.passcode = from[_L("Passcode")].toString();
    }
    else
    {
        errStr = "Missing field (Passcode)";
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}


DS_ExamDef::InjectionPhase ImrParser::ToCpp_InjectionPhase(const QVariantMap &from, QString *err, bool setMissingFields)
{
    QString errStr = "";
    DS_ExamDef::InjectionPhase ret;

    ret.type = DS_ExamDef::INJECTION_PHASE_TYPE_FLUID;
    ret.flowRate = 0;
    ret.totalVol = 0;
    ret.contrastPercentage = 0;
    ret.originalPhaseIdx = -1;
    ret.durationMs = DEFAULT_PHASE_DELAY_MS;

    if (from.contains(_L("Type")))
    {
        ret.type = ToCpp_InjectionPhaseType(from[_L("Type")].toString());
    }
    else if (!setMissingFields)
    {
        errStr = "Missing field (Type)";
    }

    if (from.contains(_L("Duration")))
    {
        ret.durationMs = Util::durationStrToMillisec(from[_L("Duration")].toString());
    }
    else if (!setMissingFields)
    {
        errStr = "Missing field (Duration)";
    }

    if (ret.type == DS_ExamDef::INJECTION_PHASE_TYPE_FLUID)
    {
        ret.flowRate = DEFAULT_PHASE_FLOW;
        ret.totalVol = DEFAULT_PHASE_VOL;
        ret.contrastPercentage = 100;

        if (from.contains(_L("FlowRate")))
        {
            ret.flowRate = from[_L("FlowRate")].toDouble();
        }
        else if (!setMissingFields)
        {
            errStr = "Missing field (FlowRate)";
        }

        if (from.contains(_L("TotalVolume")))
        {
            ret.totalVol = from[_L("TotalVolume")].toDouble();
        }
        else if (!setMissingFields)
        {
            errStr = "Missing field (TotalVolume)";
        }

        if (from.contains(_L("ContrastPercentage")))
        {
            ret.contrastPercentage = qMin(from[_L("ContrastPercentage")].toInt(), 100);
        }
        else if (!setMissingFields)
        {
            errStr = "Missing field (ContrastPercentage)";
        }

        if (from.contains(_L("OriginalPhaseIdx")))
        {
            ret.originalPhaseIdx = from[_L("OriginalPhaseIdx")].toInt();
        }
        else if (!setMissingFields)
        {
            // Can be ignored
        }

        ret.setDeliveryDuration();
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

DS_ExamDef::InjectionStep ImrParser::ToCpp_InjectionStep(const QVariantMap &from, QString *err, bool setMissingFields)
{
    QString errStr = "";
    DS_ExamDef::InjectionStep ret;

    ret.guid = Util::newGuid();
    ret.templateGuid = DEFAULT_INJECT_STEP_TEMPLATE_GUID;
    ret.name = "";
    ret.isRepeated = false;
    ret.pressureLimit = PRESSURE_LIMIT_KPA_MAX;
    ret.isTestInjection = false;
    ret.isNotScannerSynchronized = false;
    ret.programmedAtEpochMs = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
    ret.contrastFluidLocation = DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_2;
    ret.salineFluidLocation = DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_1;
    ret.isPreloaded = false;

    if (from.contains(_L("GUID")))
    {
        ret.guid = from[_L("GUID")].toString();
    }
    else if (!setMissingFields)
    {
        errStr = "Missing field (Guid)";
    }

    if (from.contains(_L("Template")))
    {
        ret.templateGuid = from[_L("Template")].toString();
    }
    else if (!setMissingFields)
    {
        errStr = "Missing field (Template)";
    }

    if (from.contains(_L("Name")))
    {
        ret.name = from[_L("Name")].toString();
    }
    else if (!setMissingFields)
    {
        errStr = "Missing field (Name)";
    }

    if (from.contains(_L("IsRepeated")))
    {
        ret.isRepeated = from[_L("IsRepeated")].toBool();
    }
    else if (!setMissingFields)
    {
        errStr = "Missing field (IsRepeated)";
    }

    if (from.contains(_L("PressureLimit")))
    {
        ret.pressureLimit = from[_L("PressureLimit")].toInt();
    }
    else if (!setMissingFields)
    {
        errStr = "Missing field (PressureLimit)";
    }

    if (from.contains(_L("IsTestInjection")))
    {
        ret.isTestInjection = from[_L("IsTestInjection")].toBool();
    }
    else if (!setMissingFields)
    {
        errStr = "Missing field (IsTestInjection)";
    }

    if (from.contains(_L("IsNotScannerSynchronized")))
    {
        ret.isNotScannerSynchronized = from[_L("IsNotScannerSynchronized")].toBool();
    }
    else if (!setMissingFields)
    {
        errStr = "Missing field (IsNotScannerSynchronized)";
    }

    if (from.contains(_L("ProgrammedAt")))
    {
        ret.programmedAtEpochMs = Util::utcDateTimeStrToQDateTime(from[_L("ProgrammedAt")].toString()).toMSecsSinceEpoch();
    }
    else if (!setMissingFields)
    {
        errStr = "Missing field (ProgrammedAt)";
    }

    if (from.contains(_L("ContrastFluidLocationName")))
    {
        ret.contrastFluidLocation = ToCpp_FluidSourceIdx(from[_L("ContrastFluidLocationName")].toString());
    }
    else if (!setMissingFields)
    {
        errStr = "Missing field (ContrastFluidLocationName)";
    }

    if (from.contains(_L("SalineFluidLocationName")))
    {
        ret.salineFluidLocation = ToCpp_FluidSourceIdx(from[_L("SalineFluidLocationName")].toString());
    }
    else if (!setMissingFields)
    {
        errStr = "Missing field (SalineFluidLocationName)";
    }

    if (from.contains(_L("Reminders")))
    {
        QVariantList reminders = from[_L("Reminders")].toList();

        for (int reminderIdx = 0; reminderIdx < reminders.length(); reminderIdx++)
        {
            DS_ExamDef::Reminder reminder = ToCpp_Reminder(reminders[reminderIdx].toMap(), &errStr);
            if (errStr != "")
            {
                errStr = QString().asprintf("Reminders[%d]: ", reminderIdx) + errStr;
            }
            ret.reminders.append(reminder);
        }
    }
    else if (!setMissingFields)
    {
        errStr = "Missing field (Reminders)";
    }

    if (from.contains(_L("Phases")))
    {
        QVariantList phases = from[_L("Phases")].toList();
        ret.phases.clear();

        for (int phaseIdx = 0; phaseIdx < phases.length(); phaseIdx++)
        {
            DS_ExamDef::InjectionPhase phase = ToCpp_InjectionPhase(phases[phaseIdx].toMap(), &errStr, setMissingFields);
            ret.phases.append(phase);

            if (errStr != "")
            {
                errStr = QString().asprintf("Phases[%d]: ", phaseIdx) + errStr;
            }
        }
    }
    else if (!setMissingFields)
    {
        errStr = "Missing field (Phases)";
    }

    if (from.contains(_L("PersonalizationNotices")))
    {
        ret.personalizationNotices = ToCpp_InjectionPersonalizationNotices(from[_L("PersonalizationNotices")].toList());
    }
    else if (!setMissingFields)
    {
        errStr = "Missing field (PersonalizationNotices)";
    }

    if (from.contains(_L("Notices")))
    {
        ret.notices = ToCpp_InjectionPersonalizationNotices(from[_L("Notices")].toList());
    }
    else if (!setMissingFields)
    {
        errStr = "Missing field (Notices)";
    }

    if (from.contains(_L("PersonalizationInputs")))
    {
        ret.personalizationInputs = ToCpp_InjectionPersonalizationInputs(from[_L("PersonalizationInputs")].toList());
    }
    else if (!setMissingFields)
    {
        errStr = "Missing field (PersonalizationInputs)";
    }

    if (from.contains(_L("PersonalizationGenerator")))
    {
        ret.personalizationGenerator = from[_L("PersonalizationGenerator")].toString();
    }
    else if (!setMissingFields)
    {
        errStr = "Missing field (PersonalizationGenerator)";
    }

    if (from.contains(_L("PersonalizationModifiers")))
    {
        ret.personalizationModifiers = from[_L("PersonalizationModifiers")].toStringList();
    }
    else if (!setMissingFields)
    {
        errStr = "Missing field (PersonalizationModifiers)";
    }

    if (from.contains(_L("IsPreloaded")))
    {
        ret.isPreloaded = from[_L("IsPreloaded")].toBool();
    }
    else if (!setMissingFields)
    {
        errStr = "Missing field (IsPreloaded)";
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

DS_ExamDef::InjectionPlan ImrParser::ToCpp_InjectionPlan(const QVariantMap &from, QString *err, bool setMissingFields)
{
    QString errStr = "";
    DS_ExamDef::InjectionPlan ret;

    ret.guid = Util::newGuid();
    ret.name = "";
    ret.description = "";
    ret.templateGuid = DEFAULT_INJECT_PLAN_TEMPLATE_GUID;
    ret.isModifiedFromTemplate = false;
    ret.isPersonalized = false;
    ret.isPreloadable = false;

    if (from.contains(_L("GUID")))
    {
        ret.guid = from[_L("GUID")].toString();
    }
    else if (!setMissingFields)
    {
        errStr = "Missing field (Guid)";
    }

    if (from.contains(_L("Name")))
    {
        ret.name = from[_L("Name")].toString();
    }
    else if (!setMissingFields)
    {
        errStr = "Missing field (Name)";
    }

    if (from.contains(_L("Description")))
    {
        ret.description = from[_L("Description")].toString();
    }
    else if (!setMissingFields)
    {
        errStr = "Missing field (Description)";
    }

    if (from.contains(_L("Template")))
    {
        ret.templateGuid = from[_L("Template")].toString();
    }
    else if (!setMissingFields)
    {
        errStr = "Missing field (Template)";
    }

    if (from.contains(_L("IsModifiedFromTemplate")))
    {
        ret.isModifiedFromTemplate = from[_L("IsModifiedFromTemplate")].toBool();
    }
    else if (!setMissingFields)
    {
        errStr = "Missing field (IsModifiedFromTemplate)";
    }

    if (from.contains(_L("IsPersonalized")))
    {
        ret.isPersonalized = from[_L("IsPersonalized")].toBool();
    }
    else if (!setMissingFields)
    {
        errStr = "Missing field (IsPersonalized)";
    }

    if (from.contains(_L("Steps")))
    {
        QVariantList steps = from[_L("Steps")].toList();

        for (int stepIdx = 0; stepIdx < steps.length(); stepIdx++)
        {
            DS_ExamDef::InjectionStep newStep = ToCpp_InjectionStep(steps[stepIdx].toMap(), &errStr, setMissingFields);
            ret.steps.append(newStep);
            if (errStr != "")
            {
                errStr = QString().asprintf("Steps[%d]: ", stepIdx) + errStr;
            }
        }
    }
    else if (!setMissingFields)
    {
        errStr = "Missing field (Steps)";
    }

    if (from.contains(_L("PersonalizationInputs")))
    {
        ret.personalizationInputs = ToCpp_InjectionPersonalizationInputs(from[_L("PersonalizationInputs")].toList());
    }
    else if (!setMissingFields)
    {
        errStr = "Missing field (PersonalizationInputs)";
    }

    if (from.contains(_L("InjectionSettings")))
    {
        ret.injectionSettings = from[_L("InjectionSettings")].toMap();
    }
    else if (!setMissingFields)
    {
        errStr = "Missing field (InjectionSettings)";
    }

    if (from.contains(_L("SharingInformation")))
    {
        ret.sharingInformation = ToCpp_SharingInformation(from[_L("SharingInformation")].toMap());
    }
    else if (!setMissingFields)
    {
        errStr = "Missing field (SharingInformation)";
    }

    if (from.contains(_L("IsPreloadable")))
    {
        ret.isPreloadable = from[_L("IsPreloadable")].toBool();
    }
    else if (!setMissingFields)
    {
        errStr = "Missing field (IsPreloadable)";
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

DS_ExamDef::InjectionPlanDigest ImrParser::ToCpp_InjectionPlanDigest(const QVariantMap &from, QString *err, bool setMissingFields)
{
    QString errStr = "";
    DS_ExamDef::InjectionPlanDigest ret;

    ret.guid = "";
    ret.histId = "";
    ret.name = "";

    if (from.contains(_L("GUID")))
    {
        ret.guid = from[_L("GUID")].toString();
    }
    else if (!setMissingFields)
    {
        errStr = "Missing field (GUID)";
    }

    if (from.contains(_L("HistId")))
    {
        ret.histId = from[_L("HistId")].toString();
    }
    else if (!setMissingFields)
    {
        errStr = "Missing field (HistId)";
    }

    if (from.contains(_L("Name")))
    {
        ret.name = from[_L("Name")].toString();
    }
    else if (!setMissingFields)
    {
        errStr = "Missing field (Name)";
    }

    if (from.contains(_L("State")))
    {
        ret.state = ToCpp_InjectionPlanDigestState(from[_L("State")].toString());
    }
    else if (!setMissingFields)
    {
        errStr = "Missing field (State)";
    }

    if (from.contains(_L("IsPersonalized")))
    {
        ret.isPersonalized = from["IsPersonalized"].toBool();
    }
    else if (!setMissingFields)
    {
        errStr = "Missing field (IsPersonalized)";
    }



    if (from.contains(_L("Plan")))
    {
        ret.plan = ToCpp_InjectionPlan(from[_L("Plan")].toMap(), &errStr);
        if (errStr != "")
        {
            errStr = "InjectionPlan: " + errStr;
        }
    }
    else if (!setMissingFields)
    {
        errStr = "Missing field (Plan)";
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

DS_ExamDef::InjectionPlanDigests ImrParser::ToCpp_InjectionPlanDigests(const QVariantList &from, QString *err, bool setMissingFields)
{
    QString errStr = "";
    DS_ExamDef::InjectionPlanDigests ret;

    for (int digestIdx = 0; digestIdx < from.length(); digestIdx++)
    {
        DS_ExamDef::InjectionPlanDigest digest = ToCpp_InjectionPlanDigest(from[digestIdx].toMap(), &errStr, setMissingFields);

        if (errStr != "")
        {
            errStr = QString().asprintf("PlanDigests[%d]: ", digestIdx) + errStr;
        }

        ret.append(digest);
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

DS_ExamDef::InjectionPlanTemplateGroup ImrParser::ToCpp_InjectionPlanTemplateGroup(const QVariantMap &from, QString *err, bool setMissingFields)
{
    QString errStr = "";
    DS_ExamDef::InjectionPlanTemplateGroup ret;

    ret.name = "";

    if (from.contains(_L("Name")))
    {
        ret.name = from[_L("Name")].toString();
    }
    else if (!setMissingFields)
    {
        errStr = "Missing field (Name)";
    }

    if (from.contains(_L("PlanDigests")))
    {
        ret.planDigests = ToCpp_InjectionPlanDigests(from[_L("PlanDigests")].toList(), &errStr, setMissingFields);

        if (errStr != _L(""))
        {
            errStr = "PlanDigests: " + errStr;
        }
    }
    else if (!setMissingFields)
    {
        errStr = "Missing field (PlanDigests)";
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

DS_ExamDef::InjectionPlanTemplateGroups ImrParser::ToCpp_InjectionPlanTemplateGroups(const QVariantList &from, QString *err, bool setMissingFields)
{
    QString errStr = "";
    DS_ExamDef::InjectionPlanTemplateGroups ret;

    for (int groupIdx = 0; groupIdx < from.length(); groupIdx++)
    {
        QVariantMap groupMap = from[groupIdx].toMap();
        DS_ExamDef::InjectionPlanTemplateGroup group = ToCpp_InjectionPlanTemplateGroup(groupMap, &errStr, setMissingFields);
        if (errStr != "")
        {
            errStr = QString().asprintf("Group%d ", groupIdx) + errStr;
        }
        ret.append(group);
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

DS_ExamDef::PhaseProgressDigest ImrParser::ToCpp_PhaseProgressDigest(const QVariantMap &from, QString *err)
{
    QString errStr = "";
    DS_ExamDef::PhaseProgressDigest ret;

    if (from.contains(_L("ElapsedMillisFromPhaseStart")))
    {
        ret.elapsedMillisFromPhaseStart = from[_L("ElapsedMillisFromPhaseStart")].toInt();
    }
    else
    {
        ret.elapsedMillisFromPhaseStart = 0;
        errStr = "Missing field (ElapsedMillisFromPhaseStart)";
    }

    if (from.contains(_L("MaxPressure")))
    {
        ret.maxPressure = from[_L("MaxPressure")].toInt();
    }
    else
    {
        ret.maxPressure = 0;
        errStr = "Missing field (MaxPressure)";
    }

    if (from.contains(_L("MaxFlowRate")))
    {
        ret.maxFlowRate = from[_L("MaxFlowRate")].toDouble();
    }
    else
    {
        ret.maxFlowRate = 0;
        errStr = "Missing field (MaxFlowRate)";
    }

    if (from.contains(_L("State")))
    {
        ret.state = ToCpp_InjectionPhaseProgressState(from[_L("State")].toString());
    }
    else
    {
        ret.state = DS_ExamDef::INJECTION_PHASE_PROGRESS_STATE_UNKNOWN;
        errStr = "Missing field (State)";
    }

    if (from.contains(_L("WasFlowAdjusted")))
    {
        ret.wasFlowAdjusted = from[_L("WasFlowAdjusted")].toBool();
    }
    else
    {
        ret.wasFlowAdjusted = false;
        errStr = "Missing field (WasFlowAdjusted)";
    }

    if (from.contains(_L("Progress")))
    {
        ret.progress = from[_L("Progress")].toInt();
    }
    else
    {
        ret.progress = 0;
        errStr = "Missing field (Progress)";
    }

    ret.injectedVolumes.volumes[SYRINGE_IDX_SALINE] = 0;
    ret.injectedVolumes.volumes[SYRINGE_IDX_CONTRAST1] = 0;
    ret.injectedVolumes.volumes[SYRINGE_IDX_CONTRAST2] = 0;

    if (from.contains(_L("InjectedVolumes")))
    {
        QVariantMap injectedVolumes = from[_L("InjectedVolumes")].toMap();
        if (injectedVolumes.contains(_L("RS0")))
        {
            ret.injectedVolumes.volumes[SYRINGE_IDX_SALINE] = injectedVolumes[_L("RS0")].toDouble();
        }
        else
        {
            errStr = "Missing field (InjectedVolumes: RS0)";
        }

        if (injectedVolumes.contains(_L("RC1")))
        {
            ret.injectedVolumes.volumes[SYRINGE_IDX_CONTRAST1] = injectedVolumes[_L("RC1")].toDouble();
        }
        else
        {
            errStr = "Missing field (InjectedVolumes: RC1)";
        }

        if (injectedVolumes.contains(_L("RC2")))
        {
            ret.injectedVolumes.volumes[SYRINGE_IDX_CONTRAST2] = injectedVolumes[_L("RC2")].toDouble();
        }
        else
        {
            errStr = "Missing field (InjectedVolumes: RC2)";
        }
    }
    else
    {
        errStr = "Missing field (InjectedVolumes)";
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

DS_ExamDef::ExecutedStep ImrParser::ToCpp_ExecutedStep(const QVariantMap &from, QString *err)
{
    QString errStr = "";
    DS_ExamDef::ExecutedStep ret;

    if (from.contains(_L("InstantaneousRates")))
    {
        QVariantMap instantaneousRates = from[_L("InstantaneousRates")].toMap();
        if (instantaneousRates.contains(_L("RS0")))
        {
            ret.instantaneousRates.flowRates[SYRINGE_IDX_SALINE] = instantaneousRates[_L("RS0")].toDouble();
        }
        else
        {
            errStr = "Missing field (instantaneousRates: RS0)";
        }

        if (instantaneousRates.contains(_L("RC1")))
        {
            ret.instantaneousRates.flowRates[SYRINGE_IDX_CONTRAST1] = instantaneousRates[_L("RC1")].toDouble();
        }
        else
        {
            errStr = "Missing field (instantaneousRates: RC1)";
        }

        if (instantaneousRates.contains(_L("RC2")))
        {
            ret.instantaneousRates.flowRates[SYRINGE_IDX_CONTRAST2] = instantaneousRates[_L("RC2")].toDouble();
        }
        else
        {
            errStr = "Missing field (instantaneousRates: RC2)";
        }
    }
    else
    {
        errStr = "Missing field (InstantaneousRates)";
    }

    if (from.contains(_L("PhaseProgress")))
    {
        QVariantList phaseProgressDigestList = from[_L("PhaseProgress")].toList();

        for (int phaseIdx = 0; phaseIdx < phaseProgressDigestList.length(); phaseIdx++)
        {
            DS_ExamDef::PhaseProgressDigest phaseProgressDigest = ToCpp_PhaseProgressDigest(phaseProgressDigestList[phaseIdx].toMap(), &errStr);
            if (errStr != "")
            {
                errStr = QString().asprintf("Phase%d ", phaseIdx) + errStr;
            }

            ret.phaseProgress.append(phaseProgressDigest);
        }
    }
    else
    {
        errStr = "Missing field (PhaseProgress)";
    }

    ret.phaseIndex = 0;
    if (from.contains(_L("PhaseIndex")))
    {
        ret.phaseIndex = from[_L("PhaseIndex")].toInt();
    }
    else
    {
        errStr = "Missing field (PhaseIndex)";
    }

    ret.stepIndex = 0;
    if (from.contains(_L("StepIndex")))
    {
        ret.stepIndex = from[_L("StepIndex")].toInt();
    }
    else
    {
        errStr = "Missing field (StepIndex)";
    }

    if (from.contains(_L("ProgrammedStep")))
    {
        ret.programmedStep = from[_L("ProgrammedStep")].toString();
    }
    else
    {
        errStr = "Missing field (ProgrammedStep)";
    }

    if (from.contains(_L("AdaptiveFlowState")))
    {
        ret.adaptiveFlowState = ToCpp_AdaptiveFlowState(from[_L("AdaptiveFlowState")].toString());
    }
    else
    {
        errStr = "Missing field (AdaptiveFlowState)";
    }

    if (from.contains(_L("HasPressureLimited")))
    {
        ret.hasPressureLimited = from[_L("HasPressureLimited")].toBool();
    }
    else
    {
        errStr = "Missing field (HasPressureLimited)";
    }

    if (from.contains(_L("PreventingBackflowSaline")))
    {
        ret.preventingBackflowSaline = from[_L("PreventingBackflowSaline")].toBool();
    }
    else
    {
        errStr = "Missing field (PreventingBackflowSaline)";
    }

    if (from.contains(_L("HasPreventedBackflowSaline")))
    {
        ret.hasPreventedBackflowSaline = from[_L("HasPreventedBackflowSaline")].toBool();
    }
    else
    {
        errStr = "Missing field (HasPreventedBackflowSaline)";
    }

    if (from.contains(_L("ActiveSalineLocation")))
    {
        ret.activeSalineLocation = ToCpp_FluidSourceIdx(from[_L("ActiveSalineLocation")].toString());
    }
    else
    {
        errStr = "Missing field (ActiveSalineLocation)";
    }

    if (from.contains(_L("ActiveContrastLocation")))
    {
        ret.activeContrastLocation = ToCpp_FluidSourceIdx(from[_L("ActiveContrastLocation")].toString());
    }
    else
    {
        errStr = "Missing field (ActiveContrastLocation)";
    }

    if (from.contains(_L("TerminationReason")))
    {
        ret.terminationReason = ToCpp_StepTerminationReason(from[_L("TerminationReason")].toString());
    }
    else
    {
        errStr = "Missing field (TerminationReason)";
    }

    if (from.contains(_L("McuTerminatedReason")))
    {
        ret.mcuTerminatedReason = from[_L("McuTerminatedReason")].toString();
    }
    else
    {
        errStr = "Missing field (McuTerminatedReason)";
    }


    if (from.contains(_L("TerminatedReasonMessage")))
    {
        ret.terminatedReasonMessage = from[_L("TerminatedReasonMessage")].toString();
    }
    else
    {
        errStr = "Missing field (TerminatedReasonMessage)";
    }

    ret.stepTriggeredEpochMs = -1;
    if (from.contains(_L("StepStartedAt")))
    {
        ret.stepTriggeredEpochMs = Util::utcDateTimeStrToQDateTime(from[_L("StepStartedAt")].toString()).toMSecsSinceEpoch();
    }
    else
    {
        errStr = "Missing field (StepStartedAt)";
    }

    ret.stepCompletedEpochMs = -1;
    if (from.contains(_L("StepCompletedAt")))
    {
        ret.stepCompletedEpochMs = Util::utcDateTimeStrToQDateTime(from[_L("StepCompletedAt")].toString()).toMSecsSinceEpoch();
    }
    else
    {
        errStr = "Missing field (StepCompletedAt)";
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

DS_ExamDef::ExecutedSteps ImrParser::ToCpp_ExecutedSteps(const QVariantList &from, QString *err)
{
    QString errStr = "";
    DS_ExamDef::ExecutedSteps ret;

    for (int digestIdx = 0; digestIdx < from.length(); digestIdx++)
    {
        DS_ExamDef::ExecutedStep digest = ToCpp_ExecutedStep(from[digestIdx].toMap(), &errStr);
        if (errStr != "")
        {
            errStr = QString().asprintf("ExecutedSteps[%d]: ", digestIdx) + errStr;
        }

        ret.append(digest);
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

DS_ExamDef::Reminder ImrParser::ToCpp_Reminder(const QVariantMap &from, QString *err)
{
    QString errStr = "";
    DS_ExamDef::Reminder ret;
    QVariantList displayHints;

    ret.handled = false;

    if (from.contains(_L("Name")))
    {
        ret.name = from[_L("Name")].toString();
    }
    else
    {
        errStr = "Missing field (Name)";
    }

    if (from.contains(_L("Description")))
    {
        ret.description = from[_L("Description")].toString();
    }
    else
    {
        errStr = "Missing field (Description)";
    }

    if (from.contains(_L("PostStepTriggerDelay")))
    {
        ret.postStepTriggerDelayMs = Util::durationStrToMillisec(from[_L("PostStepTriggerDelay")].toString());
    }
    else
    {
        errStr = "Missing field (PostStepTriggerDelay)";
    }

    if (from.contains(_L("NotifyExternalSystem")))
    {
        ret.notifyExternalSystem = from[_L("NotifyExternalSystem")].toBool();
    }
    else
    {
        errStr = "Missing field (NotifyExternalSystem)";
    }

    if (from.contains(_L("StartAfterStepCompletes")))
    {
        ret.startAfterStepCompletes = from[_L("StartAfterStepCompletes")].toBool();
    }
    else
    {
        errStr = "Missing field (StartAfterStepCompletes)";
    }

    if (from.contains(_L("DisplayHints")))
    {
        displayHints = from[_L("DisplayHints")].toList();
        for (int i = 0; i < displayHints.length(); i++)
        {
            DS_ExamDef::TimedReminderDisplayHint hint = ToCpp_TimedReminderDisplayHint(displayHints[i].toString());
            ret.displayHints.append(hint);
        }
    }
    else
    {
        errStr = "Missing field (DisplayHints)";
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

DS_ExamDef::ScannerInterlocks ImrParser::ToCpp_ScannerInterlocks(const QVariantMap &from, QString *err)
{
    QString errStr = "";
    DS_ExamDef::ScannerInterlocks ret;

    if (from.contains(_L("ConfigLockedOut")))
    {
        ret.configLockedOut = from[_L("configLockedOut")].toBool();
    }
    else
    {
        errStr = "Missing field (ConfigLockedOut)";
    }

    if (from.contains(_L("ArmLockedOut")))
    {
        ret.armLockedOut = from[_L("ArmLockedOut")].toBool();
    }
    else
    {
        errStr = "Missing field (ArmLockedOut)";
    }

    if (from.contains(_L("StartLockedOut")))
    {
        ret.startLockedOut = from[_L("StartLockedOut")].toBool();
    }
    else
    {
        errStr = "Missing field (StartLockedOut)";
    }

    if (from.contains(_L("ResumeLockedOut")))
    {
        ret.resumeLockedOut = from[_L("ResumeLockedOut")].toBool();
    }
    else
    {
        errStr = "Missing field (ResumeLockedOut)";
    }

    if (from.contains(_L("InterfaceStatus")))
    {
        ret.interfaceStatus = ToCpp_ScannerInterfaceStatus(from[_L("InterfaceStatus")].toString());
    }
    else
    {
        errStr = "Missing field (InterfaceStatus)";
    }

    if (from.contains(_L("InterfaceMode")))
    {
        ret.interfaceMode = ToCpp_ScannerInterfaceMode(from[_L("InterfaceMode")].toString());
    }
    else
    {
        errStr = "Missing field (InterfaceMode)";
    }

    if (from.contains(_L("ScannerReady")))
    {
        ret.scannerReady = from[_L("ScannerReady")].toBool();
    }
    else
    {
        errStr = "Missing field (ScannerReady)";
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

DS_ExamDef::WorklistDetails ImrParser::ToCpp_WorklistDetails(const QVariantMap &from, QString *err)
{
    QString errStr = "";
    DS_ExamDef::WorklistDetails ret;

    if (from.contains("Entry"))
    {
        ret.entry = ToCpp_WorklistEntry(from["Entry"].toMap(), &errStr);
    }
    else
    {
        errStr = "Missing field (Entry)";
    }

    if (from.contains("Panel"))
    {
        ret.panel = ToCpp_DicomFields(from["Panel"].toList(), &errStr);
    }
    else
    {
        errStr = "Missing field (Panel)";
    }

    if (from.contains("All"))
    {
        ret.all = ToCpp_DicomFields(from["All"].toList(), &errStr);
    }
    else
    {
        errStr = "Missing field (All)";
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

DS_ExamDef::ExamField ImrParser::ToCpp_ExamField(const QVariantMap &from, QString *err)
{
    QString errStr = "";
    DS_ExamDef::ExamField ret;

    if (from.contains("Name"))
    {
        ret.name = from["Name"].toString();
    }
    else
    {
        errStr = "Missing field (Name)";
    }

    if (from.contains("Value"))
    {
        ret.value = from["Value"].toString();
    }
    else
    {
        errStr = "Missing field (Value)";
    }

    if (from.contains("Units"))
    {
        ret.units = from["Units"].toString();
    }
    else
    {
        errStr = "Missing field (Units)";
    }

    if (from.contains("ValidDataType"))
    {
        ret.validDataType = from["ValidDataType"].toString();
    }
    else
    {
        errStr = "Missing field (ValidDataType)";
    }

    if (from.contains("ValidRange"))
    {
        ret.validRange = ToCpp_ConfigItemValidRange(from["ValidRange"].toMap(), &errStr);
    }
    else
    {
        errStr = "Missing field (ValidRange)";
    }

    if (from.contains("ValidList"))
    {
        ret.validList = from["ValidList"].toList();
    }
    else
    {
        errStr = "Missing field (ValidList)";
    }

    if (from.contains("NeedsTranslated"))
    {
        ret.needsTranslated = from["NeedsTranslated"].toBool();
    }
    else
    {
        errStr = "Missing field (NeedsTranslated)";
    }

    if (from.contains("IsEnabled"))
    {
        ret.isEnabled = from["IsEnabled"].toBool();
    }
    else
    {
        errStr = "Missing field (IsEnabled)";
    }

    if (from.contains("IsMandatory"))
    {
        ret.isMandatory = from["IsMandatory"].toBool();
    }
    else
    {
        errStr = "Missing field (IsMandatory)";
    }

    if (from.contains("IsMandatoryEntered"))
    {
        ret.isMandatoryEntered = from["IsMandatoryEntered"].toBool();
    }
    else
    {
        errStr = "Missing field (IsMandatoryEntered)";
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

DS_ExamDef::ExamFieldMap ImrParser::ToCpp_ExamFieldMap(const QVariantMap &from, QString *err)
{
    QString errStr = "";
    DS_ExamDef::ExamFieldMap ret;

    QMap<QString, QVariant>::const_iterator i = from.begin();

    i = from.begin();
    while (i != from.end())
    {
        QString key = i.key();
        QVariantMap examFieldImr = i.value().toMap();
        DS_ExamDef::ExamField examField = ToCpp_ExamField(examFieldImr, &errStr);
        if (errStr != "")
        {
            errStr = QString().asprintf("ExamFieldMap[%s]: ", key.CSTR()) + errStr;
        }
        ret.insert(key, examField);
        i++;
    }

    if (err != NULL)
    {
        *err = errStr;
    }

    return ret;
}

// Currently ExamFieldParameter and InjectionPersonalizationInput are identical. Creating layer so that user is independent of the implementation
DS_ExamDef::ExamFieldParameter ImrParser::ToCpp_ExamFieldParameter(const QVariantMap &from, QString *err)
{
    return ToCpp_InjectionPersonalizationInput(from, err);
}

DS_ExamDef::LinkedAccession ImrParser::ToCpp_LinkedAccession(const QVariantMap &from, QString *err)
{
    QString errStr = "";
    DS_ExamDef::LinkedAccession ret;

    if (from.contains("IsLinked"))
    {
        ret.isLinked = from["IsLinked"].toBool();
    }
    else
    {
        errStr = "Missing field (IsLinked)";
    }

    if (from.contains("Entry"))
    {
        ret.entry = ToCpp_WorklistEntry(from["Entry"].toMap());
    }
    else
    {
        errStr = "Missing field (Entry)";
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

DS_ExamDef::LinkedAccessions ImrParser::ToCpp_LinkedAccessions(const QVariantList &from, QString *err)
{
    QString errStr = "";
    DS_ExamDef::LinkedAccessions ret;

    for (int accessionIdx = 0; accessionIdx < from.length(); accessionIdx++)
    {
        DS_ExamDef::LinkedAccession accession = ToCpp_LinkedAccession(from[accessionIdx].toMap(), &errStr);

        if (errStr != "")
        {
            errStr = QString().asprintf("LinkedAccessions[%d]: ", accessionIdx) + errStr;
        }

        ret.append(accession);
    }

    if (err != NULL)
    {
        *err = errStr;
    }

    return ret;
}

DS_ExamDef::ExamAdvanceInfo ImrParser::ToCpp_ExamAdvanceInfo(const QVariantMap &from, QString *err)
{
    QString errStr = "";
    DS_ExamDef::ExamAdvanceInfo ret;

    if (from.contains("GUID"))
    {
        ret.guid = from["GUID"].toString();
    }
    else
    {
        errStr = "Missing field (GUID)";
    }

    if (from.contains("WorklistDetails"))
    {
        ret.worklistDetails = ToCpp_WorklistDetails(from["WorklistDetails"].toMap(), &errStr);
    }
    else
    {
        errStr = "Missing field (WorklistDetails)";
    }

    if (from.contains("ExamInputs"))
    {
        ret.examInputs = ToCpp_ExamFieldMap(from["ExamInputs"].toMap(), &errStr);
    }
    else
    {
        errStr = "Missing field (ExamInputs)";
    }

    if (from.contains("ExamResults"))
    {
        ret.examResults = ToCpp_ExamFieldMap(from["ExamResults"].toMap(), &errStr);
    }
    else
    {
        errStr = "Missing field (ExamResults)";
    }

    if (from.contains("LinkedAccessions"))
    {
        ret.linkedAccessions = ToCpp_LinkedAccessions(from["LinkedAccessions"].toList(), &errStr);
    }
    else
    {
        errStr = "Missing field (LinkedAccessions)";
    }

    if (from.contains("MandatoryFieldsEntered"))
    {
        ret.mandatoryFieldsEntered = from["MandatoryFieldsEntered"].toBool();
    }
    else
    {
        errStr = "Missing field (MandatoryFieldsEntered)";
    }

    if (from.contains("ExamInputsProgress"))
    {
        ret.examInputsProgress = from["ExamInputsProgress"].toInt();
    }
    else
    {
        errStr = "Missing field (ExamInputsProgress)";
    }

    if (from.contains("ExamResultsProgress"))
    {
        ret.examResultsProgress = from["ExamResultsProgress"].toInt();
    }
    else
    {
        errStr = "Missing field (ExamResultsProgress)";
    }


    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

DS_CruDef::CruLicensedFeatures ImrParser::ToCpp_CruLicensedFeatures(const QVariantList &from, QString *err)
{
    QString errStr = "";
    DS_CruDef::CruLicensedFeatures ret;

    for (int featureIdx = 0; featureIdx < from.length(); featureIdx++)
    {
        DS_CruDef::CruLicensedFeature feature = ImrParser::ToCpp_CruLicensedFeature(from[featureIdx].toString());

        if (errStr != "")
        {
            errStr = QString().asprintf("LicensedFeatures[%d]: ", featureIdx) + errStr;
        }

        ret.append(feature);
    }

    if (err != NULL)
    {
        *err = errStr;
    }

    return ret;
}

DS_CruDef::CruLinkStatus ImrParser::ToCpp_CruLinkStatus(const QVariantMap &from, QString *err)
{
    QString errStr = "";
    DS_CruDef::CruLinkStatus ret;

    if (from.contains(_L("Type")))
    {
        ret.type = ToCpp_CruLinkType(from[_L("Type")].toString());
    }
    else
    {
        errStr = "Missing field (Type)";
    }

    if (from.contains(_L("State")))
    {
        ret.state = ToCpp_CruLinkState(from[_L("State")].toString());
    }
    else
    {
        errStr = "Missing field (State)";
    }

    if (from.contains(_L("Quality")))
    {
        ret.quality = ToCpp_CruLinkQuality(from[_L("Quality")].toString());
    }
    else
    {
        errStr = "Missing field (Quality)";
    }

    if (from.contains(_L("WirelessSignalLevel")))
    {
        ret.signalLevel = from[_L("WirelessSignalLevel")].toString();
    }
    else
    {
        errStr = "Missing field (WirelessSignalLevel)";
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

DS_McuDef::InjectionCompleteStatus ImrParser::ToCpp_InjectionCompleteStatus(QString from, QString *err)
{
    QString errStr = "";
    DS_McuDef::InjectionCompleteStatus ret;

    QStringList strList = from.split(";");
    if (strList.length() > 0)
    {
        ret.reason = ToCpp_InjectionCompleteReason(strList[0]);
        if (strList.length() > 1)
        {
            ret.alarmId = (McuAlarm::AlarmId)strList[1].toInt();

            if (strList.length() > 2)
            {
                ret.message = strList[2];
            }
        }
    }


    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

DS_McuDef::ActLedParams ImrParser::ToCpp_ActLedParams(QString from, QString *err)
{
    QString errStr = "";
    DS_McuDef::ActLedParams ret;

    // Remove # character for color code
    from.replace("#", "");

    if (from == _L("NOCHANGE"))
    {
        ret.type = DS_McuDef::LED_CONTROL_TYPE_NO_CHANGE;
    }
    else if  (from == _L("OFF"))
    {
        ret.setColorOff();
    }
    else if (from.length() == 6)
    {
        ret.type = DS_McuDef::LED_CONTROL_TYPE_SET;
        ret.colorR = from.mid(0, 2).toInt(NULL, 16);
        ret.colorG = from.mid(2, 2).toInt(NULL, 16);
        ret.colorB = from.mid(4, 2).toInt(NULL, 16);
    }
    else
    {
        ret.type = DS_McuDef::LED_CONTROL_TYPE_UNKNOWN;
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

DS_McuDef::LedControlStatus ImrParser::ToCpp_LedControlStatus(const QVariantMap &from, QString *err)
{
    QString errStr = "";
    DS_McuDef::LedControlStatus ret;

    if (from.contains(_L("Saline")))
    {
        ret.paramsList[LED_IDX_SALINE] = ToCpp_ActLedParams(from[_L("Saline")].toString());
    }
    else
    {
        errStr = "Missing field (Saline)";
    }

    if (from.contains(_L("Contrast1")))
    {
        ret.paramsList[LED_IDX_CONTRAST1] = ToCpp_ActLedParams(from[_L("Contrast1")].toString());
    }
    else
    {
        errStr = "Missing field (Contrast1)";
    }

    if (from.contains(_L("Contrast2")))
    {
        ret.paramsList[LED_IDX_CONTRAST2] = ToCpp_ActLedParams(from[_L("Contrast2")].toString());
    }
    else
    {
        errStr = "Missing field (Contrast2)";
    }

    if (from.contains(_L("Suds1")))
    {
        ret.paramsList[LED_IDX_SUDS1] = ToCpp_ActLedParams(from[_L("Suds1")].toString());
    }
    else
    {
        errStr = "Missing field (Suds1)";
    }

    if (from.contains(_L("Suds2")))
    {
        ret.paramsList[LED_IDX_SUDS2] = ToCpp_ActLedParams(from[_L("Suds2")].toString());
    }
    else
    {
        errStr = "Missing field (Suds2)";
    }

    if (from.contains(_L("Suds3")))
    {
        ret.paramsList[LED_IDX_SUDS3] = ToCpp_ActLedParams(from[_L("Suds3")].toString());
    }
    else
    {
        errStr = "Missing field (Suds3)";
    }

    if (from.contains(_L("Suds4")))
    {
        ret.paramsList[LED_IDX_SUDS4] = ToCpp_ActLedParams(from[_L("Suds4")].toString());
    }
    else
    {
        errStr = "Missing field (Suds4)";
    }

    if (from.contains(_L("Door1")))
    {
        ret.paramsList[LED_IDX_DOOR1] = ToCpp_ActLedParams(from[_L("Door1")].toString());
    }
    else
    {
        errStr = "Missing field (Door1)";
    }

    if (from.contains(_L("Door2")))
    {
        ret.paramsList[LED_IDX_DOOR2] = ToCpp_ActLedParams(from[_L("Door2")].toString());
    }
    else
    {
        errStr = "Missing field (Door2)";
    }

    if (from.contains(_L("Door3")))
    {
        ret.paramsList[LED_IDX_DOOR3] = ToCpp_ActLedParams(from[_L("Door3")].toString());
    }
    else
    {
        errStr = "Missing field (Door3)";
    }

    if (from.contains(_L("Door4")))
    {
        ret.paramsList[LED_IDX_DOOR4] = ToCpp_ActLedParams(from[_L("Door4")].toString());
    }
    else
    {
        errStr = "Missing field (Door4)";
    }

    if (from.contains(_L("AirDoor")))
    {
        ret.paramsList[LED_IDX_AIR_DOOR] = ToCpp_ActLedParams(from[_L("AirDoor")].toString());
    }
    else
    {
        errStr = "Missing field (AirDoor)";
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

DS_WorkflowDef::SyringeSodStatus ImrParser::ToCpp_SyringeSodStatus(const QVariantMap &from, QString *err)
{
    QString errStr = "";
    DS_WorkflowDef::SyringeSodStatus ret;

    if (from.contains(_L("PlungerEngaged")))
    {
        ret.plungerEngaged = from[_L("PlungerEngaged")].toBool();
    }
    else
    {
        errStr = "Missing field (PlungerEngaged)";
    }

    if (from.contains(_L("Primed")))
    {
        ret.primed = from[_L("Primed")].toBool();
    }
    else
    {
        errStr = "Missing field (Primed)";
    }

    if (from.contains(_L("CalSlackDone")))
    {
        ret.calSlackDone = from[_L("CalSlackDone")].toBool();
    }
    else
    {
        errStr = "Missing field (CalSlackDone)";
    }

    if (from.contains(_L("AirCheckCalibrated")))
    {
        ret.airCheckCalibrated = from[_L("AirCheckCalibrated")].toBool();
    }
    else
    {
        errStr = "Missing field (AirCheckCalibrated)";
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

DS_WorkflowDef::MudsSodStatus ImrParser::ToCpp_MudsSodStatus(const QVariantMap &from, QString *err)
{
    QString errStr = "";
    DS_WorkflowDef::MudsSodStatus ret;

    ret.identified = false;
    if (from.contains(_L("Identified")))
    {
        ret.identified = from[_L("Identified")].toBool();
    }
    else
    {
        errStr = "Missing field (Identified)";
    }

    if (from.contains(_L("SyringeSodStatusAll")))
    {
        QVariantList syringeSodStatusAll = from["SyringeSodStatusAll"].toList();
        for (int syringeIdx = 0; syringeIdx < syringeSodStatusAll.length(); syringeIdx++)
        {
            ret.syringeSodStatusAll[syringeIdx] = ToCpp_SyringeSodStatus(syringeSodStatusAll[syringeIdx].toMap(), &errStr);

            if (errStr != "")
            {
                errStr = QString().asprintf("SyringeSodStatus[%d]: ", syringeIdx) + errStr;
            }
        }
    }
    else
    {
        errStr = "Missing field (SyringeSodStatusAll)";
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

DS_MwlDef::DicomField ImrParser::ToCpp_DicomField(const QVariantMap &from, QString *err)
{
    QString errStr = "";
    DS_MwlDef::DicomField ret;

    if (from.contains(_L("Name")))
    {
        ret.name = from[_L("Name")].toString();
    }
    else
    {
        errStr = "Missing field (Name)";
    }

    if (from.contains(_L("Value")))
    {
        ret.value = from[_L("Value")].toString();
    }
    else
    {
        errStr = "Missing field (Value)";
    }

    if (from.contains(_L("DicomValueType")))
    {
        ret.dicomValueType = from[_L("DicomValueType")].toString();
    }
    else
    {
        errStr = "Missing field (DicomValueType)";
    }

    if (from.contains(_L("ValueType")))
    {
        ret.valueType = from[_L("ValueType")].toString();
    }
    else
    {
        errStr = "Missing field (ValueType)";
    }

    if (from.contains(_L("TranslateName")))
    {
        ret.translateName = from[_L("TranslateName")].toBool();
    }
    else
    {
        errStr = "Missing field (TranslateName)";
    }

    if (from.contains(_L("TranslateValue")))
    {
        ret.translateValue = from[_L("TranslateValue")].toBool();
    }
    else
    {
        errStr = "Missing field (TranslateValue)";
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

DS_MwlDef::DicomFields ImrParser::ToCpp_DicomFields(const QVariantList &from, QString *err)
{
    QString errStr = "";
    DS_MwlDef::DicomFields ret;

    for (int fieldIdx = 0; fieldIdx < from.length(); fieldIdx++)
    {
        DS_MwlDef::DicomField field = ToCpp_DicomField(from[fieldIdx].toMap(), &errStr);

        if (errStr != "")
        {
            errStr = QString().asprintf("DicomFields[%d]: ", fieldIdx) + errStr;
        }

        ret.append(field);
    }

    if (err != NULL)
    {
        *err = errStr;
    }

    return ret;
}

DS_MwlDef::DicomFieldMap ImrParser::ToCpp_DicomFieldMap(const QVariantMap &from, QString *err)
{
    QString errStr = "";
    DS_MwlDef::DicomFieldMap ret;

    QMap<QString, QVariant>::const_iterator i = from.begin();

    i = from.begin();
    while (i != from.end())
    {
        QString key = i.key();
        QVariantMap dicomFieldImr = i.value().toMap();
        DS_MwlDef::DicomField dicomField = ToCpp_DicomField(dicomFieldImr, &errStr);
        if (errStr != "")
        {
            errStr = QString().asprintf("DicomFieldsMap[%s]: ", key.CSTR()) + errStr;
        }
        ret.insert(key, dicomField);
        i++;
    }

    if (err != NULL)
    {
        *err = errStr;
    }

    return ret;
}

DS_MwlDef::WorklistEntry ImrParser::ToCpp_WorklistEntry(const QVariantMap &from, QString *err)
{
    QString errStr = "";
    DS_MwlDef::WorklistEntry ret;

    if (from.contains(_L("StudyInstanceUid")))
    {
        ret.studyInstanceUid = from[_L("StudyInstanceUid")].toString();
    }
    else
    {
        errStr = "Missing field (StudyInstanceUid)";
    }

    if (from.contains(_L("IsAnonymous")))
    {
        ret.isAnonymous = from[_L("IsAnonymous")].toBool();
    }
    else
    {
        errStr = "Missing field (IsAnonymous)";
    }

    if (from.contains(_L("DicomFields")))
    {
        ret.dicomFields = ToCpp_DicomFieldMap(from[_L("DicomFields")].toMap(), &errStr);
        if (errStr != _L(""))
        {
            errStr = "Failed to parse DicomFields: " + errStr;
        }
    }
    else
    {
        errStr = "Missing field (DicomFields)";
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

DS_MwlDef::WorklistEntries ImrParser::ToCpp_WorklistEntries(const QVariantList &from, QString *err)
{
    QString errStr = "";
    DS_MwlDef::WorklistEntries ret;

    for (int entryIdx = 0; entryIdx < from.length(); entryIdx++)
    {
        DS_MwlDef::WorklistEntry entry = ToCpp_WorklistEntry(from[entryIdx].toMap(), &errStr);

        if (errStr != "")
        {
            errStr = QString().asprintf("WorklistEntries[%d]: ", entryIdx) + errStr;
        }

        ret.append(entry);
    }


    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QList<double> ImrParser::ToCpp_DoubleList(const QVariantList &from, QString *err)
{
    QString errStr = "";
    QList<double> ret;

    for (int i = 0; i < from.length(); i++)
    {
        ret.append(from[i].toDouble());
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_ConfigItemValidRange(const Config::ValidRange &from, QString *err)
{
    QVariantMap ret;
    QString errStr = "";

    if ( (from.lowerLimitInclusive.isNull()) ||
         (from.upperLimitInclusive.isNull()) ||
         (from.resolutionIncrement.isNull()) )
    {
        ret = QVariantMap();
    }
    else
    {
        ret.insert("LowerLimitInclusive", from.lowerLimitInclusive);
        ret.insert("UpperLimitInclusive", from.upperLimitInclusive);
        ret.insert("ResolutionIncrement", from.resolutionIncrement);
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_ConfigItem(const Config::Item &from, QString *err)
{
    QVariantMap ret;
    QString errStr = "";

    ret.insert("DefaultValue", from.defaultValue);
    ret.insert("KeyName", from.keyName);
    ret.insert("Units", from.units);
    ret.insert("ValidDataType", from.validDataType);
    ret.insert("ValidList", from.validList);
    ret.insert("DisplayIndex", from.displayIndex);
    ret.insert("ValidRange", ToImr_ConfigItemValidRange(from.validRange));

    if (from.validDataType == _L("bool"))
    {
        ret.insert("Value", from.value.toBool());
    }
    else if (from.validDataType == _L("double"))
    {
        ret.insert("Value", from.value.toDouble());
    }
    else if (from.validDataType == _L("int"))
    {
        ret.insert("Value", from.value.toInt());
    }
    else
    {
        ret.insert("Value", from.value);
    }

    if (from.changedAtEpochMs == -1)
    {
        ret.insert("ChangedAt", QVariant());
    }
    else
    {
        ret.insert("ChangedAt", Util::qDateTimeToUtcDateTimeStr(QDateTime::fromMSecsSinceEpoch(from.changedAtEpochMs)));
    }



    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;

}

QString ImrParser::ToImr_InjectionCompleteStatus(const DS_McuDef::InjectionCompleteStatus &from, QString *err)
{
    QString errStr = "";
    QString ret;

    ret = ToImr_InjectionCompleteReason(from.reason);

    if (from.reason == DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_ALARM_ABORT)
    {
        ret += ";" + QString().asprintf("%d", from.alarmId);
        ret += ";" + from.message;
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_TemperatureReadings(const DS_McuDef::TemperatureReadings &from, QString *err)
{
    QString errStr = "";
    QVariantMap ret;

    for (int heatMaintainerIdx = 0; heatMaintainerIdx < from.length(); heatMaintainerIdx++)
    {
        ret.insert(ToImr_HeatMaintainerIndex((HeatMaintainerIdx)heatMaintainerIdx), from[heatMaintainerIdx]);
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_HeatMaintainerStatus(const DS_McuDef::HeatMaintainerStatus &from, QString *err)
{
    QString errStr = "";
    QVariantMap ret;

    QVariantMap temperatureReadings = ToImr_TemperatureReadings(from.temperatureReadings, &errStr);
    if (errStr != "")
    {
        errStr = "TemperatureReadings: " + errStr;
    }

    ret.insert("TemperatureReadings", temperatureReadings);
    ret.insert("State", ToImr_HeatMaintainerState(from.state));

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_FluidOptions(const DS_DeviceDef::FluidOptions &from, QString *err)
{
    QString errStr = "";
    QVariantMap ret;
    QVariantList contrastFamilies;
    QVariantList salinePackages;
    QVariantMap knownBarcodes;

    contrastFamilies = ToImr_FluidFamilies(from.contrastFamilies, &errStr);
    if (errStr != "")
    {
        errStr = "ContrastFamilies: " + errStr;
    }
    ret.insert("ContrastFamilies", contrastFamilies);

    salinePackages = ToImr_FluidPackages(from.salinePackages, &errStr);
    if (errStr != "")
    {
        errStr = "SalinePackages: " + errStr;
    }
    ret.insert("SalinePackages", salinePackages);

    knownBarcodes = ToImr_KnownBarcodes(from.knownBarcodes, &errStr);
    if (errStr != "")
    {
        errStr = "KnownBarcodes: " + errStr;
    }
    ret.insert("KnownBarcodes", knownBarcodes);

    ret.insert("ChangedAt", Util::qDateTimeToUtcDateTimeStr(QDateTime::fromMSecsSinceEpoch(from.changedAtEpochMs)));

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantList ImrParser::ToImr_FluidFamilies(const DS_DeviceDef::FluidFamilies &from, QString *err)
{
    QString errStr = "";
    QVariantList ret;

    for (int familyIdx = 0; familyIdx< from.length(); familyIdx++)
    {
        QVariantMap contrastFamily = ToImr_ContrastFamily(from[familyIdx], &errStr);
        if (errStr != "")
        {
            errStr = QString().asprintf("FluidFamily[%d]: ", familyIdx) + errStr;
        }
        ret.append(contrastFamily);
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_ContrastFamily(const DS_DeviceDef::FluidFamily &from, QString *err)
{
    QString errStr = "";
    QVariantMap ret;

    ret.insert("Name", from.name);

    QVariantList fluidPackages = ToImr_FluidPackages(from.fluidPackages, &errStr);
    if (errStr != "")
    {
        errStr = "FluidPackages: " + errStr;
    }
    ret.insert("FluidPackages", fluidPackages);

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_FluidPackage(const DS_DeviceDef::FluidPackage &from, QString *err)
{
    QVariantMap ret;
    QString errStr;

    ret.insert("Brand", from.brand);
    ret.insert("Concentration", from.concentration);
    ret.insert("ConcentrationUnits", from.concentrationUnits);
    ret.insert("LotBatch", from.lotBatch);
    ret.insert("Volume", from.volume);
    ret.insert("ExpirationDate", from.expirationDateEpochMs == -1 ? QVariant() : Util::qDateTimeToUtcDateTimeStr(QDateTime::fromMSecsSinceEpoch(from.expirationDateEpochMs)));
    ret.insert("LoadedAt", from.loadedAtEpochMs == -1 ? QVariant() : Util::qDateTimeToUtcDateTimeStr(QDateTime::fromMSecsSinceEpoch(from.loadedAtEpochMs)));
    ret.insert("MaximumUseDuration", from.maximumUseDurationMs == -1 ? QVariant() : Util::millisecToDurationStr(from.maximumUseDurationMs));
    ret.insert("FluidKind", from.fluidKind);
    ret.insert("CompatibilityFamily", from.compatibilityFamily);

    QVariantList barcodePrefixes;
    for (int i = 0; i < from.barcodePrefixes.length(); i++)
    {
        barcodePrefixes.append(from.barcodePrefixes[i]);
    }
    ret.insert("BarcodePrefixes", barcodePrefixes);

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;

}

QVariantList ImrParser::ToImr_FluidPackages(const DS_DeviceDef::FluidPackages &from, QString *err)
{
    QString errStr = "";
    QVariantList ret;

    for (int i = 0; i < from.length(); i++)
    {
        QVariantMap fluidPackage = ToImr_FluidPackage(from[i], &errStr);

        if (errStr != "")
        {
            errStr = QString().asprintf("FluidPackages[%d]: ", i) + errStr;
        }

        ret.append(fluidPackage);
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_FluidInfo(const DS_DeviceDef::FluidInfo &from, QString *err)
{
    QString errStr = "";
    QVariantMap ret;

    ret.insert("FluidPackage", ToImr_FluidPackage(from.fluidPackage, &errStr));

    if (errStr != "")
    {
        errStr = "FluidPackages: " + errStr;
    }

    ret.insert("ColorCode", from.colorCode);
    ret.insert("Location", ToImr_FluidSourceIdx(from.location));

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_KnownBarcodes(const DS_DeviceDef::KnownBarcodes &from, QString *err)
{
    QString errStr = "";
    QVariantMap ret;

    for (int i = 0; i < from.length(); i++)
    {
        DS_DeviceDef::KnownBarcode knownBarcode = from[i];
        QVariantMap barcodeMap = ToImr_FluidPackage(knownBarcode.fluidPackage, &errStr);

        if (errStr != "")
        {
            errStr = QString().asprintf("KnownBarcodes[%d]: ", i) + errStr;
        }

        ret.insert(knownBarcode.barcode, barcodeMap);
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantList ImrParser::ToImr_FluidSourceSelectItems(const DS_DeviceDef::FluidSelectItems &from, QString *err)
{
    QString errStr = "";
    QVariantList ret;

    foreach (DS_DeviceDef::FluidSelectItem package, from)
    {
        QVariantMap map;

        map.insert("Brand", package.brand);
        map.insert("Concentration", package.concentration);
        map.insert("ConcentrationUnits", package.concentrationUnits);
        QVariantList tempList;
        for (int i = 0; i < package.volumes.length(); i++)
        {
            tempList.append(package.volumes[i]);
        }
        map.insert("Volumes", tempList);

        ret.append(map);
    }


    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantList ImrParser::ToImr_FluidSourceSelectItems(const DS_DeviceDef::FluidPackages &from, QString *err)
{
    QVariantList ret;
    QString errStr = "";

    // Prepare selectItems first
    DS_DeviceDef::FluidSelectItems selectItems;
    if (from.length() > 0)
    {
        // Populate first selection of the list
        DS_DeviceDef::FluidSelectItem fluidSelection;
        fluidSelection.getFromFluidPackage(from[0]);
        selectItems.append(fluidSelection);

        for (int i = 1; i < from.length(); i++)
        {
            bool addNewSelection = true;
            for (int j = 0; j < selectItems.length(); j++)
            {
                if (selectItems[j].isSameSelection(from.at(i)))
                {
                    selectItems[j].volumes.append(from.at(i).volume);
                    addNewSelection = false;
                }
            }

            if (addNewSelection)
            {
                DS_DeviceDef::FluidSelectItem tempSelection;
                tempSelection.getFromFluidPackage(from.at(i));
                selectItems.append(tempSelection);
            }
        }
    }

    ret = ToImr_FluidSourceSelectItems(selectItems, &errStr);

    if (errStr != "")
    {
        errStr = "FluidSourceSelectItems: " + errStr;
    }


    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}


QString ImrParser::ToImr_FluidSourceIdx(DS_DeviceDef::FluidSourceIdx from)
{
    switch (from)
    {
    case DS_DeviceDef::FLUID_SOURCE_IDX_BOTTLE_1: return "BS0";
    case DS_DeviceDef::FLUID_SOURCE_IDX_BOTTLE_2: return "BC1";
    case DS_DeviceDef::FLUID_SOURCE_IDX_BOTTLE_3: return "BC2";
    case DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_1: return "RS0";
    case DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_2: return "RC1";
    case DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_3: return "RC2";
    case DS_DeviceDef::FLUID_SOURCE_IDX_MUDS: return "ML";
    case DS_DeviceDef::FLUID_SOURCE_IDX_SUDS: return "PL";
    case DS_DeviceDef::FLUID_SOURCE_IDX_WASTE_CONTAINER: return "WC";
    default: return "UNKNOWN";
    }
}

QString ImrParser::ToImr_FluidSourceSyringeIdx(SyringeIdx from)
{
    switch (from)
    {
    case SYRINGE_IDX_SALINE: return "RS0";
    case SYRINGE_IDX_CONTRAST1: return "RC1";
    case SYRINGE_IDX_CONTRAST2: return "RC2";
    case SYRINGE_IDX_NONE: return "NONE";
    default: return "UNKNOWN";
    }
}

QString ImrParser::ToImr_FluidSourceBottleIdx(SyringeIdx from)
{
    switch (from)
    {
    case SYRINGE_IDX_SALINE: return "BS0";
    case SYRINGE_IDX_CONTRAST1: return "BC1";
    case SYRINGE_IDX_CONTRAST2: return "BC2";
    default: return "UNKNOWN";
    }
}

QString ImrParser::ToImr_FluidSourceDisposableType(DS_DeviceDef::FluidSourceDisposableType from)
{
    QString ret;

    switch (from)
    {
    case DS_DeviceDef::FLUID_SOURCE_TYPE_UNKNOWN:
        ret = "Unknown";
        break;
    case DS_DeviceDef::FLUID_SOURCE_TYPE_FLS_II:
        ret = "FLSII";
        break;
    case DS_DeviceDef::FLUID_SOURCE_TYPE_PFA_100:
        ret = "PFA100";
        break;
    case DS_DeviceDef::FLUID_SOURCE_TYPE_PFA_150:
        ret = "PFA150";
        break;
    case DS_DeviceDef::FLUID_SOURCE_TYPE_PFA_BRACO:
        ret = "PFABracco";
        break;
    case DS_DeviceDef::FLUID_SOURCE_TYPE_LEXAN:
        ret = "LEXAN";
        break;
    case DS_DeviceDef::FLUID_SOURCE_TYPE_MRXP_LEXAN_SALINE:
        ret = "MRXP_Lexan_Saline";
        break;
    case DS_DeviceDef::FLUID_SOURCE_TYPE_MRXP_LEXAN_CONTRAST:
        ret = "MRXP_Lexan_Contrast";
        break;
    case DS_DeviceDef::FLUID_SOURCE_TYPE_MRXP_QWIKFIT_115:
        ret = "MRXP_QWIKFIT_115";
        break;
    case DS_DeviceDef::FLUID_SOURCE_TYPE_MRXP_QWIKFIT_65:
        ret = "MRXP_QWIKFIT_65";
        break;
    case DS_DeviceDef::FLUID_SOURCE_TYPE_SALINET_FLS:
        ret = "Salient_FLS";
        break;
    case DS_DeviceDef::FLUID_SOURCE_TYPE_SY2_WASTE_CONTAINER:
        ret = "SY2_WasteContainer";
        break;
    case DS_DeviceDef::FLUID_SOURCE_TYPE_SY2_SUDS:
        ret = "SY2_SUDS";
        break;
    case DS_DeviceDef::FLUID_SOURCE_TYPE_SY2_MUDS_ASSEMBLY:
        ret = "SY2_MUDS_Assembly";
        break;
    case DS_DeviceDef::FLUID_SOURCE_TYPE_SY2_MUDS_RESERVOIR:
        ret = "SY2_MUDS_Reservoir";
        break;
    case DS_DeviceDef::FLUID_SOURCE_TYPE_SY2_BULK_BAG_BOTTLE:
        ret = "SY2_BulkBagBottle";
        break;
    default:
        ret = "Invalid";
        break;
    }
    return ret;
}

QString ImrParser::ToImr_StatePath(DS_SystemDef::StatePath from)
{
    QString ret;

    switch (from)
    {
    case DS_SystemDef::STATE_PATH_ON_REACHABLE:
        ret = "OnReachable";
        break;
    case DS_SystemDef::STATE_PATH_STARTUP_UNKNOWN:
        ret = "StartupUnknown";
        break;
    case DS_SystemDef::STATE_PATH_IDLE:
        ret = "Idle";
        break;
    case DS_SystemDef::STATE_PATH_READY_ARMED:
        ret = "Ready/Armed";
        break;
    case DS_SystemDef::STATE_PATH_EXECUTING:
        ret = "Executing";
        break;
    case DS_SystemDef::STATE_PATH_BUSY_WAITING:
        ret = "Busy/Waiting";
        break;
    case DS_SystemDef::STATE_PATH_BUSY_FINISHING:
        ret = "Busy/Finishing";
        break;
    case DS_SystemDef::STATE_PATH_BUSY_HOLDING:
        ret = "Busy/Holding";
        break;
    case DS_SystemDef::STATE_PATH_BUSY_SERVICING:
        ret = "Busy/Servicing";
        break;
    case DS_SystemDef::STATE_PATH_ERROR:
        ret = "Error";
        break;
    case DS_SystemDef::STATE_PATH_SERVICING:
        ret = "Servicing";
        break;
    case DS_SystemDef::STATE_PATH_OFF_UNREACHABLE:
    default:
        ret = "OffUnreachable";
        break;
    }

    return ret;
}

QString ImrParser::ToImr_InjectionPlanDigestState(DS_ExamDef::InjectionPlanDigestState from)
{
    switch (from)
    {
    case DS_ExamDef::INJECTION_PLAN_DIGEST_STATE_INITIALISING:
        return "Initialising";
    case DS_ExamDef::INJECTION_PLAN_DIGEST_STATE_BAD_DATA:
        return "BadData";
    case DS_ExamDef::INJECTION_PLAN_DIGEST_STATE_READY:
        return "Ready";
    default:
        return "Unknown";
    }
}

QString ImrParser::ToImr_InjectionPersonalizationInputSource(DS_ExamDef::InjectionPersonalizationInputSource from)
{
    switch (from)
    {
    case DS_ExamDef::INJECTION_PLAN_CUSTOM_PARAM_SOURCE_NONE:
        return "None";
    case DS_ExamDef::INJECTION_PLAN_CUSTOM_PARAM_SOURCE_ENTRY:
        return "Entry";
    case DS_ExamDef::INJECTION_PLAN_CUSTOM_PARAM_SOURCE_CURRENT:
        return "Current";
    case DS_ExamDef::INJECTION_PLAN_CUSTOM_PARAM_SOURCE_DEFAULT:
    default:
        return "Default";
    }
}

QString ImrParser::ToImr_InjectionPersonalizationNoticeType(DS_ExamDef::InjectionPersonalizationNoticeType from)
{
    switch (from)
    {
    case DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_MAXIMUM_IODINE_LOAD_LIMIT_APPLIED:
        return "MaximumIodineLoadLimitApplied";
    case DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_MAXIMUM_CONTRAST_VOLUME_LIMIT_APPLIED:
        return "MaximumContrastVolumeLimitApplied";
    case DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_MAXIMUM_SALINE_VOLUME_LIMIT_APPLIED:
        return "MaximumSalineVolumeLimitApplied";
    case DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_MAXIMUM_FLOW_RATE_LIMIT_APPLIED:
        return "MaximumFlowRateLimitApplied";
    case DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_MINIMUM_IODINE_LOAD_LIMIT_APPLIED:
        return "MinimumIodineLoadLimitApplied";
    case DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_MINIMUM_FLOW_RATE_LIMIT_APPLIED:
        return "MinimumFlowRateLimitApplied";
    case DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_MINIMUM_CONTRAST_VOLUME_LIMIT_APPLIED:
        return "MinimumContrastVolumeLimitApplied";
    case DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_MAXIMUM_IODINE_LOAD_LIMIT_IDR_DECREASED:
        return "MaximumIodineLoadLimitIdrDecreased";
    case DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_MAXIMUM_FLOW_RATE_LIMIT_IDR_DECREASED:
        return "MaximumFlowRateLimitIdrDecreased";
    case DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_MINIMUM_IODINE_LOAD_LIMIT_IDR_INCREASED:
        return "MinimumIodineLoadLimitIdrIncreased";
    case DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_MINIMUM_FLOW_RATE_LIMIT_IDR_INCREASED:
        return "MinimumFlowRateLimitIdrIncreased";
    case DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_MIN_MAX_IODINE_LOAD_FLOW_RATE_CONFLICT:
        return "MinMaxIodineLoadFlowRateConflict";
    case DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_EGFR_EQUATION_NOT_VALID_WITH_IDMS:
        return "EGFREquationNotValidWithIDMS";
    case DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_EGFR_EQUATION_ONLY_VALID_WITH_IDMS:
        return "EGFREquationOnlyValidWithIDMS";
    case DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_EGFR_MEASUREMENT_OLDER_THAN_LIMIT:
        return "EGFRMeasurementOlderThanLimit";
    case DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_INJECTION_VOLUMES_DECREASED_BY_KRULESET:
        return "InjectionVolumesDecreasedByKruleset";
    case DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_INJECTION_VOLUMES_INCREASED_BY_KRULESET:
        return "InjectionVolumesIncreasedByKruleset";
    case DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_INJECTION_VOLUMES_UNAFFECTED_BY_KRULESET:
        return "InjectionVolumesUnaffectedByKruleset";
    case DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_KVP_NOT_CONFIGURED_IN_KRULESET:
        return "KvpNotConfiguredInKruleset";
    case DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_FLOW_RATE_REDUCED_BY_CATHETER_TYPE_LIMIT:
        return "FlowrateReducedByCatheterTypeLimit";
    case DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_FLOW_RATE_REDUCED_BY_CATHETER_PLACEMENT_LIMIT:
        return "FlowrateReducedByCatheterPlacementLimit";
    case DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_FLOW_RATE_EXCEEDS_CATHETER_TYPE_LIMIT:
        return "FlowRateExceedsCatheterTypeLimit";
    case DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_FLOW_RATE_EXCEEDS_CATHETER_PLACEMENT_LIMIT:
        return "FlowRateExceedsCatheterPlacementLimit";
    case DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_PRESSURE_LIMIT_REDUCED_BY_CATHETER_TYPE_LIMIT:
        return "PressureLimitReducedByCatheterTypeLimit";
    case DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_PRESSURE_LIMIT_EXCEEDS_CATHETER_TYPE_LIMIT:
        return "PressureLimitExceedsCatheterTypeLimit";
    case DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_FLOW_RATE_INCREASED_BY_INJECTOR_CONTEXT_AND_DURATION_MAINTAINED:
        return "FlowRateIncreasedByInjectorContextAndDurationMaintained";
    case DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_FLOW_RATE_INCREASED_BY_INJECTOR_CONTEXT_AND_VOLUME_MAINTAINED:
        return "FlowRateIncreasedByInjectorContextAndVolumeMaintained";
    case DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_FLOW_RATE_DECREASED_BY_INJECTOR_CONTEXT_AND_DURATION_MAINTAINED:
        return "FlowRateDecreasedByInjectorContextAndDurationMaintained";
    case DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_FLOW_RATE_DECREASED_BY_INJECTOR_CONTEXT_AND_VOLUME_MAINTAINED:
        return "FlowRateDecreasedByInjectorContextAndVolumeMaintained";
    case DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_CONTRAST_VOLUME_DECREASED_BY_INJECTOR_CONTEXT_AND_DURATION_MAINTAINED:
        return "ContrastVolumeDecreasedByInjectorContextAndDurationMaintained";
	case DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_CONTRAST_VOLUME_DECREASED_BY_INJECTOR_CONTEXT:
        return "ContrastVolumeDecreasedByInjectorContext";
    case DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_CONTRAST_VOLUME_INCREASED_BY_INJECTOR_CONTEXT_AND_DURATION_MAINTAINED:
        return "ContrastVolumeIncreasedByInjectorContextAndDurationMaintained";
	case DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_CONTRAST_VOLUME_INCREASED_BY_INJECTOR_CONTEXT:
        return "ContrastVolumeIncreasedByInjectorContext";
    case DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_SALINE_VOLUME_DECREASED_BY_INJECTOR_CONTEXT_AND_DURATION_MAINTAINED:
        return "SalineVolumeDecreasedByInjectorContextAndDurationMaintained";
    case DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_SALINE_VOLUME_DECREASED_BY_INJECTOR_CONTEXT:
        return "SalineVolumeDecreasedByInjectorContext";		
    case DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_SALINE_VOLUME_INCREASED_BY_INJECTOR_CONTEXT_AND_DURATION_MAINTAINED:
        return "SalineVolumeIncreasedByInjectorContextAndDurationMaintained";
    case DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_SALINE_VOLUME_INCREASED_BY_INJECTOR_CONTEXT:
        return "SalineVolumeIncreasedByInjectorContext";
    case DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_UNKNOWN:
    default:
        return "Unknown";
    }
}

QString ImrParser::ToImr_InjectionPersonalizationNoticeImportance(DS_ExamDef::InjectionPersonalizationNoticeImportance from)
{
    switch (from)
    {
    case DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_SEVERITY_INFO:
        return "Info";
    case DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_SEVERITY_WARNING:
        return "Warning";
    case DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_SEVERITY_ERROR:
        return "Error";
    case DS_ExamDef::INJECTION_STEP_CUSTOM_NOTICE_SEVERITY_UNKNOWN:
    default:
        return "Unknown";
    }
}

QString ImrParser::ToImr_ExamProgressState(DS_ExamDef::ExamProgressState from)
{
    QString ret;

    switch (from)
    {
    case DS_ExamDef::EXAM_PROGRESS_STATE_IDLE:
        ret = "Idle";
        break;
    case DS_ExamDef::EXAM_PROGRESS_STATE_PREPARED:
        ret = "Prepared";
        break;
    case DS_ExamDef::EXAM_PROGRESS_STATE_STARTED:
        ret = "Started";
        break;
    case DS_ExamDef::EXAM_PROGRESS_STATE_PATIENT_SELECTION:
        ret = "PatientSelection";
        break;
    case DS_ExamDef::EXAM_PROGRESS_STATE_PROTOCOL_SELECTION:
        ret = "ProtocolSelection";
        break;
    case DS_ExamDef::EXAM_PROGRESS_STATE_PROTOCOL_MODIFICATION:
        ret = "ProtocolModification";
        break;
    case DS_ExamDef::EXAM_PROGRESS_STATE_INJECTION_EXECUTION:
        ret = "InjectionExecution";
        break;
    case DS_ExamDef::EXAM_PROGRESS_STATE_SUMMARY_CONFIRMATION:
        ret = "SummaryConfirmation";
        break;
    case DS_ExamDef::EXAM_PROGRESS_STATE_COMPLETING:
        ret = "Completing";
        break;
    case DS_ExamDef::EXAM_PROGRESS_STATE_COMPLETED:
        ret = "Completed";
        break;
    default:
        ret = "Invalid";
        break;
    }

    return ret;
}

QString ImrParser::ToImr_InjectionPhaseType(DS_ExamDef::InjectionPhaseType from)
{
    QString ret;
    switch (from)
    {
    case DS_ExamDef::INJECTION_PHASE_TYPE_UNKNOWN:
        ret = "Unknown";
        break;
    case DS_ExamDef::INJECTION_PHASE_TYPE_DELAY:
        ret = "Delay";
        break;
    case DS_ExamDef::INJECTION_PHASE_TYPE_FLUID:
        ret = "Fluid";
        break;
    default:
        ret = "Invalid";
        break;
    }
    return ret;
}


QString ImrParser::ToImr_StepTerminationReason(DS_ExamDef::StepTerminationReason from)
{
    QString ret;
    switch (from)
    {
    case DS_ExamDef::STEP_TERMINATION_REASON_UNKNOWN:
        ret = "Unknown";
        break;
    case DS_ExamDef::STEP_TERMINATION_REASON_NORMAL:
        ret = "Normal";
        break;
    case DS_ExamDef::STEP_TERMINATION_REASON_ABORTED_BY_REQUEST:
        ret = "AbortedByRequest";
        break;
    case DS_ExamDef::STEP_TERMINATION_REASON_ABORTED_BY_OVER_PRESSURE:
        ret = "AbortedByOverPressure";
        break;
    case DS_ExamDef::STEP_TERMINATION_REASON_ABORTED_BY_OTHER_INTERNAL:
        ret = "AbortedByOtherInternal";
        break;
    case DS_ExamDef::STEP_TERMINATION_REASON_ABORTED_BY_INJECTOR_COMM_LOSS:
        ret = "InjectorCommLoss";
        break;
    case DS_ExamDef::STEP_TERMINATION_REASON_ABORTED_BY_INJECTOR_CRITICAL_ERROR:
        ret = "InjectorCriticalError";
        break;
    case DS_ExamDef::STEP_TERMINATION_REASON_ABORTED_BY_STALLING:
        ret = "AbortedByStalling";
        break;
    case DS_ExamDef::STEP_TERMINATION_REASON_ABORTED_BY_HOLD_TIMEOUT:
        ret = "AbortedByHoldTimeout";
        break;
    case DS_ExamDef::STEP_TERMINATION_REASON_ABORTED_BY_SCANNER:
        ret = "AbortedByScanner";
        break;
    case DS_ExamDef::STEP_TERMINATION_REASON_ABORTED_BY_AIR_DETECTION:
        ret = "AbortedByAirDetection";
        break;
    case DS_ExamDef::STEP_TERMINATION_REASON_ABORTED_BY_DISPOSABLE_REMOVAL:
        ret = "AbortedByDisposableRemoval";
        break;
    case DS_ExamDef::STEP_TERMINATION_REASON_ABORTED_BY_CRITICAL_BATTERY_LEVEL:
        ret = "AbortedByCriticalBatteryLevel";
        break;
    case DS_ExamDef::STEP_TERMINATION_REASON_ABORTED_BY_HARDWARE_FAULT:
        ret = "AbortedByHardwareFault";
        break;
    default:
        ret = "Invalid";
        break;
    }
    return ret;
}

QString ImrParser::ToImr_TimedReminderDisplayHint(DS_ExamDef::TimedReminderDisplayHint from)
{
    QString ret;
    switch (from)
    {
    case DS_ExamDef::TIMED_REMINDER_DISPLAY_HINT_UNKNOWN:
        ret = "Unknown";
        break;
    case DS_ExamDef::TIMED_REMINDER_DISPLAY_HINT_COUNT_UP:
        ret = "CountUp";
        break;
    case DS_ExamDef::TIMED_REMINDER_DISPLAY_HINT_COUNT_DOWN:
        ret = "CountDown";
        break;
    case DS_ExamDef::TIMED_REMINDER_DISPLAY_HINT_VISUAL:
        ret = "Visual";
        break;
    case DS_ExamDef::TIMED_REMINDER_DISPLAY_HINT_AUDIBLE:
        ret = "Audible";
        break;
    case DS_ExamDef::TIMED_REMINDER_DISPLAY_HINT_FOREGROUND_COUNT:
        ret = "ForegroundCount";
        break;
    case DS_ExamDef::TIMED_REMINDER_DISPLAY_HINT_BACKGROUND_COUNT:
        ret = "BackgroundCount";
        break;
    default:
        ret = "Invalid";
        break;
    }

    return ret;
}

QString ImrParser::ToImr_ScannerInterfaceStatus(DS_ExamDef::ScannerInterfaceStatus from)
{
    QString ret;
    switch (from)
    {
    case DS_ExamDef::SCANNER_INTERFACE_STATUS_DISABLED_TEMPORARILY:
        ret = "DisabledTemporarily";
        break;
    case DS_ExamDef::SCANNER_INTERFACE_STATUS_DISABLED_PERMANENTLY:
        ret = "DisabledPermanently";
        break;
    case DS_ExamDef::SCANNER_INTERFACE_STATUS_ENABLED:
        ret = "Enabled";
        break;
    case DS_ExamDef::SCANNER_INTERFACE_STATUS_ACTIVE:
        ret = "Active";
        break;
    case DS_ExamDef::SCANNER_INTERFACE_STATUS_WAITING:
        ret = "Waiting";
        break;
    default:
        ret = "Unknown";
        break;
    }

    return ret;
}

QString ImrParser::ToImr_ScannerInterfaceMode(DS_ExamDef::ScannerInterfaceMode from)
{
    QString ret;
    switch (from)
    {
    case DS_ExamDef::SCANNER_INTERFACE_MODE_MONITOR:
        ret = "Monitor";
        break;
    case DS_ExamDef::SCANNER_INTERFACE_MODE_TRACKING:
        ret = "Tracking";
        break;
    case DS_ExamDef::SCANNER_INTERFACE_MODE_CONTROL:
        ret = "Control";
        break;
    case DS_ExamDef::SCANNER_INTERFACE_MODE_SYNCHRONIZATION:
        ret = "Synchronization";
        break;
    default:
        ret = "Unknown";
        break;
    }

    return ret;
}

QString ImrParser::ToImr_InjectionPhaseProgressState(DS_ExamDef::InjectionPhaseProgressState from)
{
    QString ret;
    switch (from)
    {
    case DS_ExamDef::INJECTION_PHASE_PROGRESS_STATE_UNKNOWN:
        ret = "Unknown";
        break;
    case DS_ExamDef::INJECTION_PHASE_PROGRESS_STATE_PROGRESS:
        ret = "Progress";
        break;
    case DS_ExamDef::INJECTION_PHASE_PROGRESS_STATE_JUMPED:
        ret = "Jumped";
        break;
    case DS_ExamDef::INJECTION_PHASE_PROGRESS_STATE_ABORTED:
        ret = "Aborted";
        break;
    case DS_ExamDef::INJECTION_PHASE_PROGRESS_STATE_COMPLETED:
        ret = "Completed";
        break;
    default:
        ret = "Invalid";
        break;
    }
    return ret;
}

QString ImrParser::ToImr_BatteryLevel(DS_McuDef::BatteryLevel from)
{
    QString ret;
    switch (from)
    {
    case DS_McuDef::BATTERY_LEVEL_UNKNOWN:
        ret = "Unknown";
        break;
    case DS_McuDef::BATTERY_LEVEL_NO_BATTERY:
        ret = "NoBattery";
        break;
    case DS_McuDef::BATTERY_LEVEL_FULL:
        ret = "Full";
        break;
    case DS_McuDef::BATTERY_LEVEL_HIGH:
        ret = "High";
        break;
    case DS_McuDef::BATTERY_LEVEL_MEDIUM:
        ret = "Medium";
        break;
    case DS_McuDef::BATTERY_LEVEL_LOW:
        ret = "Low";
        break;
    case DS_McuDef::BATTERY_LEVEL_FLAT:
        ret = "Flat";
        break;
    case DS_McuDef::BATTERY_LEVEL_DEAD:
        ret = "Dead";
        break;
    case DS_McuDef::BATTERY_LEVEL_CRITICAL:
        ret = "Critical";
        break;
    default:
        ret = "Invalid";
        break;
    }

    return ret;
}

QString ImrParser::ToImr_HeatMaintainerState(DS_McuDef::HeatMaintainerState from)
{
    QString ret;
    switch (from)
    {
    case DS_McuDef::HEAT_MAINTAINER_STATE_ENABLED_ON:
        ret = "EnabledOn";
        break;
    case DS_McuDef::HEAT_MAINTAINER_STATE_ENABLED_OFF:
        ret = "EnabledOff";
        break;
    case DS_McuDef::HEAT_MAINTAINER_STATE_DISABLED:
        ret = "Disabled";
        break;
    case DS_McuDef::HEAT_MAINTAINER_STATE_CUTOFF:
        ret = "Cutoff";
        break;
    default:
        ret = "Unknown";
        break;
    }

    return ret;
}

QString ImrParser::ToImr_CruLinkType(DS_CruDef::CruLinkType from)
{
    QString ret;

    switch (from)
    {
    case DS_CruDef::CRU_LINK_TYPE_WIRED:
        ret = "WiredEthernet";
        break;
    case DS_CruDef::CRU_LINK_TYPE_WIRELESS:
        ret = "WirelessEthernet";
        break;
    default:
        ret = "Unknown";
        break;
    }

    return ret;
}

QString ImrParser::ToImr_CruLinkState(DS_CruDef::CruLinkState from)
{
    QString ret;

    switch (from)
    {
    case DS_CruDef::CRU_LINK_STATE_INACTIVE:
        ret = "Inactive";
        break;
    case DS_CruDef::CRU_LINK_STATE_RECOVERING:
        ret = "Recovering";
        break;
    case DS_CruDef::CRU_LINK_STATE_ACTIVE:
        ret = "Active";
        break;
    default:
        ret = "Unknown";
        break;
    }

    return ret;
}

QString ImrParser::ToImr_CruLinkQuality(DS_CruDef::CruLinkQuality from)
{
    QString ret;

    switch (from)
    {
    case DS_CruDef::CRU_LINK_QUALITY_POOR:
        ret = "Poor";
        break;
    case DS_CruDef::CRU_LINK_QUALITY_FAIR:
        ret = "Fair";
        break;
    case DS_CruDef::CRU_LINK_QUALITY_GOOD:
        ret = "Good";
        break;
    case DS_CruDef::CRU_LINK_QUALITY_EXCELLENT:
        ret = "Excellent";
        break;
    default:
        ret = "Unknown";
        break;
    }

    return ret;

}

QString ImrParser::ToImr_StopcockPos(DS_McuDef::StopcockPos from)
{
    QString ret;

    switch (from)
    {
    case DS_McuDef::STOPCOCK_POS_NONE:
        ret = "NONE";
        break;
    case DS_McuDef::STOPCOCK_POS_FILL:
        ret = "FILL";
        break;
    case DS_McuDef::STOPCOCK_POS_INJECT:
        ret = "INJECT";
        break;
    case DS_McuDef::STOPCOCK_POS_CLOSED:
        ret = "CLOSED";
        break;
    case DS_McuDef::STOPCOCK_POS_MOVING:
        ret = "MOVING";
        break;
    case DS_McuDef::STOPCOCK_POS_DISENGAGED:
        ret = "DISENGAGED";
        break;
    default:
        ret = "UNKNOWN";
        break;
    }

    return ret;
}

QString ImrParser::ToImr_McuLinkState(DS_McuDef::LinkState from)
{
    switch (from)
    {
    case DS_McuDef::LINK_STATE_DISCONNECTED:
        return "DISCONNECTED";
    case DS_McuDef::LINK_STATE_CONNECTING:
        return "CONNECTING";
    case DS_McuDef::LINK_STATE_RECOVERING:
        return "RECOVERING";
    case DS_McuDef::LINK_STATE_CONNECTED:
        return "CONNECTED";
    default:
        return QString().asprintf("UNKNOWN %d", from);
    }
}

QString ImrParser::ToImr_DoorState(DS_McuDef::DoorState from)
{
    QString ret;
    switch (from)
    {
    case DS_McuDef::DOOR_OPEN:
        ret = "OPEN";
        break;
    case DS_McuDef::DOOR_CLOSED:
        ret = "CLOSED";
        break;
    case DS_McuDef::DOOR_UNKNOWN:
        ret = "UNKNOWN";
        break;
    default:
        ret = QString().asprintf("UNKNOWN %d", from);
        break;
    }
    return ret;
}

QString ImrParser::ToImr_WasteBinState(DS_McuDef::WasteBinState from)
{
    QString ret;
    switch (from)
    {
    case DS_McuDef::WASTE_BIN_STATE_MISSING:
        ret = "MISSING";
        break;
    case DS_McuDef::WASTE_BIN_STATE_LOW_FILLED:
        ret = "LOW";
        break;
    case DS_McuDef::WASTE_BIN_STATE_HIGH_FILLED:
        ret = "HIGH";
        break;
    case DS_McuDef::WASTE_BIN_STATE_FULLY_FILLED:
        ret = "FULL";
        break;
    case DS_McuDef::WASTE_BIN_STATE_COMM_DOWN:
        ret = "COMM_DOWN";
        break;
    case DS_McuDef::WASTE_BIN_STATE_UNKNOWN:
        ret = "UNKNOWN";
        break;
    default:
        ret = QString().asprintf("UNKNOWN %d", from);
        break;
    }
    return ret;
}

QString ImrParser::ToImr_OutletDoorState(DS_McuDef::OutletDoorState from)
{
    QString ret;
    switch (from)
    {
    case DS_McuDef::OUTLET_DOOR_STATE_OPEN:
        ret = "OPEN";
        break;
    case DS_McuDef::OUTLET_DOOR_STATE_CLOSED:
        ret = "CLOSED";
        break;
    default:
        ret = "UNKNOWN";
        break;
    }
    return ret;
}

QString ImrParser::ToImr_PlungerState(DS_McuDef::PlungerState from)
{
    QString strRet;
    switch (from)
    {
    case DS_McuDef::PLUNGER_STATE_DISENGAGED:
        strRet = "DISENGAGED";
        break;
    case DS_McuDef::PLUNGER_STATE_ENGAGED:
        strRet = "ENGAGED";
        break;
    case DS_McuDef::PLUNGER_STATE_LOCKFAILED:
        strRet = "LOCKFAILED";
        break;
    case DS_McuDef::PLUNGER_STATE_UNKNOWN:
    default:
        strRet = "UNKNOWN";
        break;
    }
    return strRet;
}
QString ImrParser::ToImr_SyringeState(DS_McuDef::SyringeState from)
{
    QString ret;
    switch (from)
    {
    case DS_McuDef::SYRINGE_STATE_PROCESSING:
        ret = "PROCESSING";
        break;
    case DS_McuDef::SYRINGE_STATE_COMPLETED:
        ret = "COMPLETED";
        break;
    case DS_McuDef::SYRINGE_STATE_USER_ABORT:
        ret = "USER_ABORT";
        break;
    case DS_McuDef::SYRINGE_STATE_AIR_DETECTED:
        ret = "AIR_DETECTED";
        break;
    case DS_McuDef::SYRINGE_STATE_LOST_PLUNGER:
        ret = "LOST_PLUNGER";
        break;
    case DS_McuDef::SYRINGE_STATE_PLUNGER_ENGAGE_FAULT:
        ret = "PLUNGER_ENGAGE_FAULT";
        break;
    case DS_McuDef::SYRINGE_STATE_PLUNGER_DISENGAGE_FAULT:
        ret = "PLUNGER_DISENGAGE_FAULT";
        break;
    case DS_McuDef::SYRINGE_STATE_MOTOR_FAIL:
        ret = "MOTOR_FAIL";
        break;
    case DS_McuDef::SYRINGE_STATE_OVER_PRESSURE:
        ret = "OVER_PRESSURE";
        break;
    case DS_McuDef::SYRINGE_STATE_OVER_CURRENT:
        ret = "OVER_CURRENT";
        break;
    case DS_McuDef::SYRINGE_STATE_SUDS_REMOVED:
        ret = "SUDS_REMOVED";
        break;
    case DS_McuDef::SYRINGE_STATE_SUDS_INSERTED:
        ret = "SUDS_INSERTED";
        break;
    case DS_McuDef::SYRINGE_STATE_MUDS_REMOVED:
        ret = "MUDS_REMOVED";
        break;
    case DS_McuDef::SYRINGE_STATE_INSUFFICIENT_FLUID:
        ret = "INSUFFICIENT_FLUID";
        break;
    case DS_McuDef::SYRINGE_STATE_TIMEOUT:
        ret = "TIMEOUT";
        break;
    case DS_McuDef::SYRINGE_STATE_HOME_SENSOR_MISSING:
        ret = "HOME_SENSOR_MISSING";
        break;
    case DS_McuDef::SYRINGE_STATE_INVALID_STATE:
        ret = "INVALID_STATE";
        break;
    case DS_McuDef::SYRINGE_STATE_BAD_DATA:
        ret = "BAD_DATA";
        break;
    case DS_McuDef::SYRINGE_STATE_FRAM_FAULT:
        ret = "FRAM_FAULT";
        break;
    case DS_McuDef::SYRINGE_STATE_STOP_PENDING:
        ret = "STOP_PENDING";
        break;
    case DS_McuDef::SYRINGE_STATE_SPIKE_MISSING:
        ret = "SPIKE_MISSING";
        break;
    case DS_McuDef::SYRINGE_STATE_BAD_STOPCOCK_POSITION:
        ret = "BAD_STOPCOCK_POSITION";
        break;
    default:
        ret = QString().asprintf("ERR%d", from);
        break;
    }
    return ret;
}

QString ImrParser::ToImr_InjectionPhaseType(DS_McuDef::InjectionPhaseType from)
{
    QString ret;
    switch (from)
    {
    case DS_McuDef::INJECTION_PHASE_TYPE_NONE:
        ret = "NONE";
        break;
    case DS_McuDef::INJECTION_PHASE_TYPE_PAUSE:
        ret = "PAUSE";
        break;
    case DS_McuDef::INJECTION_PHASE_TYPE_CONTRAST1:
        ret = "CONTRAST1";
        break;
    case DS_McuDef::INJECTION_PHASE_TYPE_CONTRAST2:
        ret = "CONTRAST2";
        break;
    case DS_McuDef::INJECTION_PHASE_TYPE_SALINE:
        ret = "SALINE";
        break;
    case DS_McuDef::INJECTION_PHASE_TYPE_DUAL1:
        ret = "DUAL1";
        break;
    case DS_McuDef::INJECTION_PHASE_TYPE_DUAL2:
        ret = "DUAL2";
        break;
    default:
        ret = "UNKNOWN";
        break;
    }
    return ret;
}

QString ImrParser::ToImr_ArmType(DS_ExamDef::ArmType from)
{
    switch (from)
    {
    case DS_ExamDef::ARM_TYPE_NORMAL:
        return "NORMAL";
    case DS_ExamDef::ARM_TYPE_PRELOAD_FIRST:
        return "FIRST";
    case DS_ExamDef::ARM_TYPE_PRELOAD_SECOND:
        return "SECOND";
    default:
        return QString().asprintf("UNKNOWN %d", from);
    }
}

QString ImrParser::ToImr_InjectorState(DS_McuDef::InjectorState from)
{
    switch (from)
    {
    case DS_McuDef::INJECTOR_STATE_READY_START:
        return "READY_START";
    case DS_McuDef::INJECTOR_STATE_DELIVERING:
        return "DELIVERING";
    case DS_McuDef::INJECTOR_STATE_HOLDING:
        return "HOLDING";
    case DS_McuDef::INJECTOR_STATE_PHASE_PAUSED:
        return "PHASE_PAUSED";
    case DS_McuDef::INJECTOR_STATE_COMPLETING:
        return "COMPLETING";
    case DS_McuDef::INJECTOR_STATE_COMPLETED:
        return "COMPLETED";
    default:
        return "IDLE";
        break;
    }
}

QString ImrParser::ToImr_InjectionCompleteReason(DS_McuDef::InjectionCompleteReason from)
{
    switch (from)
    {
    case DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_NORMAL:
        return "COMPLETED_NORMAL";
    case DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_ALLSTOP_ABORT:
        return "COMPLETED_ALLSTOP_ABORT";
    case DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_USER_ABORT:
        return "COMPLETED_USER_ABORT";
    case DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_HOLD_TIMEOUT:
        return "COMPLETED_HOLD_TIMEOUT";
    case DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_ALARM_ABORT:
        return "COMPLETED_ALARM_ABORT";
    case DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_OVER_PRESSURE:
        return "COMPLETED_OVER_PRESSURE";
    case DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_OVER_CURRENT:
        return "COMPLETED_OVER_CURRENT";
    case DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_AIR_DETECTED:
        return "COMPLETED_AIR_DETECTED";
    case DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_SUDS_MISSING:
        return "COMPLETED_SUDS_MISSING";
    case DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_MOTOR_STALL_1:
        return "COMPLETED_MOTOR_STALL_S0";
    case DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_MOTOR_STALL_2:
        return "COMPLETED_MOTOR_STALL_C1";
    case DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_MOTOR_STALL_3:
        return "COMPLETED_MOTOR_STALL_C2";
    case DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_MOTOR_SLIP_1:
        return "COMPLETED_MOTOR_SLIP_S0";
    case DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_MOTOR_SLIP_2:
        return "COMPLETED_MOTOR_SLIP_C1";
    case DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_MOTOR_SLIP_3:
        return "COMPLETED_MOTOR_SLIP_C2";
    case DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_HCU_COMM_DOWN:
        return "COMPLETED_HCU_COMM_DOWN";
    case DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_OVER_TEMPERATURE:
        return "COMPLETED_OVER_TEMPERATURE";
    case DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_BATTERY_CRITICAL:
        return "COMPLETED_BATTERY_CRITICAL";
    case DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_STOPCOCK_COMM_ERROR:
        return "COMPLETED_STOPCOCK_COMM_ERROR";
    case DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_MUDS_UNLATCHED:
        return "COMPLETED_MUDS_UNLATCHED";
    case DS_McuDef::INJECTION_COMPLETE_REASON_DISARMED_SUDS_MISSING:
        return "DISARMED_SUDS_MISSING";
    case DS_McuDef::INJECTION_COMPLETE_REASON_DISARMED_HCU_ABORT:
        return "DISARMED_HCU_ABORT";
    case DS_McuDef::INJECTION_COMPLETE_REASON_DISARMED_STOP_BUTTON_ABORT:
        return "DISARMED_STOP_BUTTON_ABORT";
    case DS_McuDef::INJECTION_COMPLETE_REASON_DISARMED_ARM_TIMEOUT:
        return "DISARMED_ARM_TIMEOUT";
    case DS_McuDef::INJECTION_COMPLETE_REASON_DISARMED_MUDS_UNLATCHED:
        return "DISARMED_MUDS_UNLATCHED";
    default:
        return "UNKNOWN";
    }
}

QString ImrParser::ToImr_BottleBubbleDetectorState(DS_McuDef::BottleBubbleDetectorState from)
{
    QString ret;
    switch (from)
    {
    case DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_AIR:
        ret = "AIR";
        break;
    case DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_FLUID:
        ret = "FLUID";
        break;
    case DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_MISSING:
        ret = "MISSING";
        break;
    case DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_UNKNOWN:
    default:
        ret = "UNKNOWN";
        break;
    }
    return ret;
}

QString ImrParser::ToImr_PressureCalibrationState(DS_McuDef::PressureCalibrationState from)
{
    switch (from)
    {
    case DS_McuDef::PRESSURE_CAL_STATE_IDLE:
        return "PRESSURE_CAL_STATE_IDLE";
    case DS_McuDef::PRESSURE_CAL_STATE_STARTED:
        return "PRESSURE_CAL_STATE_STARTED";
    case DS_McuDef::PRESSURE_CAL_STATE_DISENGAGE_STARTED:
        return "PRESSURE_CAL_STATE_DISENGAGE_STARTED";
    case DS_McuDef::PRESSURE_CAL_STATE_DISENGAGE_PROGRESS:
        return "PRESSURE_CAL_STATE_DISENGAGE_PROGRESS";
    case DS_McuDef::PRESSURE_CAL_STATE_DISENGAGE_FAILED:
        return "PRESSURE_CAL_STATE_DISENGAGE_FAILED";
    case DS_McuDef::PRESSURE_CAL_STATE_DISENGAGE_COMPLETED:
        return "PRESSURE_CAL_STATE_DISENGAGE_COMPLETED";
    case DS_McuDef::PRESSURE_CAL_STATE_FIND_PLUNGER_STARTED:
        return "PRESSURE_CAL_STATE_FIND_PLUNGER_STARTED";
    case DS_McuDef::PRESSURE_CAL_STATE_FIND_PLUNGER_PROGRESS:
        return "PRESSURE_CAL_STATE_FIND_PLUNGER_PROGRESS";
    case DS_McuDef::PRESSURE_CAL_STATE_FIND_PLUNGER_FAILED:
        return "PRESSURE_CAL_STATE_FIND_PLUNGER_FAILED";
    case DS_McuDef::PRESSURE_CAL_STATE_FIND_PLUNGER_COMPLETED:
        return "PRESSURE_CAL_STATE_FIND_PLUNGER_COMPLETED";
    case DS_McuDef::PRESSURE_CAL_STATE_INJECT_STAGE_STARTED:
        return "PRESSURE_CAL_STATE_INJECT_STAGE_STARTED";
    case DS_McuDef::PRESSURE_CAL_STATE_INJECT_STAGE_PROGRESS:
        return "PRESSURE_CAL_STATE_INJECT_STAGE_PROGRESS";
    case DS_McuDef::PRESSURE_CAL_STATE_INJECT_STAGE_FAILED:
        return "PRESSURE_CAL_STATE_INJECT_STAGE_FAILED";
    case DS_McuDef::PRESSURE_CAL_STATE_INJECT_STAGE_COMPLETED:
        return "PRESSURE_CAL_STATE_INJECT_STAGE_COMPLETED";
    case DS_McuDef::PRESSURE_CAL_STATE_FINAL_DISENGAGE_STARTED:
        return "PRESSURE_CAL_STATE_FINAL_DISENGAGE_STARTED";
    case DS_McuDef::PRESSURE_CAL_STATE_FINAL_DISENGAGE_PROGRESS:
        return "PRESSURE_CAL_STATE_FINAL_DISENGAGE_PROGRESS";
    case DS_McuDef::PRESSURE_CAL_STATE_FINAL_DISENGAGE_FAILED:
        return "PRESSURE_CAL_STATE_FINAL_DISENGAGE_FAILED";
    case DS_McuDef::PRESSURE_CAL_STATE_FINAL_DISENGAGE_COMPLETED:
        return "PRESSURE_CAL_STATE_FINAL_DISENGAGE_COMPLETED";
    case DS_McuDef::PRESSURE_CAL_STATE_FAILED:
        return "PRESSURE_CAL_STATE_FAILED";
    case DS_McuDef::PRESSURE_CAL_STATE_DONE:
        return "PRESSURE_CAL_STATE_DONE";
    default:
        return QString().asprintf("Unknown State%d", from);
    }
}

QString ImrParser::ToImr_LedIndex(LedIndex from)
{
    QString ret;
    switch (from)
    {
    case LED_IDX_SALINE:
        ret = "Saline";
        break;
    case LED_IDX_CONTRAST1:
        ret = "Contrast1";
        break;
    case LED_IDX_CONTRAST2:
        ret = "Contrast2";
        break;
    case LED_IDX_SUDS1:
        ret = "Suds1";
        break;
    case LED_IDX_SUDS2:
        ret = "Suds2";
        break;
    case LED_IDX_SUDS3:
        ret = "Suds3";
        break;
    case LED_IDX_SUDS4:
        ret = "Suds4";
        break;
    case LED_IDX_DOOR1:
        ret = "Door1";
        break;
    case LED_IDX_DOOR2:
        ret = "Door2";
        break;
    case LED_IDX_DOOR3:
        ret = "Door3";
        break;
    case LED_IDX_DOOR4:
        ret = "Door4";
        break;
    case LED_IDX_AIR_DOOR:
        ret = "AirDoor";
        break;
    default:
        ret = "Unknown";
        break;
    }
    return ret;
}

QString ImrParser::ToImr_LedControlType(DS_McuDef::LedControlType from)
{
    switch (from)
    {
    case DS_McuDef::LED_CONTROL_TYPE_NO_CHANGE:
        return "NO_CHANGE";
        break;
    case DS_McuDef::LED_CONTROL_TYPE_OFF:
        return "OFF";
        break;
    case DS_McuDef::LED_CONTROL_TYPE_SET:
        return "SET";
    default:
        return "UNKNOWN";
    }
}

QString ImrParser::ToImr_HeatMaintainerIndex(HeatMaintainerIdx from)
{
    switch (from)
    {
    case HEAT_MAINTAINER_IDX_DOOR:
        return "DOOR";
    case HEAT_MAINTAINER_IDX_MUDS:
        return "MUDS";
    default:
        return "UNKNOWN";
    }
}

QString ImrParser::ToImr_DataServiceActionState(DataServiceActionStateType from)
{
    switch (from)
    {
    case DS_ACTION_STATE_INIT:
        return "Init";
    case DS_ACTION_STATE_START_WAITING:
        return "Waiting";
    case DS_ACTION_STATE_STARTED:
        return "Started";
    case DS_ACTION_STATE_COMPLETED:
        return "Completed";
    case DS_ACTION_STATE_BUSY:
        return "Busy";
    case DS_ACTION_STATE_BAD_REQUEST:
        return "BadRequest";
    case DS_ACTION_STATE_TIMEOUT:
        return "Timeout";
    case DS_ACTION_STATE_INTERNAL_ERR:
        return "InternalErr";
    case DS_ACTION_STATE_ALLSTOP_BTN_ABORT:
        return "Aborted";
    case DS_ACTION_STATE_INVALID_STATE:
        return "InvalidState";
    case DS_ACTION_STATE_NOT_IMPLEMENTED:
        return "NotImplemented";
    case DS_ACTION_STATE_USER_ABORT:
        return "UserAbort";
    case DS_ACTION_STATE_UNKNOWN:
        return "Unknown";
    default:
        return QString().asprintf("State%d", from);
    }
}

QString ImrParser::ToImr_UpgradeState(DS_UpgradeDef::UpgradeState from)
{
    switch (from)
    {
    case DS_UpgradeDef::STATE_READY:
        return "Ready";
    case DS_UpgradeDef::STATE_GET_PACKAGE_INFO_STARTED:
        return "Read Package Information - Started";
    case DS_UpgradeDef::STATE_GET_PACKAGE_INFO_PROGRESS:
        return "Read Package Information - Progress";
    case DS_UpgradeDef::STATE_GET_PACKAGE_INFO_DONE:
        return "Read Package Information - Done";
    case DS_UpgradeDef::STATE_GET_PACKAGE_INFO_FAILED:
        return "Read Package Information - Failed";
    case DS_UpgradeDef::STATE_UPGRADE_STARTED:
        return "Upgrade SRU Package - Started";
    case DS_UpgradeDef::STATE_CHECK_PACKAGE_STARTED:
        return "Check Package - Started";
    case DS_UpgradeDef::STATE_CHECK_PACKAGE_DONE:
        return "Check Package - Done";
    case DS_UpgradeDef::STATE_CHECK_PACKAGE_FAILED:
        return "Check Package - Failed";
    case DS_UpgradeDef::STATE_EXTRACT_PACKAGE_STARTED:
        return "Extract Package - Started";
    case DS_UpgradeDef::STATE_EXTRACT_PACKAGE_PROGRESS:
        return "Extract Package - Progress";
    case DS_UpgradeDef::STATE_EXTRACT_PACKAGE_DONE:
        return "Extract Package - Done";
    case DS_UpgradeDef::STATE_EXTRACT_PACKAGE_FAILED:
        return "Extract Package - Failed";
    case DS_UpgradeDef::STATE_INSTALL_MCU_STARTED:
        return "Install MCU Firmware - Started";
    case DS_UpgradeDef::STATE_INSTALL_MCU_PROGRESS:
        return "Install MCU Firmware - Progress";
    case DS_UpgradeDef::STATE_INSTALL_MCU_DONE:
        return "Install MCU Firmware - Done";
    case DS_UpgradeDef::STATE_INSTALL_MCU_FAILED:
        return "Install MCU Firmware - Failed";
    case DS_UpgradeDef::STATE_INSTALL_STOPCOCK_STARTED:
        return "Install Stopcock Firmware - Started";
    case DS_UpgradeDef::STATE_INSTALL_STOPCOCK_PROGRESS:
        return "Install Stopcock Firmware - Progress";
    case DS_UpgradeDef::STATE_INSTALL_STOPCOCK_DONE:
        return "Install Stopcock Firmware - Done";
    case DS_UpgradeDef::STATE_INSTALL_STOPCOCK_FAILED:
        return "Install Stopcock Firmware - Failed";
    case DS_UpgradeDef::STATE_INSTALL_HCU_STARTED:
        return "Install HCU Package - Started";
    case DS_UpgradeDef::STATE_INSTALL_HCU_PROGRESS:
        return "Install HCU Package - Progress";
    case DS_UpgradeDef::STATE_INSTALL_HCU_DONE:
        return "Install HCU Package - Done";
    case DS_UpgradeDef::STATE_INSTALL_HCU_FAILED:
        return "Install HCU Package - Failed";
    case DS_UpgradeDef::STATE_UPGRADE_DONE:
        return "Upgrade SRU Package - Done";
    case DS_UpgradeDef::STATE_UPGRADE_FAILED:
        return "Upgrade SRU Package - Failed";
    default:
        return QString().asprintf("Unknown State%d", from);
    }
}

QString ImrParser::ToImr_WorkFlowState(DS_WorkflowDef::WorkflowState from)
{
    switch (from)
    {
    case DS_WorkflowDef::STATE_INACTIVE:
        return "STATE_INACTIVE";
    case DS_WorkflowDef::STATE_INIT:
        return "STATE_INIT";
    case DS_WorkflowDef::STATE_INIT_STOPCOCK_SETTING:
        return "STATE_INIT_STOPCOCK_SETTING";
    case DS_WorkflowDef::STATE_INIT_STOPCOCK_PROGRESS:
        return "STATE_INIT_STOPCOCK_PROGRESS";
    case DS_WorkflowDef::STATE_INIT_STOPCOCK_DONE:
        return "STATE_INIT_STOPCOCK_DONE";
    case DS_WorkflowDef::STATE_INIT_STOPCOCK_FAILED:
        return "STATE_INIT_STOPCOCK_FAILED";
    case DS_WorkflowDef::STATE_INIT_FAILED:
        return "STATE_INIT_FAILED";
    case DS_WorkflowDef::STATE_MUDS_INSERT_WAITING:
        return "STATE_MUDS_INSERT_WAITING";
    case DS_WorkflowDef::STATE_MUDS_INSERTED:
        return "STATE_MUDS_INSERTED";
    case DS_WorkflowDef::STATE_DOOR_CLOSE_WAITING:
        return "STATE_DOOR_CLOSE_WAITING";
    case DS_WorkflowDef::STATE_DOOR_CLOSED:
        return "STATE_DOOR_CLOSED";
    case DS_WorkflowDef::STATE_SOD_STARTED:
        return "STATE_SOD_STARTED";
    case DS_WorkflowDef::STATE_SOD_STARTED_BY_NEW_FLUID_LOADED:
        return "STATE_SOD_STARTED_BY_NEW_FLUID_LOADED";
    case DS_WorkflowDef::STATE_SOD_PROGRESS:
        return "STATE_SOD_PROGRESS";
    case DS_WorkflowDef::STATE_SOD_FAILED:
        return "STATE_SOD_FAILED";
    case DS_WorkflowDef::STATE_SOD_SUSPENDED:
        return "STATE_SOD_SUSPENDED";
    case DS_WorkflowDef::STATE_SOD_RESUMED:
        return "STATE_SOD_RESUMED";
    case DS_WorkflowDef::STATE_SOD_DONE:
        return "STATE_SOD_DONE";
    case DS_WorkflowDef::STATE_NORMAL_WORKFLOW_READY:
        return "STATE_NORMAL_WORKFLOW_READY";
    case DS_WorkflowDef::STATE_NORMAL_WORKFLOW_SUDS_INSERT_WAITING:
        return "STATE_NORMAL_WORKFLOW_SUDS_INSERT_WAITING";
    case DS_WorkflowDef::STATE_NORMAL_WORKFLOW_AUTO_PRIME_SALINE_WAITING:
        return "STATE_NORMAL_WORKFLOW_AUTO_PRIME_SALINE_WAITING";
    case DS_WorkflowDef::STATE_AUTO_PRIME_STARTED:
        return "STATE_AUTO_PRIME_STARTED";
    case DS_WorkflowDef::STATE_AUTO_PRIME_PROGRESS:
        return "STATE_AUTO_PRIME_PROGRESS";
    case DS_WorkflowDef::STATE_AUTO_PRIME_FAILED:
        return "STATE_AUTO_PRIME_FAILED";
    case DS_WorkflowDef::STATE_AUTO_PRIME_DONE:
        return "STATE_AUTO_PRIME_DONE";
    case DS_WorkflowDef::STATE_AUTO_EMPTY_STARTED:
        return "STATE_AUTO_EMPTY_STARTED";
    case DS_WorkflowDef::STATE_AUTO_EMPTY_PROGRESS:
        return "STATE_AUTO_EMPTY_PROGRESS";
    case DS_WorkflowDef::STATE_AUTO_EMPTY_FAILED:
        return "STATE_AUTO_EMPTY_FAILED";
    case DS_WorkflowDef::STATE_AUTO_EMPTY_DONE:
        return "STATE_AUTO_EMPTY_DONE";
    case DS_WorkflowDef::STATE_FLUID_REMOVAL_STARTED:
        return "STATE_FLUID_REMOVAL_STARTED";
    case DS_WorkflowDef::STATE_FLUID_REMOVAL_PROGRESS:
        return "STATE_FLUID_REMOVAL_PROGRESS";
    case DS_WorkflowDef::STATE_FLUID_REMOVAL_FAILED:
        return "STATE_FLUID_REMOVAL_FAILED";
    case DS_WorkflowDef::STATE_FLUID_REMOVAL_DONE:
        return "STATE_FLUID_REMOVAL_DONE";
    case DS_WorkflowDef::STATE_SYRINGE_AIR_RECOVERY_STARTED:
        return "STATE_SYRINGE_AIR_RECOVERY_STARTED";
    case DS_WorkflowDef::STATE_SYRINGE_AIR_RECOVERY_PROGRESS:
        return "STATE_SYRINGE_AIR_RECOVERY_PROGRESS";
    case DS_WorkflowDef::STATE_SYRINGE_AIR_RECOVERY_FAILED:
        return "STATE_SYRINGE_AIR_RECOVERY_FAILED";
    case DS_WorkflowDef::STATE_SYRINGE_AIR_RECOVERY_DONE:
        return "STATE_SYRINGE_AIR_RECOVERY_DONE";
    case DS_WorkflowDef::STATE_SUDS_AIR_RECOVERY_STARTED:
        return "STATE_SUDS_AIR_RECOVERY_STARTED";
    case DS_WorkflowDef::STATE_SUDS_AIR_RECOVERY_PROGRESS:
        return "STATE_SUDS_AIR_RECOVERY_PROGRESS";
    case DS_WorkflowDef::STATE_SUDS_AIR_RECOVERY_FAILED:
        return "STATE_SUDS_AIR_RECOVERY_FAILED";
    case DS_WorkflowDef::STATE_SUDS_AIR_RECOVERY_DONE:
        return "STATE_SUDS_AIR_RECOVERY_DONE";
    case DS_WorkflowDef::STATE_END_OF_DAY_PURGE_STARTED:
        return "STATE_END_OF_DAY_PURGE_STARTED";
    case DS_WorkflowDef::STATE_END_OF_DAY_PURGE_PROGRESS:
        return "STATE_END_OF_DAY_PURGE_PROGRESS";
    case DS_WorkflowDef::STATE_END_OF_DAY_PURGE_FAILED:
        return "STATE_END_OF_DAY_PURGE_FAILED";
    case DS_WorkflowDef::STATE_END_OF_DAY_PURGE_DONE:
        return "STATE_END_OF_DAY_PURGE_DONE";
    case DS_WorkflowDef::STATE_MUDS_EJECT_STARTED:
        return "STATE_MUDS_EJECT_STARTED";
    case DS_WorkflowDef::STATE_MUDS_EJECT_PROGRESS:
        return "STATE_MUDS_EJECT_PROGRESS";
    case DS_WorkflowDef::STATE_MUDS_EJECT_FAILED:
        return "STATE_MUDS_EJECT_FAILED";
    case DS_WorkflowDef::STATE_MUDS_EJECT_DONE:
        return "STATE_MUDS_EJECT_DONE";
    default:
        return QString().asprintf("STATE_UNKNOWN_%d", from);
    }
}

QString ImrParser::ToImr_WorkflowErrorState(DS_WorkflowDef::WorkflowErrorState from)
{
    switch (from)
    {

    case DS_WorkflowDef::WORKFLOW_ERROR_STATE_NONE:
        return "WORKFLOW_ERROR_STATE_NONE";
    case DS_WorkflowDef::WORKFLOW_ERROR_STATE_SOD_FILL_START_WITH_PATIENT_CONNECTED:
        return "WORKFLOW_ERROR_STATE_SOD_FILL_START_WITH_PATIENT_CONNECTED";
    case DS_WorkflowDef::WORKFLOW_ERROR_STATE_NORMAL_SUDS_REMOVED_AFTER_FIRST_PRIMED:
        return "WORKFLOW_ERROR_STATE_NORMAL_SUDS_REMOVED_AFTER_FIRST_PRIMED";
    case DS_WorkflowDef::WORKFLOW_ERROR_STATE_NORMAL_SUDS_REMOVED_AFTER_FIRST_PRIMED_AND_NEXT_PRIME_FAILED:
        return "WORKFLOW_ERROR_STATE_NORMAL_SUDS_REMOVED_AFTER_FIRST_PRIMED_AND_NEXT_PRIME_FAILED";
    case DS_WorkflowDef::WORKFLOW_ERROR_STATE_NORMAL_SUDS_REMOVED_BEFORE_FIRST_PRIME:
        return "WORKFLOW_ERROR_STATE_NORMAL_SUDS_REMOVED_BEFORE_FIRST_PRIME";
    case DS_WorkflowDef::WORKFLOW_ERROR_STATE_MUDS_EJECT_FAILED:
        return "WORKFLOW_ERROR_STATE_MUDS_EJECT_FAILED";
    case DS_WorkflowDef::WORKFLOW_ERROR_STATE_UNKNOWN:
        return "WORKFLOW_ERROR_STATE_UNKNOWN";
    default:
        return QString().asprintf("STATE_UNKNOWN_%d", from);
    }
}

QString ImrParser::ToImr_SodErrorState(DS_WorkflowDef::SodErrorState from)
{
    switch (from)
    {
    case DS_WorkflowDef::SOD_ERROR_STATE_NONE:
        return "SOD_ERROR_STATE_NONE";
    case DS_WorkflowDef::SOD_ERROR_STATE_SERVICE_ABORT:
        return "SOD_ERROR_STATE_SERVICE_ABORT";
    case DS_WorkflowDef::SOD_ERROR_STATE_USER_ABORT:
        return "SOD_ERROR_STATE_USER_ABORT";
    case DS_WorkflowDef::SOD_ERROR_STATE_FIND_PLUNGERS_USER_ABORT:
        return "SOD_ERROR_STATE_FIND_PLUNGERS_USER_ABORT";
    case DS_WorkflowDef::SOD_ERROR_STATE_FIND_PLUNGERS_FAILED:
        return "SOD_ERROR_STATE_FIND_PLUNGERS_FAILED";
    case DS_WorkflowDef::SOD_ERROR_STATE_USED_MUDS_DETECTED:
        return "SOD_ERROR_STATE_USED_MUDS_DETECTED";
    case DS_WorkflowDef::SOD_ERROR_STATE_PURGE_USER_ABORT:
        return "SOD_ERROR_STATE_PURGE_USER_ABORT";
    case DS_WorkflowDef::SOD_ERROR_STATE_PURGE_FAILED_MUDS_REMOVED:
        return "SOD_ERROR_STATE_PURGE_FAILED_MUDS_REMOVED";
    case DS_WorkflowDef::SOD_ERROR_STATE_PURGE_FAILED_SUDS_INSERTED:
        return "SOD_ERROR_STATE_PURGE_FAILED_SUDS_INSERTED";
    case DS_WorkflowDef::SOD_ERROR_STATE_PURGE_FAILED_STOPCOCK_FAILED:
        return "SOD_ERROR_STATE_PURGE_FAILED_STOPCOCK_FAILED";
    case DS_WorkflowDef::SOD_ERROR_STATE_PURGE_FAILED:
        return "SOD_ERROR_STATE_PURGE_FAILED";
    case DS_WorkflowDef::SOD_ERROR_STATE_REUSE_MUDS_WAITING_SUDS_REMOVAL:
        return "SOD_ERROR_STATE_REUSE_MUDS_WAITING_SUDS_REMOVAL";
    case DS_WorkflowDef::SOD_ERROR_STATE_ENGAGE_USER_ABORT:
        return "SOD_ERROR_STATE_ENGAGE_USER_ABORT";
    case DS_WorkflowDef::SOD_ERROR_STATE_ENGAGE_FAILED:
        return "SOD_ERROR_STATE_ENGAGE_FAILED";
    case DS_WorkflowDef::SOD_ERROR_STATE_MUDS_SOD_START_FAILED:
        return "SOD_ERROR_STATE_MUDS_SOD_START_FAILED";
    case DS_WorkflowDef::SOD_ERROR_STATE_CAL_SLACK_FAILED:
        return "SOD_ERROR_STATE_CAL_SLACK_FAILED";
    case DS_WorkflowDef::SOD_ERROR_STATE_SYRINGE_PRIME_ABORT:
        return "SOD_ERROR_STATE_SYRINGE_PRIME_ABORT";
    case DS_WorkflowDef::SOD_ERROR_STATE_SYRINGE_PRIME_FAILED_SUDS_REMOVED:
        return "SOD_ERROR_STATE_SYRINGE_PRIME_FAILED_SUDS_REMOVED";
    case DS_WorkflowDef::SOD_ERROR_STATE_SYRINGE_PRIME_FAILED_INSUFFICIENT_VOLUME:
        return "SOD_ERROR_STATE_SYRINGE_PRIME_FAILED_INSUFFICIENT_VOLUME";
    case DS_WorkflowDef::SOD_ERROR_STATE_SYRINGE_PRIME_FAILED_HW_FAULT:
        return "SOD_ERROR_STATE_SYRINGE_PRIME_FAILED_HW_FAULT";
    case DS_WorkflowDef::SOD_ERROR_STATE_SYRINGE_PRIME_FAILED_AIR_DETECTED:
        return "SOD_ERROR_STATE_SYRINGE_PRIME_FAILED_AIR_DETECTED";
    case DS_WorkflowDef::SOD_ERROR_STATE_CAL_SYRINGE_AIR_CHECK_FAILED_INSUFFICIENT_VOLUME:
        return "SOD_ERROR_STATE_CAL_SYRINGE_AIR_CHECK_FAILED_INSUFFICIENT_VOLUME";
    case DS_WorkflowDef::SOD_ERROR_STATE_CAL_SYRINGE_AIR_CHECK_FAILED_BAD_DATA:
        return "SOD_ERROR_STATE_CAL_SYRINGE_AIR_CHECK_FAILED_BAD_DATA";
    case DS_WorkflowDef::SOD_ERROR_STATE_CAL_SYRINGE_AIR_CHECK_FAILED_HW_FAULT:
        return "SOD_ERROR_STATE_CAL_SYRINGE_AIR_CHECK_FAILED_HW_FAULT";
    case DS_WorkflowDef::SOD_ERROR_STATE_MUDS_LATCH_LIFTED:
        return "SOD_ERROR_STATE_MUDS_LATCH_LIFTED";
    case DS_WorkflowDef::SOD_ERROR_STATE_MUDS_EJECTED:
        return "SOD_ERROR_STATE_MUDS_EJECTED";
    case DS_WorkflowDef::SOD_ERROR_STATE_UNKNOWN:
    default:
        return QString().asprintf("STATE_UNKNOWN_%d", from);
    }
}

QString ImrParser::ToImr_MudsSodState(DS_WorkflowDef::MudsSodState from)
{
    switch (from)
    {
    case DS_WorkflowDef::MUDS_SOD_STATE_INACTIVE:
        return "MUDS_SOD_STATE_INACTIVE";
    case DS_WorkflowDef::MUDS_SOD_STATE_READY:
        return "MUDS_SOD_STATE_READY";
    case DS_WorkflowDef::MUDS_SOD_STATE_STARTED:
        return "MUDS_SOD_STATE_STARTED";
    case DS_WorkflowDef::MUDS_SOD_STATE_SYRINGES_IDLE_WAITING:
        return "MUDS_SOD_STATE_SYRINGES_IDLE_WAITING";
    case DS_WorkflowDef::MUDS_SOD_STATE_SALINE_VOLUME_WAITING:
        return "MUDS_SOD_STATE_SALINE_VOLUME_WAITING";
    case DS_WorkflowDef::MUDS_SOD_STATE_SUDS_READY_WAITING:
        return "MUDS_SOD_STATE_SUDS_READY_WAITING";
    case DS_WorkflowDef::MUDS_SOD_STATE_OUTLET_AIR_DOOR_CLOSE_WAITING:
        return "MUDS_SOD_STATE_OUTLET_AIR_DOOR_CLOSE_WAITING";
    case DS_WorkflowDef::MUDS_SOD_STATE_WASTE_CONTAINER_WAITING:
        return "MUDS_SOD_STATE_WASTE_CONTAINER_WAITING";
    case DS_WorkflowDef::MUDS_SOD_STATE_SYRINGE_SOD_PREPARING:
        return "MUDS_SOD_STATE_SYRINGE_SOD_PREPARING";
    case DS_WorkflowDef::MUDS_SOD_STATE_SYRINGE_SOD_STARTED:
        return "MUDS_SOD_STATE_SYRINGE_SOD_STARTED";
    case DS_WorkflowDef::MUDS_SOD_STATE_SYRINGE_SOD_PROGRESS:
        return "MUDS_SOD_STATE_SYRINGE_SOD_PROGRESS";
    case DS_WorkflowDef::MUDS_SOD_STATE_SYRINGE_SOD_FAILED:
        return "MUDS_SOD_STATE_SYRINGE_SOD_FAILED";
    case DS_WorkflowDef::MUDS_SOD_STATE_SYRINGE_SOD_DONE:
        return "MUDS_SOD_STATE_SYRINGE_SOD_DONE";
    case DS_WorkflowDef::MUDS_SOD_STATE_ABORT:
        return "MUDS_SOD_STATE_ABORT";
    case DS_WorkflowDef::MUDS_SOD_STATE_DONE:
        return "MUDS_SOD_STATE_DONE";
    default:
        return QString().asprintf("STATE_UNKNOWN_%d", from);
    }
}

QString ImrParser::ToImr_SudsAirRecoveryState(DS_WorkflowDef::SudsAirRecoveryState from)
{
    switch (from)
    {
    case DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_INACTIVE:
        return "SUDS_AIR_RECOVERY_STATE_INACTIVE";
    case DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_READY:
        return "SUDS_AIR_RECOVERY_STATE_READY";
    case DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_STARTED:
        return "SUDS_AIR_RECOVERY_STATE_STARTED";
    case DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_WAIT_INJECTION_COMPLETE:
        return "SUDS_AIR_RECOVERY_STATE_WAIT_INJECTION_COMPLETE";
    case DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_USER_START_WAITING:
        return "SUDS_AIR_RECOVERY_STATE_USER_START_WAITING";
    case DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_USER_START_CONFIRMED:
        return "SUDS_AIR_RECOVERY_STATE_USER_START_CONFIRMED";
    case DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_SYRINGE_PRIME_STARTED:
        return "SUDS_AIR_RECOVERY_STATE_SYRINGE_PRIME_STARTED";
    case DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_SYRINGE_PRIME_PROGRESS:
        return "SUDS_AIR_RECOVERY_STATE_SYRINGE_PRIME_PROGRESS";
    case DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_SYRINGE_PRIME_FAILED:
        return "SUDS_AIR_RECOVERY_STATE_SYRINGE_PRIME_FAILED";
    case DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_SYRINGE_PRIME_DONE:
        return "SUDS_AIR_RECOVERY_STATE_SYRINGE_PRIME_DONE";
    case DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_OUTLET_AIR_CHECK_STARTED:
        return "SUDS_AIR_RECOVERY_STATE_OUTLET_AIR_CHECK_STARTED";
    case DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_OUTLET_AIR_CHECK_FAILED:
        return "SUDS_AIR_RECOVERY_STATE_OUTLET_AIR_CHECK_FAILED";
    case DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_OUTLET_AIR_CHECK_DONE:
        return "SUDS_AIR_RECOVERY_STATE_OUTLET_AIR_CHECK_DONE";
    case DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_SYRINGE_AIR_CHECK_STARTED:
        return "SUDS_AIR_RECOVERY_STATE_SYRINGE_AIR_CHECK_STARTED";
    case DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_SYRINGE_AIR_CHECK_PROGRESS:
        return "SUDS_AIR_RECOVERY_STATE_SYRINGE_AIR_CHECK_PROGRESS";
    case DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_SYRINGE_AIR_CHECK_FAILED:
        return "SUDS_AIR_RECOVERY_STATE_SYRINGE_AIR_CHECK_FAILED";
    case DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_SYRINGE_AIR_CHECK_DONE:
        return "SUDS_AIR_RECOVERY_STATE_SYRINGE_AIR_CHECK_DONE";
    case DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_DONE_WITH_AUTO_PRIME_OPERATION_USER_WAITING:
        return "SUDS_AIR_RECOVERY_STATE_DONE_WITH_AUTO_PRIME_OPERATION_USER_WAITING";
    case DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_DONE_WITH_AUTO_PRIME_OPERATION_USER_CONFIRMED:
        return "SUDS_AIR_RECOVERY_STATE_DONE_WITH_AUTO_PRIME_OPERATION_USER_CONFIRMED";
    case DS_WorkflowDef::SUDS_AIR_RECOVERY_STATE_DONE:
        return "SUDS_AIR_RECOVERY_STATE_DONE";
    default:
        return QString().asprintf("STATE_UNKNOWN_%d", from);
    }
}

QString ImrParser::ToImr_FluidRemovalState(DS_WorkflowDef::FluidRemovalState from)
{
    switch (from)
    {
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_INACTIVE:
        return "FLUID_REMOVAL_STATE_INACTIVE";
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_READY:
        return "FLUID_REMOVAL_STATE_READY";
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_STARTED:
        return "FLUID_REMOVAL_STATE_STARTED";
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_WAIT_USER_START:
        return "FLUID_REMOVAL_STATE_WAIT_USER_START";
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_WAIT_REMOVE_CURRENT_SUPPLY:
        return "FLUID_REMOVAL_STATE_WAIT_REMOVE_CURRENT_SUPPLY";
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_SUDS_INSERT_WAITING:
        return "FLUID_REMOVAL_STATE_SUDS_INSERT_WAITING";
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_WAIT_SUDS_TO_WASTE_CONTAINER:
        return "FLUID_REMOVAL_STATE_WAIT_SUDS_TO_WASTE_CONTAINER";
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_PURGE_STARTED:
        return "FLUID_REMOVAL_STATE_PURGE_STARTED";
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_PURGE_PROGRESS:
        return "FLUID_REMOVAL_STATE_PURGE_PROGRESS";
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_PURGE_DONE:
        return "FLUID_REMOVAL_STATE_PURGE_DONE";
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_PURGE_FAILED:
        return "FLUID_REMOVAL_STATE_PURGE_FAILED";
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_FILL_STOPCOCK_FILL_SETTING:
        return "FLUID_REMOVAL_STATE_FILL_STOPCOCK_FILL_SETTING";
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_FILL_STOPCOCK_FILL_PROGRESS:
        return "FLUID_REMOVAL_STATE_FILL_STOPCOCK_FILL_PROGRESS";
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_FILL_STOPCOCK_FILL_DONE:
        return "FLUID_REMOVAL_STATE_FILL_STOPCOCK_FILL_DONE";
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_FILL_STOPCOCK_FILL_FAILED:
        return "FLUID_REMOVAL_STATE_FILL_STOPCOCK_FILL_FAILED";
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_FILL_STARTED:
        return "FLUID_REMOVAL_STATE_FILL_STARTED";
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_FILL_PROGRESS:
        return "FLUID_REMOVAL_STATE_FILL_PROGRESS";
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_FILL_DONE:
        return "FLUID_REMOVAL_STATE_FILL_DONE";
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_FILL_FAILED:
        return "FLUID_REMOVAL_STATE_FILL_FAILED";
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_CHECK_SPIKE_STATE_STARTED:
        return "FLUID_REMOVAL_STATE_CHECK_SPIKE_STATE_STARTED";
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_CHECK_SPIKE_STATE_DONE:
        return "FLUID_REMOVAL_STATE_CHECK_SPIKE_STATE_DONE";
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_CHECK_SPIKE_STATE_FAILED:
        return "FLUID_REMOVAL_STATE_CHECK_SPIKE_STATE_FAILED";
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_FINAL_PURGE_STARTED:
        return "FLUID_REMOVAL_STATE_FINAL_PURGE_STARTED";
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_FINAL_PURGE_PROGRESS:
        return "FLUID_REMOVAL_STATE_FINAL_PURGE_PROGRESS";
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_FINAL_PURGE_DONE:
        return "FLUID_REMOVAL_STATE_FINAL_PURGE_DONE";
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_FINAL_PURGE_FAILED:
        return "FLUID_REMOVAL_STATE_FINAL_PURGE_FAILED";
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_REMOVING_TRAPPED_FLUID_ABORTED:
        return "FLUID_REMOVAL_STATE_REMOVING_TRAPPED_FLUID_ABORTED";
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_REMOVING_TRAPPED_FLUID_ABORTED_BY_USER_STOP:
        return "FLUID_REMOVAL_STATE_REMOVING_TRAPPED_FLUID_ABORTED_BY_USER_STOP";
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_REMOVING_TRAPPED_FLUID_ABORTED_BY_SUPPLY_STILL_LOADED:
        return "FLUID_REMOVAL_STATE_REMOVING_TRAPPED_FLUID_ABORTED_BY_SUPPLY_STILL_LOADED";
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_REMOVING_TRAPPED_FLUID_ABORTED_BY_HW_FAILED:
        return "FLUID_REMOVAL_STATE_REMOVING_TRAPPED_FLUID_ABORTED_BY_HW_FAILED";
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_FAILED_INVALID_STATE:
        return "FLUID_REMOVAL_STATE_FAILED_INVALID_STATE";
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_FAILED_OTHER_SYRINGE_BUSY:
        return "FLUID_REMOVAL_STATE_FAILED_OTHER_SYRINGE_BUSY";
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_FAILED_SUDS_REMOVED:
        return "FLUID_REMOVAL_STATE_FAILED_SUDS_REMOVED";
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_FAILED_WASTE_CONTAINER_NOT_READY:
        return "FLUID_REMOVAL_STATE_FAILED_WASTE_CONTAINER_NOT_READY";
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_FAILED_USER_ABORT:
        return "FLUID_REMOVAL_STATE_FAILED_USER_ABORT";
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_FAILED_HW_FAILED:
        return "FLUID_REMOVAL_STATE_FAILED_HW_FAILED";
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_FAILED:
        return "FLUID_REMOVAL_STATE_FAILED";
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_FAILED_DURING_REMOVING_TRAPPED_FLUID:
        return "FLUID_REMOVAL_STATE_FAILED_DURING_REMOVING_TRAPPED_FLUID";
    case DS_WorkflowDef::FLUID_REMOVAL_STATE_DONE:
        return "FLUID_REMOVAL_STATE_DONE";
    default:
        return QString().asprintf("STATE_UNKNOWN_%d", from);
    }
}

QString ImrParser::ToImr_EndOfDayPurgeState(DS_WorkflowDef::EndOfDayPurgeState from)
{
    switch (from)
    {
    case DS_WorkflowDef::END_OF_DAY_PURGE_STATE_INACTIVE:
        return "END_OF_DAY_PURGE_STATE_INACTIVE";
    case DS_WorkflowDef::END_OF_DAY_PURGE_STATE_READY:
        return "END_OF_DAY_PURGE_STATE_READY";
    case DS_WorkflowDef::END_OF_DAY_PURGE_STATE_STARTED:
        return "END_OF_DAY_PURGE_STATE_STARTED";
    case DS_WorkflowDef::END_OF_DAY_PURGE_STATE_WAIT_USER_START:
        return "END_OF_DAY_PURGE_STATE_WAIT_USER_START";
    case DS_WorkflowDef::END_OF_DAY_PURGE_STATE_WAIT_INSERT_SUDS:
        return "END_OF_DAY_PURGE_STATE_WAIT_INSERT_SUDS";
    case DS_WorkflowDef::END_OF_DAY_PURGE_STATE_WAIT_SUDS_TO_WASTE_CONTAINER:
        return "END_OF_DAY_PURGE_STATE_WAIT_SUDS_TO_WASTE_CONTAINER";
    case DS_WorkflowDef::END_OF_DAY_PURGE_STATE_STOPCOCK_INJECT_SETTING:
        return "END_OF_DAY_PURGE_STATE_STOPCOCK_INJECT_SETTING";
    case DS_WorkflowDef::END_OF_DAY_PURGE_STATE_STOPCOCK_INJECT_PROGRESS:
        return "END_OF_DAY_PURGE_STATE_STOPCOCK_INJECT_PROGRESS";
    case DS_WorkflowDef::END_OF_DAY_PURGE_STATE_STOPCOCK_INJECT_DONE:
        return "END_OF_DAY_PURGE_STATE_STOPCOCK_INJECT_DONE";
    case DS_WorkflowDef::END_OF_DAY_PURGE_STATE_PUSH_PLUNGERS_STARTED:
        return "END_OF_DAY_PURGE_STATE_PUSH_PLUNGERS_STARTED";
    case DS_WorkflowDef::END_OF_DAY_PURGE_STATE_PUSH_PLUNGERS_PROGRESS:
        return "END_OF_DAY_PURGE_STATE_PUSH_PLUNGERS_PROGRESS";
    case DS_WorkflowDef::END_OF_DAY_PURGE_STATE_PUSH_PLUNGERS_DONE:
        return "END_OF_DAY_PURGE_STATE_PUSH_PLUNGERS_DONE";
    case DS_WorkflowDef::END_OF_DAY_PURGE_STATE_PUSH_PLUNGERS_FAILED:
        return "END_OF_DAY_PURGE_STATE_PUSH_PLUNGERS_FAILED";
    case DS_WorkflowDef::END_OF_DAY_PURGE_STATE_CHECK_SALINE_LEVEL:
        return "END_OF_DAY_PURGE_STATE_CHECK_SALINE_LEVEL";
    case DS_WorkflowDef::END_OF_DAY_PURGE_STATE_WAIT_USER_CONFIRM_EMPTY_SALINE:
        return "END_OF_DAY_PURGE_STATE_WAIT_USER_CONFIRM_EMPTY_SALINE";
    case DS_WorkflowDef::END_OF_DAY_PURGE_STATE_FAILED_SUDS_REMOVED:
        return "END_OF_DAY_PURGE_STATE_FAILED_SUDS_REMOVED";
    case DS_WorkflowDef::END_OF_DAY_PURGE_STATE_FAILED_WASTE_CONTAINER_NOT_READY:
        return "END_OF_DAY_PURGE_STATE_FAILED_WASTE_CONTAINER_NOT_READY";
    case DS_WorkflowDef::END_OF_DAY_PURGE_STATE_FAILED_USER_ABORT:
        return "END_OF_DAY_PURGE_STATE_FAILED_USER_ABORT";
    case DS_WorkflowDef::END_OF_DAY_PURGE_STATE_FAILED:
        return "END_OF_DAY_PURGE_STATE_FAILED";
    case DS_WorkflowDef::END_OF_DAY_PURGE_STATE_DONE:
        return "END_OF_DAY_PURGE_STATE_DONE";
    default:
        return QString().asprintf("STATE_UNKNOWN_%d", from);
    }
}

QString ImrParser::ToImr_AutoEmptyState(DS_WorkflowDef::AutoEmptyState from)
{
    switch (from)
    {
    case DS_WorkflowDef::AUTO_EMPTY_STATE_INACTIVE:
        return "AUTO_EMPTY_STATE_INACTIVE";
    case DS_WorkflowDef::AUTO_EMPTY_STATE_READY:
        return "AUTO_EMPTY_STATE_READY";
    case DS_WorkflowDef::AUTO_EMPTY_STATE_STARTED:
        return "AUTO_EMPTY_STATE_STARTED";
    case DS_WorkflowDef::AUTO_EMPTY_STATE_FLUID_REMOVAL_STARTED:
        return "AUTO_EMPTY_STATE_FLUID_REMOVAL_STARTED";
    case DS_WorkflowDef::AUTO_EMPTY_STATE_FLUID_REMOVAL_PROGRESS:
        return "AUTO_EMPTY_STATE_FLUID_REMOVAL_PROGRESS";
    case DS_WorkflowDef::AUTO_EMPTY_STATE_FLUID_REMOVAL_DONE:
        return "AUTO_EMPTY_STATE_FLUID_REMOVAL_DONE";
    case DS_WorkflowDef::AUTO_EMPTY_STATE_FLUID_REMOVAL_FAILED:
        return "AUTO_EMPTY_STATE_FLUID_REMOVAL_FAILED";
    case DS_WorkflowDef::AUTO_EMPTY_STATE_FAILED:
        return "AUTO_EMPTY_STATE_FAILED";
    case DS_WorkflowDef::AUTO_EMPTY_STATE_DONE:
        return "AUTO_EMPTY_STATE_DONE";
    default:
        return QString().asprintf("STATE_UNKNOWN_%d", from);
    }
}

QString ImrParser::ToImr_MudsEjectState(DS_WorkflowDef::MudsEjectState from)
{
    switch (from)
    {
    case DS_WorkflowDef::MUDS_EJECT_STATE_INACTIVE:
        return "MUDS_EJECT_STATE_INACTIVE";
    case DS_WorkflowDef::MUDS_EJECT_STATE_READY:
        return "MUDS_EJECT_STATE_READY";
    case DS_WorkflowDef::MUDS_EJECT_STATE_STARTED:
        return "MUDS_EJECT_STATE_STARTED";
    case DS_WorkflowDef::MUDS_EJECT_STATE_WAIT_STATE_PATH_IDLE_WAITING:
        return "MUDS_EJECT_STATE_WAIT_STATE_PATH_IDLE_WAITING";
    case DS_WorkflowDef::MUDS_EJECT_STATE_SYRINGES_IDLE_WAITING:
        return "MUDS_EJECT_STATE_SYRINGES_IDLE_WAITING";
    case DS_WorkflowDef::MUDS_EJECT_STATE_PULL_PISTONS_STARTED:
        return "MUDS_EJECT_STATE_PULL_PISTONS_STARTED";
    case DS_WorkflowDef::MUDS_EJECT_STATE_PULL_PISTONS_PROGRESS:
        return "MUDS_EJECT_STATE_PULL_PISTONS_PROGRESS";
    case DS_WorkflowDef::MUDS_EJECT_STATE_PULL_PISTONS_FAILED:
        return "MUDS_EJECT_STATE_PULL_PISTONS_FAILED";
    case DS_WorkflowDef::MUDS_EJECT_STATE_PULL_PISTONS_DONE:
        return "MUDS_EJECT_STATE_PULL_PISTONS_DONE";
    case DS_WorkflowDef::MUDS_EJECT_STATE_STOPCOCK_FILL_SETTING:
        return "MUDS_EJECT_STATE_STOPCOCK_FILL_SETTING";
    case DS_WorkflowDef::MUDS_EJECT_STATE_STOPCOCK_FILL_PROGRESS:
        return "MUDS_EJECT_STATE_STOPCOCK_FILL_PROGRESS";
    case DS_WorkflowDef::MUDS_EJECT_STATE_STOPCOCK_FILL_DONE:
        return "MUDS_EJECT_STATE_STOPCOCK_FILL_DONE";
    case DS_WorkflowDef::MUDS_EJECT_STATE_STOPCOCK_FILL_FAILED:
        return "MUDS_EJECT_STATE_STOPCOCK_FILL_FAILED";
    case DS_WorkflowDef::MUDS_EJECT_STATE_DISENGAGE_STARTED:
        return "MUDS_EJECT_STATE_DISENGAGE_STARTED";
    case DS_WorkflowDef::MUDS_EJECT_STATE_DISENGAGE_PROGRESS:
        return "MUDS_EJECT_STATE_DISENGAGE_PROGRESS";
    case DS_WorkflowDef::MUDS_EJECT_STATE_DISENGAGE_DONE:
        return "MUDS_EJECT_STATE_DISENGAGE_DONE";
    case DS_WorkflowDef::MUDS_EJECT_STATE_DISENGAGE_FAILED:
        return "MUDS_EJECT_STATE_DISENGAGE_FAILED";
    case DS_WorkflowDef::MUDS_EJECT_STATE_UNLATCH_WAITING:
        return "MUDS_EJECT_STATE_UNLATCH_WAITING";
    case DS_WorkflowDef::MUDS_EJECT_STATE_UNLATCH_DONE:
        return "MUDS_EJECT_STATE_UNLATCH_DONE";
    case DS_WorkflowDef::MUDS_EJECT_STATE_REMOVAL_WAITING:
        return "MUDS_EJECT_STATE_REMOVAL_WAITING";
    case DS_WorkflowDef::MUDS_EJECT_STATE_REMOVAL_DONE:
        return "MUDS_EJECT_STATE_REMOVAL_DONE";
    case DS_WorkflowDef::MUDS_EJECT_STATE_FAILED_USER_ABORT:
        return "MUDS_EJECT_STATE_FAILED_USER_ABORT";
    case DS_WorkflowDef::MUDS_EJECT_STATE_FAILED_SYSTEM_ERROR:
        return "MUDS_EJECT_STATE_FAILED_SYSTEM_ERROR";
    case DS_WorkflowDef::MUDS_EJECT_STATE_FAILED:
        return "MUDS_EJECT_STATE_FAILED";
    case DS_WorkflowDef::MUDS_EJECT_STATE_DONE:
        return "MUDS_EJECT_STATE_DONE";
    default:
        return QString().asprintf("STATE_UNKNOWN_%d", from);
    }
}

QString ImrParser::ToImr_SyringeAirRecoveryState(DS_WorkflowDef::SyringeAirRecoveryState from)
{
    switch (from)
    {
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_INACTIVE:
        return "SYRINGE_AIR_RECOVERY_STATE_INACTIVE";
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_READY:
        return "SYRINGE_AIR_RECOVERY_STATE_READY";
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_STARTED:
        return "SYRINGE_AIR_RECOVERY_STATE_STARTED";
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_MAX_AIR_DETECTED:
        return "SYRINGE_AIR_RECOVERY_STATE_MAX_AIR_DETECTED";
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_USER_START_WAITING_1:
        return "SYRINGE_AIR_RECOVERY_STATE_USER_START_WAITING_1";
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_VOLUME_CHECKING:
        return "SYRINGE_AIR_RECOVERY_STATE_VOLUME_CHECKING";
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_USER_START_WAITING_2:
        return "SYRINGE_AIR_RECOVERY_STATE_USER_START_WAITING_2";
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_SYRINGES_IDLE_WAITING:
        return "SYRINGE_AIR_RECOVERY_STATE_SYRINGES_IDLE_WAITING";
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_SUDS_INSERT_WAITING:
        return "SYRINGE_AIR_RECOVERY_STATE_SUDS_INSERT_WAITING";
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_VACUUM_CYCLE_STARTED:
        return "SYRINGE_AIR_RECOVERY_STATE_VACUUM_CYCLE_STARTED";
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_VACUUM_CYCLE_PROGRESS:
        return "SYRINGE_AIR_RECOVERY_STATE_VACUUM_CYCLE_PROGRESS";
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_VACUUM_CYCLE_FAILED:
        return "SYRINGE_AIR_RECOVERY_STATE_VACUUM_CYCLE_FAILED";
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_VACUUM_CYCLE_DONE:
        return "SYRINGE_AIR_RECOVERY_STATE_VACUUM_CYCLE_DONE";
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_PRIME_STARTED:
        return "SYRINGE_AIR_RECOVERY_STATE_PRIME_STARTED";
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_PRIME_PROGRESS:
        return "SYRINGE_AIR_RECOVERY_STATE_PRIME_PROGRESS";
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_PRIME_FAILED:
        return "SYRINGE_AIR_RECOVERY_STATE_PRIME_FAILED";
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_PRIME_DONE:
        return "SYRINGE_AIR_RECOVERY_STATE_PRIME_DONE";
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_OUTLET_AIR_CHECK_STARTED:
        return "SYRINGE_AIR_RECOVERY_STATE_OUTLET_AIR_CHECK_STARTED";
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_OUTLET_AIR_CHECK_FAILED:
        return "SYRINGE_AIR_RECOVERY_STATE_OUTLET_AIR_CHECK_FAILED";
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_OUTLET_AIR_CHECK_DONE:
        return "SYRINGE_AIR_RECOVERY_STATE_OUTLET_AIR_CHECK_DONE";
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_EXTRA_PRIME_STARTED:
        return "SYRINGE_AIR_RECOVERY_STATE_EXTRA_PRIME_STARTED";
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_EXTRA_PRIME_PROGRESS:
        return "SYRINGE_AIR_RECOVERY_STATE_EXTRA_PRIME_PROGRESS";
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_EXTRA_PRIME_FAILED:
        return "SYRINGE_AIR_RECOVERY_STATE_EXTRA_PRIME_FAILED";
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_EXTRA_PRIME_DONE:
        return "SYRINGE_AIR_RECOVERY_STATE_EXTRA_PRIME_DONE";
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_FAILED_SUDS_REMOVED:
        return "SYRINGE_AIR_RECOVERY_STATE_FAILED_SUDS_REMOVED";
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_FAILED_USER_ABORT:
        return "SYRINGE_AIR_RECOVERY_STATE_FAILED_USER_ABORT";
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_FAILED_INSUFFICIENT_VOLUME:
        return "SYRINGE_AIR_RECOVERY_STATE_FAILED_INSUFFICIENT_VOLUME";
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_FAILED_HW_FAULT:
        return "SYRINGE_AIR_RECOVERY_STATE_FAILED_HW_FAULT";
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_FAILED_OPERATION_FAILED:
        return "SYRINGE_AIR_RECOVERY_STATE_FAILED_OPERATION_FAILED";
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_USER_END_WAITING:
        return "SYRINGE_AIR_RECOVERY_STATE_USER_END_WAITING";
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_SUSPENDED:
        return "SYRINGE_AIR_RECOVERY_STATE_SUSPENDED";
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_FAILED:
        return "SYRINGE_AIR_RECOVERY_STATE_FAILED";
    case DS_WorkflowDef::SYRINGE_AIR_RECOVERY_STATE_DONE:
        return "SYRINGE_AIR_RECOVERY_STATE_DONE";
    default:
        return QString().asprintf("STATE_UNKNOWN_%d", from);
    }
}

QString ImrParser::ToImr_WorkflowBatteryState(DS_WorkflowDef::WorkflowBatteryState from)
{
    switch (from)
    {
    case DS_WorkflowDef::WORKFLOW_BATTERY_STATE_INACTIVE:
        return "WORKFLOW_BATTERY_STATE_INACTIVE";
    case DS_WorkflowDef::WORKFLOW_BATTERY_STATE_IDLE:
        return "WORKFLOW_BATTERY_STATE_IDLE";

    case DS_WorkflowDef::WORKFLOW_BATTERY_STATE_DISCHARGE_PREPARATION:
        return "WORKFLOW_BATTERY_STATE_DISCHARGE_PREPARATION";
    case DS_WorkflowDef::WORKFLOW_BATTERY_STATE_DISCHARGE_PROGRESS:
        return "WORKFLOW_BATTERY_STATE_DISCHARGE_PROGRESS";
    case DS_WorkflowDef::WORKFLOW_BATTERY_STATE_DISCHARGE_DONE:
        return "WORKFLOW_BATTERY_STATE_DISCHARGE_DONE";
    case DS_WorkflowDef::WORKFLOW_BATTERY_STATE_DISCHARGE_ABORT:
        return "WORKFLOW_BATTERY_STATE_DISCHARGE_ABORT";
    case DS_WorkflowDef::WORKFLOW_BATTERY_STATE_DISCHARGE_FAIL:
        return "WORKFLOW_BATTERY_STATE_DISCHARGE_FAIL";

    case DS_WorkflowDef::WORKFLOW_BATTERY_STATE_CHARGE_PREPARATION:
        return "WORKFLOW_BATTERY_STATE_CHARGE_PREPARATION";
    case DS_WorkflowDef::WORKFLOW_BATTERY_STATE_CHARGE_PROGRESS:
        return "WORKFLOW_BATTERY_STATE_CHARGE_PROGRESS";
    case DS_WorkflowDef::WORKFLOW_BATTERY_STATE_CHARGE_DONE:
        return "WORKFLOW_BATTERY_STATE_CHARGE_DONE";
    case DS_WorkflowDef::WORKFLOW_BATTERY_STATE_CHARGE_ABORT:
        return "WORKFLOW_BATTERY_STATE_CHARGE_ABORT";
    case DS_WorkflowDef::WORKFLOW_BATTERY_STATE_CHARGE_FAIL:
        return "WORKFLOW_BATTERY_STATE_CHARGE_FAIL";

    case DS_WorkflowDef::WORKFLOW_BATTERY_STATE_ABORT:
        return "WORKFLOW_BATTERY_STATE_ABORT";
    case DS_WorkflowDef::WORKFLOW_BATTERY_STATE_FAIL:
        return "WORKFLOW_BATTERY_STATE_FAIL";
    case DS_WorkflowDef::WORKFLOW_BATTERY_STATE_DONE:
        return "WORKFLOW_BATTERY_STATE_DONE";

        // basebaord test states
    case DS_WorkflowDef::WORKFLOW_BATTERY_STATE_BASEBOARD_VOUTTEST_ON:
        return "WORKFLOW_BATTERY_STATE_BASEBOARD_VOUTTEST_ON";
    case DS_WorkflowDef::WORKFLOW_BATTERY_STATE_BASEBOARD_VOUTTEST_OFF:
        return "WORKFLOW_BATTERY_STATE_BASEBOARD_VOUTTEST_OFF";
    default:
        return QString().asprintf("WORKFLOW_BATTERY_STATE_UNKNOWN_%d", from);
    }
}

QString ImrParser::ToImr_ManualQualifiedDischargeState(DS_WorkflowDef::ManualQualifiedDischargeState from)
{
    switch (from)
    {
    case DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_INACTIVE:
        return "MANUAL_QUALIFIED_DISCHARGE_STATE_INACTIVE";
    case DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_READY:
        return "MANUAL_QUALIFIED_DISCHARGE_STATE_READY";
    case DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_PREPARATION:
        return "MANUAL_QUALIFIED_DISCHARGE_STATE_PREPARATION";

    case DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_USER_POWER_CONNECT_WAITING:
        return "MANUAL_QUALIFIED_DISCHARGE_STATE_USER_POWER_CONNECT_WAITING";
    case DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_USER_POWER_CONNECT_CONFIRMED:
        return "MANUAL_QUALIFIED_DISCHARGE_STATE_USER_POWER_CONNECT_CONFIRMED";

    case DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_START:
        return "MANUAL_QUALIFIED_DISCHARGE_STATE_START";

    case DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_CHARGING_FULL_PROGRESS:
        return "MANUAL_QUALIFIED_DISCHARGE_STATE_CHARGING_FULL_PROGRESS";
    case DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_CHARGING_FULL_DONE:
        return "MANUAL_QUALIFIED_DISCHARGE_STATE_CHARGING_FULL_DONE";
    case DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_CHARGING_FULL_FAIL:
        return "MANUAL_QUALIFIED_DISCHARGE_STATE_CHARGING_FULL_FAIL";

    case DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_DISCHARGING_START:
        return "MANUAL_QUALIFIED_DISCHARGE_STATE_DISCHARGING_START";
    case DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_DISCHARGING_PROGRESS:
        return "MANUAL_QUALIFIED_DISCHARGE_STATE_DISCHARGING_PROGRESS";
    case DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_DISCHARGING_DONE:
        return "MANUAL_QUALIFIED_DISCHARGE_STATE_DISCHARGING_DONE";
    case DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_DISCHARGING_FAIL:
        return "MANUAL_QUALIFIED_DISCHARGE_STATE_DISCHARGING_FAIL";

    case DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_CONNECT_EXTERNAL_LOAD:
        return "MANUAL_QUALIFIED_DISCHARGE_STATE_CONNECT_EXTERNAL_LOAD";
    case DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_LOAD_DISCHARGING_PROGRESS:
        return "MANUAL_QUALIFIED_DISCHARGE_STATE_LOAD_DISCHARGING_PROGRESS";
    case DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_LOAD_DISCHARGING_DONE:
        return "MANUAL_QUALIFIED_DISCHARGE_STATE_LOAD_DISCHARGING_DONE";
    case DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_LOAD_DISCHARGING_FAIL:
        return "MANUAL_QUALIFIED_DISCHARGE_STATE_LOAD_DISCHARGING_FAIL";

    case DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_RECONNECT_BATTERY:
        return "MANUAL_QUALIFIED_DISCHARGE_STATE_RECONNECT_BATTERY";
    case DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_RECONNECT_BATTERY_CONFIRMED:
        return "MANUAL_QUALIFIED_DISCHARGE_STATE_RECONNECT_BATTERY_CONFIRMED";

    case DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_FAILED:
        return "MANUAL_QUALIFIED_DISCHARGE_STATE_FAILED";
    case DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_ABORTED:
        return "MANUAL_QUALIFIED_DISCHARGE_STATE_ABORTED";
    case DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_STATE_DONE:
        return "MANUAL_QUALIFIED_DISCHARGE_STATE_DONE";

    default:
        return QString().asprintf("MANUAL_QUALIFIED_DISCHARGE_STATE_UNKNOWN_%d", from);
    }
}

QString ImrParser::ToImr_ShippingModeState(DS_WorkflowDef::ShippingModeState from)
{
    switch (from)
    {
    case DS_WorkflowDef::SHIPPING_MODE_STATE_INACTIVE:
        return "SHIPPING_MODE_STATE_INACTIVE";
    case DS_WorkflowDef::SHIPPING_MODE_STATE_READY:
        return "SHIPPING_MODE_STATE_READY";

    case DS_WorkflowDef::SHIPPING_MODE_STATE_PREPARING:
        return "SHIPPING_MODE_STATE_PREPARING";
    case DS_WorkflowDef::SHIPPING_MODE_STATE_USER_POWER_CONNECT_WAITING:
        return "SHIPPING_MODE_STATE_USER_POWER_CONNECT_WAITING";
    case DS_WorkflowDef::SHIPPING_MODE_STATE_USER_POWER_CONNECT_CONFIRMED:
        return "SHIPPING_MODE_STATE_USER_POWER_CONNECT_CONFIRMED";

    case DS_WorkflowDef::SHIPPING_MODE_STATE_CHARGING_DISCHARGING_PROGRESS:
        return "SHIPPING_MODE_STATE_CHARGING_DISCHARGING_PROGRESS";
    case DS_WorkflowDef::SHIPPING_MODE_STATE_CHARGING_DISCHARGING_DONE:
        return "SHIPPING_MODE_STATE_CHARGING_DISCHARGING_DONE";
    case DS_WorkflowDef::SHIPPING_MODE_STATE_CHARGING_DISCHARGING_FAIL:
        return "SHIPPING_MODE_STATE_CHARGING_DISCHARGING_FAIL";


    case DS_WorkflowDef::SHIPPING_MODE_STATE_SET_SLEEP_MODE:
        return "SHIPPING_MODE_STATE_SET_SLEEP_MODE";
    case DS_WorkflowDef::SHIPPING_MODE_STATE_USER_REMOVE_BATTERY_WAITING:
        return "SHIPPING_MODE_STATE_USER_REMOVE_BATTERY_WAITING";
    case DS_WorkflowDef::SHIPPING_MODE_STATE_USER_REMOVE_BATTERY_DONE:
        return "SHIPPING_MODE_STATE_USER_REMOVE_BATTERY_DONE";
    case DS_WorkflowDef::SHIPPING_MODE_STATE_FAILED:
        return "SHIPPING_MODE_STATE_FAILED";
    case DS_WorkflowDef::SHIPPING_MODE_STATE_ABORTED:
        return "SHIPPING_MODE_STATE_ABORTED";
    case DS_WorkflowDef::SHIPPING_MODE_STATE_DONE:
        return "SHIPPING_MODE_STATE_DONE";
    default:
        return QString().asprintf("SHIPPING_MODE_STATE_UNKNOWN_%d", from);
    }
}

QString ImrParser::ToImr_AutomaticQualifiedDischargeState(DS_WorkflowDef::AutomaticQualifiedDischargeState from)
{
    switch (from)
    {
    case DS_WorkflowDef::AQD_STATE_INACTIVE:
        return "AQD_STATE_INACTIVE";
    case DS_WorkflowDef::AQD_STATE_IDLE:
        return "AQD_STATE_IDLE";
    case DS_WorkflowDef::AQD_STATE_AQD_QUEUED:
        return "AQD_STATE_AQD_QUEUED";
    case DS_WorkflowDef::AQD_STATE_START:
        return "AQD_STATE_START";
    case DS_WorkflowDef::AQD_STATE_DISCHARGE_PROGRESS:
        return "AQD_STATE_DISCHARGE_PROGRESS";
    case DS_WorkflowDef::AQD_STATE_DISCHARGE_DONE:
        return "AQD_STATE_DISCHARGE_DONE";
    case DS_WorkflowDef::AQD_STATE_DISCHARGE_FAIL:
        return "AQD_STATE_DISCHARGE_FAIL";
    case DS_WorkflowDef::AQD_STATE_DONE:
        return "AQD_STATE_DONE";
    default:
        return QString().asprintf("AQD_STATE_UNKNOWN_%d", from);
    }
}

QString ImrParser::ToImr_PreloadProtocolState(DS_WorkflowDef::PreloadProtocolState from)
{
    switch (from)
    {
    case DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_INACTIVE:
        return "PRELOAD_PROTOCOL_STATE_INACTIVE";
    case DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_READY:
        return "PRELOAD_PROTOCOL_STATE_READY";
    case DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_STARTED:
        return "PRELOAD_PROTOCOL_STATE_STARTED";
    case DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_USER_CONFIRM_WAITING:
        return "PRELOAD_PROTOCOL_STATE_USER_CONFIRM_WAITING";
    case DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_PRELOAD_ARM_STARTED:
        return "PRELOAD_PROTOCOL_STATE_PRELOAD_ARM_STARTED";
    case DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_PRELOAD_ARM_PROGRESS:
        return "PRELOAD_PROTOCOL_STATE_PRELOAD_ARM_PROGRESS";
    case DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_PRELOAD_ARM_FAILED:
        return "PRELOAD_PROTOCOL_STATE_PRELOAD_ARM_FAILED";
    case DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_PRELOAD_ARM_DONE:
        return "PRELOAD_PROTOCOL_STATE_PRELOAD_ARM_DONE";
    case DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_PRELOAD_INJECTION_STARTED:
        return "PRELOAD_PROTOCOL_STATE_PRELOAD_INJECTION_STARTED";
    case DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_PRELOAD_INJECTION_FAILED:
        return "PRELOAD_PROTOCOL_STATE_PRELOAD_INJECTION_FAILED";
    case DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_PRELOAD_INJECTION_PROGRESS:
        return "PRELOAD_PROTOCOL_STATE_PRELOAD_INJECTION_PROGRESS";
    case DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_PRELOAD_INJECTION_END_WAITING:
        return "PRELOAD_PROTOCOL_STATE_PRELOAD_INJECTION_END_WAITING";
    case DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_PRELOAD_INJECTION_DONE:
        return "PRELOAD_PROTOCOL_STATE_PRELOAD_INJECTION_DONE";
    case DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_FAILED:
        return "PRELOAD_PROTOCOL_STATE_FAILED";
    case DS_WorkflowDef::PRELOAD_PROTOCOL_STATE_DONE:
        return "PRELOAD_PROTOCOL_STATE_DONE";
    default:
        return QString().asprintf("PRELOAD_PROTOCOL_STATE_%d", from);
    }
}

QString ImrParser::ToImr_SudsPrimeType(DS_DeviceDef::SudsPrimeType from)
{
    switch (from)
    {
    case DS_DeviceDef::SUDS_PRIME_TYPE_AUTO:
        return "AutoPrime";
    case DS_DeviceDef::SUDS_PRIME_TYPE_MANUAL:
        return "ManualPrime";
    case DS_DeviceDef::SUDS_PRIME_TYPE_SYRINGE_PRIME:
        return "MUDSPrime";
    case DS_DeviceDef::SUDS_PRIME_TYPE_FLUID_PURGE:
        return "FluidPurge";
    case DS_DeviceDef::SUDS_PRIME_TYPE_SYRINGE_AIR_RECOVERY:
        return "SyringeAirRecovery";
    case DS_DeviceDef::SUDS_PRIME_TYPE_USER_DEFINED:
        return "UserDefined";
    default:
        return QString().asprintf("Unknown (%d)", from);
    }
}

QString ImrParser::ToImr_McuInjectionPhaseType(const DS_McuDef::InjectionPhaseType from)
{
    switch (from)
    {
    case DS_McuDef::INJECTION_PHASE_TYPE_PAUSE:
        return "PAUSE";
    case DS_McuDef::INJECTION_PHASE_TYPE_CONTRAST1:
        return "CONTRAST1";
    case DS_McuDef::INJECTION_PHASE_TYPE_CONTRAST2:
        return "CONTRAST2";
    case DS_McuDef::INJECTION_PHASE_TYPE_SALINE:
        return "SALINE";
    case DS_McuDef::INJECTION_PHASE_TYPE_DUAL1:
        return "DUAL1";
    case DS_McuDef::INJECTION_PHASE_TYPE_DUAL2:
        return "DUAL2";
    default:
        return QString().asprintf("Unknown (%d)", from);
    }
}

QString ImrParser::ToImr_PowerControlType(const DS_McuDef::PowerControlType from)
{
    switch (from)
    {
    case DS_McuDef::POWER_CONTROL_TYPE_OFF:
        return "OFF";
    case DS_McuDef::POWER_CONTROL_TYPE_REBOOT:
        return "REBOOT";
    default:
        return QString().asprintf("Unknown (%d)", from);
    }
}

QString ImrParser::ToImr_FillType(DS_McuDef::FillType from)
{
    switch (from)
    {
    case DS_McuDef::FILL_TYPE_SOD:
        return "SOD";
    case DS_McuDef::FILL_TYPE_NORMAL:
    default:
        return "NORMAL";
    }
}

QString ImrParser::ToImr_AdaptiveFlowState(DS_ExamDef::AdaptiveFlowState from)
{
    switch (from)
    {
    case DS_ExamDef::ADAPTIVE_FLOW_STATE_ACTIVE:
        return "Active";
    case DS_ExamDef::ADAPTIVE_FLOW_STATE_CRITICAL:
        return "ActiveCritical";
    case DS_ExamDef::ADAPTIVE_FLOW_STATE_OFF:
    default:
        return "Off";
    }
}

QString ImrParser::ToImr_McuAdaptiveFlowState(DS_McuDef::AdaptiveFlowState from)
{
    switch (from)
    {
    case DS_McuDef::ADAPTIVE_FLOW_STATE_ACTIVE:
        return "AF_ACTIVE";
    case DS_McuDef::ADAPTIVE_FLOW_STATE_CRITICAL:
        return "AF_CRITICAL";
    case DS_McuDef::ADAPTIVE_FLOW_STATE_OFF:
    default:
        return "AF_OFF";
    }
}

QString ImrParser::ToImr_TestType(DS_TestDef::TestType from)
{
    switch (from)
    {
    case DS_TestDef::TEST_TYPE_PISTON:
        return "Piston";
    case DS_TestDef::TEST_TYPE_STOPCOCK:
        return "Stopcock";
    case DS_TestDef::TEST_TYPE_NETWORK:
        return "Network";
    case DS_TestDef::TEST_TYPE_CONTINUOUS_EXAMS:
        return "ContinuousExams";
    default:
        return QString().asprintf("Unknown (%d)", from);
    }
}

QString ImrParser::ToImr_PressureLimitSensitivityType(DS_McuDef::PressureLimitSensitivityType from)
{
    switch (from)
    {
    case DS_McuDef::PRESSURE_LIMIT_SENSITIVITY_TYPE_UNKNOWN:
        return "Unknown";
    case DS_McuDef::PRESSURE_LIMIT_SENSITIVITY_TYPE_LOW:
        return "Low";
    case DS_McuDef::PRESSURE_LIMIT_SENSITIVITY_TYPE_MEDIUM:
        return "Medium";
    case DS_McuDef::PRESSURE_LIMIT_SENSITIVITY_TYPE_DEFAULT:
        return "Default";
    case DS_McuDef::PRESSURE_LIMIT_SENSITIVITY_TYPE_HIGH:
        return "High";
    default:
        return "Invalid";
    }
}

QVariantMap ImrParser::ToImr_PhaseProgressDigest(const DS_ExamDef::PhaseProgressDigest &from, QString *err)
{
    QVariantMap ret;
    QString errStr = "";

    ret.insert("ElapsedMillisFromPhaseStart", from.elapsedMillisFromPhaseStart);
    ret.insert("MaxPressure", from.maxPressure);
    ret.insert("MaxFlowRate", from.maxFlowRate);
    ret.insert("State", ToImr_InjectionPhaseProgressState(from.state));
    ret.insert("WasFlowAdjusted", from.wasFlowAdjusted);
    ret.insert("Progress", from.progress);

    QVariantMap injectedVolumes;
    injectedVolumes.insert("RS0", from.injectedVolumes.volumes[SYRINGE_IDX_SALINE]);
    injectedVolumes.insert("RC1", from.injectedVolumes.volumes[SYRINGE_IDX_CONTRAST1]);
    injectedVolumes.insert("RC2", from.injectedVolumes.volumes[SYRINGE_IDX_CONTRAST2]);

    ret.insert("InjectedVolumes", injectedVolumes);

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_ExecutedStep(const DS_ExamDef::ExecutedStep &from, QString *err)
{
    QVariantMap ret;
    QString errStr = "";

    QVariantList phaseProgress;
    for (int phaseIdx = 0; phaseIdx < from.phaseProgress.length(); phaseIdx++)
    {
        QVariantMap phaseProgressMap = ToImr_PhaseProgressDigest(from.phaseProgress[phaseIdx], &errStr);
        if (errStr != "")
        {
            errStr = QString().asprintf("PhaseProgressDigests[%d]: ", phaseIdx) + errStr;
        }
        phaseProgress.append(phaseProgressMap);
    }

    QVariantMap instantaneousRates;
    instantaneousRates.insert("RS0", from.instantaneousRates.flowRates[SYRINGE_IDX_SALINE]);
    instantaneousRates.insert("RC1", from.instantaneousRates.flowRates[SYRINGE_IDX_CONTRAST1]);
    instantaneousRates.insert("RC2", from.instantaneousRates.flowRates[SYRINGE_IDX_CONTRAST2]);

    ret.insert("PhaseProgress", phaseProgress);
    ret.insert("InstantaneousRates", instantaneousRates);
    ret.insert("PhaseIndex", from.phaseIndex);
    ret.insert("StepIndex", from.stepIndex);
    ret.insert("ProgrammedStep", from.programmedStep);
    ret.insert("AdaptiveFlowState", ToImr_AdaptiveFlowState(from.adaptiveFlowState));
    ret.insert("HasPressureLimited", from.hasPressureLimited);
    ret.insert("PreventingBackflowSaline", from.preventingBackflowSaline);
    ret.insert("HasPreventedBackflowSaline", from.hasPreventedBackflowSaline);
    ret.insert("ActiveSalineLocation", ToImr_FluidSourceIdx(from.activeSalineLocation));
    ret.insert("ActiveContrastLocation", ToImr_FluidSourceIdx(from.activeContrastLocation));
    ret.insert("TerminationReason", ToImr_StepTerminationReason(from.terminationReason));
    ret.insert("TerminatedReasonMessage", from.terminatedReasonMessage);
    ret.insert("McuTerminatedReason", from.mcuTerminatedReason);
    ret.insert("StepStartedAt", Util::qDateTimeToUtcDateTimeStr(QDateTime::fromMSecsSinceEpoch(qMax(from.stepTriggeredEpochMs, (qint64)0))));
    ret.insert("StepCompletedAt", Util::qDateTimeToUtcDateTimeStr(QDateTime::fromMSecsSinceEpoch(qMax(from.stepCompletedEpochMs, (qint64)0))));

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantList ImrParser::ToImr_ExecutedSteps(const DS_ExamDef::ExecutedSteps &from, QString *err)
{
    QVariantList ret;
    QString errStr = "";

    for (int digestIdx = 0; digestIdx < from.length(); digestIdx++)
    {
        QVariantMap progressDigest = ToImr_ExecutedStep(from[digestIdx], &errStr);
        if (errStr != "")
        {
            errStr = QString().asprintf("ExecutedSteps[%d]: ", digestIdx) + errStr;
        }

        ret.append(progressDigest);
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_FluidSource(const DS_DeviceDef::FluidSource &from, QString *err)
{
    QVariantMap ret;
    QString errStr = "";

    if (from.isInstalled())
    {
        ret.insert("IsReady", from.isReady);
        ret.insert("NeedsReplaced", from.needsReplaced);
        ret.insert("IsBusy", from.isBusy);
        ret.insert("InstalledAt", from.installedAtEpochMs > 0 ? Util::qDateTimeToUtcDateTimeStr(QDateTime::fromMSecsSinceEpoch(from.installedAtEpochMs)) : QVariant());
        ret.insert("SourcePackages", ToImr_FluidPackages(from.sourcePackages));
        ret.insert("DisposableType", ToImr_FluidSourceDisposableType(from.disposableType));

        QVariantList currentVolumes;
        for (int i = 0; i < from.currentVolumes.length(); i++)
        {
            currentVolumes.append(from.currentVolumes[i]);
        }
        ret.insert("CurrentVolumes", currentVolumes);

        QVariantList currentFluidKinds;
        for (int i = 0; i < from.currentFluidKinds.length(); i++)
        {
            currentFluidKinds.append(from.currentFluidKinds[i]);
        }
        ret.insert("CurrentFluidKinds", currentFluidKinds);
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_FluidSources(const DS_DeviceDef::FluidSources &from, QString *err)
{
    QVariantMap ret;
    QString errStr = "";

    for (int fluidSourceIdx = 0; fluidSourceIdx < from.length(); fluidSourceIdx++)
    {
        QVariantMap fluidSource = ToImr_FluidSource(from[fluidSourceIdx], &errStr);

        if (errStr != "")
        {
            errStr = QString().asprintf("FluidSource[%d]: ", fluidSourceIdx) + errStr;
        }
        ret.insert(ToImr_FluidSourceIdx((DS_DeviceDef::FluidSourceIdx)fluidSourceIdx), fluidSource);
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_FluidSource(const DS_DeviceDef::FluidSource &from, DS_DeviceDef::FluidSourceIdx fluidSrcFilter, QString *err)
{
    QVariantMap ret;
    QString errStr = "";

    QVariantMap source = ToImr_FluidSource(from, &errStr);
    QString sourceType = ToImr_FluidSourceIdx(fluidSrcFilter);

    if (source.size() == 0)
    {
        ret.insert(sourceType, QVariant());
    }
    else
    {
        ret.insert(sourceType, source);
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_FluidSourceSyringes(const DS_DeviceDef::FluidSources &from, QString *err)
{
    QVariantMap ret;
    QString errStr = "";

    for (int srcIdx = 0; srcIdx < from.length(); srcIdx++)
    {
        QVariantMap source = ToImr_FluidSource(from[srcIdx], &errStr);
        QString sourceType = ToImr_FluidSourceSyringeIdx((SyringeIdx)srcIdx);

        if (source.size() == 0)
        {
            ret.insert(sourceType, QVariant());
        }
        else
        {
            ret.insert(sourceType, source);
        }
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_FluidSourceBottles(const DS_DeviceDef::FluidSources &from, QString *err)
{
    QVariantMap ret;
    QString errStr = "";

    for (int srcIdx = 0; srcIdx < from.length(); srcIdx++)
    {
        QVariantMap source = ToImr_FluidSource(from[srcIdx], &errStr);
        QString sourceType = ToImr_FluidSourceBottleIdx((SyringeIdx)srcIdx);

        if (source.size() == 0)
        {
            ret.insert(sourceType, QVariant());
        }
        else
        {
            ret.insert(sourceType, source);
        }
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_PowerStatus(const DS_McuDef::PowerStatus &from, QString *err)
{
    QVariantMap ret;
    QString errStr = "";

    ret.insert("BatteryLevel", ToImr_BatteryLevel(from.batteryLevel));
    ret.insert("BatteryCharge", from.batteryCharge);
    ret.insert("IsAcPowered", from.isAcPowered);

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_InjectionRequestProcessStatus(const DS_ExamDef::InjectionRequestProcessStatus &from, QString *err)
{
    QVariantMap ret;
    QString errStr = "";

    ret.insert("RequestedByHcu", from.requestedByHcu);
    ret.insert("State", from.state);

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_InjectionPersonalizationInput(const DS_ExamDef::InjectionPersonalizationInput &from, QString *err)
{
    QVariantMap ret;
    QString errStr = "";

    if (from.validDataType == _L("bool"))
    {
        ret.insert("Value", from.value.toBool());
    }
    else if (from.validDataType == _L("double"))
    {
        ret.insert("Value", from.value.toDouble());
    }
    else if (from.validDataType == _L("int"))
    {
        ret.insert("Value", from.value.toInt());
    }
    else
    {
        ret.insert("Value", from.value);
    }

    ret.insert("Name", from.name);
    ret.insert("Units", from.units);
    ret.insert("Source", ToImr_InjectionPersonalizationInputSource(from.source));
    ret.insert("ValidDataType", from.validDataType);
    ret.insert("ValidList", from.validList);
    ret.insert("ValidRange", ToImr_ConfigItemValidRange(from.validRange));
    ret.insert("IsEnabled", from.isEnabled);
    ret.insert("IsValueVisible", from.isValueVisible);

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantList ImrParser::ToImr_InjectionPersonalizationInputs(const DS_ExamDef::InjectionPersonalizationInputs &from, QString *err)
{
    QVariantList ret;
    QString errStr = "";

    for (int paramIdx = 0; paramIdx < from.length(); paramIdx++)
    {
        QVariantMap param = ToImr_InjectionPersonalizationInput(from[paramIdx], &errStr);

        if (errStr != "")
        {
            errStr = QString().asprintf("InjectionPersonalizationInputs[%d]: ", paramIdx) + errStr;
        }
        ret.append(param);
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_InjectionPersonalizationNotice(const DS_ExamDef::InjectionPersonalizationNotice &from, QString *err)
{
    QVariantMap ret;
    QString errStr = "";

    ret.insert("Name", ToImr_InjectionPersonalizationNoticeType(from.name));
    ret.insert("Importance", ToImr_InjectionPersonalizationNoticeImportance(from.importance));
    ret.insert("Values", from.values);

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantList ImrParser::ToImr_InjectionPersonalizationNotices(const DS_ExamDef::InjectionPersonalizationNotices &from, QString *err)
{
    QVariantList ret;
    QString errStr = "";

    for (int paramIdx = 0; paramIdx < from.length(); paramIdx++)
    {
        QVariantMap map = ToImr_InjectionPersonalizationNotice(from[paramIdx], &errStr);

        if (errStr != "")
        {
            errStr = QString().asprintf("InjectionPersonalizationNotices[%d]: ", paramIdx) + errStr;
        }

        ret.append(map);
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_SharingInformation(const DS_ExamDef::SharingInformation &from, QString *err)
{
    QVariantMap ret;
    QString errStr = "";

    ret.insert("Logo", from.logo);
    ret.insert("Url", from.url);
    ret.insert("Credits", from.credits);
    ret.insert("Version", from.version);
    ret.insert("Passcode", from.passcode);

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_InjectionPhase(const DS_ExamDef::InjectionPhase &from, QString *err)
{
    QVariantMap ret;
    QString errStr = "";

    ret.insert("Type", ToImr_InjectionPhaseType(from.type));
    ret.insert("Duration", Util::millisecToDurationStr(from.durationMs));
    ret.insert("FlowRate", from.flowRate);
    ret.insert("TotalVolume", from.totalVol);
    ret.insert("ContrastPercentage", from.contrastPercentage);
    ret.insert("OriginalPhaseIdx", from.originalPhaseIdx);

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantList ImrParser::ToImr_InjectionPhases(const DS_ExamDef::InjectionPhases &from, QString *err)
{
    QVariantList ret;
    QString errStr = "";

    for (int phaseIdx = 0; phaseIdx < from.length(); phaseIdx++)
    {
        QVariantMap phase = ToImr_InjectionPhase(from[phaseIdx], &errStr);

        if (errStr != "")
        {
            errStr = QString().asprintf("Phase[%d]: ", phaseIdx) + errStr;
        }
        ret.append(phase);
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_InjectionStep(const DS_ExamDef::InjectionStep &from, QString *err)
{
    QVariantMap ret;
    QVariantList reminders, phases;
    QString errStr = "";

    ret.insert("GUID", from.guid);
    ret.insert("Name", from.name);
    ret.insert("Template", from.templateGuid);
    ret.insert("PressureLimit", from.pressureLimit);
    ret.insert("IsTestInjection", from.isTestInjection);
    ret.insert("IsNotScannerSynchronized", from.isNotScannerSynchronized);
    ret.insert("ContrastFluidLocationName", ToImr_FluidSourceIdx(from.contrastFluidLocation));
    ret.insert("SalineFluidLocationName", ToImr_FluidSourceIdx(from.salineFluidLocation));
    ret.insert("PersonalizationNotices", ToImr_InjectionPersonalizationNotices(from.personalizationNotices));
    ret.insert("Notices", ToImr_InjectionPersonalizationNotices(from.notices));
    ret.insert("PersonalizationInputs", ToImr_InjectionPersonalizationInputs(from.personalizationInputs));
    ret.insert("PersonalizationGenerator", from.personalizationGenerator);
    ret.insert("IsPreloaded", from.isPreloaded);

    QVariantList personalizationModifiers;
    for (int modifierIdx = 0; modifierIdx < from.personalizationModifiers.length(); modifierIdx++)
    {
        personalizationModifiers.append(from.personalizationModifiers[modifierIdx]);
    }
    ret.insert("PersonalizationModifiers", personalizationModifiers);

    // Set ProgrammedAt
    qint64 programmedAtEpochMs = qMax((qint64)0, from.programmedAtEpochMs);
    ret.insert("ProgrammedAt", Util::qDateTimeToUtcDateTimeStr(QDateTime::fromMSecsSinceEpoch(programmedAtEpochMs)));

    // Set reminders
    reminders = ToImr_Reminders(from.reminders, &errStr);
    ret.insert("Reminders", reminders);

    // Set phases
    phases = ToImr_InjectionPhases(from.phases, &errStr);
    ret.insert("Phases", phases);

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_InjectionPlan(const DS_ExamDef::InjectionPlan &from, QString *err)
{
    QVariantMap ret;
    QString errStr = "";

    QVariantList steps;

    ret.insert("GUID", from.guid);
    ret.insert("Name", from.name);
    ret.insert("Description", from.description);
    ret.insert("Template", from.templateGuid);
    ret.insert("IsModifiedFromTemplate", from.isModifiedFromTemplate);
    ret.insert("IsPersonalized", from.isPersonalized);
    ret.insert("PersonalizationInputs", ImrParser::ToImr_InjectionPersonalizationInputs(from.personalizationInputs));
    ret.insert("InjectionSettings", from.injectionSettings);
    ret.insert("SharingInformation", ImrParser::ToImr_SharingInformation(from.sharingInformation));
    ret.insert("IsPreloadable", from.isPreloadable);

    // Parse steps
    for (int stepIdx = 0; stepIdx < from.steps.length(); stepIdx++)
    {
        QVariantMap step = ToImr_InjectionStep(from.steps[stepIdx], &errStr);

        if (errStr != "")
        {
            errStr = QString().asprintf("Steps[%d]: ", stepIdx) + errStr;
        }

        steps.append(step);
    }
    ret.insert("Steps", steps);

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_InjectionPlanDigest(const DS_ExamDef::InjectionPlanDigest &from, QString *err)
{
    QVariantMap ret;
    QString errStr = "";

    ret.insert("GUID", from.guid);
    ret.insert("HistId", from.histId);
    ret.insert("Name", from.name);
    ret.insert("State", ToImr_InjectionPlanDigestState(from.state));
    ret.insert("IsPersonalized", from.isPersonalized);

    QVariantMap plan = ToImr_InjectionPlan(from.plan, &errStr);

    if (errStr != "")
    {
        errStr = "InjectionPlan: " + errStr;
    }

    ret.insert("Plan", plan);

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantList ImrParser::ToImr_InjectionPlanDigests(const DS_ExamDef::InjectionPlanDigests &from, QString *err)
{
    QVariantList ret;
    QString errStr = "";

    for (int digestIdx = 0; digestIdx < from.length(); digestIdx++)
    {
        QVariantMap digest = ToImr_InjectionPlanDigest(from[digestIdx], &errStr);

        if (errStr != "")
        {
            errStr = QString().asprintf("PlanDigests[%d]: ", digestIdx) + errStr;
        }
        ret.append(digest);
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_InjectionPlanTemplateGroup(const DS_ExamDef::InjectionPlanTemplateGroup &from, QString *err)
{
    QVariantMap ret;
    QString errStr = "";

    ret.insert("Name", from.name);

    QVariantList planDigests = ToImr_InjectionPlanDigests(from.planDigests, &errStr);

    if (errStr != "")
    {
        errStr = "PlanDigests: " + errStr;
    }

    ret.insert("PlanDigests", planDigests);

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantList ImrParser::ToImr_InjectionPlanTemplateGroups(const DS_ExamDef::InjectionPlanTemplateGroups &from, QString *err)
{
    QVariantList ret;
    QString errStr = "";

    for (int groupIdx = 0; groupIdx < from.length(); groupIdx++)
    {
        QVariantMap group = ToImr_InjectionPlanTemplateGroup(from[groupIdx], &errStr);

        if (errStr != "")
        {
            errStr = QString().asprintf("Groups[%d]: ", groupIdx) + errStr;
        }

        ret.append(group);
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_Reminder(const DS_ExamDef::Reminder &from, QString *err)
{
    QVariantMap ret;
    QString errStr = "";

    ret.insert("Name", from.name);
    ret.insert("Description", from.description);
    ret.insert("PostStepTriggerDelay", Util::millisecToDurationStr(from.postStepTriggerDelayMs));
    ret.insert("NotifyExternalSystem", from.notifyExternalSystem);
    ret.insert("StartAfterStepCompletes", from.startAfterStepCompletes);

    QVariantList displayHints;

    for (int i = 0; i < from.displayHints.length(); i++)
    {
        QString displayHint = ToImr_TimedReminderDisplayHint(from.displayHints[i]);
        displayHints.append(displayHint);
    }

    ret.insert("DisplayHints", displayHints);

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantList ImrParser::ToImr_Reminders(const DS_ExamDef::Reminders &from, QString *err)
{
    QVariantList ret;
    QString errStr = "";

    for (int i = 0; i < from.length(); i++)
    {
        QVariantMap map = ToImr_Reminder(from[i], &errStr);
        if (errStr != "")
        {
            errStr = QString().asprintf("Reminders[%d]: ", i) + errStr;
        }
        ret.append(map);
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_ScannerInterlocks(const DS_ExamDef::ScannerInterlocks &from, QString *err)
{
    QVariantMap ret;
    QString errStr = "";

    ret.insert("ConfigLockedOut", from.configLockedOut);
    ret.insert("ArmLockedOut", from.armLockedOut);
    ret.insert("StartLockedOut", from.startLockedOut);
    ret.insert("ResumeLockedOut", from.resumeLockedOut);
    ret.insert("ScannerReady", from.scannerReady);
    ret.insert("InterfaceStatus", ToImr_ScannerInterfaceStatus(from.interfaceStatus));
    ret.insert("InterfaceMode", ToImr_ScannerInterfaceMode(from.interfaceMode));

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_WorklistDetails(const DS_ExamDef::WorklistDetails &from, QString *err)
{
    QVariantMap ret;
    QString errStr = "";

    ret.insert("Entry", ToImr_WorklistEntry(from.entry, &errStr));
    ret.insert("Panel", ToImr_DicomFields(from.panel, &errStr));
    ret.insert("All", ToImr_DicomFields(from.all, &errStr));

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_ExamField(const DS_ExamDef::ExamField &from, QString *err)
{
    QVariantMap ret;
    QString errStr = "";

    ret.insert("Name", from.name);
    ret.insert("Value", from.value);
    ret.insert("Units", from.units);
    ret.insert("ValidDataType", from.validDataType);
    ret.insert("ValidRange", ToImr_ConfigItemValidRange(from.validRange));
    ret.insert("ValidList", from.validList);
    ret.insert("NeedsTranslated", from.needsTranslated);
    ret.insert("IsEnabled", from.isEnabled);
    ret.insert("IsMandatory", from.isMandatory);
    ret.insert("IsMandatoryEntered", from.isMandatoryEntered);

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_ExamFieldMap(const DS_ExamDef::ExamFieldMap &from, QString *err)
{
    QVariantMap ret;
    QString errStr = "";

    QMap<QString, DS_ExamDef::ExamField>::const_iterator i = from.begin();

    i = from.begin();
    while (i != from.end())
    {
        QString key = i.key();
        DS_ExamDef::ExamField examFieldCpp = i.value();
        QVariantMap examField = ToImr_ExamField(examFieldCpp, &errStr);
        if (errStr != "")
        {
            errStr = QString().asprintf("ExamFieldMap[%s]: ", key.CSTR()) + errStr;
        }
        ret.insert(key, examField);
        i++;
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

// Currently ExamFieldParameter and InjectionPersonalizationInput are identical. Creating dummy layer so that caller is independent of the implementation
QVariantMap ImrParser::ToImr_ExamFieldParameter(const DS_ExamDef::ExamFieldParameter &from, QString *err)
{
    return ToImr_InjectionPersonalizationInput(from, err);
}

QVariantMap ImrParser::ToImr_LinkedAccession(const DS_ExamDef::LinkedAccession &from, QString *err)
{
    QVariantMap ret;
    QString errStr = "";

    ret.insert("IsLinked", from.isLinked);
    ret.insert("Entry", ToImr_WorklistEntry(from.entry, &errStr));

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantList ImrParser::ToImr_LinkedAccessions(const DS_ExamDef::LinkedAccessions &from, QString *err)
{
    QVariantList ret;
    QString errStr = "";

    for (int accessionId = 0; accessionId < from.length(); accessionId++)
    {
        QVariantMap params = ToImr_LinkedAccession(from[accessionId], &errStr);
        if (errStr != "")
        {
            errStr = QString().asprintf("LinkedAccessions[%d]: ", accessionId) + errStr;
        }
        ret.append(params);
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_ExamAdvanceInfo(const DS_ExamDef::ExamAdvanceInfo &from, QString *err)
{
    QVariantMap ret;
    QString errStr = "";

    ret.insert("GUID", from.guid);
    ret.insert("WorklistDetails", ToImr_WorklistDetails(from.worklistDetails, &errStr));
    ret.insert("ExamInputs", ToImr_ExamFieldMap(from.examInputs, &errStr));
    ret.insert("ExamResults", ToImr_ExamFieldMap(from.examResults, &errStr));
    ret.insert("LinkedAccessions", ToImr_LinkedAccessions(from.linkedAccessions, &errStr));
    ret.insert("MandatoryFieldsEntered", from.mandatoryFieldsEntered);
    ret.insert("ExamInputsProgress", from.examInputsProgress);
    ret.insert("ExamResultsProgress", from.examResultsProgress);

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_CruLinkStatus(const DS_CruDef::CruLinkStatus &from, QString *err)
{
    QVariantMap ret;
    QString errStr = "";

    ret.insert("Type", ToImr_CruLinkType(from.type));
    ret.insert("State", ToImr_CruLinkState(from.state));
    ret.insert("Quality", ToImr_CruLinkQuality(from.quality));
    ret.insert("SignalLevel", from.signalLevel);

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantList ImrParser::ToImr_StopcockPosAll(const DS_McuDef::StopcockPosAll &from, QString *err)
{
    QVariantList ret;
    QString errStr = "";

    for (int i = 0; i < from.length(); i++)
    {
        ret.append(ToImr_StopcockPos(from[i]));
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QString ImrParser::ToImr_SimDigestStr(const DS_McuDef::SimDigest &from, QString *err)
{
    QString ret;
    QString errStr = "";

    ret = QString().asprintf("%s,%s,%s,%d,SC_%s,SC_%s,SC_%s,%s,%s,%s,%s,%s,%s,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,"
                            ,from.alarmCode.toHex().CSTR()
                            ,from.injectState.CSTR()
                            ,from.injectCompleteStatus.CSTR()
                            ,from.pressure
                            ,from.stopcockPos[SYRINGE_IDX_SALINE].CSTR()
                            ,from.stopcockPos[SYRINGE_IDX_CONTRAST1].CSTR()
                            ,from.stopcockPos[SYRINGE_IDX_CONTRAST2].CSTR()
                            ,from.plungerState[SYRINGE_IDX_SALINE].CSTR()
                            ,from.plungerState[SYRINGE_IDX_CONTRAST1].CSTR()
                            ,from.plungerState[SYRINGE_IDX_CONTRAST2].CSTR()
                            ,from.syrAction[SYRINGE_IDX_SALINE].CSTR()
                            ,from.syrAction[SYRINGE_IDX_CONTRAST1].CSTR()
                            ,from.syrAction[SYRINGE_IDX_CONTRAST2].CSTR()
                            ,from.vol[SYRINGE_IDX_SALINE]
                            ,from.vol[SYRINGE_IDX_CONTRAST1]
                            ,from.vol[SYRINGE_IDX_CONTRAST2]
                            ,from.flow[SYRINGE_IDX_SALINE]
                            ,from.flow[SYRINGE_IDX_CONTRAST1]
                            ,from.flow[SYRINGE_IDX_CONTRAST2]);


    ret += QString().asprintf("BT_%s,%s,DOOR_%s,WC_%s,%s,%s,INLET_%s,INLET_%s,INLET_%s,%s,%s,%s,%s,%s,O_DOOR_%s,%f,%f,%s,%s,%s"
                            ,from.batteryState.CSTR()
                            ,from.isAcPowered ? "AC" : "BATTERY"
                            ,from.doorState.CSTR()
                            ,from.wasteBinState.CSTR()
                            ,from.mudsPresent ? "MUDS_PRESENT" : "MUDS_MISSING"
                            ,from.mudsLatched ? "LATCHED" : "UNLATCHED"
                            ,ToImr_BottleBubbleDetectorState(from.bottleAirDetectedState[SYRINGE_IDX_SALINE]).CSTR()
                            ,ToImr_BottleBubbleDetectorState(from.bottleAirDetectedState[SYRINGE_IDX_CONTRAST1]).CSTR()
                            ,ToImr_BottleBubbleDetectorState(from.bottleAirDetectedState[SYRINGE_IDX_CONTRAST2]).CSTR()
                            ,from.sudsInserted ? "SUDS_PRESENT" : "SUDS_MISSING"
                            ,from.sudsBubbleDetected ? "SUDS_AIR" : "SUDS_FLUID"
                            ,from.primeBtnPressed ? "PR_BTN_PRESSED" : "PR_BTN_RELEASED"
                            ,from.stopBtnPressed ? "S_BTN_PRESSED" : "S_BTN_RELEASED"
                            ,from.doorBtnPressed ? "D_BTN_PRESSED" : "D_BTN_RELEASED"
                            ,from.outletDoorState.CSTR()
                            ,from.temperature1
                            ,from.temperature2
                            ,from.heatMaintainerState.CSTR()
                            ,from.isShuttingDown ? "SHUTDOWN" : "IDLE"
                            // this is MCUDignosticEventOccurred field. Keep it empty for sim
                            , "");


    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QString ImrParser::ToImr_InjectDigestStr(const DS_McuDef::InjectDigest &from, QString *err)
{
    QString ret;
    QString errStr = "";

    ret.append(QString().asprintf("%d,%s,%d,%d,%d,%d,%d,%d,%d,%d,%d,%s,%s,%s,%d,%d,%d,%.02f,%.02f,%.02f,%.02f,%.02f,%.02f,%.02f,%.02f,%.02f",
                                 from.phaseIdx,
                                 ToImr_McuAdaptiveFlowState(from.adaptiveFlowState).CSTR(),
                                 from.scheduledPulsingActive ? 1 : 0,
                                 from.unscheduledPulsingActive ? 1 : 0,
                                 from.injectionPressure ? 1 : 0,
                                 from.syringeInjectDigests[0].pressure,
                                 from.syringeInjectDigests[1].pressure,
                                 from.syringeInjectDigests[2].pressure,
                                 from.syringeInjectDigests[0].motorPid,
                                 from.syringeInjectDigests[1].motorPid,
                                 from.syringeInjectDigests[2].motorPid,
                                 ImrParser::ToImr_StopcockPos(from.syringeInjectDigests[0].scPos).CSTR(),
                                 ImrParser::ToImr_StopcockPos(from.syringeInjectDigests[1].scPos).CSTR(),
                                 ImrParser::ToImr_StopcockPos(from.syringeInjectDigests[2].scPos).CSTR(),
                                 from.syringeInjectDigests[0].motorPos,
                                 from.syringeInjectDigests[1].motorPos,
                                 from.syringeInjectDigests[2].motorPos,
                                 from.syringeInjectDigests[0].slowStartReduction,
                                 from.syringeInjectDigests[1].slowStartReduction,
                                 from.syringeInjectDigests[2].slowStartReduction,
                                 from.syringeInjectDigests[0].storedCompliance,
                                 from.syringeInjectDigests[1].storedCompliance,
                                 from.syringeInjectDigests[2].storedCompliance,
                                 from.syringeInjectDigests[0].phaseCompliance,
                                 from.syringeInjectDigests[1].phaseCompliance,
                                 from.syringeInjectDigests[2].phaseCompliance));


    ret.append(QString().asprintf(",%d,%d,%d,%d,%d",
                                 from.patientLineAirCounts,
                                 from.adcPinReading120,
                                 from.adcPinReading121,
                                 from.adcPinReading122,
                                 from.pressureMonitorPortReading));


    ret.append(QString().asprintf(",%.02f,%.02f,%.02f,%.02f,%.02f,%.02f",
                                 from.syringeInjectDigests[SYRINGE_IDX_SALINE].flowRate,
                                 from.syringeInjectDigests[SYRINGE_IDX_SALINE + 1].flowRate,
                                 from.syringeInjectDigests[SYRINGE_IDX_SALINE + 2].flowRate,
                                 from.syringeInjectDigests[SYRINGE_IDX_SALINE].volPushed,
                                 from.syringeInjectDigests[SYRINGE_IDX_SALINE + 1].volPushed,
                                 from.syringeInjectDigests[SYRINGE_IDX_SALINE + 2].volPushed));


    for (int phasseIdx = 0; phasseIdx <= from.phaseIdx; phasseIdx++)
    {
        ret.append(QString().asprintf(",%.02f,%.02f,%.02f,%.02llu",
                                     from.phaseInjectDigests[phasseIdx].volumes[SYRINGE_IDX_SALINE],
                                     from.phaseInjectDigests[phasseIdx].volumes[SYRINGE_IDX_SALINE + 1],
                                     from.phaseInjectDigests[phasseIdx].volumes[SYRINGE_IDX_SALINE + 2],
                                     from.phaseInjectDigests[phasseIdx].duration));
    }

    if (err != NULL)
    {
        *err = errStr;
    }

    return ret;
}

QVariantMap ImrParser::ToImr_PressureCalibrationStageParams(const DS_McuDef::PressureCalibrationStageParams &from, QString *err)
{
    QVariantMap ret;
    QString errStr = "";

    ret.insert("FlowRate", from.flowRate);
    ret.insert("DataCaptureIntervalMs", from.dataCaptureIntervalMs);

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantList ImrParser::ToImr_PressureCalibrationStagesParams(const DS_McuDef::PressureCalibrationStagesParams &from, QString *err)
{
    QVariantList ret;
    QString errStr = "";

    for (int stageIdx = 0; stageIdx < from.length(); stageIdx++)
    {
        QVariantMap params = ToImr_PressureCalibrationStageParams(from[stageIdx], &errStr);
        if (errStr != "")
        {
            errStr = QString().asprintf("PressureCalibrationStagesParams[%d]: ", stageIdx) + errStr;
        }
        ret.append(params);
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_PressureCalibrationDataCheckInfo(const DS_McuDef::PressureCalibrationDataCheckInfo &from, QString *err)
{
    QVariantMap ret;
    QString errStr = "";

    ret.insert("SampleIdx", from.sampleIdx);
    ret.insert("AdcValuesSum", from.adcValuesSum);
    ret.insert("LastMovingAverage", from.lastMovingAverage);

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_PressureCalibrationStatus(const DS_McuDef::PressureCalibrationStatus &from, QString *err)
{
    QVariantMap ret;
    QString errStr = "";

    ret.insert("State", ToImr_PressureCalibrationState(from.state));
    ret.insert("Err", from.err);
    ret.insert("InjectStageIdx", from.injectStageIdx);
    ret.insert("AdcReadValue", from.adcReadValue);
    ret.insert("HomePosition", from.homePosition);
    ret.insert("EngagedPosition", from.engagedPosition);
    ret.insert("FirstGoodAdcValuePosition", from.firstGoodAdcValuePosition);
    ret.insert("StagesParams", ToImr_PressureCalibrationStagesParams(from.stagesParams));
    ret.insert("DataCheckInfo", ToImr_PressureCalibrationDataCheckInfo(from.dataCheckInfo));

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantList ImrParser::ToImr_SyringeStates(const DS_McuDef::SyringeStates &from, QString *err)
{
    QVariantList ret;
    QString errStr = "";

    for (int i = 0; i < from.length(); i++)
    {
        ret.append(ToImr_SyringeState(from[i]));
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantList ImrParser::ToImr_PlungerStates(const DS_McuDef::PlungerStates &from, QString *err)
{
    QVariantList ret;
    QString errStr = "";

    for (int i = 0; i < from.length(); i++)
    {
        ret.append(ToImr_PlungerState(from[i]));
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantList ImrParser::ToImr_BottleBubbleDetectorStates(const DS_McuDef::BottleBubbleDetectorStates &from, QString *err)
{
    QVariantList ret;
    QString errStr = "";

    for (int i = 0; i < from.length(); i++)
    {
        ret.append(ToImr_BottleBubbleDetectorState(from[i]));
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantList ImrParser::ToImr_DoubleList(const QList<double> &from, QString *err)
{
    QVariantList ret;
    QString errStr = "";

    for (int i = 0; i < from.length(); i++)
    {
        ret.append(from[i]);
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_TestStatus(const DS_TestDef::TestStatus &from, QString *err)
{
    QVariantMap ret;
    QString errStr = "";

    ret.insert("GUID", from.guid);
    ret.insert("Type", ToImr_TestType(from.type));
    ret.insert("Progress", from.progress);
    ret.insert("StateStr", from.stateStr);
    ret.insert("IsFinished", from.isFinished);
    ret.insert("UserAborted", from.userAborted);
    ret.insert("StatusMap", from.statusMap);

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QString ImrParser::ToImr_McuActLedParams(const DS_McuDef::ActLedParams &from, QString *err)
{
    QString ret;
    QString errStr = "";

    switch (from.type)
    {
    case DS_McuDef::LED_CONTROL_TYPE_NO_CHANGE:
        ret = "NOCHANGE";
        break;
    case DS_McuDef::LED_CONTROL_TYPE_OFF:
        ret = "OFF";
        break;
    case DS_McuDef::LED_CONTROL_TYPE_SET:
        ret = QString().asprintf("%02x%02x%02x", from.colorR, from.colorG, from.colorB);
        break;
    default:
        ret = "UNKNOWN";
        break;

    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_ActLedParams(const DS_McuDef::ActLedParams &from, QString *err)
{
    QVariantMap ret;
    QString errStr = "";

    ret.insert("Type", ToImr_LedControlType(from.type));
    ret.insert("Color", QString().asprintf("%02x%02x%02x", from.colorR, from.colorG, from.colorB));
    ret.insert("IsFlashing", from.isFlashing);

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_ActPrimeParams(const DS_McuDef::ActPrimeParams &from, QString *err)
{
    QVariantMap ret;
    QString errStr = "";

    ret.insert("Idx", ToImr_FluidSourceSyringeIdx(from.idx));
    ret.insert("Vol1", from.vol1);
    ret.insert("Vol2", from.vol2);
    ret.insert("Flow", from.flow);
    ret.insert("OperationName", from.operationName);

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;

}

QVariantList ImrParser::ToImr_ActPrimeParamsList(const DS_McuDef::ActPrimeParamsList &from, QString *err)
{
    QVariantList ret;
    QString errStr = "";

    for (int paramIdx = 0; paramIdx < from.length(); paramIdx++)
    {
        QVariantMap params = ToImr_ActPrimeParams(from[paramIdx], &errStr);
        if (errStr != "")
        {
            errStr = QString().asprintf("PrimeParams[%d]: ", paramIdx) + errStr;
        }
        ret.append(params);
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_LedControlStatus(const DS_McuDef::LedControlStatus &from, QString *err)
{
    QVariantMap ret;
    QString errStr = "";

    for (int ledIdx = 0; ledIdx < from.paramsList.length(); ledIdx++)
    {
        QString key = ToImr_LedIndex((LedIndex)ledIdx);
        QString ctrl = ToImr_McuActLedParams(from.paramsList[ledIdx], (errStr == "") ? &errStr : NULL);

        if (errStr != "")
        {
            errStr = QString().asprintf("LedControl[%d]: ", ledIdx) + errStr;
        }

        ret.insert(key, ctrl);
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_DataServiceActionStatus(const DataServiceActionStatus &from, QString *err)
{
    QVariantMap ret;
    QString errStr = "";

    ret.insert("State", ToImr_DataServiceActionState(from.state));
    ret.insert("GUID", from.guid);
    ret.insert("Request", from.request);
    ret.insert("Arg", from.arg);
    ret.insert("Reply", from.reply);
    ret.insert("Err", from.err);

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QString ImrParser::ToImr_DataServiceActionStatusStr(const DataServiceActionStatus &from, QString *err)
{
    QString ret;

    QString requestStr = from.request;
    QString replyStr = from.reply;

    QString type = getenv("BUILD_TYPE");
    bool anonymizeOn = (type == BUILD_TYPE_VNV || type == BUILD_TYPE_REL);

    if (anonymizeOn)
    {
        // Anonymize request string depending on URL used
        if (from.request.contains(IMR_CRU_URL_UPDATE_EXAM_FIELD))
        {
            // Anonymize by not including the value
            requestStr = requestStr.split("&value=")[0] + ANONYMIZED_STR;
        }

        // Anonymize reply string depending on URL used
        if ( (from.request.contains(IMR_CRU_URL_DIGEST)) ||
             (from.request.contains(IMR_CRU_URL_WORKLIST)) ||
             (from.request.contains(IMR_CRU_URL_SELECT_WORKLIST_ENTRY)) )
        {
            // Anonymize by not including the reply at all
            replyStr = ANONYMIZED_STR;
        }
    }

    // Output: Action[Req]: <STATE>: {}
    ret = QString().asprintf("Action[%s][%s]:<%s>", from.guid.CSTR(), requestStr.CSTR(), ToImr_DataServiceActionState(from.state).CSTR());
    if (from.arg != "")
    {
        ret += QString().asprintf(", Arg=%s", from.arg.CSTR());
    }

    if (from.reply != "")
    {
        ret += QString().asprintf(", Rply=%s", replyStr.CSTR());
    }

    if (from.err != "")
    {
        ret += QString().asprintf(", Err=%s", from.err.CSTR());
    }


    return ret;
}

QVariantMap ImrParser::ToImr_SyringeSodStatus(const DS_WorkflowDef::SyringeSodStatus &from, QString *err)
{
    QVariantMap ret;
    QString errStr = "";

    ret.insert("PlungerEngaged", from.plungerEngaged);
    ret.insert("CalSlackDone", from.calSlackDone);
    ret.insert("Primed", from.primed);
    ret.insert("AirCheckCalibrated", from.airCheckCalibrated);

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_MudsSodStatus(const DS_WorkflowDef::MudsSodStatus &from, QString *err)
{
    QVariantMap ret;
    QString errStr = "";

    ret.insert("Identified", from.identified);

    QVariantList syringeSodStatusAll;
    for (int syringeIdx = 0; syringeIdx < from.syringeSodStatusAll.length(); syringeIdx++)
    {
        syringeSodStatusAll.append(ToImr_SyringeSodStatus(from.syringeSodStatusAll[syringeIdx], &errStr));
    }

    ret.insert("SyringeSodStatusAll", syringeSodStatusAll);

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_UpgradeStatus(const DS_UpgradeDef::UpgradeStatus &from, QString *err)
{
    QVariantMap ret;
    QString errStr = "";

    ret.insert("Err", from.err);
    ret.insert("PathFile", from.pathFile);
    ret.insert("FileSizeKB", from.fileSizeKB);
    ret.insert("Progress", from.progress);

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_UpgradeDigest(const DS_UpgradeDef::UpgradeDigest &from, QString *err)
{
    QVariantMap ret;
    QString errStr = "";

    ret.insert("State", ToImr_UpgradeState(from.state));
    ret.insert("Sru", ToImr_UpgradeStatus(from.sru));
    ret.insert("Hcu", ToImr_UpgradeStatus(from.hcu));
    ret.insert("Mcu", ToImr_UpgradeStatus(from.mcu));
    ret.insert("Stopcock", ToImr_UpgradeStatus(from.stopcock));

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_BarcodeInfo(const DS_DeviceDef::BarcodeInfo &from, QString *err)
{
    QVariantMap ret;
    QString errStr = "";

    ret.insert("BarcodePrefix", from.barcodePrefix);
    ret.insert("ScannedAt", Util::qDateTimeToUtcDateTimeStr(QDateTime::fromMSecsSinceEpoch(from.scannedAtEpochMs)));
    ret.insert("IsKnownBarcode", from.isKnownBarcode);
    ret.insert("IsFluidPackageOk", from.isFluidPackageOk);
    ret.insert("FluidPackage", ToImr_FluidPackage(from.fluidPackage));
    ret.insert("Err", from.err);

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_WorkflowErrorStatus(const DS_WorkflowDef::WorkflowErrorStatus &from)
{
    QVariantMap ret;

    ret.insert("State", ToImr_WorkflowErrorState(from.state));
    ret.insert("SyringeIndexFailed", from.syringeIndexFailed);
    return ret;
}

QVariantMap ImrParser::ToImr_McuInjectionPhase(const DS_McuDef::InjectionPhase &from)
{
    QVariantMap ret;
    ret.insert("Type", ToImr_McuInjectionPhaseType(from.type));
    ret.insert("Mix", from.mix);
    ret.insert("Vol", from.vol);
    ret.insert("Flow", from.flow);
    ret.insert("Duration", from.duration);

    return ret;
}

QVariantList ImrParser::ToImr_McuInjectionPhases(const DS_McuDef::InjectionPhases &from)
{
    QVariantList ret;

    for (int phaseIdx = 0; phaseIdx < from.length(); phaseIdx++)
    {
        QVariantMap phase = ToImr_McuInjectionPhase(from[phaseIdx]);
        ret.append(phase);
    }

    return ret;
}

QVariantMap ImrParser::ToImr_McuInjectionProtocol(const DS_McuDef::InjectionProtocol &from)
{
    QVariantMap ret;

    ret.insert("PressureLimit", from.pressureLimit);
    ret.insert("TwoContrastInjectPhaseIndex", from.twoContrastInjectPhaseIndex);
    ret.insert("Phases", ToImr_McuInjectionPhases(from.phases));

    return ret;
}

QVariantMap ImrParser::ToImr_McuInjectorStatus(const DS_McuDef::InjectorStatus &from)
{
    QVariantMap ret;

    ret.insert("State", ToImr_InjectorState(from.state));
    ret.insert("CompleteStatus", ToImr_InjectionCompleteStatus(from.completeStatus));

    return ret;
}

QVariantList ImrParser::ToImr_SyringeAirCheckCalDigests(const DS_McuDef::SyringeAirCheckCalDigests &from, QString *err)
{
    QString errStr = "";
    QVariantList ret;

    for (int digestIdx = 0; digestIdx < from.length(); digestIdx++)
    {
        QVariantMap digest = ToImr_SyringeAirCheckCalData(from[digestIdx], &errStr);
        if (errStr != "")
        {
            errStr = QString().asprintf("SyringeAirCheckCalData[%d]: ", digestIdx) + errStr;
        }
        ret.append(digest);
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_SyringeAirCheckCalData(const DS_McuDef::SyringeAirCheckCalData &from, QString *err)
{
    QString errStr = "";
    QVariantMap ret;

    ret.insert("Displacement", from.displacement);
    ret.insert("PressureKpa", from.pressureKpa);

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantList ImrParser::ToImr_SyringeAirCheckCoeffDigests(const DS_McuDef::SyringeAirCheckCoeffDigests &from, QString *err)
{
    QString errStr = "";
    QVariantList ret;

    for (int digestIdx = 0; digestIdx < from.length(); digestIdx++)
    {
        QVariantMap digest = ToImr_SyringeAirCheckCoeff(from[digestIdx], &errStr);
        if (errStr != "")
        {
            errStr = QString().asprintf("SyringeAirCheckCoeff[%d]: ", digestIdx) + errStr;
        }
        ret.append(digest);
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_SyringeAirCheckCoeff(const DS_McuDef::SyringeAirCheckCoeff &from, QString *err)
{
    QString errStr = "";
    QVariantMap ret;

    ret.insert("Slope", from.slope);
    ret.insert("Intercept", from.intercept);
    ret.insert("State", from.state);

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantList ImrParser::ToImr_SyringeAirCheckDigests(const DS_McuDef::SyringeAirCheckDigests &from, QString *err)
{
    QString errStr = "";
    QVariantList ret;

    for (int digestIdx = 0; digestIdx < from.length(); digestIdx++)
    {
        QVariantMap digest = ToImr_SyringeAirCheckDigest(from[digestIdx], &errStr);
        if (errStr != "")
        {
            errStr = QString().asprintf("SyringeAirCheckDigest[%d]: ", digestIdx) + errStr;
        }
        ret.append(digest);
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_SyringeAirCheckDigest(const DS_McuDef::SyringeAirCheckDigest &from, QString *err)
{
    QString errStr = "";
    QVariantMap ret;

    ret.insert("AirVolume", from.airVolume);
    ret.insert("AirVolume2", from.airVolume2);

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantList ImrParser::ToImr_PressureCalCoeffDigests(const DS_McuDef::PressureCalCoeffDigests &from, QString *err)
{
    QString errStr = "";
    QVariantList ret;

    for (int digestIdx = 0; digestIdx < from.length(); digestIdx++)
    {
        QVariantMap digest = ToImr_PressureCalCoeff(from[digestIdx], &errStr);
        if (errStr != "")
        {
            errStr = QString().asprintf("PressureCalCoeff[%d]: ", digestIdx) + errStr;
        }
        ret.append(digest);
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_PressureCalCoeff(const DS_McuDef::PressureCalCoeff &from, QString *err)
{
    QString errStr = "";
    QVariantMap ret;

    ret.insert("Coeff0", from.coeff0);
    ret.insert("Coeff1", from.coeff1);
    ret.insert("Coeff2", from.coeff2);
    ret.insert("Coeff3", from.coeff3);
    ret.insert("Coeff4", from.coeff4);
    ret.insert("Coeff5", from.coeff5);
    ret.insert("State", from.state);

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_BMSDigestParamsDeviceInfo(const DS_McuDef::BMSDigestParamsDeviceInfo &from, QString *err)
{
    QString errStr = "";
    QVariantMap ret;

    ret.insert("DeviceNumber", from.deviceNumber);
    ret.insert("FirmwareVersion", from.firmwareVersion);
    ret.insert("BuildNumber", from.buildNumber);
    ret.insert("CEDVVersion", from.cedvVersion);
    ret.insert("DeviceName", from.deviceName);

    if (err != NULL)
    {
        *err = errStr;
    }

    return ret;
}

QVariantMap ImrParser::ToImr_BMSDigestParamsSBSStatus(const DS_McuDef::BMSDigestParamsSBSStatus &from, QString *err)
{
    QString errStr = "";
    QVariantMap ret;

    ret.insert("Temperature (°C)", from.temperature);
    ret.insert("Current (mA)", from.current);
    ret.insert("MaxError (%)", from.maxError);
    ret.insert("RelativeStateOfCharge (%)", from.relativeStateOfCharge);
    ret.insert("RunTimeToEmpty (min)", from.runTimeToEmpty);
    ret.insert("AveTimeToEmpty (min)", from.aveTimeToEmpty);
    ret.insert("AveTimeToFull (min)", from.aveTimeToFull);

    ret.insert("BatteryStatus", QString().asprintf("%04x", from.batteryStatus));

    // Below breaks Battery Status bits down to more meaningfull data
    ret.insert("BatteryStatus : Overcharge Alarm", QString().asprintf("%s", ((from.overchargeAlarm)?"True":"False")));
    ret.insert("BatteryStatus : Terminate Charge Alarm", QString().asprintf("%s", ((from.terminateChargeAlarm)?"True":"False")));
    ret.insert("BatteryStatus : Over temperature Alarm", QString().asprintf("%s", ((from.overTemperatureAlarm)?"True":"False")));
    ret.insert("BatteryStatus : Terminate Discharge Alarm", QString().asprintf("%s", ((from.terminateDischargeAlarm)?"True":"False")));
    ret.insert("BatteryStatus : Fully Charged", QString().asprintf("%s", ((from.fullyCharged)?"True":"False")));
    ret.insert("BatteryStatus : Fully Discharged", QString().asprintf("%s", ((from.fullyDischarged)?"True":"False")));
    ret.insert("BatteryStatus : Error Code (Bit 3-0)", QString().asprintf("%04x", (from.errorCode & 0x000F)));

    ret.insert("CycleCount (cycles)", from.cycleCount);
    ret.insert("ManufactureDate", from.manufactureDate);
    ret.insert("SerialNumber", QString().asprintf("%04x", from.serialNumber));
    ret.insert("HostFETControl", QString().asprintf("%04x", from.hostFETControl));
    for (int vIdx = 0; vIdx < ARRAY_LEN(from.cellVoltages); vIdx++)
    {
        ret.insert(QString().asprintf("CellVoltage%02d (mV)", vIdx + 1), from.cellVoltages[vIdx]);
    }
    ret.insert("StateOfHealth (%)", from.stateOfHealth);

    if (err != NULL)
    {
        *err = errStr;
    }

    return ret;
}

QVariantMap ImrParser::ToImr_BMSDigestParamsSafetyStatus(const DS_McuDef::BMSDigestParamsSafetyStatus &from, QString *err)
{
    QString errStr = "";
    QVariantMap ret;

    ret.insert("Overcharged", from.overcharged);
    ret.insert("ChargeTimeout", from.chargeTimeout);
    ret.insert("PrechargeTimeout", from.prechargeTimeout);
    ret.insert("UnderTemperatureDuringDischarge", from.underTemperatureDuringDischarge);
    ret.insert("UnderTemperatureDuringCharge", from.underTemperatureDuringCharge);
    ret.insert("OverTemperatureDuringDischarge", from.overTemperatureDuringDischarge);
    ret.insert("OverTemperatureDuringCharge", from.overTemperatureDuringCharge);
    ret.insert("ShortCircuitDuringDischargeLatch", from.shortCircuitDuringDischargeLatch);
    ret.insert("ShortCircuitDuringDischarge", from.shortCircuitDuringDischarge);
    ret.insert("OverloadDuringDischargeLatch", from.overloadDuringDischargeLatch);
    ret.insert("OverloadDuringDischarge", from.overloadDuringDischarge);
    ret.insert("OverCurrentDuringDischarge", from.overCurrentDuringDischarge);
    ret.insert("OverCurrentDuringCharge", from.overCurrentDuringCharge);
    ret.insert("CellOverVoltage", from.cellOverVoltage);
    ret.insert("CellUnderVoltage", from.cellUnderVoltage);

    if (err != NULL)
    {
        *err = errStr;
    }

    return ret;
}

QVariantMap ImrParser::ToImr_BMSDigestParamsPfStatus(const DS_McuDef::BMSDigestParamsPfStatus &from, QString *err)
{
    QString errStr = "";
    QVariantMap ret;

    ret.insert("DataFlashWearoutFailure", from.dataFlashWearoutFailure);
    ret.insert("InstructionFlashCehcksumFailure", from.instructionFlashCehcksumFailure);
    ret.insert("SafetyOvertemperatureFETFailure", from.safetyOvertemperatureFETFailure);
    ret.insert("OpenThermistorTS3Failure", from.openThermistorTS3Failure);
    ret.insert("OpenThermistorTS2Failure", from.openThermistorTS2Failure);
    ret.insert("OpenThermistorTS1Failure", from.openThermistorTS1Failure);
    ret.insert("CompanionAfeXReadyFailure", from.companionAfeXReadyFailure);
    ret.insert("CompanionAfeOveride", from.companionAfeOveride);
    ret.insert("AfeCommunicationFailure", from.afeCommunicationFailure);
    ret.insert("AfeRegisterFailure", from.afeRegisterFailure);
    ret.insert("DischargeFETFailure", from.dischargeFETFailure);
    ret.insert("ChargeFETFailure", from.chargeFETFailure);
    ret.insert("VoltageImbalanceWhilePackRestFailure", from.voltageImbalanceWhilePackRestFailure);
    ret.insert("SafetyOvertemperatureCellFailure", from.safetyOvertemperatureCellFailure);
    ret.insert("SafetyOvercurrentInDischarge", from.safetyOvercurrentInDischarge);
    ret.insert("SafetyOvercurrentInCharge", from.safetyOvercurrentInCharge);
    ret.insert("SafetyCellOvervoltageFailure", from.safetyCellOvervoltageFailure);
    ret.insert("SafetyCellUndervoltageFailure", from.safetyCellUndervoltageFailure);

    if (err != NULL)
    {
        *err = errStr;
    }

    return ret;
}

QVariantMap ImrParser::ToImr_BMSDigestParamsOperationStatus(const DS_McuDef::BMSDigestParamsOperationStatus &from, QString *err)
{
    QString errStr = "";
    QVariantMap ret;

    ret.insert("CellBalancingStatus", from.cellBalancingStatus);
    ret.insert("CcMeasurementInSleepMode", from.ccMeasurementInSleepMode);
    ret.insert("AdcMeasurementInSleepMode", from.adcMeasurementInSleepMode);
    ret.insert("InitializationAfterFullReset", from.initializationAfterFullReset);
    ret.insert("SleepMode", from.sleepMode);
    ret.insert("ChargingDisabled", from.chargingDisabled);
    ret.insert("DischargingDisabled", from.dischargingDisabled);
    ret.insert("PermanentFailureModeActive", from.permanentFailureModeActive);
    ret.insert("SafetyModeActive", from.safetyModeActive);
    ret.insert("ShutdownTriggeredViaLowPackVoltage", from.shutdownTriggeredViaLowPackVoltage);
    ret.insert("SecurityMode", from.securityMode);
    ret.insert("SafePinActive", from.safePinActive);
    ret.insert("IsUnderHostFETControl", from.isUnderHostFETControl);
    ret.insert("PrechargeFETActive", from.prechargeFETActive);
    ret.insert("DsgFETActive", from.dsgFETActive);
    ret.insert("ChgFETActive", from.chgFETActive);
    ret.insert("SystemPresentLowActive", from.systemPresentLowActive);

    if (err != NULL)
    {
        *err = errStr;
    }

    return ret;
}

QVariantMap ImrParser::ToImr_BMSDigestParamsChargingStatus(const DS_McuDef::BMSDigestParamsChargingStatus &from, QString *err)
{
    QString errStr = "";
    QVariantMap ret;

    ret.insert("OverTemperatureRegion", from.overTemperatureRegion);
    ret.insert("HighTemperatureRegion", from.highTemperatureRegion);
    ret.insert("StandardTemperatureRegion", from.standardTemperatureRegion);
    ret.insert("LowTemperatureRegion", from.lowTemperatureRegion);
    ret.insert("UndertemperatureRegion", from.undertemperatureRegion);
    ret.insert("ChargeTermination", from.chargeTermination);
    ret.insert("ChargeSuspend", from.chargeSuspend);
    ret.insert("ChargeInhibit", from.chargeInhibit);

    if (err != NULL)
    {
        *err = errStr;
    }

    return ret;
}

QVariantMap ImrParser::ToImr_BMSDigestParamsGaugingStatus(const DS_McuDef::BMSDigestParamsGaugingStatus &from, QString *err)
{
    QString errStr = "";
    QVariantMap ret;

    ret.insert("DischargeQualifiedForLearning", from.dischargeQualifiedForLearning);
    ret.insert("EndOfDischargeVoltageLevel2", from.endOfDischargeVoltageLevel2);
    ret.insert("EndOfDischargeVoltageLevel1", from.endOfDischargeVoltageLevel1);
    ret.insert("OcvReadingTaken", from.ocvReadingTaken);
    ret.insert("ConditionFlag", from.conditionFlag);
    ret.insert("DischargeDetected", from.dischargeDetected);
    ret.insert("EndOfDischargeVoltageLevel0", from.endOfDischargeVoltageLevel0);
    ret.insert("CellBalancingPossible", from.cellBalancingPossible);
    ret.insert("TerminateCharge", from.terminateCharge);
    ret.insert("TerminateDischarge", from.terminateDischarge);
    ret.insert("FullyCharged", from.fullyCharged);
    ret.insert("FullyDischarged", from.fullyDischarged);

    if (err != NULL)
    {
        *err = errStr;
    }

    return ret;
}

QVariantMap ImrParser::ToImr_BMSDigestParamsManufacturingStatus(const DS_McuDef::BMSDigestParamsManufacturingStatus &from, QString *err)
{
    QString errStr = "";
    QVariantMap ret;

    ret.insert("LedDisplay", from.ledDisplay);
    ret.insert("SafeAction", from.safeAction);
    ret.insert("BlackBoxReorder", from.blackBoxRecorder);
    ret.insert("PermanentFailure", from.permanentFailure);
    ret.insert("LifetimeDataCollection", from.lifetimeDataCollection);
    ret.insert("AllFetAction", from.allFetAction);

    if (err != NULL)
    {
        *err = errStr;
    }

    return ret;
}

QVariantMap ImrParser::ToImr_BMSDigest(const DS_McuDef::BMSDigest &from, QString *err)
{
    QString errStr = "";
    QVariantMap ret;

    ret.insert("FirmwareVersionStr", from.firmwareVersionStr);
    ret.insert("HardwareVersion", from.hardwareVersion);
    ret.insert("SafetyStatusBytes", from.safetyStatusBytes);
    ret.insert("PfStatusBytes", from.pfStatusBytes);
    ret.insert("OperationStatusBytes", from.operationStatusBytes);
    ret.insert("ChargingStatusBytes", from.chargingStatusBytes);
    ret.insert("GaugingStatusBytes", from.gaugingStatusBytes);
    ret.insert("ManufacturingStatusBytes", from.manufacturingStatusBytes);

    ret.insert("DeviceInfo", ToImr_BMSDigestParamsDeviceInfo(from.deviceInfo));
    ret.insert("SbsStatus", ToImr_BMSDigestParamsSBSStatus(from.sbsStatus));
    ret.insert("SafetyStatus", ToImr_BMSDigestParamsSafetyStatus(from.safetyStatus));
    ret.insert("PfStatus", ToImr_BMSDigestParamsPfStatus(from.pfStatus));
    ret.insert("OperationStatus", ToImr_BMSDigestParamsOperationStatus(from.operationStatus));
    ret.insert("ChargingStatus", ToImr_BMSDigestParamsChargingStatus(from.chargingStatus));
    ret.insert("GaugingStatus", ToImr_BMSDigestParamsGaugingStatus(from.gaugingStatus));
    ret.insert("ManufacturingStatus", ToImr_BMSDigestParamsManufacturingStatus(from.manufacturingStatus));

    if (err != NULL)
    {
        *err = errStr;
    }

    return ret;
}

QVariantList ImrParser::ToImr_BMSDigests(const DS_McuDef::BMSDigests &from, QString *err)
{
    QString errStr = "";
    QVariantList ret;

    for (int digestIdx = 0; digestIdx < from.length(); digestIdx++)
    {
        QVariantMap digest = ToImr_BMSDigest(from[digestIdx], &errStr);
        if (errStr != "")
        {
            errStr = QString().asprintf("BMSDigests[%d]: ", digestIdx) + errStr;
        }
        ret.append(digest);
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_DicomField(const DS_MwlDef::DicomField &from, QString *err)
{
    QString errStr = "";
    QVariantMap ret;

    ret.insert("Name", from.name);
    ret.insert("Value", from.value);
    ret.insert("DicomValueType", from.dicomValueType);
    ret.insert("ValueType", from.valueType);
    ret.insert("TranslateName", from.translateName);
    ret.insert("TranslateValue", from.translateValue);

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantList ImrParser::ToImr_DicomFields(const DS_MwlDef::DicomFields &from, QString *err)
{
    QString errStr = "";
    QVariantList ret;

    for (int fieldIdx = 0; fieldIdx < from.length(); fieldIdx++)
    {
        QVariantMap field = ToImr_DicomField(from[fieldIdx], &errStr);
        if (errStr != "")
        {
            errStr = QString().asprintf("DicomFields[%d]: ", fieldIdx) + errStr;
        }
        ret.append(field);
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_DicomFieldMap(const DS_MwlDef::DicomFieldMap &from, QString *err)
{
    QString errStr = "";
    QVariantMap ret;

    QMap<QString, DS_MwlDef::DicomField>::const_iterator i = from.begin();

    i = from.begin();
    while (i != from.end())
    {
        QString key = i.key();
        DS_MwlDef::DicomField dicomFieldCpp = i.value();
        QVariantMap dicomField = ToImr_DicomField(dicomFieldCpp, &errStr);
        if (errStr != "")
        {
            errStr = QString().asprintf("DicomFieldMap[%s]: ", key.CSTR()) + errStr;
        }
        ret.insert(key, dicomField);
        i++;
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_WorklistEntry(const DS_MwlDef::WorklistEntry &from, QString *err)
{
    QString errStr = "";
    QVariantMap ret;

    ret.insert("StudyInstanceUid", from.studyInstanceUid);
    ret.insert("DicomFields", ToImr_DicomFieldMap(from.dicomFields, &errStr));
    ret.insert("IsAnonymous", from.isAnonymous);

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantList ImrParser::ToImr_WorklistEntries(const DS_MwlDef::WorklistEntries &from, QString *err)
{
    QString errStr = "";
    QVariantList ret;

    for (int entryIdx = 0; entryIdx < from.length(); entryIdx++)
    {
        QVariantMap entry = ToImr_WorklistEntry(from[entryIdx], &errStr);
        if (errStr != "")
        {
            errStr = QString().asprintf("WorklistEntries[%d]: ", entryIdx) + errStr;
        }
        ret.append(entry);
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_NetworkSettingParams(const DS_SystemDef::NetworkSettingParams &from, QString *err)
{
    QString errStr = "";
    QVariantMap ret;

    ret.insert("IsWifiType", from.isWifiType);
    ret.insert("IsDhcpMode", from.isDhcpMode);
    ret.insert("ActiveIface", from.activeIface);
    ret.insert("InactiveIface", from.inactiveIface);
    ret.insert("Netmask", from.netmask);
    ret.insert("LocalIp", from.localIp);
    ret.insert("ServerIp", from.serverIp);
    ret.insert("RouterIp", from.routerIp);

    if (from.isWifiType)
    {
        ret.insert("Ssid", from.ssid);
        ret.insert("Pwd", from.pwd);
        ret.insert("CountryCode", from.countryCode);
    }

    if (from.setupCompletedEpochMs != -1)
    {
        ret.insert("SetupCompletedAt", Util::qDateTimeToUtcDateTimeStr(QDateTime::fromMSecsSinceEpoch(from.setupCompletedEpochMs)));
    }

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_WorkflowBatteryStatus(const DS_WorkflowDef::WorkflowBatteryStatus &from, QString *msg)
{
    QVariantMap ret;
    QString message = "";

    ret.insert("State", ToImr_WorkflowBatteryState(from.state));
    ret.insert("Message", from.message);

    if (msg != NULL)
    {
        *msg = message;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_ShippingModeStatus(const DS_WorkflowDef::ShippingModeStatus &from, QString *msg)
{
    QVariantMap ret;
    QString message = "";

    ret.insert("State", ToImr_ShippingModeState(from.state));
    ret.insert("Message", from.message);

    if (msg != NULL)
    {
        *msg = message;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_ManualQualifiedDischargeStatus(const DS_WorkflowDef::ManualQualifiedDischargeStatus &from, QString *msg)
{
    QVariantMap ret;
    QString message = "";

    ret.insert("State", ToImr_ManualQualifiedDischargeState(from.state));
    ret.insert("Message", from.message);

    if (msg != NULL)
    {
        *msg = message;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_AutomaticQualifiedDischargeStatus(const DS_WorkflowDef::AutomaticQualifiedDischargeStatus &from, QString *msg)
{
    QVariantMap ret;
    QString message = "";

    ret.insert("State", ToImr_AutomaticQualifiedDischargeState(from.state));
    ret.insert("Message", from.message);

    if (msg != NULL)
    {
        *msg = message;
    }
    return ret;
}

QVariantMap ImrParser::ToImr_Iwconfig(const DS_SystemDef::IwconfigParams &from, QString *err)
{
    QString errStr = "";
    QVariantMap ret;

    ret.insert("SamplePeriodSeconds", from.samplePeriodSeconds);
    ret.insert("SampleWindowSeconds", from.sampleWindowSeconds);

    ret.insert("LinkQuality", from.linkQualityAvg);
    ret.insert("LinkQualityMin", from.linkQualityMin);
    ret.insert("LinkQualityMax", from.linkQualityMax);

    ret.insert("SignalLevel", from.signalLevelAvg);
    ret.insert("SignalLevelMin", from.signalLevelMin);
    ret.insert("SignalLevelMax", from.signalLevelMax);

    ret.insert("NoiseLevel", from.noiseLevelAvg);
    ret.insert("NoiseLevelMin", from.noiseLevelMin);
    ret.insert("NoiseLevelMax", from.noiseLevelMax);

    if (err != NULL)
    {
        *err = errStr;
    }
    return ret;
}

