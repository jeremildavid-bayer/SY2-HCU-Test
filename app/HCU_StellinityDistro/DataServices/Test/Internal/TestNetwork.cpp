#include "TestNetwork.h"
#include "Common/ImrParser.h"
#include "DataServices/Test/DS_TestData.h"

TestNetwork::TestNetwork(QObject *parent, EnvGlobal *env_, EnvLocal *envLocal_) :
    ActionBaseExt(parent, env_)
{
    envLocal = envLocal_;
    setState(STATE_IDLE);
}


TestNetwork::~TestNetwork()
{
}

DataServiceActionStatus TestNetwork::actStart(QVariantList testParamArgs, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "TextNetwork-Start", QString().asprintf("%s", Util::qVarientToJsonData(testParamArgs).CSTR()));

    DS_TestDef::TestStatus testStatus = env->ds.testData->getTestStatus();
    testStatus.guid = actGuid;
    testStatus.type = DS_TestDef::TEST_TYPE_NETWORK;
    testStatus.progress = 0;
    testStatus.userAborted = false;
    testStatus.isFinished = false;
    testStatus.stateStr = "";

    if (state != STATE_IDLE)
    {
        status.state = DS_ACTION_STATE_BUSY;
        status.err = QString().asprintf("Bad state (%d)", state);
        actionStarted(status);
        testStatus.isFinished = true;
        env->ds.testData->setTestStatus(testStatus);
        setState(STATE_IDLE);
        goto bail;
    }
    else if (testParamArgs.length() != 10)
    {
        status.state = DS_ACTION_STATE_BAD_REQUEST;
        status.err = QString().asprintf("Bad parameters");
        actionStarted(status);
        testStatus.isFinished = true;
        env->ds.testData->setTestStatus(testStatus);
        setState(STATE_IDLE);
        goto bail;
    }

    LOG_INFO("\n\n");
    LOG_INFO("TEST_NETWORK: Test started\n");

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);

    env->ds.testData->setTestStatus(testStatus);
    setState(STATE_STARTED);

bail:
    return status;
}

DataServiceActionStatus TestNetwork::actStop(QString actGuid)
{
    DataServiceActionStatus status;
    status.guid = actGuid;

    if (state == STATE_IDLE)
    {
        LOG_ERROR("TEST_NETWORK: STOP: Test is not running\n");
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

void TestNetwork::setStateSynch(int newState)
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

void TestNetwork::processState()
{
    DS_TestDef::TestStatus testStatus = env->ds.testData->getTestStatus();

    switch (state)
    {
    case STATE_IDLE:
        LOG_INFO("TEST_NETWORK: STATE_IDLE\n");
        testStatus.guid = EMPTY_GUID;
        testStatus.type = DS_TestDef::TEST_TYPE_UNKNOWN;
        env->ds.testData->setTestStatus(testStatus);
        break;
    case STATE_STARTED:
        LOG_INFO("TEST_NETWORK: STATE_STARTED\n");
        break;
    case STATE_ABORTED:
        {
            LOG_INFO("TEST_NETWORK: STATE_ABORTED\n");

            DataServiceActionStatus status;
            status.guid = testStatus.guid;
            if (testStatus.userAborted)
            {
                status.state = DS_ACTION_STATE_ALLSTOP_BTN_ABORT;
                testStatus.stateStr = QString().asprintf("WARNING: Test aborted by user");
            }
            else
            {
                status.state = DS_ACTION_STATE_INTERNAL_ERR;
                testStatus.stateStr = QString().asprintf("ERROR: Test aborted with error");
            }
            testStatus.isFinished = true;
            testStatus.userAborted = false;
            env->ds.testData->setTestStatus(testStatus);
            actionCompleted(status);
            setState(STATE_IDLE);
        }
        break;
    default:
        LOG_ERROR("TEST_NETWORK: Unknown state(%d)\n", state);
        break;
    }
}


