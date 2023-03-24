#ifndef WORKFLOW_MANUAL_QUALIFIED_DISCHARGE_H
#define WORKFLOW_MANUAL_QUALIFIED_DISCHARGE_H

#include "Common/Common.h"
#include "Common/ActionBaseExt.h"
#include "DataServices/Workflow/DS_WorkflowDef.h"

class WorkflowManualQualifiedDischarge : public ActionBaseExt
{
    Q_OBJECT

public:
    explicit WorkflowManualQualifiedDischarge(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~WorkflowManualQualifiedDischarge();

    DataServiceActionStatus actManualQualifiedDischargeStart(int batteryIdx, DS_WorkflowDef::ManualQualifiedDischargeMethod methodType, QString actGuid = "");
    DataServiceActionStatus actManualQualifiedDischargeResume(QString actGuid = "");
    DataServiceActionStatus actManualQualifiedDischargeAbort(QString actGuid = "");
    
private:
    void (WorkflowManualQualifiedDischarge::*signalBMSChanged_func)(const DS_McuDef::BMSDigests &bmsDigests, const DS_McuDef::BMSDigests &prevBmsDigests);
    void BMSChanged_ExternalLoad_Discharge_Progress(const DS_McuDef::BMSDigests &bmsDigests, const DS_McuDef::BMSDigests &prevBmsDigests);

    int batteryIndex;
    DS_WorkflowDef::ManualQualifiedDischargeMethod dischargeMethod;

    DS_WorkflowDef::ManualQualifiedDischargeStatus getManualQualifiedDischargeStatus();
    int getState();
    QString getStateStr(int state);
    void setStateSynch(int newState);
    bool isSetStateReady();
    void setStatusMessage(QString message);
    void processState();
    void reset();

    QString getBatteryName(int batteryIdx);

    int digestCount;

private slots:
    void slotAppInitialised();
};

#endif // WORKFLOW_MANUAL_QUALIFIED_DISCHARGE_H
