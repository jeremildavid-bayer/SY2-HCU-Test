#ifndef MCU_SIM_H
#define MCU_SIM_H

#include <QObject>
#include "Common/Common.h"
#include "DataServices/Mcu/DS_McuDef.h"
#include "McuSimStateMachine.h"
#include "McuSimSyringeGroup.h"

class McuSim : public QObject
{
    Q_OBJECT

public:
    explicit McuSim(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~McuSim();

    void activate(bool activateHw);
    void sendMsg(QString msg);

private:
    EnvGlobal *env;
    EnvLocal *envLocal;
    bool isActive;
    QTimer tmrReconnectMcu;

    McuSimStateMachine *mcuStateMachine;
    McuSimSyringeGroup salineControlGroup;
    McuSimSyringeGroup contrast1ControlGroup;
    McuSimSyringeGroup contrast2ControlGroup;

    void handleActionRequest(QString action, QStringList params, QString msgIn);
    void sendResponse(QString originalMsg, QString reply);
    void updateDigest();
    bool isSyringeEngaged(int idx);
    bool isValidSyringeIdx(int idx);
    QString checkMcuArmCondition(QString errCodePrefix);
    QString checkMcuInjectStartCondition(QString errCodePrefix);
    void updateHwStates(DS_McuDef::SimDigest newHwData, DS_McuDef::SimDigest hwData);

signals:
    void signalConnected();
    void signalDisconnected();
    void signalRxTimeout();
    void signalRxMsg(QString msg);
    void signalRequestReceived(QString req);

private slots:
    void slotAppInitialised();
    void slotAppStarted();
    void slotRequestReceived(QString req);
    void slotConnectStart();
};

#endif // MCU_SIM_H
