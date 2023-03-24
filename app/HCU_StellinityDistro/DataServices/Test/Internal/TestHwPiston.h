#ifndef TEST_HW_PISTON_H
#define TEST_HW_PISTON_H

#include "Common/Common.h"
#include "Common/ActionBaseExt.h"
#include "DataServices/Test/DS_TestDef.h"

class TestHwPiston : public ActionBaseExt
{
    Q_OBJECT

public:
    explicit TestHwPiston(QObject *parent = 0, EnvGlobal *env_ = NULL, EnvLocal *envLocal_ = NULL);
    ~TestHwPiston();
    bool isBusy() { return state != STATE_IDLE; }

    DataServiceActionStatus actStart(QVariantList testParams, QString actGuid = "");
    DataServiceActionStatus actStop(QString actGuid = "");

private:

    enum State
    {
        STATE_IDLE = 0,
        STATE_STARTED,
        STATE_REPEATING,
        STATE_FIND_PLUNGER_STARTED,
        STATE_FIND_PLUNGER_PROGRESS,
        STATE_FIND_PLUNGER_DONE,
        STATE_STOPCOCK_TO_INJECT_STARTED,
        STATE_STOPCOCK_TO_INJECT_PROGRESS,
        STATE_STOPCOCK_TO_INJECT_DONE,
        STATE_STOPCOCK_TO_INJECT_FAILED,
        STATE_PUSH_PLUNGER_STARTED,
        STATE_PUSH_PLUNGER_PROGRESS,
        STATE_PUSH_PLUNGER_DONE,
        STATE_ENGAGE_STARTED,
        STATE_ENGAGE_PROGRESS,
        STATE_ENGAGE_DONE,
        STATE_INJECT_STARTED,
        STATE_INJECT_PROGRESS,
        STATE_INJECT_DONE,
        STATE_STOPCOCK_TO_FILL_STARTED,
        STATE_STOPCOCK_TO_FILL_PROGRESS,
        STATE_STOPCOCK_TO_FILL_DONE,
        STATE_FILL_STARTED,
        STATE_FILL_PROGRESS,
        STATE_FILL_DONE,
        STATE_FILL_PAUSE_STARTED,
        STATE_FILL_PAUSE_PROGRESS,
        STATE_FILL_PAUSE_DONE,
        STATE_DISENGAGE_STARTED,
        STATE_DISENGAGE_PROGRESS,
        STATE_DISENGAGE_DONE,
        STATE_DISENGAGE_PAUSE_STARTED,
        STATE_DISENGAGE_PAUSE_PROGRESS,
        STATE_DISENGAGE_PAUSE_DONE,
        STATE_COMPLETED,
        STATE_ALL_COMPLETED,
        STATE_ABORTED
    };

    struct TestParams
    {
        int cycles;
        int curCycleCount;
        bool pistonEnabled[3];
        bool pistonActionCompleted[3];
        bool disengageAfterFill;
        double injectVol;
        double injectFlow;
        double fillFlow;
        int pauseAfterFillMs;
        int pauseAfterDisengageMs;
        bool purgeAfterEngage;
    };

    TestParams testParams;

    void setStateSynch(int newState);
    void processState();

    void setPistonHw();
    void setStopcockHw();

};

#endif // TEST_HW_PISTON_H
