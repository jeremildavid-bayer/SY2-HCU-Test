#ifndef DEVICE_SYRINGE_PRIME_H
#define DEVICE_SYRINGE_PRIME_H

#include "Common/ActionBaseExt.h"
#include "DataServices/Device/DS_DeviceDef.h"

class DeviceSyringePrime : public ActionBaseExt
{
    Q_OBJECT
public:
    explicit DeviceSyringePrime(QObject *parent = 0, EnvGlobal *env_ = NULL, EnvLocal *envLocal_ = NULL, SyringeIdx location = SYRINGE_IDX_SALINE);
    ~DeviceSyringePrime();

    DataServiceActionStatus actPrime(DS_DeviceDef::SudsPrimeType primeType = DS_DeviceDef::SUDS_PRIME_TYPE_AUTO, QString actGuid = "");
    DataServiceActionStatus actPrime(const DS_McuDef::ActPrimeParams &primeParams, QString actGuid = "");


private:
    enum State
    {
        STATE_INACTIVE = 0,

        STATE_READY,
        STATE_STARTED,

        STATE_STOPCOCK_INJECT_SETTING,
        STATE_STOPCOCK_INJECT_PROGRESS,
        STATE_STOPCOCK_INJECT_DONE,
        STATE_STOPCOCK_INJECT_FAILED,

        STATE_INJECT_STARTED,
        STATE_INJECT_PROGRESS,
        STATE_INJECT_DONE,
        STATE_INJECT_FAILED,
        STATE_FAILED,

        STATE_PRESSURE_DECAY_STARTED,
        STATE_PRESSURE_DECAY_DONE,

        STATE_STOPCOCK_CLOSE_SETTING,
        STATE_STOPCOCK_CLOSE_PROGRESS,
        STATE_STOPCOCK_CLOSE_DONE,
        STATE_STOPCOCK_CLOSE_FAILED,

        STATE_DONE
    };

    SyringeIdx location;
    QString guidStopBtnMonitor;
    DS_DeviceDef::SudsPrimeType primeType;
    double syringeVolBeforePrime;

    bool isSetStateReady();
    void processState();

    void checkAutoPrimeCondition(DataServiceActionStatus *status);
    void checkManualPrimeCondition(DataServiceActionStatus *status);
    void checkSyringePrimeCondition(DataServiceActionStatus *status);
    void checkSyringeAirRecoveryPrimeCondition(DataServiceActionStatus *status);
    void checkUserDefinedPrimeCondition(DataServiceActionStatus *status);
    DS_McuDef::ActPrimeParams getPrimeParams();
    DS_McuDef::ActPrimeParams primeParams;

    bool isAutoPrimePostAutoEmpty();
    bool isAutoPrimePostAutoEmptyCompleted();
    int extendedAutoPrimeCounter;

private slots:
    void slotAppInitialised();
};

#endif // DEVICE_SYRINGE_PRIME_H
