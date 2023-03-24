#include "Common/ImrParser.h"
#include "McuSimSyringe.h"
#include "McuSimStopcock.h"
#include "McuSimStateMachine.h"
#include "DataServices/Mcu/DS_McuData.h"
#include "DataServices/Capabilities/DS_Capabilities.h"

McuSimStateMachine::McuSimStateMachine(QObject *parent,
                                       EnvGlobal *env_,
                                       EnvLocal *envLocal_,
                                       McuSimSyringeGroup *saline,
                                       McuSimSyringeGroup *contrast1,
                                       McuSimSyringeGroup *contrast2) :
    QObject(parent),
    env(env_),
    envLocal(envLocal_),
    salineGroup(saline),
    contrast1Group(contrast1),
    contrast2Group(contrast2)
{
    curState = MCU_STATE_FILL;
    requestedState = MCU_STATE_FILL;
    injectProgress.state = DS_McuDef::INJECTOR_STATE_IDLE;
    injectProgress.completeStatus.init();
    syrActState[0] = "COMPLETED";
    syrActState[1] = "COMPLETED";
    syrActState[2] = "COMPLETED";
    pausePhaseTimeRemaining = 0;

    connect(&tmrPausePhase, SIGNAL(timeout()), this, SLOT(slotTmrPausePhaseTimeout()));
    connect(&tmrHoldTimeout, SIGNAL(timeout()), this, SLOT(slotTmrHoldTimeout()));
    connect(&tmrArmTimeout, SIGNAL(timeout()), this, SLOT(slotTmrArmTimeout()));

    connect(salineGroup->syringe, SIGNAL(signalSyringeActionDone(QString)), this, SLOT(slotSalineActionComplete(QString)));
    connect(contrast1Group->syringe, SIGNAL(signalSyringeActionDone(QString)), this, SLOT(slotContrast1ActionComplete(QString)));
    connect(contrast2Group->syringe, SIGNAL(signalSyringeActionDone(QString)), this, SLOT(slotContrast2ActionComplete(QString)));
    connect(&tmrInjectDigest, SIGNAL(timeout()), this, SLOT(slotTmrInjectDigestTimeout()));
}

McuSimStateMachine::~McuSimStateMachine()
{
}

QString McuSimStateMachine::adjustFlow(double delta, QString errCodePrefix)
{
    QString err = "OK";
    if (curState == MCU_STATE_INJECTING)
    {
        DS_McuDef::InjectionPhase *curPhase = &curProtocol.phases[getCurPhase()];
        double newFlow = curPhase->flow + delta;

        // Set Min and Max flow rates
        double flowMin = env->ds.capabilities->get_Hidden_InjectionPhaseFlowRateMin();
        double flowMax = env->ds.capabilities->get_Hidden_InjectionPhaseFlowRateMax();

        if (Util::isFloatVarGreaterThan(flowMin, newFlow))
        {
            err = errCodePrefix + "InvalidParameter";
        }
        else if (Util::isFloatVarGreaterThan(newFlow, flowMax))
        {
            err = errCodePrefix + "InvalidParameter";
        }
        else
        {
            curPhase->flow = newFlow;
            switch (curPhase->type)
            {
            case DS_McuDef::INJECTION_PHASE_TYPE_CONTRAST1:
                contrast1Group->syringe->adjustFlow(curPhase->flow);
                break;
            case DS_McuDef::INJECTION_PHASE_TYPE_CONTRAST2:
                contrast2Group->syringe->adjustFlow(curPhase->flow);
                break;
            case DS_McuDef::INJECTION_PHASE_TYPE_SALINE:
                salineGroup->syringe->adjustFlow(curPhase->flow);
                break;
            case DS_McuDef::INJECTION_PHASE_TYPE_DUAL1:
                {
                    double contrastFlow = (curPhase->flow * curPhase->mix) / 100;
                    double salineFlow = (curPhase->mix == 50) ? contrastFlow : (curPhase->flow - contrastFlow);
                    contrast1Group->syringe->adjustFlow(contrastFlow);
                    salineGroup->syringe->adjustFlow(salineFlow);
                }
                break;
            case DS_McuDef::INJECTION_PHASE_TYPE_DUAL2:
                {
                    double contrastFlow = (curPhase->flow * curPhase->mix) / 100;
                    double salineFlow = (curPhase->mix == 50) ? contrastFlow : (curPhase->flow - contrastFlow);
                    contrast2Group->syringe->adjustFlow(contrastFlow);
                    salineGroup->syringe->adjustFlow(salineFlow);
                }
                break;
            default:
                err = errCodePrefix + "InvalidState";
                break;
            }
        }
    }
    else
    {
        err = errCodePrefix + "InvalidState";
    }

    return err;
}

