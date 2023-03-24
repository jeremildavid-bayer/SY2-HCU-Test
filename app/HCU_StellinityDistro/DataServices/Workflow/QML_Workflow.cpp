#include "QML_Workflow.h"
#include "Common/Util.h"
#include "DS_WorkflowData.h"
#include "DS_WorkflowAction.h"
#include "DataServices/CfgLocal/DS_CfgLocal.h"
#include "DataServices/Device/DS_DeviceAction.h"
#include "DataServices/Device/DS_DeviceData.h"
#include "DataServices/Alert/DS_AlertAction.h"
#include "Common/ImrParser.h"

QML_Workflow::QML_Workflow(QObject *parent, EnvGlobal *env_) :
    QObject(parent),
    env(env_)
{
    envLocal = new EnvLocal("QML_Workflow", "QML_WORKFLOW");
    qmlSrc = env->qml.object->findChild<QObject*>("dsWorkflow");
    env->qml.engine->rootContext()->setContextProperty("dsWorkflowCpp", this);

    if (qmlSrc == NULL)
    {
        LOG_ERROR("Failed to assign qmlSrc.\n");
    }

    // Register Data Callbacks
    connect(env->ds.workflowData, &DS_WorkflowData::signalDataChanged_WorkflowState, this, [=](DS_WorkflowDef::WorkflowState workflowState) {
        qmlSrc->setProperty("workflowState", ImrParser::ToImr_WorkFlowState(workflowState));
    });

    connect(env->ds.workflowData, &DS_WorkflowData::signalDataChanged_WorkflowErrorStatus, this, [=](const DS_WorkflowDef::WorkflowErrorStatus &workflowErrorStatus) {
        qmlSrc->setProperty("workflowErrorStatus", ImrParser::ToImr_WorkflowErrorStatus(workflowErrorStatus));
    });

    connect(env->ds.workflowData, &DS_WorkflowData::signalDataChanged_SodErrorState, this, [=](const DS_WorkflowDef::SodErrorState &sodErrorState) {
        qmlSrc->setProperty("sodErrorState", ImrParser::ToImr_SodErrorState(sodErrorState));
    });

    connect(env->ds.workflowData, &DS_WorkflowData::signalDataChanged_MudsSodState, this, [=](DS_WorkflowDef::MudsSodState mudsSodState) {
        qmlSrc->setProperty("workflowMudsSodState", ImrParser::ToImr_MudsSodState(mudsSodState));
    });

    connect(env->ds.workflowData, &DS_WorkflowData::signalDataChanged_FluidRemovalState, this, [=](DS_WorkflowDef::FluidRemovalState fluidRemovalState) {
        qmlSrc->setProperty("workflowFluidRemovalState", ImrParser::ToImr_FluidRemovalState(fluidRemovalState));
    });

    connect(env->ds.workflowData, &DS_WorkflowData::signalDataChanged_EndOfDayPurgeState, this, [=](DS_WorkflowDef::EndOfDayPurgeState endOfDayPurgeState) {
        qmlSrc->setProperty("workflowEndOfDayPurgeState", ImrParser::ToImr_EndOfDayPurgeState(endOfDayPurgeState));
    });

    connect(env->ds.workflowData, &DS_WorkflowData::signalDataChanged_AutoEmptyState, this, [=](DS_WorkflowDef::AutoEmptyState autoEmptyState) {
        qmlSrc->setProperty("workflowAutoEmptyState", ImrParser::ToImr_AutoEmptyState(autoEmptyState));
    });

    connect(env->ds.workflowData, &DS_WorkflowData::signalDataChanged_MudsEjectState, this, [=](DS_WorkflowDef::MudsEjectState mudsEjectState) {
        qmlSrc->setProperty("workflowMudsEjectState", ImrParser::ToImr_MudsEjectState(mudsEjectState));
    });

    connect(env->ds.workflowData, &DS_WorkflowData::signalDataChanged_SyringeAirRecoveryState, this, [=](DS_WorkflowDef::SyringeAirRecoveryState syringeAirRecoveryState) {
        qmlSrc->setProperty("workflowSyringeAirRecoveryState", ImrParser::ToImr_SyringeAirRecoveryState(syringeAirRecoveryState));
    });

    connect(env->ds.workflowData, &DS_WorkflowData::signalDataChanged_SudsAirRecoveryState, this, [=](DS_WorkflowDef::SudsAirRecoveryState state) {
        qmlSrc->setProperty("workflowSudsAirRecoveryState", ImrParser::ToImr_SudsAirRecoveryState(state));
    });

    connect(env->ds.workflowData, &DS_WorkflowData::signalDataChanged_WorkflowBatteryStatus, this, [=](const DS_WorkflowDef::WorkflowBatteryStatus &status) {
        qmlSrc->setProperty("workflowBatteryStatus", ImrParser::ToImr_WorkflowBatteryStatus(status));
    });

    connect(env->ds.workflowData, &DS_WorkflowData::signalDataChanged_ManualQualifiedDischargeStatus, this, [=](const DS_WorkflowDef::ManualQualifiedDischargeStatus &status) {
        qmlSrc->setProperty("workflowManualQualifiedDischargeStatus", ImrParser::ToImr_ManualQualifiedDischargeStatus(status));
    });

    connect(env->ds.workflowData, &DS_WorkflowData::signalDataChanged_AutomaticQualifiedDischargeStatus, this, [=](const DS_WorkflowDef::AutomaticQualifiedDischargeStatus &status) {
        qmlSrc->setProperty("workflowAutomaticQualifiedDischargeStatus", ImrParser::ToImr_AutomaticQualifiedDischargeStatus(status));
    });

    connect(env->ds.workflowData, &DS_WorkflowData::signalDataChanged_ShippingModeStatus, this, [=](DS_WorkflowDef::ShippingModeStatus status) {
        qmlSrc->setProperty("workflowShippingModeStatus", ImrParser::ToImr_ShippingModeStatus(status));
    });

    connect(env->ds.workflowData, &DS_WorkflowData::signalDataChanged_PreloadProtocolState, this, [=](DS_WorkflowDef::PreloadProtocolState state) {
        qmlSrc->setProperty("workflowPreloadProtocolState", ImrParser::ToImr_PreloadProtocolState(state));
    });

    connect(env->ds.workflowData, &DS_WorkflowData::signalDataChanged_MudsSodStatus, this, [=](const DS_WorkflowDef::MudsSodStatus &mudsSodStatus) {
        qmlSrc->setProperty("mudsSodStatus", ImrParser::ToImr_MudsSodStatus(mudsSodStatus));
    });

    connect(env->ds.workflowData, &DS_WorkflowData::signalDataChanged_ContrastAvailable1, this, [=](bool contrastAvailable1) {
        qmlSrc->setProperty("contrastAvailable1", contrastAvailable1);
    });

    connect(env->ds.workflowData, &DS_WorkflowData::signalDataChanged_ContrastAvailable2, this, [=](bool contrastAvailable2) {
        qmlSrc->setProperty("contrastAvailable2", contrastAvailable2);
    });
}

