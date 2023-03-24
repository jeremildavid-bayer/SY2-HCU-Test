#include "Apps/AppManager.h"
#include "McuMsgHandler.h"
#include "Common/ImrParser.h"
#include "DataServices/Mcu/DS_McuData.h"
#include "DataServices/System/DS_SystemData.h"
#include "DataServices/Capabilities/DS_Capabilities.h"
#include "DataServices/Alert/DS_AlertAction.h"

McuMsgHandler::McuMsgHandler(QObject *parent, EnvGlobal *env_, EnvLocal *envLocal_) :
    QObject(parent),
    env(env_),
    envLocal(envLocal_)
{
    mcuLog = new Log(PATH_LOG_DIR + QString("Mcu.log"), LOG_MID_SIZE_BYTES);

    mcuLog->open();
    mcuLog->writeInfo("===========================================================\n");
    mcuLog->writeInfo("\n\n\n");
    mcuLog->writeInfo("HCU_Stellinity Distro - Mcu Started..\n");


    mcuInjectDigestLog = new Log(PATH_LOG_DIR + QString("Mcu-InjectDigest.log"));
    mcuInjectDigestLog->open();
    mcuInjectDigestLog->writeInfo("===========================================================\n");
    mcuInjectDigestLog->writeInfo("\n\n\n");
    mcuInjectDigestLog->writeInfo("HCU_Stellinity Distro - Mcu Inject Digest Started..\n");

    diagnosticEvents.clear();

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });
}

McuMsgHandler::~McuMsgHandler()
{
    delete mcuLog;
    delete mcuInjectDigestLog;
}

void McuMsgHandler::slotAppInitialised()
{
    connect(env->ds.systemData, &DS_SystemData::signalDataChanged_StatePath, this, [=](DS_SystemDef::StatePath curStatePath, DS_SystemDef::StatePath prevStatePath) {
        if ( (prevStatePath == DS_SystemDef::STATE_PATH_READY_ARMED) &&
             (curStatePath == DS_SystemDef::STATE_PATH_EXECUTING) )
        {
            if (env->ds.capabilities->get_Logging_McuInjectDigest())
            {
                mcuInjectDigestLog->writeDebug("\n\nINJECTION STARTED:\n");
            }
        }
    });

    connect(&tmrLastDiagnosticEventTimeOut, SIGNAL(timeout()), SLOT(slotActivateDiagnosticEventAlert()));
}

QString McuMsgHandler::handleMsg(QString id, QString body, QString reply)
{
    if (id == _L("VERSION"))
    {
        return handleVersion(reply);
    }
    else if (id == _L("INFO"))
    {
        return handleInfo(reply);
    }
    else if (id == _L("DIGEST"))
    {
        return handleDigest(body, reply);
    }
    else if (id == _L("HWDIGEST"))
    {
        return handleHwDigest(reply);
    }
    else if (id == _L("BMSDIGEST"))
    {
        return handleBMSDigest(reply);
    }
    else if (id == _L("INJECTDIGEST"))
    {
        return handleInjectDigest(reply);
    }
    else if (id == _L("GETSYRINGEAIRCHECKDATA"))
    {
        return handleGetSyringeAirCheckData(body, reply);
    }
    else if (id == _L("GETSYRINGEAIRCHECKCOEFF"))
    {
        return handleGetSyringeAirCheckCoeff(body, reply);
    }
    else if (id == _L("GETSYRINGEAIRVOL"))
    {
         return handleGetSyringeAirVol(body, reply);
    }
    else if (id == _L("GETPCALCOEFF"))
    {
        return handleGetPressureCalCoeff(body, reply);
    }
    else if (id == _L("GETPLUNGERFRICTION"))
    {
        return handleGetPlungerFriction(body, reply);
    }    

    return QString().asprintf("Not supported message id(%s)", id.CSTR());
}

