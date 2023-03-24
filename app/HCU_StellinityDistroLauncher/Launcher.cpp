#include <QCoreApplication>
#include <QFile>
#include "Launcher.h"

Launcher::Launcher(QObject *parent) :
    QObject(parent)
{
    log = new Log(QString(PATH_LOG_DIR) + "DistroLauncher.log", "");
    log->open();

    log->writeInfo("\n\n\n");
    log->writeInfo("===========================================================\n");
    log->writeInfo("HCU_Stellinity Distro Launcher Started..\n");

    logDistro = new Log(QString(PATH_LOG_DIR) + "Distro.log", "", LOG_LRG_SIZE_BYTES * 2);
    distroLogStart();

    qml.engine = new QQmlEngine();
    qml.context = new QQmlContext(qml.engine->rootContext());
    qml.component = new QQmlComponent(qml.engine, QUrl("qrc:/Launcher/Launcher.qml"));
    qml.object = nullptr;

    if (!qml.component->isLoading())
    {
        slotContinueLoading();
    }
    else
    {
        QObject::connect(qml.component, &QQmlComponent::statusChanged, this, &Launcher::slotContinueLoading);
    }
}

void Launcher::slotContinueLoading()
{

    if (qml.component->isError())
    {
        Q_FOREACH (const auto& error, qml.component->errors()) {
            log->writeError(QString().asprintf("QML error: %s\n", error.toString().toStdString().c_str()));
        }
        log->writeError(QString().asprintf("Failed to create QML Component. Error=%s\n", qml.component->errorString().toStdString().c_str()));
        slotExit();
        return;
    }

    qml.object = qml.component->create(qml.context);


    // Join signals and slots
    connect(qml.object, SIGNAL(qmlSignalClose()), this, SLOT(slotExit()));
    connect(qml.object, SIGNAL(qmlSignalLaunchStart()), this, SLOT(slotLaunchStart()));

    connect(&procApp, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotProcAppFinished(int,QProcess::ExitStatus)));
    connect(&procApp, SIGNAL(errorOccurred(QProcess::ProcessError)), this, SLOT(slotProcAppErrorOccurred(QProcess::ProcessError)));
    connect(&procApp, SIGNAL(readyReadStandardOutput()), this, SLOT(slotProcReadAll()));
    connect(&procApp, SIGNAL(readyReadStandardError()), this, SLOT(slotProcReadAll()));

    connect(&procSaveCrashLog, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotProcSaveCrashLogFinished(int,QProcess::ExitStatus)));

    updateShutdownInfoFile(true);

    slotLaunchStart();
}

Launcher::~Launcher()
{
    disconnect(&procApp, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotProcAppFinished(int,QProcess::ExitStatus)));
    disconnect(&procApp, SIGNAL(errorOccurred(QProcess::ProcessError)), this, SLOT(slotProcAppErrorOccurred(QProcess::ProcessError)));
    disconnect(&procApp, SIGNAL(readyReadStandardOutput()), this, SLOT(slotProcReadAll()));
    disconnect(&procApp, SIGNAL(readyReadStandardError()), this, SLOT(slotProcReadAll()));
    disconnect(&procSaveCrashLog, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotProcSaveCrashLogFinished(int,QProcess::ExitStatus)));

    procApp.close();
    procSaveCrashLog.close();

    log->close();
    logDistro->close();

    delete log;
    delete logDistro;
}

void Launcher::slotExit()
{
    QCoreApplication::quit();
}

