#ifndef DEVICE_SUDS_H
#define DEVICE_SUDS_H

#include <QObject>
#include <QTimer>
#include "Common/Common.h"
#include "Common/ActionBaseExt.h"
#include "Common/HwCapabilities.h"
#include "DataServices/Mcu/DS_McuData.h"

class DeviceSuds : public ActionBaseExt
{
    Q_OBJECT
public:
    explicit DeviceSuds(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~DeviceSuds();

    DataServiceActionStatus actPrimeInit(SyringeIdx syringeIdx = SYRINGE_IDX_SALINE, QString actGuid = "");
    DataServiceActionStatus actPrimeUpdateState(SyringeIdx syringeIdx = SYRINGE_IDX_SALINE, QString actGuid = "");
    DataServiceActionStatus actSetNeedsReplace(bool needed, QString actGuid = "");
    DataServiceActionStatus actSetStateNotPrimed(bool clearProgress = true, QString actGuid = "");
    DataServiceActionStatus actSetStatePrimed(QString actGuid = "");

    // With current design, SUDS does not record which contrast syringe was used for the content inside.
    // as a workaround, Exam module will set active contrast syringe location whenever this changes.
    // this was done as part of SWSRUSW-3447 fix
    DataServiceActionStatus actSetActiveContrastLocation(SyringeIdx syringeIdx, QString actGuid = "");

private:
    enum State
    {
        STATE_INIT = 0,
        STATE_INACTIVE,

        STATE_MISSING,
        STATE_NOT_PRIMED,
        STATE_PRIMING,
        STATE_PRIMED,
        STATE_NEEDS_REPLACE,
        STATE_PRIME_FAILED
    };

    struct PrimedFluid
    {
        SyringeIdx syringeIdx;
        SyringeIdx fluidType;
        double primedVol;

        PrimedFluid()
        {
            syringeIdx = SYRINGE_IDX_SALINE;
            fluidType = SYRINGE_IDX_SALINE;
            primedVol = 0;
        }
    };

    struct PrimedPressureData
    {
        int timeMs;
        int pressure;

        PrimedPressureData()
        {
			timeMs = 0;
            pressure = 0.0;
        }
    };

    double primeStartSyringeVol;    
    QList<PrimedFluid> listPrimedFluids;    
    int primedPercentage;

    qint64 primeStartEpochMs;
    bool primeStartManifoldHasContrast;
    QList<PrimedPressureData> listPrimedPressures;

    QTimer tmrSudsPortExposedTimeout;

    SyringeIdx activeContrastSyringe;

    bool isSetStateReady();
    void processState();

    void initPrime(SyringeIdx syringeIdx);
    SyringeIdx getPrimingSyringeIdxFromStopcockPos();
    void updatePrimeProgress(SyringeIdx primeSyringeIdx);
    void updatePrimeProgressFromVolChanged(QList<double> curVols, QList<double> prevVols);
    void updatePrimeProgressFromSyringeStateChanged(DS_McuDef::SyringeStates curStates, DS_McuDef::SyringeStates prevStates);
    void resetPrimeParameters(SyringeIdx syringeIdx, SyringeIdx fluidType);
    void updateLedWhileMissing();
    void updateFluidSourceWhileInjecting();
    void updateSudsLed(bool injecting);
    void updateFluidSource();
    void updateStateFromSudsInsertedChanged();
    void updateSudsPortExposedState();
    void resetPrimePressures();
    void evaluatePrimePressures();
    bool evaluatePrimePressuresSlopeScore(double &slopeScore);
    double getSteadyStatePressure(QList<int> pressures);
    double getLinearRegressionSlope(QList<int> x_values, QList<double> y_values);
    double getPatientLineVolume();
    void dumpListPrimedFluids();
    void updateSudsContent(const QList<double> &vols, const QList<double> &prevVols);
    bool isFluidFlowingThroughSuds();
    QString getSudsContent();
    bool isSudsNeedsReplace();
    bool shouldForceSetNeedsReplace();
    void updateSUDSInsertedAlerts(bool inserted, bool prevInserted);
    void activateEnsureNoPatientConnectedAlert();
    void deactivateEnsureNoPatientConnectedAlert();

private slots:
    void slotAppInitialised();
    void slotAppStarted();

};

#endif // DEVICE_SUDS_H