QString McuMsgHandler::handleDigest(QString body, QString reply)
{
    static bool badParameterWarned = false;

    QString errStr = "";

    QStringList listParam = reply.split(",");
    int expectedParams = 39;

    if (listParam.length() < expectedParams)
    {
        errStr = QString().asprintf("Bad MCU Parameter Count(%d)", (int)listParam.count());
        LOG_ERROR("handleDigest(): Bad Digest Reply received. Err=%s, Reply=%s\n", errStr.CSTR(), reply.CSTR());
        if (!badParameterWarned)
        {
            badParameterWarned = true;
            env->ds.alertAction->activate("HCUInternalSoftwareError", errStr);
        }

        while (listParam.length() < expectedParams)
        {
            listParam.append("");
        }
    }

    QByteArray alarmCodes = env->ds.mcuData->getAlarmCodes();
    DS_McuDef::InjectorStatus injectorStatus = env->ds.mcuData->getInjectorStatus();
    int pressure = env->ds.mcuData->getPressure();
    DS_McuDef::StopcockPosAll stopcockPosAll = env->ds.mcuData->getStopcockPosAll();
    DS_McuDef::PlungerStates plungerStates = env->ds.mcuData->getPlungerStates();
    DS_McuDef::SyringeStates syringeStates = env->ds.mcuData->getSyringeStates();
    QList<double> syringeVols = env->ds.mcuData->getSyringeVols();
    QList<double> syringeFlows = env->ds.mcuData->getSyringeFlows();
    DS_McuDef::PowerStatus powerStatus = env->ds.mcuData->getPowerStatus();
    DS_McuDef::DoorState doorState = env->ds.mcuData->getDoorState();
    DS_McuDef::WasteBinState wasteBinState = env->ds.mcuData->getWasteBinState();
    bool mudsInserted = env->ds.mcuData->getMudsInserted(); //NON Digest Parameter
    bool mudsPresent = env->ds.mcuData->getMudsPresent();
    bool mudsLatched = env->ds.mcuData->getMudsLatched();
    DS_McuDef::BottleBubbleDetectorStates bottleBubbleStates = env->ds.mcuData->getBottleBubbleStates();
    bool sudsInserted = env->ds.mcuData->getSudsInserted();
    bool sudsBubbleDetected = env->ds.mcuData->getSudsBubbleDetected();
    bool primeBtnPressed = env->ds.mcuData->getPrimeBtnPressed();
    bool stopBtnPressed = env->ds.mcuData->getStopBtnPressed();
    bool doorBtnPressed = env->ds.mcuData->getDoorBtnPressed();
    DS_McuDef::OutletDoorState outletDoorState = env->ds.mcuData->getOutletDoorState();
    DS_McuDef::HeatMaintainerStatus heatMaintainerStatus = env->ds.mcuData->getHeatMaintainerStatus();
    bool isShuttingDown = env->ds.mcuData->getIsShuttingDown();

    QByteArray prevAlarmCodes = alarmCodes;
    DS_McuDef::InjectorStatus prevInjectorStatus = injectorStatus;
    int prevPressure = pressure;
    DS_McuDef::StopcockPosAll prevStopcockPosAll = stopcockPosAll;
    DS_McuDef::PlungerStates prevPlungerStates = plungerStates;
    DS_McuDef::SyringeStates prevSyringeStates = syringeStates;
    QList<double> prevSyringeVols = syringeVols;
    QList<double> prevSyringeFlows = syringeFlows;
    DS_McuDef::PowerStatus prevPowerStatus = powerStatus;
    DS_McuDef::DoorState prevDoorState = doorState;
    DS_McuDef::WasteBinState prevWasteBinState = wasteBinState;
    bool prevMudsInserted = mudsInserted;
    bool prevMudsPresent = mudsPresent;
    bool prevMudsLatched = mudsLatched;
    DS_McuDef::BottleBubbleDetectorStates prevBottleBubbleStates = bottleBubbleStates;
    bool prevSudsInserted = sudsInserted;
    bool prevSudsBubbleDetected = sudsBubbleDetected;
    bool prevPrimeBtnPressed = primeBtnPressed;
    bool prevStopBtnPressed = stopBtnPressed;
    bool prevDoorBtnPressed = doorBtnPressed;
    DS_McuDef::OutletDoorState prevOutletDoorState = outletDoorState;
    DS_McuDef::HeatMaintainerStatus prevHeatMaintainerStatus = heatMaintainerStatus;
    bool prevIsShuttingDown = isShuttingDown;

    int idx = 0;

    alarmCodes = QByteArray::fromHex(listParam[idx++].toLocal8Bit());
    injectorStatus.state = ImrParser::ToCpp_InjectorState(listParam[idx++]);
    injectorStatus.completeStatus = ImrParser::ToCpp_InjectionCompleteStatus(listParam[idx++]);
    pressure = listParam[idx++].toInt();

    for (int syringeIdx = 0; syringeIdx < SYRINGE_IDX_MAX; syringeIdx++)
    {
        stopcockPosAll[syringeIdx] = ImrParser::ToCpp_StopcockPos(listParam[idx++]);
    }

    for (int syringeIdx = 0; syringeIdx < SYRINGE_IDX_MAX; syringeIdx++)
    {
        plungerStates[syringeIdx] = ImrParser::ToCpp_PlungerState(listParam[idx++]);
    }

    for (int syringeIdx = 0; syringeIdx < SYRINGE_IDX_MAX; syringeIdx++)
    {
        syringeStates[syringeIdx] = ImrParser::ToCpp_SyringeState(listParam[idx++]);
    }

    for (int syringeIdx = 0; syringeIdx < SYRINGE_IDX_MAX; syringeIdx++)
    {
        syringeVols[syringeIdx] = listParam[idx++].toDouble();
    }

    for (int syringeIdx = 0; syringeIdx < SYRINGE_IDX_MAX; syringeIdx++)
    {
        syringeFlows[syringeIdx] = listParam[idx++].toDouble();
    }

    powerStatus.batteryLevel = ImrParser::ToCpp_McuBatteryLevel(listParam[idx++]);
    powerStatus.isAcPowered = (listParam[idx++] == _L("AC"));

    // BatteryCharge is set by other (MCU_Monitor)
    //powerStatus.batteryCharge = 0;

    doorState = ImrParser::ToCpp_DoorState(listParam[idx++]);
    wasteBinState = ImrParser::ToCpp_WasteBinState(listParam[idx++]);
    mudsPresent = ImrParser::ToCpp_MudsPresent(listParam[idx++]);
    mudsLatched = ImrParser::ToCpp_MudsLatched(listParam[idx++]);
    mudsInserted = (mudsPresent && mudsLatched); //NON Digest Parameter

    for (int syringeIdx = 0; syringeIdx < SYRINGE_IDX_MAX; syringeIdx++)
    {
        bottleBubbleStates[syringeIdx] = ImrParser::ToCpp_BubbleState(listParam[idx++]);
    }

    sudsInserted = ImrParser::ToCpp_SudsInserted(listParam[idx++]);
    sudsBubbleDetected = ImrParser::ToCpp_SudsbubbleDetected(listParam[idx++]);
    primeBtnPressed = ImrParser::ToCpp_PrimeBtnPressed(listParam[idx++]);
    stopBtnPressed = ImrParser::ToCpp_StopBtnPressed(listParam[idx++]);
    doorBtnPressed = ImrParser::ToCpp_DoorBtnPressed(listParam[idx++]);
    outletDoorState = ImrParser::ToCpp_OutletDoorState(listParam[idx++]);

    for (int devIdx = 0; devIdx < HEAT_MAINTAINER_IDX_MAX; devIdx++)
    {
        heatMaintainerStatus.temperatureReadings[devIdx] = listParam[idx++].toDouble();
    }
    heatMaintainerStatus.state = ImrParser::ToCpp_HeatMaintainerState(listParam[idx++]);

    isShuttingDown = ImrParser::ToCpp_IsShutdown(listParam[idx++]);

    // MCU Diagnostic Event
    QString mcuDiagnosticEvent = listParam[idx++];
    if (mcuDiagnosticEvent != "")
    {
        // get events in reply and reference time
        QStringList newEvents = mcuDiagnosticEvent.split("|");
        QDateTime refTime = QDateTime::fromString(body, MCU_TIMESTAMP_DIGESTSENT_FORMAT);
        int activationPeriodMs = env->ds.capabilities->get_Alert_McuDiagnosticEventTriggerPeriod()* 1000;

        // add each event to list
        for (int i = 0; i < newEvents.size(); ++i)
        {        
            QStringList eventData =  newEvents.at(i).split(":");
            int timeOffsetMs = eventData[0].toInt();
            // calculate timestamp
            eventData[0] = refTime.addMSecs(-timeOffsetMs).toString("hh:mm:ss.zzzz");
            diagnosticEvents.append(eventData.join(":"));
        }

        // Restart the timer to raise alert after time out
        tmrLastDiagnosticEventTimeOut.stop();
        tmrLastDiagnosticEventTimeOut.start(activationPeriodMs);
    }

    //check if need to raise alert
    int maxSize = env->ds.capabilities->get_Alert_McuDiagnosticEventTriggerCount();
    if ( (diagnosticEvents.size() >= maxSize) || (isShuttingDown) )
    {
        slotActivateDiagnosticEventAlert();
    }


    // Write Mcu.log
    while (idx < listParam.length())
    {
        mcuLog->writeDebug(listParam[idx++] + "\n");
    }

    // Save Last Digest
    QVariantMap digestMap;
    digestMap.insert("Digest", reply);

    QFile fileBuf(PATH_LAST_MCU_DIGEST);
    if (fileBuf.open(QFile::WriteOnly | QFile::Text))
    {
        fileBuf.write(Util::qVarientToJsonData(digestMap, false));
        fileBuf.close();
    }

    // Set data
    env->ds.mcuData->setDataLocked(true);
    env->ds.mcuData->setAlarmCodes(alarmCodes);
    env->ds.mcuData->setInjectorStatus(injectorStatus);
    env->ds.mcuData->setPressure(pressure);
    env->ds.mcuData->setStopcockPosAll(stopcockPosAll);
    env->ds.mcuData->setPlungerStates(plungerStates);
    env->ds.mcuData->setSyringeStates(syringeStates);
    env->ds.mcuData->setSyringeVols(syringeVols);
    env->ds.mcuData->setSyringeFlows(syringeFlows);
    env->ds.mcuData->setPowerStatus(powerStatus);
    env->ds.mcuData->setDoorState(doorState);
    env->ds.mcuData->setWasteBinState(wasteBinState);
    env->ds.mcuData->setMudsPresent(mudsPresent);
    env->ds.mcuData->setMudsLatched(mudsLatched);
    env->ds.mcuData->setMudsInserted(mudsInserted); // NON Digest Parameter
    env->ds.mcuData->setBottleBubbleStates(bottleBubbleStates);
    env->ds.mcuData->setSudsInserted(sudsInserted);
    env->ds.mcuData->setSudsBubbleDetected(sudsBubbleDetected);
    env->ds.mcuData->setPrimeBtnPressed(primeBtnPressed);
    env->ds.mcuData->setStopBtnPressed(stopBtnPressed);
    env->ds.mcuData->setDoorBtnPressed(doorBtnPressed);
    env->ds.mcuData->setOutletDoorState(outletDoorState);
    env->ds.mcuData->setHeatMaintainerStatus(heatMaintainerStatus);
    env->ds.mcuData->setIsShuttingDown(isShuttingDown);
    env->ds.mcuData->setDataLocked(false);

    if (env->ds.mcuData->getLinkState() != DS_McuDef::LINK_STATE_CONNECTED)
    {
        // MCU is not connected yet. Don't send the signals. All signals shall be emitted by McuMonitor.
        return errStr;
    }

    if (alarmCodes != prevAlarmCodes)
    {
        emit env->ds.mcuData->signalDataChanged_AlarmCodes(alarmCodes, prevAlarmCodes);
    }
    if (injectorStatus != prevInjectorStatus)
    {
        emit env->ds.mcuData->signalDataChanged_InjectorStatus(injectorStatus, prevInjectorStatus);
    }
    if (pressure != prevPressure)
    {
        emit env->ds.mcuData->signalDataChanged_Pressure(pressure, prevPressure);
    }
    if (stopcockPosAll != prevStopcockPosAll)
    {
        emit env->ds.mcuData->signalDataChanged_StopcockPosAll(stopcockPosAll, prevStopcockPosAll);
    }
    if (plungerStates != prevPlungerStates)
    {
        emit env->ds.mcuData->signalDataChanged_PlungerStates(plungerStates, prevPlungerStates);
    }
    if (syringeStates != prevSyringeStates)
    {
        emit env->ds.mcuData->signalDataChanged_SyringeStates(syringeStates, prevSyringeStates);
    }
    if (syringeVols != prevSyringeVols)
    {
        emit env->ds.mcuData->signalDataChanged_SyringeVols(syringeVols, prevSyringeVols);
    }
    if (syringeFlows != prevSyringeFlows)
    {
        emit env->ds.mcuData->signalDataChanged_SyringeFlows(syringeFlows, prevSyringeFlows);
    }
    if (powerStatus != prevPowerStatus)
    {
        emit env->ds.mcuData->signalDataChanged_PowerStatus(powerStatus, prevPowerStatus);
    }
    if (doorState != prevDoorState)
    {
        emit env->ds.mcuData->signalDataChanged_DoorState(doorState, prevDoorState);
    }
    if (wasteBinState != prevWasteBinState)
    {
        emit env->ds.mcuData->signalDataChanged_WasteBinState(wasteBinState, prevWasteBinState);
    }
    if (mudsPresent != prevMudsPresent)
    {
        emit env->ds.mcuData->signalDataChanged_MudsPresent(mudsPresent, prevMudsPresent);
    }
    if (mudsLatched != prevMudsLatched)
    {
        emit env->ds.mcuData->signalDataChanged_MudsLatched(mudsLatched, prevMudsLatched);
    }
    if (mudsInserted != prevMudsInserted)
    {
        //NON Digest Parameter
        emit env->ds.mcuData->signalDataChanged_MudsInserted(mudsInserted, prevMudsInserted);
    }
    if (bottleBubbleStates != prevBottleBubbleStates)
    {
        emit env->ds.mcuData->signalDataChanged_BottleBubbleStates(bottleBubbleStates, prevBottleBubbleStates);
    }
    if (sudsInserted != prevSudsInserted)
    {
        emit env->ds.mcuData->signalDataChanged_SudsInserted(sudsInserted, prevSudsInserted);
    }
    if (sudsBubbleDetected != prevSudsBubbleDetected)
    {
        emit env->ds.mcuData->signalDataChanged_SudsBubbleDetected(sudsBubbleDetected, prevSudsBubbleDetected);
    }
    if (primeBtnPressed != prevPrimeBtnPressed)
    {
        emit env->ds.mcuData->signalDataChanged_PrimeBtnPressed(primeBtnPressed, prevPrimeBtnPressed);
    }
    if (stopBtnPressed != prevStopBtnPressed)
    {
        emit env->ds.mcuData->signalDataChanged_StopBtnPressed(stopBtnPressed, prevStopBtnPressed);
    }
    if (doorBtnPressed != prevDoorBtnPressed)
    {
        emit env->ds.mcuData->signalDataChanged_DoorBtnPressed(doorBtnPressed, prevDoorBtnPressed);
    }
    if (outletDoorState != prevOutletDoorState)
    {
        emit env->ds.mcuData->signalDataChanged_OutletDoorState(outletDoorState, prevOutletDoorState);
    }
    if (heatMaintainerStatus != prevHeatMaintainerStatus)
    {
        emit env->ds.mcuData->signalDataChanged_HeatMaintainerStatus(heatMaintainerStatus, prevHeatMaintainerStatus);
    }
    if (isShuttingDown != prevIsShuttingDown)
    {
        emit env->ds.mcuData->signalDataChanged_IsShuttingDown(isShuttingDown, prevIsShuttingDown);
    }

    return errStr;
}

