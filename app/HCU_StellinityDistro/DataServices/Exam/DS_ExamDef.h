#ifndef DS_EXAM_DEF_H
#define DS_EXAM_DEF_H

#include "Common/Util.h"
#include "Common/HwCapabilities.h"
#include "Common/Config.h"
#include "DataServices/Device/DS_DeviceDef.h"
#include "DataServices/System/DS_SystemDef.h"
#include "DataServices/Mwl/DS_MwlDef.h"
#include "Common/Config.h"

class DS_ExamDef
{
public:
    // ==============================================
    // Enumerations
    enum ScannerInterfaceStatus
    {
        SCANNER_INTERFACE_STATUS_UNKNOWN = 0,
        SCANNER_INTERFACE_STATUS_DISABLED_PERMANENTLY,
        SCANNER_INTERFACE_STATUS_ENABLED,
        SCANNER_INTERFACE_STATUS_ACTIVE,
        SCANNER_INTERFACE_STATUS_DISABLED_TEMPORARILY,
        SCANNER_INTERFACE_STATUS_WAITING
    };

    enum ScannerInterfaceMode
    {
        SCANNER_INTERFACE_MODE_UNKNOWN = 0,
        SCANNER_INTERFACE_MODE_MONITOR,
        SCANNER_INTERFACE_MODE_TRACKING,
        SCANNER_INTERFACE_MODE_CONTROL,
        SCANNER_INTERFACE_MODE_SYNCHRONIZATION
    };

    enum StepTerminationReason
    {
        STEP_TERMINATION_REASON_INVALID = 0,
        STEP_TERMINATION_REASON_UNKNOWN,
        STEP_TERMINATION_REASON_NORMAL,
        STEP_TERMINATION_REASON_ABORTED_BY_REQUEST,
        STEP_TERMINATION_REASON_ABORTED_BY_OVER_PRESSURE,
        STEP_TERMINATION_REASON_ABORTED_BY_OTHER_INTERNAL,
        STEP_TERMINATION_REASON_ABORTED_BY_INJECTOR_COMM_LOSS,
        STEP_TERMINATION_REASON_ABORTED_BY_INJECTOR_CRITICAL_ERROR,
        STEP_TERMINATION_REASON_ABORTED_BY_STALLING,
        STEP_TERMINATION_REASON_ABORTED_BY_HOLD_TIMEOUT,
        STEP_TERMINATION_REASON_ABORTED_BY_SCANNER,
        STEP_TERMINATION_REASON_ABORTED_BY_AIR_DETECTION,
        STEP_TERMINATION_REASON_ABORTED_BY_DISPOSABLE_REMOVAL,
        STEP_TERMINATION_REASON_ABORTED_BY_CRITICAL_BATTERY_LEVEL,
        STEP_TERMINATION_REASON_ABORTED_BY_HARDWARE_FAULT
    };

    enum InjectionPhaseType
    {
        INJECTION_PHASE_TYPE_INVALID = 0,
        INJECTION_PHASE_TYPE_UNKNOWN,
        INJECTION_PHASE_TYPE_DELAY,
        INJECTION_PHASE_TYPE_FLUID
    };

    enum InjectionPhaseProgressState
    {
        INJECTION_PHASE_PROGRESS_STATE_INVALID = 0,
        INJECTION_PHASE_PROGRESS_STATE_UNKNOWN,
        INJECTION_PHASE_PROGRESS_STATE_PROGRESS,
        INJECTION_PHASE_PROGRESS_STATE_JUMPED,
        INJECTION_PHASE_PROGRESS_STATE_ABORTED,
        INJECTION_PHASE_PROGRESS_STATE_COMPLETED
    };

    enum ArmType
    {
        ARM_TYPE_UNKNOWN = 0,
        ARM_TYPE_NORMAL,
        ARM_TYPE_PRELOAD_FIRST,
        ARM_TYPE_PRELOAD_SECOND
    };

    enum TimedReminderDisplayHint
    {
        TIMED_REMINDER_DISPLAY_HINT_INVALID = 0,
        TIMED_REMINDER_DISPLAY_HINT_UNKNOWN,
        TIMED_REMINDER_DISPLAY_HINT_COUNT_UP,
        TIMED_REMINDER_DISPLAY_HINT_COUNT_DOWN,
        TIMED_REMINDER_DISPLAY_HINT_VISUAL,
        TIMED_REMINDER_DISPLAY_HINT_AUDIBLE,
        TIMED_REMINDER_DISPLAY_HINT_FOREGROUND_COUNT,
        TIMED_REMINDER_DISPLAY_HINT_BACKGROUND_COUNT
    };

    enum ExamProgressState
    {
        EXAM_PROGRESS_STATE_INVALID = 0,
        EXAM_PROGRESS_STATE_IDLE,
        EXAM_PROGRESS_STATE_PREPARED, // action
        EXAM_PROGRESS_STATE_PATIENT_SELECTION,
        EXAM_PROGRESS_STATE_PROTOCOL_SELECTION,
        EXAM_PROGRESS_STATE_PROTOCOL_MODIFICATION,
        EXAM_PROGRESS_STATE_STARTED, // action
        EXAM_PROGRESS_STATE_INJECTION_EXECUTION,
        EXAM_PROGRESS_STATE_SUMMARY_CONFIRMATION,
        EXAM_PROGRESS_STATE_COMPLETING,
        EXAM_PROGRESS_STATE_COMPLETED // action
    };

    enum AdaptiveFlowState
    {
        ADAPTIVE_FLOW_STATE_OFF,
        ADAPTIVE_FLOW_STATE_ACTIVE,
        ADAPTIVE_FLOW_STATE_CRITICAL
    };

