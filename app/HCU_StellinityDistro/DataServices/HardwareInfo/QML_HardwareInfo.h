#ifndef QML_HARDWARE_INFO_H
#define QML_HARDWARE_INFO_H

#include "Common/Common.h"

class QML_HardwareInfo : public QObject
{
    Q_OBJECT
public:
    explicit QML_HardwareInfo(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~QML_HardwareInfo();
    Q_INVOKABLE void slotConfigChanged(QVariant configItem);

private:
    EnvGlobal *env;
    EnvLocal *envLocal;
    QObject *qmlSrc;

};

#endif // QML_HARDWARE_INFO_H
