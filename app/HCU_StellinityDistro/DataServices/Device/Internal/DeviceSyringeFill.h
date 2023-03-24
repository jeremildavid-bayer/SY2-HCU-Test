#ifndef DEVICE_SYRINGE_FILL_H
#define DEVICE_SYRINGE_FILL_H

#include "Common/ActionBaseExt.h"
#include "DataServices/Device/DS_DeviceDef.h"

class DeviceSyringeFill : public ActionBaseExt
{
    Q_OBJECT

public:
    explicit DeviceSyringeFill(QObject *parent = 0, EnvGlobal *env_ = NULL, EnvLocal *envLocal_ = NULL, SyringeIdx location = SYRINGE_IDX_SALINE);
    ~DeviceSyringeFill();

    DataServiceActionStatus actFill(bool preAirCheckEnabled, QString actGuid = "");

    bool isBusy();

private:
    enum State
    {
        // FILL states
        STATE_INACTIVE = 0,

        STATE_READY,
        STATE_STARTED,
        STATE_WAITING_MUDS,

        STATE_STOPCOCK_FILL_SETTING,
        STATE_STOPCOCK_FILL_PROGRESS,
        STATE_STOPCOCK_FILL_FAILED,
        STATE_STOPCOCK_FILL_DONE,

        STATE_FILL_STARTED,
        STATE_FILL_PROGRESS,
        STATE_FILL_FAILED,
        STATE_FILL_DONE,

        STATE_STOPCOCK_CLOSE_SETTING,
        STATE_STOPCOCK_CLOSE_PROGRESS,
        STATE_STOPCOCK_CLOSE_FAILED,
        STATE_STOPCOCK_CLOSE_DONE,

        STATE_SYRINGE_AIR_CHECK_STARTED,
        STATE_SYRINGE_AIR_CHECK_PROGRESS,
        STATE_SYRINGE_AIR_CHECK_FAILED,
        STATE_SYRINGE_AIR_CHECK_DONE,

        STATE_DONE,
        STATE_FAILED,
    };


    SyringeIdx location;
    QString guidMudsInsertWaiting;

    void processState();

private slots:
    void slotAppInitialised();
};

#endif // DEVICE_SYRINGE_FILL_H
