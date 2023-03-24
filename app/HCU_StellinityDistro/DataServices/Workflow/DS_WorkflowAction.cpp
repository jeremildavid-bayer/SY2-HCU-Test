#include "Apps/AppManager.h"
#include "DS_WorkflowAction.h"
#include "DataServices/Alert/DS_AlertAction.h"
#include "DataServices/Workflow/DS_WorkflowData.h"
#include "DataServices/Device/DS_DeviceAction.h"
#include "DataServices/Exam/DS_ExamData.h"

DS_WorkflowAction::DS_WorkflowAction(QObject *parent, EnvGlobal *env_) :
    ActionBase(parent, env_)
{
    envLocal = new EnvLocal("DS_Workflow-Action", "WORKFLOW_ACTION");

    workflowMain = new WorkflowMain(this, env);

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });
}

DS_WorkflowAction::~DS_WorkflowAction()
{
    delete workflowMain;
    delete envLocal;
}

void DS_WorkflowAction::slotAppInitialised()
{
    connect(env->ds.systemData, &DS_SystemData::signalDataChanged_StatePath, this, [=](DS_SystemDef::StatePath curStatePath, DS_SystemDef::StatePath prevStatePath) {
        if ( ( (prevStatePath == DS_SystemDef::STATE_PATH_IDLE) && (curStatePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING) ) ||
             ( (prevStatePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING) && (curStatePath == DS_SystemDef::STATE_PATH_IDLE) ) )
        {
            LOG_INFO("\n\n-----------------\nStatePath=%s\n\n", ImrParser::ToImr_StatePath(curStatePath).CSTR());
        }
    });
}

DataServiceActionStatus DS_WorkflowAction::actBottleLoad(SyringeIdx bottleIdx, DS_DeviceDef::FluidPackage package, QString actGuid)
{
    return env->ds.deviceAction->actBottleLoad(bottleIdx, package, actGuid);
}

DataServiceActionStatus DS_WorkflowAction::actForceFill(SyringeIdx syringeIdx, QString actGuid)
{
    return workflowMain->actForceFill(syringeIdx, actGuid);
}

DataServiceActionStatus DS_WorkflowAction::actAutoPrime(QString actGuid)
{
    return workflowMain->actAutoPrime(actGuid);
}

DataServiceActionStatus DS_WorkflowAction::actAutoEmpty(QString actGuid)
{
    return workflowMain->actAutoEmpty(actGuid);
}

DataServiceActionStatus DS_WorkflowAction::actMudsEject(QString actGuid)
{
    return workflowMain->actMudsEject(actGuid);
}

DataServiceActionStatus DS_WorkflowAction::actSodRestart(QString actGuid)
{
    return workflowMain->actSodRestart(actGuid);
}

DataServiceActionStatus DS_WorkflowAction::actSodAbort(DS_WorkflowDef::SodErrorState sodError, QString actGuid)
{
    return workflowMain->actSodAbort(sodError, actGuid);
}

DataServiceActionStatus DS_WorkflowAction::actSetAutoPrimeComplete(QString actGuid)
{
    return workflowMain->actSetAutoPrimeComplete(actGuid);
}

DataServiceActionStatus DS_WorkflowAction::actSudsAirRecoveryStart(QString actGuid)
{
    return workflowMain->actSudsAirRecoveryStart(actGuid);
}

DataServiceActionStatus DS_WorkflowAction::actSudsAirRecoveryResume(QString actGuid)
{
    return workflowMain->actSudsAirRecoveryResume(actGuid);
}

DataServiceActionStatus DS_WorkflowAction::actSyringeAirRecoveryStart(QString actGuid)
{
    return workflowMain->actSyringeAirRecoveryStart(actGuid);
}

DataServiceActionStatus DS_WorkflowAction::actSyringeAirRecoveryResume(QString actGuid)
{
    return workflowMain->actSyringeAirRecoveryResume(actGuid);
}

DataServiceActionStatus DS_WorkflowAction::actFluidRemovalStart(SyringeIdx syringeIdx, bool isAutoEmpty, QString actGuid)
{
    return workflowMain->actFluidRemovalStart(syringeIdx, isAutoEmpty, actGuid);
}

