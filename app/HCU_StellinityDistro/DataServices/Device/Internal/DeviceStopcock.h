#ifndef DEVICE_STOPCOCK_H
#define DEVICE_STOPCOCK_H

#include <QObject>
#include "Common/Common.h"
#include "Common/ActionBaseExt.h"
#include "DataServices/Mcu/DS_McuDef.h"

class DeviceStopcock : public ActionBaseExt
{
    Q_OBJECT
public:
    explicit DeviceStopcock(QObject *parent = 0, EnvGlobal *env_ = NULL, EnvLocal *envLocal_ = NULL, SyringeIdx location = SYRINGE_IDX_SALINE);
    ~DeviceStopcock();

    DataServiceActionStatus actStopcock(DS_McuDef::StopcockPos pos, int retryLimit = STOPCOCK_ACTION_TRIALS_LIMIT, QString actGuid = "");

private:

    enum State
    {
        STATE_SET_STOPCOCKS_READY = 0,
        STATE_SET_STOPCOCKS_STARTED,
        STATE_SET_STOPCOCKS_PROGRESS,
        STATE_SET_STOPCOCKS_FAILED,
        STATE_SET_STOPCOCKS_DONE
    };

    SyringeIdx location;
    DS_McuDef::StopcockPos actStopcockTargetPos;
    QList<DataServiceActionStatus> actStopcockStatusList;
    QList<DataServiceActionStatus> actStopcockStatusListBuf;
    int actStopcockRetryLimit;
    int actStopcockRetryCount;

    void processState();

private slots:
    void slotAppInitialised();
};

#endif // DEVICE_STOPCOCK_H
