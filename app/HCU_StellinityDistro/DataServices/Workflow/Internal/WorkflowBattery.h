#ifndef WORKFLOWBATTERY_H
#define WORKFLOWBATTERY_H

#include "Common/Common.h"
#include "Common/ActionBaseExt.h"
#include "DataServices/Workflow/DS_WorkflowDef.h"

class WorkflowBattery : public ActionBaseExt
{
    Q_OBJECT

public:
    explicit WorkflowBattery(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~WorkflowBattery();

    DataServiceActionStatus actBatteryChargeToStart(int batteryIdx, int targetCharge, QString actGuid = "");
    DataServiceActionStatus actBatteryAction(int batteryIdx, QString action, QString actGuid = "");
    DataServiceActionStatus actAbort(QString actGuid = "");

    DS_WorkflowDef::WorkflowBatteryStatus getWorkflowBatteryStatus();
private:
    int batteryIndex;
    int targetCharge;
    // used to store max charge current to detect high max error full charge
    int maxChargeCurrent;

    unsigned int digestCount;
    QString guidBmsDigestMonitor;

    void reset();
    void setStateSynch(int newState);
    bool isSetStateReady();
    int getState();
    QString getStateStr(int state);
    void setStatusMessage(QString message);

    DataServiceActionStatus actDischargeBatteryStart(int batteryIdx, int targetCharge, QString actGuid = "");
    DataServiceActionStatus actChargeBatteryStart(int batteryIdx, int targetCharge, QString actGuid = "");

    bool basicFailCheck(DataServiceActionStatus &status, int batteryIdx);
    bool checkCommonBatteryErrors(const DS_McuDef::BMSDigest &bmsDigest, QString *err = NULL);
    void (WorkflowBattery::*signalBMSChanged_func)(const DS_McuDef::BMSDigests &bmsDigests, const DS_McuDef::BMSDigests &prevBmsDigests);

    void BMSChanged_Idle(const DS_McuDef::BMSDigests &bmsDigests, const DS_McuDef::BMSDigests &prevBmsDigests);

    void BMSChanged_Charge_Preparation(const DS_McuDef::BMSDigests &bmsDigests, const DS_McuDef::BMSDigests &prevBmsDigests);
    void BMSChanged_Charge_Progress(const DS_McuDef::BMSDigests &bmsDigests, const DS_McuDef::BMSDigests &prevBmsDigests);

    void BMSChanged_Discharge_Preparation(const DS_McuDef::BMSDigests &bmsDigests, const DS_McuDef::BMSDigests &prevBmsDigests);
    void BMSChanged_Discharge_Progress(const DS_McuDef::BMSDigests &bmsDigests, const DS_McuDef::BMSDigests &prevBmsDigests);

    void BMSChanged_BaseboardTest(const DS_McuDef::BMSDigests &bmsDigests, const DS_McuDef::BMSDigests &prevBmsDigests);

    void processState();
    void fasterDischarging(bool enable);

    QString getBatteryName(int batteryIdx);
    int getOtherBatteryIndex(int batteryIdx);
private slots:
    void slotAppInitialised();
};


#endif // WORKFLOWBATTERY_H
