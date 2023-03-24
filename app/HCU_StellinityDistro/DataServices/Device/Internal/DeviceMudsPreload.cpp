#include <QtMath>
#include "Apps/AppManager.h"
#include "DeviceMudsPreload.h"
#include "Common/ImrParser.h"
#include "DataServices/Workflow/DS_WorkflowData.h"
#include "DataServices/Capabilities/DS_Capabilities.h"
#include "DataServices/Device/DS_DeviceAction.h"


DeviceMudsPreload::DeviceMudsPreload(QObject *parent, EnvGlobal *env_, EnvLocal *envLocal_) :
    ActionBaseExt(parent, env_)
{
    envLocal = new EnvLocal("DS_Device-MudsPreload", "DEVICE_MUDS_PRELOAD");

    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });

    // Initialisation
    state = STATE_READY;
}

DeviceMudsPreload::~DeviceMudsPreload()
{
    delete envLocal;
}

void DeviceMudsPreload::slotAppInitialised()
{
    connect(env->ds.workflowData, &DS_WorkflowData::signalDataChanged_WorkflowState, this, [=](DS_WorkflowDef::WorkflowState workflowState, DS_WorkflowDef::WorkflowState workflowStatePrev) {
        if ( (workflowState != DS_WorkflowDef::STATE_INACTIVE) &&
             (workflowStatePrev == DS_WorkflowDef::STATE_INACTIVE) )
        {
            state = STATE_READY;
            processState();
        }
        else if ( (workflowState == DS_WorkflowDef::STATE_INACTIVE) &&
                  (workflowStatePrev != DS_WorkflowDef::STATE_INACTIVE) )
        {
            env->actionMgr->deleteActAll(actStatusBuf.guid);
            state = STATE_INACTIVE;
        }
    });
}

DataServiceActionStatus DeviceMudsPreload::actMudsPreload(const DS_McuDef::InjectionProtocol &injectionProtocol, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "actMudsPreload", QString().asprintf("%s", Util::qVarientToJsonData(ImrParser::ToImr_McuInjectionProtocol(injectionProtocol)).CSTR()));

    if ( (state != STATE_READY) &&
         (state != STATE_INACTIVE) )
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = QString().asprintf("Preload in progress, state=%d", state);
        actionStarted(status);
        return status;
    }

    DataServiceActionStatus statusBuf = actGetPreloadPrimeParamsList(primeParamsList, injectionProtocol);

    if (statusBuf.state != DS_ACTION_STATE_COMPLETED)
    {
        LOG_ERROR("actMudsPreload(): Failed to get PrimeParamsList from InjectionProtocol: Status=%s\n", ImrParser::ToImr_DataServiceActionStatusStr(statusBuf).CSTR());
        actionStarted(status, &statusBuf);
        return status;
    }

    actStatusBuf = status;
    actStatusBuf.state = DS_ACTION_STATE_STARTED;

    setState(STATE_STARTED);

    actionStarted(actStatusBuf);
    return actStatusBuf;
}