void McuSimStateMachine::setState(State state)
{
    curState = state;
}

McuSimStateMachine::State McuSimStateMachine::getState()
{
    return curState;
}

QString McuSimStateMachine::setInjectState(QString injState, QString errCodePrefix)
{
    QString err = "OK";
    if (injState == _L("ARM"))
    {
        if (curState == MCU_STATE_FILL)
        {
            injectProgress.state = DS_McuDef::INJECTOR_STATE_READY_START;
            curState = MCU_STATE_ARMED;
            clearInjectDigest();
            tmrArmTimeout.start(INJECTOR_ARM_TIMEOUT_MS);
        }
        else
        {
            err = errCodePrefix + "StateNotIdle";
        }
    }
    else if (injState == _L("DISARM"))
    {
        if (curState == MCU_STATE_ARMED)
        {
            disarm(DS_McuDef::INJECTION_COMPLETE_REASON_DISARMED_HCU_ABORT);
        }
        else
        {
            err = errCodePrefix + "StateNotArmed";
        }
    }
    else
    {
        LOG_ERROR("SM: Bad inject state request(%s)\n", injState.CSTR());
    }

    return err;
}

QString McuSimStateMachine::setActiveProtocol(QStringList params, QString errCodePrefix)
{
    QString err = "OK";
    if (curState == MCU_STATE_FILL)
    {
        int paramIdxOffset = 5;
        if (params.length() > paramIdxOffset)
        {
            curProtocol.pressureLimit = params.at(0).toInt();
            curProtocol.phases.clear();
            int phaseCount = params.at(1).toInt();
            //ignore params.at(2) - this is the line primed with saline or contrast parameter

            if (params.length() >= ((phaseCount * 5) + paramIdxOffset) )
            {
                int paramCount = paramIdxOffset;
                for (int i = 0; i < phaseCount; i++)
                {
                    bool ok, tempOk;
                    DS_McuDef::InjectionPhase phase;
                    phase.type = ImrParser::ToCpp_McuInjectionPhaseType(params.at(paramCount++));
                    phase.mix = params.at(paramCount++).toInt(&ok);
                    tempOk = ok;
                    phase.vol = params.at(paramCount++).toDouble(&ok);
                    tempOk &= ok;
                    phase.flow = params.at(paramCount++).toDouble(&ok);
                    tempOk &= ok;
                    phase.duration = params.at(paramCount++).toInt(&ok);
                    tempOk &= ok;

                    curProtocol.phases.append(phase);

                    if (!tempOk)
                    {
                        err = errCodePrefix + "InvalidInjection";
                        break;
                    }
                }
            }
            else
            {
                err = errCodePrefix + "InvalidInjection";
            }
        }
        else
        {
            err = errCodePrefix + "InvalidInjection";
        }
    }
    else
    {
        err = errCodePrefix + "StateNotIdle";
    }

    if (err != _L("OK"))
    {
         LOG_ERROR("SM: ARM: err='%s'\n", err.CSTR());
    }

    return err;
}

QString McuSimStateMachine::setSyringeAction(int syringeIdx, QString action, QString errCodePrefix, double param1, double param2, double param3, double param4)
{
    QString err = "OK";

    if (curState == MCU_STATE_FILL)
    {
        double vol = param1;
        double flow = (action == _L("PRIME")) ? param3 : param2;
        bool airCheck = param4 == 1;

        McuSimSyringeGroup *syringeGroup = NULL;

        switch (syringeIdx)
        {
        case 0:
            syringeGroup = salineGroup;
            break;
        case 1:
            syringeGroup = contrast1Group;
            break;
        case 2:
            syringeGroup = contrast2Group;
            break;
        default:
            err = errCodePrefix + "InvalidParameter";
            return err;
        }

        syrActState[syringeIdx] = "PROCESSING";
        err = syringeGroup->syringe->startAction(action, errCodePrefix, vol, flow, airCheck);
        if (action == _L("STOP"))
        {
            if (syringeGroup->syringe->getInfo().curAction == _L("NONE"))
            {
                syrActState[syringeIdx] = "COMPLETED";
            }
        }
    }
    else
    {
        LOG_ERROR("SM: setSyringeAction(): Unexpected MCU State. Expected='MCU_STATE_FILL', current=%d\n", curState);
        err = errCodePrefix + "InvalidState";
    }
    return err;
}

