#include "Apps/AppManager.h"
#include "DS_ExamData.h"
#include "DataServices/CfgLocal/DS_CfgLocal.h"
#include "DataServices/CfgGlobal/DS_CfgGlobal.h"
#include "DataServices/Mcu/DS_McuData.h"
#include "DataServices/System/DS_SystemData.h"
#include "DataServices/Device/DS_DeviceData.h"
#include "DataServices/Exam/DS_ExamAction.h"
#include "DataServices/Capabilities/DS_Capabilities.h"
#include "Common/ImrParser.h"

DS_ExamData::DS_ExamData(QObject *parent, EnvGlobal *env_) :
    QObject(parent),
    env(env_)
{
    envLocal = new EnvLocal("DS_Exam-Data", "EXAM_DATA");

    connect(env->appManager, &AppManager::signalAppStarted, this, [=]() {
        slotAppStarted();
    });

    m_DataLocked = false;

    QString errStr;

    // Init InjectionPlanTemplateGroups
    QVariantList planTemplateGroups = env->ds.cfgLocal->get_Hidden_InjectionPlanTemplateGroups();
    m_InjectionPlanTemplateGroups = ImrParser::ToCpp_InjectionPlanTemplateGroups(planTemplateGroups, &errStr, true);
    if (errStr != "")
    {
        LOG_ERROR("Failed to get InjectionPlanTemplateGroups. Err=%s\n", errStr.CSTR());
    }

    // Initialises default if none is found
    if (getDefaultInjectionPlanTemplate() == NULL)
    {
        LOG_ERROR("Failed to init default injection plan. Was not found in local.cfg\n");
    }

    // Init InjectionPlan
    const DS_ExamDef::InjectionPlan *defaultPlan = getDefaultInjectionPlanTemplate();
    m_InjectionPlan.loadFromTemplate(*defaultPlan);
    m_InjectionPlanPreview = *defaultPlan;

    // Clear Preloaded State
    for (int stepIdx = m_ExecutedSteps.length(); stepIdx < m_InjectionPlan.steps.length(); stepIdx++)
    {
        m_InjectionPlan.steps[stepIdx].isPreloaded = false;
    }
    //SET_LAST_DATA(PreloadedStep1)
    //SET_LAST_DATA(PreloadedStep2)

    // set SUDS length to default on startup
    m_SUDSLength = env->ds.cfgGlobal->get_Configuration_Behaviors_DefaultSUDSLength();

    // Init exam state data set
    m_IsAirCheckNeeded = false;
    m_IsScannerInterlocksCheckNeeded = false;
    m_ExamGuid = EMPTY_GUID;
    m_ExamProgressState = DS_ExamDef::EXAM_PROGRESS_STATE_IDLE;
    m_ExamStartedAtEpochMs = 0;

    QVariantMap examAdvanceInfoMap = env->ds.cfgLocal->get_Hidden_LastExamAdvanceInfo();
    m_ExamAdvanceInfo = ImrParser::ToCpp_ExamAdvanceInfo(examAdvanceInfoMap, &errStr);

    // Init PulseSalineVolume Lookup Table (contrast percentage, flow rate, required pulsing volume)
    m_PulseSalineVolumeLookupRows.append(DS_ExamDef::PulseSalineVolumeLookupRow(95, 1, 6));
    m_PulseSalineVolumeLookupRows.append(DS_ExamDef::PulseSalineVolumeLookupRow(95, 2, 6));
    m_PulseSalineVolumeLookupRows.append(DS_ExamDef::PulseSalineVolumeLookupRow(95, 3, 7));
    m_PulseSalineVolumeLookupRows.append(DS_ExamDef::PulseSalineVolumeLookupRow(95, 4, 6));
    m_PulseSalineVolumeLookupRows.append(DS_ExamDef::PulseSalineVolumeLookupRow(95, 5, 7));
    m_PulseSalineVolumeLookupRows.append(DS_ExamDef::PulseSalineVolumeLookupRow(95, 6, 8));
    m_PulseSalineVolumeLookupRows.append(DS_ExamDef::PulseSalineVolumeLookupRow(95, 7, 8));
    m_PulseSalineVolumeLookupRows.append(DS_ExamDef::PulseSalineVolumeLookupRow(95, 8, 9));
    m_PulseSalineVolumeLookupRows.append(DS_ExamDef::PulseSalineVolumeLookupRow(95, 9, 8));
    m_PulseSalineVolumeLookupRows.append(DS_ExamDef::PulseSalineVolumeLookupRow(95, 10, 8));
    m_PulseSalineVolumeLookupRows.append(DS_ExamDef::PulseSalineVolumeLookupRow(90, 1, 5));
    m_PulseSalineVolumeLookupRows.append(DS_ExamDef::PulseSalineVolumeLookupRow(90, 2, 3));
    m_PulseSalineVolumeLookupRows.append(DS_ExamDef::PulseSalineVolumeLookupRow(90, 3, 4));
    m_PulseSalineVolumeLookupRows.append(DS_ExamDef::PulseSalineVolumeLookupRow(90, 4, 8));
    m_PulseSalineVolumeLookupRows.append(DS_ExamDef::PulseSalineVolumeLookupRow(90, 5, 6));
    m_PulseSalineVolumeLookupRows.append(DS_ExamDef::PulseSalineVolumeLookupRow(90, 6, 9));
    m_PulseSalineVolumeLookupRows.append(DS_ExamDef::PulseSalineVolumeLookupRow(90, 7, 9));
    m_PulseSalineVolumeLookupRows.append(DS_ExamDef::PulseSalineVolumeLookupRow(90, 8, 9));
    m_PulseSalineVolumeLookupRows.append(DS_ExamDef::PulseSalineVolumeLookupRow(90, 9, 8));
    m_PulseSalineVolumeLookupRows.append(DS_ExamDef::PulseSalineVolumeLookupRow(90, 10, 7));
    m_PulseSalineVolumeLookupRows.append(DS_ExamDef::PulseSalineVolumeLookupRow(85, 1, 4));
    m_PulseSalineVolumeLookupRows.append(DS_ExamDef::PulseSalineVolumeLookupRow(85, 2, 3));
    m_PulseSalineVolumeLookupRows.append(DS_ExamDef::PulseSalineVolumeLookupRow(85, 3, 4));
    m_PulseSalineVolumeLookupRows.append(DS_ExamDef::PulseSalineVolumeLookupRow(85, 4, 5));
    m_PulseSalineVolumeLookupRows.append(DS_ExamDef::PulseSalineVolumeLookupRow(85, 5, 5));
    m_PulseSalineVolumeLookupRows.append(DS_ExamDef::PulseSalineVolumeLookupRow(85, 6, 8));
    m_PulseSalineVolumeLookupRows.append(DS_ExamDef::PulseSalineVolumeLookupRow(85, 7, 6));
    m_PulseSalineVolumeLookupRows.append(DS_ExamDef::PulseSalineVolumeLookupRow(85, 8, 5));
    m_PulseSalineVolumeLookupRows.append(DS_ExamDef::PulseSalineVolumeLookupRow(85, 9, 6));
    m_PulseSalineVolumeLookupRows.append(DS_ExamDef::PulseSalineVolumeLookupRow(85, 10, 5));
    m_PulseSalineVolumeLookupRows.append(DS_ExamDef::PulseSalineVolumeLookupRow(80, 1, 5));
    m_PulseSalineVolumeLookupRows.append(DS_ExamDef::PulseSalineVolumeLookupRow(80, 2, 1));
    m_PulseSalineVolumeLookupRows.append(DS_ExamDef::PulseSalineVolumeLookupRow(80, 3, 3));
    m_PulseSalineVolumeLookupRows.append(DS_ExamDef::PulseSalineVolumeLookupRow(80, 4, 5));
    m_PulseSalineVolumeLookupRows.append(DS_ExamDef::PulseSalineVolumeLookupRow(80, 5, 3));
    m_PulseSalineVolumeLookupRows.append(DS_ExamDef::PulseSalineVolumeLookupRow(80, 6, 7));
    m_PulseSalineVolumeLookupRows.append(DS_ExamDef::PulseSalineVolumeLookupRow(80, 7, 6));
    m_PulseSalineVolumeLookupRows.append(DS_ExamDef::PulseSalineVolumeLookupRow(80, 8, 7));
    m_PulseSalineVolumeLookupRows.append(DS_ExamDef::PulseSalineVolumeLookupRow(80, 9, 5));
    m_PulseSalineVolumeLookupRows.append(DS_ExamDef::PulseSalineVolumeLookupRow(80, 10, 4));

    SET_LAST_DATA(DataLocked)
    SET_LAST_DATA(ScannerInterlocks)
    SET_LAST_DATA(InjectionPlan)
    SET_LAST_DATA(IsAirCheckNeeded)
    SET_LAST_DATA(IsScannerInterlocksCheckNeeded)
    SET_LAST_DATA(ExecutedSteps)
    SET_LAST_DATA(InjectionPlanPreview)
    SET_LAST_DATA(InjectionPlanTemplateGroups)
    SET_LAST_DATA(ExamGuid)
    SET_LAST_DATA(ExamProgressState)
    SET_LAST_DATA(SelectedContrast)
    SET_LAST_DATA(InjectionRequestProcessStatus)
}

