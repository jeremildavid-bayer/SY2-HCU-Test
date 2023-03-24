#ifndef MCU_SIM_SYRINGE_H
#define MCU_SIM_SYRINGE_H

#include <QObject>
#include <QTimer>
#include "Common/Common.h"

class McuSimSyringe : public QObject
{
    Q_OBJECT
public:
    enum State
    {
        SYR_STATE_IDLE = 0,
        SYR_STATE_ENGAGING,
        SYR_STATE_FILLING,
        SYR_STATE_INJECTING,
        SYR_STATE_ABORTING,
        SYR_STATE_UNKNOWN
    };

    struct Status
    {
        double volume;
        double flow;
        int pressure;
        bool engaged;
        bool airCheck;
        State state;
        QString curAction;
        QString actionDoneState;
    };

    explicit McuSimSyringe(QObject *parent = 0, EnvGlobal *env_ = NULL, EnvLocal *envLocal_ = NULL, SyringeIdx location = SYRINGE_IDX_SALINE);
    ~McuSimSyringe();

    QString startAction(QString action, QString errCodePrefix, double vol = 0, double flow = 0, bool airCheck = false);
    void pause();
    void resume();
    bool stopAction(QString actionDone);
    bool resetSyringe();
    Status getInfo();
    void setVol(double vol);
    bool adjustFlow(double flow);
    double getInjectedVol();

private:
    EnvGlobal *env;
    EnvLocal *envLocal;
    SyringeIdx location;
    Status curInfo;
    Status targetInfo;
    bool paused;

    double injectedVol;
    double targetInjectVol;

    double calculateTargetVolume(double vol, bool isFill);
    void calculateVolume(bool isFill);
    QString setSyringeActionDone(QString result, QString type);
    bool isVolumeAtTarget(bool isFill);

private slots:
    void slotUpdateStatus();

signals:
    void signalSyringeActionDone(QString state);

};

#endif // MCU_SIM_SYRINGE_H