int McuSimStateMachine::getCurPhase()
{
    DS_McuDef::InjectDigest hwInjectDigest = env->ds.mcuData->getSimInjectDigest();

    if (hwInjectDigest.phaseIdx >= MAX_MCU_PHASES)
    {
        hwInjectDigest.phaseIdx = MAX_MCU_PHASES - 1;
    }
    env->ds.mcuData->setSimInjectDigest(hwInjectDigest);
    return hwInjectDigest.phaseIdx;
}

QString McuSimStateMachine::injectStart(QString errCodePrefix)
{
    QString err = "OK";

    if (curState == MCU_STATE_ARMED)
    {
        LOG_DEBUG("SM: injectStart(): Injection Started\n");
        //Start injection progress
        curState = MCU_STATE_INJECTING;

        // Start inject digest timer
        tmrInjectDigest.start(50);

        // Reset neccessary parameters when injection starts
        DS_McuDef::InjectDigest hwInjectDigest = env->ds.mcuData->getSimInjectDigest();
        hwInjectDigest.phaseIdx = 0;
        pausePhaseTimeRemaining = 0;
        env->ds.mcuData->setSimInjectDigest(hwInjectDigest);

        initiateNextPhase();

        tmrArmTimeout.stop();
    }
    else if (curState == MCU_STATE_INJECTING)
    {
        LOG_DEBUG("SM: injectStart(): Injection Resumed\n");
        resume();
    }
    else
    {
        err = errCodePrefix + "InvalidState";
    }
    return err;
}

QString McuSimStateMachine::injectStop(QString errCodePrefix)
{
    QString err = "OK";

    switch (curState)
    {
    case MCU_STATE_ARMED:
        disarm(DS_McuDef::INJECTION_COMPLETE_REASON_DISARMED_HCU_ABORT);
        break;
    case MCU_STATE_INJECTING:
        abortInjection(DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_USER_ABORT);
        break;
    default:
        err = errCodePrefix + "InvalidState";
        break;
    }

    return err;
}

QString McuSimStateMachine::injectHold(QString errCodePrefix)
{
    QString err = "OK";

    if (curState == MCU_STATE_INJECTING)
    {
        pauseInjection();
    }
    else
    {
        err = errCodePrefix + "InvalidState";
    }
    return err;
}

QString McuSimStateMachine::injectJump(int toIdx, QString errCodePrefix)
{
    QString err = "OK";

    if (curState == MCU_STATE_INJECTING)
    {
        err = phaseJump(toIdx);
    }
    else
    {
        err = errCodePrefix + "InvalidState";
    }
    return err;
}

void McuSimStateMachine::disarm(DS_McuDef::InjectionCompleteReason disarmReason)
{
    injectProgress.completeStatus.reason = disarmReason;
    injectProgress.state = DS_McuDef::INJECTOR_STATE_IDLE;
    curState = MCU_STATE_FILL;
    tmrArmTimeout.stop();
}

void McuSimStateMachine::abortInjection(DS_McuDef::InjectionCompleteReason completeReason)
{
    if (injectProgress.state == DS_McuDef::INJECTOR_STATE_DELIVERING)
    {
        salineGroup->syringe->stopAction("USER_ABORT");
        contrast1Group->syringe->stopAction("USER_ABORT");
        contrast2Group->syringe->stopAction("USER_ABORT");
        endInjection(completeReason);
    }
    else if (injectProgress.state == DS_McuDef::INJECTOR_STATE_HOLDING)
    {
        salineGroup->syringe->resume();
        contrast1Group->syringe->resume();
        contrast2Group->syringe->resume();
        salineGroup->syringe->stopAction("USER_ABORT");
        contrast1Group->syringe->stopAction("USER_ABORT");
        contrast2Group->syringe->stopAction("USER_ABORT");
        endInjection(completeReason);
    }
    else if (injectProgress.state == DS_McuDef::INJECTOR_STATE_PHASE_PAUSED)
    {
        if (tmrPausePhase.isActive())
        {
            tmrPausePhase.stop();
        }
        endInjection(completeReason);
    }
}

void McuSimStateMachine::pauseInjection()
{
    if (injectProgress.state == DS_McuDef::INJECTOR_STATE_DELIVERING)
    {
        salineGroup->syringe->pause();
        contrast1Group->syringe->pause();
        contrast2Group->syringe->pause();
        injectProgress.state = DS_McuDef::INJECTOR_STATE_HOLDING;
    }
    else if (injectProgress.state == DS_McuDef::INJECTOR_STATE_PHASE_PAUSED)
    {
        if (tmrPausePhase.isActive())
        {
            pausePhaseTimeRemaining = tmrPausePhase.remainingTime();
            tmrPausePhase.stop();
            injectProgress.state = DS_McuDef::INJECTOR_STATE_HOLDING;
        }
    }

    tmrHoldTimeout.start(INJECTOR_ARM_TIMEOUT_MS);
}

