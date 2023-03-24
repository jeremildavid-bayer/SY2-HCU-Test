#ifndef WORKFLOW_SYRINGE_AIR_RECOVERY_STATE_H
#define WORKFLOW_SYRINGE_AIR_RECOVERY_STATE_H

#include "Common/Common.h"
#include "Common/ActionBaseExt.h"
#include "DataServices/Workflow/DS_WorkflowDef.h"

class WorkflowSyringeAirRecovery : public ActionBaseExt
{
    Q_OBJECT

public:
    explicit WorkflowSyringeAirRecovery(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~WorkflowSyringeAirRecovery();

    DataServiceActionStatus actSyringeAirRecoveryStart(QString actGuid = "");
    DataServiceActionStatus actSyringeAirRecoveryResume(QString actGuid = "");
    
private:
    SyringeIdx location;
    QString guidSudsWaiting;
    QString guidSyringesIdleWaiting;
    int extraPrimeCount;

    int getState();
    QString getStateStr(int state);
    void setStateSynch(int newState);
    bool isSetStateReady();
    void processState();

private slots:
    void slotAppInitialised();
};

#endif //WORKFLOW_SYRINGE_AIR_RECOVERY_STATE_H
