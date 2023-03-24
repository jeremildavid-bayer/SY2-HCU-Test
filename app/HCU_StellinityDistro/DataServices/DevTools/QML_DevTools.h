#ifndef QML_DEVTOOLS_H
#define QML_DEVTOOLS_H

#include <QProcess>
#include "Common/Common.h"

class QML_DevTools : public QObject
{
    Q_OBJECT
public:
    explicit QML_DevTools(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~QML_DevTools();

    Q_INVOKABLE void slotShellCommand(QString command, QString args);

private:
    EnvGlobal *env;
    EnvLocal *envLocal;
    QObject *qmlSrc;
    QProcess procDevShell;

private slots:
    void slotProcDevShellFinished(QString err);
};

#endif // QML_DEVTOOLS_H
