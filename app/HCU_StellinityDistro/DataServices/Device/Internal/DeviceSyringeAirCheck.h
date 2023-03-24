#ifndef DEVICE_SYRINGE_AIR_CHECK_H
#define DEVICE_SYRINGE_AIR_CHECK_H

#include "Common/ActionBaseExt.h"
#include "DataServices/Device/DS_DeviceDef.h"

class DeviceSyringeAirCheck : public ActionBaseExt
{
    Q_OBJECT
public:
    explicit DeviceSyringeAirCheck(QObject *parent = 0, EnvGlobal *env_ = NULL, EnvLocal *envLocal_ = NULL, SyringeIdx location = SYRINGE_IDX_SALINE);
    ~DeviceSyringeAirCheck();

    DataServiceActionStatus actAirCheck(QString actGuid = "");
    bool isBusy();

private:
    enum State
    {
        STATE_INACTIVE = 0,

        STATE_READY,
        STATE_STARTED,

        STATE_SYRINGE_AIR_CHECK_STARTED,
        STATE_SYRINGE_AIR_CHECK_PROGRESS,
        STATE_SYRINGE_AIR_CHECK_FAILED,
        STATE_SYRINGE_AIR_CHECK_DONE,

        STATE_SYRINGE_GET_AIR_VOLUME_STARTED,
        STATE_SYRINGE_GET_AIR_VOLUME_PROGRESS,
        STATE_SYRINGE_GET_AIR_VOLUME_FAILED,
        STATE_SYRINGE_GET_AIR_VOLUME_DONE,

        STATE_FAILED,
        STATE_DONE
    };

    SyringeIdx location;

    void processState();

private slots:
    void slotAppInitialised();
};

#endif // DEVICE_SYRINGE_AIR_CHECK_H
