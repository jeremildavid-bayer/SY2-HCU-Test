#include "Apps/AppManager.h"
#include "DS_MwlAction.h"
#include "DataServices/Mwl/DS_MwlData.h"
#include "DataServices/Cru/DS_CruAction.h"

DS_MwlAction::DS_MwlAction(QObject *parent, EnvGlobal *env_) :
    ActionBase(parent, env_)
{
    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });

    envLocal = new EnvLocal("DS_Mwl-Action", "MWL_ACTION");
    monitor = new MwlMonitor(this, env);
}

DS_MwlAction::~DS_MwlAction()
{
    delete monitor;
    delete envLocal;
}

void DS_MwlAction::slotAppInitialised()
{
    connect(env->ds.systemData, &DS_SystemData::signalDataChanged_StatePath, this, [=](DS_SystemDef::StatePath curStatePath, DS_SystemDef::StatePath prevStatePath) {
        if ( ( (prevStatePath == DS_SystemDef::STATE_PATH_IDLE) && (curStatePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING) ) ||
             ( (prevStatePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING) && (curStatePath == DS_SystemDef::STATE_PATH_IDLE) ) )
        {
            LOG_INFO("\n\n-----------------\nStatePath=%s\n\n", ImrParser::ToImr_StatePath(curStatePath).CSTR());
        }
    });
}

DataServiceActionStatus DS_MwlAction::actSelectWorklistEntry(QString studyInstanceUid, QString actGuid)
{
    QString url = IMR_CRU_URL_SELECT_WORKLIST_ENTRY + QString().asprintf("?studyInstanceUid=%s", studyInstanceUid.CSTR());
    return env->ds.cruAction->actSendRequest(actGuid, url, "post");
}

DataServiceActionStatus DS_MwlAction::actQueryWorklist(QString actGuid)
{
    // NOTE: Sending two different requests.
    // 1. Asking CRU to send HCU the "current" cache
    // 2. Asking CRU to check with the MWL server to see if CRU needs to update the cache.
    // (1) is due to HCU not having re-try logic when the invalidate-send request times out.
    //     e.g worklist "get" timed out, HCU is not showing matching MWL as CRU but since CRU doesn't know HCU timed out, there is no way HCU can update the list.
    //         Refresh button will force-get the "current" list from CRU
    // (2) sometimes CRU needs to get new data from MWL server. Refresh button will also ask CRU to check for this.
    // When both cases are true, HCU will receive two list data (1. current cache from CRU, 2. updated cache from CRU)
    env->ds.cruAction->actSendRequest(actGuid, IMR_CRU_URL_WORKLIST, "get");
    return env->ds.cruAction->actSendRequest(actGuid, IMR_CRU_URL_QUERY_WORKLIST, "post");
}

