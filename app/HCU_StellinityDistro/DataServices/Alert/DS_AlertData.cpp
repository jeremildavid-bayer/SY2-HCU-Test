#include "Apps/AppManager.h"
#include "DS_AlertData.h"
#include "DS_AlertAction.h"

DS_AlertData::DS_AlertData(QObject *parent, EnvGlobal *env_):
        QObject(parent),
        env(env_)
{
    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });

    m_DataLocked = false;

    SET_LAST_DATA(DataLocked)
    SET_LAST_DATA(AllAlerts)
    SET_LAST_DATA(InactiveAlerts)
    SET_LAST_DATA(ActiveAlerts)
    SET_LAST_DATA(ActiveSystemAlerts)
    SET_LAST_DATA(ActiveFatalAlerts)
    SET_LAST_DATA(ActiveFluidSourceAlerts)
}

DS_AlertData::~DS_AlertData()
{
}

void DS_AlertData::slotAppInitialised()
{
    //EMIT_DATA_CHANGED_SIGNAL(DataLocked)
    EMIT_DATA_CHANGED_SIGNAL(ActiveAlerts)
}

