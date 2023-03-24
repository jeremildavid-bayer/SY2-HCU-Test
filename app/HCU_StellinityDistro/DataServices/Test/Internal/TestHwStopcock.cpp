#include "TestHwStopcock.h"
#include "Common/ImrParser.h"
#include "DataServices/Test/DS_TestData.h"
#include "DataServices/Device/DS_DeviceAction.h"

TestHwStopcock::TestHwStopcock(QObject *parent, EnvGlobal *env_, EnvLocal *envLocal_) :
    ActionBaseExt(parent, env_)
{
    envLocal = envLocal_;
    setState(STATE_IDLE);
}

TestHwStopcock::~TestHwStopcock()
{
}

DataServiceActionStatus TestHwStopcock::actStart(QVariantList testParamArgs, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "TestHwStopcock-Start", QString().asprintf("%s", Util::qVarientToJsonData(testParamArgs).CSTR()));

    DS_TestDef::TestStatus testStatus = env->ds.testData->getTestStatus();
    testStatus.guid = actGuid;
    testStatus.type = DS_TestDef::TEST_TYPE_STOPCOCK;
    testStatus.progress = 0;
    testStatus.userAborted = false;
    testStatus.isFinished = false;
    testStatus.stateStr = "";

    if (state != STATE_IDLE)
    {
        status.state = DS_ACTION_STATE_BUSY;
        status.err = "Test In Progress";
        actionStarted(status);

        testStatus.isFinished = true;
        env->ds.testData->setTestStatus(testStatus);
        setState(STATE_IDLE);
        goto bail;
    }
    else if (testParamArgs.length() != 5)
    {
        status.state = DS_ACTION_STATE_BAD_REQUEST;
        status.err = "Bad Parameters";
        actionStarted(status);
        testStatus.isFinished = true;
        env->ds.testData->setTestStatus(testStatus);
        setState(STATE_IDLE);
        goto bail;
    }

    // Set test params
    testParams.cycles = testParamArgs[0].toInt();
    testParams.stopcockEnabled[0] = testParamArgs[1].toBool();
    testParams.stopcockEnabled[1] = testParamArgs[2].toBool();
    testParams.stopcockEnabled[2] = testParamArgs[3].toBool();
    testParams.pauseAfterMoveMs = testParamArgs[4].toInt();

    testParams.curCycleCount = 0;

    if ( (!testParams.stopcockEnabled[0]) &&
         (!testParams.stopcockEnabled[1]) &&
         (!testParams.stopcockEnabled[2]) )
    {
        LOG_ERROR("TEST_STOPCOCK: START: No stopcock is selected\n");
        status.state = DS_ACTION_STATE_BAD_REQUEST;
        status.err = QString().asprintf("No stopcock is selected");
        actionStarted(status);
        testStatus.isFinished = true;
        setState(STATE_IDLE);
        goto bail;
    }

    LOG_INFO("\n\n");
    LOG_INFO("TEST_STOPCOCK: Cycle test started\n");

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);

    env->ds.testData->setTestStatus(testStatus);
    setState(STATE_STARTED);

bail:
    return status;
}

