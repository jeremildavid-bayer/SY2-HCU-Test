#include "Apps/AppManager.h"
#include "McuPressureCalibration.h"
#include "Common/Util.h"
#include "Common/ImrParser.h"
#include "DataServices/Mcu/DS_McuAction.h"
#include "DataServices/Mcu/DS_McuData.h"
#include "DataServices/Alert/DS_AlertAction.h"
#include "DataServices/Capabilities/DS_Capabilities.h"
#include "DataServices/HardwareInfo/DS_HardwareInfo.h"

McuPressureCalibration::McuPressureCalibration(QObject *parent, EnvGlobal *env_) :
    ActionBaseExt(parent, env_)
{
    envLocal = new EnvLocal("DS_Mcu-PressureCal", "MCU_PRESSURE_CAL");

    connect(&tmrCaptureData, SIGNAL(timeout()), this, SLOT(slotCaptureData()));

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });

    syringeIdx = SYRINGE_IDX_NONE;
}

McuPressureCalibration::~McuPressureCalibration()
{
    delete envLocal;
}

void McuPressureCalibration::slotAppInitialised()
{
    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_SyringeVols, this, [=](const QList<double> &vols) {
        DS_McuDef::PressureCalibrationState state = (DS_McuDef::PressureCalibrationState)getState();

        if (state != DS_McuDef::PRESSURE_CAL_STATE_FIND_PLUNGER_PROGRESS)
        {
            return;
        }

        // Check the plunger is travelled within limit until engaged
        DS_McuDef::PressureCalibrationStatus calStatus = env->ds.mcuData->getPressureCalibrationStatus();
        double syringeVol = vols[syringeIdx];
        double maxTravelAllowed = env->ds.capabilities->get_Calibration_PressureCalMaxTravelUntilEngaged();
        double traveledUntilEngaged = calStatus.homePosition - syringeVol;
        if (traveledUntilEngaged > maxTravelAllowed)
        {
            LOG_ERROR("signalDataChanged_SyringeVols(): Syringe[%d]: SyringeTraveledUntilEngaged = (%.1fml - %.1fml) = %.1fml: > MaxAllowed(%.1fml)\n",
                      calStatus.injectStageIdx, calStatus.homePosition, syringeVol, traveledUntilEngaged, maxTravelAllowed);
            calStatus.err = "Plunger Not Found";
            env->ds.mcuData->setPressureCalibrationStatus(calStatus);
            env->actionMgr->deleteActAll(guidSubAction);
            setState(DS_McuDef::PRESSURE_CAL_STATE_FIND_PLUNGER_FAILED);
        }
    });

    setStateSynch(DS_McuDef::PRESSURE_CAL_STATE_IDLE);
    processState();
}

