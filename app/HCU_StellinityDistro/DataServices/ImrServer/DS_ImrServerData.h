#ifndef DS_IMR_SERVER_DATA_H
#define DS_IMR_SERVER_DATA_H

#include "DS_ImrServerDef.h"
#include "DataServices/DataServicesMacros.h"

class DS_ImrServerData : public QObject
{
    Q_OBJECT

public:
    explicit DS_ImrServerData(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~DS_ImrServerData();

    static void registerDataTypesForThread()
    {
        qRegisterMetaType<DS_ImrServerDef::ImrRequests>("DS_ImrServerDef::ImrRequests");
    }

signals:
    // Define OUTGOING Signals (i.e. WHEN DATA CHANGED)
    CREATE_DATA_CHANGED_SIGNAL(bool, DataLocked)
    CREATE_DATA_CHANGED_SIGNAL(DS_ImrServerDef::ImrRequests, LastImrCruRequests)
    CREATE_DATA_CHANGED_SIGNAL(int, ClientCount)
    CREATE_DATA_CHANGED_SIGNAL(QVariantMap, DataGroup)


private:
    // Creates GET/SET Properties
    CREATE_DATA_MEMBERS(bool, DataLocked)
    CREATE_DATA_MEMBERS(DS_ImrServerDef::ImrRequests, LastImrCruRequests)
    CREATE_DATA_MEMBERS(int, ClientCount)
    CREATE_DATA_MEMBERS(QVariantMap, DataGroup)

    EnvGlobal *env;
    EnvLocal *envLocal;

private slots:
    void slotAppStarted();

};

#endif // DS_IMR_SERVER_DATA_H
