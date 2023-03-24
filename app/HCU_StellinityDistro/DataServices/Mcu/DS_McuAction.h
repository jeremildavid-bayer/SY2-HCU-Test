#ifndef DS_MCU_ACTION_H
#define DS_MCU_ACTION_H

#include "DS_McuDef.h"
#include "Common/Common.h"
#include "Common/Util.h"
#include "Common/ActionBase.h"
#include "Internal/McuLink.h"
#include "Internal/McuMonitor.h"
#include "Internal/McuAlarmMonitor.h"
#include "Internal/McuPressureCalibration.h"

class DS_McuAction : public ActionBase
{
    Q_OBJECT

public:
    explicit DS_McuAction(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~DS_McuAction();

    DataServiceActionStatus actRestoreLastMcuDigest(QString actGuid = "");
    DataServiceActionStatus actVersion(QString actGuid = "");
    DataServiceActionStatus actInfo(QString actGuid = "");
    DataServiceActionStatus actSetBaseType(QString baseBoardType = "", QString actGuid = "");
    DataServiceActionStatus actSetHwTypes(QString actGuid = "");
    DataServiceActionStatus actDigest(QString actGuid = "");
    DataServiceActionStatus actArm(const DS_McuDef::InjectionProtocol &injProt, QString actGuid = "");
    DataServiceActionStatus actDisarm(QString actGuid = "");
    DataServiceActionStatus actInjectStart(QString actGuid = "");
    DataServiceActionStatus actInjectStop(QString actGuid = "");
    DataServiceActionStatus actInjectHold(QString actGuid = "");
    DataServiceActionStatus actInjectJump(int jumpFromIdx, int jumpToIdx, QString actGuid = "");
    DataServiceActionStatus actPower(DS_McuDef::PowerControlType ctrl, QString actGuid = "");
    DataServiceActionStatus actAdjflow(double delta, QString actGuid = "");
    DataServiceActionStatus actStopcocks(DS_McuDef::ActStopcockParams params, QString actGuid = "");
    DataServiceActionStatus actStopcock(SyringeIdx location, DS_McuDef::StopcockPos pos, QString actGuid = "");
    DataServiceActionStatus actPiston(const DS_McuDef::ActPistonParams &params, QString actGuid = "");
    DataServiceActionStatus actPistonAll(const DS_McuDef::ActPistonParams &params, QString actGuid = "");
    DataServiceActionStatus actLeds(const DS_McuDef::ActLedParamsList &paramsList, QString actGuid = "");
    DataServiceActionStatus actEngage(SyringeIdx syringeIdx, QString actGuid = "");
    DataServiceActionStatus actEngageAll(QString actGuid = "");
    DataServiceActionStatus actDisengage(SyringeIdx syringeIdx, QString actGuid = "");
    DataServiceActionStatus actDisengageAll(QString actGuid = "");
    DataServiceActionStatus actPurge(SyringeIdx syringeIdx, QString actGuid = "");
    DataServiceActionStatus actPurgeAll(QString actGuid = "");
    DataServiceActionStatus actPullPlunger(SyringeIdx syringeIdx, QString actGuid = "");
    DataServiceActionStatus actPullPlungers(QString actGuid = "");
    DataServiceActionStatus actFindPlunger(SyringeIdx syringeIdx, QString actGuid = "");
    DataServiceActionStatus actFindPlungerAll(QString actGuid = "");
    DataServiceActionStatus actFill(const DS_McuDef::ActFillParams &params, QString actGuid = "");
    DataServiceActionStatus actPrime(const DS_McuDef::ActPrimeParams &params, QString actGuid = "");
    DataServiceActionStatus actStopAll(QString actGuid = "");
    DataServiceActionStatus actCalSyringeAirCheck(SyringeIdx syringeIdx, QString actGuid = "");
    DataServiceActionStatus actGetSyringeAirCheckData(SyringeIdx syringeIdx, QString actGuid = "");
    DataServiceActionStatus actGetSyringeAirCheckCoeff(SyringeIdx syringeIdx, QString actGuid = "");
    DataServiceActionStatus actSyringeAirCheck(SyringeIdx syringeIdx, QString actGuid = "");
    DataServiceActionStatus actGetSyringeAirVol(SyringeIdx syringeIdx, QString actGuid = "");
    DataServiceActionStatus actSyringeRecoveryVacuum(SyringeIdx syringeIdx, QString actGuid = "");
    DataServiceActionStatus actCalSlack(SyringeIdx syringeIdx, QString actGuid = "");
    DataServiceActionStatus actSetSlack(SyringeIdx syringeIdx, double volume, QString actGuid = "");
    DataServiceActionStatus actCalInletAirDetector(SyringeIdx syringeIdx, QString actGuid = "");
    DataServiceActionStatus actCalMotor(SyringeIdx syringeIdx, QString actGuid = "");
    DataServiceActionStatus actCalSudsSensor(QString actGuid = "");
    DataServiceActionStatus actCalPlunger(SyringeIdx syringeIdx, QString actGuid = "");
    DataServiceActionStatus actCalSetPressureMeter(QString actGuid = "");
    DataServiceActionStatus actCalPressureStart(SyringeIdx syringeIdx, QString actGuid = "");
    DataServiceActionStatus actCalPressureStop(QString actGuid = "");
    DataServiceActionStatus actCalDefaultPressure(SyringeIdx syringeIdx, QString actGuid = "");
    DataServiceActionStatus actCalGetPressureData(SyringeIdx syringeIdx, bool isProductionType = false, QString actGuid = "");
    DataServiceActionStatus actCalSetPressureData(SyringeIdx syringeIdx, QString actGuid = "");
    DataServiceActionStatus actCalGetPressureCoeff(SyringeIdx syringeIdx, QString actGuid = "");
    DataServiceActionStatus actGetPlungerFrictionData(QString actGuid = "");
    DataServiceActionStatus actCalDigest(QString actGuid = "");
    DataServiceActionStatus actAlarmClear(QString actGuid = "");
    DataServiceActionStatus actResetMuds(QString actGuid = "");
    DataServiceActionStatus actHeatMaintainerOn(double temp, QString actGuid = "");
    DataServiceActionStatus actHwDigest(QString actGuid = "");
    DataServiceActionStatus actInjectDigest(QString actGuid = "");
    DataServiceActionStatus actBmsDigest(QString actGuid = "");
    DataServiceActionStatus actBmsCommand(int index, QString data, QString actGuid = "");
    DataServiceActionStatus actBmsEnableFET(int index, bool enabled = true, QString actGuid = "");
    DataServiceActionStatus actBmsBootCtl(int batteryIdx, bool enabled, QString actGuid = "");
    DataServiceActionStatus actChgFetCtl(int batteryIdx, bool enabled, QString actGuid = "");
    DataServiceActionStatus actVOutDchgCtl(bool enabled, QString actGuid = "");
    DataServiceActionStatus actInitPowerSources(QString actGuid = "");
    DataServiceActionStatus actChargeBattery(int batteryIdx, bool enabled, QString actGuid = "");
    DataServiceActionStatus actBatteryToUnit(int batteryIdx, bool enabled, QString actGuid = "");
    DataServiceActionStatus actMainToUnit(bool enabled, QString actGuid = "");
    DataServiceActionStatus actLockDoor(bool lock, QString actGuid = "");
    DataServiceActionStatus actSetMotorModuleSerialNumber(SyringeIdx syringeIdx, QString serialNumber, QString actGuid = "");
    DataServiceActionStatus actLinkConnect(QString actGuid = "");
    DataServiceActionStatus actLinkDisconnect(QString actGuid = "");
    DataServiceActionStatus actResetMcuHw(QString actGuid = "");
    DataServiceActionStatus actResetScbHw(QString actGuid = "");
    DataServiceActionStatus actSetBaseFanTemperature(int temperature, QString actGuid = "");

private slots:
    void slotAppInitialised();

private:
    McuMonitor *monitor;
    McuAlarmMonitor *alarmMonitor;
    QString replyMsg;
    McuLink *mcuLink;
    QMap<QString, QString> mapActionReply;
    McuPressureCalibration *mcuPressureCalibrationHandler;
    bool adjustFlowRateBusy;
    QMap<QString, DataServiceActionStatus> actStatusMap;
    QMutex mutexActStatusMap;

    DataServiceActionStatus startAction(QString msgId, QString body, QString actGuid = "", int timeoutMs = MCU_ACTION_TIMEOUT_MS);
    void handleSyringeActionCompleted(SyringeIdx syringeIdx, QString actionId);
    void handleSyringeActionFailed(SyringeIdx syringeIdx, QString actionId, DS_McuDef::SyringeState syringeState, DataServiceActionStatus &actionStatus);
    void waitSyringeActionComplete(SyringeIdx syringeIdx, QString actionId, const DataServiceActionStatus &actionStatus);
    void waitAllSyringeActionComplete(QString actionId, const DataServiceActionStatus &actionStatus);
};

#endif // DS_MCU_ACTION_H
