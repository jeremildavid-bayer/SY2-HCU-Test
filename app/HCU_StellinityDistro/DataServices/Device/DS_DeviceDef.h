#ifndef DS_DEVICE_DEF_H
#define DS_DEVICE_DEF_H

#include "Common/Common.h"
#include "DataServices/Mcu/DS_McuDef.h"

class DS_DeviceDef
{
public:
    // ==============================================
    // Enumerations

    enum SudsPrimeType
    {
        SUDS_PRIME_TYPE_AUTO = 0,
        SUDS_PRIME_TYPE_MANUAL,
        SUDS_PRIME_TYPE_SYRINGE_PRIME,
        SUDS_PRIME_TYPE_FLUID_PURGE,
        SUDS_PRIME_TYPE_SYRINGE_AIR_RECOVERY,
        SUDS_PRIME_TYPE_USER_DEFINED
    };

    enum FluidSourceIdx
    {
        FLUID_SOURCE_IDX_START = 0,
        FLUID_SOURCE_IDX_BOTTLE_1 = FLUID_SOURCE_IDX_START,
        FLUID_SOURCE_IDX_BOTTLE_2,
        FLUID_SOURCE_IDX_BOTTLE_3,
        FLUID_SOURCE_IDX_SYRINGE_1,
        FLUID_SOURCE_IDX_SYRINGE_2,
        FLUID_SOURCE_IDX_SYRINGE_3,
        FLUID_SOURCE_IDX_MUDS,
        FLUID_SOURCE_IDX_SUDS,
        FLUID_SOURCE_IDX_WASTE_CONTAINER,
        FLUID_SOURCE_IDX_MAX,
        FLUID_SOURCE_IDX_UNKNOWN = FLUID_SOURCE_IDX_MAX
    };

    enum FluidSourceDisposableType
    {
        FLUID_SOURCE_TYPE_INVALID = 0,
        FLUID_SOURCE_TYPE_UNKNOWN,
        FLUID_SOURCE_TYPE_FLS_II,
        FLUID_SOURCE_TYPE_PFA_100,
        FLUID_SOURCE_TYPE_PFA_150,
        FLUID_SOURCE_TYPE_PFA_BRACO,
        FLUID_SOURCE_TYPE_LEXAN,
        FLUID_SOURCE_TYPE_MRXP_LEXAN_SALINE,
        FLUID_SOURCE_TYPE_MRXP_LEXAN_CONTRAST,
        FLUID_SOURCE_TYPE_MRXP_QWIKFIT_115,
        FLUID_SOURCE_TYPE_MRXP_QWIKFIT_65,
        FLUID_SOURCE_TYPE_SALINET_FLS,
        FLUID_SOURCE_TYPE_SY2_WASTE_CONTAINER,
        FLUID_SOURCE_TYPE_SY2_SUDS,
        FLUID_SOURCE_TYPE_SY2_MUDS_ASSEMBLY,
        FLUID_SOURCE_TYPE_SY2_MUDS_RESERVOIR,
        FLUID_SOURCE_TYPE_SY2_BULK_BAG_BOTTLE
    };

    enum WasteContainerLevel
    {
        WASTE_CONTAINER_LEVEL_LOW = 0,
        WASTE_CONTAINER_LEVEL_HIGH,
        WASTE_CONTAINER_LEVEL_FULL,
        WASTE_CONTAINER_LEVEL_UNKNOWN_COMM_DOWN
    };

    // ==============================================
    // Data Structures

    struct FluidPackage
    {
        QString brand;
        double concentration;
        QString concentrationUnits;
        double volume; // 0 if not specified
        QString lotBatch;
        qint64 maximumUseDurationMs;
        qint64 loadedAtEpochMs;
        qint64 expirationDateEpochMs;
        QStringList barcodePrefixes;
        QString fluidKind;
        QString compatibilityFamily;

        bool operator==(const FluidPackage &arg) const
        {
            return ( (brand == arg.brand) &&
                     (concentration == arg.concentration) &&
                     (concentrationUnits == arg.concentrationUnits) &&
                     (volume == arg.volume) &&
                     (lotBatch == arg.lotBatch) &&
                     (maximumUseDurationMs == arg.maximumUseDurationMs) &&
                     (loadedAtEpochMs == arg.loadedAtEpochMs) &&
                     (expirationDateEpochMs == arg.expirationDateEpochMs) &&
                     (barcodePrefixes == arg.barcodePrefixes) &&
                     (fluidKind == arg.fluidKind) &&
                     (compatibilityFamily == arg.compatibilityFamily) );
        }

        bool operator!=(const FluidPackage &arg) const
        {
            return !operator==(arg);
        }

        FluidPackage()
        {
            init();
        }

