#ifndef WORKFLOW_SHIPPING_MODE_H
#define WORKFLOW_SHIPPING_MODE_H

#include "Common/Common.h"
#include "Common/ActionBaseExt.h"
#include "DataServices/Workflow/DS_WorkflowDef.h"

class WorkflowShippingMode : public ActionBaseExt
{
    Q_OBJECT

public:
    explicit WorkflowShippingMode(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~WorkflowShippingMode();

    DataServiceActionStatus actShippingModeStart(int batteryIdx, int targetCharge, QString actGuid = "");
    DataServiceActionStatus actShippingModeResume(QString actGuid = "");
    DataServiceActionStatus actShippingModeAbort(QString actGuid = "");
    
    DS_WorkflowDef::ShippingModeStatus getShippingModeStatus();
private:
    int batteryIndex;
    int targetChargedLevel;

    int getState();
    QString getStateStr(int state);
    void setStateSynch(int newState);
    bool isSetStateReady();
    void setStatusMessage(QString message);

    void processState();

    QString getBatteryName(int batteryIndex);
    int getOtherBatteryIndex(int batteryIndex);

private slots:
    void slotAppInitialised();
};

#endif // WORKFLOW_SHIPPING_MODE_H
