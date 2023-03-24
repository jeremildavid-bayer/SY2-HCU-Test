#include "Apps/AppManager.h"
#include "DS_TestAction.h"
#include "DataServices/Test/DS_TestData.h"

DS_TestAction::DS_TestAction(QObject *parent, EnvGlobal *env_) :
    ActionBase(parent, env_)
{
    envLocal = new EnvLocal("DS_Test-Action", "TEST_ACTION");

    testHwPiston = new TestHwPiston(this, env, envLocal);
    testHwStopcock = new TestHwStopcock(this, env, envLocal);
    testNetwork = new TestNetwork(this, env, envLocal);
    testContinuousExams = new TestContinuousExams(this, env, envLocal);

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });
}

DS_TestAction::~DS_TestAction()
{
    delete envLocal;
    delete testHwPiston;
    delete testHwStopcock;
    delete testNetwork;
    delete testContinuousExams;
}

void DS_TestAction::slotAppInitialised()
{
}

DataServiceActionStatus DS_TestAction::actStart(DS_TestDef::TestType type, QVariantList testParams, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "Start", QString().asprintf("%s;%s", ImrParser::ToImr_TestType(type).CSTR(), Util::qVarientToJsonData(testParams).CSTR()));

    if ( (testHwPiston->isBusy()) ||
         (testHwStopcock->isBusy()) ||
         (testNetwork->isBusy()) ||
         (testContinuousExams->isBusy()) )
    {
        status.state = DS_ACTION_STATE_BUSY;
        status.err = "Test In Progress";
        actionStarted(status);
        return status;
    }

    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);

        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            env->actionMgr->onActionCompleted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
                actionCompleted(curStatus, &status);
            });
        }
    });

    switch (type)
    {
    case DS_TestDef::TEST_TYPE_PISTON:
        return testHwPiston->actStart(testParams, guid);
    case DS_TestDef::TEST_TYPE_STOPCOCK:
        return testHwStopcock->actStart(testParams, guid);
    case DS_TestDef::TEST_TYPE_NETWORK:
        return testNetwork->actStart(testParams, guid);
    case DS_TestDef::TEST_TYPE_CONTINUOUS_EXAMS:
        return testContinuousExams->actStart(testParams, guid);
    default:
        status.state = DS_ACTION_STATE_BAD_REQUEST;
        status.err = "Bad test name";
        actionStarted(status);
        return status;
    }
}

DataServiceActionStatus DS_TestAction::actStop(QString actGuid)
{
    DS_TestDef::TestStatus testStatus = env->ds.testData->getTestStatus();
    switch (testStatus.type)
    {
    case DS_TestDef::TEST_TYPE_PISTON:
        return testHwPiston->actStop();
    case DS_TestDef::TEST_TYPE_STOPCOCK:
        return testHwStopcock->actStop();
    case DS_TestDef::TEST_TYPE_NETWORK:
        return testNetwork->actStop();
    case DS_TestDef::TEST_TYPE_CONTINUOUS_EXAMS:
        return testContinuousExams->actStop();
    default:
        {
            DataServiceActionStatus status;
            status.guid = actGuid;
            status.state = DS_ACTION_STATE_BAD_REQUEST;
            status.err = "Bad test name";
            actionStarted(status);
            return status;
        }
    }
}