void McuSimStateMachine::skipPhase()
{
    if (injectProgress.state == DS_McuDef::INJECTOR_STATE_DELIVERING)
    {
        salineGroup->syringe->stopAction("COMPLETE");
        contrast1Group->syringe->stopAction("COMPLETE");
        contrast2Group->syringe->stopAction("COMPLETE");
    }
    else if (injectProgress.state == DS_McuDef::INJECTOR_STATE_PHASE_PAUSED)
    {
        if (tmrPausePhase.isActive())
        {
            tmrPausePhase.stop();
            slotTmrPausePhaseTimeout();
        }
    }
}

QString McuSimStateMachine::phaseJump(int jumpToIdx)
{
    QString err = "T_INJECTFAILED_NoJumpPhase";
    DS_McuDef::InjectDigest hwInjectDigest = env->ds.mcuData->getSimInjectDigest();

    if (injectProgress.state == DS_McuDef::INJECTOR_STATE_DELIVERING)
    {
        if ( (jumpToIdx > hwInjectDigest.phaseIdx) &&
             (jumpToIdx < curProtocol.phases.length()) )
        {
            hwInjectDigest.phaseIdx = jumpToIdx - 1;
            env->ds.mcuData->setSimInjectDigest(hwInjectDigest);
            salineGroup->syringe->stopAction("COMPLETE");
            contrast1Group->syringe->stopAction("COMPLETE");
            contrast2Group->syringe->stopAction("COMPLETE");

            err = "OK";
        }
    }
    else if (injectProgress.state == DS_McuDef::INJECTOR_STATE_PHASE_PAUSED)
    {
        if (  (jumpToIdx > hwInjectDigest.phaseIdx) &&
              (jumpToIdx < curProtocol.phases.length()) &&
              (tmrPausePhase.isActive()) )
        {
            tmrPausePhase.stop();

            hwInjectDigest.phaseIdx = jumpToIdx - 1;
            env->ds.mcuData->setSimInjectDigest(hwInjectDigest);
            salineGroup->syringe->stopAction("COMPLETE");
            contrast1Group->syringe->stopAction("COMPLETE");
            contrast2Group->syringe->stopAction("COMPLETE");

            slotTmrPausePhaseTimeout();
            err = "OK";
        }
    }

    return err;
}

void McuSimStateMachine::resume()
{
    if (injectProgress.state == DS_McuDef::INJECTOR_STATE_HOLDING)
    {
        if (pausePhaseTimeRemaining > 0)
        {
            // Resumed in pause phase
            tmrPausePhase.start(pausePhaseTimeRemaining);
            injectProgress.state = DS_McuDef::INJECTOR_STATE_PHASE_PAUSED;
        }
        else
        {
            // Resumed in deliver phase
            salineGroup->syringe->resume();
            contrast1Group->syringe->resume();
            contrast2Group->syringe->resume();
            injectProgress.state = DS_McuDef::INJECTOR_STATE_DELIVERING;
        }

        tmrHoldTimeout.stop();
    }
}

DS_McuDef::InjectorState McuSimStateMachine::getInjectorState()
{
    return injectProgress.state;
}

DS_McuDef::InjectionCompleteStatus McuSimStateMachine::getInjectionCompleteStatus()
{
    return injectProgress.completeStatus;
}

