#ifndef QML_ALERT_H
#define QML_ALERT_H

#include "Common/Common.h"
#include "DataServices/Alert/DS_AlertDef.h"

class QML_Alert : public QObject
{
    Q_OBJECT
public:
    explicit QML_Alert(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~QML_Alert();
    Q_INVOKABLE void slotActivateAlert(QString codeName, QString data);
    Q_INVOKABLE void slotDeactivateAlert(QString codeName, QString data);
    Q_INVOKABLE void slotDeactivateAlertWithReason(QString codeName, QString newData, QString oldData, bool ignoreData);
    Q_INVOKABLE QVariantMap slotGetActiveAlertFromCodeName(QString codeName);
    Q_INVOKABLE QVariantMap slotGetAlertFromCodeName(QString codeName);
    Q_INVOKABLE QVariantList slotGetMergedAlerts(QVariantList alerts);

private:
    EnvGlobal *env;
    EnvLocal *envLocal;
    QObject *qmlSrc;

private slots:

};

#endif // QML_ALERT_H
