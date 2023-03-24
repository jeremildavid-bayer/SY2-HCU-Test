#ifndef DS_ALERT_DATA_H
#define DS_ALERT_DATA_H

#include "DS_AlertDef.h"
#include "Common/Common.h"
#include "DataServices/DataServicesMacros.h"

class DS_AlertData : public QObject
{
    Q_OBJECT

public:
    explicit DS_AlertData(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~DS_AlertData();

    static void registerDataTypesForThread()
    {
    }

signals:
    // Define OUTGOING Signals (i.e. WHEN DATA CHANGED)
    CREATE_DATA_CHANGED_SIGNAL(bool, DataLocked)
    CREATE_DATA_CHANGED_SIGNAL(QVariantList, AllAlerts)
    CREATE_DATA_CHANGED_SIGNAL(QVariantList, InactiveAlerts)
    CREATE_DATA_CHANGED_SIGNAL(QVariantList, ActiveAlerts)
    CREATE_DATA_CHANGED_SIGNAL(QVariantList, ActiveSystemAlerts)
    CREATE_DATA_CHANGED_SIGNAL(QVariantList, ActiveFatalAlerts)
    CREATE_DATA_CHANGED_SIGNAL(QVariantMap, ActiveFluidSourceAlerts)

private:
    // Creates GET/SET Properties
    CREATE_DATA_MEMBERS(bool, DataLocked)
    CREATE_DATA_MEMBERS(QVariantList, AllAlerts)
    CREATE_DATA_MEMBERS(QVariantList, InactiveAlerts)
    CREATE_DATA_MEMBERS(QVariantList, ActiveAlerts)
    CREATE_DATA_MEMBERS(QVariantList, ActiveSystemAlerts)
    CREATE_DATA_MEMBERS(QVariantList, ActiveFatalAlerts)
    CREATE_DATA_MEMBERS(QVariantMap, ActiveFluidSourceAlerts)

    EnvGlobal *env;

private slots:
    void slotAppInitialised();
};

#endif // DS_ALERT_DATA_H
