#ifndef MCU_MSG_HANDLER_H
#define MCU_MSG_HANDLER_H

#include <QTimer>
#include "Common/Common.h"
#include "DataServices/Mcu/DS_McuDef.h"

#define MCU_TIMESTAMP_DIGESTSENT_FORMAT "MMdd-hh:mm:ss.zzz"

class McuMsgHandler : public QObject
{
    Q_OBJECT
public:
    explicit McuMsgHandler(QObject *parent = 0, EnvGlobal *env_ = NULL, EnvLocal *envLocal_ = NULL);
    ~McuMsgHandler();

    QString handleMsg(QString id, QString body, QString reply);

private:
    EnvGlobal *env;
    EnvLocal *envLocal;
    Log *mcuLog;
    Log *mcuInjectDigestLog;

    QStringList diagnosticEvents;
    QTimer tmrLastDiagnosticEventTimeOut;

    QString handleDigest(QString body, QString reply);
    QString handleHwDigest(QString reply);
    QString handleBMSDigest(QString reply);
    QString handleVersion(QString reply);
    QString handleInfo(QString reply);
    QString handleInjectDigest(QString reply);
    QString handleGetSyringeAirVol(QString body, QString reply);
    QString handleGetSyringeAirCheckData(QString body, QString reply);
    QString handleGetSyringeAirCheckCoeff(QString body, QString reply);
    QString handleGetPressureCalCoeff(QString body, QString reply);
    QString handleGetPlungerFriction(QString body, QString reply);


private slots:
    void slotAppInitialised();
    void slotActivateDiagnosticEventAlert();
};

#endif // MCU_MSG_HANDLER_H
