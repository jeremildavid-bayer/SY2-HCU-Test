#ifndef DEVICE_SYRINGE_SOD_H
#define DEVICE_SYRINGE_SOD_H

#include "Common/ActionBaseExt.h"
#include "DataServices/Device/DS_DeviceDef.h"

class DeviceSyringeSod : public ActionBaseExt
{
    Q_OBJECT
public:
    explicit DeviceSyringeSod(QObject *parent = 0, EnvGlobal *env_ = NULL, EnvLocal *envLocal_ = NULL, SyringeIdx location = SYRINGE_IDX_SALINE);
    ~DeviceSyringeSod();

    DataServiceActionStatus actSodStart(QString actGuid = "");
    DataServiceActionStatus actSodAbort(QString actGuid = "");

    bool isBusy();

private:
    enum State
    {
        STATE_INACTIVE = 0,

        STATE_READY,
        STATE_STARTED,
        STATE_SYRINGES_IDLE_WAITING,
        STATE_SUDS_INSERT_WAITING,

        STATE_CAL_SLACK_STARTED,
        STATE_CAL_SLACK_PROGRESS,
        STATE_CAL_SLACK_FAILED,
        STATE_CAL_SLACK_DONE,

        STATE_PRIME_STARTED,
        STATE_PRIME_PROGRESS,
        STATE_PRIME_FAILED,
        STATE_PRIME_DONE,

        STATE_OUTLET_AIR_CHECK_STARTED,
        STATE_OUTLET_AIR_CHECK_FAILED,
        STATE_OUTLET_AIR_CHECK_DONE,

        STATE_CAL_SYRINGE_AIR_CHECK_STARTED,
        STATE_CAL_SYRINGE_AIR_CHECK_PROGRESS,
        STATE_CAL_SYRINGE_AIR_CHECK_FAILED,
        STATE_CAL_SYRINGE_AIR_CHECK_DONE,

        STATE_CAL_SYRINGE_GET_AIR_CHECK_COEFF_STARTED,
        STATE_CAL_SYRINGE_GET_AIR_CHECK_COEFF_PROGRESS,
        STATE_CAL_SYRINGE_GET_AIR_CHECK_COEFF_FAILED,
        STATE_CAL_SYRINGE_GET_AIR_CHECK_COEFF_DONE,

        STATE_ABORT,
        STATE_FAILED,
        STATE_DONE
    };

    SyringeIdx location;
    bool abortRequested;
    QString guidSyringesIdleWaiting;
    QString guidSudsWaiting;
    int primeTrialIndex;

    void processState();

private slots:
    void slotAppInitialised();
};

#endif // DEVICE_SYRINGE_SOD_H
