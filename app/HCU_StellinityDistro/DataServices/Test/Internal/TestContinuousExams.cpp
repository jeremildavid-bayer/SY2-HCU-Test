#include "Apps/AppManager.h"
#include "TestContinuousExams.h"
#include "Common/ImrParser.h"
#include "DataServices/Test/DS_TestData.h"
#include "DataServices/Capabilities/DS_Capabilities.h"
#include "DataServices/Exam/DS_ExamAction.h"
#include "DataServices/Exam/DS_ExamData.h"
#include "DataServices/Device/DS_DeviceAction.h"
#include "DataServices/Device/DS_DeviceData.h"
#include "DataServices/Alert/DS_AlertData.h"
#include "DataServices/Mcu/DS_McuData.h"
#include "DataServices/Cru/DS_CruData.h"
#include "DataServices/Alert/DS_AlertAction.h"

TestContinuousExams::TestContinuousExams(QObject *parent, EnvGlobal *env_, EnvLocal *envLocal_) :
    ActionBaseExt(parent, env_)
{
    envLocal = envLocal_;

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });
}

TestContinuousExams::~TestContinuousExams()
{
}

void TestContinuousExams::slotAppInitialised()
{
    connect(env->ds.cruData, &DS_CruData::signalDataChanged_CruLinkStatus, this, [=](const DS_CruDef::CruLinkStatus &cruLinkStatus, const DS_CruDef::CruLinkStatus &prevCruLinkStatus) {
        if ( (cruLinkStatus.state != prevCruLinkStatus.state) &&
             (cruLinkStatus.state == DS_CruDef::CRU_LINK_STATE_INACTIVE) )
        {
            if (state == STATE_IDLE)
            {
                return;
            }
            linkDroppedCount++;
            DS_TestDef::TestStatus testStatus = env->ds.testData->getTestStatus();
            testStatus.statusMap.insert("LinkDropped", linkDroppedCount);
            env->ds.testData->setTestStatus(testStatus);
        }
    });

    connect(env->ds.alertData, &DS_AlertData::signalDataChanged_AllAlerts, this, [=] {
        if (state == STATE_IDLE)
        {
            return;
        }

        // Get alerts that are occurred after test started
        QVariantList allAlerts = env->ds.alertData->getAllAlerts();
        for (int alertIdx = 0; alertIdx < allAlerts.length(); alertIdx++)
        {
            QVariantMap alert = allAlerts[alertIdx].toMap();
            qint64 alertActiveAtEpochMs = Util::utcDateTimeStrToQDateTime(alert[_L("ActiveAt")].toString()).toMSecsSinceEpoch();

            if (alertActiveAtEpochMs < testStartedAtEpochMs)
            {
                // alert occurred before test started
                allAlerts.removeAt(alertIdx);
                alertIdx--;
            }
        }

        //LOG_DEBUG("signalDataChanged_AllAlerts: All alerts occurred since test started=\n%s\n", Util::qVarientToJsonData(allAlerts, false).CSTR());

        bool monitoredAlertsChanged = false;
        int alertUpdateFrom = -1;

        // Update monitoredAlerts with new alerts
        if (monitoredAlerts.length() == 0)
        {
            alertUpdateFrom = 0;
        }
        else
        {
            QVariantMap lastAlert = monitoredAlerts.last().toMap();
            qint64 lastMonitoredAlertActiveAt = Util::utcDateTimeStrToQDateTime(lastAlert[_L("ActiveAt")].toString()).toMSecsSinceEpoch();

            for (int alertIdx = allAlerts.length() - 1; alertIdx >= 0 ; alertIdx--)
            {
                QVariantMap alert = allAlerts[alertIdx].toMap();
                qint64 alertActiveAtEpochMs = Util::utcDateTimeStrToQDateTime(alert[_L("ActiveAt")].toString()).toMSecsSinceEpoch();
                if (alertActiveAtEpochMs < lastMonitoredAlertActiveAt)
                {
                    // Alert is already added to monitoredAlerts
                    break;
                }
                alertUpdateFrom = alertIdx;
            }
        }


        if (alertUpdateFrom != -1)
        {
            for (int alertIdx = alertUpdateFrom; alertIdx < allAlerts.length(); alertIdx++)
            {
                QVariantMap alert = allAlerts[alertIdx].toMap();
                QString alertSeverity = alert[_L("Severity")].toString();
                if ( (alertSeverity == _L("Fatal")) ||
                     (alertSeverity == _L("Error")) )
                {
                    monitoredAlerts.append(alert);
                    monitoredAlertsChanged |= true;
                }
            }
        }


        if (monitoredAlertsChanged)
        {
            LOG_DEBUG("signalDataChanged_AllAlerts: Alerts Monitored=%d\n", (int)monitoredAlerts.length());
            DS_TestDef::TestStatus testStatus = env->ds.testData->getTestStatus();
            testStatus.statusMap.insert("AlertsOccurred", monitoredAlerts.length());
            env->ds.testData->setTestStatus(testStatus);
        }
    });

    setState(STATE_IDLE);
}