    enum InjectionPlanDigestState
    {
        INJECTION_PLAN_DIGEST_STATE_UNKNOWN = 0,
        INJECTION_PLAN_DIGEST_STATE_INITIALISING,
        INJECTION_PLAN_DIGEST_STATE_BAD_DATA,
        INJECTION_PLAN_DIGEST_STATE_READY
    };

    enum InjectionPersonalizationInputSource
    {
        INJECTION_PLAN_CUSTOM_PARAM_SOURCE_NONE = 0,
        INJECTION_PLAN_CUSTOM_PARAM_SOURCE_ENTRY,
        INJECTION_PLAN_CUSTOM_PARAM_SOURCE_CURRENT,
        INJECTION_PLAN_CUSTOM_PARAM_SOURCE_DEFAULT
    };

    enum InjectionPersonalizationNoticeType
    {
        INJECTION_STEP_CUSTOM_NOTICE_UNKNOWN = 0,
        INJECTION_STEP_CUSTOM_NOTICE_MAXIMUM_IODINE_LOAD_LIMIT_APPLIED,
        INJECTION_STEP_CUSTOM_NOTICE_MAXIMUM_CONTRAST_VOLUME_LIMIT_APPLIED,
        INJECTION_STEP_CUSTOM_NOTICE_MAXIMUM_SALINE_VOLUME_LIMIT_APPLIED,
        INJECTION_STEP_CUSTOM_NOTICE_MAXIMUM_FLOW_RATE_LIMIT_APPLIED,
        INJECTION_STEP_CUSTOM_NOTICE_MINIMUM_IODINE_LOAD_LIMIT_APPLIED,
        INJECTION_STEP_CUSTOM_NOTICE_MINIMUM_FLOW_RATE_LIMIT_APPLIED,
        INJECTION_STEP_CUSTOM_NOTICE_MINIMUM_CONTRAST_VOLUME_LIMIT_APPLIED,
        INJECTION_STEP_CUSTOM_NOTICE_MAXIMUM_IODINE_LOAD_LIMIT_IDR_DECREASED,
        INJECTION_STEP_CUSTOM_NOTICE_MAXIMUM_FLOW_RATE_LIMIT_IDR_DECREASED,
        INJECTION_STEP_CUSTOM_NOTICE_MINIMUM_IODINE_LOAD_LIMIT_IDR_INCREASED,
        INJECTION_STEP_CUSTOM_NOTICE_MINIMUM_FLOW_RATE_LIMIT_IDR_INCREASED,
        INJECTION_STEP_CUSTOM_NOTICE_MIN_MAX_IODINE_LOAD_FLOW_RATE_CONFLICT,
        INJECTION_STEP_CUSTOM_NOTICE_EGFR_EQUATION_NOT_VALID_WITH_IDMS,
        INJECTION_STEP_CUSTOM_NOTICE_EGFR_EQUATION_ONLY_VALID_WITH_IDMS,
        INJECTION_STEP_CUSTOM_NOTICE_EGFR_MEASUREMENT_OLDER_THAN_LIMIT,
        INJECTION_STEP_CUSTOM_NOTICE_INJECTION_VOLUMES_DECREASED_BY_KRULESET,
        INJECTION_STEP_CUSTOM_NOTICE_INJECTION_VOLUMES_INCREASED_BY_KRULESET,
        INJECTION_STEP_CUSTOM_NOTICE_INJECTION_VOLUMES_UNAFFECTED_BY_KRULESET,
        INJECTION_STEP_CUSTOM_NOTICE_KVP_NOT_CONFIGURED_IN_KRULESET,
        INJECTION_STEP_CUSTOM_NOTICE_FLOW_RATE_REDUCED_BY_CATHETER_TYPE_LIMIT,
        INJECTION_STEP_CUSTOM_NOTICE_FLOW_RATE_REDUCED_BY_CATHETER_PLACEMENT_LIMIT,
        INJECTION_STEP_CUSTOM_NOTICE_FLOW_RATE_EXCEEDS_CATHETER_TYPE_LIMIT,
        INJECTION_STEP_CUSTOM_NOTICE_FLOW_RATE_EXCEEDS_CATHETER_PLACEMENT_LIMIT,
        INJECTION_STEP_CUSTOM_NOTICE_PRESSURE_LIMIT_REDUCED_BY_CATHETER_TYPE_LIMIT,
        INJECTION_STEP_CUSTOM_NOTICE_PRESSURE_LIMIT_EXCEEDS_CATHETER_TYPE_LIMIT,
        INJECTION_STEP_CUSTOM_NOTICE_FLOW_RATE_INCREASED_BY_INJECTOR_CONTEXT_AND_DURATION_MAINTAINED,
        INJECTION_STEP_CUSTOM_NOTICE_FLOW_RATE_INCREASED_BY_INJECTOR_CONTEXT_AND_VOLUME_MAINTAINED,
        INJECTION_STEP_CUSTOM_NOTICE_FLOW_RATE_DECREASED_BY_INJECTOR_CONTEXT_AND_DURATION_MAINTAINED,
        INJECTION_STEP_CUSTOM_NOTICE_FLOW_RATE_DECREASED_BY_INJECTOR_CONTEXT_AND_VOLUME_MAINTAINED,
        INJECTION_STEP_CUSTOM_NOTICE_CONTRAST_VOLUME_DECREASED_BY_INJECTOR_CONTEXT_AND_DURATION_MAINTAINED,
        INJECTION_STEP_CUSTOM_NOTICE_CONTRAST_VOLUME_DECREASED_BY_INJECTOR_CONTEXT,
        INJECTION_STEP_CUSTOM_NOTICE_CONTRAST_VOLUME_INCREASED_BY_INJECTOR_CONTEXT_AND_DURATION_MAINTAINED,
        INJECTION_STEP_CUSTOM_NOTICE_CONTRAST_VOLUME_INCREASED_BY_INJECTOR_CONTEXT,
        INJECTION_STEP_CUSTOM_NOTICE_SALINE_VOLUME_DECREASED_BY_INJECTOR_CONTEXT_AND_DURATION_MAINTAINED,
        INJECTION_STEP_CUSTOM_NOTICE_SALINE_VOLUME_DECREASED_BY_INJECTOR_CONTEXT,
        INJECTION_STEP_CUSTOM_NOTICE_SALINE_VOLUME_INCREASED_BY_INJECTOR_CONTEXT_AND_DURATION_MAINTAINED,
        INJECTION_STEP_CUSTOM_NOTICE_SALINE_VOLUME_INCREASED_BY_INJECTOR_CONTEXT
    };