QML_Workflow::~QML_Workflow()
{
    delete envLocal;
}

void QML_Workflow::slotSodResume()
{
    env->ds.workflowAction->actSodRestart();
}

void QML_Workflow::slotFill(int index)
{
    SyringeIdx syringeIdx = (SyringeIdx)index;
    if ( (syringeIdx < SYRINGE_IDX_START) ||
         (syringeIdx >= SYRINGE_IDX_MAX) )
    {
        LOG_ERROR("slotFill(): Bad Syringe Index=%d\n", syringeIdx);
        return;
    }
    env->ds.workflowAction->actForceFill(syringeIdx);
}

void QML_Workflow::slotEjectMuds()
{
    env->ds.workflowAction->actSodAbort(DS_WorkflowDef::SOD_ERROR_STATE_MUDS_EJECTED);
    env->ds.workflowAction->actMudsEject();
}

void QML_Workflow::slotSudsAutoPrimeForceStart()
{
    if (env->ds.alertAction->isActivated("SUDSReinsertedPrimeRequired"))
    {
        env->ds.alertAction->deactivate("SUDSReinsertedPrimeRequired");
    }
    env->ds.workflowAction->actAutoPrime();
}

void QML_Workflow::slotLoadNewSource(int index, QString brand, double concentration, double volume, QString lotBatch, QString expirationDateStr)
{
    LOG_INFO("Load New Source Requested - index(%d), brand(%s), concentration(%f), volume(%f), lotBatch(%s), expirationDate(%s)\n",
             index, brand.CSTR(), concentration, volume, lotBatch.CSTR(), expirationDateStr.CSTR());

    DS_DeviceDef::FluidPackages sourcePackages;
    if (index == SYRINGE_IDX_SALINE)
    {
        sourcePackages = env->ds.deviceData->getAllowedSalinePackages();
    }
    else
    {
        sourcePackages = env->ds.deviceData->getAllowedContrastPackages();
    }

    for (int i = 0; i < sourcePackages.length(); i++)
    {
        DS_DeviceDef::FluidPackage *tempPackage = &sourcePackages[i];
        if ( (tempPackage->brand.toLower() == brand.toLower()) &&
             (tempPackage->concentration == concentration) &&
             (tempPackage->volume == volume) )
        {
            if ( (expirationDateStr == "") ||
                 (expirationDateStr == _L("--")) )
            {
                tempPackage->expirationDateEpochMs = -1;
            }
            else
            {
                expirationDateStr.replace("/", "");

                // Get expiration date. The date should be the 'end' of the month at 23:59.59pm.
                QDateTime expirationDate = QDateTime::fromString(expirationDateStr, "yyyyMM");
                expirationDate = expirationDate.addMonths(1);
                expirationDate = expirationDate.addMSecs(-1);
                // CRU expects every date field to be correctly UTC offset
                int utcOffsetSeconds = env->ds.cfgLocal->get_Hidden_CurrentUtcOffsetMinutes() * -60;
                expirationDate = expirationDate.addSecs(utcOffsetSeconds);
                tempPackage->expirationDateEpochMs = expirationDate.toMSecsSinceEpoch();
            }

            if ( (lotBatch == "") ||
                 (lotBatch == _L("--")) )
            {
                tempPackage->lotBatch = "";
            }
            else
            {
                tempPackage->lotBatch = lotBatch;
            }

            env->ds.workflowAction->actBottleLoad((SyringeIdx)index, *tempPackage);
            return;
        }
    }

    LOG_ERROR("Failed to find fluid brand(%s) & concentration(%.1f) & volume(%.1f) in AllowedPackages\n", brand.CSTR(), concentration, volume);
}

