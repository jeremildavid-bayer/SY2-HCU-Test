#ifndef TEST_NETWORK_H
#define TEST_NETWORK_H

#include "Common/ActionBaseExt.h"
#include "DataServices/Test/DS_TestDef.h"

class TestNetwork : public ActionBaseExt
{
    Q_OBJECT

public:
    explicit TestNetwork(QObject *parent = 0, EnvGlobal *env_ = NULL, EnvLocal *envLocal_ = NULL);
    ~TestNetwork();
    bool isBusy() { return state != STATE_IDLE; }

    DataServiceActionStatus actStart(QVariantList testParams, QString actGuid = "");
    DataServiceActionStatus actStop(QString actGuid = "");

private:

    enum State
    {
        STATE_IDLE = 0,
        STATE_STARTED,
        STATE_TEST_MESSAGE_PREPARING,
        STATE_TEST_MESSAGE_SENDING,
        STATE_TEST_MESSAGE_WAITING_REPLY,
        STATE_TEST_MESSAGE_TEST_REPORT_UPDATING,
        STATE_ABORTED
    };

    struct TestParams
    {
    };

    TestParams testParams;

    void setStateSynch(int newState);
    void processState();
};

#endif // TEST_NETWORK_H