QString McuMsgHandler::handleHwDigest(QString reply)
{
    QString errStr = "";
    QVariantMap digestMap;
    QStringList params = reply.split(",");

    if (params.length() >= 22)
    {
        digestMap.insert("MudsLatchState", params[0] == _L("0") ? _L("UNLATCHED") : _L("LATCHED"));
        digestMap.insert("MudsPresence", params[1] != _L("0"));
        digestMap.insert("UsbHostVoltage", params[2]);
        digestMap.insert("BatteryVoltage", params[3]);
        digestMap.insert("MainConnected", params[4] == _L("ON"));
        digestMap.insert("V5Voltage", params[5]);
        digestMap.insert("V12AuxVoltage", params[6]);
        digestMap.insert("V285Voltage", params[7]);
        digestMap.insert("HeatMaintainerCurrent", params[8]);
        digestMap.insert("HcuCurrent", params[9]);
        digestMap.insert("AuxCurrent", params[10]);
        digestMap.insert("UnknownCalibrationCurrent", params[11]);
        digestMap.insert("DoorButtonPressed", params[12] == _L("1"));
        digestMap.insert("SCEngagedSaline", params[13] == _L("1"));
        digestMap.insert("SCEngagedContrast1", params[14] == _L("1"));
        digestMap.insert("SCEngagedContrast2", params[15] == _L("1"));
        digestMap.insert("WastePresence", params[16] == _L("1"));
        digestMap.insert("WasteLevelSensor1", params[17]);
        digestMap.insert("WasteLevelSensor2", params[18]);
        digestMap.insert("InletAirSensorValue1", params[19]);
        digestMap.insert("InletAirSensorValue2", params[20]);
        digestMap.insert("InletAirSensorValue3", params[21]);

        env->ds.mcuData->setDigestMap(digestMap);
    }
    else
    {
        errStr = QString().asprintf("Bad Parameter Count(%d)", (int)params.length());
    }
    return errStr;
}

