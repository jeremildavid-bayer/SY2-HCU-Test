#ifndef WORKFLOWAUTOMATICQUALIFIEDDISCHARGE_H
#define WORKFLOWAUTOMATICQUALIFIEDDISCHARGE_H

#include "Common/Common.h"
#include "Common/ActionBaseExt.h"
#include "DataServices/Workflow/DS_WorkflowDef.h"

class WorkflowAutomaticQualifiedDischarge : public ActionBaseExt
{
    Q_OBJECT

public:
    explicit WorkflowAutomaticQualifiedDischarge(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~WorkflowAutomaticQualifiedDischarge();

    DataServiceActionStatus actForceQueAQD(int batteryIdx, QString actGuid = "");
    DataServiceActionStatus actCancelAQD(QString actGuid = "");

private:
    int batteryIndex;
    bool batteryPackMissing;
    bool batteryPermanentFailures;
    bool batteryARequiresAQD;
    bool batteryBRequiresAQD;

    // from capabilities
    int otherBatteryChargeLimit;
    int batteryATrigger;
    int batteryBTrigger;

    void (WorkflowAutomaticQualifiedDischarge::*signalBMSChanged_func)(const DS_McuDef::BMSDigests &bmsDigests, const DS_McuDef::BMSDigests &prevBmsDigests);
    void processState();
    DS_WorkflowDef::AutomaticQualifiedDischargeStatus getAutomaticQualifiedDischargeStatus();
    int getState();
    void setStatusMessage(QString message);
    void setStateSynch(int newState);
    bool isSetStateReady();
    void reset();
    void logLiveBMS(const DS_McuDef::BMSDigests &bmsDigests);

    QString getBatteryName(int batteryIdx);
    int getOtherBatteryIndex(int batteryIdx);
    void BMSChanged_Idle(const DS_McuDef::BMSDigests &bmsDigests, const DS_McuDef::BMSDigests &prevBmsDigests);
    void BMSChanged_AQDQueued(const DS_McuDef::BMSDigests &bmsDigests, const DS_McuDef::BMSDigests &prevBmsDigests);
    void BMSChanged_AQDProgress(const DS_McuDef::BMSDigests &bmsDigests, const DS_McuDef::BMSDigests &prevBmsDigests);

private slots:
    void slotAppInitialized();
};

#endif // WORKFLOWAUTOMATICQUALIFIEDDISCHARGE_H
