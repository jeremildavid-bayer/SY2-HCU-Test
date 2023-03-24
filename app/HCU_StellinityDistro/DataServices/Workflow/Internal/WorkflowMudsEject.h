#ifndef WORKFLOW_MUDS_EJECT_H
#define WORKFLOW_MUDS_EJECT_H

#include "Common/Common.h"
#include "Common/ActionBaseExt.h"
#include "DataServices/Workflow/DS_WorkflowDef.h"
#include "DataServices/Device/DS_DeviceDef.h"

class WorkflowMudsEject : public ActionBaseExt
{
    Q_OBJECT

public:
    explicit WorkflowMudsEject(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~WorkflowMudsEject();

    DataServiceActionStatus actMudsEjectStart(QString actGuid = "");

private:
    DS_DeviceDef::FluidSources fluidSourceSyringesBeforeEject;
    QString guidSyringesIdleWaiting;
    QString guidStatePathIdleWaiting;

    int getState();
    QString getStateStr(int state);
    void setStateSynch(int newState);
    bool isSetStateReady();
    void processState();

private slots:
    void slotAppInitialised();
};

#endif //WORKFLOW_MUDS_EJECT_H