void McuSimStateMachine::initiateNextPhase()
{
    int phaseIdx = getCurPhase();

    DS_McuDef::InjectionPhase *curPhase = &curProtocol.phases[phaseIdx];
    tmrPausePhase.stop();

    //Start elapsed timer for the phase duration
    elapsedPhaseTimer.start();

    LOG_DEBUG("SM: initiateNextPhase(): Initiating next phase: index=%d, %s\n", phaseIdx, ImrParser::ToImr_InjectionPhaseType(curPhase->type).CSTR());

    switch (curPhase->type)
    {
    case DS_McuDef::INJECTION_PHASE_TYPE_NONE:
        break;
    case DS_McuDef::INJECTION_PHASE_TYPE_PAUSE:
        tmrPausePhase.start(curPhase->duration);
        injectProgress.state = DS_McuDef::INJECTOR_STATE_PHASE_PAUSED;
        break;
    case DS_McuDef::INJECTION_PHASE_TYPE_CONTRAST1:
        contrast1Group->stopCock->setPosition("INJECT");
        contrast2Group->stopCock->setPosition("CLOSED");
        salineGroup->stopCock->setPosition("CLOSED");
        injectProgress.state = DS_McuDef::INJECTOR_STATE_DELIVERING;
        contrast1Group->syringe->startAction("INJECT", "T_INJECTFAILED", curPhase->vol, curPhase->flow);
        break;
    case DS_McuDef::INJECTION_PHASE_TYPE_CONTRAST2:
        contrast1Group->stopCock->setPosition("CLOSED");
        contrast2Group->stopCock->setPosition("INJECT");
        salineGroup->stopCock->setPosition("CLOSED");
        injectProgress.state = DS_McuDef::INJECTOR_STATE_DELIVERING;
        contrast2Group->syringe->startAction("INJECT", "T_INJECTFAILED", curPhase->vol, curPhase->flow);
        break;
    case DS_McuDef::INJECTION_PHASE_TYPE_SALINE:
        contrast1Group->stopCock->setPosition("CLOSED");
        contrast2Group->stopCock->setPosition("CLOSED");
        salineGroup->stopCock->setPosition("INJECT");
        injectProgress.state = DS_McuDef::INJECTOR_STATE_DELIVERING;
        salineGroup->syringe->startAction("INJECT", "T_INJECTFAILED", curPhase->vol, curPhase->flow);
        break;
    case DS_McuDef::INJECTION_PHASE_TYPE_DUAL1:
        {
            contrast1Group->stopCock->setPosition("INJECT");
            contrast2Group->stopCock->setPosition("CLOSED");
            salineGroup->stopCock->setPosition("INJECT");
            injectProgress.state = DS_McuDef::INJECTOR_STATE_DELIVERING;
            double contrastFlow = (curPhase->flow * curPhase->mix) / 100.0;
            double contrastVol = (curPhase->vol * curPhase->mix) / 100.0;
            double salineFlow = (curPhase->mix == 50) ? contrastFlow : (curPhase->flow - contrastFlow);
            double salineVol = (curPhase->mix == 50) ? contrastVol : (curPhase->vol - contrastVol);
            salineGroup->syringe->startAction("INJECT", "T_INJECTFAILED", salineVol, salineFlow);
            contrast1Group->syringe->startAction("INJECT", "T_INJECTFAILED", contrastVol, contrastFlow);
        }
        break;
    case DS_McuDef::INJECTION_PHASE_TYPE_DUAL2:
        {
            contrast1Group->stopCock->setPosition("CLOSED");
            contrast2Group->stopCock->setPosition("INJECT");
            salineGroup->stopCock->setPosition("INJECT");
            injectProgress.state = DS_McuDef::INJECTOR_STATE_DELIVERING;
            double contrastFlow = (curPhase->flow * curPhase->mix) / 100.0;
            double contrastVol = (curPhase->vol * curPhase->mix) / 100.0;
            double salineFlow = curPhase->flow - contrastFlow;
            double salineVol = curPhase->vol - contrastVol;
            salineGroup->syringe->startAction("INJECT", "T_INJECTFAILED", salineVol, salineFlow);
            contrast2Group->syringe->startAction("INJECT", "T_INJECTFAILED", contrastVol, contrastFlow);
        }
        break;
    default:
        LOG_ERROR("SM: Starting unknown phase type '%d'\n", curPhase->type);
        break;
    }
}

QString McuSimStateMachine::resetMuds(QString errCodePrefix)
{
    if (curState == MCU_STATE_FILL)
    {
        if (!salineGroup->syringe->resetSyringe())
        {
            return errCodePrefix + "PlungerMoveFailed1";
        }
        else if (!contrast1Group->syringe->resetSyringe())
        {
            return errCodePrefix + "PlungerMoveFailed2";
        }
        else if (!contrast2Group->syringe->resetSyringe())
        {
            return errCodePrefix + "PlungerMoveFailed3";
        }
        else
        {
            return "OK";
        }
    }

    return errCodePrefix + "PlungerNotEngaged";
}

void McuSimStateMachine::slotSalineActionComplete(QString result)
{
    //Stop elapsed timer for the phase transition

    if (curState == MCU_STATE_INJECTING)
    {
        if ( (curProtocol.phases[getCurPhase()].type == DS_McuDef::INJECTION_PHASE_TYPE_DUAL1) ||
             (curProtocol.phases[getCurPhase()].type == DS_McuDef::INJECTION_PHASE_TYPE_DUAL2) )
        {
            //Do nothing
        }
        else
        {
            if (injectProgress.state != DS_McuDef::INJECTOR_STATE_COMPLETING)
            {
                handlePhaseTransition();
            }
        }
    }
    else
    {
        syrActState[0] = result;
    }
}