void McuPressureCalibration::calibrationDataAdd(double flow, double pid, double pressureCalAdc)
{
    // Divided by 1000 to prevent overflow of matrix values
    double adjustedPid = pid / 1000;

    Matrix tempColumn(MATRIX_N, 1);
    tempColumn(0 ,0) = pressureCalAdc;
    tempColumn(1 ,0) = flow * pressureCalAdc;
    tempColumn(2 ,0) = adjustedPid * pressureCalAdc;
    tempColumn(3 ,0) = flow * adjustedPid * pressureCalAdc;
    tempColumn(4 ,0) = flow * flow * pressureCalAdc;
    tempColumn(5 ,0) = adjustedPid * adjustedPid * pressureCalAdc;

    Matrix tempSquare(MATRIX_N, MATRIX_N);
    tempSquare(0 ,0) = 1; // N (Number of data points)
    tempSquare(0 ,1) = flow;
    tempSquare(0 ,2) = adjustedPid;
    tempSquare(0 ,3) = flow * adjustedPid;
    tempSquare(0 ,4) = flow * flow;
    tempSquare(0 ,5) = adjustedPid * adjustedPid;

    tempSquare(1 ,0) = flow;
    tempSquare(1 ,1) = flow * flow;
    tempSquare(1 ,2) = flow * adjustedPid;
    tempSquare(1 ,3) = flow * flow * adjustedPid;
    tempSquare(1 ,4) = flow * flow * flow;
    tempSquare(1 ,5) = flow * adjustedPid * adjustedPid;

    tempSquare(2 ,0) = adjustedPid;
    tempSquare(2 ,1) = adjustedPid * flow;
    tempSquare(2 ,2) = adjustedPid * adjustedPid;
    tempSquare(2 ,3) = adjustedPid * flow * adjustedPid;
    tempSquare(2 ,4) = adjustedPid * flow * flow;
    tempSquare(2 ,5) = adjustedPid * adjustedPid * adjustedPid;

    tempSquare(3 ,0) = flow * adjustedPid;
    tempSquare(3 ,1) = flow * adjustedPid * flow;
    tempSquare(3 ,2) = flow * adjustedPid * adjustedPid;
    tempSquare(3 ,3) = flow * adjustedPid * flow * adjustedPid;
    tempSquare(3 ,4) = flow * adjustedPid * flow * flow;
    tempSquare(3 ,5) = flow * adjustedPid * adjustedPid * adjustedPid;

    tempSquare(4 ,0) = flow * flow;
    tempSquare(4 ,1) = flow * flow * flow;
    tempSquare(4 ,2) = flow * flow * adjustedPid;
    tempSquare(4 ,3) = flow * flow * flow * adjustedPid;
    tempSquare(4 ,4) = flow * flow * flow * flow;
    tempSquare(4 ,5) = flow * flow * adjustedPid * adjustedPid;

    tempSquare(5 ,0) = adjustedPid * adjustedPid;
    tempSquare(5 ,1) = adjustedPid * adjustedPid * flow;
    tempSquare(5 ,2) = adjustedPid * adjustedPid * adjustedPid;
    tempSquare(5 ,3) = adjustedPid * adjustedPid * flow * adjustedPid;
    tempSquare(5 ,4) = adjustedPid * adjustedPid * flow * flow;
    tempSquare(5 ,5) = adjustedPid * adjustedPid * adjustedPid * adjustedPid;

    columnMatrix = columnMatrix + tempColumn;
    squareMatrix = squareMatrix + tempSquare;
}

bool McuPressureCalibration::calibrationStart(SyringeIdx idx)
{
    DS_McuDef::PressureCalibrationState state = (DS_McuDef::PressureCalibrationState)getState();

    if (state != DS_McuDef::PRESSURE_CAL_STATE_IDLE)
    {
        LOG_ERROR("calibrationStart(): Failed to start. Invalid curState=%s, expected=%s\n", ImrParser::ToImr_PressureCalibrationState(state).CSTR(), ImrParser::ToImr_PressureCalibrationState(DS_McuDef::PRESSURE_CAL_STATE_IDLE).CSTR());
        return false;
    }

    if ( (idx < 0) || (idx >= SYRINGE_IDX_MAX) )
    {
        LOG_ERROR("calibrationStart(): Failed to start. Incorrect idx(%d) received\n", idx);
        return false;
    }

    // Reset Calibration Data
    syringeIdx = idx;
    calibrationDataReset();

    actStatusBuf.state = DS_ACTION_STATE_STARTED;
    actStatusBuf.err = "";

    setState(DS_McuDef::PRESSURE_CAL_STATE_STARTED);
    return true;
}

