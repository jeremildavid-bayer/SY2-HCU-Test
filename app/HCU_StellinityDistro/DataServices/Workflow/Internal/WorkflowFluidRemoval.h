#ifndef WORKFLOW_FLUID_REMOVAL_H
#define WORKFLOW_FLUID_REMOVAL_H

#include "Common/ActionBaseExt.h"
#include "DataServices/Workflow/DS_WorkflowDef.h"

class WorkflowFluidRemoval : public ActionBaseExt
{
    Q_OBJECT

public:
    explicit WorkflowFluidRemoval(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~WorkflowFluidRemoval();

    DataServiceActionStatus actFluidRemovalStart(SyringeIdx syringeIdx, bool isAutoEmpty = false, QString actGuid = "");
    DataServiceActionStatus actFluidRemovalResume(QString actGuid = "");
    DataServiceActionStatus actFluidRemovalAbort(QString actGuid = "");

private:
    SyringeIdx location;
    QString guidSudsWaiting;
    QString guidWasteContainerMonitor;
    bool isAutoEmpty;

    DataServiceActionStatus actStatusBeforeResumed;

    int getState();
    QString getStateStr(int state);
    void setStateSynch(int newState);
    bool isSetStateReady();
    void processState();

private slots:
    void slotAppInitialised();
};

#endif //WORKFLOW_FLUID_REMOVAL_H