    enum InjectionPersonalizationNoticeImportance
    {
        INJECTION_STEP_CUSTOM_NOTICE_SEVERITY_UNKNOWN = 0,
        INJECTION_STEP_CUSTOM_NOTICE_SEVERITY_INFO,
        INJECTION_STEP_CUSTOM_NOTICE_SEVERITY_WARNING,
        INJECTION_STEP_CUSTOM_NOTICE_SEVERITY_ERROR
    };

    // ==============================================
    // Data Structures
    struct InjectionRequestProcessStatus
    {
        bool requestedByHcu;
        QString state;

        InjectionRequestProcessStatus()
        {
            requestedByHcu = true;
            state = "UNKNOWN";
        }

        bool operator==(const InjectionRequestProcessStatus &arg) const
        {
            return ( (requestedByHcu == arg.requestedByHcu) &&
                     (state == arg.state) );
        }

        bool operator!=(const InjectionRequestProcessStatus &arg) const
        {
            return !operator==(arg);
        }
    };

    struct ScannerInterlocks
    {
        bool configLockedOut;
        bool armLockedOut;
        bool startLockedOut;
        bool resumeLockedOut;
        bool scannerReady;
        ScannerInterfaceStatus interfaceStatus;
        ScannerInterfaceMode interfaceMode;

        ScannerInterlocks()
        {
            configLockedOut = false;
            armLockedOut = false;
            resumeLockedOut = false;
            startLockedOut = false;
            scannerReady = false;
            interfaceMode = DS_ExamDef::SCANNER_INTERFACE_MODE_UNKNOWN;
            interfaceStatus = DS_ExamDef::SCANNER_INTERFACE_STATUS_UNKNOWN;
        }        

        bool operator==(const ScannerInterlocks &arg) const
        {
            return ( (configLockedOut == arg.configLockedOut) &&
                     (armLockedOut == arg.armLockedOut) &&
                     (startLockedOut == arg.startLockedOut) &&
                     (resumeLockedOut == arg.resumeLockedOut) &&
                     (scannerReady == arg.scannerReady) &&
                     (interfaceStatus == arg.interfaceStatus) &&
                     (interfaceMode == arg.interfaceMode) );
        }

        bool operator!=(const ScannerInterlocks &arg) const
        {
            return !operator==(arg);
        }

        bool isIsiActive() const
        {
            return ( (interfaceStatus == DS_ExamDef::SCANNER_INTERFACE_STATUS_ENABLED) ||
                     (interfaceStatus == DS_ExamDef::SCANNER_INTERFACE_STATUS_ACTIVE) ||
                     (interfaceStatus == DS_ExamDef::SCANNER_INTERFACE_STATUS_WAITING) );
        }
    };

    struct InjectedVolume
    {
        DS_DeviceDef::FluidSourceIdx fluidLocation;
        double volume;

        bool operator==(const InjectedVolume &arg) const
        {
            return ( (fluidLocation == arg.fluidLocation) &&
                     (volume == arg.volume) );
        }

        bool operator!=(const InjectedVolume &arg) const
        {
            return !operator==(arg);
        }
    };

    struct InstantaneousRate
    {
        DS_DeviceDef::FluidSourceIdx fluidLocation;
        double flow;

        bool operator==(const InstantaneousRate &arg) const
        {
            return ( (fluidLocation == arg.fluidLocation) &&
                     (flow == arg.flow) );
        }

        bool operator!=(const InstantaneousRate &arg) const
        {
            return !operator==(arg);
        }
    };

    struct FluidVolumes
    {
        QList<double> volumes;

        bool operator==(const FluidVolumes &arg) const
        {
            return (volumes == arg.volumes);
        }

        bool operator!=(const FluidVolumes &arg) const
        {
            return !operator==(arg);
        }

        FluidVolumes()
        {
            init();
        }

        void init()
        {
            volumes.clear();
            for (int syringeIdx = 0; syringeIdx < SYRINGE_IDX_MAX; syringeIdx++)
            {
                volumes.append(0);
            }
        }

        double total() const
        {
            double totalVol = 0;
            for (int syringeIdx = 0; syringeIdx < volumes.length(); syringeIdx++)
            {
                totalVol += volumes[syringeIdx];
            }
            return totalVol;
        }
    };

    struct FluidFlowRates
    {
        QList<double> flowRates;

        FluidFlowRates()
        {
            init();
        }

        bool operator==(const FluidFlowRates &arg) const
        {
            return (flowRates == arg.flowRates);
        }

        bool operator!=(const FluidFlowRates &arg) const
        {
            return !operator==(arg);
        }

        void init()
        {
            flowRates.clear();
            for (int syringeIdx = 0; syringeIdx < SYRINGE_IDX_MAX; syringeIdx++)
            {
                flowRates.append(0.0);
            }
        }

        double getTotal() const
        {
            double flowRateTotal = 0.0;
            for (int syringeIdx = 0; syringeIdx< flowRates.length(); syringeIdx++)
            {
                flowRateTotal += flowRates[syringeIdx];
            }
            return flowRateTotal;
        }
    };