void Launcher::updateShutdownInfoFile(bool isInitType)
{
    QFile fileBuf(PATH_SHUTDOWN_INFO_FILE);
    if (fileBuf.exists())
    {
        if (fileBuf.open(QFile::ReadOnly | QFile::Text))
        {
            QString shutDownInfoFileRead = fileBuf.readAll();
            log->writeInfo(QString().asprintf("Last Application Exit Info: \n%s\n", shutDownInfoFileRead.toStdString().c_str()));

            if (shutDownInfoFileRead.contains("NORMAL"))
            {
                // Last App Exit Reason - Normal
                lastAppExitReason = "NORMAL";
                return;
            }
            else
            {
                lastAppExitReason = isInitType ? "HW" : "SW";
                log->writeWarning(QString().asprintf("Last Application Exit is expected to be %s\n", lastAppExitReason.toStdString().c_str()));
            }
            fileBuf.close();
        }
    }
    else
    {
        lastAppExitReason = "NORMAL";
        log->writeInfo(QString().asprintf("Last Application Exit Info File not found(%s).. Creating one..\n", PATH_SHUTDOWN_INFO_FILE));
    }

    QString dateTimeStr = QDateTime::currentDateTimeUtc().toUTC().toString("yyyy-MM-ddTHH:mm:ss.zzzZ");

    log->writeDebug(QString().asprintf("Writing reason as %s...\n", lastAppExitReason.toStdString().c_str()));

    if (fileBuf.open(QFile::WriteOnly | QFile::Text))
    {
        fileBuf.write(dateTimeStr.toStdString().c_str());
        fileBuf.write("\n");
        fileBuf.write(lastAppExitReason.toStdString().c_str());
        fileBuf.write("\n");
        fileBuf.close();
    }
}

void Launcher::slotProcAppErrorOccurred(QProcess::ProcessError procError)
{
    QString procErrStr = procApp.errorString();

    logDistro->writeError("HCU_Distro procApp ErrorOccurred(" + procErrStr + ") \n");

    if (procError == QProcess::Crashed)
    {
        // Will be handled later..
    }
    else
    {
        log->writeWarning(QString().asprintf("Application failed to run, error(%d, %s). Restarting in %d sec..\n", procError, procErrStr.toStdString().c_str(), APP_RESTART_INTERVAL_SEC));
        QMetaObject::invokeMethod(qml.object, "slotStartCountDown", Q_ARG(QVariant, APP_RESTART_INTERVAL_SEC));
    }
}

void Launcher::slotProcAppFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    // Check if application crashed
    updateShutdownInfoFile(false);

    // Update Distro log
    slotProcReadAll();
    logDistro->writeInfo("HCU_Distro procApp Finished\n");
    distroLogStart();

    QString procErrStr = procApp.errorString();

    if (lastAppExitReason == "SW")
    {
        // SW Crash observed. Save crash log.
        log->writeWarning(QString().asprintf("Application crashed with code(%d), status(%d, %s). Capturing logs..\n", exitCode, exitStatus, procErrStr.toStdString().c_str()));
        QMetaObject::invokeMethod(qml.object, "slotSavingLogData");
        procSaveCrashLog.start(PATH_SAVE_USER_DATA, QStringList() << "-c");
    }
    else
    {
        // Start count down then start app
        log->writeInfo(QString().asprintf("Application exited with code(%d), status(%d, %s). Restarting in %d sec..\n", exitCode, exitStatus, procErrStr.toStdString().c_str(), APP_RESTART_INTERVAL_SEC));
        QMetaObject::invokeMethod(qml.object, "slotStartCountDown", Q_ARG(QVariant, APP_RESTART_INTERVAL_SEC));
    }
}

void Launcher::slotProcSaveCrashLogFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    log->writeDebug(QString().asprintf("LogData saved. Output=%s, exitCode=%d,%d\n", procSaveCrashLog.readAll().toStdString().c_str(), exitCode, exitStatus));
    procSaveCrashLog.close();
    QMetaObject::invokeMethod(qml.object, "slotStartCountDown", Q_ARG(QVariant, APP_RESTART_INTERVAL_SEC));
}

void Launcher::slotProcReadAll()
{
    QString stdOut = procApp.readAllStandardOutput();
    QString stdErr = procApp.readAllStandardError();

    if (stdOut != "")
    {
        logDistro->writeUnknown(stdOut);
    }

    if (stdErr != "")
    {
        logDistro->writeUnknown(stdErr);
    }
}

void Launcher::slotLaunchStart()
{
    log->writeDebug("App is started..\n\n");
    logDistro->writeInfo("HCU_Distro procApp start\n");
    procApp.close();
    procApp.start(APP_PATH, QStringList());
}

void Launcher::distroLogStart()
{
    logDistro->open();

    logDistro->writeInfo("\n\n\n");
    logDistro->writeInfo("===========================================================\n");
    logDistro->writeInfo("HCU_Stellinity Distro Log Started\n");
}

