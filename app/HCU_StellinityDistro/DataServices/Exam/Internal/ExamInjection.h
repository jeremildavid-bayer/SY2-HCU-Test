#ifndef EXAM_INJECTION_H
#define EXAM_INJECTION_H

#include <QObject>
#include <QTimer>
#include "Common/Common.h"
#include "DataServices/Exam/DS_ExamDef.h"
#include "DataServices/Mcu/DS_McuDef.h"

class ExamInjection : public QObject
{
    Q_OBJECT
public:
    explicit ExamInjection(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~ExamInjection();

private:
    EnvGlobal *env;
    EnvLocal *envLocal;
    QTimer tmrInjectionProgressUpdate;
    qint64 injectionPrgressUpdatedAtEpochMs;

    DS_ExamDef::PhaseProgressDigest getPhaseProgressDigest(const DS_ExamDef::ExecutedStep &stepProgress, const DS_ExamDef::InjectionPhase &phase, int phaseIndex);
    DS_SystemDef::StatePath getStatePathFromMcuInjectorState(DS_McuDef::InjectorState state);

    DS_DeviceDef::FluidSourceIdx getActiveContrastLocation(const DS_ExamDef::ExecutedStep &stepProgressDigest, int mcuPhaseIdx);
    void setLastPhaseProgress(DS_ExamDef::ExecutedStep &stepProgressDigest, const DS_ExamDef::InjectionStep *executingStep);
    void handleDisarmedState();
    void handleInjectionPlanChanged();
    void handleInjectDigest();
    void activateAlertSalineReservoirContrastPossible(const DS_ExamDef::ExecutedStep &stepProgressDigest, const DS_ExamDef::InjectionStep *executingStep);
    void updateMaxPressure(DS_ExamDef::PhaseProgressDigest &phaseProgressDigest, int phaseIndex, int curPressureKpa);
    void updateMaxFlowRate(DS_ExamDef::PhaseProgressDigest &phaseProgressDigest, int phaseIndex, double curFlowRateTotal);

private slots:
    void slotAppInitialised();
    void slotMcuDataChanged_InjectionProgress();

};

#endif // EXAM_INJECTION_H
