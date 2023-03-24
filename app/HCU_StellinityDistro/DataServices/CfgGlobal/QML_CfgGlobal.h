#ifndef QML_CFG_GLOBAL_H
#define QML_CFG_GLOBAL_H

#include "Common/Common.h"

class QML_CfgGlobal : public QObject
{
    Q_OBJECT
public:
    explicit QML_CfgGlobal(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~QML_CfgGlobal();
    Q_INVOKABLE void slotConfigChanged(QVariant configItem);

private:
    EnvGlobal *env;
    EnvLocal *envLocal;
    QObject *qmlSrc;

};

#endif // QML_CFG_GLOBAL_H
