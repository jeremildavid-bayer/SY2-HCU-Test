#ifndef QML_UPGRADE_H
#define QML_UPGRADE_H

#include "Common/Common.h"
#include "DataServices/Upgrade/DS_UpgradeDef.h"

class QML_Upgrade : public QObject
{
    Q_OBJECT
public:
    explicit QML_Upgrade(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~QML_Upgrade();

    Q_INVOKABLE void slotUpdateSelectedPackageInfo(QString pathPackage);
    Q_INVOKABLE void slotUpgrade();

private:
    EnvGlobal *env;
    EnvLocal *envLocal;
    QObject *qmlSrc;
};

#endif // QML_UPGRADE_H
