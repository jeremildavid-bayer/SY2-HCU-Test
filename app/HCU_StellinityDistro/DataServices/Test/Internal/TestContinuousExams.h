#ifndef TEST_CONTINUOUS_EXAMS_H
#define TEST_CONTINUOUS_EXAMS_H

#include "Common/ActionBaseExt.h"
#include "DataServices/Test/DS_TestDef.h"

class TestContinuousExams : public ActionBaseExt
{
    Q_OBJECT

public:
    explicit TestContinuousExams(QObject *parent = 0, EnvGlobal *env_ = NULL, EnvLocal *envLocal_ = NULL);
    ~TestContinuousExams();
    bool isBusy() { return state != STATE_IDLE; }

    DataServiceActionStatus actStart(QVariantList testParams, QString actGuid = "");
    DataServiceActionStatus actStop(QString actGuid = "");

private:

    enum State
    {
        STATE_IDLE = 0,
        STATE_STARTED,

        STATE_REFILL_MONITOR_STARTED,
        STATE_REFILL_MONITOR_PROGRESS,
        STATE_REFILL_MONITOR_DONE,

        STATE_SUDS_PREPARING,
        STATE_SUDS_PREPARED,

        STATE_EXAM_START_WAITING,

        STATE_EXAM_PREPARE_STARTED,
        STATE_EXAM_PREPARE_PROGRESS,
        STATE_EXAM_PREPARE_DONE,
        STATE_EXAM_PREPARE_FAILED,

        STATE_EXAM_PLAN_SELECT_STARTED,
        STATE_EXAM_PLAN_SELECT_PROGRESS,
        STATE_EXAM_PLAN_SELECT_DONE,
        STATE_EXAM_PLAN_SELECT_FAILED,

        STATE_EXAM_STEP_PREPARING,
        STATE_EXAM_STEP_PREPARED,

        STATE_EXAM_ARM_STARTED,
        STATE_EXAM_ARM_PROGRESS,
        STATE_EXAM_ARM_DONE,
        STATE_EXAM_ARM_FAILED,

        STATE_EXAM_INJECTION_START_STARTED,
        STATE_EXAM_INJECTION_START_PROGRESS,
        STATE_EXAM_INJECTION_START_DONE,
        STATE_EXAM_INJECTION_START_FAILED,

        STATE_EXAM_INJECTION_MONITOR_STARTED,
        STATE_EXAM_INJECTION_MONITOR_PROGRESS,
        STATE_EXAM_INJECTION_MONITOR_DONE,
        STATE_EXAM_INJECTION_MONITOR_FAILED,

        STATE_EXAM_COMPLETE_STARTED,
        STATE_EXAM_COMPLETE_PROGRESS,
        STATE_EXAM_COMPLETE_DONE,
        STATE_EXAM_COMPLETE_FAILED,

        STATE_FAILED,
        STATE_ABORTED,
        STATE_DONE
    };

    QString guidExamPreparing;
    QStringList planTemplateGuids;
    int planTemplateGuidIndex;
    SyringeIdx activeContrastLocation;
    int executedExamsCount;
    qint64 testStartedAtEpochMs;
    QVariantList monitoredAlerts;
    int linkDroppedCount;

    void setStateSynch(int newState);
    QString getStateStr(int state);
    void updateTestStateStr();
    void processState();

private slots:
    void slotAppInitialised();
};

#endif // TEST_CONTINUOUS_EXAMS_H