DataServiceActionStatus TestContinuousExams::actStart(QVariantList testParamArgs, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "TestContinuous-Start", QString().asprintf("%s", Util::qVarientToJsonData(testParamArgs).CSTR()));
    DS_SystemDef::StatePath statePath;
    DS_DeviceDef::FluidSources fluidSourceBottles = env->ds.deviceData->getFluidSourceBottles();
    DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
    QString activeContrastLocationStr = env->ds.capabilities->get_Developer_ContinuousExamsTestActiveContrastLocation();
    activeContrastLocation = activeContrastLocationStr == "C1" ? SYRINGE_IDX_CONTRAST1 : SYRINGE_IDX_CONTRAST2 ;

    DS_TestDef::TestStatus testStatus = env->ds.testData->getTestStatus();
    testStatus.guid = actGuid;
    testStatus.type = DS_TestDef::TEST_TYPE_CONTINUOUS_EXAMS;
    testStatus.progress = 0;
    testStatus.userAborted = false;
    testStatus.isFinished = false;
    testStatus.stateStr = "";

    linkDroppedCount = 0;
    executedExamsCount = 0;
    monitoredAlerts.clear();

    QDateTime testStartedAt = QDateTime::currentDateTimeUtc();
    testStatus.statusMap = QVariantMap();
    testStatus.statusMap.insert("StartedAt", Util::qDateTimeToUtcDateTimeStr(testStartedAt));
    testStatus.statusMap.insert("ExecutedExams", executedExamsCount);
    testStatus.statusMap.insert("LinkDropped", linkDroppedCount);
    testStatus.statusMap.insert("AlertsOccurred", monitoredAlerts.length());
    testStatus.statusMap.insert("Error", "");
    testStartedAtEpochMs = testStartedAt.toMSecsSinceEpoch();


    if (state != STATE_IDLE)
    {
        status.state = DS_ACTION_STATE_BUSY;
        status.err = QString().asprintf("Bad Test State (%d)", state);
        goto bail;
    }

    // Check Condition: StatePath
    statePath = env->ds.systemData->getStatePath();
    if (statePath != DS_SystemDef::STATE_PATH_IDLE)
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = QString().asprintf("Invalid State: StatePath=%s", ImrParser::ToImr_StatePath(statePath).CSTR());
        goto bail;
    }

    // Check Condition: MUDS Inserted
    if (!env->ds.mcuData->getMudsInserted())
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = QString().asprintf("DaySet Not Inserted");
        goto bail;
    }

    // Check Condition: MUDS Inserted
    if (!env->ds.mcuData->getSudsInserted())
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = QString().asprintf("PatientLine Not Inserted");
        goto bail;
    }

    // Check Condition: Saline bottle ready
    if (!fluidSourceBottles[SYRINGE_IDX_SALINE].readyForInjection())
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = QString().asprintf("BS0 Not Ready");
        goto bail;
    }

    // Check Condition: Saline syringe ready
    if (!fluidSourceSyringes[SYRINGE_IDX_SALINE].readyForInjection())
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = QString().asprintf("RS0 Not Ready");
        goto bail;
    }

    // Check Condition: Saline syringe not busy
    if (fluidSourceSyringes[SYRINGE_IDX_SALINE].isBusy)
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = QString().asprintf("RS0 Busy");
        goto bail;
    }

    // Check Condition: Contrast bottle ready
    if (!fluidSourceBottles[activeContrastLocation].readyForInjection())
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = QString().asprintf("%s Not Ready", ImrParser::ToImr_FluidSourceBottleIdx(activeContrastLocation).CSTR());
        goto bail;
    }

    // Check Condition: Contrast syringe ready
    if (!fluidSourceSyringes[activeContrastLocation].readyForInjection())
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = QString().asprintf("%s Not Ready", ImrParser::ToImr_FluidSourceSyringeIdx(activeContrastLocation).CSTR());
        goto bail;
    }

    // Check Condition: Contrast syringe not busy
    if (fluidSourceSyringes[activeContrastLocation].isBusy)
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = QString().asprintf("%s Busy", ImrParser::ToImr_FluidSourceSyringeIdx(activeContrastLocation).CSTR());
        goto bail;
    }

    // Set stepTemplateGuids
    {
        QString selectedPlansStr = env->ds.capabilities->get_Developer_ContinuousExamsTestSelectedPlans();
        planTemplateGuids = Util::jsonDataToQVariant(selectedPlansStr).toStringList();
        planTemplateGuidIndex = 0;

        for (int i = 0; i < planTemplateGuids.length(); i++)
        {
            LOG_DEBUG("TEST_CONTINUOUS_EXAMS: planTemplateGuids[%d]=%s\n", i, planTemplateGuids[i].CSTR());
        }
    }

