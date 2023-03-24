#ifndef QML_CFG_LOCAL_H
#define QML_CFG_LOCAL_H

#include "Common/Common.h"

class QML_CfgLocal : public QObject
{
    Q_OBJECT
public:
    explicit QML_CfgLocal(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~QML_CfgLocal();

    Q_INVOKABLE void slotUiThemeChanged(QString uiTheme);
    Q_INVOKABLE void slotConfigChanged(QVariantMap configItem);
    Q_INVOKABLE void slotFluidOptionsChanged(QVariantMap fluidOptions);
    Q_INVOKABLE void slotHeaterActive(bool active);
    Q_INVOKABLE void slotLoadSampleFluidOptions();
    Q_INVOKABLE void slotLoadSampleInjectionPlans();
    Q_INVOKABLE void slotSetLastPMReminderAtToNow();
    Q_INVOKABLE void slotSetLastPMPerformedAtToNow();

private:
    EnvGlobal *env;
    EnvLocal *envLocal;
    QObject *qmlSrc;

    void setScreenGeometry();
};

#endif // QML_CFG_LOCAL_H
