#ifndef DS_TEST_DATA_H
#define DS_TEST_DATA_H

#include "DS_TestDef.h"
#include "DataServices/DataServicesMacros.h"

class DS_TestData : public QObject
{
    Q_OBJECT

public:
    explicit DS_TestData(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~DS_TestData();

    static void registerDataTypesForThread()
    {
        qRegisterMetaType<DS_TestDef::TestStatus>("DS_TestDef::TestStatus");
    }

signals:
    // Define OUTGOING Signals (i.e. WHEN DATA CHANGED)
    CREATE_DATA_CHANGED_SIGNAL(bool, DataLocked)
    CREATE_DATA_CHANGED_SIGNAL(DS_TestDef::TestStatus, TestStatus)

private:
    // Creates GET/SET Properties
    CREATE_DATA_MEMBERS(bool, DataLocked)
    CREATE_DATA_MEMBERS(DS_TestDef::TestStatus, TestStatus)

    EnvGlobal *env;
    EnvLocal *envLocal;

private slots:
    void slotAppStarted();

};

#endif // DS_TEST_DATA_H