void QML_Workflow::slotUnloadSource(int index)
{
    env->ds.deviceAction->actBottleUnload((SyringeIdx)index);
}

void QML_Workflow::slotSudsAirRecoveryResume()
{
    env->ds.workflowAction->actSudsAirRecoveryResume();
}

QVariantMap QML_Workflow::slotFluidRemovalStart(int index)
{
    DataServiceActionStatus status = env->ds.workflowAction->actFluidRemovalStart((SyringeIdx)index);
    return ImrParser::ToImr_DataServiceActionStatus(status);
}

void QML_Workflow::slotFluidRemovalResume()
{
    env->ds.workflowAction->actFluidRemovalResume();
}

void QML_Workflow::slotFluidRemovalAbort()
{
    env->ds.workflowAction->actFluidRemovalAbort();
}

QVariantMap QML_Workflow::slotEndOfDayPurgeStart()
{
    DataServiceActionStatus status = env->ds.workflowAction->actEndOfDayPurgeStart();
    return ImrParser::ToImr_DataServiceActionStatus(status);
}

void QML_Workflow::slotEndOfDayPurgeResume()
{
    env->ds.workflowAction->actEndOfDayPurgeResume();
}

void QML_Workflow::slotEndOfDayPurgeAbort()
{
    env->ds.workflowAction->actEndOfDayPurgeAbort();
}

void QML_Workflow::slotWorkflowBatteryAbort()
{
    env->ds.workflowAction->actWorkflowBatteryAbort();
}

void QML_Workflow::slotQueAutomaticQualifiedDischarge(int batteryIdx)
{
    env->ds.workflowAction->actQueAutomaticQualifiedDischarge(batteryIdx);
}