DataServiceActionStatus TestHwStopcock::actStop(QString actGuid)
{
    DataServiceActionStatus status;
    status.guid = actGuid;

    if (state == STATE_IDLE)
    {
        LOG_ERROR("TEST_STOPCOCK: STOP: Test is not running\n");
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

void TestHwStopcock::setStopcockHw()
{
    State progressState, completeState;
    DS_McuDef::StopcockPos newScPos = DS_McuDef::STOPCOCK_POS_UNKNOWN;

    switch (state)
    {
    case STATE_MOVE_INJ_TO_FILL_STARTED:
        progressState = STATE_MOVE_INJ_TO_FILL_PROGRESS;
        completeState = STATE_MOVE_INJ_TO_FILL_DONE;
        newScPos = DS_McuDef::STOPCOCK_POS_FILL;
        break;
    case STATE_MOVE_FILL_TO_INJ_STARTED:
        progressState = STATE_MOVE_FILL_TO_INJ_PROGRESS;
        completeState = STATE_MOVE_FILL_TO_INJ_DONE;
        newScPos = DS_McuDef::STOPCOCK_POS_INJECT;
        break;
    case STATE_MOVE_INJ_TO_CLOSE_STARTED:
        progressState = STATE_MOVE_INJ_TO_CLOSE_PROGRESS;
        completeState = STATE_MOVE_INJ_TO_CLOSE_DONE;
        newScPos = DS_McuDef::STOPCOCK_POS_CLOSED;
        break;
    case STATE_MOVE_CLOSE_TO_FILL_STARTED:
        progressState = STATE_MOVE_CLOSE_TO_FILL_PROGRESS;
        completeState = STATE_MOVE_CLOSE_TO_FILL_DONE;
        newScPos = DS_McuDef::STOPCOCK_POS_FILL;
        break;
    case STATE_MOVE_FILL_TO_CLOSE_STARTED:
        progressState = STATE_MOVE_FILL_TO_CLOSE_PROGRESS;
        completeState = STATE_MOVE_FILL_TO_CLOSE_DONE;
        newScPos = DS_McuDef::STOPCOCK_POS_CLOSED;
        break;
    case STATE_MOVE_CLOSE_TO_INJ_STARTED:
        progressState = STATE_MOVE_CLOSE_TO_INJ_PROGRESS;
        completeState = STATE_MOVE_CLOSE_TO_INJ_DONE;
        newScPos = DS_McuDef::STOPCOCK_POS_INJECT;
        break;
    default:
        LOG_ERROR("TEST_STOPCOCK: Cannot start hw action: bad state(%d)\n", state);
        return;
    }

    QString actionStr = QString().asprintf("Set %s position", ImrParser::ToImr_StopcockPos(newScPos).CSTR());

    DS_TestDef::TestStatus testStatus = env->ds.testData->getTestStatus();
    testStatus.stateStr = QString().asprintf("#%d: Stopcocks: %s started", testParams.curCycleCount + 1, actionStr.CSTR());
    env->ds.testData->setTestStatus(testStatus);
    setState(progressState);

    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            env->actionMgr->onActionCompleted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
                if (state == progressState)
                {
                    DS_TestDef::TestStatus testStatus = env->ds.testData->getTestStatus();
                    if (curStatus.state == DS_ACTION_STATE_COMPLETED)
                    {
                        testStatus.stateStr = QString().asprintf("#%d: Stopcocks: %s complete", testParams.curCycleCount + 1, actionStr.CSTR());
                        env->ds.testData->setTestStatus(testStatus);
                        setState(completeState);
                    }
                    else
                    {
                        LOG_ERROR("TEST_STOPCOCK: Action Failed Complete: %s\n", ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());
                        testStatus.stateStr = QString().asprintf("#%d: ERROR: Stopcock: %s complete failed (%s)", testParams.curCycleCount + 1, actionStr.CSTR(), ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());
                        env->ds.testData->setTestStatus(testStatus);
                        setState(STATE_ABORTED);
                    }
                }
            });
        }
        else
        {
            LOG_ERROR("TEST_STOPCOCK: Action Failed Start: %s\n", ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());
            DS_TestDef::TestStatus testStatus = env->ds.testData->getTestStatus();
            testStatus.stateStr = QString().asprintf("#%d: ERROR: Stopcock: %s start failed (%s)", testParams.curCycleCount + 1, actionStr.CSTR(), ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());
            env->ds.testData->setTestStatus(testStatus);
            setState(STATE_ABORTED);
        }
    });

    DS_McuDef::ActStopcockParams params;
    for (int i = 0; i < params.posAll.length(); i++)
    {
        params.posAll[i] = testParams.stopcockEnabled[i] ? newScPos : DS_McuDef::STOPCOCK_POS_NONE;
    }

    env->ds.deviceAction->actStopcock(params, STOPCOCK_ACTION_TRIALS_LIMIT, guid);
}

void TestHwStopcock::setStateSynch(int newState)
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

