#include "TestHwPiston.h"
#include "Common/ImrParser.h"
#include "DataServices/Test/DS_TestData.h"
#include "DataServices/Device/DS_DeviceAction.h"
#include "DataServices/Mcu/DS_McuAction.h"
#include "DataServices/Mcu/DS_McuData.h"

TestHwPiston::TestHwPiston(QObject *parent, EnvGlobal *env_, EnvLocal *envLocal_) :
    ActionBaseExt(parent, env_)
{
    envLocal = envLocal_;
    setState(STATE_IDLE);
}


TestHwPiston::~TestHwPiston()
{
}

DataServiceActionStatus TestHwPiston::actStart(QVariantList testParamArgs, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "TestHwPiston-Start", QString().asprintf("%s", Util::qVarientToJsonData(testParamArgs).CSTR()));

    DS_TestDef::TestStatus testStatus = env->ds.testData->getTestStatus();
    testStatus.guid = actGuid;
    testStatus.type = DS_TestDef::TEST_TYPE_PISTON;
    testStatus.progress = 0;
    testStatus.userAborted = false;
    testStatus.isFinished = false;
    testStatus.stateStr = "";

    if (state != STATE_IDLE)
    {
        status.state = DS_ACTION_STATE_BUSY;
        status.err = "Test In Progress";
        actionStarted(status);
        testStatus.isFinished = true;
        env->ds.testData->setTestStatus(testStatus);
        setState(STATE_IDLE);
        goto bail;
    }
    else if (testParamArgs.length() != 11)
    {
        status.state = DS_ACTION_STATE_BAD_REQUEST;
        status.err = "Bad Parameters";
        actionStarted(status);
        testStatus.isFinished = true;
        env->ds.testData->setTestStatus(testStatus);
        setState(STATE_IDLE);
        goto bail;
    }

    // Set test params
    testParams.cycles = testParamArgs[0].toInt();
    testParams.pistonEnabled[0] = testParamArgs[1].toBool();
    testParams.pistonEnabled[1] = testParamArgs[2].toBool();
    testParams.pistonEnabled[2] = testParamArgs[3].toBool();

    testParams.disengageAfterFill = testParamArgs[4].toBool();
    testParams.injectVol = testParamArgs[5].toDouble();
    testParams.injectFlow = testParamArgs[6].toDouble();
    testParams.fillFlow = testParamArgs[7].toDouble();
    testParams.pauseAfterFillMs = testParamArgs[8].toInt();
    testParams.pauseAfterDisengageMs = testParamArgs[9].toInt();
    testParams.purgeAfterEngage = testParamArgs[10].toBool();
    testParams.curCycleCount = 0;

    if ( (!testParams.pistonEnabled[0]) &&
         (!testParams.pistonEnabled[1]) &&
         (!testParams.pistonEnabled[2]) )
    {
        status.state = DS_ACTION_STATE_BAD_REQUEST;
        status.err = "No Piston Selected";
        actionStarted(status);
        testStatus.isFinished = true;
        env->ds.testData->setTestStatus(testStatus);
        setState(STATE_IDLE);
        goto bail;
    }

    LOG_INFO("\n================================\n");
    LOG_INFO("TEST_PISTON: Cycle test started\n");

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);

    env->ds.testData->setTestStatus(testStatus);
    setState(STATE_STARTED);

bail:
    return status;
}

DataServiceActionStatus TestHwPiston::actStop(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "TestHwPiston-Stop");

    if (state == STATE_IDLE)
    {
        LOG_ERROR("TEST_PISTON: STOP: Test is not running\n");
        status.state = DS_ACTION_STATE_BAD_REQUEST;
        status.err = "Test is not running";
        actionStarted(status);
    }
    else
    {
        status.state = DS_ACTION_STATE_STARTED;
        actionStarted(status);

        env->ds.mcuAction->actStopAll();

        DS_TestDef::TestStatus testStatus = env->ds.testData->getTestStatus();
        testStatus.userAborted = true;
        env->ds.testData->setTestStatus(testStatus);

        setState(STATE_ABORTED);

        status.state = DS_ACTION_STATE_COMPLETED;
        actionCompleted(status);
    }

    return status;
}

