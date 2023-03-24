#ifndef QML_WORKFLOW_H
#define QML_WORKFLOW_H

#include "Common/Common.h"
#include "DataServices/Workflow/DS_WorkflowDef.h"

class QML_Workflow : public QObject
{
    Q_OBJECT
public:
    explicit QML_Workflow(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~QML_Workflow();

    Q_INVOKABLE void slotSodResume();
    Q_INVOKABLE void slotFill(int index);
    Q_INVOKABLE void slotEjectMuds();
    Q_INVOKABLE void slotSudsAutoPrimeForceStart();
    Q_INVOKABLE void slotLoadNewSource(int index, QString brand, double concentration, double volume, QString lotBatch, QString expirationDateStr);
    Q_INVOKABLE void slotUnloadSource(int index);
    Q_INVOKABLE void slotSudsAirRecoveryResume();
    Q_INVOKABLE QVariantMap slotFluidRemovalStart(int index);
    Q_INVOKABLE void slotFluidRemovalResume();
    Q_INVOKABLE void slotFluidRemovalAbort();
    Q_INVOKABLE QVariantMap slotEndOfDayPurgeStart();
    Q_INVOKABLE void slotEndOfDayPurgeResume();
    Q_INVOKABLE void slotEndOfDayPurgeAbort();
    Q_INVOKABLE void slotClearSodError();
    Q_INVOKABLE QVariantMap slotSyringeAirRecoveryResume();
    Q_INVOKABLE void slotQueAutomaticQualifiedDischarge(int batteryIdx);
    Q_INVOKABLE void slotCancelAutomaticQualifiedDischarge();
    Q_INVOKABLE QVariantMap slotWorkflowBatteryAction(int batteryIdx, QString action);
    Q_INVOKABLE void slotWorkflowBatteryAbort();
    Q_INVOKABLE QVariantMap slotManualQualifiedDischargeStart(int batteryIdx, QString methodTypeStr);
    Q_INVOKABLE void slotManualQualifiedDischargeResume();
    Q_INVOKABLE void slotManualQualifiedDischargeAbort();
    Q_INVOKABLE QVariantMap slotShippingModeStart(int batteryIdx, int targetCharge);
    Q_INVOKABLE void slotShippingModeResume();
    Q_INVOKABLE void slotShippingModeAbort();
    Q_INVOKABLE void slotPreloadProtocolStart(bool userConfirmRequired);
    Q_INVOKABLE void slotPreloadProtocolResume();
    Q_INVOKABLE void slotPreloadProtocolAbort();

private:
    EnvGlobal *env;
    EnvLocal *envLocal;
    QObject *qmlSrc;

};

#endif // QML_WORKFLOW_H
