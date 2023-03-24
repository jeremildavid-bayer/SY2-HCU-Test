#ifndef DEVICE_WASTE_CONTAINER_H
#define DEVICE_WASTE_CONTAINER_H

#include "Common/Common.h"
#include "Common/ActionBase.h"
#include "DataServices/Mcu/DS_McuDef.h"
#include "DataServices/Device/DS_DeviceDef.h"

class DeviceWasteContainer : public ActionBase
{
    Q_OBJECT
public:
    explicit DeviceWasteContainer(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~DeviceWasteContainer();
    

private:
    void updateAlerts(DS_McuDef::WasteBinState state, DS_McuDef::WasteBinState prevState);
    void updateFluidSource();
    DS_DeviceDef::WasteContainerLevel getLevelFromState(DS_McuDef::WasteBinState state);
private slots:
    void slotAppInitialised();
};

#endif // DEVICE_WASTE_CONTAINER_H
