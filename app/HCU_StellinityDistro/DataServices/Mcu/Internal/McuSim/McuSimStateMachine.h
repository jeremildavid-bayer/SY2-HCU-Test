#ifndef MCU_SIM_STATE_MACHINE_H
#define MCU_SIM_STATE_MACHINE_H

#include <QTimer>
#include <QElapsedTimer>
#include "Common/Common.h"
#include "DataServices/Mcu/DS_McuDef.h"
#include "McuSimSyringeGroup.h"

class McuSimStateMachine : public QObject
{
    Q_OBJECT
public:
    enum State
    {
        MCU_STATE_START = 0,
        MCU_STATE_FILL,
        MCU_STATE_ARMED,
        MCU_STATE_INJECTING
    };

    explicit McuSimStateMachine(QObject *parent = 0, EnvGlobal *env_ = NULL, EnvLocal *envLocal_ = NULL, McuSimSyringeGroup *saline = NULL, McuSimSyringeGroup *contrast1 = NULL, McuSimSyringeGroup *contrast2 = NULL);
    ~McuSimStateMachine();

    void triggerSudsRemoved();
    void triggerSudsInserted();
    void triggerMudsUnlatched();
    void triggerAirDetected();
    void triggerSalineAirDetector();
    void triggerContrast1AirDetector();
    void triggerContrast2AirDetector();
    void triggerStopButtonClicked();
    void triggerBatteryCritical();

    QString resetMuds(QString errCodePrefix);
    QString adjustFlow(double delta, QString errCodePrefix);

    void setState(State state);
    State getState();
    QString setInjectState(QString injState, QString errCodePrefix);
    QString setSyringeAction(int syringeIdx, QString action, QString errCodePrefix, double param1 = 0, double param2 = 0, double param3 = 0, double param4 = 0);
    QString setActiveProtocol(QStringList params, QString errCodePrefix);

    //Inter injection
    int getCurPhase();
    QString injectStart(QString errCodePrefix);
    QString injectStop(QString errCodePrefix);
    QString injectHold(QString errCodePrefix);
    QString injectJump(int toIdx, QString errCodePrefix);
    DS_McuDef::InjectorState getInjectorState();
    DS_McuDef::InjectionCompleteStatus getInjectionCompleteStatus();
    QString getSyrActState(int idx);
    void setSyringeActState(int idx, QString state);

private:
    EnvGlobal *env;
    EnvLocal *envLocal;
    McuSimSyringeGroup *salineGroup;
    McuSimSyringeGroup *contrast1Group;
    McuSimSyringeGroup *contrast2Group;
    DS_McuDef::InjectorStatus injectProgress;
    DS_McuDef::InjectionProtocol curProtocol;
    QString syrActState[3];
    QTimer tmrPausePhase;
    State curState;
    State requestedState;
    int pausePhaseTimeRemaining;
    QTimer tmrHoldTimeout;
    QTimer tmrArmTimeout;

    void disarm(DS_McuDef::InjectionCompleteReason disarmReason);
    void abortInjection(DS_McuDef::InjectionCompleteReason completeReason = DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_NORMAL);
    void pauseInjection();
    void skipPhase();
    void resume();
    QString phaseJump(int protIdx);

    void initiateNextPhase();
    void handlePhaseTransition();
    void endInjection(DS_McuDef::InjectionCompleteReason completeReason);

    // Inject Digest related params
    QTimer tmrInjectDigest;
    QElapsedTimer elapsedPhaseTimer;

    void clearInjectDigest();

private slots:
    void slotTmrPausePhaseTimeout();
    void slotTmrHoldTimeout();
    void slotTmrArmTimeout();
    void slotSalineActionComplete(QString result);
    void slotContrast1ActionComplete(QString result);
    void slotContrast2ActionComplete(QString result);
    void slotSetInjectorStateDone();
    void slotResetInjectorState();
    void slotTmrInjectDigestTimeout();
};

#endif // MCU_SIM_STATE_MACHINE_H
