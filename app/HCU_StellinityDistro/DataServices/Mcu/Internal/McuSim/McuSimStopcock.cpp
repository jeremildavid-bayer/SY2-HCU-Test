#include "McuSimStopcock.h"
#include "DataServices/System/DS_SystemData.h"
#include "DataServices/CfgLocal/DS_CfgLocal.h"

McuSimStopcock::McuSimStopcock(QObject *parent, EnvGlobal *env_) :
    QObject(parent),
    env(env_)
{
    curPosition = "FILL";
    requestedPosition = "FILL";
    engaged = true;
    connect(&tmrStopcock, SIGNAL(timeout()), this, SLOT(slotTmrStopcockFinished()));
}

McuSimStopcock::~McuSimStopcock()
{
    tmrStopcock.stop();
}

QString McuSimStopcock::setPosition(QString position, QString errCodePrefix)
{
    QString err = "OK";
    if (position == _L("IDLE"))
    {
    }
    else if (position == _L("NONE"))
    {
    }
    else if (tmrStopcock.isActive())
    {
        err = errCodePrefix + "CommFailed";
    }
    else if (curPosition != position)
    {
        requestedPosition = position;
        curPosition = "MOVING";
        tmrStopcock.start(MCU_SIM_STOPCOCK_DELAY_MS);
    }

    return err;
}

bool McuSimStopcock::setEngaged(bool engage)
{
    if (curPosition == _L("UNKNOWN"))
    {
        return false;
    }

    engaged = engage;
    return true;
}

bool McuSimStopcock::isEngaged()
{
    return engaged;
}

QString McuSimStopcock::getPosition()
{
    return curPosition;
}

void McuSimStopcock::slotTmrStopcockFinished()
{
    tmrStopcock.stop();
    curPosition = requestedPosition;
    emit signalPositionChanged(curPosition);
}

