#include "Apps/AppManager.h"
#include "DS_TestData.h"

DS_TestData::DS_TestData(QObject *parent, EnvGlobal *env_) :
    QObject(parent),
    env(env_)
{
    envLocal = new EnvLocal("DS_Test-Data", "TEST_DATA");

    connect(env->appManager, &AppManager::signalAppStarted, this, [=]() {
        slotAppStarted();
    });

    m_DataLocked = false;

    m_TestStatus.guid = EMPTY_GUID;
    m_TestStatus.type = DS_TestDef::TEST_TYPE_UNKNOWN;
    m_TestStatus.stateStr = "";
    m_TestStatus.progress = 0;
    m_TestStatus.isFinished = false;
    m_TestStatus.userAborted = false;

    SET_LAST_DATA(DataLocked)
    SET_LAST_DATA(TestStatus)
}

DS_TestData::~DS_TestData()
{
    delete envLocal;
}

void DS_TestData::slotAppStarted()
{
    EMIT_DATA_CHANGED_SIGNAL(TestStatus)
}