QString McuMsgHandler::handleBMSDigest(QString reply)
{
    /* Reply Example:
        ;FirmwareVersion:;00 00 00 00 00 00 00 00 00 00 00 00 : 0B 1E 9B 01 03 00 16 00 03 80 00 00 ;
        HardwareVersion:;00 00 00 : 02 AA 00 ;
        Temperature:;0 : 2956;
        MaxError:;0 : 0;
        RelativeStateOfCharge:;0 : 99;
        RunTimeToEmpty:;0 : 65535;
        AveTimeToEmpty:;0 : 65535;
        AveTimeToFull:;0 : 3420;
        BatteryStatus:;0 : 199;
        CycleCount:;0 : 0;
        ManufactureDate:;0 : 19529;
        SerialNumber:;0000 : 0000;
        HostFETControl:;0000 : 0002;
        CellVoltage10:;0 : 3333;
        CellVoltage9:;0 : 3329;
        CellVoltage8:;0 : 3335;
        CellVoltage7:;0 : 3328;
        CellVoltage6:;0 : 3329;
        CellVoltage5:;0 : 3338;
        CellVoltage4:;0 : 3338;
        CellVoltage3:;0 : 3344;
        CellVoltage2:;0 : 3328;
        CellVoltage1:;0 : 3330;
        StateOfHealth:;0 : 96;
        SafetyStatus:;00 00 00 00 00 : 04 00 00 00 00 ;
        OperationStatus:;00 00 00 00 00 : 04 05 01 00 00 ;
        ChargingStatus:;00 00 00 : 02 04 02 ;
        GaugingStatus:;00 00 00 : 02 40 01 ;
        ManufacturingStatus:;00 00 00 : 02 30 00 ;
        PermanentFailStatus:;00 00 00 00 00 : 04 00 00 00 00 ;
        DeviceName:;PACKA_REV0J : PACKA_REV0J
    */

    QString errStr = "";
    QStringList params = reply.split(";");
    QString paramType;
    DS_McuDef::BMSDigests digests = env->ds.mcuData->getBMSDigests();

    for (int batteryIdx = 0; batteryIdx < digests.length(); batteryIdx++)
    {
        digests[batteryIdx].init();
    }

    for (int paramIdx = 0; paramIdx < params.length(); paramIdx++)
    {
        QString param = params[paramIdx];
        if (param.length() == 0)
        {
            continue;
        }

        if (param.endsWith(":"))
        {
            paramType = param;
            paramType.replace(":", "");
            continue;
        }

        QStringList paramVals = param.split(":");
        if (paramVals.length() < POWER_BATTERY_INDEX_MAX)
        {
            // unexpected format
            continue;
        }

        for (int i = 0; i < paramVals.length(); i++)
        {
            paramVals[i] = paramVals[i].trimmed();
        }

        if (paramType.contains("FirmwareVersion"))
        {
            for (int batteryIdx = 0; batteryIdx < digests.length(); batteryIdx++)
            {
                digests[batteryIdx].firmwareVersionStr = paramVals[batteryIdx];
                QStringList strList = digests[batteryIdx].firmwareVersionStr.split(" ");
                if (strList.length() >= 12)
                {
                    digests[batteryIdx].deviceInfo.deviceNumber = strList[1] + strList[2];
                    digests[batteryIdx].deviceInfo.firmwareVersion = strList[3] + strList[4];
                    digests[batteryIdx].deviceInfo.buildNumber = strList[5] + strList[6];
                    digests[batteryIdx].deviceInfo.cedvVersion = strList[8] + strList[9];
                }
            }
        }
        else if (paramType.contains("HardwareVersion"))
        {
            for (int batteryIdx = 0; batteryIdx < digests.length(); batteryIdx++)
            {
                digests[batteryIdx].hardwareVersion = paramVals[batteryIdx];
            }
        }
        else if (paramType.contains("Temperature"))
        {
            for (int batteryIdx = 0; batteryIdx < digests.length(); batteryIdx++)
            {
                // Convert Kelvin to Celsius
                digests[batteryIdx].sbsStatus.temperature = (paramVals[batteryIdx].toUInt() / 10.0) - 273.15;
            }
        }
        else if (paramType.contains("Current"))
        {
            for (int batteryIdx = 0; batteryIdx < digests.length(); batteryIdx++)
            {
                digests[batteryIdx].sbsStatus.current = paramVals[batteryIdx].toInt();
            }
        }
        else if (paramType.contains("MaxError"))
        {
            for (int batteryIdx = 0; batteryIdx < digests.length(); batteryIdx++)
            {
                digests[batteryIdx].sbsStatus.maxError = paramVals[batteryIdx].toUInt();
            }
        }
        else if (paramType.contains("RelativeStateOfCharge"))
        {
            for (int batteryIdx = 0; batteryIdx < digests.length(); batteryIdx++)
            {
                digests[batteryIdx].sbsStatus.relativeStateOfCharge = paramVals[batteryIdx].toUInt();
            }
        }
        else if (paramType.contains("RunTimeToEmpty"))
        {
            for (int batteryIdx = 0; batteryIdx < digests.length(); batteryIdx++)
            {
                digests[batteryIdx].sbsStatus.runTimeToEmpty = paramVals[batteryIdx].toUInt();
            }
        }
        else if (paramType.contains("AveTimeToEmpty"))
        {
            for (int batteryIdx = 0; batteryIdx < digests.length(); batteryIdx++)
            {
                digests[batteryIdx].sbsStatus.aveTimeToEmpty = paramVals[batteryIdx].toUInt();
            }
        }
        else if (paramType.contains("AveTimeToFull"))
        {
            for (int batteryIdx = 0; batteryIdx < digests.length(); batteryIdx++)
            {
                digests[batteryIdx].sbsStatus.aveTimeToFull = paramVals[batteryIdx].toUInt();
            }
        }
        else if (paramType.contains("BatteryStatus"))
        {
            for (int batteryIdx = 0; batteryIdx < digests.length(); batteryIdx++)
            {
                digests[batteryIdx].sbsStatus.batteryStatus = paramVals[batteryIdx].toUInt();

                digests[batteryIdx].sbsStatus.overchargeAlarm = ((digests[batteryIdx].sbsStatus.batteryStatus & 0x8000) == 0x8000);
                digests[batteryIdx].sbsStatus.terminateChargeAlarm = ((digests[batteryIdx].sbsStatus.batteryStatus & 0x4000) == 0x4000);
                digests[batteryIdx].sbsStatus.overTemperatureAlarm = ((digests[batteryIdx].sbsStatus.batteryStatus & 0x1000) == 0x1000);
                digests[batteryIdx].sbsStatus.terminateDischargeAlarm = ((digests[batteryIdx].sbsStatus.batteryStatus & 0x0800) == 0x0800);
                digests[batteryIdx].sbsStatus.fullyCharged = ((digests[batteryIdx].sbsStatus.batteryStatus & 0x0020) == 0x0020);
                digests[batteryIdx].sbsStatus.fullyDischarged = ((digests[batteryIdx].sbsStatus.batteryStatus & 0x0010) == 0x0010);
                digests[batteryIdx].sbsStatus.errorCode = (digests[batteryIdx].sbsStatus.batteryStatus & 0x000F);
            }
        }
        else if (paramType.contains("CycleCount"))
        {
            for (int batteryIdx = 0; batteryIdx < digests.length(); batteryIdx++)
            {
                digests[batteryIdx].sbsStatus.cycleCount = paramVals[batteryIdx].toUInt();
            }
        }
        else if (paramType.contains("ManufactureDate"))
        {
            for (int batteryIdx = 0; batteryIdx < digests.length(); batteryIdx++)
            {
                digests[batteryIdx].sbsStatus.manufactureDate = paramVals[batteryIdx].toUInt();
            }
        }
        else if (paramType.contains("SerialNumber"))
        {
            for (int batteryIdx = 0; batteryIdx < digests.length(); batteryIdx++)
            {
                digests[batteryIdx].sbsStatus.serialNumber = paramVals[batteryIdx].toUInt(NULL, 16);
            }
        }
        else if (paramType.contains("HostFETControl"))
        {
            for (int batteryIdx = 0; batteryIdx < digests.length(); batteryIdx++)
            {
                digests[batteryIdx].sbsStatus.hostFETControl = paramVals[batteryIdx].toUInt(NULL, 16);
            }
        }
        else if (paramType.contains("CellVoltage"))
        {
            QString voltageIdxStr = paramType;
            voltageIdxStr.replace("CellVoltage", "");
            quint8 voltageIdx = voltageIdxStr.toUInt() - 1;
            if (voltageIdx < ARRAY_LEN(digests[0].sbsStatus.cellVoltages))
            {
                for (int batteryIdx = 0; batteryIdx < digests.length(); batteryIdx++)
                {
                    digests[batteryIdx].sbsStatus.cellVoltages[voltageIdx] = paramVals[batteryIdx].toUInt();
                }
            }

        }
        else if (paramType.contains("StateOfHealth"))
        {
            for (int batteryIdx = 0; batteryIdx < digests.length(); batteryIdx++)
            {
                digests[batteryIdx].sbsStatus.stateOfHealth = paramVals[batteryIdx].toUInt();
            }
        }
        else if (paramType.contains("SafetyStatus"))
        {
            LOG_DEBUG("handleBMSDigest(): %s: %s\n", paramType.CSTR(), paramVals.join(",").CSTR());
            for (int batteryIdx = 0; batteryIdx < digests.length(); batteryIdx++)
            {
                QStringList paramValList = paramVals[batteryIdx].split(" ");
                if (paramValList.length() >= 2)
                {
                    int paramValLen = paramValList[0].toUInt(NULL, 16);
                    if (paramValLen >= 4)
                    {
                        int valByte1 = paramValList[1].toUInt(NULL, 16); // bit 0-7
                        int valByte2 = paramValList[2].toUInt(NULL, 16); // bit 8-15
                        int valByte3 = paramValList[3].toUInt(NULL, 16); // bit 16-23
                        int valByte4 = paramValList[4].toUInt(NULL, 16); // bit 24-31

                        digests[batteryIdx].safetyStatus.overcharged = ((valByte3 & 0x10) != 0x00);
                        digests[batteryIdx].safetyStatus.chargeTimeout = ((valByte3 & 0x04) != 0x00);
                        digests[batteryIdx].safetyStatus.prechargeTimeout = ((valByte3 & 0x01) != 0x00);
                        digests[batteryIdx].safetyStatus.underTemperatureDuringDischarge = ((valByte2 & 0x08) != 0x00);
                        digests[batteryIdx].safetyStatus.underTemperatureDuringCharge = ((valByte2 & 0x04) != 0x00);
                        digests[batteryIdx].safetyStatus.overTemperatureDuringDischarge = ((valByte2 & 0x02) != 0x00);
                        digests[batteryIdx].safetyStatus.overTemperatureDuringCharge = ((valByte2 & 0x01) != 0x00);
                        digests[batteryIdx].safetyStatus.shortCircuitDuringDischargeLatch = ((valByte1 & 0x80) != 0x00);
                        digests[batteryIdx].safetyStatus.shortCircuitDuringDischarge = ((valByte1 & 0x40) != 0x00);
                        digests[batteryIdx].safetyStatus.overloadDuringDischargeLatch = ((valByte1 & 0x20) != 0x00);
                        digests[batteryIdx].safetyStatus.overloadDuringDischarge = ((valByte1 & 0x10) != 0x00);
                        digests[batteryIdx].safetyStatus.overCurrentDuringDischarge = ((valByte1 & 0x08) != 0x00);
                        digests[batteryIdx].safetyStatus.overCurrentDuringCharge = ((valByte1 & 0x04) != 0x00);
                        digests[batteryIdx].safetyStatus.cellOverVoltage = ((valByte1 & 0x02) != 0x00);
                        digests[batteryIdx].safetyStatus.cellUnderVoltage = ((valByte1 & 0x01) != 0x00);

                        digests[batteryIdx].safetyStatusBytes = QString().asprintf("%02x %02x %02x %02x", valByte4, valByte3, valByte2, valByte1);
                    }
                }
            }
        }
        else if (paramType.contains("OperationStatus"))
        {
            LOG_DEBUG("handleBMSDigest(): %s: %s\n", paramType.CSTR(), paramVals.join(",").CSTR());
            for (int batteryIdx = 0; batteryIdx < digests.length(); batteryIdx++)
            {
                QStringList paramValList = paramVals[batteryIdx].split(" ");
                if (paramValList.length() >= 2)
                {
                    int paramValLen = paramValList[0].toUInt(NULL, 16);
                    if (paramValLen >= 4)
                    {
                        int valByte1 = paramValList[1].toUInt(NULL, 16); // bit 0-7
                        int valByte2 = paramValList[2].toUInt(NULL, 16); // bit 8-15
                        int valByte3 = paramValList[3].toUInt(NULL, 16); // bit 16-23
                        int valByte4 = paramValList[4].toUInt(NULL, 16); // bit 24-31

                        digests[batteryIdx].operationStatus.cellBalancingStatus = ((valByte4 & 0x10) != 0x00);
                        digests[batteryIdx].operationStatus.ccMeasurementInSleepMode = ((valByte4 & 0x08) != 0x00);
                        digests[batteryIdx].operationStatus.adcMeasurementInSleepMode = ((valByte4 & 0x04) != 0x00);
                        digests[batteryIdx].operationStatus.initializationAfterFullReset = ((valByte4 & 0x01) != 0x00);
                        digests[batteryIdx].operationStatus.sleepMode = ((valByte3 & 0x80) != 0x00);
                        digests[batteryIdx].operationStatus.chargingDisabled = ((valByte2 & 0x40) != 0x00);
                        digests[batteryIdx].operationStatus.dischargingDisabled = ((valByte2 & 0x20) != 0x00);
                        digests[batteryIdx].operationStatus.permanentFailureModeActive = ((valByte2 & 0x10) != 0x00);
                        digests[batteryIdx].operationStatus.safetyModeActive = ((valByte2 & 0x08) != 0x00);
                        digests[batteryIdx].operationStatus.shutdownTriggeredViaLowPackVoltage = ((valByte2 & 0x04) != 0x00);
                        digests[batteryIdx].operationStatus.safePinActive = ((valByte1 & 0x20) != 0x00);
                        digests[batteryIdx].operationStatus.isUnderHostFETControl = ((valByte1 & 0x10) != 0x00);
                        digests[batteryIdx].operationStatus.prechargeFETActive = ((valByte1 & 0x08) != 0x00);
                        digests[batteryIdx].operationStatus.dsgFETActive = ((valByte1 & 0x04) != 0x00);
                        digests[batteryIdx].operationStatus.chgFETActive = ((valByte1 & 0x02) != 0x00);
                        digests[batteryIdx].operationStatus.systemPresentLowActive = ((valByte1 & 0x01) != 0x00);

                        bool sec1 = ((valByte2 & 0x02) != 0x00);
                        bool sec0 = ((valByte2 & 0x01) != 0x00);
                        if (sec1 && sec0)
                        {
                            digests[batteryIdx].operationStatus.securityMode = "SEALED";
                        }
                        else if (sec1)
                        {
                            digests[batteryIdx].operationStatus.securityMode = "UNSEALED";
                        }
                        else if (sec0)
                        {
                            digests[batteryIdx].operationStatus.securityMode = "FULL_ACCESS";
                        }

                        digests[batteryIdx].operationStatusBytes = QString().asprintf("%02x %02x %02x %02x", valByte4, valByte3, valByte2, valByte1);
                    }
                }
            }
        }
        else if (paramType.contains("ChargingStatus"))
        {
            LOG_DEBUG("handleBMSDigest(): %s: %s\n", paramType.CSTR(), paramVals.join(",").CSTR());
            for (int batteryIdx = 0; batteryIdx < digests.length(); batteryIdx++)
            {
                QStringList paramValList = paramVals[batteryIdx].split(" ");
                if (paramValList.length() >= 2)
                {
                    int paramValLen = paramValList[0].toUInt(NULL, 16);
                    if (paramValLen >= 2)
                    {
                        int valByte1 = paramValList[1].toUInt(NULL, 16); // bit 0-7
                        int valByte2 = paramValList[2].toUInt(NULL, 16); // bit 8-15

                        digests[batteryIdx].chargingStatus.overTemperatureRegion =  ((valByte2 & 0x10) != 0x00);
                        digests[batteryIdx].chargingStatus.highTemperatureRegion =  ((valByte2 & 0x08) != 0x00);
                        digests[batteryIdx].chargingStatus.standardTemperatureRegion =  ((valByte2 & 0x04) != 0x00);
                        digests[batteryIdx].chargingStatus.lowTemperatureRegion =  ((valByte2 & 0x02) != 0x00);
                        digests[batteryIdx].chargingStatus.undertemperatureRegion =  ((valByte2 & 0x01) != 0x00);
                        digests[batteryIdx].chargingStatus.chargeTermination =  ((valByte1 & 0x80) != 0x00);
                        digests[batteryIdx].chargingStatus.chargeSuspend =  ((valByte1 & 0x20) != 0x00);
                        digests[batteryIdx].chargingStatus.chargeInhibit =  ((valByte1 & 0x10) != 0x00);

                        digests[batteryIdx].chargingStatusBytes = QString().asprintf("%02x %02x", valByte2, valByte1);
                    }
                }
            }
        }
        else if (paramType.contains("GaugingStatus"))
        {
            LOG_DEBUG("handleBMSDigest(): %s: %s\n", paramType.CSTR(), paramVals.join(",").CSTR());
            for (int batteryIdx = 0; batteryIdx < digests.length(); batteryIdx++)
            {
                QStringList paramValList = paramVals[batteryIdx].split(" ");
                if (paramValList.length() >= 2)
                {
                    int paramValLen = paramValList[0].toUInt(NULL, 16);
                    if (paramValLen >= 2)
                    {
                        int valByte1 = paramValList[1].toUInt(NULL, 16); // bit 0-7
                        int valByte2 = paramValList[2].toUInt(NULL, 16); // bit 8-15

                        digests[batteryIdx].gaugingStatus.dischargeQualifiedForLearning = ((valByte2 & 0x80) != 0x00);
                        digests[batteryIdx].gaugingStatus.endOfDischargeVoltageLevel2 = ((valByte2 & 0x40) != 0x00);
                        digests[batteryIdx].gaugingStatus.endOfDischargeVoltageLevel1 = ((valByte2 & 0x20) != 0x00);
                        digests[batteryIdx].gaugingStatus.ocvReadingTaken = ((valByte2 & 0x01) != 0x00);
                        digests[batteryIdx].gaugingStatus.conditionFlag = ((valByte1 & 0x80) != 0x00);
                        digests[batteryIdx].gaugingStatus.dischargeDetected = ((valByte1 & 0x40) != 0x00);
                        digests[batteryIdx].gaugingStatus.endOfDischargeVoltageLevel0 = ((valByte1 & 0x20) != 0x00);
                        digests[batteryIdx].gaugingStatus.cellBalancingPossible = ((valByte1 & 0x10) != 0x00);
                        digests[batteryIdx].gaugingStatus.terminateCharge = ((valByte1 & 0x08) != 0x00);
                        digests[batteryIdx].gaugingStatus.terminateDischarge = ((valByte1 & 0x04) != 0x00);
                        digests[batteryIdx].gaugingStatus.fullyCharged = ((valByte1 & 0x02) != 0x00);
                        digests[batteryIdx].gaugingStatus.fullyDischarged = ((valByte1 & 0x01) != 0x00);

                        digests[batteryIdx].gaugingStatusBytes = QString().asprintf("%02x %02x", valByte2, valByte1);
                    }
                }
            }
        }
        else if (paramType.contains("ManufacturingStatus"))
        {
            LOG_DEBUG("handleBMSDigest(): %s: %s\n", paramType.CSTR(), paramVals.join(",").CSTR());

            for (int batteryIdx = 0; batteryIdx < digests.length(); batteryIdx++)
            {
                QStringList paramValList = paramVals[batteryIdx].split(" ");
                if (paramValList.length() >= 2)
                {
                    int paramValLen = paramValList[0].toUInt(NULL, 16);
                    if (paramValLen >= 2)
                    {
                        int valByte1 = paramValList[1].toUInt(NULL, 16); // bit 0-7
                        int valByte2 = paramValList[2].toUInt(NULL, 16); // bit 8-15

                        digests[batteryIdx].manufacturingStatus.ledDisplay = ((valByte2 & 0x02) != 0x00);
                        digests[batteryIdx].manufacturingStatus.safeAction = ((valByte2 & 0x01) != 0x00);
                        digests[batteryIdx].manufacturingStatus.blackBoxRecorder = ((valByte1 & 0x80) != 0x00);
                        digests[batteryIdx].manufacturingStatus.permanentFailure = ((valByte1 & 0x40) != 0x00);
                        digests[batteryIdx].manufacturingStatus.lifetimeDataCollection = ((valByte1 & 0x20) != 0x00);
                        digests[batteryIdx].manufacturingStatus.allFetAction = ((valByte1 & 0x10) != 0x00);

                        digests[batteryIdx].manufacturingStatusBytes = QString().asprintf("%02x %02x", valByte2, valByte1);
                    }
                }
            }
        }
        else if (paramType.contains("PermanentFailStatus"))
        {
            LOG_DEBUG("handleBMSDigest(): %s: %s\n", paramType.CSTR(), paramVals.join(",").CSTR());
            for (int batteryIdx = 0; batteryIdx < digests.length(); batteryIdx++)
            {
                QStringList paramValList = paramVals[batteryIdx].split(" ");
                if (paramValList.length() >= 2)
                {
                    int paramValLen = paramValList[0].toUInt(NULL, 16);
                    if (paramValLen >= 4)
                    {
                        int valByte1 = paramValList[1].toUInt(NULL, 16); // bit 0-7
                        int valByte2 = paramValList[2].toUInt(NULL, 16); // bit 8-15
                        int valByte3 = paramValList[3].toUInt(NULL, 16); // bit 16-23
                        int valByte4 = paramValList[4].toUInt(NULL, 16); // bit 24-31

                        digests[batteryIdx].pfStatus.dataFlashWearoutFailure = ((valByte3 & 0x02) != 0x00);
                        digests[batteryIdx].pfStatus.instructionFlashCehcksumFailure = ((valByte3 & 0x01) != 0x00);
                        digests[batteryIdx].pfStatus.safetyOvertemperatureFETFailure = ((valByte2 & 0x80) != 0x00);
                        digests[batteryIdx].pfStatus.openThermistorTS3Failure = ((valByte2 & 0x40) != 0x00);
                        digests[batteryIdx].pfStatus.openThermistorTS2Failure = ((valByte2 & 0x20) != 0x00);
                        digests[batteryIdx].pfStatus.openThermistorTS1Failure = ((valByte2 & 0x10) != 0x00);
                        digests[batteryIdx].pfStatus.companionAfeXReadyFailure = ((valByte2 & 0x08) != 0x00);
                        digests[batteryIdx].pfStatus.companionAfeOveride = ((valByte2 & 0x04) != 0x00);
                        digests[batteryIdx].pfStatus.afeCommunicationFailure = ((valByte2 & 0x02) != 0x00);
                        digests[batteryIdx].pfStatus.afeRegisterFailure = ((valByte2 & 0x01) != 0x00);
                        digests[batteryIdx].pfStatus.dischargeFETFailure = ((valByte1 & 0x80) != 0x00);
                        digests[batteryIdx].pfStatus.chargeFETFailure = ((valByte1 & 0x40) != 0x00);
                        digests[batteryIdx].pfStatus.voltageImbalanceWhilePackRestFailure = ((valByte1 & 0x20) != 0x00);
                        digests[batteryIdx].pfStatus.safetyOvertemperatureCellFailure = ((valByte1 & 0x10) != 0x00);
                        digests[batteryIdx].pfStatus.safetyOvercurrentInDischarge = ((valByte1 & 0x08) != 0x00);
                        digests[batteryIdx].pfStatus.safetyOvercurrentInCharge = ((valByte1 & 0x04) != 0x00);
                        digests[batteryIdx].pfStatus.safetyCellOvervoltageFailure = ((valByte1 & 0x02) != 0x00);
                        digests[batteryIdx].pfStatus.safetyCellUndervoltageFailure = ((valByte1 & 0x01) != 0x00);

                        digests[batteryIdx].pfStatusBytes = QString().asprintf("%02x %02x %02x %02x", valByte4, valByte3, valByte2, valByte1);
                    }
                }
            }
        }
        else if (paramType.contains("DeviceName"))
        {
            LOG_DEBUG("handleBMSDigest(): %s: %s\n", paramType.CSTR(), paramVals.join(",").CSTR());

            for (int batteryIdx = 0; batteryIdx < digests.length(); batteryIdx++)
            {
                digests[batteryIdx].deviceInfo.deviceName = (paramVals[batteryIdx].CSTR());
            }
        }
    }

    if (errStr == "")
    {
        env->ds.mcuData->setBMSDigests(digests);

        QFile fileBuf(PATH_LAST_BMS_DIGESTS);
        if (fileBuf.open(QFile::WriteOnly | QFile::Text))
        {
            QVariantList bmsDigestsJson = ImrParser::ToImr_BMSDigests(digests);
            fileBuf.write(Util::qVarientToJsonData(bmsDigestsJson, false));
            fileBuf.close();
        }
    }

    return errStr;
}