        void init()
        {
            brand = "Contrast";
            concentration = 0;
            concentrationUnits = "";
            volume = 0;
            lotBatch = "";
            maximumUseDurationMs = -1;
            loadedAtEpochMs = -1;
            expirationDateEpochMs = -1;
            barcodePrefixes = QStringList();
            fluidKind = "Contrast";
            compatibilityFamily = "";
        }

        bool isSameFluidPackage(const FluidPackage &arg) const
        {
            return ( (brand == arg.brand) &&
                     (concentration == arg.concentration) &&
                     (concentrationUnits == arg.concentrationUnits) &&
                     (fluidKind == arg.fluidKind) &&
                     (compatibilityFamily == arg.compatibilityFamily) );
        }
    };

    typedef QList<FluidPackage> FluidPackages;

    struct FluidSelectItem
    {
        QString brand;
        double concentration;
        QString concentrationUnits;
        QList<double> volumes;

        bool operator==(const FluidSelectItem &arg) const
        {
            return( (brand == arg.brand) &&
                    (concentration == arg.concentration) &&
                    (concentrationUnits == arg.concentrationUnits) &&
                    (volumes == arg.volumes) );
        }

        bool operator!=(const FluidSelectItem &arg) const
        {
            return !operator ==(arg);
        }

        bool isSameSelection(const FluidSelectItem &arg) const
        {
            return ( (brand == arg.brand) &&
                     (concentration == arg.concentration) &&
                     (concentrationUnits == arg.concentrationUnits) );
        }

        bool isSameSelection(const FluidPackage &arg) const
        {
            return ( (brand == arg.brand) &&
                     (concentration == arg.concentration) &&
                     (concentrationUnits == arg.concentrationUnits) );
        }

        void getFromFluidPackage(const FluidPackage &arg)
        {
            brand = arg.brand;
            concentration = arg.concentration;
            concentrationUnits = arg.concentrationUnits;
            volumes.append(arg.volume);
        }
    };

    typedef QList<FluidSelectItem> FluidSelectItems;

    struct FluidSource
    {
        bool isReady;
        bool isBusy;
        qint64 installedAtEpochMs;
        QList<double> currentVolumes;
        QList<QString> currentFluidKinds;
        FluidPackages sourcePackages;
        FluidSourceDisposableType disposableType;
        bool needsReplaced;

        FluidSource()
        {
            isReady = false;
            isBusy = false;
            setIsInstalled(false);
            currentVolumes.clear();
            currentVolumes.append(0);
            currentFluidKinds.clear();
            currentFluidKinds.append("Invalid");
            disposableType = DS_DeviceDef::FLUID_SOURCE_TYPE_UNKNOWN;
            needsReplaced = false;
        }

        bool operator==(const FluidSource &arg) const
        {
            return ( (isReady == arg.isReady) &&
                     (isBusy == arg.isBusy) &&
                     (installedAtEpochMs == arg.installedAtEpochMs) &&
                     (currentVolumes == arg.currentVolumes) &&
                     (currentFluidKinds == arg.currentFluidKinds) &&
                     (sourcePackages == arg.sourcePackages) &&
                     (disposableType == arg.disposableType) &&
                     (needsReplaced == arg.needsReplaced) );
        }

        bool operator!=(const FluidSource &arg) const
        {
            return !operator==(arg);
        }

        bool isInstalled() const
        {
            return (installedAtEpochMs > 0);
        }

        void setIsInstalled(bool isInstalled, bool clearSourcePackage = true)
        {
            if (isInstalled)
            {
                installedAtEpochMs = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
            }
            else
            {
                installedAtEpochMs = -1;
                if (clearSourcePackage)
                {
                    sourcePackages.clear();
                }
            }
        }

        double currentVolumesTotal() const
        {
            double curVolume = 0;
            for (int i = 0; i < currentVolumes.length(); i++)
            {
                curVolume += currentVolumes[i];
            }
            return curVolume;
        }

        double currentAvailableVolumesTotal() const
        {
            if (isReady)
            {
                return currentVolumesTotal();
            }
            return 0;
        }

        bool readyForInjection() const
        {
            return (isReady) &&
                   (sourcePackages.length() > 0);
        }
    };

    typedef QList<FluidSource> FluidSources;

    struct FluidFamily
    {
        QString name;
        FluidPackages fluidPackages;

        bool operator==(const FluidFamily &arg) const
        {
            return ( (name == arg.name) &&
                     (fluidPackages == arg.fluidPackages) );
        }

        bool operator!=(const FluidFamily &arg) const
        {
            return !operator==(arg);
        }
    };

    typedef QList<FluidFamily> FluidFamilies;

    struct KnownBarcode
    {
        QString barcode;
        FluidPackage fluidPackage;

        bool operator==(const KnownBarcode &arg) const
        {
            return ( (barcode == arg.barcode) &&
                     (fluidPackage == arg.fluidPackage) );
        }