bail:

    if (status.err != "")
    {
        // Check Condition failed
        LOG_ERROR("TEST_CONTINUOUS_EXAMS: Check Condition Failed: Err=%s\n", status.err.CSTR());
        actionStarted(status);
        testStatus.isFinished = true;
        testStatus.stateStr = "Start Condition Check Failed";
        testStatus.statusMap.insert("Error", status.err);
        env->ds.testData->setTestStatus(testStatus);
        return status;
    }

    LOG_INFO("\n\n");
    LOG_INFO("TEST_CONTINUOUS_EXAMS: Test started\n");

    actStatusBuf = status;
    actStatusBuf.state = DS_ACTION_STATE_START_WAITING;
    actionStarted(actStatusBuf);
    env->ds.testData->setTestStatus(testStatus);
    setState(STATE_STARTED);
    return actStatusBuf;
}

DataServiceActionStatus TestContinuousExams::actStop(QString actGuid)
{
    DataServiceActionStatus status;
    status.guid = actGuid;

    if (state == STATE_IDLE)
    {
        LOG_ERROR("TEST_CONTINUOUS_EXAMS: STOP: Test is not running\n");
        status.state = DS_ACTION_STATE_BAD_REQUEST;
        status.err = "Test is not running";
        actionStarted(status);
    }
    else
    {
        status.state = DS_ACTION_STATE_STARTED;
        actionStarted(status);

        DS_TestDef::TestStatus testStatus = env->ds.testData->getTestStatus();
        testStatus.userAborted = true;
        env->ds.testData->setTestStatus(testStatus);

        setState(STATE_ABORTED);

        status.state = DS_ACTION_STATE_COMPLETED;
        actionCompleted(status);
    }

    return status;
}

void TestContinuousExams::setStateSynch(int newState)
{
    DS_TestDef::TestStatus testStatus = env->ds.testData->getTestStatus();
    if (testStatus.isFinished)
    {
        state = STATE_IDLE;
    }
    else
    {
        state = newState;
    }
}

