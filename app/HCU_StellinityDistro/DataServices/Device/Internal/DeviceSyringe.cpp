#include "Apps/AppManager.h"
#include "DeviceSyringe.h"
#include "DataServices/Device/DS_DeviceData.h"

DeviceSyringe::DeviceSyringe(QObject *parent, EnvGlobal *env_, EnvLocal *envLocal_, SyringeIdx location_) :
    ActionBase(parent, env_),
    location(location_)
{
    envLocal = envLocal_;

    deviceSyringeFill = new DeviceSyringeFill(parent, env, envLocal, location);
    deviceSyringePrime = new DeviceSyringePrime(parent, env, envLocal, location);
    deviceSyringeSod = new DeviceSyringeSod(parent, env, envLocal, location);
    deviceSyringeAirCheck = new DeviceSyringeAirCheck(parent, env, envLocal, location);
}

DeviceSyringe::~DeviceSyringe()
{
    delete deviceSyringeFill;
    delete deviceSyringePrime;
    delete deviceSyringeSod;
    delete deviceSyringeAirCheck;
}

DataServiceActionStatus DeviceSyringe::actFill(bool preAirCheckEnabled, QString actGuid)
{
    return deviceSyringeFill->actFill(preAirCheckEnabled, actGuid);
}

DataServiceActionStatus DeviceSyringe::actPrime(DS_DeviceDef::SudsPrimeType newPrimeType, QString actGuid)
{
    return deviceSyringePrime->actPrime(newPrimeType, actGuid);
}

DataServiceActionStatus DeviceSyringe::actPrime(const DS_McuDef::ActPrimeParams &primeParams, QString actGuid)
{
    return deviceSyringePrime->actPrime(primeParams, actGuid);
}

DataServiceActionStatus DeviceSyringe::actSodStart(QString actGuid)
{
    return deviceSyringeSod->actSodStart(actGuid);
}

DataServiceActionStatus DeviceSyringe::actSodAbort(QString actGuid)
{
    return deviceSyringeSod->actSodAbort(actGuid);
}

DataServiceActionStatus DeviceSyringe::actAirCheck(QString actGuid)
{
    return deviceSyringeAirCheck->actAirCheck(actGuid);
}
