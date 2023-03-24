#ifndef DS_TEST_ACTION_H
#define DS_TEST_ACTION_H

#include "DS_TestDef.h"
#include "Common/Common.h"
#include "Common/ActionBase.h"
#include "Internal/TestHwPiston.h"
#include "Internal/TestHwStopcock.h"
#include "Internal/TestNetwork.h"
#include "Internal/TestContinuousExams.h"

class DS_TestAction : public ActionBase
{
    Q_OBJECT

public:
    explicit DS_TestAction(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~DS_TestAction();

    DataServiceActionStatus actStart(DS_TestDef::TestType type,  QVariantList testParams = {}, QString actGuid = "");
    DataServiceActionStatus actStop(QString actGuid = "");

private:
    TestHwPiston *testHwPiston;
    TestHwStopcock *testHwStopcock;
    TestNetwork *testNetwork;
    TestContinuousExams *testContinuousExams;

private slots:
    void slotAppInitialised();
};

#endif // DS_TEST_ACTION_H
