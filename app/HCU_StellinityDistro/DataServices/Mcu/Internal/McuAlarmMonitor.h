#ifndef MCU_ALARM_MONITOR_H
#define MCU_ALARM_MONITOR_H

#include <QObject>
#include <QTimer>
#include "Common/Common.h"
#include "DataServices/Alert/DS_AlertDef.h"
#include "McuAlarm.h"

class McuAlarmMonitor : public QObject
{
    Q_OBJECT
public:
    explicit McuAlarmMonitor(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~McuAlarmMonitor();

private:
    EnvGlobal *env;
    EnvLocal *envLocal;

    QVariantMap getHcuAlertFromMcuAlarm(McuAlarm::AlarmId alarmId);

private slots:
    void slotAppInitialised();
    void slotHandleMcuAlarms();
};

#endif // MCU_ALARM_MONITOR_H