void McuSimStateMachine::slotContrast1ActionComplete(QString result)
{
    //Stop elapsed timer for the phase transition

    if (curState == MCU_STATE_INJECTING)
    {
        if (curProtocol.phases[getCurPhase()].type == DS_McuDef::INJECTION_PHASE_TYPE_DUAL1)
        {
            if (salineGroup->syringe->getInfo().actionDoneState == _L("PROCESSING"))
            {
                LOG_DEBUG("SM: Dual flow completed only in contrast. Waiting for saline to be completed\n");
                QTimer::singleShot(50, this, [=]{
                    slotContrast1ActionComplete(result);
                });
                return;
            }
        }
        if (injectProgress.state != DS_McuDef::INJECTOR_STATE_COMPLETING)
        {
            handlePhaseTransition();
        }
    }
    else
    {
        syrActState[1] = result;
    }
}

void McuSimStateMachine::slotContrast2ActionComplete(QString result)
{
    //Stop elapsed timer for the phase transition

    if (curState == MCU_STATE_INJECTING)
    {
        if (curProtocol.phases[getCurPhase()].type == DS_McuDef::INJECTION_PHASE_TYPE_DUAL2)
        {
            salineGroup->syringe->stopAction("COMPLETED");
        }
        if (injectProgress.state != DS_McuDef::INJECTOR_STATE_COMPLETING)
        {
            handlePhaseTransition();
        }
    }
    else
    {
        syrActState[2] = result;
    }
}

void McuSimStateMachine::handlePhaseTransition()
{
    DS_McuDef::InjectDigest hwInjectDigest = env->ds.mcuData->getSimInjectDigest();

    hwInjectDigest.phaseIdx++;

    if (hwInjectDigest.phaseIdx >= curProtocol.phases.length())
    {
        hwInjectDigest.phaseIdx = curProtocol.phases.length() - 1;
        LOG_DEBUG("SM: handlePhaseTransition(): COMPLETED: hwInjectDigest.phaseIdx=%d\n", hwInjectDigest.phaseIdx);
        env->ds.mcuData->setSimInjectDigest(hwInjectDigest);
        endInjection(DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_NORMAL);
    }
    else
    {
        LOG_DEBUG("SM: handlePhaseTransition(): IN_PROGRESS: hwInjectDigest.phaseIdx=%d\n", hwInjectDigest.phaseIdx);
        env->ds.mcuData->setSimInjectDigest(hwInjectDigest);
        initiateNextPhase();
    }
}

void McuSimStateMachine::slotTmrPausePhaseTimeout()
{
    if (curState == MCU_STATE_INJECTING)
    {
        pausePhaseTimeRemaining = 0;
        handlePhaseTransition();
    }
}

void McuSimStateMachine::slotTmrHoldTimeout()
{
    tmrHoldTimeout.stop();
    if (curState == MCU_STATE_INJECTING)
    {
        abortInjection(DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_HOLD_TIMEOUT);
    }
}

void McuSimStateMachine::slotTmrArmTimeout()
{
    tmrArmTimeout.stop();
    if (curState == MCU_STATE_ARMED)
    {
        disarm(DS_McuDef::INJECTION_COMPLETE_REASON_DISARMED_ARM_TIMEOUT);
    }
}

void McuSimStateMachine::endInjection(DS_McuDef::InjectionCompleteReason completeReason)
{
    tmrInjectDigest.stop();
    elapsedPhaseTimer.invalidate();

    injectProgress.completeStatus.reason = completeReason;
    injectProgress.state = DS_McuDef::INJECTOR_STATE_COMPLETING;

    tmrHoldTimeout.stop();
    QTimer().singleShot(2000, this, SLOT(slotSetInjectorStateDone()));
}

void McuSimStateMachine::slotSetInjectorStateDone()
{
    //This is used to simulate the wait for pressure to dissipate
    salineGroup->stopCock->setPosition("CLOSED");
    contrast1Group->stopCock->setPosition("CLOSED");
    contrast2Group->stopCock->setPosition("CLOSED");

    injectProgress.state = DS_McuDef::INJECTOR_STATE_COMPLETED;
    curState = MCU_STATE_FILL;
    QTimer().singleShot(500, this, SLOT(slotResetInjectorState()));

}

void McuSimStateMachine::slotResetInjectorState()
{
   injectProgress.state = DS_McuDef::INJECTOR_STATE_IDLE;
}

