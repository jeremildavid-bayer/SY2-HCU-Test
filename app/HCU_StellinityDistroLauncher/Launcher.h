#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <QObject>
#include <QProcess>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include "Log.h"

#define APP_RESTART_INTERVAL_SEC        3
#define PATH_HOME                       "/home/user/Imaxeon"
#define PATH_USER                       "/IMAX_USER"
#define APP_PATH                        PATH_HOME "/bin/HCU_StellinityDistro"
#define UBUNTU_TASKBAR_COVER_PATH       PATH_HOME "/bin/HCU_UbuntuTaskbarCover"
#define PATH_SAVE_USER_DATA             PATH_HOME "/script/save_user_data.sh"
#define PATH_SHUTDOWN_INFO_FILE         PATH_USER "/db/shutdown.inf"
#define PATH_LOG_DIR                    PATH_USER "/log/"

class Launcher : public QObject
{
    Q_OBJECT

public:
    explicit Launcher(QObject *parent = 0);
    ~Launcher();

private:
    struct QmlParams
    {
        QQmlEngine *engine;
        QQmlComponent *component;
        QObject *object;
        QQmlContext *context;
    };

    QmlParams qml;
    QProcess procApp;
    QProcess procSaveCrashLog;
    Log *log;
    Log *logDistro;
    QString lastAppExitReason;

    void updateShutdownInfoFile(bool isInitType);
    void distroLogStart();

private slots:    
    void slotContinueLoading();
    void slotExit();
    void slotLaunchStart();
    void slotProcAppFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void slotProcAppErrorOccurred(QProcess::ProcessError procError);
    void slotProcSaveCrashLogFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void slotProcReadAll();
};

#endif // LAUNCHER_H
