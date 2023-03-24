#ifndef DS_EXAM_ACTION_H
#define DS_EXAM_ACTION_H

#include "Common/Common.h"
#include "Common/ActionBase.h"
#include "Internal/ExamMonitor.h"
#include "Internal/ExamInjection.h"
#include "DS_ExamDef.h"
#include "DataServices/Mcu/DS_McuDef.h"

class DS_ExamAction : public ActionBase
{
    Q_OBJECT

public:
    explicit DS_ExamAction(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~DS_ExamAction();

    DataServiceActionStatus actSetContrastFluidLocation(SyringeIdx contrastSyringeIdx, QString actGuid = "");
    DataServiceActionStatus actSetContrastFluidLocation(DS_DeviceDef::FluidSourceIdx contrastFluidLocation, QString actGuid = "");
    DataServiceActionStatus actSetContrastFluidLocationByPlan(DS_ExamDef::InjectionPlan plan, QString actGuid = "");
    DataServiceActionStatus actReloadSelectedContrast(QString brand = "", double concentration = 0, QString actGuid = "");
    DataServiceActionStatus actGetAvailableVolumesForInjection(DS_DeviceDef::FluidSourceIdx contrastFluidLocation, double &salineVolume, double &contrastVolume, double &salineVolumeAvailable, double &contrastVolumeAvailable, QString actGuid = "");
    DataServiceActionStatus actInjectionSelect(DS_ExamDef::InjectionPlan plan, DS_ExamDef::InjectionPlan planTemplate, QString actGuid = "");
    DataServiceActionStatus actInjectionProgram(DS_ExamDef::InjectionPlan plan, QString actGuid = "");
    DataServiceActionStatus actInjectionProgramReadyCheck(DS_ExamDef::InjectionPlan plan, QString actGuid = "");
    DataServiceActionStatus actInjectionStart(QString actGuid = "");
    DataServiceActionStatus actInjectionStop(QString actGuid = "");
    DataServiceActionStatus actInjectionHold(QString actGuid = "");
    DataServiceActionStatus actInjectionJump(int jumpToIdx, QString actGuid = "");
    DataServiceActionStatus actInjectionAdjustFlowRate(double delta, QString actGuid = "");
    DataServiceActionStatus actInjectionRepeat(QString actGuid = "");
    DataServiceActionStatus actHandleInsertedInjectionStepOrPhase(DS_ExamDef::InjectionPlan &plan, QString actGuid = "");
    DataServiceActionStatus actResetPreloadSteps(QString actGuid = "");
    DataServiceActionStatus actGetPreloadStepsFromStep(DS_ExamDef::InjectionStep &preloadStep, DS_ExamDef::InjectionStep &injectStep, const DS_ExamDef::InjectionStep &srcStep, QString actGuid = "");
    DataServiceActionStatus actGetMcuProtocolFromStep(const DS_ExamDef::InjectionStep &step, DS_McuDef::InjectionProtocol &injectProtocol, QString actGuid = "");
    DataServiceActionStatus actGetMcuPhasesFromHcuPhase(const DS_ExamDef::InjectionPhase &hcuPhase, DS_McuDef::InjectionPhases &mcuPhases, double &contrastVolumeAvailable1, double &contrastVolumeAvailable2, SyringeIdx preferredContrastSyringe, bool &crossOverVolumeTrimmed, QString actGuid = "");
    DataServiceActionStatus actGetHcuPhaseIdxFromMcuPhaseIdx(int &hcuPhaseIdx, int mcuPhaseIdx, const DS_McuDef::InjectionProtocol *mcuInjectProtocol = NULL, QString actGuid = "");
    DataServiceActionStatus actGetHcuPhaseInjectedVolsFromMcuInjectDigest(DS_ExamDef::FluidVolumes &injectedVols, const DS_McuDef::InjectDigest &injectDigest, int curPhaseIndex, QString actGuid = "");
    DataServiceActionStatus actGetPreloadedInjectDigest(DS_McuDef::InjectDigest &injectDigestDst, const DS_McuDef::InjectDigest &injectDigestSrc, QString actGuid = "");
    DataServiceActionStatus actGetPulseSalineVolume(const DS_ExamDef::InjectionStep &step, double &salineVol, QString actGuid = "");
    DataServiceActionStatus actArmReadyCheck(DS_ExamDef::ArmType type, const DS_ExamDef::InjectionStep &step, QString actGuid = "");
    DataServiceActionStatus actArm(DS_ExamDef::ArmType type = DS_ExamDef::ARM_TYPE_NORMAL, QString actGuid = "");
    DataServiceActionStatus actDisarm(QString actGuid = "");
    DataServiceActionStatus actGetInjectionTerminationReason(DS_ExamDef::StepTerminationReason &hcuTerminatedReason, QString &mcuTerminatedReason, QString &mcuTerminatedReasonMessage, const DS_McuDef::InjectionCompleteStatus &mcuInjectionCompletedStatus, QString actGuid = "");
    DataServiceActionStatus actScannerInterlocksBypass(QString actGuid = "");
    DataServiceActionStatus actExamPrepare(QString actGuid = "");
    DataServiceActionStatus actExamStart(bool startedFromCru = false, QString actGuid = "");
    DataServiceActionStatus actExamEnd(bool endedFromCru = false, QString actGuid = "");
    DataServiceActionStatus actExamReset(QString actGuid = "");
    DataServiceActionStatus actExamRestore(QString examGuid, qint64 examStartedAtEpochMs, DS_ExamDef::ExamProgressState examProgressState, DS_ExamDef::InjectionPlan injectionPlan, DS_ExamDef::ExecutedSteps executedStep, QString actGuid = "");
    DataServiceActionStatus actAdjustInjectionVolumes(QString actGuid = "");
    DataServiceActionStatus actAutoRefill(QString reason, QString actGuid = "");


private:
    struct FillActionStatus
    {
        QString actGuid[SYRINGE_IDX_MAX];
        bool started[SYRINGE_IDX_MAX];
        bool completed[SYRINGE_IDX_MAX];
    };

    ExamMonitor *monitor;
    ExamInjection *injection;
    QMap<QString, FillActionStatus> fillActionStatusMap;

    // gets result of adjusted injection volumes if it can be adjusted
    bool canAdjustInjectionVolumes(DS_ExamDef::InjectionStep *adjustedStep, QString *err);

    // called inside canAdjustInjectionVolumes.
    DS_ExamDef::InjectionStep getStepWithAdjustedInjectionVolume(const DS_ExamDef::InjectionStep &curStep, double volumeAvailable, bool isContrast, QString *err);

    void handleInsertedTestInjectionStep(DS_ExamDef::InjectionPlan &plan);
    void handleInsertedInjectionStep(DS_ExamDef::InjectionPlan &plan, const DS_ExamDef::InjectionPlan &prevPlan);
    void handleInsertedInjectionPhase(DS_ExamDef::InjectionStep &step, const DS_ExamDef::InjectionStep &prevStep);
    double getPreloadVolumeForSyringeIdx(SyringeIdx idx);

    void resetSUDSLength(bool extendedSUDSAAvailable);
    void storeSyringesUsedInLastExam();

    bool armReadyCheckWaitForCruDataCheck(DataServiceActionStatus &status);

    bool armReadyCheckIsInjectionValid(const DS_ExamDef::InjectionStep &curStep, DataServiceActionStatus &status);
    bool armReadyCheckMudsCheck(DataServiceActionStatus &status);
    bool armReadyCheckSudsCheck(DataServiceActionStatus &status);
    bool armReadyCheckExamInputProgressCheck(DataServiceActionStatus &status);
    bool armReadyCheckP3TParameters(const DS_ExamDef::InjectionStep &curStep, DataServiceActionStatus &status);
    bool armReadyCheckP3TUnusableConcentration(DataServiceActionStatus &status);
    bool armReadyCheckSI2009NonLoadedConcentrationCheck(DataServiceActionStatus &status);
    bool armReadyCheckSyringeReadyCheck(const DS_ExamDef::InjectionStep &curStep, DataServiceActionStatus &status);
    bool armReadyCheckInjectorBusyCheck(DataServiceActionStatus &status);
    bool armReadyCheckCatheterLimitCheck(const DS_ExamDef::InjectionStep &curStep, DataServiceActionStatus &status);
    bool armReadyCheckInjectionInsufficientVolumeCheck(DS_ExamDef::ArmType type, const DS_ExamDef::InjectionStep &curStep, DataServiceActionStatus &status);
    bool armReadyCheckRemainingStepsInsufficientVolumeCheck(const DS_ExamDef::InjectionStep &curStep, DataServiceActionStatus &status);
    bool armReadyCheckScannerInterlocksCheck(DataServiceActionStatus &status);
    bool armReadyCheckAirCheckOK(DataServiceActionStatus &status);


private slots:
    void slotAppInitialised();
};

#endif // DS_EXAM_ACTION_H
