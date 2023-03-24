#ifndef MCU_SIM_STOPCOCK_H
#define MCU_SIM_STOPCOCK_H

#include <QObject>
#include <QTimer>
#include "Common/Common.h"

class McuSimStopcock : public QObject
{
    Q_OBJECT
public:
    explicit McuSimStopcock(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~McuSimStopcock();

    QString setPosition(QString position, QString errCodePrefix = "T_STOPCOCKFAILED_");
    QString getPosition();
    bool setEngaged(bool engage);
    bool isEngaged();

private:
    EnvGlobal *env;
    QTimer tmrStopcock;
    QString requestedPosition;
    QString curPosition;
    bool engaged;

signals:
    void signalPositionChanged(QString position);

private slots:
    void slotTmrStopcockFinished();
};

#endif // MCU_SIM_STOPCOCK_H