    struct PhaseProgressDigest
    {
        quint64 elapsedMillisFromPhaseStart;
        FluidVolumes injectedVolumes;
        int maxPressure; // kpa
        double maxFlowRate; // ml/s
        InjectionPhaseProgressState state;
        bool wasFlowAdjusted;
        double progress; // 0-100%

        PhaseProgressDigest()
        {
            maxFlowRate = 0;
            maxPressure = 0;
            elapsedMillisFromPhaseStart = 0;
            wasFlowAdjusted = false;
            state = INJECTION_PHASE_PROGRESS_STATE_UNKNOWN;
            progress = 0;
        }

        bool operator==(const PhaseProgressDigest &arg) const
        {
            return ( (elapsedMillisFromPhaseStart == arg.elapsedMillisFromPhaseStart) &&
                     (injectedVolumes == arg.injectedVolumes) &&
                     (maxPressure == arg.maxPressure) &&
                     (maxFlowRate == arg.maxFlowRate) &&
                     (state == arg.state) &&
                     (wasFlowAdjusted == arg.wasFlowAdjusted) &&
                     (progress == arg.progress) );
        }

        bool operator!=(const PhaseProgressDigest &arg) const
        {
            return !operator==(arg);
        }
    };

    struct ExecutedStep
    {
        FluidFlowRates instantaneousRates;
        int phaseIndex;
        int stepIndex;
        QString programmedStep;
        AdaptiveFlowState adaptiveFlowState;
        bool hasPressureLimited;
        bool preventingBackflowSaline;
        bool hasPreventedBackflowSaline;
        QList<PhaseProgressDigest> phaseProgress;
        DS_DeviceDef::FluidSourceIdx activeSalineLocation;
        DS_DeviceDef::FluidSourceIdx activeContrastLocation;
        DS_ExamDef::StepTerminationReason terminationReason;

        // Non-IMR parameters
        QString mcuTerminatedReason; // TerminatedReason from MCU
        QString terminatedReasonMessage; // TerminatedReasonMessage from MCU OR FatalAlerts
        qint64 stepTriggeredEpochMs;
        qint64 stepCompletedEpochMs;
        qint64 stepCompleteExpectedEpochMs;
        qint64 userHoldStartedAtEpochMs;
        qint64 userHoldPerformedMs;

        ExecutedStep()
        {
            init();
        }

        void init()
        {
            programmedStep = EMPTY_GUID;
            stepIndex = -1;
            phaseIndex = -1;
            stepTriggeredEpochMs = -1;
            stepCompletedEpochMs = -1;
            stepCompleteExpectedEpochMs = -1;
            userHoldStartedAtEpochMs = -1;
            userHoldPerformedMs = -1;
            adaptiveFlowState = DS_ExamDef::ADAPTIVE_FLOW_STATE_OFF;
            hasPressureLimited = false;
            preventingBackflowSaline = false;
            hasPreventedBackflowSaline = false;
            phaseProgress.clear();
            terminationReason = DS_ExamDef::STEP_TERMINATION_REASON_INVALID;
            mcuTerminatedReason = "";
            terminatedReasonMessage = "";
            activeSalineLocation = DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_1;
            activeContrastLocation = DS_DeviceDef::FLUID_SOURCE_IDX_UNKNOWN;
            instantaneousRates.init();
        }

        bool operator==(const ExecutedStep &arg) const
        {
            return ( (instantaneousRates == arg.instantaneousRates) &&
                     (phaseIndex == arg.phaseIndex) &&
                     (stepIndex == arg.stepIndex) &&
                     (programmedStep == arg.programmedStep) &&
                     (adaptiveFlowState == arg.adaptiveFlowState) &&
                     (hasPressureLimited == arg.hasPressureLimited) &&
                     (preventingBackflowSaline == arg.preventingBackflowSaline) &&
                     (hasPreventedBackflowSaline == arg.hasPreventedBackflowSaline) &&
                     (phaseProgress == arg.phaseProgress) &&
                     (activeSalineLocation == arg.activeSalineLocation) &&
                     (activeContrastLocation == arg.activeContrastLocation) &&
                     (terminationReason == arg.terminationReason) &&
                     (mcuTerminatedReason == arg.mcuTerminatedReason) &&
                     (terminatedReasonMessage == arg.terminatedReasonMessage) &&
                     (stepTriggeredEpochMs == arg.stepTriggeredEpochMs) &&
                     (stepCompletedEpochMs == arg.stepCompletedEpochMs) &&
                     (stepCompleteExpectedEpochMs == arg.stepCompleteExpectedEpochMs) &&
                     (userHoldStartedAtEpochMs == arg.userHoldStartedAtEpochMs) &&
                     (userHoldPerformedMs == arg.userHoldPerformedMs) );
        }

        bool operator!=(const ExecutedStep &arg) const
        {
            return !operator==(arg);
        }
    };

    typedef QList<ExecutedStep> ExecutedSteps;

    struct Reminder
    {
        QString name;
        QString description;
        qint64 postStepTriggerDelayMs;
        bool notifyExternalSystem;
        bool startAfterStepCompletes;
        QList<TimedReminderDisplayHint> displayHints;

        // Non-IMR parameters
        bool handled;

        bool operator==(const Reminder &arg) const
        {
            return ( (name == arg.name) &&
                     (description == arg.description) &&
                     (postStepTriggerDelayMs == arg.postStepTriggerDelayMs) &&
                     (notifyExternalSystem == arg.notifyExternalSystem) &&
                     (startAfterStepCompletes == arg.startAfterStepCompletes) &&
                     (displayHints == arg.displayHints) &&
                     (handled == arg.handled) );
        }

