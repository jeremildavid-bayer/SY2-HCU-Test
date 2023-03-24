#ifndef DEVICE_OUTLET_AIR_DOOR_H
#define DEVICE_OUTLET_AIR_DOOR_H

#include <QObject>
#include <QTimer>
#include "Common/Common.h"
#include "Common/ActionBase.h"
#include "Common/HwCapabilities.h"
#include "DataServices/Mcu/DS_McuData.h"

class DeviceOutletAirDoor : public ActionBase
{
    Q_OBJECT
public:
    explicit DeviceOutletAirDoor(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~DeviceOutletAirDoor();

private:
    enum AirDetectorDoorState
    {
        STATE_IDLE = 0,
        STATE_START,
        STATE_WAITING_FOR_DOOR_OPEN,
        STATE_WAITING_FOR_DOOR_CLOSED,
        STATE_WAITING_FOR_MUDS,
        STATE_MUDS_CHECK,
        STATE_COMPLETE
    };
    AirDetectorDoorState curState;
    bool isActive;

    void setLed();
    void setAlert();

private slots:
    void slotAppInitialised();

};

#endif // DEVICE_OUTLET_AIR_DOOR_H