DS_ExamData::~DS_ExamData()
{
    delete envLocal;
}

void DS_ExamData::slotAppStarted()
{
    //EMIT_DATA_CHANGED_SIGNAL(DataLocked)
    EMIT_DATA_CHANGED_SIGNAL(ScannerInterlocks)
    EMIT_DATA_CHANGED_SIGNAL(InjectionPlan)
    EMIT_DATA_CHANGED_SIGNAL(IsAirCheckNeeded)
    EMIT_DATA_CHANGED_SIGNAL(IsScannerInterlocksCheckNeeded)
    EMIT_DATA_CHANGED_SIGNAL(ExecutedSteps)
    EMIT_DATA_CHANGED_SIGNAL(InjectionPlanPreview)
    EMIT_DATA_CHANGED_SIGNAL(InjectionPlanTemplateGroups)
    EMIT_DATA_CHANGED_SIGNAL(ExamGuid)
    EMIT_DATA_CHANGED_SIGNAL(ExamProgressState)
    EMIT_DATA_CHANGED_SIGNAL(SelectedContrast)
    EMIT_DATA_CHANGED_SIGNAL(InjectionRequestProcessStatus)
    EMIT_DATA_CHANGED_SIGNAL(PreloadedStep1)
    EMIT_DATA_CHANGED_SIGNAL(PreloadedStep2)
    EMIT_DATA_CHANGED_SIGNAL(SUDSLength)
    EMIT_DATA_CHANGED_SIGNAL(ExamAdvanceInfo)
    EMIT_DATA_CHANGED_SIGNAL(PulseSalineVolumeLookupRows)
}

