#ifndef SYSTEM_MONITOR_OS_H
#define SYSTEM_MONITOR_OS_H

#include <QTimer>
#include <QProcess>
#include "Common/Common.h"
#include "DataServices/System/DS_SystemData.h"
#include "DataServices/Cru/DS_CruData.h"

class SystemMonitorOS : public QObject
{
    Q_OBJECT
public:
    explicit SystemMonitorOS(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~SystemMonitorOS();

private:
    EnvGlobal *env;
    EnvLocal *envLocal;

    bool lastCruLinkStateIsInactive;
    DS_SystemDef::IwconfigParams iwconfigParams;
    QList<DS_SystemDef::IwconfigParams> iwconfigParamsHistory;

    QTimer tmrMonitorTimeShifted;
    QTimer tmrMonitorMainHdd;
    QTimer tmrMonitorUserHdd;
    QTimer tmrMonitorUsbStickInserted;
    QTimer tmrMonitorHcuTemperatureHw;
    QTimer tmrMonitorCpuMemUsage;
    QTimer tmrMonitorCruLink;
    QTimer tmrMonitorIwconfig;

    QProcess procMonitorTimeShifted;
    QProcess procMonitorMainHdd;
    QProcess procMonitorUserHdd;
    QProcess procMonitorUsbStickInserted;
    QProcess procMonitorHcuTemperatureHw;
    QProcess procMonitorCpuMemUsage;
    QProcess procMonitorCruLink;
    QProcess procMonitorIwconfig;

private slots:
    void slotAppInitialised();
    void slotMonitorTimeShifted();
    void slotProcMonitorMainHddFinished(QString err);
    void slotProcMonitorUserHddFinished(QString err);
    void slotProcMonitorUsbStickInsertedFinished(QString err);
    void slotProcMonitorHcuTemperatureHwFinished(QString err);
    void slotProcMonitorCpuMemUsageFinished(QString err);
    void slotProcMonitorCruLinkFinished(QString err);
    void slotProcMonitorIwconfigFinished(QString err);

};

#endif // SYSTEM_MONITOR_OS_H
