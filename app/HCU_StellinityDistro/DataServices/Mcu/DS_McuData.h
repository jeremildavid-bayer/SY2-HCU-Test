#ifndef DS_MCU_DATA_H
#define DS_MCU_DATA_H

#include "DS_McuDef.h"
#include "DataServices/DataServicesMacros.h"

class DS_McuData : public QObject
{
    Q_OBJECT

public:
    explicit DS_McuData(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~DS_McuData();

    static void registerDataTypesForThread()
    {
        qRegisterMetaType<DS_McuDef::LinkState>("DS_McuDef::LinkState");
        qRegisterMetaType<DS_McuDef::PowerStatus>("DS_McuDef::PowerStatus");
        qRegisterMetaType<DS_McuDef::DoorState>("DS_McuDef::DoorState");
        qRegisterMetaType<DS_McuDef::WasteBinState>("DS_McuDef::WasteBinState");
        qRegisterMetaType<DS_McuDef::StopcockPosAll>("DS_McuDef::StopcockPosAll");
        qRegisterMetaType<DS_McuDef::PlungerStates>("DS_McuDef::PlungerStates");
        qRegisterMetaType<DS_McuDef::SyringeStates>("DS_McuDef::SyringeStates");
        qRegisterMetaType<DS_McuDef::InjectorStatus>("DS_McuDef::InjectorStatus");
        qRegisterMetaType<DS_McuDef::InjectionProtocol>("DS_McuDef::InjectionProtocol");
        qRegisterMetaType<DS_McuDef::SimDigest>("DS_McuDef::SimDigest");
        qRegisterMetaType<DS_McuDef::HeatMaintainerStatus>("DS_McuDef::HeatMaintainerStatus");
        qRegisterMetaType<DS_McuDef::OutletDoorState>("DS_McuDef::OutletDoorState");
        qRegisterMetaType<DS_McuDef::BottleBubbleDetectorStates>("DS_McuDef::BottleBubbleDetectorStates");
        qRegisterMetaType<DS_McuDef::PressureCalibrationStatus>("DS_McuDef::PressureCalibrationStatus");
        qRegisterMetaType<DS_McuDef::LedControlStatus>("DS_McuDef::LedControlStatus");
        qRegisterMetaType<DS_McuDef::PhaseInjectDigest>("DS_McuDef::PhaseInjectDigest");
        qRegisterMetaType<DS_McuDef::InjectDigest>("DS_McuDef::InjectDigest");
        qRegisterMetaType<DS_McuDef::SyringeAirCheckCalDigests>("DS_McuDef::SyringeAirCheckCalDigests");
        qRegisterMetaType<DS_McuDef::SyringeAirCheckCoeffDigests>("DS_McuDef::SyringeAirCheckCoeffDigests");
        qRegisterMetaType<DS_McuDef::SyringeAirCheckDigests>("DS_McuDef::SyringeAirCheckDigests");
        qRegisterMetaType<DS_McuDef::PressureCalCoeffDigests>("DS_McuDef::PressureCalCoeffDigests");
        qRegisterMetaType<DS_McuDef::BMSDigests>("DS_McuDef::BMSDigests");
        qRegisterMetaType<DS_McuDef::HwRevCompatibilityGroups>("DS_McuDef::HwRevCompatibilityGroups");
    }

    void emitAllDataChanged();
    bool getSyringesBusy(SyringeIdx ignoredSyringe = SYRINGE_IDX_NONE) const;
    bool getPlungersEngaged() const;

signals:
    // Define OUTGOING Signals (i.e. WHEN DATA CHANGED)
    CREATE_DATA_CHANGED_SIGNAL(bool, DataLocked)
    CREATE_DATA_CHANGED_SIGNAL(QString, McuVersion)
    CREATE_DATA_CHANGED_SIGNAL(QString, McuCommandVersion)
    CREATE_DATA_CHANGED_SIGNAL(QString, StopcockVersion)
    CREATE_DATA_CHANGED_SIGNAL(QString, McuSerialNumber)
    CREATE_DATA_CHANGED_SIGNAL(QStringList, MotorModuleSerialNumbers)
    CREATE_DATA_CHANGED_SIGNAL(DS_McuDef::LinkState, LinkState)
    CREATE_DATA_CHANGED_SIGNAL(QByteArray, AlarmCodes)
    CREATE_DATA_CHANGED_SIGNAL(int, Pressure)
    CREATE_DATA_CHANGED_SIGNAL(DS_McuDef::HeatMaintainerStatus, HeatMaintainerStatus)
    CREATE_DATA_CHANGED_SIGNAL(DS_McuDef::OutletDoorState, OutletDoorState)
    CREATE_DATA_CHANGED_SIGNAL(QList<double>, SyringeVols)
    CREATE_DATA_CHANGED_SIGNAL(QList<double>, LastSyringeVols)
    CREATE_DATA_CHANGED_SIGNAL(QList<double>, SyringeFlows)
    CREATE_DATA_CHANGED_SIGNAL(DS_McuDef::PowerStatus, PowerStatus)
    CREATE_DATA_CHANGED_SIGNAL(DS_McuDef::DoorState, DoorState)
    CREATE_DATA_CHANGED_SIGNAL(DS_McuDef::WasteBinState, WasteBinState)
    CREATE_DATA_CHANGED_SIGNAL(bool, MudsInserted)
    CREATE_DATA_CHANGED_SIGNAL(bool, MudsPresent)
    CREATE_DATA_CHANGED_SIGNAL(bool, MudsLatched)
    CREATE_DATA_CHANGED_SIGNAL(DS_McuDef::BottleBubbleDetectorStates, BottleBubbleStates)
    CREATE_DATA_CHANGED_SIGNAL(bool, SudsInserted)
    CREATE_DATA_CHANGED_SIGNAL(bool, SudsBubbleDetected)
    CREATE_DATA_CHANGED_SIGNAL(bool, PrimeBtnPressed)
    CREATE_DATA_CHANGED_SIGNAL(bool, StopBtnPressed)
    CREATE_DATA_CHANGED_SIGNAL(bool, DoorBtnPressed)
    CREATE_DATA_CHANGED_SIGNAL(DS_McuDef::StopcockPosAll, StopcockPosAll)
    CREATE_DATA_CHANGED_SIGNAL(DS_McuDef::PlungerStates, PlungerStates)
    CREATE_DATA_CHANGED_SIGNAL(DS_McuDef::SyringeStates, SyringeStates)
    CREATE_DATA_CHANGED_SIGNAL(DS_McuDef::InjectorStatus, InjectorStatus)
    CREATE_DATA_CHANGED_SIGNAL(DS_McuDef::InjectionProtocol, InjectionProtocol)
    CREATE_DATA_CHANGED_SIGNAL(DS_McuDef::SimDigest, SimDigest)
    CREATE_DATA_CHANGED_SIGNAL(DS_McuDef::InjectDigest, SimInjectDigest)
    CREATE_DATA_CHANGED_SIGNAL(bool, IsShuttingDown)
    CREATE_DATA_CHANGED_SIGNAL(bool, HwDigestMonitorEnabled)
    CREATE_DATA_CHANGED_SIGNAL(QVariantMap, DigestMap)
    CREATE_DATA_CHANGED_SIGNAL(DS_McuDef::PressureCalibrationStatus, PressureCalibrationStatus)
    CREATE_DATA_CHANGED_SIGNAL(DS_McuDef::LedControlStatus, LedControlStatus)
    CREATE_DATA_CHANGED_SIGNAL(DS_McuDef::InjectDigest, InjectDigest)
    CREATE_DATA_CHANGED_SIGNAL(DS_McuDef::SyringeAirCheckCalDigests, SyringeAirCheckCalDigests)
	CREATE_DATA_CHANGED_SIGNAL(DS_McuDef::SyringeAirCheckCoeffDigests, SyringeAirCheckCoeffDigests)
    CREATE_DATA_CHANGED_SIGNAL(DS_McuDef::SyringeAirCheckDigests, SyringeAirCheckDigests)
    CREATE_DATA_CHANGED_SIGNAL(DS_McuDef::PressureCalCoeffDigests, PressureCalCoeffDigests)
    CREATE_DATA_CHANGED_SIGNAL(DS_McuDef::BMSDigests, BMSDigests)
    CREATE_DATA_CHANGED_SIGNAL(DS_McuDef::HwRevCompatibilityGroups, HwRevCompatibilityGroups)

private:
    // Creates GET/SET Properties
    CREATE_DATA_MEMBERS(bool, DataLocked)
    CREATE_DATA_MEMBERS(QString, McuVersion)
    CREATE_DATA_MEMBERS(QString, McuCommandVersion)
    CREATE_DATA_MEMBERS(QString, StopcockVersion)
    CREATE_DATA_MEMBERS(QString, McuSerialNumber)
    CREATE_DATA_MEMBERS(QStringList, MotorModuleSerialNumbers)
    CREATE_DATA_MEMBERS(DS_McuDef::LinkState, LinkState)
    CREATE_DATA_MEMBERS(QByteArray, AlarmCodes)
    CREATE_DATA_MEMBERS(int, Pressure)
    CREATE_DATA_MEMBERS(DS_McuDef::HeatMaintainerStatus, HeatMaintainerStatus)
    CREATE_DATA_MEMBERS(DS_McuDef::OutletDoorState, OutletDoorState)
    CREATE_DATA_MEMBERS(QList<double>, SyringeVols)
    CREATE_DATA_MEMBERS(QList<double>, LastSyringeVols)
    CREATE_DATA_MEMBERS(QList<double>, SyringeFlows)
    CREATE_DATA_MEMBERS(DS_McuDef::PowerStatus, PowerStatus)
    CREATE_DATA_MEMBERS(DS_McuDef::DoorState, DoorState)
    CREATE_DATA_MEMBERS(DS_McuDef::WasteBinState, WasteBinState)
    CREATE_DATA_MEMBERS(bool, MudsInserted) // (mudsPresent && mudsLatched)
    CREATE_DATA_MEMBERS(bool, MudsPresent)
    CREATE_DATA_MEMBERS(bool, MudsLatched)
    CREATE_DATA_MEMBERS(DS_McuDef::BottleBubbleDetectorStates, BottleBubbleStates)
    CREATE_DATA_MEMBERS(bool, SudsInserted)
    CREATE_DATA_MEMBERS(bool, SudsBubbleDetected)
    CREATE_DATA_MEMBERS(bool, PrimeBtnPressed)
    CREATE_DATA_MEMBERS(bool, StopBtnPressed)
    CREATE_DATA_MEMBERS(bool, DoorBtnPressed)
    CREATE_DATA_MEMBERS(DS_McuDef::StopcockPosAll, StopcockPosAll)
    CREATE_DATA_MEMBERS(DS_McuDef::PlungerStates, PlungerStates)
    CREATE_DATA_MEMBERS(DS_McuDef::SyringeStates, SyringeStates)
    CREATE_DATA_MEMBERS(DS_McuDef::InjectorStatus, InjectorStatus)
    CREATE_DATA_MEMBERS(DS_McuDef::InjectionProtocol, InjectionProtocol)
    CREATE_DATA_MEMBERS(DS_McuDef::SimDigest, SimDigest)
    CREATE_DATA_MEMBERS(DS_McuDef::InjectDigest, SimInjectDigest)
    CREATE_DATA_MEMBERS(bool, IsShuttingDown)
    CREATE_DATA_MEMBERS(bool, HwDigestMonitorEnabled)
    CREATE_DATA_MEMBERS(QVariantMap, DigestMap)
    CREATE_DATA_MEMBERS(DS_McuDef::PressureCalibrationStatus, PressureCalibrationStatus)
    CREATE_DATA_MEMBERS(DS_McuDef::LedControlStatus, LedControlStatus)
    CREATE_DATA_MEMBERS(DS_McuDef::InjectDigest, InjectDigest)
    CREATE_DATA_MEMBERS(DS_McuDef::SyringeAirCheckCalDigests, SyringeAirCheckCalDigests)
    CREATE_DATA_MEMBERS(DS_McuDef::SyringeAirCheckCoeffDigests, SyringeAirCheckCoeffDigests)
    CREATE_DATA_MEMBERS(DS_McuDef::SyringeAirCheckDigests, SyringeAirCheckDigests)
    CREATE_DATA_MEMBERS(DS_McuDef::PressureCalCoeffDigests, PressureCalCoeffDigests)
    CREATE_DATA_MEMBERS(DS_McuDef::BMSDigests, BMSDigests)
    CREATE_DATA_MEMBERS(DS_McuDef::HwRevCompatibilityGroups, HwRevCompatibilityGroups)

    EnvGlobal *env;
    EnvLocal *envLocal;

private slots:
    void slotAppStarted();
};

#endif // DS_MCU_DATA_H
