#ifndef DS_WORKFLOW_DATA_H
#define DS_WORKFLOW_DATA_H

#include "Common/Common.h"
#include "DS_WorkflowDef.h"
#include "DataServices/DataServicesMacros.h"

class DS_WorkflowData : public QObject
{
    Q_OBJECT

public:
    explicit DS_WorkflowData(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~DS_WorkflowData();

    static void registerDataTypesForThread()
    {
        qRegisterMetaType<DS_WorkflowDef::WorkflowState>("DS_WorkflowDef::WorkflowState");
        qRegisterMetaType<DS_WorkflowDef::WorkflowErrorStatus>("DS_WorkflowDef::WorkflowErrorStatus");
        qRegisterMetaType<DS_WorkflowDef::SodErrorState>("DS_WorkflowDef::SodErrorState");
        qRegisterMetaType<DS_WorkflowDef::FluidRemovalState>("DS_WorkflowDef::FluidRemovalState");
        qRegisterMetaType<DS_WorkflowDef::EndOfDayPurgeState>("DS_WorkflowDef::EndOfDayPurgeState");
        qRegisterMetaType<DS_WorkflowDef::AutoEmptyState>("DS_WorkflowDef::AutoEmptyState");
        qRegisterMetaType<DS_WorkflowDef::MudsEjectState>("DS_WorkflowDef::MudsEjectState");
        qRegisterMetaType<DS_WorkflowDef::MudsSodStatus>("DS_WorkflowDef::MudsSodStatus");
        qRegisterMetaType<DS_WorkflowDef::SyringeAirRecoveryState>("DS_WorkflowDef::SyringeAirRecoveryState");
        qRegisterMetaType<DS_WorkflowDef::SudsAirRecoveryState>("DS_WorkflowDef::SudsAirRecoveryState");
        qRegisterMetaType<DS_WorkflowDef::MudsSodState>("DS_WorkflowDef::MudsSodState");
        qRegisterMetaType<DS_WorkflowDef::ManualQualifiedDischargeStatus>("DS_WorkflowDef::ManualQualifiedDischargeStatus");
        qRegisterMetaType<DS_WorkflowDef::WorkflowBatteryStatus>("DS_WorkflowDef::WorkflowBatteryStatus");
        qRegisterMetaType<DS_WorkflowDef::ShippingModeStatus>("DS_WorkflowDef::ShippingModeStatus");
        qRegisterMetaType<DS_WorkflowDef::AutomaticQualifiedDischargeStatus>("DS_WorkflowDef::AutomaticQualifiedDischargeStatus");
        qRegisterMetaType<DS_WorkflowDef::PreloadProtocolState>("DS_WorkflowDef::PreloadProtocolState");
    }

    double getAutoPrimeVolume();
    void resetSyringesUsedInLastExam();

signals:
    // Define OUTGOING Signals (i.e. WHEN DATA CHANGED)
    CREATE_DATA_CHANGED_SIGNAL(bool, DataLocked)
    CREATE_DATA_CHANGED_SIGNAL(DS_WorkflowDef::WorkflowState, WorkflowState)
    CREATE_DATA_CHANGED_SIGNAL(DS_WorkflowDef::WorkflowErrorStatus, WorkflowErrorStatus)
    CREATE_DATA_CHANGED_SIGNAL(DS_WorkflowDef::SodErrorState, SodErrorState)
    CREATE_DATA_CHANGED_SIGNAL(DS_WorkflowDef::FluidRemovalState, FluidRemovalState)
    CREATE_DATA_CHANGED_SIGNAL(DS_WorkflowDef::EndOfDayPurgeState, EndOfDayPurgeState)
    CREATE_DATA_CHANGED_SIGNAL(DS_WorkflowDef::AutoEmptyState, AutoEmptyState)
    CREATE_DATA_CHANGED_SIGNAL(DS_WorkflowDef::MudsEjectState, MudsEjectState)
    CREATE_DATA_CHANGED_SIGNAL(DS_WorkflowDef::MudsSodStatus, MudsSodStatus)
    CREATE_DATA_CHANGED_SIGNAL(DS_WorkflowDef::SyringeAirRecoveryState, SyringeAirRecoveryState)
    CREATE_DATA_CHANGED_SIGNAL(DS_WorkflowDef::SudsAirRecoveryState, SudsAirRecoveryState)
    CREATE_DATA_CHANGED_SIGNAL(DS_WorkflowDef::MudsSodState, MudsSodState)
    CREATE_DATA_CHANGED_SIGNAL(DS_WorkflowDef::ManualQualifiedDischargeStatus, ManualQualifiedDischargeStatus)
    CREATE_DATA_CHANGED_SIGNAL(DS_WorkflowDef::WorkflowBatteryStatus, WorkflowBatteryStatus)
    CREATE_DATA_CHANGED_SIGNAL(DS_WorkflowDef::ShippingModeStatus, ShippingModeStatus)
    CREATE_DATA_CHANGED_SIGNAL(DS_WorkflowDef::AutomaticQualifiedDischargeStatus, AutomaticQualifiedDischargeStatus)
    CREATE_DATA_CHANGED_SIGNAL(DS_WorkflowDef::PreloadProtocolState, PreloadProtocolState)
    CREATE_DATA_CHANGED_SIGNAL(bool, UseExtendedAutoPrimeVolume)
    CREATE_DATA_CHANGED_SIGNAL(bool, ContrastAvailable1)
    CREATE_DATA_CHANGED_SIGNAL(bool, ContrastAvailable2)
    CREATE_DATA_CHANGED_SIGNAL(QList<bool>, SyringesUsedInLastExam)

private:
    // Creates GET/SET Properties
    CREATE_DATA_MEMBERS(bool, DataLocked)
    CREATE_DATA_MEMBERS(DS_WorkflowDef::WorkflowState, WorkflowState)
    CREATE_DATA_MEMBERS(DS_WorkflowDef::WorkflowErrorStatus, WorkflowErrorStatus)
    CREATE_DATA_MEMBERS(DS_WorkflowDef::SodErrorState, SodErrorState)
    CREATE_DATA_MEMBERS(DS_WorkflowDef::FluidRemovalState, FluidRemovalState)
    CREATE_DATA_MEMBERS(DS_WorkflowDef::EndOfDayPurgeState, EndOfDayPurgeState)
    CREATE_DATA_MEMBERS(DS_WorkflowDef::AutoEmptyState, AutoEmptyState)
    CREATE_DATA_MEMBERS(DS_WorkflowDef::MudsEjectState, MudsEjectState)
    CREATE_DATA_MEMBERS(DS_WorkflowDef::MudsSodStatus, MudsSodStatus)
    CREATE_DATA_MEMBERS(DS_WorkflowDef::SyringeAirRecoveryState, SyringeAirRecoveryState)
    CREATE_DATA_MEMBERS(DS_WorkflowDef::SudsAirRecoveryState, SudsAirRecoveryState)
    CREATE_DATA_MEMBERS(DS_WorkflowDef::MudsSodState, MudsSodState)
    CREATE_DATA_MEMBERS(DS_WorkflowDef::ManualQualifiedDischargeStatus, ManualQualifiedDischargeStatus)
    CREATE_DATA_MEMBERS(DS_WorkflowDef::WorkflowBatteryStatus, WorkflowBatteryStatus)
    CREATE_DATA_MEMBERS(DS_WorkflowDef::ShippingModeStatus, ShippingModeStatus)
    CREATE_DATA_MEMBERS(DS_WorkflowDef::AutomaticQualifiedDischargeStatus, AutomaticQualifiedDischargeStatus)
    CREATE_DATA_MEMBERS(DS_WorkflowDef::PreloadProtocolState, PreloadProtocolState)
    CREATE_DATA_MEMBERS(bool, UseExtendedAutoPrimeVolume)
    CREATE_DATA_MEMBERS(bool, ContrastAvailable1)
    CREATE_DATA_MEMBERS(bool, ContrastAvailable2)
    CREATE_DATA_MEMBERS(QList<bool>, SyringesUsedInLastExam)

    EnvGlobal *env;
    EnvLocal *envLocal;

private slots:
    void slotAppStarted();

};

#endif // DS_WORKFLOW_DATA_H