void McuSimStateMachine::triggerSudsRemoved()
{
    switch (curState)
    {
    case MCU_STATE_ARMED:
        disarm(DS_McuDef::INJECTION_COMPLETE_REASON_DISARMED_SUDS_MISSING);
        break;
    case MCU_STATE_INJECTING:
        abortInjection(DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_SUDS_MISSING);
        break;
    case MCU_STATE_FILL:
        if (salineGroup->syringe->getInfo().curAction == _L("PRIME"))
        {
            salineGroup->syringe->stopAction("SUDS_REMOVED");
        }

        if (contrast1Group->syringe->getInfo().curAction == _L("PRIME"))
        {
            contrast1Group->syringe->stopAction("SUDS_REMOVED");
        }

        if (contrast2Group->syringe->getInfo().curAction == _L("PRIME"))
        {
            contrast2Group->syringe->stopAction("SUDS_REMOVED");
        }
        break;
    default:
        break;
    }
}

void McuSimStateMachine::triggerSudsInserted()
{
    if (curState == MCU_STATE_FILL)
    {
        if (salineGroup->syringe->getInfo().curAction == _L("PURGE"))
        {
            salineGroup->syringe->stopAction("SUDS_INSERTED");
        }

        if (contrast1Group->syringe->getInfo().curAction == _L("PURGE"))
        {
            contrast1Group->syringe->stopAction("SUDS_INSERTED");
        }

        if (contrast2Group->syringe->getInfo().curAction == _L("PURGE"))
        {
            contrast2Group->syringe->stopAction("SUDS_INSERTED");
        }
    }
}

void McuSimStateMachine::triggerMudsUnlatched()
{
    switch (curState)
    {
    case MCU_STATE_ARMED:
        disarm(DS_McuDef::INJECTION_COMPLETE_REASON_DISARMED_MUDS_UNLATCHED);
        break;
    case MCU_STATE_INJECTING:
        abortInjection(DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_MUDS_UNLATCHED);
        break;
    default:
        break;
    }
}

void McuSimStateMachine::triggerAirDetected()
{
    if (curState == MCU_STATE_INJECTING)
    {
        abortInjection(DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_AIR_DETECTED);
    }
    else if (curState == MCU_STATE_FILL)
    {
        if (salineGroup->syringe->getInfo().curAction == _L("PRIME"))
        {
            if (salineGroup->syringe->getInfo().airCheck)
            {
                salineGroup->syringe->stopAction("AIR_DETECTED");
            }
        }

        if (contrast1Group->syringe->getInfo().curAction == _L("PRIME"))
        {
            if (contrast1Group->syringe->getInfo().airCheck)
            {
                contrast1Group->syringe->stopAction("AIR_DETECTED");
            }
        }

        if (contrast2Group->syringe->getInfo().curAction == _L("PRIME"))
        {
            if (contrast2Group->syringe->getInfo().airCheck)
            {
                contrast2Group->syringe->stopAction("AIR_DETECTED");
            }
        }
    }
}

void McuSimStateMachine::triggerStopButtonClicked()
{
    switch (curState)
    {
    case MCU_STATE_ARMED:
        disarm(DS_McuDef::INJECTION_COMPLETE_REASON_DISARMED_STOP_BUTTON_ABORT);
        break;
    case MCU_STATE_INJECTING:
        abortInjection(DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_ALLSTOP_ABORT );
        break;
    default:
        salineGroup->syringe->stopAction("USER_ABORT");
        contrast1Group->syringe->stopAction("USER_ABORT");
        contrast2Group->syringe->stopAction("USER_ABORT");
        break;
    }
}

void McuSimStateMachine::triggerBatteryCritical()
{
    switch (curState)
    {
    case MCU_STATE_INJECTING:
        abortInjection(DS_McuDef::INJECTION_COMPLETE_REASON_COMPLETED_BATTERY_CRITICAL);
        break;
    default:
        break;
    }
}

void McuSimStateMachine::triggerSalineAirDetector()
{
    if (curState == MCU_STATE_FILL)
    {
        McuSimSyringe::Status syrInfo = salineGroup->syringe->getInfo();
        if (syrInfo.curAction == _L("FILL"))
        {
            salineGroup->syringe->stopAction("AIR_DETECTED");
        }
    }
}

void McuSimStateMachine::triggerContrast1AirDetector()
{
    if (curState == MCU_STATE_FILL)
    {
        McuSimSyringe::Status syrInfo = contrast1Group->syringe->getInfo();
        if (syrInfo.curAction == _L("FILL"))
        {
            contrast1Group->syringe->stopAction("AIR_DETECTED");
        }
    }
}