        bool operator!=(const Reminder &arg) const
        {
            return !operator==(arg);
        }
    };

    typedef QList<Reminder> Reminders;


    struct InjectionPersonalizationInput
    {
        QString name;
        QVariant value;
        QString units;
        InjectionPersonalizationInputSource source;
        QString validDataType;
        Config::ValidRange validRange;
        QVariantList validList;
        bool isEnabled;
        bool isValueVisible;

        bool operator==(const InjectionPersonalizationInput &arg) const
        {
            return ( (name == arg.name) &&
                     (value == arg.value) &&
                     (units == arg.units) &&
                     (source == arg.source) &&
                     (validDataType == arg.validDataType) &&
                     (validRange == arg.validRange) &&
                     (validList == arg.validList) &&
                     (isEnabled == arg.isEnabled) &&
                     (isValueVisible == arg.isValueVisible) );
        }

        bool operator!=(const InjectionPersonalizationInput &arg) const
        {
            return !operator==(arg);
        }
    };

    struct InjectionPersonalizationNotice
    {
        InjectionPersonalizationNoticeType name;
        InjectionPersonalizationNoticeImportance importance;
        QVariantList values;

        bool operator==(const InjectionPersonalizationNotice &arg) const
        {
            return ( (name == arg.name) &&
                     (importance == arg.importance) &&
                     (values == arg.values) );
        }

        bool operator!=(const InjectionPersonalizationNotice &arg) const
        {
            return !operator==(arg);
        }
    };

    typedef QList<InjectionPersonalizationInput> InjectionPersonalizationInputs;
    typedef QList<InjectionPersonalizationNotice> InjectionPersonalizationNotices;

    struct SharingInformation
    {
        QString logo;
        QString url;
        QString credits;
        QString version;
        QString passcode;

        bool operator==(const SharingInformation &arg) const
        {
            return ( (logo == arg.logo) &&
                     (url == arg.url) &&
                     (credits == arg.credits) &&
                     (version == arg.version) &&
                     (passcode == arg.passcode) );
        }

        bool operator!=(const SharingInformation &arg) const
        {
            return !operator==(arg);
        }
    };

    struct InjectionPhase
    {
        InjectionPhaseType type;
        int contrastPercentage;
        double totalVol;
        quint64 durationMs;
        double flowRate;
        int originalPhaseIdx; // If step is preloaded type, this is the original phase index

        bool operator==(const InjectionPhase &arg) const
        {
            return ( (type == arg.type) &&
                     (contrastPercentage == arg.contrastPercentage) &&
                     (totalVol == arg.totalVol) &&
                     (durationMs == arg.durationMs) &&
                     (flowRate == arg.flowRate) &&
                     (originalPhaseIdx == arg.originalPhaseIdx) );
        }

        bool operator!=(const InjectionPhase &arg) const
        {
            return !operator==(arg);
        }

        bool isDualPhase() const
        {
            return (type == INJECTION_PHASE_TYPE_FLUID) && (contrastPercentage > 0) && (contrastPercentage < 100);
        }

        bool isContrastPhase() const
        {
            return (type == INJECTION_PHASE_TYPE_FLUID) && (contrastPercentage == 100);
        }

        bool isSalinePhase() const
        {
            return (type == INJECTION_PHASE_TYPE_FLUID) && (contrastPercentage == 0);
        }

        double contrastVol() const
        {
            return (totalVol * contrastPercentage * 0.01);
        }

        double salineVol() const
        {
            return (totalVol * (100 - contrastPercentage) * 0.01);
        }

        void setDeliveryDuration()
        {
            durationMs = (totalVol * 1000) / flowRate;
        }
    };

    typedef QList<InjectionPhase> InjectionPhases;

    struct InjectionPlan;
    struct InjectionStep
    {
        QString guid;
        QString name;
        bool isTestInjection;
        bool isNotScannerSynchronized; // if not, do not send step info to ISI bus
        int pressureLimit; // kPa
        qint64 programmedAtEpochMs;
        QString templateGuid;
        Reminders reminders;
        DS_DeviceDef::FluidSourceIdx contrastFluidLocation;
        DS_DeviceDef::FluidSourceIdx salineFluidLocation;
        InjectionPhases phases;
        bool isRepeated;
        InjectionPersonalizationNotices personalizationNotices;
        InjectionPersonalizationNotices notices;
        InjectionPersonalizationInputs personalizationInputs;
        QString personalizationGenerator;
        QStringList personalizationModifiers;
        bool isPreloaded;

        InjectionStep()
        {
            guid = EMPTY_GUID;
            name = "";
            isPreloaded = false;
        }

        bool operator==(const InjectionStep &arg) const
        {
            return ( (guid == arg.guid) &&
                     (name == arg.name) &&
                     (isTestInjection == arg.isTestInjection) &&
                     (isNotScannerSynchronized == arg.isNotScannerSynchronized) &&
                     (pressureLimit == arg.pressureLimit) &&
                     (programmedAtEpochMs == arg.programmedAtEpochMs) &&
                     (templateGuid == arg.templateGuid) &&
                     (reminders == arg.reminders) &&
                     (contrastFluidLocation == arg.contrastFluidLocation) &&
                     (salineFluidLocation == arg.salineFluidLocation) &&
                     (phases == arg.phases) &&
                     (isRepeated == arg.isRepeated) &&
                     (personalizationNotices == arg.personalizationNotices) &&
                     (notices == arg.notices) &&
                     (personalizationInputs == arg.personalizationInputs) &&
                     (personalizationGenerator == arg.personalizationGenerator) &&
                     (personalizationModifiers == arg.personalizationModifiers) &&
                     (isPreloaded == arg.isPreloaded) );
        }

