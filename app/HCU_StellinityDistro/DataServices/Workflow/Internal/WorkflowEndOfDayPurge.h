#ifndef WORKFLOW_END_OF_DAY_PURGE_H
#define WORKFLOW_END_OF_DAY_PURGE_H

#include "Common/ActionBaseExt.h"
#include "DataServices/Workflow/DS_WorkflowDef.h"

class WorkflowEndOfDayPurge : public ActionBaseExt
{
    Q_OBJECT

public:
    explicit WorkflowEndOfDayPurge(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~WorkflowEndOfDayPurge();

    DataServiceActionStatus actEndOfDayPurgeStart(QString actGuid = "");
    DataServiceActionStatus actEndOfDayPurgeResume(QString actGuid = "");
    DataServiceActionStatus actEndOfDayPurgeAbort(QString actGuid = "");

private:
    QList<double> purgeStartSyringeVols;
    QString guidWasteContainerMonitor;
    // Emopty Contrast First Function
    bool emptyContrastFirstEnabled;
    QList<bool> purgeSyringe;

    int getState();
    QString getStateStr(int state);
    void setStateSynch(int newState);
    bool isSetStateReady();
    void processState();

private slots:
    void slotAppInitialised();
};

#endif //WORKFLOW_END_OF_DAY_PURGE_H
