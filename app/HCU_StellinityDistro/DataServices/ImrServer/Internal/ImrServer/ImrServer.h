#ifndef IMRSERVER_H
#define IMRSERVER_H

#include "Common/Common.h"
#include "Common/ActionBase.h"
#include "DigestProvider.h"
#include "../ImrServerBase.h"
#include "DataServices/System/DS_SystemDef.h"

class ImrServer : public ImrServerBase
{
    Q_OBJECT

public:
    ImrServer(QObject *parent = NULL, QString baseUrl = "", EnvGlobal *env = NULL, EnvLocal *envLocal = NULL);
    ~ImrServer();

    void ImrService(Params &params) override;

private:
    DigestProvider *digestProvider;
    HttpResponse *response;
    QMutex mutexActStatusMap;
    QMap<QString, DataServiceActionStatus> actStatusMap;


    // Request Handler Functions
    void rhProfile(Params &params);
    void rhDataGroup(Params &params);
    void rhDigests(Params &params);
    void rhDigestsDelta(Params &params);
    void rhAlert(Params &params);
    void rhConfigs(Params &params);
    void rhFluidOptions(Params &params);
    void rhActionDigest(Params &params);
    void rhCalibrations(Params &params);
    void rhPermissions(Params &params);
    void rhCommandInvalidate(Params &params);
    void rhCommandProgram(Params &params);
    void rhCommandTruncate(Params &params);
    void rhCommandStart(Params &params);
    void rhCommandStop(Params &params);
    void rhCommandHold(Params &params);
    void rhCommandJump(Params &params);
    void rhCommandAdjust(Params &params);
    void rhCommandArm(Params &params);
    void rhCommandDisarm(Params &params);
    void rhCommandKvoStart(Params &params);
    void rhCommandKvoStop(Params &params);
    void rhCommandPatencyStart(Params &params);
    void rhCommandPatencyStop(Params &params);
    void rhCommandAircheck(Params &params);
    void rhCommandAlertsRecorded(Params &params);
    void rhCommandAlertsRaise(Params &params);
    void rhCommandAlertsUpdate(Params &params);
    void rhCommandScannerInterlocks(Params &params);
    void rhCommandStartExam(Params &params);
    void rhCommandEndExam(Params &params);


    QString getActStatusMapDump();

    template <typename LambdaFn>
    void performAction(Params &params, LambdaFn actionStartCb);

signals:
    void signalActionCompleted();
};

#endif // IMRSERVER_H