        bool operator!=(const InjectionStep &arg) const
        {
            return !operator==(arg);
        }

        int getIndex(InjectionPlan plan) const
        {
            for (int stepIdx = 0; stepIdx < plan.steps.length(); stepIdx++)
            {
                if (plan.steps[stepIdx] == *this)
                {
                    return stepIdx;
                }
            }
            return -1;
        }

        double getDurationMsTotal() const
        {
            quint64 durationMs = 0;
            for (int phaseIdx = 0; phaseIdx < phases.length(); phaseIdx++)
            {
                durationMs += phases[phaseIdx].durationMs;
            }
            return durationMs;
        }

        double getContrastTotal() const
        {
            double fluidTotal = 0;
            for (int phaseIdx = 0; phaseIdx < phases.length(); phaseIdx++)
            {
                if (phases[phaseIdx].type == INJECTION_PHASE_TYPE_FLUID)
                {
                    fluidTotal += ((phases[phaseIdx].contrastPercentage * phases[phaseIdx].totalVol) / 100);
                }
            }
            return fluidTotal;
        }

        double getSalineTotal() const
        {
            double fluidTotal = 0;
            for (int phaseIdx = 0; phaseIdx < phases.length(); phaseIdx++)
            {
                if (phases[phaseIdx].type == INJECTION_PHASE_TYPE_FLUID)
                {
                    fluidTotal += (((100 - phases[phaseIdx].contrastPercentage) * phases[phaseIdx].totalVol) / 100);
                }
            }
            return fluidTotal;
        }

        double getFluidTotal() const
        {
            double fluidTotal = 0;
            for (int phaseIdx = 0; phaseIdx < phases.length(); phaseIdx++)
            {
                fluidTotal += phases[phaseIdx].totalVol;
            }
            return fluidTotal;
        }

    };

    typedef QList<InjectionStep> InjectionSteps;

    struct InjectionPlan
    {
        QString guid;
        QString name;
        QString templateGuid;
        QString description;
        bool isModifiedFromTemplate;
        bool isPersonalized;
        InjectionSteps steps;
        InjectionPersonalizationInputs personalizationInputs;
        QVariantMap injectionSettings;
        SharingInformation sharingInformation;
        bool isPreloadable;

        bool operator==(const InjectionPlan &arg) const
        {
            return ( (guid == arg.guid) &&
                     (name == arg.name) &&
                     (templateGuid == arg.templateGuid) &&
                     (description == arg.description) &&
                     (isModifiedFromTemplate == arg.isModifiedFromTemplate) &&
                     (isPersonalized == arg.isPersonalized) &&
                     (steps == arg.steps) &&
                     (personalizationInputs == arg.personalizationInputs) &&
                     (injectionSettings == arg.injectionSettings) &&
                     (sharingInformation == arg.sharingInformation) &&
                     (isPreloadable == arg.isPreloadable) );
        }

        bool operator!=(const InjectionPlan &arg) const
        {
            return !operator==(arg);
        }

        int getPhaseCount() const
        {
            int phaseCount = 0;
            for (int stepIdx = 0; stepIdx < steps.length(); stepIdx++)
            {
                phaseCount += steps[stepIdx].phases.length();
            }
            return phaseCount;
        }

        int getTestInjectionCount() const
        {
            int testInjectionCount = 0;
            for (int stepIdx = 0; stepIdx < steps.length(); stepIdx++)
            {
                if (steps[stepIdx].isTestInjection)
                {
                    testInjectionCount++;
                }
            }
            return testInjectionCount;
        }

        int getStepIdxFromStepGuid(QString stepGuid) const
        {
            for (int stepIdx = 0; stepIdx < steps.length(); stepIdx++)
            {
                if (steps[stepIdx].guid == stepGuid)
                {
                    return stepIdx;
                }
            }
            return -1;
        }

        void loadFromTemplate(const InjectionPlan &templatePlan)
        {
            guid = Util::newGuid();
            name = templatePlan.name;
            templateGuid = templatePlan.templateGuid;
            description = templatePlan.description;
            isModifiedFromTemplate = false;
            isPersonalized = templatePlan.isPersonalized;
            personalizationInputs = templatePlan.personalizationInputs;
            injectionSettings = templatePlan.injectionSettings;
            sharingInformation = templatePlan.sharingInformation;
            isPreloadable = templatePlan.isPreloadable;            

            // Keep current contrast fluid type and programmedAtEpochMs
            DS_DeviceDef::FluidSourceIdx contrastFluidLocation = DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_2;
            if (steps.length() > 0)
            {
                contrastFluidLocation = steps[0].contrastFluidLocation;
            }

            steps = templatePlan.steps;
            for (int stepIdx = 0; stepIdx < steps.length(); stepIdx++)
            {
                steps[stepIdx].programmedAtEpochMs = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
                steps[stepIdx].contrastFluidLocation = contrastFluidLocation;
                steps[stepIdx].guid = Util::newGuid();
                steps[stepIdx].isRepeated = false;
                steps[stepIdx].isPreloaded = false;
            }
        }

        void updateIsModifiedFromTemplated(const InjectionPlan &planTemplate, bool isCruCommsActive)
        {
            // If a plan is personalized, consider it modified always if CRU comms is down.
            // Otherwise CRU controls this flag
            if (planTemplate.isPersonalized)
            {
                if (!isCruCommsActive)
                {
                    isModifiedFromTemplate = true;
                }
                return;
            }

            isModifiedFromTemplate = false;

            if (steps.length() != planTemplate.steps.length())
            {
                isModifiedFromTemplate = true;
            }
            else
            {
                for (int stepIdx = 0; stepIdx < steps.length(); stepIdx++)
                {
                    if ( (steps[stepIdx].isTestInjection != planTemplate.steps[stepIdx].isTestInjection) ||
                         (steps[stepIdx].isNotScannerSynchronized != planTemplate.steps[stepIdx].isNotScannerSynchronized) ||
                         (steps[stepIdx].name != planTemplate.steps[stepIdx].name) ||
                         (steps[stepIdx].phases != planTemplate.steps[stepIdx].phases))
                    {
                        isModifiedFromTemplate = true;
                        break;
                    }
                }
            }
        }