void TestHwPiston::setPistonHw()
{
    State progressState, completeState;
    QString actionStr;

    switch (state)
    {
    case STATE_FIND_PLUNGER_STARTED:
        progressState = STATE_FIND_PLUNGER_PROGRESS;
        completeState = STATE_FIND_PLUNGER_DONE;
        actionStr = "FIND_PLUNGER";
        break;
    case STATE_PUSH_PLUNGER_STARTED:
        progressState = STATE_PUSH_PLUNGER_PROGRESS;
        completeState = STATE_PUSH_PLUNGER_DONE;
        actionStr = "PUSH_PLUNGER";
        break;
    case STATE_ENGAGE_STARTED:
        progressState = STATE_ENGAGE_PROGRESS;
        completeState = STATE_ENGAGE_DONE;
        actionStr = "ENGAGE";
        break;
    case STATE_INJECT_STARTED:
        progressState = STATE_INJECT_PROGRESS;
        completeState = STATE_INJECT_DONE;
        actionStr = "INJECT";
        break;
    case STATE_FILL_STARTED:
        progressState = STATE_FILL_PROGRESS;
        completeState = STATE_FILL_DONE;
        actionStr = "FILL";
        break;
    case STATE_DISENGAGE_STARTED:
        progressState = STATE_DISENGAGE_PROGRESS;
        completeState = STATE_DISENGAGE_DONE;
        actionStr = "DISENGAGE";
        break;
    default:
        LOG_ERROR("TEST_PISTON: Cannot start hw action: bad state(%d)\n", state);
        return;
    }

    DS_TestDef::TestStatus testStatus = env->ds.testData->getTestStatus();
    testStatus.stateStr = QString().asprintf("#%d: Pistons: %s started", testParams.curCycleCount + 1, actionStr.CSTR());
    env->ds.testData->setTestStatus(testStatus);
    setState(progressState);

    for (int pistonIdx = 0; pistonIdx < ARRAY_LEN(testParams.pistonEnabled); pistonIdx++)
    {
        if (testParams.pistonEnabled[pistonIdx])
        {
            testParams.pistonActionCompleted[pistonIdx] = false;

            QString guid = Util::newGuid();
            env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
                if (curStatus.state == DS_ACTION_STATE_STARTED)
                {
                    env->actionMgr->onActionCompleted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
                        if (state == progressState)
                        {
                            DS_TestDef::TestStatus testStatus = env->ds.testData->getTestStatus();
                            if (curStatus.state == DS_ACTION_STATE_COMPLETED)
                            {
                                // Check if ok to go to next state
                                bool stateCompleted = false;
                                DS_McuDef::PlungerStates plungerStates = env->ds.mcuData->getPlungerStates();
                                DS_McuDef::SyringeStates syringeStates = env->ds.mcuData->getSyringeStates();
                                switch (state)
                                {
                                case STATE_ENGAGE_PROGRESS:
                                    stateCompleted = ( ( (!testParams.pistonEnabled[SYRINGE_IDX_SALINE]) || (plungerStates[SYRINGE_IDX_SALINE] == DS_McuDef::PLUNGER_STATE_ENGAGED) ) &&
                                                       ( (!testParams.pistonEnabled[SYRINGE_IDX_CONTRAST1]) || (plungerStates[SYRINGE_IDX_CONTRAST1] == DS_McuDef::PLUNGER_STATE_ENGAGED) ) &&
                                                       ( (!testParams.pistonEnabled[SYRINGE_IDX_CONTRAST2]) || (plungerStates[SYRINGE_IDX_CONTRAST2] == DS_McuDef::PLUNGER_STATE_ENGAGED) ) );
                                    break;
                                case STATE_DISENGAGE_PROGRESS:
                                    stateCompleted = ( ( (!testParams.pistonEnabled[SYRINGE_IDX_SALINE]) || (plungerStates[SYRINGE_IDX_SALINE] == DS_McuDef::PLUNGER_STATE_DISENGAGED) ) &&
                                                       ( (!testParams.pistonEnabled[SYRINGE_IDX_CONTRAST1]) || (plungerStates[SYRINGE_IDX_CONTRAST1] == DS_McuDef::PLUNGER_STATE_DISENGAGED) ) &&
                                                       ( (!testParams.pistonEnabled[SYRINGE_IDX_CONTRAST2]) || (plungerStates[SYRINGE_IDX_CONTRAST2] == DS_McuDef::PLUNGER_STATE_DISENGAGED) ) );
                                    break;
                                case STATE_FIND_PLUNGER_PROGRESS:
                                case STATE_PUSH_PLUNGER_PROGRESS:
                                case STATE_INJECT_PROGRESS:
                                case STATE_FILL_PROGRESS:
                                    stateCompleted = ( ( (!testParams.pistonEnabled[SYRINGE_IDX_SALINE]) || (syringeStates[SYRINGE_IDX_SALINE] == DS_McuDef::SYRINGE_STATE_COMPLETED) ) &&
                                                       ( (!testParams.pistonEnabled[SYRINGE_IDX_CONTRAST1]) || (syringeStates[SYRINGE_IDX_CONTRAST1] == DS_McuDef::SYRINGE_STATE_COMPLETED) ) &&
                                                       ( (!testParams.pistonEnabled[SYRINGE_IDX_CONTRAST2]) || (syringeStates[SYRINGE_IDX_CONTRAST2] == DS_McuDef::SYRINGE_STATE_COMPLETED) ) );
                                    break;
                                default:
                                    break;
                                }

                                if (!testParams.pistonActionCompleted[pistonIdx])
                                {
                                    testParams.pistonActionCompleted[pistonIdx] = true;
                                    testStatus.stateStr = QString().asprintf("#%d:     Piston[%s]: %s complete", testParams.curCycleCount + 1, ImrParser::ToImr_FluidSourceSyringeIdx((SyringeIdx)pistonIdx).CSTR(), actionStr.CSTR());
                                    env->ds.testData->setTestStatus(testStatus);
                                }

                                if (stateCompleted)
                                {
                                    for (int pistonIdx2 = 0; pistonIdx2 < ARRAY_LEN(testParams.pistonActionCompleted); pistonIdx2++)
                                    {
                                        if (!testParams.pistonActionCompleted[pistonIdx2])
                                        {
                                            // Other piston is expected to be completed. Print out for other piston.
                                            testParams.pistonActionCompleted[pistonIdx2] = true;
                                            testStatus.stateStr = QString().asprintf("#%d:     Piston[%s]: %s complete", testParams.curCycleCount + 1, ImrParser::ToImr_FluidSourceSyringeIdx((SyringeIdx)pistonIdx2).CSTR(), actionStr.CSTR());
                                            env->ds.testData->setTestStatus(testStatus);
                                        }
                                    }

                                    testStatus.stateStr = QString().asprintf("#%d: Pistons: %s complete", testParams.curCycleCount + 1, actionStr.CSTR());
                                    env->ds.testData->setTestStatus(testStatus);
                                    setState(completeState);
                                }
                            }
                            else
                            {
                                LOG_ERROR("TEST_PISTON: Piston[%s]: ACTION_STATUS: %s\n", ImrParser::ToImr_FluidSourceSyringeIdx((SyringeIdx)pistonIdx).CSTR(), ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());

                                testStatus.stateStr = QString().asprintf("#%d: ERROR: Piston[%s]: %s complete failed (%s)", testParams.curCycleCount + 1, ImrParser::ToImr_FluidSourceSyringeIdx((SyringeIdx)pistonIdx).CSTR(), actionStr.CSTR(), ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());
                                env->ds.testData->setTestStatus(testStatus);
                                setState(STATE_ABORTED);
                            }
                        }
                    });
                }
                else
                {
                    LOG_ERROR("TEST_PISTON: Piston[%s]: ACTION_STATUS: %s\n", ImrParser::ToImr_FluidSourceSyringeIdx((SyringeIdx)pistonIdx).CSTR(), ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());
                    DS_TestDef::TestStatus testStatus = env->ds.testData->getTestStatus();
                    testStatus.stateStr = QString().asprintf("#%d: ERROR: Piston[%s]: %s start failed (%s)", testParams.curCycleCount + 1, ImrParser::ToImr_FluidSourceSyringeIdx((SyringeIdx)pistonIdx).CSTR(), actionStr.CSTR(), ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());
                    env->ds.testData->setTestStatus(testStatus);
                    setState(STATE_ABORTED);
                }
            });

            switch (progressState)
            {
            case STATE_FIND_PLUNGER_PROGRESS:
                env->ds.mcuAction->actFindPlunger((SyringeIdx)pistonIdx, guid);
                break;
            case STATE_PUSH_PLUNGER_PROGRESS:
                {
                    double curVol = env->ds.mcuData->getSyringeVols()[pistonIdx];
                    double injectVol = qMin(curVol * 0.9, SYRINGE_VOLUME_MAX - 10);
                    DS_McuDef::ActPistonParams params;
                    params.idx = (SyringeIdx)pistonIdx;
                    params.vol = injectVol;
                    params.flow = testParams.injectFlow;
                    LOG_DEBUG("TEST_PISTON: STATE_PUSH_PLUNGER_PROGRESS: Pushing syringe=%d, curVol=%.1fml, injectVol=%.1fml @ %.1fml/s\n", params.idx, curVol, params.vol, params.flow);
                    env->ds.mcuAction->actPiston(params, guid);
                }
                break;
            case STATE_ENGAGE_PROGRESS:
                {
                    double curVol = env->ds.mcuData->getSyringeVols()[pistonIdx];
                    LOG_DEBUG("TEST_PISTON: STATE_ENGAGE_PROGRESS: Engaging at syringe=%d, vol=%.fml\n", pistonIdx, curVol);
                    env->ds.mcuAction->actEngage((SyringeIdx)pistonIdx, guid);
                }
                break;
            case STATE_INJECT_PROGRESS:
                {
                    if (testParams.purgeAfterEngage)
                    {
                        if (pistonIdx == 0) {
                            // only print once
                            testStatus.stateStr = QString().asprintf("#%d: Pistons: %s started - PURGING", testParams.curCycleCount + 1, actionStr.CSTR());
                            env->ds.testData->setTestStatus(testStatus);
                        }
                        env->ds.mcuAction->actPurge((SyringeIdx)pistonIdx, guid);
                    }
                    else
                    {
                        DS_McuDef::ActPistonParams params;
                        params.idx = (SyringeIdx)pistonIdx;
                        params.vol = testParams.injectVol;
                        params.flow = 2;
                        env->ds.mcuAction->actPiston(params, guid);
                    }
                }
                break;
            case STATE_FILL_PROGRESS:
                {
                    DS_McuDef::ActPistonParams params;
                    params.idx = (SyringeIdx)pistonIdx;
                    params.vol = SYRINGE_VOLUME_FILL_ALL;
                    params.flow = -testParams.fillFlow;
                    env->ds.mcuAction->actPiston(params, guid);
                }
                break;
            case STATE_DISENGAGE_PROGRESS:
                env->ds.mcuAction->actDisengage((SyringeIdx)pistonIdx, guid);
                break;
            default:
                LOG_ERROR("TEST_PISTON: Incorrect test state (%d). Did not start action\n", progressState);
                break;
            }
        }
        else
        {
            // Piston test not started
            testParams.pistonActionCompleted[pistonIdx] = true;
        }
    }
}