QString TestContinuousExams::getStateStr(int state)
{
    switch (state)
    {
    case STATE_IDLE: return "IDLE";
    case STATE_REFILL_MONITOR_STARTED: return "REFILL_MONITOR_STARTED";
    case STATE_REFILL_MONITOR_PROGRESS: return "REFILL_MONITOR_PROGRESS";
    case STATE_REFILL_MONITOR_DONE: return "REFILL_MONITOR_DONE";
    case STATE_STARTED: return "STARTED";
    case STATE_SUDS_PREPARING: return "SUDS_PREPARING";
    case STATE_SUDS_PREPARED: return "SUDS_PREPARED";
    case STATE_EXAM_START_WAITING: return "EXAM_START_WAITING";
    case STATE_EXAM_PREPARE_STARTED: return "EXAM_PREPARE_STARTED";
    case STATE_EXAM_PREPARE_PROGRESS: return "EXAM_PREPARE_PROGRESS";
    case STATE_EXAM_PREPARE_DONE: return "EXAM_PREPARE_DONE";
    case STATE_EXAM_PREPARE_FAILED: return "EXAM_PREPARE_FAILED";
    case STATE_EXAM_PLAN_SELECT_STARTED: return "EXAM_PLAN_SELECT_STARTED";
    case STATE_EXAM_PLAN_SELECT_PROGRESS: return "EXAM_PLAN_SELECT_PROGRESS";
    case STATE_EXAM_PLAN_SELECT_DONE: return "EXAM_PLAN_SELECT_DONE";
    case STATE_EXAM_PLAN_SELECT_FAILED: return "EXAM_PLAN_SELECT_FAILED";
    case STATE_EXAM_STEP_PREPARING: return "EXAM_STEP_PREPARING";
    case STATE_EXAM_STEP_PREPARED: return "EXAM_STEP_PREPARED";
    case STATE_EXAM_ARM_STARTED: return "EXAM_ARM_STARTED";
    case STATE_EXAM_ARM_PROGRESS: return "EXAM_ARM_PROGRESS";
    case STATE_EXAM_ARM_DONE: return "EXAM_ARM_DONE";
    case STATE_EXAM_ARM_FAILED: return "EXAM_ARM_FAILED";
    case STATE_EXAM_INJECTION_START_STARTED: return "EXAM_INJECTION_START_STARTED";
    case STATE_EXAM_INJECTION_START_PROGRESS: return "EXAM_INJECTION_START_PROGRESS";
    case STATE_EXAM_INJECTION_START_DONE: return "EXAM_INJECTION_START_DONE";
    case STATE_EXAM_INJECTION_START_FAILED: return "EXAM_INJECTION_START_FAILED";
    case STATE_EXAM_INJECTION_MONITOR_STARTED: return "EXAM_INJECTION_MONITOR_STARTED";
    case STATE_EXAM_INJECTION_MONITOR_PROGRESS: return "EXAM_INJECTION_MONITOR_PROGRESS";
    case STATE_EXAM_INJECTION_MONITOR_DONE: return "EXAM_INJECTION_MONITOR_DONE";
    case STATE_EXAM_INJECTION_MONITOR_FAILED: return "EXAM_INJECTION_MONITOR_FAILED";
    case STATE_EXAM_COMPLETE_STARTED: return "EXAM_COMPLETE_STARTED";
    case STATE_EXAM_COMPLETE_PROGRESS: return "EXAM_COMPLETE_PROGRESS";
    case STATE_EXAM_COMPLETE_DONE: return "EXAM_COMPLETE_DONE";
    case STATE_EXAM_COMPLETE_FAILED: return "EXAM_COMPLETE_FAILED";
    case STATE_FAILED: return "FAILED";
    case STATE_ABORTED: return "ABORTED";
    case STATE_DONE: return "DONE";
    default: return QString().asprintf("UNKNOWN(%d)", state);
    }
}