        int getLastExecutedStepIndex(const ExecutedSteps &executedSteps) const
        {
            int index = executedSteps.length() - 1;
            return index;
        }

        int getNextExecuteStepIndex(const ExecutedSteps &executedSteps) const
        {
            int index = executedSteps.length();
            if ( (index < 0) || (index >= steps.length()) )
            {
                return -1;
            }
            return index;
        }

        double getContrastTotalToInject(const ExecutedSteps &executedSteps) const
        {
            double fluidTotal = 0;
            for (int stepIdx = executedSteps.length(); stepIdx < steps.length(); stepIdx++)
            {
                fluidTotal += steps[stepIdx].getContrastTotal();
            }
            return fluidTotal;
        }

        double getSalineTotalToInject(const ExecutedSteps &executedSteps) const
        {
            double fluidTotal = 0;
            for (int stepIdx = executedSteps.length(); stepIdx < steps.length(); stepIdx++)
            {
                fluidTotal += steps[stepIdx].getSalineTotal();
            }
            return fluidTotal;
        }

        double getContrastTotal() const
        {
            double fluidTotal = 0;
            for (int stepIdx = 0; stepIdx < steps.length(); stepIdx++)
            {
                fluidTotal += steps[stepIdx].getContrastTotal();
            }
            return fluidTotal;
        }

        double getSalineTotal() const
        {
            double fluidTotal = 0;
            for (int stepIdx = 0; stepIdx < steps.length(); stepIdx++)
            {
                fluidTotal += steps[stepIdx].getSalineTotal();
            }
            return fluidTotal;
        }

        InjectionStep *getExecutingStep(DS_SystemDef::StatePath statePath, const ExecutedSteps &executedSteps)
        {
            int stepIdx = -1;

            if ( (statePath == DS_SystemDef::STATE_PATH_EXECUTING) ||
                 (statePath == DS_SystemDef::STATE_PATH_BUSY_HOLDING) ||
                 (statePath == DS_SystemDef::STATE_PATH_BUSY_FINISHING) )
            {
                stepIdx = executedSteps.length() - 1;
            }
            else
            {
                stepIdx = getNextExecuteStepIndex(executedSteps);
            }

            if ( (stepIdx != -1) && (stepIdx < steps.length()) )
            {
                return &steps[stepIdx];
            }

            return NULL;
        }

        DS_DeviceDef::FluidSourceIdx getCurContrastLocation(DS_SystemDef::StatePath statePath, const ExecutedSteps &executedSteps)
        {
            InjectionStep *executingStep = getExecutingStep(statePath, executedSteps);

            if (executingStep != NULL)
            {
                return executingStep->contrastFluidLocation;
            }
            else if (executedSteps.length() > 0)
            {
                return executedSteps.last().activeContrastLocation;
            }
            return DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_2;
        }

        void setContrastFluidLocation(DS_SystemDef::StatePath statePath, const ExecutedSteps &executedSteps, DS_DeviceDef::FluidSourceIdx contrastFluidLocation)
        {
            int stepIdx = executedSteps.length();
            const InjectionStep* executingStep = getExecutingStep(statePath, executedSteps);

            if ((executingStep != NULL) && (stepIdx > 0))
            {
                stepIdx = stepIdx - 1;
            }

            for (; stepIdx < steps.length(); stepIdx++)
            {
                // Update contrast location to all remaining steps
                steps[stepIdx].contrastFluidLocation = contrastFluidLocation;
            }
        }

        DS_DeviceDef::FluidSourceIdx getLastStepContrastLocation() const
        {
            // Returns the contrast location that will be used for the
            // remainder of the steps in the plan
            if (steps.length() > 0)
            {
                return steps[steps.length() - 1].contrastFluidLocation;
            }
            return DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_2;
        }
    };

    struct InjectionPlanDigest
    {
        QString guid;
        QString histId;
        QString name;
        InjectionPlan plan;
        InjectionPlanDigestState state;
        bool isPersonalized;

        InjectionPlanDigest()
        {
            guid = EMPTY_GUID;
            histId = EMPTY_GUID;
            name = "";
            state = INJECTION_PLAN_DIGEST_STATE_UNKNOWN;
            isPersonalized = false;
        }

        bool operator==(const InjectionPlanDigest &arg) const
        {
            bool equal = ( (guid == arg.guid) &&
                           (histId == arg.histId) &&
                           (name == arg.name) &&
                           (plan == arg.plan) &&
                           (state == arg.state) &&
                           (isPersonalized == arg.isPersonalized) );
            return equal;
        }

        bool operator!=(const InjectionPlanDigest &arg) const
        {
            return !operator==(arg);
        }
    };

    typedef QList<InjectionPlanDigest> InjectionPlanDigests;

    struct InjectionPlanTemplateGroup
    {
        QString name;
        InjectionPlanDigests planDigests;

        bool operator==(const InjectionPlanTemplateGroup &arg) const
        {
            bool equal = ( (name == arg.name) &&
                           (planDigests == arg.planDigests) );
            return equal;
        }

        bool operator!=(const InjectionPlanTemplateGroup &arg) const
        {
            return !operator==(arg);
        }

        InjectionPlanDigest *getPlanDigestFromTemplateGuid(QString templateGuid)
        {
            for (int i = 0; i < planDigests.length(); i++)
            {
                if (planDigests[i].guid == templateGuid)
                {
                    return &planDigests[i];
                }
            }
            return NULL;
        }

