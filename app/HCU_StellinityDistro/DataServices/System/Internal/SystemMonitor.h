#ifndef SYSTEM_MONITOR_H
#define SYSTEM_MONITOR_H

#include <QObject>
#include <QTimer>

#include "Common/Common.h"
#include "SystemMonitorOS.h"

class SystemMonitor : public QObject
{
    Q_OBJECT
public:
    explicit SystemMonitor(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~SystemMonitor();

private:
    EnvGlobal *env;
    EnvLocal *envLocal;
    SystemMonitorOS *systemMonitorOs;
    QTimer tmrRetryLinkSetup;

    void updateNetworkInterface();
    void handleBatteryCritical();
    void registerScreenWakeUpEvents();

private slots:
    void slotAppInitialised();
};

#endif // SYSTEM_MONITOR_H
