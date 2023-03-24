#ifndef DEVICE_SYRINGE_H
#define DEVICE_SYRINGE_H

#include "DeviceSyringeFill.h"
#include "DeviceSyringePrime.h"
#include "DeviceSyringeSod.h"
#include "DeviceSyringeAirCheck.h"

class DeviceSyringe : public ActionBase
{
    Q_OBJECT
public:
    explicit DeviceSyringe(QObject *parent = 0, EnvGlobal *env_ = NULL, EnvLocal *envLocal_ = NULL, SyringeIdx location = SYRINGE_IDX_SALINE);
    ~DeviceSyringe();

    DataServiceActionStatus actFill(bool preAirCheckEnabled, QString actGuid = "");
    DataServiceActionStatus actPrime(DS_DeviceDef::SudsPrimeType primeType = DS_DeviceDef::SUDS_PRIME_TYPE_AUTO, QString actGuid = "");
    DataServiceActionStatus actPrime(const DS_McuDef::ActPrimeParams &primeParams, QString actGuid = "");
    DataServiceActionStatus actSodStart(QString actGuid = "");
    DataServiceActionStatus actSodAbort(QString actGuid = "");
    DataServiceActionStatus actAirCheck(QString actGuid = "");

private:
    SyringeIdx location;
    DeviceSyringeFill *deviceSyringeFill;
    DeviceSyringePrime *deviceSyringePrime;
    DeviceSyringeSod *deviceSyringeSod;
    DeviceSyringeAirCheck *deviceSyringeAirCheck;
};

#endif // DEVICE_SYRINGE_H