void McuPressureCalibration::calibrationStop()
{
    tmrCaptureData.stop();

    QString guid = Util::newGuid();

    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            env->actionMgr->onActionCompleted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
                DS_McuDef::PressureCalibrationState state = (DS_McuDef::PressureCalibrationState)getState();

                if (state == DS_McuDef::PRESSURE_CAL_STATE_FAILED)
                {
                    // Already failed state, User Abort is already handled by other.
                    return;
                }

                if (curStatus.state == DS_ACTION_STATE_COMPLETED)
                {
                    if (actStatusBuf.err == "")
                    {
                        actStatusBuf.state = DS_ACTION_STATE_USER_ABORT;
                        actStatusBuf.err = "User Aborted";
                    }
                    setState(DS_McuDef::PRESSURE_CAL_STATE_FAILED);
                }
                else
                {
                    LOG_WARNING("calibrationStop(): ACTION_STATUS: %s\n", ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());
                    QTimer::singleShot(1000, this, [=] {
                        if (actStatusBuf.err == "")
                        {
                            actStatusBuf.state = DS_ACTION_STATE_USER_ABORT;
                            actStatusBuf.err = "User Aborted";
                        }
                        setState(DS_McuDef::PRESSURE_CAL_STATE_FAILED);
                    });
                }
            });
        }
        else
        {
            LOG_WARNING("calibrationStop(): ACTION_STATUS: %s\n", ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());
            QTimer::singleShot(1000, this, [=] {
                if (actStatusBuf.err == "")
                {
                    actStatusBuf.state = DS_ACTION_STATE_USER_ABORT;
                    actStatusBuf.err = "User Aborted";
                }
                setState(DS_McuDef::PRESSURE_CAL_STATE_FAILED);
            });
        }
    });

    env->ds.mcuAction->actStopAll(guid);
}

void McuPressureCalibration::calibrationDataReset()
{
    // Clear Matrixes
    squareMatrix = Matrix(MATRIX_N, MATRIX_N);
    columnMatrix = Matrix(MATRIX_N, 1);
}