void TestHwPiston::setStopcockHw()
{
    State progressState, completeState;
    DS_McuDef::StopcockPos newScPos;
    QString actionStr;

    switch (state)
    {
    case TestHwPiston::STATE_STOPCOCK_TO_INJECT_STARTED:
        progressState = STATE_STOPCOCK_TO_INJECT_PROGRESS;
        completeState = STATE_STOPCOCK_TO_INJECT_DONE;
        newScPos = DS_McuDef::STOPCOCK_POS_INJECT;
        actionStr = "SET STOPCOCK TO INJECT";
        break;
    case TestHwPiston::STATE_STOPCOCK_TO_FILL_STARTED:
        progressState = STATE_STOPCOCK_TO_FILL_PROGRESS;
        completeState = STATE_STOPCOCK_TO_FILL_DONE;
        newScPos = DS_McuDef::STOPCOCK_POS_FILL;
        actionStr = "SET STOPCOCK TO FILL";
        break;
    default:
        LOG_ERROR("TEST_PISTON: Cannot start stopcock hw action: bad state(%d)\n", state);
        return;
    }

    DS_TestDef::TestStatus testStatus = env->ds.testData->getTestStatus();
    testStatus.stateStr = QString().asprintf("#%d: Pistons: %s started", testParams.curCycleCount + 1, actionStr.CSTR());
    env->ds.testData->setTestStatus(testStatus);
    setState(progressState);

    for (int pistonIdx = 0; pistonIdx < ARRAY_LEN(testParams.pistonEnabled); pistonIdx++)
    {
        if (testParams.pistonEnabled[pistonIdx])
        {
            testParams.pistonActionCompleted[pistonIdx] = false;
            QString guid = Util::newGuid();
            env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
                if (curStatus.state == DS_ACTION_STATE_STARTED)
                {
                    env->actionMgr->onActionCompleted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
                        if (state == progressState)
                        {
                            DS_TestDef::TestStatus testStatus = env->ds.testData->getTestStatus();
                            if (curStatus.state == DS_ACTION_STATE_COMPLETED)
                            {
                                // Check if ok to go to next state
                                bool stateCompleted = false;
                                DS_McuDef::StopcockPosAll stopcockPositons = env->ds.mcuData->getStopcockPosAll();

                                switch (state)
                                {
                                case STATE_STOPCOCK_TO_INJECT_PROGRESS:
                                    stateCompleted = ( ( (!testParams.pistonEnabled[SYRINGE_IDX_SALINE]) || (stopcockPositons[SYRINGE_IDX_SALINE] == DS_McuDef::STOPCOCK_POS_INJECT) ) &&
                                                       ( (!testParams.pistonEnabled[SYRINGE_IDX_CONTRAST1]) || (stopcockPositons[SYRINGE_IDX_CONTRAST1] == DS_McuDef::STOPCOCK_POS_INJECT) ) &&
                                                       ( (!testParams.pistonEnabled[SYRINGE_IDX_CONTRAST2]) || (stopcockPositons[SYRINGE_IDX_CONTRAST2] == DS_McuDef::STOPCOCK_POS_INJECT) ) );
                                    break;
                                case STATE_STOPCOCK_TO_FILL_PROGRESS:
                                    stateCompleted = ( ( (!testParams.pistonEnabled[SYRINGE_IDX_SALINE]) || (stopcockPositons[SYRINGE_IDX_SALINE] == DS_McuDef::STOPCOCK_POS_FILL) ) &&
                                                       ( (!testParams.pistonEnabled[SYRINGE_IDX_CONTRAST1]) || (stopcockPositons[SYRINGE_IDX_CONTRAST1] == DS_McuDef::STOPCOCK_POS_FILL) ) &&
                                                       ( (!testParams.pistonEnabled[SYRINGE_IDX_CONTRAST2]) || (stopcockPositons[SYRINGE_IDX_CONTRAST2] == DS_McuDef::STOPCOCK_POS_FILL) ) );
                                    break;
                                default:
                                    break;
                                }

                                if (!testParams.pistonActionCompleted[pistonIdx])
                                {
                                    testParams.pistonActionCompleted[pistonIdx] = true;
                                    testStatus.stateStr = QString().asprintf("#%d:     Piston[%s]: %s complete", testParams.curCycleCount + 1, ImrParser::ToImr_FluidSourceSyringeIdx((SyringeIdx)pistonIdx).CSTR(), actionStr.CSTR());
                                    env->ds.testData->setTestStatus(testStatus);
                                }

                                if (stateCompleted)
                                {
                                    for (int pistonIdx2 = 0; pistonIdx2 < ARRAY_LEN(testParams.pistonActionCompleted); pistonIdx2++)
                                    {
                                        if (!testParams.pistonActionCompleted[pistonIdx2])
                                        {
                                            // Other piston is expected to be completed. Print out for other piston.
                                            testParams.pistonActionCompleted[pistonIdx2] = true;
                                            testStatus.stateStr = QString().asprintf("#%d:     Piston[%s]: %s complete", testParams.curCycleCount + 1, ImrParser::ToImr_FluidSourceSyringeIdx((SyringeIdx)pistonIdx2).CSTR(), actionStr.CSTR());
                                            env->ds.testData->setTestStatus(testStatus);
                                        }
                                    }

                                    testStatus.stateStr = QString().asprintf("#%d: Pistons: %s complete", testParams.curCycleCount + 1, actionStr.CSTR());
                                    env->ds.testData->setTestStatus(testStatus);
                                    setState(completeState);
                                }
                            }
                            else
                            {
                                LOG_ERROR("TEST_PISTON: Piston[%s]: ACTION_STATUS: %s\n", ImrParser::ToImr_FluidSourceSyringeIdx((SyringeIdx)pistonIdx).CSTR(), ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());
                                testStatus.stateStr = QString().asprintf("#%d: ERROR: Piston[%s]: %s complete failed (%s)", testParams.curCycleCount + 1, ImrParser::ToImr_FluidSourceSyringeIdx((SyringeIdx)pistonIdx).CSTR(), actionStr.CSTR(), ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());
                                env->ds.testData->setTestStatus(testStatus);
                                setState(STATE_ABORTED);
                            }
                        }
                    });
                }
                else
                {
                    LOG_ERROR("TEST_PISTON: Piston[%s]: ACTION_STATUS: %s\n", ImrParser::ToImr_FluidSourceSyringeIdx((SyringeIdx)pistonIdx).CSTR(), ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());
                    DS_TestDef::TestStatus testStatus = env->ds.testData->getTestStatus();
                    testStatus.stateStr = QString().asprintf("#%d: ERROR: Piston[%s]: %s start failed (%s)", testParams.curCycleCount + 1, ImrParser::ToImr_FluidSourceSyringeIdx((SyringeIdx)pistonIdx).CSTR(), actionStr.CSTR(), ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());
                    env->ds.testData->setTestStatus(testStatus);
                    setState(STATE_ABORTED);
                }
            });

            env->ds.deviceAction->actStopcock((SyringeIdx)pistonIdx, newScPos, STOPCOCK_ACTION_TRIALS_LIMIT, guid);
        }
        else
        {
            testParams.pistonActionCompleted[pistonIdx] = true;
        }
    }
}

