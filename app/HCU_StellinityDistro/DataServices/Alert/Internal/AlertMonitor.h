#ifndef ALERT_MONITOR_H
#define ALERT_MONITOR_H

#include <QTimer>
#include "Common/Common.h"

class AlertMonitor : public QObject
{
    Q_OBJECT
public:
    explicit AlertMonitor(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~AlertMonitor();

private:
    EnvGlobal *env;
    EnvLocal *envLocal;

    QTimer tmrSaveLastAlertsWaiting;

    void handleRemoteServicing();
    void handleActiveAlerts(const QVariantList &activeAlerts, const QVariantList &prevActiveAlerts);
    void handleActiveFatalAlerts();
    QVariantList getMergedSystemAlerts(const QVariantList &systemAlerts);

private slots:
    void slotAppInitialised();
};

#endif // ALERT_MONITOR_H
