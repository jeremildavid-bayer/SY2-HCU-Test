#ifndef WORKFLOW_SOD_H
#define WORKFLOW_SOD_H

#include "Common/Common.h"
#include "DataServices/Workflow/DS_WorkflowDef.h"
#include "Common/ActionBaseExt.h"

class WorkflowSod : public ActionBaseExt
{
    Q_OBJECT

public:
    explicit WorkflowSod(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~WorkflowSod();

    DataServiceActionStatus actSodStart(QString actGuid = "");
    DataServiceActionStatus actSodRestart(QString actGuid = "");
    DataServiceActionStatus actSodAbort(DS_WorkflowDef::SodErrorState sodError, QString actGuid = "");
    DataServiceActionStatus actMudsSodWithNewFluid(QString actGuid = "");

private:

    enum State
    {
        STATE_INACTIVE = 0,

        STATE_READY,
        STATE_STARTED,

        // Find Plungers States
        STATE_FIND_PLUNGERS_STARTED,
        STATE_FIND_PLUNGERS_PROGRESS,
        STATE_FIND_PLUNGERS_DONE,
        STATE_FIND_PLUNGERS_FAILED,

        // Identification States
        STATE_IDENTIFYING_MUDS_AGE,

        // Reuse MUDS States
        STATE_REUSE_MUDS_STARTED,
        STATE_REUSE_MUDS_SUDS_REMOVAL_WAITING,

        // Purge Preparing
        STATE_PURGE_PREPARING,
        STATE_PURGE_SUDS_REMOVAL_WAITING,

        // Purge States
        STATE_PURGE_STOPCOCK_SETTING,
        STATE_PURGE_STOPCOCK_PROGRESS,
        STATE_PURGE_STOPCOCK_DONE,
        STATE_PURGE_STOPCOCK_FAILED,
        STATE_PURGE_STARTED,
        STATE_PURGE_PROGRESS,
        STATE_PURGE_SUSPENDED,
        STATE_PURGE_DONE,
        STATE_PURGE_FAILED,

        // Engage States
        STATE_ENGAGE_STARTED,
        STATE_ENGAGE_PROGRESS,
        STATE_ENGAGE_DONE,
        STATE_ENGAGE_FAILED,

        // MUDS Sod States
        STATE_MUDS_SOD_STARTED,
        STATE_MUDS_SOD_PROGRESS,
        STATE_MUDS_SOD_DONE,
        STATE_MUDS_SOD_FAILED,

        STATE_SUSPENDED,
        STATE_FAILED,
        STATE_DONE
    };

    QString guidSudsWaiting;
    QString guidSyringeStatesWaiting;

    bool isSetStateReady();
    void processState();

    // Helpers
    void handleFailedCases();
    bool isSystemBusy();
    void waitForIdleSyringes(State curState, State returnState);

private slots:
    void slotAppInitialised();

};

#endif //WORKFLOW_SOD_H
