#ifndef WORKFLOW_PRELOAD_PROTOCOL_H
#define WORKFLOW_PRELOAD_PROTOCOL_H

#include "Common/Common.h"
#include "Common/ActionBaseExt.h"
#include "DataServices/Workflow/DS_WorkflowDef.h"

class WorkflowPreloadProtocol : public ActionBaseExt
{
    Q_OBJECT

public:
    explicit WorkflowPreloadProtocol(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~WorkflowPreloadProtocol();

    DataServiceActionStatus actPreloadProtocolStart(bool userConfirmRequired = true, QString actGuid = "");
    DataServiceActionStatus actPreloadProtocolResume(QString actGuid = "");
    DataServiceActionStatus actPreloadProtocolAbort(QString actGuid = "");
    
private:
    int getState();
    QString getStateStr(int state);
    void setStateSynch(int newState);
    bool isSetStateReady();
    void processState();
    QString guidInjectionEndWaiting;
    bool userConfirmRequired;

private slots:
    void slotAppInitialised();
};

#endif //WORKFLOW_PRELOAD_PROTOCOL_H