void TestContinuousExams::updateTestStateStr()
{
    DS_TestDef::TestStatus testStatus = env->ds.testData->getTestStatus();
    testStatus.stateStr = getStateStr(state);
    if ( (state == STATE_EXAM_PREPARE_FAILED) ||
         (state == STATE_EXAM_PLAN_SELECT_FAILED) ||
         (state == STATE_EXAM_INJECTION_START_FAILED) ||
         (state == STATE_EXAM_INJECTION_MONITOR_FAILED) ||
         (state == STATE_EXAM_COMPLETE_FAILED) ||
         (state == STATE_FAILED) )
    {
        LOG_ERROR("TEST_CONTINUOUS_EXAMS: %s\n", testStatus.stateStr.CSTR());
    }
    else
    {
        LOG_INFO("TEST_CONTINUOUS_EXAMS: %s\n", testStatus.stateStr.CSTR());
    }
    env->ds.testData->setTestStatus(testStatus);
}

void TestContinuousExams::processState()
{
    updateTestStateStr();
    DS_TestDef::TestStatus testStatus = env->ds.testData->getTestStatus();

    switch (state)
    {
    case STATE_IDLE:
        break;
    case STATE_STARTED:
        env->ds.alertAction->activate("ContinuousExamsTestStarted");
        setState(STATE_REFILL_MONITOR_STARTED);
        break;
    // ===========================
    case STATE_REFILL_MONITOR_STARTED:
        {
            DS_DeviceDef::FluidSources fluidSourceSyringes = env->ds.deviceData->getFluidSourceSyringes();
            if ( (fluidSourceSyringes[SYRINGE_IDX_SALINE].readyForInjection()) &&
                 (!fluidSourceSyringes[SYRINGE_IDX_SALINE].isBusy) &&
                 (fluidSourceSyringes[activeContrastLocation].readyForInjection()) &&
                 (!fluidSourceSyringes[activeContrastLocation].isBusy) )
            {
                 setState(STATE_REFILL_MONITOR_DONE);
                 return;
            }

            guidSubAction = Util::newGuid();
            QMetaObject::Connection conn = connect(env->ds.deviceData, &DS_DeviceData::signalDataChanged_FluidSourceSyringes, this, [=](const DS_DeviceDef::FluidSources &fluidSourceSyringes) {
                if (state != STATE_REFILL_MONITOR_PROGRESS)
                {
                    // unexpected callback
                    env->actionMgr->deleteActAll(guidSubAction);
                    return;
                }


                if ( (fluidSourceSyringes[SYRINGE_IDX_SALINE].readyForInjection()) &&
                     (!fluidSourceSyringes[SYRINGE_IDX_SALINE].isBusy) &&
                     (fluidSourceSyringes[activeContrastLocation].readyForInjection()) &&
                     (!fluidSourceSyringes[activeContrastLocation].isBusy) )
                {
                    env->actionMgr->deleteActAll(guidSubAction);
                    setState(STATE_REFILL_MONITOR_DONE);
                }
            });
            env->actionMgr->createActCompleted(guidSubAction, conn, QString(__PRETTY_FUNCTION__) + ": STATE_REFILL_MONITOR_STARTED");
            setState(STATE_REFILL_MONITOR_PROGRESS);
        }
        break;
    case STATE_REFILL_MONITOR_PROGRESS:
        break;
    case STATE_REFILL_MONITOR_DONE:
        setState(STATE_SUDS_PREPARING);
        break;
    // ===========================
    case STATE_SUDS_PREPARING:
        env->ds.deviceAction->actSudsSetNeedsReplace(false);
        env->ds.deviceAction->actSudsSetNeedsPrime(false);
        setState(STATE_SUDS_PREPARED);
        break;
    case STATE_SUDS_PREPARED:
        setState(STATE_EXAM_START_WAITING);
        break;
    case STATE_EXAM_START_WAITING:
        {
            int examStartDelaySec = env->ds.capabilities->get_Developer_ContinuousExamsTestExamStartDelaySec();
            QTimer::singleShot(examStartDelaySec * 1000, this, [=] {
                if (state == STATE_EXAM_START_WAITING)
                {
                    setState(STATE_EXAM_PREPARE_STARTED);
                }
            });
        }
        break;
    // ===========================
    case STATE_EXAM_PREPARE_STARTED:
        if (env->ds.cruData->getLicenseEnabledPatientStudyContext())
        {
            actStatusBuf.err = "PatientStudyContext License should be disabled";
            LOG_ERROR("TEST_CONTINUOUS_EXAMS: Err=%s\n", actStatusBuf.err.CSTR());
            setState(STATE_FAILED);
            return;
        }
        executedExamsCount++;
        testStatus.statusMap.insert("ExecutedExams", executedExamsCount);
        env->ds.testData->setTestStatus(testStatus);
        handleSubAction(STATE_EXAM_PREPARE_PROGRESS, STATE_EXAM_PREPARE_DONE, STATE_EXAM_PREPARE_FAILED);
        env->ds.examAction->actExamPrepare(guidSubAction);
        break;
    case STATE_EXAM_PREPARE_PROGRESS:
        break;
    case STATE_EXAM_PREPARE_DONE:
        setState(STATE_EXAM_PLAN_SELECT_STARTED);
        break;
    case STATE_EXAM_PREPARE_FAILED:
        setState(STATE_FAILED);
        break;
    // ===========================
    case STATE_EXAM_PLAN_SELECT_STARTED:
        {
            if (planTemplateGuidIndex >= planTemplateGuids.length())
            {
                actStatusBuf.state = DS_ACTION_STATE_INTERNAL_ERR;
                actStatusBuf.err = "Unable to select plan";
                setState(STATE_EXAM_PLAN_SELECT_FAILED);
                return;
            }

            QString planTemplateGuid = planTemplateGuids[planTemplateGuidIndex];
            DS_ExamDef::InjectionPlanTemplateGroups groups = env->ds.examData->getInjectionPlanTemplateGroups();
            DS_ExamDef::InjectionPlanDigest *planDigest = env->ds.examData->getPlanDigestFromTemplateGuid(groups, planTemplateGuid);
            if (planDigest == NULL)
            {
                actStatusBuf.state = DS_ACTION_STATE_INTERNAL_ERR;
                actStatusBuf.err = "Unable to load plan";
                setState(STATE_EXAM_PLAN_SELECT_FAILED);
                return;
            }

            // Initiate plan with empty guid. This is to trigger the plan_changed signal (to force auto-refill)
            DS_ExamDef::InjectionPlan plan = env->ds.examData->getInjectionPlan();
            plan.templateGuid = EMPTY_GUID;
            env->ds.examData->setInjectionPlan(plan);

            // Select plan
            handleSubAction(STATE_EXAM_PLAN_SELECT_PROGRESS, STATE_EXAM_PLAN_SELECT_DONE, STATE_EXAM_PLAN_SELECT_FAILED);
            env->ds.examAction->actInjectionSelect(plan, planDigest->plan, guidSubAction);
        }
        break;
    case STATE_EXAM_PLAN_SELECT_PROGRESS:
        break;
    case STATE_EXAM_PLAN_SELECT_DONE:
        {
            env->ds.examAction->actSetContrastFluidLocation(activeContrastLocation);
            DS_ExamDef::InjectionPlan plan = env->ds.examData->getInjectionPlan();
            LOG_DEBUG("TEST_CONTINUOUS_EXAMS: Plan[%d] selected=%s\n", planTemplateGuidIndex, plan.templateGuid.CSTR());

            if (planTemplateGuidIndex < planTemplateGuids.length() - 1)
            {
                planTemplateGuidIndex++;
            }
            else
            {
                planTemplateGuidIndex = 0;
            }
        }
        setState(STATE_EXAM_STEP_PREPARING);
        break;
    case STATE_EXAM_PLAN_SELECT_FAILED:
        setState(STATE_FAILED);
        break;
    // ===========================
    case STATE_EXAM_STEP_PREPARING:
        QTimer::singleShot(500, this, [=] {
            if (state == STATE_EXAM_STEP_PREPARING)
            {
                setState(STATE_EXAM_STEP_PREPARED);
            }
        });
        break;
    case STATE_EXAM_STEP_PREPARED:
        {
            DS_ExamDef::ExecutedSteps executedSteps = env->ds.examData->getExecutedSteps();
            DS_ExamDef::InjectionPlan plan = env->ds.examData->getInjectionPlan();
            if (executedSteps.length() < plan.steps.length())
            {
                LOG_DEBUG("TEST_CONTINUOUS_EXAMS: Step[%d]: prepared\n", (int)executedSteps.length());
                setState(STATE_EXAM_ARM_STARTED);
            }
            else
            {
                // All steps executed
                setState(STATE_EXAM_COMPLETE_STARTED);
            }
        }
        break;
    // ===========================
    case STATE_EXAM_ARM_STARTED:
        env->ds.examData->setIsAirCheckNeeded(false);
        handleSubAction(STATE_EXAM_ARM_PROGRESS, STATE_EXAM_ARM_DONE, STATE_EXAM_ARM_FAILED);
        env->ds.examAction->actArm(DS_ExamDef::ARM_TYPE_NORMAL, guidSubAction);
        break;
    case STATE_EXAM_ARM_PROGRESS:
        break;
    case STATE_EXAM_ARM_DONE:
        LOG_DEBUG("TEST_CONTINUOUS_EXAMS: armed\n");
        setState(STATE_EXAM_INJECTION_START_STARTED);
        break;
    case STATE_EXAM_ARM_FAILED:
        setState(STATE_FAILED);
        break;
    // ===========================
    case STATE_EXAM_INJECTION_START_STARTED:
        LOG_DEBUG("TEST_CONTINUOUS_EXAMS: injection starting..\n");
        handleSubAction(STATE_EXAM_INJECTION_START_PROGRESS, STATE_EXAM_INJECTION_START_DONE, STATE_EXAM_INJECTION_START_FAILED);
        env->ds.examAction->actInjectionStart(guidSubAction);
        break;
    case STATE_EXAM_INJECTION_START_PROGRESS:
        break;
    case STATE_EXAM_INJECTION_START_DONE:
        LOG_DEBUG("TEST_CONTINUOUS_EXAMS: injection completed.\n");
        setState(STATE_EXAM_INJECTION_MONITOR_STARTED);
        break;
    case STATE_EXAM_INJECTION_START_FAILED:
        setState(STATE_FAILED);
        break;
    // ===========================
    case STATE_EXAM_INJECTION_MONITOR_STARTED:
        {
            guidSubAction = Util::newGuid();
            QMetaObject::Connection conn = connect(env->ds.systemData, &DS_SystemData::signalDataChanged_StatePath, this, [=](DS_SystemDef::StatePath statePath) {
                if (state != STATE_EXAM_INJECTION_MONITOR_PROGRESS)
                {
                    // unexpected callback
                    env->actionMgr->deleteActAll(guidSubAction);
                    return;
                }

                if ( (statePath != DS_SystemDef::STATE_PATH_EXECUTING) &&
                     (statePath != DS_SystemDef::STATE_PATH_BUSY_HOLDING) &&
                     (statePath != DS_SystemDef::STATE_PATH_BUSY_FINISHING) )
                {
                    env->actionMgr->deleteActAll(guidSubAction);
                    if (statePath == DS_SystemDef::STATE_PATH_IDLE)
                    {
                        setState(STATE_EXAM_INJECTION_MONITOR_DONE);
                    }
                    else
                    {
                        actStatusBuf.state = DS_ACTION_STATE_INTERNAL_ERR;
                        actStatusBuf.err = QString().asprintf("Injection aborted with bad state(%s)", ImrParser::ToImr_StatePath(statePath).CSTR());
                        setState(STATE_EXAM_PLAN_SELECT_FAILED);
                    }
                }

            });
            env->actionMgr->createActCompleted(guidSubAction, conn, QString(__PRETTY_FUNCTION__) + ": STATE_EXAM_INJECTION_MONITOR_PROGRESS");
            setState(STATE_EXAM_INJECTION_MONITOR_PROGRESS);
        }
        break;
    case STATE_EXAM_INJECTION_MONITOR_PROGRESS:
        break;
    case STATE_EXAM_INJECTION_MONITOR_DONE:
        setState(STATE_EXAM_STEP_PREPARING);
        break;
    case STATE_EXAM_INJECTION_MONITOR_FAILED:
        setState(STATE_FAILED);
        break;
    // ===========================
    case STATE_EXAM_COMPLETE_STARTED:
        LOG_DEBUG("TEST_CONTINUOUS_EXAMS: All injections completed. Ending exam..\n");
        handleSubAction(STATE_EXAM_COMPLETE_PROGRESS, STATE_EXAM_COMPLETE_DONE, STATE_EXAM_COMPLETE_FAILED);
        env->ds.examAction->actExamEnd(false, guidSubAction);
        break;
    case STATE_EXAM_COMPLETE_PROGRESS:
        break;
    case STATE_EXAM_COMPLETE_DONE:
        {
            LOG_DEBUG("TEST_CONTINUOUS_EXAMS: All injections completed. exam ended.\n");
            int examEndLimit = env->ds.capabilities->get_Developer_ContinuousExamsTestLimit();
            if ( (examEndLimit > 0) &&
                 (executedExamsCount >= examEndLimit) )
            {
                setState(STATE_DONE);
            }
            else
            {
                QTimer::singleShot(3000, this, [=] {
                    if (state == STATE_EXAM_COMPLETE_DONE)
                    {
                        setState(STATE_REFILL_MONITOR_STARTED);
                    }
                });
            }
        }
        break;
    case STATE_EXAM_COMPLETE_FAILED:
        setState(STATE_FAILED);
        break;
    // ===========================
    case STATE_FAILED:
        LOG_ERROR("TEST_CONTINUOUS_EXAMS: STATE_FAILED\n");
        testStatus.isFinished = true;
        testStatus.userAborted = false;
        testStatus.statusMap.insert("Error", actStatusBuf.err);
        env->ds.testData->setTestStatus(testStatus);
        actionCompleted(actStatusBuf);
        env->ds.alertAction->activate("ContinuousExamsTestEnded");
        setState(STATE_IDLE);
        break;
    case STATE_ABORTED:
        actStatusBuf.state = DS_ACTION_STATE_USER_ABORT;
        testStatus.isFinished = true;
        testStatus.userAborted = true;
        testStatus.statusMap.insert("Error", "User Aborted");
        env->ds.testData->setTestStatus(testStatus);
        actionCompleted(actStatusBuf);
        env->ds.alertAction->activate("ContinuousExamsTestEnded");
        setState(STATE_IDLE);
        break;
    case STATE_DONE:
        actStatusBuf.state = DS_ACTION_STATE_USER_ABORT;
        testStatus.isFinished = true;
        testStatus.userAborted = false;
        testStatus.statusMap.insert("Error", "Test Completed");
        env->ds.testData->setTestStatus(testStatus);
        actionCompleted(actStatusBuf);
        env->ds.alertAction->activate("ContinuousExamsTestEnded");
        setState(STATE_IDLE);
        break;
    default:
        LOG_ERROR("TEST_CONTINUOUS_EXAMS: Unknown state(%d)\n", state);
        break;
    }
}