void McuSimStateMachine::triggerContrast2AirDetector()
{
    if (curState == MCU_STATE_FILL)
    {
        McuSimSyringe::Status syrInfo = contrast2Group->syringe->getInfo();
        if (syrInfo.curAction == _L("FILL"))
        {
            contrast2Group->syringe->stopAction("AIR_DETECTED");
        }
    }
}

QString McuSimStateMachine::getSyrActState(int idx)
{
    if ( (idx >= 0) && (idx <= 2) )
    {
        return syrActState[idx];
    }
    return "UNKNOWN";
}

void McuSimStateMachine::setSyringeActState(int idx, QString state)
{
    if ( (idx >= 0) && (idx <= 2) )
    {
        syrActState[idx] = state;
    }
}

void McuSimStateMachine::slotTmrInjectDigestTimeout()
{
    // Phase count starts from 0
    DS_McuDef::InjectDigest hwInjectDigest = env->ds.mcuData->getSimInjectDigest();

    if ( (hwInjectDigest.phaseIdx >= 0) && (hwInjectDigest.phaseIdx < MAX_MCU_PHASES) )
    {
        hwInjectDigest.phaseInjectDigests[hwInjectDigest.phaseIdx].duration = elapsedPhaseTimer.elapsed();

        const DS_McuDef::InjectionPhase *curPhase = &curProtocol.phases[getCurPhase()];

        hwInjectDigest.syringeInjectDigests[SYRINGE_IDX_SALINE].flowRate = curPhase->flow * ((100 - curPhase->mix) / 100.0);
        hwInjectDigest.syringeInjectDigests[SYRINGE_IDX_CONTRAST1].flowRate = 0;
        hwInjectDigest.syringeInjectDigests[SYRINGE_IDX_CONTRAST2].flowRate = 0;

        switch (curPhase->type)
        {
        case DS_McuDef::INJECTION_PHASE_TYPE_CONTRAST1:
        case DS_McuDef::INJECTION_PHASE_TYPE_DUAL1:
            hwInjectDigest.syringeInjectDigests[SYRINGE_IDX_CONTRAST1].flowRate = curPhase->flow * (curPhase->mix / 100.0);
            break;
        case DS_McuDef::INJECTION_PHASE_TYPE_CONTRAST2:
        case DS_McuDef::INJECTION_PHASE_TYPE_DUAL2:
            hwInjectDigest.syringeInjectDigests[SYRINGE_IDX_CONTRAST2].flowRate = curPhase->flow * (curPhase->mix / 100.0);
            break;
        default:
            break;
        }

        DS_McuDef::PhaseInjectDigest *curPhaseInjectDigest = &hwInjectDigest.phaseInjectDigests[hwInjectDigest.phaseIdx];

        switch (curPhase->type)
        {
        case DS_McuDef::INJECTION_PHASE_TYPE_CONTRAST1:
            curPhaseInjectDigest->volumes[SYRINGE_IDX_CONTRAST1] = contrast1Group->syringe->getInjectedVol();
            break;
        case DS_McuDef::INJECTION_PHASE_TYPE_CONTRAST2:
            curPhaseInjectDigest->volumes[SYRINGE_IDX_CONTRAST2] = contrast2Group->syringe->getInjectedVol();
            break;
        case DS_McuDef::INJECTION_PHASE_TYPE_SALINE:
            curPhaseInjectDigest->volumes[SYRINGE_IDX_SALINE] = salineGroup->syringe->getInjectedVol();
            break;
        case DS_McuDef::INJECTION_PHASE_TYPE_DUAL1:
            curPhaseInjectDigest->volumes[SYRINGE_IDX_SALINE] = salineGroup->syringe->getInjectedVol();
            curPhaseInjectDigest->volumes[SYRINGE_IDX_CONTRAST1] = contrast1Group->syringe->getInjectedVol();
            break;
        case DS_McuDef::INJECTION_PHASE_TYPE_DUAL2:
            curPhaseInjectDigest->volumes[SYRINGE_IDX_SALINE] = salineGroup->syringe->getInjectedVol();
            curPhaseInjectDigest->volumes[SYRINGE_IDX_CONTRAST2] = contrast2Group->syringe->getInjectedVol();
            break;
        default:
            // Only update the duration
            break;
        }
    }
    else
    {
        LOG_ERROR("SM: Index Out Of Range - injectDigest.phaseIdx = %d\n", hwInjectDigest.phaseIdx);
    }

    env->ds.mcuData->setSimInjectDigest(hwInjectDigest);
}

void McuSimStateMachine::clearInjectDigest()
{
    DS_McuDef::InjectDigest hwInjectDigest;
    hwInjectDigest.init();
    env->ds.mcuData->setSimInjectDigest(hwInjectDigest);
}