QString McuMsgHandler::handleVersion(QString reply)
{
    QString errStr = "";
    QStringList params = reply.split(",");
    if (params.length() >= 3)
    {
        env->ds.mcuData->setMcuVersion(params[0]);
        env->ds.mcuData->setStopcockVersion(params[1]);
        env->ds.mcuData->setMcuCommandVersion(params[2]);
    }
    else
    {
        errStr = QString().asprintf("Bad Parameter Count(%d)", (int)params.length());
    }
    return errStr;
}

QString McuMsgHandler::handleInfo(QString reply)
{
    QString errStr = "";
    QStringList params = reply.split(",");
    if (params.length() >= 10)
    {
        env->ds.mcuData->setMcuSerialNumber(params[6]);

        QStringList motorModuleSerialNumbers;
        motorModuleSerialNumbers.append(params[7]);
        motorModuleSerialNumbers.append(params[8]);
        motorModuleSerialNumbers.append(params[9]);
        env->ds.mcuData->setMotorModuleSerialNumbers(motorModuleSerialNumbers);
    }
    else
    {
        errStr = QString().asprintf("Bad Parameter Count(%d)", (int)params.length());
    }
    return errStr;
}

QString McuMsgHandler::handleInjectDigest(QString reply)
{
    QString errStr = "";
    QStringList params = reply.split(",");
    int paramIdx = 0;
    int paramsForAllPhase = 0;
    int paramsRequired = 36;

    if (env->ds.capabilities->get_Logging_McuInjectDigest())
    {
        mcuInjectDigestLog->writeDebug(reply + "\n");
    }

    DS_McuDef::InjectDigest injectDigest = env->ds.mcuData->getInjectDigest();

    injectDigest.phaseIdx = params[paramIdx++].toInt();
    injectDigest.adaptiveFlowState = ImrParser::ToCpp_McuAdaptiveFlowState(params[paramIdx++]);
    injectDigest.scheduledPulsingActive = params[paramIdx++].toInt() ? 1 : 0;
    injectDigest.unscheduledPulsingActive = params[paramIdx++].toInt() ? 1 : 0;

    injectDigest.injectionPressure = params[paramIdx++].toInt();

    for (int syringeIdx = 0; syringeIdx < SYRINGE_IDX_MAX; syringeIdx++)
    {
        injectDigest.syringeInjectDigests[syringeIdx].pressure = params[paramIdx++].toInt();
    }

    for (int syringeIdx = 0; syringeIdx < SYRINGE_IDX_MAX; syringeIdx++)
    {
        injectDigest.syringeInjectDigests[syringeIdx].motorPid = params[paramIdx++].toInt();
    }

    for (int syringeIdx = 0; syringeIdx < SYRINGE_IDX_MAX; syringeIdx++)
    {
        injectDigest.syringeInjectDigests[syringeIdx].scPos = ImrParser::ToCpp_StopcockPos(params[paramIdx++]);
    }

    for (int syringeIdx = 0; syringeIdx < SYRINGE_IDX_MAX; syringeIdx++)
    {
        injectDigest.syringeInjectDigests[syringeIdx].motorPos = params[paramIdx++].toInt();
    }

    for (int syringeIdx = 0; syringeIdx < SYRINGE_IDX_MAX; syringeIdx++)
    {
        injectDigest.syringeInjectDigests[syringeIdx].slowStartReduction = params[paramIdx++].toDouble();
    }

    for (int syringeIdx = 0; syringeIdx < SYRINGE_IDX_MAX; syringeIdx++)
    {
        injectDigest.syringeInjectDigests[syringeIdx].storedCompliance = params[paramIdx++].toDouble();
    }

    for (int syringeIdx = 0; syringeIdx < SYRINGE_IDX_MAX; syringeIdx++)
    {
        injectDigest.syringeInjectDigests[syringeIdx].phaseCompliance = params[paramIdx++].toDouble();
    }

    injectDigest.patientLineAirCounts = params[paramIdx++].toInt();
    injectDigest.adcPinReading120 = params[paramIdx++].toInt();
    injectDigest.adcPinReading121 = params[paramIdx++].toInt();
    injectDigest.adcPinReading122 = params[paramIdx++].toInt();
    injectDigest.pressureMonitorPortReading = params[paramIdx++].toInt();

    for (int syringeIdx = 0; syringeIdx < SYRINGE_IDX_MAX; syringeIdx++)
    {
        double flowRate = params[paramIdx++].toDouble();
        DS_McuDef::StopcockPos scPos = injectDigest.syringeInjectDigests[syringeIdx].scPos;

        if (scPos == DS_McuDef::STOPCOCK_POS_CLOSED)
        {
            injectDigest.syringeInjectDigests[syringeIdx].flowRate = 0;
        }
        else
        {
            injectDigest.syringeInjectDigests[syringeIdx].flowRate = flowRate;
        }
    }

    for (int syringeIdx = 0; syringeIdx < SYRINGE_IDX_MAX; syringeIdx++)
    {
        injectDigest.syringeInjectDigests[syringeIdx].volPushed = params[paramIdx++].toDouble();
    }

    if ( (injectDigest.phaseIdx < 0) ||
         (injectDigest.phaseIdx >= MAX_MCU_PHASES) )
    {
        errStr = QString().asprintf("Bad Inject Digest: Invalid PhaseIdx (%d): Msg=%s\n", injectDigest.phaseIdx, reply.CSTR());
        goto bail;
    }

    paramsForAllPhase = (injectDigest.phaseIdx + 1) * MCU_INJECT_DIGEST_PHASE_NUM_PARAMS;
    paramsRequired = paramIdx + paramsForAllPhase;

    if (params.length() < paramsRequired)
    {
        errStr = QString().asprintf("Bad Inject Digest: Invalid Params (len=%d<%d): Msg=%s\n", (int)params.length(), paramsRequired, reply.CSTR());
        goto bail;
    }

    for (int phaseIdx = 0; phaseIdx < injectDigest.phaseInjectDigests.length(); phaseIdx++)
    {
        if (phaseIdx <= injectDigest.phaseIdx)
        {
            DS_McuDef::PhaseInjectDigest *phaseInjDigest = &injectDigest.phaseInjectDigests[phaseIdx];
            phaseInjDigest->volumes[SYRINGE_IDX_SALINE] = params[paramIdx++].toDouble();
            phaseInjDigest->volumes[SYRINGE_IDX_CONTRAST1] = params[paramIdx++].toDouble();
            phaseInjDigest->volumes[SYRINGE_IDX_CONTRAST2] = params[paramIdx++].toDouble();
            phaseInjDigest->duration = params[paramIdx++].toDouble();

            if ( (!phaseInjDigest->pulsingActivated) &&
                 ( (injectDigest.scheduledPulsingActive) || (injectDigest.unscheduledPulsingActive) ) )
            {
                LOG_INFO("handleInjectDigest: Phase[%d]: PulsingActivated = true\n", phaseIdx);
                phaseInjDigest->pulsingActivated = true;
            }
        }
    }
    env->ds.mcuData->setInjectDigest(injectDigest);

bail:
    if (errStr != "")
    {
        LOG_ERROR("%s", errStr.CSTR());
        env->ds.alertAction->activate("HCUInternalSoftwareError", errStr);
    }
    return errStr;
}

