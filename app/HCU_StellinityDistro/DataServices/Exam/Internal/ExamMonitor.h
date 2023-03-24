#ifndef EXAM_MONITOR_H
#define EXAM_MONITOR_H

#include <QTimer>
#include "Common/Common.h"
#include "DataServices/Exam/DS_ExamDef.h"

class ExamMonitor : public QObject
{
    Q_OBJECT
public:
    explicit ExamMonitor(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~ExamMonitor();

private:
    EnvGlobal *env;
    EnvLocal *envLocal;
    QTimer tmrExamTimeout;

    void handlePersonalizedProtocol();
    void handleArmedStateFromCruLinkStatusChanged();
    void handleArmedStateWhileSyringesBusy();
    void handlePreloadProtocol(const DS_ExamDef::InjectionPlan &prevPlan);
    void handleManualPrimeWhileInjectionPreloaded();

private slots:
    void slotAppInitialised();
};

#endif // EXAM_MONITOR_H