void McuPressureCalibration::slotCaptureData()
{
    tmrCaptureData.stop();

    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        if (curStatus.state != DS_ACTION_STATE_STARTED)
        {
            LOG_WARNING("slotCaptureData(): Action Failed Start: %s\n", ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());
            return;
        }

        env->actionMgr->onActionCompleted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus status) {
            LOG_DEBUG("slotCaptureData(): RX: '%s'\n", status.reply.CSTR());
            QStringList lstReply = status.reply.split(",");
            if (lstReply.length() < 3)
            {
                LOG_WARNING("slotCaptureData(): Incorrect args(%d) received: Reply=%s\n", (int)lstReply.length(), status.reply.CSTR());
                return;
            }

            double flow = lstReply.at(0).toDouble();
            double pid = lstReply.at(1).toDouble();
            double pressureCalAdc = lstReply.at(2).toDouble();
            //double motorCurrent = lstReply.at(3).toDouble();

            DS_McuDef::PressureCalibrationStatus calStatus = env->ds.mcuData->getPressureCalibrationStatus();
            calStatus.adcReadValue = pressureCalAdc;
            env->ds.mcuData->setPressureCalibrationStatus(calStatus);

            bool dataCaptureStartOk = true;
            double adcValueRangeMin = env->ds.capabilities->get_Calibration_PressureCalADCRangeMin();
            double adcValueRangeMax = env->ds.capabilities->get_Calibration_PressureCalADCRangeMax();

            double syringeVol = env->ds.mcuData->getSyringeVols()[syringeIdx];

            // Check if Data is ok (flow rate > 0) && (ADC_Min <= ADC <= ADC_Max)
            // If Data is ok, add it to the coefficient matrix
            if (flow <= 0)
            {
                LOG_INFO("slotCaptureData(): Flow Rate Too Small (%.1f <= 0): Skipping Data\n", flow);
            }
            else if (Util::isFloatVarGreaterThan(adcValueRangeMin, pressureCalAdc))
            {
                // Skip data until within range
                LOG_INFO("slotCaptureData(): ADC Value Too Small (%.1f < %.1f): Skipping Data\n", pressureCalAdc, adcValueRangeMin);
            }
            else if (Util::isFloatVarGreaterThan(pressureCalAdc, adcValueRangeMax))
            {
                // ADC value became out of range. Stop Data Capture
                LOG_WARNING("slotCaptureData(): ADC Value Too Big (%.1f > %.1f): Stopping Data Capture\n", pressureCalAdc, adcValueRangeMax);
                dataCaptureStartOk = false;
            }
            else
            {
                // Data is ok, add the ADC value to the coefficient matrixes
                calibrationDataAdd(flow, pid, pressureCalAdc);
            }

            if (calStatus.firstGoodAdcValuePosition == -1)
            {
                double maxTravelUntilDataReady = env->ds.capabilities->get_Calibration_PressureCalMaxTravelUntilDataReady();
                double traveledAfterEngaged = calStatus.engagedPosition - syringeVol;

                if (pressureCalAdc >= adcValueRangeMin)
                {
                    LOG_INFO("slotCaptureData(): First Good ADC Value found: %.1fml(>=%.1f), Traveled=%.1f(<%.1fml)\n", pressureCalAdc, adcValueRangeMin, traveledAfterEngaged, maxTravelUntilDataReady);
                    calStatus.firstGoodAdcValuePosition = syringeVol;
                }
                else
                {
                    // Check if the ADC is not reached within PressureCalMaxTravelUntilDataReady value.
                    if (Util::isFloatVarGreaterThan(traveledAfterEngaged, maxTravelUntilDataReady))
                    {
                        LOG_ERROR("slotCaptureData(): ADC value reached to %.1f(<%.1f) while travelled for %.1fml(>%.1fml)\n", pressureCalAdc, adcValueRangeMin, traveledAfterEngaged, maxTravelUntilDataReady);
                        calStatus.err = "Signal Not Detected";
                        env->ds.mcuData->setPressureCalibrationStatus(calStatus);
                        calibrationStop();
                        dataCaptureStartOk = false;
                    }
                }
            }

            // If flow value is ok, add it to the Moving Mean buffer.
            // Note, it is still ok to add BAD ADC value to the Moving Mean buffer in order to make sure all samples are collected.
            if (flow > 0)
            {
                // Perform Data Sanity Check - Ensure the Moving Average value of ADC readings are increasing
                calStatus.dataCheckInfo.adcValuesSum += pressureCalAdc;
                calStatus.dataCheckInfo.sampleIdx++;

                int sampleSize = env->ds.capabilities->get_Calibration_PressureCalMovingMeanSampleSize();

                if (calStatus.dataCheckInfo.sampleIdx >= sampleSize)
                {
                    double curMovingAverage = calStatus.dataCheckInfo.adcValuesSum / calStatus.dataCheckInfo.sampleIdx;
                    if (Util::isFloatVarGreaterThan(curMovingAverage, calStatus.dataCheckInfo.lastMovingAverage))
                    {
                        LOG_INFO("slotCaptureData(): Moving Average (sampleN=%d) = %.1f to %.1f\n", sampleSize, calStatus.dataCheckInfo.lastMovingAverage, curMovingAverage);
                        calStatus.dataCheckInfo.init();
                        calStatus.dataCheckInfo.lastMovingAverage = curMovingAverage;
                    }
                    else if (calStatus.firstGoodAdcValuePosition != -1)
                    {
                        // Only report "Bad Data" after we have encountered a good ADV value
                        // so that the "Signal Not Detected" error is properly reported

                        LOG_ERROR("slotCaptureData(): Moving Average (sampleN=%d) = %.1f to %.1f: Bad Data Change\n", sampleSize, calStatus.dataCheckInfo.lastMovingAverage, curMovingAverage);
                        calStatus.err = "Bad Data Generated";
                        env->ds.mcuData->setPressureCalibrationStatus(calStatus);
                        calibrationStop();
                        dataCaptureStartOk = false;
                    }
                }
            }

            if (dataCaptureStartOk)
            {
                calibrationDataCaptureStartAsync();
            }

            env->ds.mcuData->setPressureCalibrationStatus(calStatus);
        });
    });
    env->ds.mcuAction->actCalGetPressureData(syringeIdx, false, guid);
}

QString McuPressureCalibration::getCoefficients()
{
    Matrix inverseMatrix = squareMatrix.inverse();
    Matrix coefficients = inverseMatrix * columnMatrix;
    QString coeffStr = QString().asprintf("%f, %f, %f, %f, %f, %f", coefficients(0, 0), coefficients(1, 0), coefficients(2, 0), coefficients(3, 0), coefficients(4, 0), coefficients(5, 0));
    LOG_DEBUG("getCoefficients(): Coefficients = %s\n", coeffStr.CSTR());
    return coeffStr;
}

