#include "QML_Device.h"
#include "Common/Config.h"
#include "Common/ImrParser.h"
#include "DataServices/Device/DS_DeviceData.h"
#include "DataServices/Device/DS_DeviceAction.h"

QML_Device::QML_Device(QObject *parent, EnvGlobal *env_) :
    QObject(parent),
    env(env_)
{
    envLocal = new EnvLocal("QML_Device", "QML_DEVICE");
    qmlSrc = env->qml.object->findChild<QObject*>("dsDevice");
    env->qml.engine->rootContext()->setContextProperty("dsDeviceCpp", this);

    if (qmlSrc == NULL)
    {
        LOG_ERROR("Failed to assign qmlSrc.\n");
    }

    // Register Data Callbacks
    connect(env->ds.deviceData, &DS_DeviceData::signalDataChanged_FluidSourceSuds, this, [=](const DS_DeviceDef::FluidSource &fluidSourceSuds) {
        qmlSrc->setProperty("fluidSourceSuds", ImrParser::ToImr_FluidSource(fluidSourceSuds));
    });

    connect(env->ds.deviceData, &DS_DeviceData::signalDataChanged_FluidSourceSyringes, this, [=](const DS_DeviceDef::FluidSources &fluidSourceSyringes, const DS_DeviceDef::FluidSources &prevFluidSourceSyringes) {
        if (fluidSourceSyringes[SYRINGE_IDX_SALINE] != prevFluidSourceSyringes[SYRINGE_IDX_SALINE])
        {
            qmlSrc->setProperty("fluidSourceSyringe1", ImrParser::ToImr_FluidSource(fluidSourceSyringes[SYRINGE_IDX_SALINE]));
            if (fluidSourceSyringes[SYRINGE_IDX_SALINE].sourcePackages != prevFluidSourceSyringes[SYRINGE_IDX_SALINE].sourcePackages)
            {
                qmlSrc->setProperty("fluidSourceSyringePackages1", ImrParser::ToImr_FluidPackages(fluidSourceSyringes[SYRINGE_IDX_SALINE].sourcePackages));
            }
        }

        if (fluidSourceSyringes[SYRINGE_IDX_CONTRAST1] != prevFluidSourceSyringes[SYRINGE_IDX_CONTRAST1])
        {
            qmlSrc->setProperty("fluidSourceSyringe2", ImrParser::ToImr_FluidSource(fluidSourceSyringes[SYRINGE_IDX_CONTRAST1]));
            if (fluidSourceSyringes[SYRINGE_IDX_CONTRAST1].sourcePackages != prevFluidSourceSyringes[SYRINGE_IDX_CONTRAST1].sourcePackages)
            {
                qmlSrc->setProperty("fluidSourceSyringePackages2", ImrParser::ToImr_FluidPackages(fluidSourceSyringes[SYRINGE_IDX_CONTRAST1].sourcePackages));
            }
        }

        if (fluidSourceSyringes[SYRINGE_IDX_CONTRAST2] != prevFluidSourceSyringes[SYRINGE_IDX_CONTRAST2])
        {
            qmlSrc->setProperty("fluidSourceSyringe3", ImrParser::ToImr_FluidSource(fluidSourceSyringes[SYRINGE_IDX_CONTRAST2]));
            if (fluidSourceSyringes[SYRINGE_IDX_CONTRAST2].sourcePackages != prevFluidSourceSyringes[SYRINGE_IDX_CONTRAST2].sourcePackages)
            {
                qmlSrc->setProperty("fluidSourceSyringePackages3", ImrParser::ToImr_FluidPackages(fluidSourceSyringes[SYRINGE_IDX_CONTRAST2].sourcePackages));
            }
        }
    });

    connect(env->ds.deviceData, &DS_DeviceData::signalDataChanged_FluidSourceBottles, this, [=](const DS_DeviceDef::FluidSources &fluidSourceBottles, const DS_DeviceDef::FluidSources &prevFluidSourceBottles) {
        if (fluidSourceBottles[SYRINGE_IDX_SALINE] != prevFluidSourceBottles[SYRINGE_IDX_SALINE])
        {
            qmlSrc->setProperty("fluidSourceBottle1", ImrParser::ToImr_FluidSource(fluidSourceBottles[SYRINGE_IDX_SALINE]));
            if (fluidSourceBottles[SYRINGE_IDX_SALINE].sourcePackages != prevFluidSourceBottles[SYRINGE_IDX_SALINE].sourcePackages)
            {
                qmlSrc->setProperty("fluidSourceBottlePackages1", ImrParser::ToImr_FluidPackages(fluidSourceBottles[SYRINGE_IDX_SALINE].sourcePackages));
            }
        }
        if (fluidSourceBottles[SYRINGE_IDX_CONTRAST1] != prevFluidSourceBottles[SYRINGE_IDX_CONTRAST1])
        {
            qmlSrc->setProperty("fluidSourceBottle2", ImrParser::ToImr_FluidSource(fluidSourceBottles[SYRINGE_IDX_CONTRAST1]));
            if (fluidSourceBottles[SYRINGE_IDX_CONTRAST1].sourcePackages != prevFluidSourceBottles[SYRINGE_IDX_CONTRAST1].sourcePackages)
            {
                qmlSrc->setProperty("fluidSourceBottlePackages2", ImrParser::ToImr_FluidPackages(fluidSourceBottles[SYRINGE_IDX_CONTRAST1].sourcePackages));
            }
        }
        if (fluidSourceBottles[SYRINGE_IDX_CONTRAST2] != prevFluidSourceBottles[SYRINGE_IDX_CONTRAST2])
        {
            qmlSrc->setProperty("fluidSourceBottle3", ImrParser::ToImr_FluidSource(fluidSourceBottles[SYRINGE_IDX_CONTRAST2]));
            if (fluidSourceBottles[SYRINGE_IDX_CONTRAST2].sourcePackages != prevFluidSourceBottles[SYRINGE_IDX_CONTRAST2].sourcePackages)
            {
                qmlSrc->setProperty("fluidSourceBottlePackages3", ImrParser::ToImr_FluidPackages(fluidSourceBottles[SYRINGE_IDX_CONTRAST2].sourcePackages));
            }
        }
        qmlSrc->setProperty("sameContrasts", env->ds.deviceData->isSameContrastsLoaded());
    });

    connect(env->ds.deviceData, &DS_DeviceData::signalDataChanged_FluidSourceMuds, this, [=](const DS_DeviceDef::FluidSource &fluidSourceMuds) {
        qmlSrc->setProperty("fluidSourceMuds", ImrParser::ToImr_FluidSource(fluidSourceMuds));
    });

    connect(env->ds.deviceData, &DS_DeviceData::signalDataChanged_FluidSourceWasteContainer, this, [=](const DS_DeviceDef::FluidSource &fluidSourceWasteContainer) {
        qmlSrc->setProperty("fluidSourceWasteContainer", ImrParser::ToImr_FluidSource(fluidSourceWasteContainer));
    });

    connect(env->ds.deviceData, &DS_DeviceData::signalDataChanged_MudsLineFluidSyringeIndex, this, [=](const SyringeIdx &mudsLineFluidSyringeIndex) {
        qmlSrc->setProperty("mudsLineFluidSyringeIndex", ImrParser::ToImr_FluidSourceSyringeIdx(mudsLineFluidSyringeIndex));
    });

    connect(env->ds.deviceData, &DS_DeviceData::signalDataChanged_AllowedContrastPackages, this, [=](const DS_DeviceDef::FluidPackages &allowedPackages) {
        qmlSrc->setProperty("contrastSelectItems", ImrParser::ToImr_FluidSourceSelectItems(allowedPackages));
    });

    connect(env->ds.deviceData, &DS_DeviceData::signalDataChanged_AllowedSalinePackages, this, [=](const DS_DeviceDef::FluidPackages &allowedPackages) {
        qmlSrc->setProperty("salineSelectItems", ImrParser::ToImr_FluidSourceSelectItems(allowedPackages));
    });

    connect(env->ds.deviceData, &DS_DeviceData::signalDataChanged_BarcodeInfo, this, [=](const DS_DeviceDef::BarcodeInfo &barcodeInfo) {
        qmlSrc->setProperty("barcodeInfo", ImrParser::ToImr_BarcodeInfo(barcodeInfo));
    });

    connect(env->ds.deviceData, &DS_DeviceData::signalDataChanged_BarcodeReaderConnected, this, [=](const bool &barcodeReaderConnected) {
        qmlSrc->setProperty("barcodeReaderConnected", barcodeReaderConnected);
    });
}