DataServiceActionStatus DeviceMudsPreload::actGetPreloadPrimeParamsList(QList<DS_McuDef::ActPrimeParams> &primeParamsList, const DS_McuDef::InjectionProtocol &injectionProtocol, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "actGetPreloadPrimeParamsList", QString().asprintf("%s", Util::qVarientToJsonData(ImrParser::ToImr_McuInjectionProtocol(injectionProtocol)).CSTR()));

    actionStarted(status);

    double primeFlowRate = env->ds.capabilities->get_Preload_PreloadFlowRate();

    primeParamsList.clear();

    const DS_McuDef::InjectionPhases &phases = injectionProtocol.phases;
    for (int phaseIdx = 0; phaseIdx < phases.length(); phaseIdx++)
    {
        const DS_McuDef::InjectionPhase *phase = &phases[phaseIdx];

        DS_McuDef::ActPrimeParams primeParams;
        primeParams.flow = primeFlowRate;
        primeParams.operationName = "PreloadPrime";
        primeParams.vol2 = 0.1; // enable air-check

        switch (phase->type)
        {
        case DS_McuDef::INJECTION_PHASE_TYPE_CONTRAST1:
        case DS_McuDef::INJECTION_PHASE_TYPE_CONTRAST2:
            primeParams.vol1 = phase->vol;
            primeParams.idx = phase->type == DS_McuDef::INJECTION_PHASE_TYPE_CONTRAST1 ? SYRINGE_IDX_CONTRAST1 : SYRINGE_IDX_CONTRAST2;
            primeParamsList.append(primeParams);
            break;
        case DS_McuDef::INJECTION_PHASE_TYPE_SALINE:
            primeParams.vol1 = phase->vol;
            primeParams.idx = SYRINGE_IDX_SALINE;
            primeParamsList.append(primeParams);
            break;
        case DS_McuDef::INJECTION_PHASE_TYPE_DUAL1:
        case DS_McuDef::INJECTION_PHASE_TYPE_DUAL2:
            {
                // Add multiple iterations for the current phase
                int splitCount = 0;
                double contrastVolRemaining = phase->vol * phase->mix * 0.01;
                double salineVolRemaining = phase->vol * (100 - phase->mix) * 0.01;

                while ( (Util::isFloatVarGreaterThan(contrastVolRemaining, 2.0)) &&
                        (Util::isFloatVarGreaterThan(salineVolRemaining, 2.0)) )
                {
                    contrastVolRemaining = contrastVolRemaining / 2;
                    salineVolRemaining = salineVolRemaining / 2;
                    splitCount++;
                }

                int subPhaseCount = qPow(2, splitCount);
                LOG_DEBUG("actGetPreloadPrimeParamsList(): Dual phases split by %d times, %d sub-phases with %.2fml,%.2fml\n", splitCount, subPhaseCount, contrastVolRemaining, salineVolRemaining);


                for (int subPhaseIdx = 0; subPhaseIdx < subPhaseCount; subPhaseIdx++)
                {
                    // Add contrast phase first
                    primeParams.vol1 = contrastVolRemaining;
                    primeParams.idx = phase->type == DS_McuDef::INJECTION_PHASE_TYPE_DUAL1 ? SYRINGE_IDX_CONTRAST1 : SYRINGE_IDX_CONTRAST2;
                    primeParamsList.append(primeParams);

                    // Add saline phase first
                    primeParams.vol1 = salineVolRemaining;
                    primeParams.idx = SYRINGE_IDX_SALINE;
                    primeParamsList.append(primeParams);
                }
            }
            break;
        default:
            break;
        }
    }

    LOG_DEBUG("actGetPreloadPrimeParamsList(): PrimeParamList = \n%s\n", Util::qVarientToJsonData(ImrParser::ToImr_ActPrimeParamsList(primeParamsList), false).CSTR());

    actionCompleted(status);
    return status;
}

bool DeviceMudsPreload::isSetStateReady()
{
    if (state == STATE_INACTIVE)
    {
        return false;
    }
    return true;
}

void DeviceMudsPreload::processState()
{
    switch (state)
    {
    case STATE_INACTIVE:
        LOG_INFO("PRELOAD: STATE_INACTIVE\n");
        break;
    case STATE_READY:
        LOG_INFO("PRELOAD: STATE_READY\n");
        break;
    case STATE_STARTED:
        LOG_INFO("PRELOAD: STATE_STARTED\n");
        primeIndex = 0;
        setState(STATE_PRIME_PHASES_STARTED);
        break;
    case STATE_PRIME_PHASES_STARTED:
        LOG_INFO("PRELOAD: STATE_PRIME_PHASES_STARTED\n");
        handleSubAction(STATE_PRIME_PHASES_PROGRESS, STATE_PRIME_PHASES_DONE, STATE_PRIME_PHASES_FAILED);
        env->ds.deviceAction->actSyringePrime(primeParamsList[primeIndex], guidSubAction);
        break;
    case STATE_PRIME_PHASES_PROGRESS:
        LOG_INFO("PRELOAD: STATE_PRIME_PHASES_PROGRESS: Prime[%d]:\n", primeIndex);
        break;
    case STATE_PRIME_PHASES_FAILED:
        LOG_INFO("PRELOAD: STATE_PRIME_PHASES_FAILED\n");
        setState(STATE_FAILED);
        break;
    case STATE_PRIME_PHASES_DONE:
        LOG_INFO("PRELOAD: STATE_PRIME_PHASES_DONE\n");
        primeIndex++;
        if (primeIndex < primeParamsList.length())
        {
            setState(STATE_PRIME_PHASES_STARTED);
        }
        else
        {
            setState(STATE_DONE);
        }
        break;

    case STATE_FAILED:
        LOG_ERROR("PRELOAD: STATE_FAILED: Status=%s\n", ImrParser::ToImr_DataServiceActionStatusStr(actStatusBuf).CSTR());
        actionCompleted(actStatusBuf);
        setState(STATE_READY);
        break;
    case STATE_DONE:
        LOG_INFO("PRELOAD: STATE_DONE\n");
        actionCompleted(actStatusBuf);
        setState(STATE_READY);
        break;
    default:
        LOG_ERROR("PRELOAD: Unknown State(%d)\n", state);
        break;
    }
}
