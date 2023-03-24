#ifndef TEST_HW_MCU_SIM_STOPCOCK_H
#define TEST_HW_MCU_SIM_STOPCOCK_H

#include "Common/ActionBaseExt.h"
#include "DataServices/Test/DS_TestDef.h"

class TestHwStopcock : public ActionBaseExt
{
    Q_OBJECT

public:
    explicit TestHwStopcock(QObject *parent = 0, EnvGlobal *env_ = NULL, EnvLocal *envLocal_ = NULL);
    ~TestHwStopcock();
    bool isBusy() { return state != STATE_IDLE; }

    DataServiceActionStatus actStart(QVariantList testParams, QString actGuid = "");
    DataServiceActionStatus actStop(QString actGuid = "");

private:

    enum State
    {
        STATE_IDLE = 0,
        STATE_STARTED,
        STATE_REPEATING,

        STATE_MOVE_INJ_TO_FILL_STARTED,
        STATE_MOVE_INJ_TO_FILL_PROGRESS,
        STATE_MOVE_INJ_TO_FILL_DONE,
        STATE_MOVE_INJ_TO_FILL_PAUSE_STARTED,
        STATE_MOVE_INJ_TO_FILL_PAUSE_PROGRESS,
        STATE_MOVE_INJ_TO_FILL_PAUSE_DONE,

        STATE_MOVE_FILL_TO_INJ_STARTED,
        STATE_MOVE_FILL_TO_INJ_PROGRESS,
        STATE_MOVE_FILL_TO_INJ_DONE,
        STATE_MOVE_FILL_TO_INJ_PAUSE_STARTED,
        STATE_MOVE_FILL_TO_INJ_PAUSE_PROGRESS,
        STATE_MOVE_FILL_TO_INJ_PAUSE_DONE,

        STATE_MOVE_INJ_TO_CLOSE_STARTED,
        STATE_MOVE_INJ_TO_CLOSE_PROGRESS,
        STATE_MOVE_INJ_TO_CLOSE_DONE,
        STATE_MOVE_INJ_TO_CLOSE_PAUSE_STARTED,
        STATE_MOVE_INJ_TO_CLOSE_PAUSE_PROGRESS,
        STATE_MOVE_INJ_TO_CLOSE_PAUSE_DONE,

        STATE_MOVE_CLOSE_TO_FILL_STARTED,
        STATE_MOVE_CLOSE_TO_FILL_PROGRESS,
        STATE_MOVE_CLOSE_TO_FILL_DONE,
        STATE_MOVE_CLOSE_TO_FILL_PAUSE_STARTED,
        STATE_MOVE_CLOSE_TO_FILL_PAUSE_PROGRESS,
        STATE_MOVE_CLOSE_TO_FILL_PAUSE_DONE,

        STATE_MOVE_FILL_TO_CLOSE_STARTED,
        STATE_MOVE_FILL_TO_CLOSE_PROGRESS,
        STATE_MOVE_FILL_TO_CLOSE_DONE,
        STATE_MOVE_FILL_TO_CLOSE_PAUSE_STARTED,
        STATE_MOVE_FILL_TO_CLOSE_PAUSE_PROGRESS,
        STATE_MOVE_FILL_TO_CLOSE_PAUSE_DONE,

        STATE_MOVE_CLOSE_TO_INJ_STARTED,
        STATE_MOVE_CLOSE_TO_INJ_PROGRESS,
        STATE_MOVE_CLOSE_TO_INJ_DONE,
        STATE_MOVE_CLOSE_TO_INJ_PAUSE_STARTED,
        STATE_MOVE_CLOSE_TO_INJ_PAUSE_PROGRESS,
        STATE_MOVE_CLOSE_TO_INJ_PAUSE_DONE,

        STATE_COMPLETED,
        STATE_ALL_COMPLETED,
        STATE_ABORTED
    };

    struct TestParams
    {
        int cycles;
        int curCycleCount;
        bool stopcockEnabled[3];
        int pauseAfterMoveMs;
    };

    TestParams testParams;

    void setStateSynch(int newState);
    void processState();

    void setStopcockHw();


private slots:


};

#endif // TEST_HW_MCU_SIM_STOPCOCK_H