void QML_Workflow::slotCancelAutomaticQualifiedDischarge()
{
    env->ds.workflowAction->actCancelAutomaticQualifiedDischarge();
}

QVariantMap QML_Workflow::slotWorkflowBatteryAction(int batteryIdx, QString action)
{
    DataServiceActionStatus status;
    if (action == "Que AQD")
    {
        // route to automatic qualified discharge workflow
        status = env->ds.workflowAction->actQueAutomaticQualifiedDischarge(batteryIdx);
    }
    else
    {
        status = env->ds.workflowAction->actWorkflowBatteryAction(batteryIdx, action);
    }

    return ImrParser::ToImr_DataServiceActionStatus(status);
}

QVariantMap QML_Workflow::slotManualQualifiedDischargeStart(int batteryIdx, QString methodTypeStr)
{
    methodTypeStr = methodTypeStr.toLower();
    DS_WorkflowDef::ManualQualifiedDischargeMethod methodType;
    if (methodTypeStr == "self discharge")
    {
        methodType = DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_METHOD_SELF;
    }
    else if (methodTypeStr == "external load")
    {
        methodType = DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_METHOD_EXTERNAL_LOAD;
    }
    else
    {
        // code shouldn't reach here
        LOG_ERROR("Failed to assign Manual Qualified Discharge method");
        methodType = DS_WorkflowDef::MANUAL_QUALIFIED_DISCHARGE_METHOD_SELF; //to make the compiler happy
    }

    DataServiceActionStatus status = env->ds.workflowAction->actManualQualifiedDischargeStart(batteryIdx, methodType);
    return ImrParser::ToImr_DataServiceActionStatus(status);
}

void QML_Workflow::slotManualQualifiedDischargeResume()
{
    env->ds.workflowAction->actManualQualifiedDischargeResume();
}

void QML_Workflow::slotManualQualifiedDischargeAbort()
{
    env->ds.workflowAction->actManualQualifiedDischargeAbort();
}

QVariantMap QML_Workflow::slotShippingModeStart(int batteryIdx, int targetCharge)
{
    DataServiceActionStatus status = env->ds.workflowAction->actShippingModeStart(batteryIdx, targetCharge);
    return ImrParser::ToImr_DataServiceActionStatus(status);
}

void QML_Workflow::slotShippingModeResume()
{
    env->ds.workflowAction->actShippingModeResume();
}

void QML_Workflow::slotShippingModeAbort()
{
    env->ds.workflowAction->actShippingModeAbort();
}

void QML_Workflow::slotPreloadProtocolStart(bool userConfirmRequired)
{
    QString actGuid = Util::newGuid();
    env->actionMgr->onActionStarted(actGuid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus status) {
        qmlSrc->setProperty("preloadStatus", ImrParser::ToImr_DataServiceActionStatus(status));
        env->actionMgr->onActionCompleted(actGuid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus status) {
            qmlSrc->setProperty("preloadStatus", ImrParser::ToImr_DataServiceActionStatus(status));
        });
    });

    DataServiceActionStatus status = env->ds.workflowAction->actPreloadProtocolStart(userConfirmRequired, actGuid);
    qmlSrc->setProperty("preloadStatus", ImrParser::ToImr_DataServiceActionStatus(status));
}

void QML_Workflow::slotPreloadProtocolResume()
{
    env->ds.workflowAction->actPreloadProtocolResume();
}

void QML_Workflow::slotPreloadProtocolAbort()
{
    env->ds.workflowAction->actPreloadProtocolAbort();
}

void QML_Workflow::slotClearSodError()
{
    DS_WorkflowDef::WorkflowErrorStatus workflowErrorStatus = env->ds.workflowData->getWorkflowErrorStatus();
    workflowErrorStatus.state = DS_WorkflowDef::WORKFLOW_ERROR_STATE_NONE;
    workflowErrorStatus.syringeIndexFailed = SYRINGE_IDX_NONE;
    env->ds.workflowData->setWorkflowErrorStatus(workflowErrorStatus);
}

QVariantMap QML_Workflow::slotSyringeAirRecoveryResume()
{
    DataServiceActionStatus status = env->ds.workflowAction->actSyringeAirRecoveryResume();
    return ImrParser::ToImr_DataServiceActionStatus(status);
}