        bool operator!=(const KnownBarcode &arg) const
        {
            return !operator==(arg);
        }
    };

    typedef QList<KnownBarcode> KnownBarcodes;

    struct FluidOptions
    {
        FluidFamilies contrastFamilies;
        FluidPackages salinePackages;
        qint64 changedAtEpochMs;
        KnownBarcodes knownBarcodes;

        bool isSameFluidOptions(const FluidOptions &arg) const
        {
            return ( (contrastFamilies == arg.contrastFamilies) &&
                     (salinePackages == arg.salinePackages) &&
                     (knownBarcodes == arg.knownBarcodes) );
        }

        bool operator==(const FluidOptions &arg) const
        {
            return ( (contrastFamilies == arg.contrastFamilies) &&
                     (salinePackages == arg.salinePackages) &&
                     (changedAtEpochMs == arg.changedAtEpochMs) &&
                     (knownBarcodes == arg.knownBarcodes) );
        }

        bool operator!=(const FluidOptions &arg) const
        {
            return !operator==(arg);
        }

        FluidPackages getAllContrastPackages() const
        {
            FluidPackages packages;

            for (int familyIdx = 0; familyIdx < contrastFamilies.length(); familyIdx++)
            {
                packages.append(contrastFamilies[familyIdx].fluidPackages);
            }

            return packages;
        }

        FluidPackages getAllowedContrastPackages(const FluidPackage &fluidPackage) const
        {
            FluidPackages packages;

            // Get FamilyName for fluidPackage
            for (int familyIdx = 0; familyIdx < contrastFamilies.length(); familyIdx++)
            {
                for (int packageIdx = 0; packageIdx < contrastFamilies[familyIdx].fluidPackages.length(); packageIdx++)
                {
                    if ( (contrastFamilies[familyIdx].fluidPackages[packageIdx].brand.toLower() == fluidPackage.brand.toLower()) &&
                         (contrastFamilies[familyIdx].fluidPackages[packageIdx].concentration == fluidPackage.concentration) )
                    {
                        return contrastFamilies[familyIdx].fluidPackages;
                    }
                }
            }
            return packages;
        }
    };

    struct FluidInfo
    {
        DS_DeviceDef::FluidSourceIdx location;
        QString colorCode;
        FluidPackage fluidPackage;

        bool operator==(const FluidInfo &arg) const
        {
            return ( (location == arg.location) &&
                     (colorCode == arg.colorCode) &&
                     (fluidPackage == arg.fluidPackage) );
        }

        bool operator!=(const FluidInfo &arg) const
        {
            return !operator==(arg);
        }

        FluidInfo()
        {
            init();
        }

        void init()
        {
            location = DS_DeviceDef::FLUID_SOURCE_IDX_SYRINGE_2;
            colorCode = "GREEN";
            unloadFluidPackage();
        }

        void unloadFluidPackage()
        {
            fluidPackage.init();
        }

        bool isDefined()
        {
            return ( (fluidPackage.brand != "Contrast") &&
                     (fluidPackage.concentration != 0) );

        }
    };

    struct BarcodeInfo
    {
        QString barcodePrefix;
        qint64 scannedAtEpochMs;
        bool isKnownBarcode;
        bool isFluidPackageOk;
        DS_DeviceDef::FluidPackage fluidPackage;
        QString err;

        BarcodeInfo()
        {
            barcodePrefix = "";
            scannedAtEpochMs = 0;
            isKnownBarcode = false;
            isFluidPackageOk = false;
            fluidPackage.brand = "";
            fluidPackage.compatibilityFamily = "";
            fluidPackage.concentration = 0;
            err = "";
        }

        bool operator==(const BarcodeInfo &arg)
        {
            return ( (barcodePrefix == arg.barcodePrefix) &&
                     (scannedAtEpochMs == arg.scannedAtEpochMs) &&
                     (isKnownBarcode == arg.isKnownBarcode) &&
                     (isFluidPackageOk == arg.isFluidPackageOk) &&
                     (fluidPackage == arg.fluidPackage) &&
                     (err == arg.err) );
        }

        bool operator!=(const BarcodeInfo &arg)
        {
            return !operator ==(arg);
        }
    };

};

Q_DECLARE_METATYPE(DS_DeviceDef::FluidSource);
Q_DECLARE_METATYPE(DS_DeviceDef::FluidSources);
Q_DECLARE_METATYPE(DS_DeviceDef::FluidOptions);
Q_DECLARE_METATYPE(DS_DeviceDef::FluidPackages);
Q_DECLARE_METATYPE(DS_DeviceDef::FluidInfo);
Q_DECLARE_METATYPE(DS_DeviceDef::BarcodeInfo);
#endif // DS_DEVICE_DEF_H
