#include "Apps/AppManager.h"
#include "DS_McuData.h"
#include "Common/ImrParser.h"
#include "DataServices/System/DS_SystemData.h"

DS_McuData::DS_McuData(QObject *parent, EnvGlobal *env_) :
    QObject(parent),
    env(env_)
{
    envLocal = new EnvLocal("DS_Mcu-Data", "MCU_DATA");

    connect(env->appManager, &AppManager::signalAppStarted, this, [=]() {
        slotAppStarted();
    });

    m_DataLocked = false;

    m_McuVersion = "";
    m_McuCommandVersion = "";
    m_StopcockVersion = "";
    m_McuSerialNumber = "";

    m_LinkState = DS_McuDef::LINK_STATE_UNKNOWN;
    m_AlarmCodes = 0;
    m_Pressure = 0;

    m_DoorState = DS_McuDef::DOOR_UNKNOWN;
    m_WasteBinState = DS_McuDef::WASTE_BIN_STATE_UNKNOWN;
    m_MudsInserted = false;
    m_MudsPresent = false;
    m_MudsLatched = false;
    m_SudsInserted = false;
    m_SudsBubbleDetected = false;
    m_PrimeBtnPressed = false;
    m_StopBtnPressed = false;
    m_DoorBtnPressed = false;
    m_OutletDoorState = DS_McuDef::OUTLET_DOOR_STATE_UNKNOWN;

    for (int devIdx = 0; devIdx < HEAT_MAINTAINER_IDX_MAX; devIdx++)
    {
        m_HeatMaintainerStatus.temperatureReadings.append(0.0);
    }
    m_HeatMaintainerStatus.state = DS_McuDef::HEAT_MAINTAINER_STATE_UNKNOWN;

    m_McuSerialNumber = "unknown";

    for (int i = 0; i < SYRINGE_IDX_MAX; i++)
    {
        m_MotorModuleSerialNumbers.append("unknown");
        m_StopcockPosAll.append(DS_McuDef::STOPCOCK_POS_UNKNOWN);
        m_SyringeStates.append(DS_McuDef::SYRINGE_STATE_UNKNOWN);
        m_PlungerStates.append(DS_McuDef::PLUNGER_STATE_UNKNOWN);
        m_SyringeVols.append(0);
        m_SyringeFlows.append(0);
        m_LastSyringeVols.append(0);
        m_BottleBubbleStates.append(DS_McuDef::BOTTLE_BUBBLE_DETECTOR_STATE_MISSING);
    }

    m_IsShuttingDown = false;
    m_HwDigestMonitorEnabled = false;

    for (int i = 0; i < SYRINGE_IDX_MAX; i++)
    {
        DS_McuDef::SyringeAirCheckCalData SyringeAirCheckCalDataBuf;
        m_SyringeAirCheckCalDigests.append(SyringeAirCheckCalDataBuf);
    }

    for (int i = 0; i < SYRINGE_IDX_MAX; i++)
    {
        DS_McuDef::SyringeAirCheckCoeff SyringeAirCheckCoeffBuf;
        m_SyringeAirCheckCoeffDigests.append(SyringeAirCheckCoeffBuf);
    }

    for (int i = 0; i < SYRINGE_IDX_MAX; i++)
    {
        DS_McuDef::SyringeAirCheckDigest SyringeAirCheckDigestBuf;
        m_SyringeAirCheckDigests.append(SyringeAirCheckDigestBuf);
    }

    for (int i = 0; i < SYRINGE_IDX_MAX; i++)
    {
        DS_McuDef::PressureCalCoeff PressureCalCoeffBuf;
        m_PressureCalCoeffDigests.append(PressureCalCoeffBuf);
    }

    DS_McuDef::BMSDigest bmsDigest;
    for (int i = 0; i < POWER_BATTERY_INDEX_MAX; i++)
    {
        m_BMSDigests.append(bmsDigest);
    }

    // Setup Hardware Rev Compatibility Groups
    // TODO: Log should contain m_HwRevCompatibilityGroups data values
    DS_McuDef::HwRevCompatibilityGroup hwRevCompatibilityGroup;

    hwRevCompatibilityGroup.init("BASE", "BASE1");
    /*
    // Example of declaring compatibility List
    DS_McuDef::HwRevCompatibility hwRevCompatibility;
    hwRevCompatibility.hwNumber = 2; // BASE2
    hwRevCompatibility.revOffset = "Ca";
    hwRevCompatibilityGroup.compatibilityList.append(hwRevCompatibility);
    */
    m_HwRevCompatibilityGroups.append(hwRevCompatibilityGroup);

    hwRevCompatibilityGroup.init("MAIN", "MAIN1");
    m_HwRevCompatibilityGroups.append(hwRevCompatibilityGroup);

    hwRevCompatibilityGroup.init("MM1", "MM1");
    m_HwRevCompatibilityGroups.append(hwRevCompatibilityGroup);

    hwRevCompatibilityGroup.init("MM2", "MM1");
    m_HwRevCompatibilityGroups.append(hwRevCompatibilityGroup);

    hwRevCompatibilityGroup.init("MM3", "MM1");
    m_HwRevCompatibilityGroups.append(hwRevCompatibilityGroup);

    hwRevCompatibilityGroup.init("SC", "SC1");
    m_HwRevCompatibilityGroups.append(hwRevCompatibilityGroup);

    // Set last data
    SET_LAST_DATA(DataLocked)
    SET_LAST_DATA(McuVersion)
    SET_LAST_DATA(McuCommandVersion)
    SET_LAST_DATA(StopcockVersion)
    SET_LAST_DATA(McuSerialNumber)
    SET_LAST_DATA(MotorModuleSerialNumbers)
    SET_LAST_DATA(LinkState)
    SET_LAST_DATA(AlarmCodes)
    SET_LAST_DATA(Pressure)
    SET_LAST_DATA(HeatMaintainerStatus)
    SET_LAST_DATA(OutletDoorState)
    SET_LAST_DATA(SyringeVols)
    SET_LAST_DATA(LastSyringeVols)
    SET_LAST_DATA(SyringeFlows)
    SET_LAST_DATA(PowerStatus)
    SET_LAST_DATA(DoorState)
    SET_LAST_DATA(WasteBinState)
    SET_LAST_DATA(MudsInserted)
    SET_LAST_DATA(MudsPresent)
    SET_LAST_DATA(MudsLatched)
    SET_LAST_DATA(BottleBubbleStates)
    SET_LAST_DATA(SudsInserted)
    SET_LAST_DATA(SudsBubbleDetected)
    SET_LAST_DATA(PrimeBtnPressed)
    SET_LAST_DATA(StopBtnPressed)
    SET_LAST_DATA(DoorBtnPressed)
    SET_LAST_DATA(StopcockPosAll)
    SET_LAST_DATA(PlungerStates)
    SET_LAST_DATA(SyringeStates)
    SET_LAST_DATA(InjectorStatus)
    SET_LAST_DATA(InjectionProtocol)
    SET_LAST_DATA(SimDigest)
    SET_LAST_DATA(SimInjectDigest)
    SET_LAST_DATA(IsShuttingDown)
    SET_LAST_DATA(HwDigestMonitorEnabled)
    SET_LAST_DATA(DigestMap)
    SET_LAST_DATA(PressureCalibrationStatus)
    SET_LAST_DATA(LedControlStatus)
    SET_LAST_DATA(InjectDigest)
    SET_LAST_DATA(SyringeAirCheckCalDigests)
    SET_LAST_DATA(SyringeAirCheckCoeffDigests)
    SET_LAST_DATA(SyringeAirCheckDigests)
    SET_LAST_DATA(PressureCalCoeffDigests)
    SET_LAST_DATA(BMSDigests)
    SET_LAST_DATA(HwRevCompatibilityGroups)
}

