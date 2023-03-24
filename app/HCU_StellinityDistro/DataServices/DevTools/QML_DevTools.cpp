#include "QML_DevTools.h"

QML_DevTools::QML_DevTools(QObject *parent, EnvGlobal *env_) :
    QObject(parent), env(env_)
{
    envLocal = new EnvLocal("QML_DevTools", "QML_DEVTOOLS");
    qmlSrc = env->qml.object->findChild<QObject*>("dsDevTools");
    env->qml.engine->rootContext()->setContextProperty("dsDevToolsCpp", this);

    if (qmlSrc == NULL)
    {
        LOG_ERROR("Failed to assign qmlSrc.\n");
    }

    connect(&procDevShell, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)> (&QProcess::finished), this, [=] { slotProcDevShellFinished(""); });
    connect(&procDevShell, static_cast<void (QProcess::*)(QProcess::ProcessError)> (&QProcess::errorOccurred), this, [=] (QProcess::ProcessError err){ slotProcDevShellFinished(QString().asprintf("%d",err)); });
}

QML_DevTools::~QML_DevTools()
{
    procDevShell.close();
    delete envLocal;
}


void QML_DevTools::slotShellCommand(QString command, QString args)
{
    procDevShell.setProgram(command);
    QStringList argumentList;

    if (args != "")
    {
        argumentList = args.split(" ");
    }

    procDevShell.setArguments(argumentList);

    procDevShell.start();
}


void QML_DevTools::slotProcDevShellFinished(QString err)
{
    QString output = "";

    if (env->state == EnvGlobal::STATE_EXITING)
    {
        return;
    }

    if (err != "")
    {
        output += QString().asprintf("Error \"%s\" returned from shell!", err.CSTR());
        output += "\n StandardOutput : " + procDevShell.readAllStandardOutput().trimmed();
        output += "\n StandardError : " + procDevShell.readAllStandardError().trimmed();
    }
    else
    {
        output += procDevShell.readAllStandardOutput().trimmed();
    }

    procDevShell.close();
    qmlSrc->setProperty("shellOutput", output);
}
