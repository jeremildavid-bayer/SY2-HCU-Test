#ifndef DS_MCU_SIM_QML_H
#define DS_MCU_SIM_QML_H

#include "Common/Common.h"

class QML_McuSim : public QObject
{
    Q_OBJECT
public:
    explicit QML_McuSim(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~QML_McuSim();

    Q_INVOKABLE void slotAdaptiveFlowStateChanged(QString state);
    Q_INVOKABLE void slotDoorStateChanged(QString state);
    Q_INVOKABLE void slotOutletDoorStateChanged(QString state);
    Q_INVOKABLE void slotMudsPresent(bool present);
    Q_INVOKABLE void slotMudsLatched(bool latched);
    Q_INVOKABLE void slotSudsInserted(bool inserted);
    Q_INVOKABLE void slotWasteBinStateChanged(QString state);
    Q_INVOKABLE void slotStopButtonPressed(bool pressed);
    Q_INVOKABLE void slotManualPrimeButtonPressed(bool pressed);
    Q_INVOKABLE void slotBatteryStateChanged(QString state);
    Q_INVOKABLE void slotAcConnectedStateChanged(QString state);
    Q_INVOKABLE void slotTemperature1Changed(double temperature);
    Q_INVOKABLE void slotTemperature2Changed(double temperature);
    Q_INVOKABLE void slotSyringesAir();
    Q_INVOKABLE void slotBubblePatientLine();
    Q_INVOKABLE void slotBubbleSaline();
    Q_INVOKABLE void slotBubbleContrast1();
    Q_INVOKABLE void slotBubbleContrast2();
    Q_INVOKABLE QString slotAlarmGetNext(QString alarmName);
    Q_INVOKABLE void slotAlarmActivate(QString alarmName);


private:
    EnvGlobal *env;
    EnvLocal *envLocal;
    QObject *qmlSrc;

};

#endif // DS_MCU_SIM_QML_H
