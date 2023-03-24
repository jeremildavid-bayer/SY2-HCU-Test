#ifndef MCU_MONITOR_H
#define MCU_MONITOR_H

#include <QObject>
#include <QTimer>
#include "Common/Common.h"

class McuMonitor : public QObject
{
    Q_OBJECT
public:
    explicit McuMonitor(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~McuMonitor();

private:
    EnvGlobal *env;
    EnvLocal *envLocal;
    QTimer tmrMcuCommMonitor;

    void checkMcuVersion();

private slots:
    void slotAppInitialised();
};

#endif // MCU_MONITOR_H
