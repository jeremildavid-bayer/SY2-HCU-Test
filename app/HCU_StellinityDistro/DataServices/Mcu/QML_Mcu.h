#ifndef QML_MCU_H
#define QML_MCU_H

#include "Common/Common.h"
#include "DataServices/Mcu/DS_McuDef.h"

class QML_Mcu : public QObject
{
    Q_OBJECT
public:
    explicit QML_Mcu(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~QML_Mcu();

    Q_INVOKABLE void slotResetMuds();
    Q_INVOKABLE void slotResetMcu();
    Q_INVOKABLE void slotResetStopcock();
    Q_INVOKABLE void slotHwDigestMonitorActive(bool active);
    Q_INVOKABLE void slotCalibrateAirDetector(int idx);
    Q_INVOKABLE void slotCalibrateMotor(int idx);
    Q_INVOKABLE void slotCalibrateSuds();
    Q_INVOKABLE void slotCalibratePlunger(int idx);
    Q_INVOKABLE void slotCalibrateSetPressureMeter();
    Q_INVOKABLE void slotCalibratePressureStart(int idx);
    Q_INVOKABLE void slotCalibratePressureStop();
    Q_INVOKABLE void slotCalibrateSetPressure(int idx);
    Q_INVOKABLE void slotGetCalibrationStatus();
    Q_INVOKABLE void slotFindPlunger(int index);
    Q_INVOKABLE void slotPullPlungers();
    Q_INVOKABLE void slotPistonEngage(int index);
    Q_INVOKABLE void slotPistonDisengage(int index);
    Q_INVOKABLE void slotPistonStop(int);
    Q_INVOKABLE void slotPistonUp(int index, double flow);
    Q_INVOKABLE void slotPistonDown(int index, double flow);
    Q_INVOKABLE void slotLedControl(QVariant params);
    Q_INVOKABLE void slotDoorLock(bool lock);
    Q_INVOKABLE void slotMovePistonToCleaningPos(int index);
    Q_INVOKABLE void slotBmsDigest();
    Q_INVOKABLE void slotBmsCommand(int index, QString data);
    Q_INVOKABLE void slotSetBaseFanTemperature(int temperature);

private:
    EnvGlobal *env;
    EnvLocal *envLocal;
    QObject *qmlSrc;

    void calibrateHw(QString actGuid, QString name);
};

#endif // QML_MCU_H
