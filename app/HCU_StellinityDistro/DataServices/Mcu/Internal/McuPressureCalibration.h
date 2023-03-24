#ifndef MCU_PRESSURE_CALIBRATION_FLUIDLESS_H
#define MCU_PRESSURE_CALIBRATION_FLUIDLESS_H

#include <QObject>
#include <QTimer>
#include <QGenericMatrix>
#include "Common/Common.h"
#include "Common/Matrix.h"
#include "Common/ActionBaseExt.h"

class McuPressureCalibration : public ActionBaseExt
{
    Q_OBJECT
public:

    explicit McuPressureCalibration(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~McuPressureCalibration();

    bool calibrationStart(SyringeIdx idx);
    void calibrationStop();
    QString getCoefficients();

private:
    SyringeIdx syringeIdx;
    QTimer tmrCaptureData;
    QString guidMudsLatchedMonitor;

    // TODO: Move these to MCUData
    Matrix squareMatrix;
    Matrix columnMatrix;

    void processState();
    int getState();
    QString getStateStr(int state);
    void setStateSynch(int newState);

    void calibrationDataCaptureStartAsync();
    void calibrationDataAdd(double flow, double pid, double meterPressure);
    void calibrationDataReset();

private slots:
    void slotCaptureData();
    void slotAppInitialised();
};

#endif // MCU_PRESSURE_CALIBRATION_FLUIDLESS_H