        bool isReady() const
        {
            for (int planIdx = 0; planIdx < planDigests.length(); planIdx++)
            {
                if (planDigests[planIdx].state == DS_ExamDef::INJECTION_PLAN_DIGEST_STATE_INITIALISING)
                {
                    return false;
                }
            }
            return true;
        }
    };

    typedef QList<InjectionPlanTemplateGroup> InjectionPlanTemplateGroups;

    struct WorklistDetails
    {
        DS_MwlDef::WorklistEntry entry;
        DS_MwlDef::DicomFields panel;
        DS_MwlDef::DicomFields all;

        bool operator==(const WorklistDetails &arg) const
        {
            bool equal = ( (entry == arg.entry) &&
                           (panel == arg.panel) &&
                           (all == arg.all) );
            return equal;
        }

        bool operator!=(const WorklistDetails &arg) const
        {
            return !operator==(arg);
        }
    };

    struct ExamField
    {
        QString name;
        QString value;
        QString units;
        QString validDataType;
        Config::ValidRange validRange;
        QVariantList validList;
        bool needsTranslated;
        bool isEnabled;
        bool isMandatory;
        bool isMandatoryEntered;

        bool operator==(const ExamField &arg) const
        {
            bool equal = ( (name == arg.name) &&
                           (value == arg.value) &&
                           (units == arg.units) &&
                           (validDataType == arg.validDataType) &&
                           (validRange == arg.validRange) &&
                           (validList == arg.validList) &&
                           (needsTranslated == arg.needsTranslated) &&
                           (isEnabled == arg.isEnabled) &&
                           (isMandatory == arg.isMandatory) &&
                           (isMandatoryEntered == arg.isMandatoryEntered) );
            return equal;
        }

        bool operator!=(const ExamField &arg) const
        {
            return !operator==(arg);
        }
    };

    typedef QMap<QString, ExamField> ExamFieldMap;

    // Currently ExamFieldParameter and InjectionPersonalizationInput are identical. Creating typedef so that user is independent of the implementation
    typedef InjectionPersonalizationInput ExamFieldParameter;

    struct LinkedAccession
    {
        bool isLinked;
        DS_MwlDef::WorklistEntry entry;

        bool operator==(const LinkedAccession &arg) const
        {
            bool equal = ( (isLinked == arg.isLinked) &&
                           (entry == arg.entry) );
            return equal;
        }

        bool operator!=(const LinkedAccession &arg) const
        {
            return !operator==(arg);
        }
    };

    typedef QList<LinkedAccession> LinkedAccessions;

    struct ExamAdvanceInfo
    {
        QString guid; // ExamGuid
        WorklistDetails worklistDetails;
        ExamFieldMap examInputs;
        ExamFieldMap examResults;
        LinkedAccessions linkedAccessions;
        bool mandatoryFieldsEntered;
        int examInputsProgress;
        int examResultsProgress;

        ExamAdvanceInfo()
        {
            init();
        }

        void init()
        {
            guid = EMPTY_GUID;
            examInputsProgress = -1; // CRU will update this value with a number ranged from 0 - 100
            examResultsProgress = 0;
            mandatoryFieldsEntered = false;
        }

        bool operator==(const ExamAdvanceInfo &arg) const
        {
            bool equal = ( (guid == arg.guid) &&
                           (worklistDetails == arg.worklistDetails) &&
                           (examInputs == arg.examInputs) &&
                           (examResults == arg.examResults) &&
                           (linkedAccessions == arg.linkedAccessions) &&
                           (mandatoryFieldsEntered == arg.mandatoryFieldsEntered) &&
                           (examInputsProgress == arg.examInputsProgress) &&
                           (examResultsProgress == arg.examResultsProgress) );
            return equal;
        }

        bool operator!=(const ExamAdvanceInfo &arg) const
        {
            return !operator==(arg);
        }
    };

    struct PulseSalineVolumeLookupRow
    {
        int contrastPercentage;
        double flowRate;
        double salineVol;

        PulseSalineVolumeLookupRow(int contrastPercentage_, double flowRate_, double salineVol_) :
            contrastPercentage(contrastPercentage_),
            flowRate(flowRate_),
            salineVol(salineVol_)
        {
        }

        bool operator==(const PulseSalineVolumeLookupRow &arg) const
        {
            return ( (contrastPercentage == arg.contrastPercentage) &&
                     (flowRate == arg.flowRate) &&
                     (salineVol == arg.salineVol) );
        }

        bool operator!=(const PulseSalineVolumeLookupRow &arg) const
        {
            return !operator==(arg);
        }
    };

    typedef QList<PulseSalineVolumeLookupRow> PulseSalineVolumeLookupRows;
};


Q_DECLARE_METATYPE(DS_ExamDef::ScannerInterlocks);
Q_DECLARE_METATYPE(DS_ExamDef::InjectionPlan);
Q_DECLARE_METATYPE(DS_ExamDef::FluidVolumes);
Q_DECLARE_METATYPE(DS_ExamDef::ExecutedSteps);
Q_DECLARE_METATYPE(DS_ExamDef::InjectionPlanDigests);
Q_DECLARE_METATYPE(DS_ExamDef::InjectionPlanTemplateGroups);
Q_DECLARE_METATYPE(DS_ExamDef::ExamProgressState);
Q_DECLARE_METATYPE(DS_ExamDef::InjectionRequestProcessStatus);
Q_DECLARE_METATYPE(DS_ExamDef::ExamAdvanceInfo);
Q_DECLARE_METATYPE(DS_ExamDef::PulseSalineVolumeLookupRows);
#endif // DS_EXAM_DEF_H
