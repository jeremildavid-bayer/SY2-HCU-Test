#ifndef DS_DEVICE_DATA_H
#define DS_DEVICE_DATA_H

#include "Common/Common.h"
#include "DS_DeviceDef.h"
#include "DataServices/DataServicesMacros.h"

class DS_DeviceData : public QObject
{
    Q_OBJECT

public:
    explicit DS_DeviceData(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~DS_DeviceData();

    static void registerDataTypesForThread()
    {
        qRegisterMetaType<DS_DeviceDef::FluidSource>("DS_DeviceDef::FluidSource");
        qRegisterMetaType<DS_DeviceDef::FluidSources>("DS_DeviceDef::FluidSources");
        qRegisterMetaType<DS_DeviceDef::FluidOptions>("DS_DeviceDef::FluidOptions");
        qRegisterMetaType<DS_DeviceDef::FluidPackages>("DS_DeviceDef::FluidPackages");
        qRegisterMetaType<DS_DeviceDef::FluidInfo>("DS_DeviceDef::FluidInfo");
        qRegisterMetaType<DS_DeviceDef::BarcodeInfo>("DS_DeviceDef::BarcodeInfo");
    }

    bool isSameContrastsLoaded() const;
    bool isRefillReady(SyringeIdx syringeIdx) const;
    void setFluidSourceSyringe(SyringeIdx syringeIdx, const DS_DeviceDef::FluidSource &fluidSource);
    void setFluidSourceBottle(SyringeIdx syringeIdx, const DS_DeviceDef::FluidSource &fluidSource);
    bool getFluidSourceSyringesBusy(SyringeIdx ignoredSyringe = SYRINGE_IDX_NONE) const;
    bool getSyringeSodStartReady(SyringeIdx syringeIdx) const;
    double getSyringeCalSlackVolRequired(SyringeIdx syringeIdx) const;
    double getSyringePrimeVolRequired(SyringeIdx syringeIdx) const;
    bool isFluidSourceWasteContainerReady() const;
    bool getIsBottleMandatoryFieldsFilled(SyringeIdx syringeIdx) const;

signals:
    // Define OUTGOING Signals (i.e. WHEN DATA CHANGED)
    CREATE_DATA_CHANGED_SIGNAL(bool, DataLocked)
    CREATE_DATA_CHANGED_SIGNAL(bool, IsFirstAutoPrimeCompleted)
    CREATE_DATA_CHANGED_SIGNAL(bool, IsSudsUsed)
    CREATE_DATA_CHANGED_SIGNAL(DS_DeviceDef::FluidSources, FluidSourceBottles)
    CREATE_DATA_CHANGED_SIGNAL(DS_DeviceDef::FluidSources, FluidSourceSyringes)
    CREATE_DATA_CHANGED_SIGNAL(DS_DeviceDef::FluidSource, FluidSourceMuds)
    CREATE_DATA_CHANGED_SIGNAL(DS_DeviceDef::FluidSource, FluidSourceSuds)
    CREATE_DATA_CHANGED_SIGNAL(DS_DeviceDef::FluidSource, FluidSourceWasteContainer)
    CREATE_DATA_CHANGED_SIGNAL(DS_DeviceDef::FluidOptions, FluidOptions)
    CREATE_DATA_CHANGED_SIGNAL(DS_DeviceDef::FluidSources, LastFluidSources)
    CREATE_DATA_CHANGED_SIGNAL(DS_DeviceDef::FluidPackages, AllowedContrastPackages)
    CREATE_DATA_CHANGED_SIGNAL(DS_DeviceDef::FluidPackages, AllowedSalinePackages)
    CREATE_DATA_CHANGED_SIGNAL(DS_DeviceDef::BarcodeInfo, BarcodeInfo)
    CREATE_DATA_CHANGED_SIGNAL(bool, BarcodeReaderConnected)
    CREATE_DATA_CHANGED_SIGNAL(SyringeIdx, MudsLineFluidSyringeIndex)

private:
    // Creates GET/SET Properties
    CREATE_DATA_MEMBERS(bool, DataLocked)
    CREATE_DATA_MEMBERS(bool, IsFirstAutoPrimeCompleted)
    CREATE_DATA_MEMBERS(bool, IsSudsUsed)
    CREATE_DATA_MEMBERS(DS_DeviceDef::FluidSources, FluidSourceBottles)
    CREATE_DATA_MEMBERS(DS_DeviceDef::FluidSources, FluidSourceSyringes)
    CREATE_DATA_MEMBERS(DS_DeviceDef::FluidSource, FluidSourceMuds)
    CREATE_DATA_MEMBERS(DS_DeviceDef::FluidSource, FluidSourceSuds)
    CREATE_DATA_MEMBERS(DS_DeviceDef::FluidSource, FluidSourceWasteContainer)
    CREATE_DATA_MEMBERS(DS_DeviceDef::FluidOptions, FluidOptions)
    CREATE_DATA_MEMBERS(DS_DeviceDef::FluidSources, LastFluidSources)
    CREATE_DATA_MEMBERS(DS_DeviceDef::FluidPackages, AllowedContrastPackages)
    CREATE_DATA_MEMBERS(DS_DeviceDef::FluidPackages, AllowedSalinePackages)
    CREATE_DATA_MEMBERS(DS_DeviceDef::BarcodeInfo, BarcodeInfo)
    CREATE_DATA_MEMBERS(bool, BarcodeReaderConnected)
    CREATE_DATA_MEMBERS(SyringeIdx, MudsLineFluidSyringeIndex)

    EnvGlobal *env;
    EnvLocal *envLocal;

private slots:
    void slotAppStarted();
};

#endif // DS_DEVICE_DATA_H
