#ifndef WORKFLOW_AUTO_EMPTY_H
#define WORKFLOW_AUTO_EMPTY_H

#include "Common/ActionBaseExt.h"
#include "DataServices/Workflow/DS_WorkflowDef.h"

class WorkflowAutoEmpty : public ActionBaseExt
{
    Q_OBJECT

public:
    explicit WorkflowAutoEmpty(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~WorkflowAutoEmpty();

    DataServiceActionStatus actAutoEmptyStart(QString actGuid = "");


private:
    int getState();
    QString getStateStr(int state);
    void setStateSynch(int newState);
    bool isSetStateReady();
    void processState();

    bool shouldAutoEmptySyringe(SyringeIdx idx);
    void fillAutoEmptySyringeList(QList<SyringeIdx> &autoEmptySyringeList);

    SyringeIdx emptySyringeIdx;
    QList<SyringeIdx> autoEmptySyringeIdxList;

private slots:
    void slotAppInitialised();
};

#endif //WORKFLOW_AUTO_EMPTY_H
