#ifndef QML_SYSTEM_H
#define QML_SYSTEM_H

#include "Common/Common.h"
#include "DataServices/System/DS_SystemDef.h"

class QML_System : public QObject
{
    Q_OBJECT
public:
    explicit QML_System(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~QML_System();

    Q_INVOKABLE void slotStartupScreenExit();
    Q_INVOKABLE void slotServiceModeActive(bool serviceModeActive);
    Q_INVOKABLE void slotShutdown(bool isShutdownType);
    Q_INVOKABLE void slotFactoryReset();
    Q_INVOKABLE void slotSaveUserData();
    Q_INVOKABLE void slotScreenWakeup();
    Q_INVOKABLE void slotSetScreenSleepTime(int sleepTimeMinutes);
    Q_INVOKABLE void slotSetScreenSleepTimeToDefault();

private:
    EnvGlobal *env;
    EnvLocal *envLocal;
    QObject *qmlSrc;

};

#endif // QML_SYSTEM_H