void TestHwStopcock::processState()
{
    DS_TestDef::TestStatus testStatus = env->ds.testData->getTestStatus();

    switch (state)
    {
    case STATE_IDLE:
        LOG_INFO("TEST_STOPCOCK: STATE_IDLE\n");
        testStatus.guid = EMPTY_GUID;
        testStatus.type = DS_TestDef::TEST_TYPE_UNKNOWN;
        env->ds.testData->setTestStatus(testStatus);
        break;
    case STATE_STARTED:
        LOG_INFO("TEST_STOPCOCK: STATE_STARTED\n");
        testStatus.stateStr = QString().asprintf("Test started - %d cycles", testParams.cycles);
        env->ds.testData->setTestStatus(testStatus);
        setState(STATE_MOVE_INJ_TO_FILL_STARTED);
        break;
    case STATE_REPEATING:
        LOG_INFO("TEST_STOPCOCK: STATE_REPEATING\n");
        setState(STATE_MOVE_INJ_TO_FILL_STARTED);
        break;
    case STATE_MOVE_INJ_TO_FILL_STARTED:
        LOG_INFO("TEST_STOPCOCK: STATE_MOVE_INJ_TO_FILL_STARTED\n");
        setStopcockHw();
        break;
    case STATE_MOVE_INJ_TO_FILL_PROGRESS:
        LOG_INFO("TEST_STOPCOCK: STATE_MOVE_INJ_TO_FILL_PROGRESS\n");
        break;
    case STATE_MOVE_INJ_TO_FILL_DONE:
        LOG_INFO("TEST_STOPCOCK: STATE_MOVE_INJ_TO_FILL_DONE\n");
        setState(STATE_MOVE_INJ_TO_FILL_PAUSE_STARTED);
        break;
    case STATE_MOVE_INJ_TO_FILL_PAUSE_STARTED:
        LOG_INFO("TEST_STOPCOCK: STATE_MOVE_INJ_TO_FILL_PAUSE_STARTED\n");
        QTimer::singleShot(testParams.pauseAfterMoveMs, [=] {
            setState(STATE_MOVE_INJ_TO_FILL_PAUSE_DONE);
        });
        setState(STATE_MOVE_INJ_TO_FILL_PAUSE_PROGRESS);
        break;
    case STATE_MOVE_INJ_TO_FILL_PAUSE_PROGRESS:
        LOG_INFO("TEST_STOPCOCK: STATE_MOVE_INJ_TO_FILL_PAUSE_PROGRESS\n");
        break;
    case STATE_MOVE_INJ_TO_FILL_PAUSE_DONE:
        LOG_INFO("TEST_STOPCOCK: STATE_MOVE_INJ_TO_FILL_PAUSE_DONE\n");
        setState(STATE_MOVE_FILL_TO_INJ_STARTED);
        break;
    case STATE_MOVE_FILL_TO_INJ_STARTED:
        LOG_INFO("TEST_STOPCOCK: STATE_MOVE_FILL_TO_INJ_STARTED\n");
        setStopcockHw();
        break;
    case STATE_MOVE_FILL_TO_INJ_PROGRESS:
        LOG_INFO("TEST_STOPCOCK: STATE_MOVE_FILL_TO_INJ_PROGRESS\n");
        break;
    case STATE_MOVE_FILL_TO_INJ_DONE:
        LOG_INFO("TEST_STOPCOCK: STATE_MOVE_FILL_TO_INJ_DONE\n");
        setState(STATE_MOVE_FILL_TO_INJ_PAUSE_STARTED);
        break;
    case STATE_MOVE_FILL_TO_INJ_PAUSE_STARTED:
        LOG_INFO("TEST_STOPCOCK: STATE_MOVE_FILL_TO_INJ_PAUSE_STARTED\n");
        QTimer::singleShot(testParams.pauseAfterMoveMs, [=] {
            setState(STATE_MOVE_FILL_TO_INJ_PAUSE_DONE);
        });
        setState(STATE_MOVE_FILL_TO_INJ_PAUSE_PROGRESS);
        break;
    case STATE_MOVE_FILL_TO_INJ_PAUSE_PROGRESS:
        LOG_INFO("TEST_STOPCOCK: STATE_MOVE_FILL_TO_INJ_PAUSE_PROGRESS\n");
        break;
    case STATE_MOVE_FILL_TO_INJ_PAUSE_DONE:
        LOG_INFO("TEST_STOPCOCK: STATE_MOVE_FILL_TO_INJ_PAUSE_DONE\n");
        setState(STATE_MOVE_INJ_TO_CLOSE_STARTED);
        break;
    case STATE_MOVE_INJ_TO_CLOSE_STARTED:
        LOG_INFO("TEST_STOPCOCK: STATE_MOVE_INJ_TO_CLOSE_STARTED\n");
        setStopcockHw();
        break;
    case STATE_MOVE_INJ_TO_CLOSE_PROGRESS:
        LOG_INFO("TEST_STOPCOCK: STATE_MOVE_INJ_TO_CLOSE_PROGRESS\n");
        break;
    case STATE_MOVE_INJ_TO_CLOSE_DONE:
        LOG_INFO("TEST_STOPCOCK: STATE_MOVE_INJ_TO_CLOSE_DONE\n");
        setState(STATE_MOVE_INJ_TO_CLOSE_PAUSE_STARTED);
        break;
    case STATE_MOVE_INJ_TO_CLOSE_PAUSE_STARTED:
        LOG_INFO("TEST_STOPCOCK: STATE_MOVE_INJ_TO_CLOSE_PAUSE_STARTED\n");
        QTimer::singleShot(testParams.pauseAfterMoveMs, [=] {
            setState(STATE_MOVE_INJ_TO_CLOSE_PAUSE_DONE);
        });
        setState(STATE_MOVE_INJ_TO_CLOSE_PAUSE_PROGRESS);
        break;
    case STATE_MOVE_INJ_TO_CLOSE_PAUSE_PROGRESS:
        LOG_INFO("TEST_STOPCOCK: STATE_MOVE_INJ_TO_CLOSE_PAUSE_PROGRESS\n");
        break;
    case STATE_MOVE_INJ_TO_CLOSE_PAUSE_DONE:
        LOG_INFO("TEST_STOPCOCK: STATE_MOVE_INJ_TO_CLOSE_PAUSE_DONE\n");
        setState(STATE_MOVE_CLOSE_TO_FILL_STARTED);
        break;
    case STATE_MOVE_CLOSE_TO_FILL_STARTED:
        LOG_INFO("TEST_STOPCOCK: STATE_MOVE_CLOSE_TO_FILL_STARTED\n");
        setStopcockHw();
        break;
    case STATE_MOVE_CLOSE_TO_FILL_PROGRESS:
        LOG_INFO("TEST_STOPCOCK: STATE_MOVE_CLOSE_TO_FILL_PROGRESS\n");
        break;
    case STATE_MOVE_CLOSE_TO_FILL_DONE:
        LOG_INFO("TEST_STOPCOCK: STATE_MOVE_CLOSE_TO_FILL_DONE\n");
        setState(STATE_MOVE_CLOSE_TO_FILL_PAUSE_STARTED);
        break;
    case STATE_MOVE_CLOSE_TO_FILL_PAUSE_STARTED:
        LOG_INFO("TEST_STOPCOCK: STATE_MOVE_CLOSE_TO_FILL_PAUSE_STARTED\n");
        QTimer::singleShot(testParams.pauseAfterMoveMs, [=] {
            setState(STATE_MOVE_CLOSE_TO_FILL_PAUSE_DONE);
        });
        setState(STATE_MOVE_CLOSE_TO_FILL_PAUSE_PROGRESS);
        break;
    case STATE_MOVE_CLOSE_TO_FILL_PAUSE_PROGRESS:
        LOG_INFO("TEST_STOPCOCK: STATE_MOVE_CLOSE_TO_FILL_PAUSE_PROGRESS\n");
        break;
    case STATE_MOVE_CLOSE_TO_FILL_PAUSE_DONE:
        LOG_INFO("TEST_STOPCOCK: STATE_MOVE_CLOSE_TO_FILL_PAUSE_DONE\n");
        setState(STATE_MOVE_FILL_TO_CLOSE_STARTED);
        break;
    case STATE_MOVE_FILL_TO_CLOSE_STARTED:
        LOG_INFO("TEST_STOPCOCK: STATE_MOVE_FILL_TO_CLOSE_STARTED\n");
        setStopcockHw();
        break;
    case STATE_MOVE_FILL_TO_CLOSE_PROGRESS:
        LOG_INFO("TEST_STOPCOCK: STATE_MOVE_FILL_TO_CLOSE_PROGRESS\n");
        break;
    case STATE_MOVE_FILL_TO_CLOSE_DONE:
        LOG_INFO("TEST_STOPCOCK: STATE_MOVE_FILL_TO_CLOSE_DONE\n");
        setState(STATE_MOVE_FILL_TO_CLOSE_PAUSE_STARTED);
        break;
    case STATE_MOVE_FILL_TO_CLOSE_PAUSE_STARTED:
        LOG_INFO("TEST_STOPCOCK: STATE_MOVE_FILL_TO_CLOSE_PAUSE_STARTED\n");
        QTimer::singleShot(testParams.pauseAfterMoveMs, [=] {
            setState(STATE_MOVE_FILL_TO_CLOSE_PAUSE_DONE);
        });
        setState(STATE_MOVE_FILL_TO_CLOSE_PAUSE_PROGRESS);
        break;
    case STATE_MOVE_FILL_TO_CLOSE_PAUSE_PROGRESS:
        LOG_INFO("TEST_STOPCOCK: STATE_MOVE_FILL_TO_CLOSE_PAUSE_PROGRESS\n");
        break;
    case STATE_MOVE_FILL_TO_CLOSE_PAUSE_DONE:
        LOG_INFO("TEST_STOPCOCK: STATE_MOVE_FILL_TO_CLOSE_PAUSE_DONE\n");
        setState(STATE_MOVE_CLOSE_TO_INJ_STARTED);
        break;
    case STATE_MOVE_CLOSE_TO_INJ_STARTED:
        LOG_INFO("TEST_STOPCOCK: STATE_MOVE_CLOSE_TO_INJ_STARTED\n");
        setStopcockHw();
        break;
    case STATE_MOVE_CLOSE_TO_INJ_PROGRESS:
        LOG_INFO("TEST_STOPCOCK: STATE_MOVE_CLOSE_TO_INJ_PROGRESS\n");
        break;
    case STATE_MOVE_CLOSE_TO_INJ_DONE:
        LOG_INFO("TEST_STOPCOCK: STATE_MOVE_CLOSE_TO_INJ_DONE\n");
        setState(STATE_MOVE_CLOSE_TO_INJ_PAUSE_STARTED);
        break;
    case STATE_MOVE_CLOSE_TO_INJ_PAUSE_STARTED:
        LOG_INFO("TEST_STOPCOCK: STATE_MOVE_CLOSE_TO_INJ_PAUSE_STARTED\n");
        QTimer::singleShot(testParams.pauseAfterMoveMs, [=] {
            setState(STATE_MOVE_CLOSE_TO_INJ_PAUSE_DONE);
        });
        setState(STATE_MOVE_CLOSE_TO_INJ_PAUSE_PROGRESS);
        break;
    case STATE_MOVE_CLOSE_TO_INJ_PAUSE_PROGRESS:
        LOG_INFO("TEST_STOPCOCK: STATE_MOVE_CLOSE_TO_INJ_PAUSE_PROGRESS\n");
        break;
    case STATE_MOVE_CLOSE_TO_INJ_PAUSE_DONE:
        LOG_INFO("TEST_STOPCOCK: STATE_MOVE_CLOSE_TO_INJ_PAUSE_DONE\n");
        setState(STATE_COMPLETED);
        break;
    case STATE_COMPLETED:
        LOG_INFO("TEST_STOPCOCK: STATE_COMPLETED\n");
        testParams.curCycleCount++;
        testStatus.progress = (testParams.curCycleCount * 100) / testParams.cycles;
        testStatus.stateStr = QString().asprintf("Cycle completed %d/%d, progress=%d%%", testParams.curCycleCount, testParams.cycles, testStatus.progress);
        env->ds.testData->setTestStatus(testStatus);

        if (testParams.curCycleCount >= testParams.cycles)
        {
            setState(STATE_ALL_COMPLETED);
        }
        else
        {
            setState(STATE_REPEATING);
        }
        break;
    case STATE_ALL_COMPLETED:
        {
            LOG_INFO("TEST_STOPCOCK: STATE_ALL_COMPLETED\n");
            testStatus.progress = 100;
            testStatus.stateStr = "All cycles completed\n";
            testStatus.isFinished = true;
            env->ds.testData->setTestStatus(testStatus);

            DataServiceActionStatus status;
            status.guid = testStatus.guid;
            status.state = DS_ACTION_STATE_COMPLETED;

            actionCompleted(status);
            setState(STATE_IDLE);
        }
        break;
    case STATE_ABORTED:
        {
            LOG_INFO("TEST_STOPCOCK: STATE_ABORTED\n");
            DataServiceActionStatus status;
            status.guid = testStatus.guid;
            if (testStatus.userAborted)
            {
                status.state = DS_ACTION_STATE_ALLSTOP_BTN_ABORT;
                testStatus.stateStr = QString().asprintf("#%d: ERROR: Test aborted by user", testParams.curCycleCount + 1);
            }
            else
            {
                status.state = DS_ACTION_STATE_INTERNAL_ERR;
                testStatus.stateStr = QString().asprintf("#%d: ERROR: Test aborted with error", testParams.curCycleCount + 1);
            }
            testStatus.isFinished = true;
            testStatus.userAborted = false;
            env->ds.testData->setTestStatus(testStatus);
            actionCompleted(status);
            setState(STATE_IDLE);
        }
        break;
    default:
        LOG_ERROR("TEST_STOPCOCK: Unknown state(%d)\n", state);
        break;
    }
}


