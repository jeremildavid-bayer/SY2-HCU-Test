#ifndef WORKFLOW_MAIN_H
#define WORKFLOW_MAIN_H

#include "Common/Common.h"
#include "DataServices/Workflow/DS_WorkflowDef.h"
#include "DataServices/Device/DS_DeviceDef.h"
#include "WorkflowSod.h"
#include "WorkflowMudsEject.h"
#include "WorkflowAdvanceProtocol.h"
#include "WorkflowSudsAirRecovery.h"
#include "WorkflowFluidRemoval.h"
#include "WorkflowEndOfDayPurge.h"
#include "WorkflowAutoEmpty.h"
#include "WorkflowSyringeAirRecovery.h"
#include "WorkflowManualQualifiedDischarge.h"
#include "WorkflowAutomaticQualifiedDischarge.h"
#include "WorkflowBattery.h"
#include "WorkflowShippingMode.h"
#include "WorkflowPreloadProtocol.h"

class WorkflowMain : public ActionBaseExt
{
    Q_OBJECT

public:
    explicit WorkflowMain(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~WorkflowMain();

    DataServiceActionStatus actAutoPrime(QString actGuid = "");
    DataServiceActionStatus actAutoEmpty(QString actGuid = "");
    DataServiceActionStatus actMudsSodWithNewFluid(QString actGuid = "");
    DataServiceActionStatus actMudsEject(QString actGuid = "");
    DataServiceActionStatus actSodRestart(QString actGuid = "");
    DataServiceActionStatus actSodAbort(DS_WorkflowDef::SodErrorState sodError, QString actGuid = "");
    DataServiceActionStatus actForceFill(SyringeIdx syringeIdx, QString actGuid = "");
    DataServiceActionStatus actSetAutoPrimeComplete(QString actGuid = "");
    DataServiceActionStatus actSudsAirRecoveryStart(QString actGuid = "");
    DataServiceActionStatus actSudsAirRecoveryResume(QString actGuid = "");
    DataServiceActionStatus actSyringeAirRecoveryStart(QString actGuid = "");
    DataServiceActionStatus actSyringeAirRecoveryResume(QString actGuid = "");
    DataServiceActionStatus actFluidRemovalStart(SyringeIdx syringeIdx, bool isAutoEmpty = false, QString actGuid = "");
    DataServiceActionStatus actFluidRemovalResume(QString actGuid = "");
    DataServiceActionStatus actFluidRemovalAbort(QString actGuid = "");
    DataServiceActionStatus actEndOfDayPurgeStart(QString actGuid = "");
    DataServiceActionStatus actEndOfDayPurgeResume(QString actGuid = "");
    DataServiceActionStatus actEndOfDayPurgeAbort(QString actGuid = "");
    DataServiceActionStatus actQueAutomaticQualifiedDischarge(int batteryIdx, QString actGuid = "");
    DataServiceActionStatus actCancelAutomaticQualifiedDischarge(QString actGuid = "");
    DataServiceActionStatus actWorkflowBatteryChargeToStart(int batteryIdx, int target, QString actGuid = "");
    DataServiceActionStatus actWorkflowBatteryAction(int batteryIdx, QString action, QString actGuid = "");
    DataServiceActionStatus actWorkflowBatteryAbort(QString actGuid = "");
    DataServiceActionStatus actManualQualifiedDischargeStart(int batteryIdx, DS_WorkflowDef::ManualQualifiedDischargeMethod methodType, QString actGuid = "");
    DataServiceActionStatus actManualQualifiedDischargeResume(QString actGuid = "");
    DataServiceActionStatus actManualQualifiedDischargeAbort(QString actGuid = "");
    DataServiceActionStatus actShippingModeStart(int batteryIdx, int targetCharge, QString actGuid = "");
    DataServiceActionStatus actShippingModeResume(QString actGuid = "");
    DataServiceActionStatus actShippingModeAbort(QString actGuid = "");
    DataServiceActionStatus actPreloadProtocolStart(bool userConfirmRequired, QString actGuid = "");
    DataServiceActionStatus actPreloadProtocolResume(QString actGuid = "");
    DataServiceActionStatus actPreloadProtocolAbort(QString actGuid = "");


    // used only for simulator
    void setSystemToReady();

private:
    WorkflowSod *workflowSod;
    WorkflowMudsEject *workflowMudsEject;
    WorkflowAdvanceProtocol *workflowAdvanceProtocol;
    WorkflowSudsAirRecovery *workflowSudsAirRecovery;
    WorkflowFluidRemoval *workflowFluidRemoval;
    WorkflowEndOfDayPurge *workflowEndOfDayPurge;
    WorkflowAutoEmpty *workflowAutoEmpty;
    WorkflowSyringeAirRecovery *workflowSyringeAirRecovery;
    WorkflowManualQualifiedDischarge *workflowManualQualifiedDischarge;
    WorkflowAutomaticQualifiedDischarge *workflowAutomaticQualifiedDischarge;
    WorkflowBattery *workflowBattery;
    WorkflowShippingMode *workflowShippingMode;
    WorkflowPreloadProtocol *workflowPreloadProtocol;

    SyringeIdx fluidRemovalIndex;

    QString sudsMonitorGuid;
    QString syringesBusyMonitorGuid;
    QString syringeVolMonitorGuid;
    QString examEndedMonitorGuid;
    QString statePathMonitorGuid;
    QString mudsInsertWaitingMonitorGuid;

    void activate();
    void waitForSyringesToBecomeIdle(DS_WorkflowDef::WorkflowState curState);
    void waitForSudsToBeInserted(DS_WorkflowDef::WorkflowState curState);
    void waitForSudsToBeRemoved(DS_WorkflowDef::WorkflowState curState);
    void waitAndHandleExamEnded();
    void handleMudsUnlatch(bool curLatched, bool prevLatched);
    bool isMotorPositionFaultActive();
    void clearMonitorConnections();
    void resetWorkflowStatus();
    void updateWorkflowErrorStatusFromSudsRemoved(const DS_DeviceDef::FluidSource &fluidSourceSuds, const DS_DeviceDef::FluidSource &prevFluidSourceSuds);
    void handleSudsReinsertion();
    void startDelayedMudsEject(int delayMs = 2000);
    bool isMudsPrimeStartRequired();
    void resetWorkflowState();
    bool allInjectionsCompleted();
    void freeAutoPrime();

    int getState();
    QString getStateStr(int state);
    void setStateSynch(int newState);
    bool isSetStateReady();
    void processState();

    void clearBottleFillCount();
    void incrementBottleFillCount();
    bool isNewBottleFill(const SyringeIdx syringeIdx);

private slots:
    void slotAppInitialised();

};

#endif //WORKFLOW_MAIN_H