const DS_ExamDef::InjectionStep *DS_ExamData::getExecutingStep()
{
    return m_InjectionPlan.getExecutingStep(env->ds.systemData->getStatePath(), m_ExecutedSteps);
}

const DS_ExamDef::InjectionPhase *DS_ExamData::getExecutingPhase()
{
    DS_McuDef::InjectorStatus mcuProgress = env->ds.mcuData->getInjectorStatus();
    const DS_ExamDef::InjectionStep *executingStep = getExecutingStep();
    if (executingStep == NULL)
    {
        return NULL;
    }

    int curPhaseIdx = 0;

    if (mcuProgress.state == DS_McuDef::INJECTOR_STATE_READY_START)
    {
        // Injection is not started yet. CurPhaseIdx = 0.
        curPhaseIdx = 0;
    }
    else
    {
        DS_McuDef::InjectDigest injectDigest = env->ds.mcuData->getInjectDigest();
        env->ds.examAction->actGetHcuPhaseIdxFromMcuPhaseIdx(curPhaseIdx, injectDigest.phaseIdx);
    }

    if (curPhaseIdx >= executingStep->phases.length())
    {
        return NULL;
    }

    return &executingStep->phases[curPhaseIdx];
}

const DS_ExamDef::InjectionPlan *DS_ExamData::getDefaultInjectionPlanTemplate()
{
    for (int groupIdx = 0; groupIdx < m_InjectionPlanTemplateGroups.length(); groupIdx++)
    {
        const DS_ExamDef::InjectionPlanDigest *planDigest = m_InjectionPlanTemplateGroups[groupIdx].getPlanDigestFromTemplateGuid(DEFAULT_INJECT_PLAN_TEMPLATE_GUID);
        if (planDigest != NULL)
        {
            return &planDigest->plan;
        }
    }

    // Try again after initialising
    initDefaultInjectionPlanTemplate();

    for (int groupIdx = 0; groupIdx < m_InjectionPlanTemplateGroups.length(); groupIdx++)
    {
        const DS_ExamDef::InjectionPlanDigest *planDigest = m_InjectionPlanTemplateGroups[groupIdx].getPlanDigestFromTemplateGuid(DEFAULT_INJECT_PLAN_TEMPLATE_GUID);
        if (planDigest != NULL)
        {
            return &planDigest->plan;
        }
    }

    return NULL;
}

