#ifndef DEVICE_HEAT_MAINTAINER_H
#define DEVICE_HEAT_MAINTAINER_H

#include <QObject>
#include "Common/Common.h"
#include "Common/ActionBase.h"
#include "DataServices/Mcu/DS_McuData.h"

class DeviceHeatMaintainer : public ActionBase
{
    Q_OBJECT
public:
    explicit DeviceHeatMaintainer(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~DeviceHeatMaintainer();
    
private:
    bool isActive;
    QTimer tmrTemperatureMonitor;
    DS_McuDef::TemperatureReadings lastTemperatureReadings;

    void handleHeatMaintainerStatusChanged();

private slots:
    void slotAppInitialised();
};

#endif // DEVICE_HEAT_MAINTAINER_H
