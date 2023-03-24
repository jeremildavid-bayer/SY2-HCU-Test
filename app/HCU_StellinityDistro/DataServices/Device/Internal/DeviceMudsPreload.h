#ifndef DEVICE_MUDS_PRELOAD_H
#define DEVICE_MUDS_PRELOAD_H

#include "Common/ActionBaseExt.h"
#include "DataServices/Device/DS_DeviceDef.h"

class DeviceMudsPreload : public ActionBaseExt
{
    Q_OBJECT
public:
    explicit DeviceMudsPreload(QObject *parent = 0, EnvGlobal *env_ = NULL, EnvLocal *envLocal_ = NULL);
    ~DeviceMudsPreload();

    DataServiceActionStatus actMudsPreload(const DS_McuDef::InjectionProtocol &injectionProtocol, QString actGuid = "");
    DataServiceActionStatus actGetPreloadPrimeParamsList(QList<DS_McuDef::ActPrimeParams> &primeParamsList, const DS_McuDef::InjectionProtocol &injectionProtocol, QString actGuid = "");

private:
    enum State
    {
        STATE_INACTIVE = 0,
        STATE_READY,
        STATE_STARTED,

        STATE_PRIME_PHASES_STARTED,
        STATE_PRIME_PHASES_PROGRESS,
        STATE_PRIME_PHASES_FAILED,
        STATE_PRIME_PHASES_DONE,

        STATE_FAILED,
        STATE_DONE
    };

    bool isSetStateReady();
    void processState();

    int primeIndex;
    QList<DS_McuDef::ActPrimeParams> primeParamsList;


private slots:
    void slotAppInitialised();
};

#endif // DEVICE_MUDS_PRELOAD_H