void TestHwPiston::setStateSynch(int newState)
{
    DS_TestDef::TestStatus testStatus = env->ds.testData->getTestStatus();
    if (testStatus.isFinished)
    {
        state = STATE_IDLE;
    }
    else
    {
        state = newState;
    }
}

void TestHwPiston::processState()
{
    DS_TestDef::TestStatus testStatus = env->ds.testData->getTestStatus();

    switch (state)
    {
    case STATE_IDLE:
        LOG_INFO("TEST_PISTON: STATE_IDLE\n");
        testStatus.guid = EMPTY_GUID;
        testStatus.type = DS_TestDef::TEST_TYPE_UNKNOWN;
        env->ds.testData->setTestStatus(testStatus);
        break;
    case STATE_STARTED:
        LOG_INFO("TEST_PISTON: STATE_STARTED\n");
        {
            DS_McuDef::ActLedParams param;
            param.setColorYellow();
            env->ds.deviceAction->actLeds(LED_IDX_SALINE, param);
            env->ds.deviceAction->actLeds(LED_IDX_CONTRAST1, param);
            env->ds.deviceAction->actLeds(LED_IDX_CONTRAST2, param);
            env->ds.deviceAction->actLeds(LED_IDX_SUDS1, param);
            env->ds.deviceAction->actLeds(LED_IDX_DOOR1, param);
            env->ds.deviceAction->actLeds(LED_IDX_AIR_DOOR, param);

            testStatus.stateStr = QString().asprintf("Test started - %d cycles", testParams.cycles);
            env->ds.testData->setTestStatus(testStatus);
            setState(STATE_FIND_PLUNGER_STARTED);
        }
        break;
    case STATE_REPEATING:
        LOG_INFO("TEST_PISTON: STATE_REPEATING\n");
        setState(STATE_FIND_PLUNGER_STARTED);
        break;
    case STATE_FIND_PLUNGER_STARTED:
        LOG_INFO("TEST_PISTON: STATE_FIND_PLUNGER_STARTED\n");
        setPistonHw();
        break;
    case STATE_FIND_PLUNGER_PROGRESS:
        LOG_INFO("TEST_PISTON: STATE_FIND_PLUNGER_PROGRESS\n");
        break;
    case STATE_FIND_PLUNGER_DONE:
        LOG_INFO("TEST_PISTON: STATE_FIND_PLUNGER_DONE\n");
        setState(STATE_STOPCOCK_TO_INJECT_STARTED);
        break;
    case STATE_STOPCOCK_TO_INJECT_STARTED:
        LOG_INFO("TEST_PISTON: STATE_STOPCOCK_TO_INJECT_STARTED\n");
        setStopcockHw();
        break;
    case STATE_STOPCOCK_TO_INJECT_PROGRESS:
        LOG_INFO("TEST_PISTON: STATE_STOPCOCK_TO_INJECT_PROGRESS\n");
        break;
    case STATE_STOPCOCK_TO_INJECT_DONE:
        LOG_INFO("TEST_PISTON: STATE_STOPCOCK_TO_INJECT_DONE\n");
        setState(STATE_PUSH_PLUNGER_STARTED);
        break;
    case STATE_PUSH_PLUNGER_STARTED:
        LOG_INFO("TEST_PISTON: STATE_PUSH_PLUNGER_STARTED\n");
        setPistonHw();
        break;
    case STATE_PUSH_PLUNGER_PROGRESS:
        LOG_INFO("TEST_PISTON: STATE_PUSH_PLUNGER_PROGRESS\n");
        break;
    case STATE_PUSH_PLUNGER_DONE:
        LOG_INFO("TEST_PISTON: STATE_PUSH_PLUNGER_DONE\n");
        setState(STATE_ENGAGE_STARTED);
        break;
    case STATE_ENGAGE_STARTED:
        LOG_INFO("TEST_PISTON: STATE_ENGAGE_STARTED\n");
        if (env->ds.mcuData->getMudsInserted())
        {
            if (env->ds.mcuData->getMudsInserted())
            {
                bool stateCompleted = false;
                DS_McuDef::PlungerStates plungerStates = env->ds.mcuData->getPlungerStates();
                stateCompleted = ( ( (!testParams.pistonEnabled[0]) || (plungerStates[SYRINGE_IDX_SALINE] == DS_McuDef::PLUNGER_STATE_ENGAGED) ) &&
                                   ( (!testParams.pistonEnabled[1]) || (plungerStates[SYRINGE_IDX_CONTRAST1] == DS_McuDef::PLUNGER_STATE_ENGAGED) ) &&
                                   ( (!testParams.pistonEnabled[2]) || (plungerStates[SYRINGE_IDX_CONTRAST2] == DS_McuDef::PLUNGER_STATE_ENGAGED) ) );
                if (stateCompleted)
                {
                    LOG_INFO("TEST_PISTON: STATE_ENGAGE_STARTED: Plunger(s) already engaged.\n");
                    setState(STATE_ENGAGE_DONE);
                    return;
                }
            }
        }
        setPistonHw();
        break;
    case STATE_ENGAGE_PROGRESS:
        LOG_INFO("TEST_PISTON: STATE_ENGAGE_PROGRESS\n");
        break;
    case STATE_ENGAGE_DONE:
        LOG_INFO("TEST_PISTON: STATE_ENGAGE_DONE\n");
        setState(STATE_INJECT_STARTED);
        break;
    case STATE_INJECT_STARTED:
        LOG_INFO("TEST_PISTON: STATE_INJECT_STARTED\n");
        setPistonHw();
        break;
    case STATE_INJECT_PROGRESS:
        LOG_INFO("TEST_PISTON: STATE_INJECT_PROGRESS\n");
        break;
    case STATE_INJECT_DONE:
        LOG_INFO("TEST_PISTON: STATE_INJECT_DONE\n");
        setState(STATE_STOPCOCK_TO_FILL_STARTED);
        break;
    case STATE_STOPCOCK_TO_FILL_STARTED:
        LOG_INFO("TEST_PISTON: STATE_STOPCOCK_TO_FILL_STARTED\n");
        setStopcockHw();
        break;
    case STATE_STOPCOCK_TO_FILL_PROGRESS:
        LOG_INFO("TEST_PISTON: STATE_STOPCOCK_TO_FILL_PROGRESS\n");
        break;
    case STATE_STOPCOCK_TO_FILL_DONE:
        LOG_INFO("TEST_PISTON: STATE_STOPCOCK_TO_FILL_DONE\n");
        setState(STATE_FILL_STARTED);
        break;
    case STATE_FILL_STARTED:
        LOG_INFO("TEST_PISTON: STATE_FILL_STARTED\n");
        setPistonHw();
        break;
    case STATE_FILL_PROGRESS:
        LOG_INFO("TEST_PISTON: STATE_FILL_PROGRESS\n");
        break;
    case STATE_FILL_DONE:
        LOG_INFO("TEST_PISTON: STATE_FILL_DONE\n");
        setState(STATE_FILL_PAUSE_STARTED);
        break;
    case STATE_FILL_PAUSE_STARTED:
        LOG_INFO("TEST_PISTON: STATE_FILL_PAUSE_STARTED\n");
        testStatus.stateStr = QString().asprintf("#%d: Pausing for %dms", testParams.curCycleCount + 1, testParams.pauseAfterFillMs);
        env->ds.testData->setTestStatus(testStatus);

        QTimer::singleShot(testParams.pauseAfterFillMs, [=] {
            setState(STATE_FILL_PAUSE_DONE);
        });
        setState(STATE_FILL_PAUSE_PROGRESS);
        break;
    case STATE_FILL_PAUSE_PROGRESS:
        LOG_INFO("TEST_PISTON: STATE_FILL_PAUSE_PROGRESS\n");
        break;
    case STATE_FILL_PAUSE_DONE:
        LOG_INFO("TEST_PISTON: STATE_FILL_PAUSE_DONE\n");
        if (testParams.disengageAfterFill)
        {
            setState(STATE_DISENGAGE_STARTED);
        }
        else
        {
            setState(STATE_COMPLETED);
        }
        break;
    case STATE_DISENGAGE_STARTED:
        LOG_INFO("TEST_PISTON: STATE_DISENGAGE_STARTED\n");
        setPistonHw();
        break;
    case STATE_DISENGAGE_PROGRESS:
        LOG_INFO("TEST_PISTON: STATE_DISENGAGE_PROGRESS\n");
        break;
    case STATE_DISENGAGE_DONE:
        LOG_INFO("TEST_PISTON: STATE_DISENGAGE_DONE\n");
        setState(STATE_DISENGAGE_PAUSE_STARTED);
        break;
    case STATE_DISENGAGE_PAUSE_STARTED:
        LOG_INFO("TEST_PISTON: STATE_DISENGAGE_PAUSE_STARTED\n");
        testStatus.stateStr = QString().asprintf("#%d: Pausing for %dms", testParams.curCycleCount + 1, testParams.pauseAfterFillMs);
        env->ds.testData->setTestStatus(testStatus);

        QTimer::singleShot(testParams.pauseAfterDisengageMs, [=] {
            setState(STATE_DISENGAGE_PAUSE_DONE);
        });
        setState(STATE_DISENGAGE_PAUSE_PROGRESS);
        break;
    case STATE_DISENGAGE_PAUSE_PROGRESS:
        LOG_INFO("TEST_PISTON: STATE_DISENGAGE_PAUSE_PROGRESS\n");
        break;
    case STATE_DISENGAGE_PAUSE_DONE:
        LOG_INFO("TEST_PISTON: STATE_DISENGAGE_PAUSE_DONE\n");
        setState(STATE_COMPLETED);
        break;
    case STATE_COMPLETED:
        LOG_INFO("TEST_PISTON: STATE_COMPLETED\n");
        testParams.curCycleCount++;
        testStatus.progress = (testParams.curCycleCount * 100) / testParams.cycles;
        testStatus.stateStr = QString().asprintf("Cycle completed %d/%d, progress=%d%%", testParams.curCycleCount, testParams.cycles, testStatus.progress);
        env->ds.testData->setTestStatus(testStatus);

        if (testParams.curCycleCount >= testParams.cycles)
        {
            setState(STATE_ALL_COMPLETED);
        }
        else
        {
            setState(STATE_REPEATING);
        }
        break;
    case STATE_ALL_COMPLETED:
        {
            LOG_INFO("TEST_PISTON: STATE_ALL_COMPLETED\n");
            DS_McuDef::ActLedParams param;
            param.setColorOff();
            env->ds.deviceAction->actLeds(LED_IDX_SALINE, param);
            env->ds.deviceAction->actLeds(LED_IDX_CONTRAST1, param);
            env->ds.deviceAction->actLeds(LED_IDX_CONTRAST2, param);
            env->ds.deviceAction->actLeds(LED_IDX_SUDS1, param);
            env->ds.deviceAction->actLeds(LED_IDX_DOOR1, param);
            env->ds.deviceAction->actLeds(LED_IDX_AIR_DOOR, param);

            testStatus.progress = 100;
            testStatus.stateStr = "All cycles completed\n";
            testStatus.isFinished = true;
            env->ds.testData->setTestStatus(testStatus);

            DataServiceActionStatus status;
            status.guid = testStatus.guid;
            status.state = DS_ACTION_STATE_COMPLETED;

            actionCompleted(status);
            setState(STATE_IDLE);
        }
        break;
    case STATE_ABORTED:
        {
            LOG_INFO("TEST_PISTON: STATE_ABORTED\n");

            DS_McuDef::ActLedParams param;
            param.setColorOff();
            env->ds.deviceAction->actLeds(LED_IDX_SALINE, param);
            env->ds.deviceAction->actLeds(LED_IDX_CONTRAST1, param);
            env->ds.deviceAction->actLeds(LED_IDX_CONTRAST2, param);
            env->ds.deviceAction->actLeds(LED_IDX_SUDS1, param);
            env->ds.deviceAction->actLeds(LED_IDX_DOOR1, param);
            env->ds.deviceAction->actLeds(LED_IDX_AIR_DOOR, param);

            DataServiceActionStatus status;
            status.guid = testStatus.guid;
            if (testStatus.userAborted)
            {
                status.state = DS_ACTION_STATE_ALLSTOP_BTN_ABORT;
                testStatus.stateStr = QString().asprintf("#%d: ERROR: Test aborted by user", testParams.curCycleCount + 1);
            }
            else
            {
                status.state = DS_ACTION_STATE_INTERNAL_ERR;
                testStatus.stateStr = QString().asprintf("#%d: ERROR: Test aborted with error", testParams.curCycleCount + 1);
            }
            testStatus.isFinished = true;
            testStatus.userAborted = false;
            env->ds.testData->setTestStatus(testStatus);
            actionCompleted(status);
            setState(STATE_IDLE);
        }
        break;
    default:
        LOG_ERROR("TEST_PISTON: Unknown state(%d)\n", state);
        break;
    }
}


