#ifndef QML_EXAM_H
#define QML_EXAM_H

#include <QTimer>
#include "Common/Common.h"
#include "DataServices/Exam/DS_ExamDef.h"

class QML_Exam : public QObject
{
    Q_OBJECT
public:
    explicit QML_Exam(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~QML_Exam();

    Q_INVOKABLE void slotAirCheckDone();
    Q_INVOKABLE void slotScannerInterlocksCheckDone();
    Q_INVOKABLE void slotDefaultPlanTemplateSelected();
    Q_INVOKABLE void slotInjectionArmed();
    Q_INVOKABLE void slotInjectionStarted();
    Q_INVOKABLE void slotInjectionPaused();
    Q_INVOKABLE void slotInjectionAborted();
    Q_INVOKABLE void slotInjectionSkipped();
    Q_INVOKABLE void slotInjectionRepeat();
    Q_INVOKABLE void slotInjectionFlowAdjusted(bool up);
    Q_INVOKABLE void slotExamProgressStateChanged(QString examProgressState);
    Q_INVOKABLE void slotAdjustInjectionVolume();
    Q_INVOKABLE void slotPressureLimitChanged(int pressure);
    Q_INVOKABLE void slotContrastTypeChanged(QString brand, double concentration);
    Q_INVOKABLE void slotSUDSLengthChanged(QString length);
    Q_INVOKABLE void slotPlanChanged(QVariant plan);
    Q_INVOKABLE void slotPlanPreviewChanged(QVariant planTemplateMap);
    Q_INVOKABLE void slotLoadPlanTemplateFromPlan(QVariant planMap);
    Q_INVOKABLE void slotLoadPlanFromPlanPreview(QVariant planTemplateMap);
    Q_INVOKABLE void slotUpdateReviewPlot(int stepIndex);

private:
    EnvGlobal *env;
    EnvLocal *envLocal;
    QObject *qmlSrc;
    QFile injPlotDataFile;
    QTimer tmrInjPlotUpdate;

    void setExecutingStep(const DS_ExamDef::InjectionPlan &plan, const DS_ExamDef::InjectionStep *executingStep, const DS_ExamDef::InjectionStep *prevExecutingStep);
    void setIsContrastSelectAllowed();
    void setInjectionProgressMonitor();
    void setInjectionProgressPlotWrite(QString plotAction, QVariantList args = QVariantList());
    void setInjectionProgressPlotWriteHeader();
    void setInjectionProgressPlot(DS_ExamDef::ExecutedSteps executedSteps, DS_ExamDef::ExecutedSteps prevExecutedSteps);
    void setInjectionProgressPlotArmed(const DS_ExamDef::InjectionPlan &plan, const DS_ExamDef::InjectionStep &executingStep);
    void setInjectionProgressPlotExecuting(qint64 timePastMs, DS_SystemDef::StatePath lastStatePath, const DS_ExamDef::ExecutedStep &stepProgressDigest, const DS_ExamDef::InjectionStep &executingStep, const DS_ExamDef::InjectionPhase &phaseData, int lastPhaseIdx);
    void setInjectionProgressPlotBusyHolding(qint64 timePastMs, DS_SystemDef::StatePath lastStatePath, const DS_ExamDef::ExecutedStep &stepProgressDigest, qint64 curTimeEpochMs, qint64 lastPlotUpdatedEpochMs);
    void setInjectionProgressPlotBusyFinishing(qint64 timePastMs, DS_SystemDef::StatePath lastStatePath, const DS_ExamDef::ExecutedStep &stepProgressDigest);

private slots:
    void slotUpdateReviewPlotSamples();
};

#endif // QML_EXAM_H