bool DS_ExamData::isContrastSelectAllowed()
{
    static bool wasContrastSelectAllowed = false;
    QString reason = "";
    bool contrastSelectAllowed = true;

    if (!m_SelectedContrast.isDefined())
    {
        contrastSelectAllowed = true;
        reason = "Current selected contrast is not defined";
    }
    else
    {
        bool isSameContrastsLoaded = env->ds.deviceData->isSameContrastsLoaded();
        for (int stepIdx = 0; stepIdx < m_ExecutedSteps.length(); stepIdx++)
        {
            if (m_InjectionPlan.steps[stepIdx].getContrastTotal() > 0)
            {
                // Contrast already injected in one of executed step
                if (!isSameContrastsLoaded)
                {
                    contrastSelectAllowed = false;
                    reason = "Contrast injection executed and different contrast loaded";
                }
                break;
            }
        }
    }

    if (reason == "")
    {
        contrastSelectAllowed = true;
        reason = "No contrast injection executed";
    }

    if (contrastSelectAllowed != wasContrastSelectAllowed)
    {
        LOG_INFO("isContrastSelectAllowed(): contrastSelectAllowed=%s: Reason=%s\n", contrastSelectAllowed ? "TRUE" : "FALSE", reason.CSTR());
        wasContrastSelectAllowed = contrastSelectAllowed;
    }

    return contrastSelectAllowed;
}

bool DS_ExamData::isExamStarted()
{
    return m_ExamGuid != EMPTY_GUID;
}

DS_ExamDef::InjectionPlanDigest *DS_ExamData::getPlanDigestFromTemplateGuid(DS_ExamDef::InjectionPlanTemplateGroups &group, QString templateGuid)
{
    for (int groupIdx = 0; groupIdx < group.length(); groupIdx++)
    {
        DS_ExamDef::InjectionPlanDigest *planDigest = group[groupIdx].getPlanDigestFromTemplateGuid(templateGuid);
        if (planDigest != NULL)
        {
            return planDigest;
        }
    }

    return NULL;
}

void DS_ExamData::initDefaultInjectionPlanTemplate()
{
    QVariantMap defaultInjectionPlanTemplateMap = env->ds.cfgLocal->get_Hidden_DefaultInjectionPlanTemplate();
    DS_ExamDef::InjectionPlanTemplateGroup defaultGroup;
    DS_ExamDef::InjectionPlanDigest planDigest;
    QString errStr;
    planDigest.plan = ImrParser::ToCpp_InjectionPlan(defaultInjectionPlanTemplateMap, &errStr, true);

    if (errStr != "")
    {
        LOG_ERROR("Failed to get DefaultInjectionPlanTemplate. Err=%s\n", errStr.CSTR());
    }

    planDigest.name = planDigest.plan.name;
    planDigest.guid = planDigest.plan.templateGuid;
    planDigest.histId = Util::newGuid();

    defaultGroup.name = "";
    defaultGroup.planDigests.append(planDigest);
    m_InjectionPlanTemplateGroups.append(defaultGroup);
}
