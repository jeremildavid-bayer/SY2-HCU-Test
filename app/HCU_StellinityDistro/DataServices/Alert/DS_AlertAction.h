#ifndef DS_ALERT_ACTION_H
#define DS_ALERT_ACTION_H

#include "DS_AlertDef.h"
#include "Common/Common.h"
#include "Internal/AlertMonitor.h"

class DS_AlertAction : public QObject
{
    Q_OBJECT

public:
    explicit DS_AlertAction(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~DS_AlertAction();

    void activate(QString codeName, QString data = "", QString reporter = "HCU");
    void activate(QVariantMap alert);

    void deactivate(QString codeName, QString data = "");
    void deactivateWithReason(QString codeName, QString newData, QString oldData = "", bool ignoreData = true);
    void deactivateFromSyringeIdx(QString codeName, SyringeIdx syringeIdx);

    void setActiveAlertData(QString guid, QString data);

    void update(QString codeName, QString data, QString status);
    void update(QString guid, QString status);
    void update(int activeAlertIdx, QString status);

    void remove(QString guid);
    void remove(QVariantList guidList);
    void removeAll();

    bool isActivated(QVariantList alertMap, QString codeName, QString data = "", bool ignoreData = false);
    bool isActivated(QString codeName, QString data = "", bool ignoreData = false);
    bool isActivated(QVariantMap alert);
    bool isActivatedWithSyringeIdx(QString codeName, SyringeIdx syringeIdx);
    bool isLastOccurredWithSyringeIdx(QString codeName, SyringeIdx syringeIdx);

    void saveLastAlerts();

    QString getActiveAlertGuid(QString codeName, QString data = "", bool ignoreData = false);
    QString getAlertGuid(QString codeName, QString data = "", bool ignoreData = false);
    QVariantMap getFromGuid(QString guid = EMPTY_GUID);
    QVariantMap getActiveAlert(QString codeName, QString data = "", bool ignoreData = false);
    QVariantMap getAlert(QString codeName, QString data = "", bool ignoreData = false);
    QVariantMap prepareAlert(QString codeName, QString data = "");
    QVariantList getMergedAlerts(const QVariantList &alerts);

    QVariantList getAlertsFromTimeOffset(QDateTime from);

private:
    EnvGlobal *env;
    EnvLocal *envLocal;
    AlertMonitor *monitor;
    QVariantMap alertDescriptions;
    QVariantMap alertBufferOverflowOccurred;

    void loadAlertDescriptions();
    void addAlert(QVariantMap alert);
    void updateAllAlerts();
    void insertAlertItem(QVariantList &target, int index, QVariantMap newItem);
    void insertAlertItem(QVariantList &target, int targetStartIdx, int targetEndIdx, QVariantMap newAlert);
    void removeAlertItem(QVariantList &target, int index);
    void restoreLastSavedAlerts();
    void populateAlertFields(QVariantMap &alert);

private slots:
    void slotAppInitialised();
};

#endif // DS_ALERT_ACTION_H
