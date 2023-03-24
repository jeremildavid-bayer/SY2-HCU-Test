#ifndef QML_CAPABILITIES_H
#define QML_CAPABILITIES_H

#include "Common/Common.h"

class QML_Capabilities : public QObject
{
    Q_OBJECT
public:
    explicit QML_Capabilities(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~QML_Capabilities();
    Q_INVOKABLE void slotConfigChanged(QVariant configItem);

private:
    EnvGlobal *env;
    EnvLocal *envLocal;
    QObject *qmlSrc;

};

#endif // QML_CAPABILITIES_H
