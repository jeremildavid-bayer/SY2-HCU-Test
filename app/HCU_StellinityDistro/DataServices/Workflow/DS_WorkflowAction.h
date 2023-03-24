#ifndef DS_WORKFLOW_ACTION_H
#define DS_WORKFLOW_ACTION_H

#include "Common/Common.h"
#include "Common/ActionBase.h"
#include "DS_WorkflowDef.h"
#include "Internal/WorkflowMain.h"
#include "DataServices/Device/DS_DeviceDef.h"

class DS_WorkflowAction : public ActionBase
{
    Q_OBJECT

public:
    explicit DS_WorkflowAction(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~DS_WorkflowAction();

    DataServiceActionStatus actBottleLoad(SyringeIdx bottleIdx, DS_DeviceDef::FluidPackage package, QString actGuid = "");
    DataServiceActionStatus actForceFill(SyringeIdx syringeIdx, QString actGuid = "");
    DataServiceActionStatus actAutoPrime(QString actGuid = "");
    DataServiceActionStatus actAutoEmpty(QString actGuid = "");
    DataServiceActionStatus actMudsEject(QString actGuid = "");
    DataServiceActionStatus actSodRestart(QString actGuid = "");
    DataServiceActionStatus actSodAbort(DS_WorkflowDef::SodErrorState sodError, QString actGuid = "");
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

    // Simulator Only
    void setSystemToReady();

private:
    WorkflowMain *workflowMain;


private slots:
    void slotAppInitialised();
};

#endif // DS_WORKFLOW_ACTION_H
