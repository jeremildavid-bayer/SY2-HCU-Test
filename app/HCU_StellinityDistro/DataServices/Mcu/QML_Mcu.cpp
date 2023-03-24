#include "QML_Mcu.h"
#include "DataServices/Mcu/DS_McuData.h"
#include "DataServices/Mcu/DS_McuAction.h"
#include "Common/ImrParser.h"

QML_Mcu::QML_Mcu(QObject *parent, EnvGlobal *env_) :
    QObject(parent),
    env(env_)
{
    envLocal = new EnvLocal("QML_Mcu", "QML_MCU");
    qmlSrc = env->qml.object->findChild<QObject*>("dsMcu");
    env->qml.engine->rootContext()->setContextProperty("dsMcuCpp", this);

    if (qmlSrc == NULL)
    {
        LOG_ERROR("Failed to assign qmlSrc.\n");
    }

    // Register Data Callbacks
    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_PowerStatus, this, [=](const DS_McuDef::PowerStatus &powerStatus) {
        qmlSrc->setProperty("powerStatus", ImrParser::ToImr_PowerStatus(powerStatus));
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_Pressure, this, [=](int pressureKpa) {
        qmlSrc->setProperty("pressureKpa", pressureKpa);
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_DigestMap, this, [=](const QVariantMap &digestMap) {
        qmlSrc->setProperty("hwDigest", digestMap);
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_PressureCalibrationStatus, this, [=](const DS_McuDef::PressureCalibrationStatus &status) {
        qmlSrc->setProperty("pressureCalibrationStatus", ImrParser::ToImr_PressureCalibrationStatus(status));
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_LinkState, this, [=](DS_McuDef::LinkState linkState) {
        qmlSrc->setProperty("mcuLinkState", ImrParser::ToImr_McuLinkState(linkState));
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_McuVersion, this, [=](QString version) {
        qmlSrc->setProperty("mcuVersion", version);
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_McuCommandVersion, this, [=](QString version) {
        qmlSrc->setProperty("mcuCommandVersion", version);
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_StopcockVersion, this, [=](QString version) {
        qmlSrc->setProperty("stopcockVersion", version);
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_McuSerialNumber, this, [=](QString serialNumber) {
        qmlSrc->setProperty("mcuSerialNumber", serialNumber);
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_MotorModuleSerialNumbers, this, [=](const QStringList &motorModuleSerialNumbers) {
        qmlSrc->setProperty("motorModuleSerialNumbers", motorModuleSerialNumbers);
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_MudsPresent, this, [=](bool isPresent) {
        qmlSrc->setProperty("isMudsPresent", isPresent);
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_SyringeAirCheckDigests, this, [=](const DS_McuDef::SyringeAirCheckDigests &syringeAirCheckDigests) {
        qmlSrc->setProperty("syringeAirCheckDigests", ImrParser::ToImr_SyringeAirCheckDigests(syringeAirCheckDigests));
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_SyringeVols, this, [=](const QList<double> &vols) {
        qmlSrc->setProperty("syringeVols", ImrParser::ToImr_DoubleList(vols));
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_SyringeFlows, this, [=](const QList<double> &flows) {
        qmlSrc->setProperty("syringeFlows", ImrParser::ToImr_DoubleList(flows));
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_PlungerStates, this, [=](const DS_McuDef::PlungerStates &states) {
        qmlSrc->setProperty("plungerStates", ImrParser::ToImr_PlungerStates(states));
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_SyringeStates, this, [=](const DS_McuDef::SyringeStates &states) {
        qmlSrc->setProperty("syringeStates", ImrParser::ToImr_SyringeStates(states));
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_BottleBubbleStates, this, [=](const DS_McuDef::BottleBubbleDetectorStates &states) {
        qmlSrc->setProperty("bottleBubbleStates", ImrParser::ToImr_BottleBubbleDetectorStates(states));
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_SudsBubbleDetected, this, [=](bool detected) {
        qmlSrc->setProperty("sudsBubbleDetected", detected);
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_StopcockPosAll, this, [=](const DS_McuDef::StopcockPosAll &stopcockPosAll) {
        qmlSrc->setProperty("stopcockPositions", ImrParser::ToImr_StopcockPosAll(stopcockPosAll));
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_DoorState, this, [=](DS_McuDef::DoorState state) {
        qmlSrc->setProperty("doorState", ImrParser::ToImr_DoorState(state));
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_OutletDoorState, this, [=](DS_McuDef::OutletDoorState state) {
        qmlSrc->setProperty("outletDoorState", ImrParser::ToImr_OutletDoorState(state));
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_MudsInserted, this, [=](bool inserted) {
        qmlSrc->setProperty("mudsInserted", inserted);
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_MudsLatched, this, [=](bool latched) {
        qmlSrc->setProperty("mudsLatched", latched);
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_MudsPresent, this, [=](bool latched) {
        qmlSrc->setProperty("mudsPresent", latched);
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_SudsInserted, this, [=](bool inserted) {
        qmlSrc->setProperty("sudsInserted", inserted);
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_PrimeBtnPressed, this, [=](bool pressed) {
       qmlSrc->setProperty("primeBtnPressed", pressed);
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_StopBtnPressed, this, [=](bool pressed) {
       qmlSrc->setProperty("stopBtnPressed", pressed);
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_DoorBtnPressed, this, [=](bool pressed) {
       qmlSrc->setProperty("doorBtnPressed", pressed);
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_HeatMaintainerStatus, this, [=](const DS_McuDef::HeatMaintainerStatus &heatMaintainerStatus) {
        qmlSrc->setProperty("heatMaintainerStatus", ImrParser::ToImr_HeatMaintainerStatus(heatMaintainerStatus));
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_IsShuttingDown, this, [=](bool isShuttingDown) {
        qmlSrc->setProperty("isShuttingDown", isShuttingDown);
    });

    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_BMSDigests, this, [=](const DS_McuDef::BMSDigests &bmsDigests) {
        qmlSrc->setProperty("bmsDigests", ImrParser::ToImr_BMSDigests(bmsDigests));
    });
}

QML_Mcu::~QML_Mcu()
{
    delete envLocal;
}

void QML_Mcu::calibrateHw(QString actGuid, QString name)
{
    env->actionMgr->onActionStarted(actGuid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus status) {
        QString result;

        if (status.state == DS_ACTION_STATE_STARTED)
        {
            LOG_INFO("CALIBRATE_HW: %s\n", ImrParser::ToImr_DataServiceActionStatusStr(status).CSTR());
            result = QString().asprintf("%s Completed", name.CSTR());
        }
        else
        {
            LOG_ERROR("CALIBRATE_HW: %s\n", ImrParser::ToImr_DataServiceActionStatusStr(status).CSTR());
            result = QString().asprintf("ERROR: %s Failed.\nState: %s\nReply: %s", name.CSTR(), ImrParser::ToImr_DataServiceActionState(status.state).CSTR(), status.reply.CSTR());
        }

        qmlSrc->setProperty("calibrationResult", result);
    });

    // Action should be started from outside this function
    LOG_INFO("CALIBRATE_HW: ACTION_STARTING: Action=%s\n", name.CSTR());
}

void QML_Mcu::slotHwDigestMonitorActive(bool active)
{
    env->ds.mcuData->setHwDigestMonitorEnabled(active);
}

void QML_Mcu::slotResetMuds()
{
    env->ds.mcuAction->actResetMuds();
}

void QML_Mcu::slotResetMcu()
{
    env->ds.mcuAction->actResetMcuHw();
    env->ds.mcuAction->actLinkConnect();
}

void QML_Mcu::slotResetStopcock()
{
    env->ds.mcuAction->actResetScbHw();
    env->ds.mcuAction->actLinkConnect();
}

void QML_Mcu::slotCalibrateAirDetector(int idx)
{
    QString guid = Util::newGuid();
    calibrateHw(guid, QString().asprintf("InletAirDetector%d Calibration", idx));
    env->ds.mcuAction->actCalInletAirDetector((SyringeIdx)idx, guid);
}

void QML_Mcu::slotCalibrateMotor(int idx)
{
    QString guid = Util::newGuid();
    calibrateHw(guid, QString().asprintf("Motor%d Calibration", idx));
    env->ds.mcuAction->actCalMotor((SyringeIdx)idx, guid);
}

void QML_Mcu::slotCalibrateSuds()
{
    QString guid = Util::newGuid();
    calibrateHw(guid, QString().asprintf("SUDS Calibration"));
    env->ds.mcuAction->actCalSudsSensor(guid);
}

void QML_Mcu::slotCalibratePlunger(int idx)
{
    QString guid = Util::newGuid();
    calibrateHw(guid, QString().asprintf("Plunger%d Calibration", idx));
    env->ds.mcuAction->actCalPlunger((SyringeIdx)idx, guid);
}

void QML_Mcu::slotCalibrateSetPressureMeter()
{
    QString guid = Util::newGuid();
    calibrateHw(guid, QString().asprintf("Set Pressure Port"));
    env->ds.mcuAction->actCalSetPressureMeter(guid);
}

void QML_Mcu::slotGetCalibrationStatus()
{
    QString actGuid = Util::newGuid();
    env->actionMgr->onActionStarted(actGuid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus status) {
        LOG_INFO("GET_CALIBRATION_STATUS - %s\n", ImrParser::ToImr_DataServiceActionStatusStr(status).CSTR());

        if (status.state == DS_ACTION_STATE_STARTED)
        {
            QString result = status.reply.replace(";", "\n");
            qmlSrc->setProperty("calibrationResult", result);
        }
    });
    env->ds.mcuAction->actCalDigest(actGuid);
}

void QML_Mcu::slotCalibratePressureStart(int idx)
{
    env->ds.mcuAction->actCalPressureStart((SyringeIdx)idx);
}

void QML_Mcu::slotCalibratePressureStop()
{
    env->ds.mcuAction->actCalPressureStop();
}

void QML_Mcu::slotCalibrateSetPressure(int idx)
{
    env->ds.mcuAction->actCalSetPressureData((SyringeIdx)idx);
}

void QML_Mcu::slotFindPlunger(int index)
{
    env->ds.mcuAction->actFindPlunger((SyringeIdx)index);
}

void QML_Mcu::slotPullPlungers()
{
    env->ds.mcuAction->actPullPlungers();
}

void QML_Mcu::slotPistonEngage(int index)
{
    env->ds.mcuAction->actEngage((SyringeIdx)index);
}

void QML_Mcu::slotPistonDisengage(int index)
{
    env->ds.mcuAction->actDisengage((SyringeIdx)index);
}

void QML_Mcu::slotPistonStop(int)
{
    env->ds.mcuAction->actStopAll();
}

void QML_Mcu::slotPistonUp(int index, double flow)
{
    DS_McuDef::ActPistonParams pistonParams;
    pistonParams.idx = (SyringeIdx)index;
    pistonParams.vol = SYRINGE_VOLUME_FILL_ALL;
    pistonParams.flow = flow;
    env->ds.mcuAction->actPiston(pistonParams);
}

void QML_Mcu::slotPistonDown(int index, double flow)
{
    DS_McuDef::ActPistonParams pistonParams;
    pistonParams.idx = (SyringeIdx)index;
    pistonParams.vol = SYRINGE_VOLUME_FILL_ALL;
    pistonParams.flow = -flow;
    env->ds.mcuAction->actPiston(pistonParams);
}

void QML_Mcu::slotLedControl(QVariant params)
{
    DS_McuDef::LedControlStatus ledControlStatus = ImrParser::ToCpp_LedControlStatus(params.toMap());
    env->ds.mcuAction->actLeds(ledControlStatus.paramsList);
}

void QML_Mcu::slotDoorLock(bool lock)
{
    env->ds.mcuAction->actLockDoor(lock);
}

void QML_Mcu::slotMovePistonToCleaningPos(int index)
{
    DS_McuDef::ActPistonParams params;
    params.flow = PISTON_CLEANING_POSITION_FLOW_RATE;
    double syringeVol = env->ds.mcuData->getSyringeVols()[index];
    if (Util::isFloatVarGreaterThan(syringeVol, PISTON_CLEANING_POSITION_ML, 1))
    {
        params.vol = syringeVol - PISTON_CLEANING_POSITION_ML;
    }
    else
    {
        params.vol = SYRINGE_VOLUME_FILL_ALL;
    }
    params.idx = (SyringeIdx)index;
    env->ds.mcuAction->actPiston(params);
}

void QML_Mcu::slotBmsDigest()
{
    QString actGuid = Util::newGuid();
    env->actionMgr->onActionStarted(actGuid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus status) {
        LOG_INFO("GET_BMS_DIGEST - %s\n", ImrParser::ToImr_DataServiceActionStatusStr(status).CSTR());
        QString output = status.reply.replace(";", "\n");
        qmlSrc->setProperty("bmsDigestOutput", output);
    });
    env->ds.mcuAction->actBmsDigest(actGuid);
}

void QML_Mcu::slotBmsCommand(int index, QString data)
{
    QString actGuid = Util::newGuid();
    env->actionMgr->onActionStarted(actGuid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus status) {
        LOG_INFO("GET_BMS_Command - %s\n", ImrParser::ToImr_DataServiceActionStatusStr(status).CSTR());
        qmlSrc->setProperty("bmsCommandOutput", status.reply);
    });
    env->ds.mcuAction->actBmsCommand(index, data, actGuid);
}

void QML_Mcu::slotSetBaseFanTemperature(int temperature)
{
    env->ds.mcuAction->actSetBaseFanTemperature(temperature);
}