QString McuMsgHandler::handleGetSyringeAirVol(QString body, QString reply)
{
    SyringeIdx syringeIdx = (SyringeIdx)body.toInt();

    if ( (syringeIdx != SYRINGE_IDX_SALINE) &&
         (syringeIdx != SYRINGE_IDX_CONTRAST1) &&
         (syringeIdx != SYRINGE_IDX_CONTRAST2) )
    {
        return QString().asprintf("Bad parameters. SyringeIdx=%d\n", syringeIdx);
    }

    QStringList params = reply.split(",");
    int paramIdx = 0;

    DS_McuDef::SyringeAirCheckDigests syringeAirCheckDigests = env->ds.mcuData->getSyringeAirCheckDigests();
    syringeAirCheckDigests[syringeIdx].airVolume = params[paramIdx++].toDouble();
    syringeAirCheckDigests[syringeIdx].airVolume2 = params[paramIdx++].toDouble();

    env->ds.mcuData->setSyringeAirCheckDigests(syringeAirCheckDigests);

    return "";
}

QString McuMsgHandler::handleGetSyringeAirCheckData(QString body, QString reply)
{
    SyringeIdx syringeIdx = (SyringeIdx)body.toInt();

    if ( (syringeIdx != SYRINGE_IDX_SALINE) &&
         (syringeIdx != SYRINGE_IDX_CONTRAST1) &&
         (syringeIdx != SYRINGE_IDX_CONTRAST2) )
    {
        return QString().asprintf("Bad parameters. SyringeIdx=%d\n", syringeIdx);
    }

    QStringList params = reply.split(",");
    int paramIdx = 0;

    // TODO S2SRUSW-3379 This is really a list of displacements and pressures, with the first value
    // equal to the data length. Remove the code regarding this function, or implement as per the ticket.

    DS_McuDef::SyringeAirCheckCalDigests syringeAirCheckCalDigests = env->ds.mcuData->getSyringeAirCheckCalDigests();
    syringeAirCheckCalDigests[syringeIdx].displacement = params[paramIdx++].toInt();
    syringeAirCheckCalDigests[syringeIdx].pressureKpa = params[paramIdx++].toInt();

    env->ds.mcuData->setSyringeAirCheckCalDigests(syringeAirCheckCalDigests);

    return "";
}

