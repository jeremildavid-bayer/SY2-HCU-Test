#ifndef QML_IMR_SERVER_H
#define QML_IMR_SERVER_H

#include "Common/Common.h"
#include "DataServices/ImrServer/DS_ImrServerDef.h"

class QML_ImrServer : public QObject
{
    Q_OBJECT
public:
    explicit QML_ImrServer(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~QML_ImrServer();

private:
    EnvGlobal *env;
    EnvLocal *envLocal;
    QObject *qmlSrc;
};

#endif // QML_IMR_SERVER_H