DS_McuData::~DS_McuData()
{
    delete envLocal;
}

void DS_McuData::slotAppStarted()
{
    //emitAllDataChanged();
}

void DS_McuData::emitAllDataChanged()
{
    //EMIT_DATA_CHANGED_SIGNAL(DataLocked)
    //EMIT_DATA_CHANGED_SIGNAL(LinkState)
    EMIT_DATA_CHANGED_SIGNAL(AlarmCodes)
    EMIT_DATA_CHANGED_SIGNAL(Pressure)
    EMIT_DATA_CHANGED_SIGNAL(HeatMaintainerStatus)
    EMIT_DATA_CHANGED_SIGNAL(OutletDoorState)
    EMIT_DATA_CHANGED_SIGNAL(SyringeVols)
    EMIT_DATA_CHANGED_SIGNAL(LastSyringeVols)
    EMIT_DATA_CHANGED_SIGNAL(SyringeFlows)
    EMIT_DATA_CHANGED_SIGNAL(PowerStatus)
    EMIT_DATA_CHANGED_SIGNAL(DoorState)
    EMIT_DATA_CHANGED_SIGNAL(WasteBinState)
    EMIT_DATA_CHANGED_SIGNAL(MudsInserted)
    EMIT_DATA_CHANGED_SIGNAL(MudsPresent)
    EMIT_DATA_CHANGED_SIGNAL(MudsLatched)
    EMIT_DATA_CHANGED_SIGNAL(BottleBubbleStates)
    EMIT_DATA_CHANGED_SIGNAL(SudsInserted)
    EMIT_DATA_CHANGED_SIGNAL(SudsBubbleDetected)
    EMIT_DATA_CHANGED_SIGNAL(PrimeBtnPressed)
    EMIT_DATA_CHANGED_SIGNAL(StopBtnPressed)
    EMIT_DATA_CHANGED_SIGNAL(DoorBtnPressed)
    EMIT_DATA_CHANGED_SIGNAL(StopcockPosAll)
    EMIT_DATA_CHANGED_SIGNAL(PlungerStates)
    EMIT_DATA_CHANGED_SIGNAL(SyringeStates)
    EMIT_DATA_CHANGED_SIGNAL(InjectorStatus)
    EMIT_DATA_CHANGED_SIGNAL(InjectionProtocol)
    EMIT_DATA_CHANGED_SIGNAL(SimDigest)
    EMIT_DATA_CHANGED_SIGNAL(SimInjectDigest)
    EMIT_DATA_CHANGED_SIGNAL(IsShuttingDown)
    EMIT_DATA_CHANGED_SIGNAL(PressureCalibrationStatus)
    EMIT_DATA_CHANGED_SIGNAL(LedControlStatus)
    //EMIT_DATA_CHANGED_SIGNAL(BMSDigests)
    EMIT_DATA_CHANGED_SIGNAL(HwRevCompatibilityGroups)
}

bool DS_McuData::getSyringesBusy(SyringeIdx ignoredSyringe) const
{
    for (int syringeIdx = 0; syringeIdx < m_SyringeStates.length(); syringeIdx++)
    {
        if (syringeIdx == ignoredSyringe)
        {
            continue;
        }

        if ( (m_SyringeStates[syringeIdx] == DS_McuDef::SYRINGE_STATE_PROCESSING) ||
             (m_SyringeStates[syringeIdx] == DS_McuDef::SYRINGE_STATE_STOP_PENDING) )
        {
            return true;
        }
    }
    return false;
}

bool DS_McuData::getPlungersEngaged() const
{
    return ( (m_PlungerStates[SYRINGE_IDX_SALINE] == DS_McuDef::PLUNGER_STATE_ENGAGED) &&
             (m_PlungerStates[SYRINGE_IDX_CONTRAST1] == DS_McuDef::PLUNGER_STATE_ENGAGED) &&
             (m_PlungerStates[SYRINGE_IDX_CONTRAST2] == DS_McuDef::PLUNGER_STATE_ENGAGED) );
}