QString McuMsgHandler::handleGetSyringeAirCheckCoeff(QString body, QString reply)
{
    SyringeIdx syringeIdx = (SyringeIdx)body.toInt();

    if ( (syringeIdx != SYRINGE_IDX_SALINE) &&
         (syringeIdx != SYRINGE_IDX_CONTRAST1) &&
         (syringeIdx != SYRINGE_IDX_CONTRAST2) )
    {
        return QString().asprintf("Bad parameters. SyringeIdx=%d\n", syringeIdx);
    }

    QStringList params = reply.split(",");
    int paramIdx = 0;

    DS_McuDef::SyringeAirCheckCoeffDigests syringeAirCheckCoeffDigests = env->ds.mcuData->getSyringeAirCheckCoeffDigests();
    syringeAirCheckCoeffDigests[syringeIdx].slope = params[paramIdx++].toDouble();
    syringeAirCheckCoeffDigests[syringeIdx].intercept = params[paramIdx++].toDouble();
    syringeAirCheckCoeffDigests[syringeIdx].state = params[paramIdx++];

    env->ds.mcuData->setSyringeAirCheckCoeffDigests(syringeAirCheckCoeffDigests);

    return "";
}

QString McuMsgHandler::handleGetPressureCalCoeff(QString body, QString reply)
{
    SyringeIdx syringeIdx = (SyringeIdx)body.toInt();

    if ( (syringeIdx != SYRINGE_IDX_SALINE) &&
         (syringeIdx != SYRINGE_IDX_CONTRAST1) &&
         (syringeIdx != SYRINGE_IDX_CONTRAST2) )
    {
        return QString().asprintf("handleGetPressureCalCoeff: Bad parameters. SyringeIdx=%d\n", syringeIdx);
    }

    QStringList params = reply.split(",");
    int paramIdx = 0;

    // Check reply length since this data is just used for diagnostic alert purposes
    if (params.length() != 7)
    {
        return QString().asprintf("handleGetPressureCalCoeff: Bad reply. Expected 7 params, recieved %d\n", (int)params.length());
    }

    DS_McuDef::PressureCalCoeffDigests pressureCalCoeffDigests = env->ds.mcuData->getPressureCalCoeffDigests();
    pressureCalCoeffDigests[syringeIdx].coeff0 = params[paramIdx++].toDouble();
    pressureCalCoeffDigests[syringeIdx].coeff1 = params[paramIdx++].toDouble();
    pressureCalCoeffDigests[syringeIdx].coeff2 = params[paramIdx++].toDouble();
    pressureCalCoeffDigests[syringeIdx].coeff3 = params[paramIdx++].toDouble();
    pressureCalCoeffDigests[syringeIdx].coeff4 = params[paramIdx++].toDouble();
    pressureCalCoeffDigests[syringeIdx].coeff5 = params[paramIdx++].toDouble();
    pressureCalCoeffDigests[syringeIdx].state = params[paramIdx++];

    env->ds.mcuData->setPressureCalCoeffDigests(pressureCalCoeffDigests);

    return "";
}