QML_Device::~QML_Device()
{
    delete envLocal;
}

void QML_Device::slotStopcockFillPosition(QString syringeIdxStr)
{
    env->ds.deviceAction->actStopcock(ImrParser::ToCpp_FluidSourceSyringeIdx(syringeIdxStr), DS_McuDef::STOPCOCK_POS_FILL);
}

void QML_Device::slotStopcockInjectPosition(QString syringeIdxStr)
{
    env->ds.deviceAction->actStopcock(ImrParser::ToCpp_FluidSourceSyringeIdx(syringeIdxStr), DS_McuDef::STOPCOCK_POS_INJECT);
}

void QML_Device::slotStopcockClosePosition(QString syringeIdxStr)
{
    env->ds.deviceAction->actStopcock(ImrParser::ToCpp_FluidSourceSyringeIdx(syringeIdxStr), DS_McuDef::STOPCOCK_POS_CLOSED);
}

void QML_Device::slotBarcodeReaderStart(int sleepTimeoutMs)
{
    env->ds.deviceAction->actBarcodeReaderStart(sleepTimeoutMs);
}

void QML_Device::slotBarcodeReaderStop()
{
    env->ds.deviceAction->actBarcodeReaderStop();
}

void QML_Device::slotBarcodeReaderConnect()
{
    env->ds.deviceAction->actBarcodeReaderConnect();
}

void QML_Device::slotBarcodeReaderDisconnect()
{
    env->ds.deviceAction->actBarcodeReaderDisconnect();
}

void QML_Device::slotBarcodeReaderSetBarcodeData(QString data)
{
    env->ds.deviceAction->actBarcodeReaderSetBarcodeData(data);
}

void QML_Device::slotSyringeAirCheck(QString syringeIdxStr)
{
    env->ds.deviceAction->actSyringeAirCheck(ImrParser::ToCpp_FluidSourceSyringeIdx(syringeIdxStr));
}

bool QML_Device::slotGetSyringeSodStartReady(QString syringeIdxStr)
{
    return env->ds.deviceData->getSyringeSodStartReady(ImrParser::ToCpp_FluidSourceSyringeIdx(syringeIdxStr));
}