void McuPressureCalibration::calibrationDataCaptureStartAsync()
{
    DS_McuDef::PressureCalibrationState state = (DS_McuDef::PressureCalibrationState)getState();

    if (state != DS_McuDef::PRESSURE_CAL_STATE_INJECT_STAGE_PROGRESS)
    {
        return;
    }

    DS_McuDef::PressureCalibrationStatus calStatus = env->ds.mcuData->getPressureCalibrationStatus();
    int injectStageIdx = calStatus.injectStageIdx;

    // Start the capture and let the logic decide when to accumulate the data
    tmrCaptureData.start(calStatus.stagesParams[injectStageIdx].dataCaptureIntervalMs);
}

int McuPressureCalibration::getState()
{
    return env->ds.mcuData->getPressureCalibrationStatus().state;
}

QString McuPressureCalibration::getStateStr(int state)
{
    return ImrParser::ToImr_PressureCalibrationState((DS_McuDef::PressureCalibrationState)state);
}

void McuPressureCalibration::setStateSynch(int newState)
{
    DS_McuDef::PressureCalibrationStatus calStatus = env->ds.mcuData->getPressureCalibrationStatus();
    calStatus.state = (DS_McuDef::PressureCalibrationState)newState;
    env->ds.mcuData->setPressureCalibrationStatus(calStatus);
}