QString McuMsgHandler::handleGetPlungerFriction(QString body, QString reply)
{
    if (body != "all")
    {
        return QString().asprintf("handleGetPressureCalCoeff: Bad parameters: %s\n", body.CSTR());
    }

    QStringList params = reply.split(",");
    int paramIdx = 0;

    // Check reply length since this data is just used for diagnostic alert purposes
    if (params.length() != 18)
    {
        return QString().asprintf("handleGetPressureCalCoeff: Bad reply. Expected 18 params, recieved %d\n", (int)params.length());
    }

    // Calculate friction (lbf) per PID value. Formula is as follows:
    // Pressure ADC = A + B*X + C*Y + D*X*Y + E*X*X + F*Y*Y, where X = Flow Rate (m/s) * 30, Y = PID / 1000, and A-E are pressure cal coeffs
    // Friction (pounds force) = Pressure ADC * 0.630303 (makes Pressure KPA) * 0.145038 (makes Pressure PSI) * 2.6706 (makes Pressure friction)
    double x = 300; // purge happens at 10ml/s
    DS_McuDef::PressureCalCoeffDigests pressureCalCoeffDigests = env->ds.mcuData->getPressureCalCoeffDigests();
    for (int syringeIdx = 0; syringeIdx < SYRINGE_IDX_MAX; syringeIdx++)
    {
        DS_McuDef::PressureCalCoeff pressureCalCoeff = pressureCalCoeffDigests[syringeIdx];

        // Set up variables for our alert data
        QList<QVariant> pidValues;
        QList<QVariant> frictionValues;

        for (int pidIdx = 0; pidIdx < 6; pidIdx++)
        {
            double pid = params[paramIdx++].toDouble();
            pidValues.append(pid);

            double y = pid / 1000;
            double pressureAdc = pressureCalCoeff.coeff0 +
                                 (pressureCalCoeff.coeff1 * x) +
                                 (pressureCalCoeff.coeff2 * y) +
                                 (pressureCalCoeff.coeff3 * x * y) +
                                 (pressureCalCoeff.coeff4 * x * x) +
                                 (pressureCalCoeff.coeff5 * y * y);
            frictionValues.append(pressureAdc * 0.630303 * 0.145038 * 2.6706);
        }

        // Each param represents a PID value, with the first and fourth element representing the average PID. We get
        // the difference of those averages (after converting to lbf) here.
        double avgDiff = frictionValues[0].toDouble() - frictionValues[3].toDouble();

        QVariantMap jsonMap;
        jsonMap.insert("Location", ImrParser::ToImr_FluidSourceSyringeIdx((SyringeIdx)syringeIdx));
        jsonMap.insert("AverageFrictionsDiff", avgDiff);
        jsonMap.insert("Frictions", frictionValues);
        jsonMap.insert("Pids", pidValues);
        jsonMap.insert("PressureCalCoeff", ImrParser::ToImr_PressureCalCoeff(pressureCalCoeff));
        QString alertData = Util::qVarientToJsonData(jsonMap);

        env->ds.alertAction->activate("SRUPlungerFriction", alertData);
    }

    return "";
}


void McuMsgHandler::slotActivateDiagnosticEventAlert()
{
    tmrLastDiagnosticEventTimeOut.stop();
    if (!diagnosticEvents.isEmpty())
    {
        QString alertData = diagnosticEvents.join(" | ");
        LOG_INFO("[DIGEST] MCU DIAGNOSTIC EVENT = %s\n", alertData.CSTR());
        env->ds.alertAction->activate("MCUDiagnosticEventOccurred", alertData);
        diagnosticEvents.clear();
    }
}


