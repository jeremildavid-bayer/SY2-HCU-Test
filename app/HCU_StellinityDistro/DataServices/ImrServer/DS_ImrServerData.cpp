#include "Apps/AppManager.h"
#include "DS_ImrServerData.h"

DS_ImrServerData::DS_ImrServerData(QObject *parent, EnvGlobal *env_) :
    QObject(parent),
    env(env_)
{
    envLocal = new EnvLocal("DS_ImrServer-Data", "IMR_SERVER_DATA");

    connect(env->appManager, &AppManager::signalAppStarted, this, [=]() {
        slotAppStarted();
    });

    m_DataLocked = false;
    m_ClientCount = 0;

    SET_LAST_DATA(LastImrCruRequests)
    SET_LAST_DATA(ClientCount)
}

DS_ImrServerData::~DS_ImrServerData()
{
    delete envLocal;
}

void DS_ImrServerData::slotAppStarted()
{
    EMIT_DATA_CHANGED_SIGNAL(LastImrCruRequests)
    EMIT_DATA_CHANGED_SIGNAL(ClientCount)
}

