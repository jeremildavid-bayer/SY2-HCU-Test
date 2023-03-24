#ifndef DEVICE_MUDS_DISENGAGE_H
#define DEVICE_MUDS_DISENGAGE_H

#include "Common/ActionBaseExt.h"
#include "DataServices/Device/DS_DeviceDef.h"

class DeviceMudsDisengage : public ActionBaseExt
{
    Q_OBJECT
public:
    explicit DeviceMudsDisengage(QObject *parent = 0, EnvGlobal *env_ = NULL, EnvLocal *envLocal_ = NULL);
    ~DeviceMudsDisengage();

    DataServiceActionStatus actDisengageAll(int retryLimit = DISENGAGE_ACTION_TRIALS_LIMIT, QString actGuid = "");

private:
    enum State
    {
        STATE_INACTIVE = 0,
        STATE_READY,
        STATE_STARTED,
        STATE_PROGRESS,
        STATE_FAILED,
        STATE_DONE
    };

    int disengageRetryCount;
    int disengageRetryLimit;

    bool isSetStateReady();
    void processState();

private slots:
    void slotAppInitialised();
};

#endif // DEVICE_MUDS_DISENGAGE_H