DataServiceActionStatus DS_WorkflowAction::actFluidRemovalResume(QString actGuid)
{
    return workflowMain->actFluidRemovalResume(actGuid);
}

DataServiceActionStatus DS_WorkflowAction::actFluidRemovalAbort(QString actGuid)
{
    return workflowMain->actFluidRemovalAbort(actGuid);
}

DataServiceActionStatus DS_WorkflowAction::actEndOfDayPurgeStart(QString actGuid)
{
    return workflowMain->actEndOfDayPurgeStart(actGuid);
}

DataServiceActionStatus DS_WorkflowAction::actEndOfDayPurgeResume(QString actGuid)
{
    return workflowMain->actEndOfDayPurgeResume(actGuid);
}

DataServiceActionStatus DS_WorkflowAction::actEndOfDayPurgeAbort(QString actGuid)
{
    return workflowMain->actEndOfDayPurgeAbort(actGuid);
}

DataServiceActionStatus DS_WorkflowAction::actQueAutomaticQualifiedDischarge(int batteryIdx, QString actGuid)
{
    return workflowMain->actQueAutomaticQualifiedDischarge(batteryIdx, actGuid);
}

DataServiceActionStatus DS_WorkflowAction::actCancelAutomaticQualifiedDischarge(QString actGuid)
{
    return workflowMain->actCancelAutomaticQualifiedDischarge(actGuid);
}

DataServiceActionStatus DS_WorkflowAction::actWorkflowBatteryChargeToStart(int batteryIdx, int target, QString actGuid)
{
    return workflowMain->actWorkflowBatteryChargeToStart(batteryIdx, target, actGuid);
}

DataServiceActionStatus DS_WorkflowAction::actWorkflowBatteryAction(int batteryIdx, QString action, QString actGuid)
{
    return workflowMain->actWorkflowBatteryAction(batteryIdx, action, actGuid);
}

DataServiceActionStatus DS_WorkflowAction::actWorkflowBatteryAbort(QString actGuid)
{
    return workflowMain->actWorkflowBatteryAbort(actGuid);
}

DataServiceActionStatus DS_WorkflowAction::actManualQualifiedDischargeStart(int batteryIdx, DS_WorkflowDef::ManualQualifiedDischargeMethod methodType, QString actGuid)
{
    return workflowMain->actManualQualifiedDischargeStart(batteryIdx, methodType, actGuid);
}

DataServiceActionStatus DS_WorkflowAction::actManualQualifiedDischargeResume(QString actGuid)
{
    return workflowMain->actManualQualifiedDischargeResume(actGuid);
}

DataServiceActionStatus DS_WorkflowAction::actManualQualifiedDischargeAbort(QString actGuid)
{
    return workflowMain->actManualQualifiedDischargeAbort(actGuid);
}

DataServiceActionStatus DS_WorkflowAction::actShippingModeStart(int batteryIdx, int targetCharge, QString actGuid)
{
    return workflowMain->actShippingModeStart(batteryIdx, targetCharge, actGuid);
}

DataServiceActionStatus DS_WorkflowAction::actShippingModeResume(QString actGuid)
{
    return workflowMain->actShippingModeResume(actGuid);
}

DataServiceActionStatus DS_WorkflowAction::actShippingModeAbort(QString actGuid)
{
    return workflowMain->actShippingModeAbort(actGuid);
}

DataServiceActionStatus DS_WorkflowAction::actPreloadProtocolStart(bool userConfirmRequired, QString actGuid)
{
    return workflowMain->actPreloadProtocolStart(userConfirmRequired, actGuid);
}

DataServiceActionStatus DS_WorkflowAction::actPreloadProtocolResume(QString actGuid)
{
    return workflowMain->actPreloadProtocolResume(actGuid);
}

DataServiceActionStatus DS_WorkflowAction::actPreloadProtocolAbort(QString actGuid)
{
    return workflowMain->actPreloadProtocolAbort(actGuid);
}

void DS_WorkflowAction::setSystemToReady()
{
    // Simulator Only
    workflowMain->setSystemToReady();
}
