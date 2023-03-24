#ifndef DEVICE_MONITOR_H
#define DEVICE_MONITOR_H

#include <QObject>
#include <QTimer>
#include "Common/Common.h"

class DeviceMonitor : public QObject
{
    Q_OBJECT
public:
    explicit DeviceMonitor(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~DeviceMonitor();

    void init();

private:
    EnvGlobal *env;
    EnvLocal *envLocal;

    bool isActive;
    void setAllowedSalinePackages();
    void setAllowedContrastPackages();

private slots:
    void slotAppInitialised();

};

#endif // DEVICE_MONITOR_H