void McuPressureCalibration::processState()
{
    DS_McuDef::PressureCalibrationState state = (DS_McuDef::PressureCalibrationState)getState();
    DS_McuDef::PressureCalibrationStatus calStatus = env->ds.mcuData->getPressureCalibrationStatus();

    switch (state)
    {
    //-----------------------------------
    case DS_McuDef::PRESSURE_CAL_STATE_IDLE:
        LOG_INFO("PRESSURE_CAL_STATE_IDLE\n");
        env->actionMgr->deleteActAll(actStatusBuf.guid);
        env->actionMgr->deleteActAll(guidSubAction);
        env->actionMgr->deleteActAll(guidMudsLatchedMonitor);
        break;
    case DS_McuDef::PRESSURE_CAL_STATE_STARTED:
        LOG_INFO("PRESSURE_CAL_STATE_STARTED\n");
        {
            calStatus.init();
            env->ds.mcuData->setPressureCalibrationStatus(calStatus);

            // Ensure the MUDS is latched before start
            bool mudsLatched = env->ds.mcuData->getMudsLatched();
            if (!mudsLatched)
            {
                LOG_WARNING("%s: Muds is not latched\n", getStateStr(getState()).CSTR());
                actStatusBuf.state = DS_ACTION_STATE_INVALID_STATE;
                actStatusBuf.err = "Muds Unlatched";
                setState(DS_McuDef::PRESSURE_CAL_STATE_FAILED);
                return;
            }

            // Ensure the MUDS is latched during operation
            env->actionMgr->deleteActAll(guidMudsLatchedMonitor);
            guidMudsLatchedMonitor = Util::newGuid();

            QMetaObject::Connection conn = connect(env->ds.mcuData, &DS_McuData::signalDataChanged_MudsLatched, this, [=](bool mudsLatched) {
                if (!mudsLatched)
                {
                    LOG_WARNING("%s: Muds is not latched\n", getStateStr(getState()).CSTR());
                    actStatusBuf.state = DS_ACTION_STATE_INVALID_STATE;
                    actStatusBuf.err = "Muds Unlatched";
                    calibrationStop();
                    setState(DS_McuDef::PRESSURE_CAL_STATE_FAILED);
                }
            });
            env->actionMgr->createActCompleted(guidMudsLatchedMonitor, conn, QString(__PRETTY_FUNCTION__) + ": PRESSURE_CAL_STATE_STARTED: MudsLatched");

            // Initialise Inject Stage Params
            DS_McuDef::PressureCalibrationStageParams params;
            params.flowRate = 1.0;
            params.dataCaptureIntervalMs = 400;
            calStatus.stagesParams.append(params);

            params.flowRate = 2.0;
            params.dataCaptureIntervalMs = 200;
            calStatus.stagesParams.append(params);

            params.flowRate = 4.0;
            params.dataCaptureIntervalMs = 100;
            calStatus.stagesParams.append(params);

            params.flowRate = 6.0;
            params.dataCaptureIntervalMs = 75;
            calStatus.stagesParams.append(params);

            params.flowRate = 8.0;
            params.dataCaptureIntervalMs = 50;
            calStatus.stagesParams.append(params);

            params.flowRate = 10.0;
            params.dataCaptureIntervalMs = 25;
            calStatus.stagesParams.append(params);

            env->ds.mcuData->setPressureCalibrationStatus(calStatus);

            squareMatrix = Matrix(MATRIX_N, MATRIX_N);
            columnMatrix = Matrix(MATRIX_N, 1);
        }
        setState(DS_McuDef::PRESSURE_CAL_STATE_DISENGAGE_STARTED);
        break;
    //-----------------------------------
    case DS_McuDef::PRESSURE_CAL_STATE_DISENGAGE_STARTED:
        LOG_INFO("PRESSURE_CAL_STATE_DISENGAGE_STARTED[%d]\n", calStatus.injectStageIdx);
        handleSubAction(DS_McuDef::PRESSURE_CAL_STATE_DISENGAGE_PROGRESS, DS_McuDef::PRESSURE_CAL_STATE_DISENGAGE_COMPLETED, DS_McuDef::PRESSURE_CAL_STATE_DISENGAGE_FAILED);
        env->ds.mcuAction->actPullPlunger(syringeIdx, guidSubAction);
        break;
    case DS_McuDef::PRESSURE_CAL_STATE_DISENGAGE_PROGRESS:
        LOG_INFO("PRESSURE_CAL_STATE_DISENGAGE_PROGRESS[%d]\n", calStatus.injectStageIdx);
        break;
    case DS_McuDef::PRESSURE_CAL_STATE_DISENGAGE_FAILED:
        LOG_ERROR("PRESSURE_CAL_STATE_DISENGAGE_FAILED[%d]\n", calStatus.injectStageIdx);
        calibrationStop();
        break;
    case DS_McuDef::PRESSURE_CAL_STATE_DISENGAGE_COMPLETED:
        LOG_INFO("PRESSURE_CAL_STATE_DISENGAGE_COMPLETED[%d]\n", calStatus.injectStageIdx);
        setState(DS_McuDef::PRESSURE_CAL_STATE_FIND_PLUNGER_STARTED);
        break;
    //-----------------------------------
    case DS_McuDef::PRESSURE_CAL_STATE_FIND_PLUNGER_STARTED:
        LOG_INFO("PRESSURE_CAL_STATE_FIND_PLUNGER_STARTED[%d]\n", calStatus.injectStageIdx);
        {
            double syringeVol = env->ds.mcuData->getSyringeVols()[syringeIdx];
            calStatus.homePosition = syringeVol;
            env->ds.mcuData->setPressureCalibrationStatus(calStatus);
            LOG_DEBUG("PRESSURE_CAL_STATE_FIND_PLUNGER_STARTED[%d]: HomePosition=%.1fml\n", calStatus.injectStageIdx, calStatus.homePosition);
            handleSubAction(DS_McuDef::PRESSURE_CAL_STATE_FIND_PLUNGER_PROGRESS, DS_McuDef::PRESSURE_CAL_STATE_FIND_PLUNGER_COMPLETED, DS_McuDef::PRESSURE_CAL_STATE_FIND_PLUNGER_FAILED);
            env->ds.mcuAction->actFindPlunger(syringeIdx, guidSubAction);
        }
        break;
    case DS_McuDef::PRESSURE_CAL_STATE_FIND_PLUNGER_PROGRESS:
        LOG_INFO("PRESSURE_CAL_STATE_FIND_PLUNGER_PROGRESS[%d]\n", calStatus.injectStageIdx);
        break;
    case DS_McuDef::PRESSURE_CAL_STATE_FIND_PLUNGER_FAILED:
        LOG_ERROR("PRESSURE_CAL_STATE_FIND_PLUNGER_FAILED[%d]\n", calStatus.injectStageIdx);
        calibrationStop();
        break;
    case DS_McuDef::PRESSURE_CAL_STATE_FIND_PLUNGER_COMPLETED:
        LOG_INFO("PRESSURE_CAL_STATE_FIND_PLUNGER_COMPLETED[%d]\n", calStatus.injectStageIdx);
        {
            double syringeVol = env->ds.mcuData->getSyringeVols()[syringeIdx];
            calStatus.engagedPosition = syringeVol;
            env->ds.mcuData->setPressureCalibrationStatus(calStatus);

            double maxTravelAllowed = env->ds.capabilities->get_Calibration_PressureCalMaxTravelUntilEngaged();
            double traveledUntilEngaged = calStatus.homePosition - syringeVol;
            LOG_DEBUG("PRESSURE_CAL_STATE_FIND_PLUNGER_COMPLETED[%d]: SyringeTraveledUntilEngaged = (%.1fml - %.1fml) = %.1fml: < MaxAllowed(%.1fml)\n",
                      calStatus.injectStageIdx, calStatus.homePosition, calStatus.engagedPosition, traveledUntilEngaged, maxTravelAllowed);
            setState(DS_McuDef::PRESSURE_CAL_STATE_INJECT_STAGE_STARTED);
        }
        break;
    //-----------------------------------
    case DS_McuDef::PRESSURE_CAL_STATE_INJECT_STAGE_STARTED:
        LOG_INFO("PRESSURE_CAL_STATE_INJECT_STAGE_STARTED[%d]\n", calStatus.injectStageIdx);
        {
            calStatus.dataCheckInfo.init();
            env->ds.mcuData->setPressureCalibrationStatus(calStatus);

            double injectVol = env->ds.capabilities->get_Calibration_PressureCalInjectVolume();
            DS_McuDef::ActPistonParams params;
            params.idx = syringeIdx;
            params.vol = injectVol;
            params.flow = calStatus.stagesParams[calStatus.injectStageIdx].flowRate;

            LOG_DEBUG("PRESSURE_CAL_STATE_INJECT_STAGE_STARTED[%d]: Injecting vol=%.1fml, flow=%.1fml/s\n", calStatus.injectStageIdx, params.vol, params.flow);
            handleSubAction(DS_McuDef::PRESSURE_CAL_STATE_INJECT_STAGE_PROGRESS, DS_McuDef::PRESSURE_CAL_STATE_INJECT_STAGE_COMPLETED, DS_McuDef::PRESSURE_CAL_STATE_INJECT_STAGE_FAILED);
            env->ds.mcuAction->actPiston(params, guidSubAction);
        }
        break;
    case DS_McuDef::PRESSURE_CAL_STATE_INJECT_STAGE_PROGRESS:
        LOG_INFO("PRESSURE_CAL_STATE_INJECT_STAGE_PROGRESS[%d]\n", calStatus.injectStageIdx);
        calibrationDataCaptureStartAsync();
        break;
    case DS_McuDef::PRESSURE_CAL_STATE_INJECT_STAGE_FAILED:
        LOG_ERROR("PRESSURE_CAL_STATE_INJECT_STAGE_FAILED[%d]\n", calStatus.injectStageIdx);
        calibrationStop();
        break;
    case DS_McuDef::PRESSURE_CAL_STATE_INJECT_STAGE_COMPLETED:
        LOG_INFO("PRESSURE_CAL_STATE_INJECT_STAGE_COMPLETED[%d]\n", calStatus.injectStageIdx);
        {
            tmrCaptureData.stop();
            calStatus.injectStageIdx++;
            calStatus.homePosition = -1;
            calStatus.engagedPosition = -1;
            calStatus.firstGoodAdcValuePosition = -1;
            env->ds.mcuData->setPressureCalibrationStatus(calStatus);

            if (calStatus.injectStageIdx == calStatus.stagesParams.length())
            {
                setState(DS_McuDef::PRESSURE_CAL_STATE_FINAL_DISENGAGE_STARTED);
            }
            else
            {
                setState(DS_McuDef::PRESSURE_CAL_STATE_DISENGAGE_STARTED);
            }
        }
        break;
    //-----------------------------------
    case DS_McuDef::PRESSURE_CAL_STATE_FINAL_DISENGAGE_STARTED:
        LOG_INFO("PRESSURE_CAL_STATE_FINAL_DISENGAGE_STARTED\n");
        handleSubAction(DS_McuDef::PRESSURE_CAL_STATE_FINAL_DISENGAGE_PROGRESS, DS_McuDef::PRESSURE_CAL_STATE_FINAL_DISENGAGE_COMPLETED, DS_McuDef::PRESSURE_CAL_STATE_FINAL_DISENGAGE_FAILED);
        env->ds.mcuAction->actPullPlunger(syringeIdx, guidSubAction);
        break;
    case DS_McuDef::PRESSURE_CAL_STATE_FINAL_DISENGAGE_PROGRESS:
        LOG_INFO("PRESSURE_CAL_STATE_FINAL_DISENGAGE_PROGRESS\n");
        break;
    case DS_McuDef::PRESSURE_CAL_STATE_FINAL_DISENGAGE_FAILED:
        LOG_ERROR("PRESSURE_CAL_STATE_FINAL_DISENGAGE_FAILED\n");
        calibrationStop();
        break;
    case DS_McuDef::PRESSURE_CAL_STATE_FINAL_DISENGAGE_COMPLETED:
        LOG_INFO("PRESSURE_CAL_STATE_FINAL_DISENGAGE_COMPLETED\n");
        setState(DS_McuDef::PRESSURE_CAL_STATE_DONE);
        break;
    //-----------------------------------
    case DS_McuDef::PRESSURE_CAL_STATE_FAILED:
        LOG_ERROR("PRESSURE_CAL_STATE_FAILED: Reason=%s\n", ImrParser::ToImr_DataServiceActionStatusStr(actStatusBuf).CSTR());
        {
            tmrCaptureData.stop();

            env->ds.alertAction->activate("SRUHardwareCalibrated", QString().asprintf("PressureCalibration;%s;NA;%s", ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdx).CSTR(), actStatusBuf.err.CSTR()));

            if ( (calStatus.err == "") &&
                 (actStatusBuf.err != "") )
            {
                calStatus.err = actStatusBuf.err;
                env->ds.mcuData->setPressureCalibrationStatus(calStatus);
            }
            else if ( (calStatus.err != "") &&
                      (actStatusBuf.err == "") )
            {
                actStatusBuf.err = calStatus.err;
                actStatusBuf.state = DS_ACTION_STATE_INTERNAL_ERR;
            }

            // Make sure plunger is disengaged to prevent bad spring state
            LOG_INFO("PRESSURE_CAL_STATE_FAILED: Pulling plunger%d to home position..\n", syringeIdx);
            QTimer::singleShot(MCU_STOP_COMMAND_ALIVE_MS, this, [=] {
                env->ds.mcuAction->actPullPlunger(syringeIdx);
                setState(DS_McuDef::PRESSURE_CAL_STATE_IDLE);
            });
        }
        break;
    case DS_McuDef::PRESSURE_CAL_STATE_DONE:
        LOG_INFO("PRESSURE_CAL_STATE_DONE\n");
        env->ds.alertAction->activate("SRUHardwareCalibrated", QString().asprintf("PressureCalibration;%s;NA;OK", ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdx).CSTR()));
        env->ds.hardwareInfo->updateCalibrationInfo("SRUHardwareCalibrated");
        setState(DS_McuDef::PRESSURE_CAL_STATE_IDLE);
        break;

    default:
        LOG_ERROR("Unexpected State(%s)\n", ImrParser::ToImr_PressureCalibrationState(state).CSTR());
        break;
    }
}

