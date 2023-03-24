#include "Apps/AppManager.h"
#include "DS_McuAction.h"
#include "Common/Util.h"
#include "Common/ImrParser.h"
#include "Common/McuHardware.h"
#include "DataServices/Mcu/DS_McuData.h"
#include "DataServices/Device/DS_DeviceData.h"
#include "DataServices/Alert/DS_AlertAction.h"
#include "DataServices/CfgLocal/DS_CfgLocal.h"
#include "DataServices/Capabilities/DS_Capabilities.h"
#include "DataServices/HardwareInfo/DS_HardwareInfo.h"
#include "DataServices/CfgGlobal/DS_CfgGlobal.h"

DS_McuAction::DS_McuAction(QObject *parent, EnvGlobal *env_) :
    ActionBase(parent, env_)
{
    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });

    envLocal = new EnvLocal("DS_Mcu-Action", "MCU_ACTION", LOG_LRG_SIZE_BYTES);
    monitor = new McuMonitor(this, env);
    alarmMonitor = new McuAlarmMonitor(this, env);

    mcuLink = new McuLink(NULL, env);
    mcuPressureCalibrationHandler = new McuPressureCalibration(this, env);

    adjustFlowRateBusy = false;

    actResetMcuHw();
}

DS_McuAction::~DS_McuAction()
{
    delete mcuLink;
    delete mcuPressureCalibrationHandler;
    delete monitor;
    delete alarmMonitor;
    delete envLocal;
}

void DS_McuAction::slotAppInitialised()
{
    actRestoreLastMcuDigest();

    connect(env->ds.systemData, &DS_SystemData::signalDataChanged_StatePath, this, [=](DS_SystemDef::StatePath curStatePath, DS_SystemDef::StatePath prevStatePath) {
        if ( ( (prevStatePath == DS_SystemDef::STATE_PATH_IDLE) && (curStatePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING) ) ||
             ( (prevStatePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING) && (curStatePath == DS_SystemDef::STATE_PATH_IDLE) ) )
        {
            LOG_INFO("\n\n-----------------\nStatePath=%s\n\n", ImrParser::ToImr_StatePath(curStatePath).CSTR());
        }
    });

    connect(mcuLink, &McuLink::signalRxMsg, [=](QString guid, QString rxMsg) {
        mutexActStatusMap.lock();

        // Find ActionStatus from List
        if (!actStatusMap.contains(guid))
        {
            LOG_WARNING("McuLink::signalRxMsg: Unexpected Reply Received. guid=%s, rxMsg=%s \n", guid.CSTR(), rxMsg.CSTR());
            mutexActStatusMap.unlock();
            return;
        }

        // ActionStatus found
        DataServiceActionStatus curStatus = actStatusMap[guid];
        actStatusMap.remove(guid);

        mutexActStatusMap.unlock();

        DS_McuDef::parseMsg(rxMsg, curStatus.request, curStatus.arg, curStatus.reply);

        if (curStatus.reply.contains(_L("InvalidParameter")))
        {
            curStatus.state = DS_ACTION_STATE_BAD_REQUEST;
            curStatus.err = QString().asprintf("%s", curStatus.reply.CSTR());
        }
        else if ( (curStatus.request.contains(_L("INFO"))) ||
                  (curStatus.request.contains(_L("VERSION"))) ||
                  (curStatus.request.contains(_L("DIGEST"))) ||
                  (curStatus.request.contains(_L("HWDIGEST"))) ||
                  (curStatus.request.contains(_L("GETPCALDATA"))) ||
                  (curStatus.request.contains(_L("GETPCALCOEFF"))) ||
                  (curStatus.request.contains(_L("GETPLUNGERFRICTION"))) ||
                  (curStatus.request.contains(_L("CALDIGEST"))) ||
                  (curStatus.request.contains(_L("BMSDIGEST"))) ||
                  (curStatus.request.contains(_L("GETSYRINGEAIRCHECKDATA"))) ||
                  (curStatus.request.contains(_L("GETSYRINGEAIRCHECKCOEFF"))) ||
                  (curStatus.request.contains(_L("GETSYRINGEAIRVOL"))) )
        {
            // Reply will be handled separately
            curStatus.state = DS_ACTION_STATE_STARTED;
        }
        else if (curStatus.reply.contains(_L("OK")))
        {
            curStatus.state = DS_ACTION_STATE_STARTED;
            curStatus.err = "";
        }
        else if (curStatus.reply.contains(_L("StopButtonPressed")))
        {
            curStatus.state = DS_ACTION_STATE_ALLSTOP_BTN_ABORT;
            curStatus.err = "USER_ABORT";
        }
        else if (curStatus.reply.contains(_L("InvalidState")))
        {
            curStatus.state = DS_ACTION_STATE_INVALID_STATE;
            curStatus.err = "INVALID_STATE";
        }
        else
        {
            curStatus.state = DS_ACTION_STATE_INTERNAL_ERR;
            curStatus.err = curStatus.reply;
        }

        bool verbose = true;

        if ( (curStatus.request.contains(_L("DIGEST"))) ||
             (curStatus.request.contains(_L("HWDIGEST"))) ||
             (curStatus.request.contains(_L("LEDS"))) )
        {
            verbose = false;
        }

        actionStarted(curStatus, NULL, verbose);
    });


    connect(env->ds.mcuData, &DS_McuData::signalDataChanged_LinkState, this, [=](DS_McuDef::LinkState curLinkState, DS_McuDef::LinkState prevLinkState) {
        if ( (prevLinkState == DS_McuDef::LINK_STATE_CONNECTED) &&
             (curLinkState == DS_McuDef::LINK_STATE_DISCONNECTED) )
        {
            LOG_WARNING("MCU Link is down. Processing all action status as MCU TIMEOUT..\n");
            mutexActStatusMap.lock();
            QMap<QString, DataServiceActionStatus>::const_iterator i = actStatusMap.begin();
            while (i != actStatusMap.end())
            {
                DataServiceActionStatus curStatus = i.value();
                curStatus.state = DS_ACTION_STATE_TIMEOUT;
                curStatus.err = "MCU TIMEOUT";
                actionStarted(curStatus, NULL);
                i++;
            }
            actStatusMap.clear();
            mutexActStatusMap.unlock();
        }
    });
}

void DS_McuAction::handleSyringeActionCompleted(SyringeIdx syringeIdx, QString actionId)
{
    if (actionId == _L("DISENGAGE"))
    {
        if (env->ds.alertAction->isActivatedWithSyringeIdx("MotorPositionFault", syringeIdx))
        {
            env->ds.alertAction->deactivateFromSyringeIdx("MotorPositionFault", syringeIdx);
        }
    }
    if (actionId == _L("PURGE"))
    {
		// Since we want the pressure coefficients for the plunger friction alert,
		// which is generated right after the new muds purge is complete, we grab
		// this data at this time to ensure that it is available.
        env->ds.mcuAction->actCalGetPressureCoeff(syringeIdx);
    }
}

void DS_McuAction::handleSyringeActionFailed(SyringeIdx syringeIdx, QString actionId, DS_McuDef::SyringeState syringeState, DataServiceActionStatus &actionStatus)
{
    QString actionName = actionId;

    if (actionId == _L("FILL"))
    {
        actionName = "Fill";
    }
    else if (actionId == _L("PURGE"))
    {
        actionName = "Purge";
    }
    else if (actionId == _L("ENGAGE"))
    {
        actionName = "Engage";
    }
    else if (actionId == _L("DISENGAGE"))
    {
        actionName = "Disengage";
    }
    else if (actionId == _L("FINDPLUNGER"))
    {
        actionName = "FindPlungers";
    }
    else if (actionId == _L("PRIME"))
    {
        actionName = actionStatus.arg2;
    }
    else if (actionId == _L("PISTON"))
    {
        actionName = "ManualMove";
    }
	else if (actionId == _L("CALSLACK"))
    {
        actionName = "SlackCalibration";
    }

    QString syringeIdxStr = ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdx);

    // Activate alerts
    switch (syringeState)
    {
    case DS_McuDef::SYRINGE_STATE_MUDS_REMOVED:
        env->ds.alertAction->activate("InterruptedMUDSUnlatched", syringeIdxStr + ";" + actionName);
        break;
    case DS_McuDef::SYRINGE_STATE_OVER_CURRENT:
        env->ds.alertAction->activate("OverCurrentFault", syringeIdxStr + ";" + actionName);
        break;
    case DS_McuDef::SYRINGE_STATE_OVER_PRESSURE:
        env->ds.alertAction->activate("OverPressureFault", syringeIdxStr + ";" + actionName);
        break;
    case DS_McuDef::SYRINGE_STATE_USER_ABORT:
        if ( (actionName == "FindPlungers") ||
             (actionName == "Engage") ||
             (actionName == "Disengage") )
        {
            QString alertDataActionName = actionName;
            alertDataActionName.replace("All", "");
            env->ds.alertAction->activate("InterruptedAllStopPressed", QString().asprintf("%s;%s", alertDataActionName.CSTR(), syringeIdxStr.CSTR()));
        }
        break;
    case DS_McuDef::SYRINGE_STATE_TIMEOUT:
        break;
    case DS_McuDef::SYRINGE_STATE_PLUNGER_ENGAGE_FAULT:
        // Note: Plunger engagement fault can also be occurred from PURGE
        break;
    case DS_McuDef::SYRINGE_STATE_PLUNGER_DISENGAGE_FAULT:
        break;
    case DS_McuDef::SYRINGE_STATE_LOST_PLUNGER:
        env->ds.alertAction->activate("PlungerNotDetected", syringeIdxStr);
        // Stop all MCU actions
        actStopAll();
        break;
    default:
        break;
    }

    // Update Plunger Engagement/DisengagementFault
    if (syringeState != DS_McuDef::SYRINGE_STATE_USER_ABORT)
    {
        if ( (actionName == "Engage") ||
             (syringeState == DS_McuDef::SYRINGE_STATE_PLUNGER_ENGAGE_FAULT) )
        {
            env->ds.alertAction->activate("PlungerEngagementFault", syringeIdxStr);
        }

        if (syringeState == DS_McuDef::SYRINGE_STATE_PLUNGER_DISENGAGE_FAULT)
        {
            // Disengage failed with generic reason
            env->ds.alertAction->activate("PlungerDisengagementFault", QString().asprintf("%s;%s", syringeIdxStr.CSTR(), ImrParser::ToImr_SyringeState(syringeState).CSTR()));
        }
        else if ( (actionName == "Disengage") &&
                  ( (syringeState == DS_McuDef::SYRINGE_STATE_MOTOR_FAIL) ||
                    (syringeState == DS_McuDef::SYRINGE_STATE_HOME_SENSOR_MISSING) ) )
        {
            // Disengage failed with other reasons
            env->ds.alertAction->activate("PlungerDisengagementFault", QString().asprintf("%s;%s", syringeIdxStr.CSTR(), ImrParser::ToImr_SyringeState(syringeState).CSTR()));
        }
    }

    // Update Action Status
    switch (syringeState)
    {
    case DS_McuDef::SYRINGE_STATE_USER_ABORT:
        actionStatus.state = DS_ACTION_STATE_ALLSTOP_BTN_ABORT;
        break;
    case DS_McuDef::SYRINGE_STATE_TIMEOUT:
        actionStatus.state = DS_ACTION_STATE_TIMEOUT;
        break;
    case DS_McuDef::SYRINGE_STATE_INSUFFICIENT_FLUID:
    case DS_McuDef::SYRINGE_STATE_AIR_DETECTED:
    case DS_McuDef::SYRINGE_STATE_OVER_PRESSURE:
    case DS_McuDef::SYRINGE_STATE_OVER_CURRENT:
    case DS_McuDef::SYRINGE_STATE_SUDS_REMOVED:
    case DS_McuDef::SYRINGE_STATE_SUDS_INSERTED:
    case DS_McuDef::SYRINGE_STATE_MUDS_REMOVED:
    case DS_McuDef::SYRINGE_STATE_INVALID_STATE:
        actionStatus.state = DS_ACTION_STATE_INVALID_STATE;
        break;
    default:
        actionStatus.state = DS_ACTION_STATE_INTERNAL_ERR;
        break;
    }
    actionStatus.err = ImrParser::ToImr_SyringeState(syringeState);
}

void DS_McuAction::waitSyringeActionComplete(SyringeIdx syringeIdx, QString actionId, const DataServiceActionStatus &status)
{
    if (status.state != DS_ACTION_STATE_STARTED)
    {
        // Syringe action is not started
        LOG_DEBUG("WaitSyringeActionComplete(%s,%s): Syringe Action Not Started: Status=%s\n",
                  ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdx).CSTR(),
                  actionId.CSTR(),
                  ImrParser::ToImr_DataServiceActionStatusStr(status).CSTR());
        return;
    }

    // Set current syringe action state to processing
    DS_McuDef::SyringeStates states = env->ds.mcuData->getSyringeStates();
    states[syringeIdx] = DS_McuDef::SYRINGE_STATE_PROCESSING;
    env->ds.mcuData->setSyringeStates(states);

    // Register callback for checking action state
    QString dataChangedGuid = Util::newGuid();
    QMetaObject::Connection conn = connect(env->ds.mcuData, &DS_McuData::signalDataChanged_SyringeStates, this, [=](const DS_McuDef::SyringeStates &syringeStates) {
        LOG_DEBUG("WaitSyringeActionComplete(%s,%s): syringeStates=%s\n",
                  ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdx).CSTR(),
                  actionId.CSTR(),
                  Util::qVarientToJsonData(ImrParser::ToImr_SyringeStates(syringeStates)).CSTR());

        DS_McuDef::SyringeState syringeState = syringeStates[syringeIdx];
        DataServiceActionStatus curStatus = status;

        if ( (syringeState == DS_McuDef::SYRINGE_STATE_PROCESSING) ||
             (syringeState == DS_McuDef::SYRINGE_STATE_STOP_PENDING) )
        {
            // Action is in progress
            return;
        }
        else if (syringeState == DS_McuDef::SYRINGE_STATE_COMPLETED)
        {
            curStatus.state = DS_ACTION_STATE_COMPLETED;
            handleSyringeActionCompleted((SyringeIdx)syringeIdx, actionId);
        }
        else
        {
            handleSyringeActionFailed((SyringeIdx)syringeIdx, actionId, syringeState, curStatus);
        }

        // Syringe action is completed
        actionCompleted(curStatus);
        env->actionMgr->deleteActCompleted(dataChangedGuid);
    });

    env->actionMgr->createActCompleted(dataChangedGuid, conn, QString(__PRETTY_FUNCTION__) + ": waitSyringeActionComplete: SyringeStates");
}


void DS_McuAction::waitAllSyringeActionComplete(QString actionId, const DataServiceActionStatus &status)
{
    if (status.state != DS_ACTION_STATE_STARTED)
    {
        // Syringe action is not started
        LOG_DEBUG("WaitAllSyringeActionComplete: action=%s: Syringe Action Not Started: Status=%s\n", actionId.CSTR(), ImrParser::ToImr_DataServiceActionStatusStr(status).CSTR());
        return;
    }

    // Set current syringe action state to processing
    DS_McuDef::SyringeStates states = env->ds.mcuData->getSyringeStates();
    for (int syringeIdx = 0; syringeIdx < states.length(); syringeIdx++)
    {
        states[syringeIdx] = DS_McuDef::SYRINGE_STATE_PROCESSING;
    }
    env->ds.mcuData->setSyringeStates(states);

    QString dataChangedGuid = Util::newGuid();
    QMetaObject::Connection conn = connect(env->ds.mcuData, &DS_McuData::signalDataChanged_SyringeStates, this, [=](const DS_McuDef::SyringeStates &syringeStates) {
        LOG_DEBUG("WaitAllSyringeActionComplete: action=%s, syringeStates=%s\n", actionId.CSTR(), Util::qVarientToJsonData(ImrParser::ToImr_SyringeStates(syringeStates)).CSTR());

        for (int syringeIdx = 0; syringeIdx < SYRINGE_IDX_MAX; syringeIdx++)
        {
            DS_McuDef::SyringeState syringeState = syringeStates[syringeIdx];

            if ( (syringeState == DS_McuDef::SYRINGE_STATE_PROCESSING) ||
                 (syringeState == DS_McuDef::SYRINGE_STATE_STOP_PENDING) )
            {
                // At least one action is in progress. Process all actions later
                return;
            }
        }

        DataServiceActionStatus curStatus = status;
        curStatus.state = DS_ACTION_STATE_COMPLETED;

        for (int syringeIdx = 0; syringeIdx < SYRINGE_IDX_MAX; syringeIdx++)
        {
            DS_McuDef::SyringeState syringeState = syringeStates[syringeIdx];

            if (syringeState == DS_McuDef::SYRINGE_STATE_COMPLETED)
            {
                handleSyringeActionCompleted((SyringeIdx)syringeIdx, actionId);
            }
            else
            {
                LOG_ERROR("WaitAllSyringeActionComplete: action=%s, syringeState[%s]=%s\n",
                          actionId.CSTR(),
                          ImrParser::ToImr_FluidSourceSyringeIdx((SyringeIdx)syringeIdx).CSTR(),
                          ImrParser::ToImr_SyringeState(syringeState).CSTR());
                handleSyringeActionFailed((SyringeIdx)syringeIdx, actionId, syringeState, curStatus);
            }
        }

        // All syringe actions are completed
        actionCompleted(curStatus);

        env->actionMgr->deleteActCompleted(dataChangedGuid);
    });
    env->actionMgr->createActCompleted(dataChangedGuid, conn, QString(__PRETTY_FUNCTION__) + ": waitAllSyringeActionComplete: SyringeStates");
}

DataServiceActionStatus DS_McuAction::startAction(QString msgId, QString body, QString actGuid, int timeoutMs)
{
    if (actGuid == "")
    {
        actGuid = Util::newGuid();
    }

    bool verbose = true;

    if ( (msgId.contains(_L("DIGEST"))) ||
         (msgId.contains(_L("HWDIGEST"))) ||
         (msgId.contains(_L("LEDS"))) )
    {
        verbose = false;
    }

    DataServiceActionStatus status = actionInit(actGuid, msgId, body);
    status.state = DS_ACTION_STATE_START_WAITING;

    // Check Application State
    if (env->state == EnvGlobal::STATE_EXITING)
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = "Distro is not running";
        actionStarted(status, NULL, verbose);
        return status;
    }

    // Check MCU Link state
    DS_McuDef::LinkState mcuLinkState = env->ds.mcuData->getLinkState();
    bool linkStateOk = true;

    if (status.request == _L("VERSION"))
    {
        if ( (mcuLinkState != DS_McuDef::LINK_STATE_CONNECTING) &&
             (mcuLinkState != DS_McuDef::LINK_STATE_RECOVERING) &&
             (mcuLinkState != DS_McuDef::LINK_STATE_CONNECTED) )
        {
            linkStateOk = false;
        }
    }
    else if (status.request == _L("DIGEST"))
    {
        if ( (mcuLinkState != DS_McuDef::LINK_STATE_RECOVERING) &&
             (mcuLinkState != DS_McuDef::LINK_STATE_CONNECTED) )
        {
            linkStateOk = false;
        }
    }
    else
    {
        if (mcuLinkState != DS_McuDef::LINK_STATE_CONNECTED)
        {
            linkStateOk = false;
        }
    }

    if (!linkStateOk)
    {
        status.state = DS_ACTION_STATE_INVALID_STATE;
        status.err = ImrParser::ToImr_McuLinkState(mcuLinkState);
        actionStarted(status, NULL, verbose);
        return status;
    }

    QString mcuLinkActionGuid = Util::newGuid();

    mutexActStatusMap.lock();
    actStatusMap.insert(mcuLinkActionGuid, status);
    mutexActStatusMap.unlock();

    emit mcuLink->signalTxMsg(mcuLinkActionGuid, DS_McuDef::newMsg(msgId, body), timeoutMs);

    return status;
}

DataServiceActionStatus DS_McuAction::actLinkConnect(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "LinkConnect");

    emit mcuLink->signalLinkConnect();

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);
    status.state = DS_ACTION_STATE_COMPLETED;
    actionCompleted(status);

    return status;

}

DataServiceActionStatus DS_McuAction::actLinkDisconnect(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "LinkDisconnect");

    emit mcuLink->signalLinkDisconnect();

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);
    status.state = DS_ACTION_STATE_COMPLETED;
    actionCompleted(status);

    return status;
}

DataServiceActionStatus DS_McuAction::actRestoreLastMcuDigest(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "RestoreLastMcuDigest");

    // Restore Last MCU Digest
    QFile fileBuf(PATH_LAST_MCU_DIGEST);
    if (!fileBuf.open(QFile::ReadOnly | QFile::Text))
    {
        status.err = QString().asprintf("File Open Err: %s", fileBuf.errorString().CSTR());
        status.state = DS_ACTION_STATE_INVALID_STATE;
        return status;
    }

    QJsonParseError parseErr;
    QString jsonStr = fileBuf.readAll();
    fileBuf.close();

    QJsonDocument document = QJsonDocument::fromJson(jsonStr.toUtf8(), &parseErr);

    if (parseErr.error != QJsonParseError::NoError)
    {
        QString parseErrStr = parseErr.errorString();
        QString err = QString().asprintf("Failed to open last MCU digest. ParseErr=%s\n", parseErrStr.CSTR());
        LOG_ERROR("%s", err.CSTR());
        env->ds.alertAction->activate("HCUInternalSoftwareError", err);

        status.err = QString().asprintf("Parse Err: %s", parseErrStr.CSTR());
        status.state = DS_ACTION_STATE_INVALID_STATE;
        return status;
    }

    QVariantMap digestMap = document.toVariant().toMap();

    if (digestMap.contains("Digest"))
    {
        QString lastDigest = digestMap["Digest"].toString();
        LOG_INFO("actRestoreLastMcuDigest(): Restoring last MCU digest(%s)\n", lastDigest.CSTR());
        mcuLink->msgHandler->handleMsg("DIGEST", "", lastDigest);

        QList<double> lastSyringeVols = env->ds.mcuData->getSyringeVols();
        LOG_INFO("actRestoreLastMcuDigest(): Setting last syringe vols=%s\n", Util::qVarientToJsonData(ImrParser::ToImr_DoubleList(lastSyringeVols)).CSTR());
        env->ds.mcuData->setLastSyringeVols(lastSyringeVols);
    }

    status.state = DS_ACTION_STATE_STARTED;
    actionStarted(status);

    status.state = DS_ACTION_STATE_COMPLETED;
    actionCompleted(status);

    return status;
}

DataServiceActionStatus DS_McuAction::actVersion(QString actGuid)
{
    return startAction("VERSION", "", actGuid);
}

DataServiceActionStatus DS_McuAction::actInfo(QString actGuid)
{
    return startAction("INFO", "", actGuid);
}

DataServiceActionStatus DS_McuAction::actSetBaseType(QString baseBoardType, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "SetBaseType");
    QString arg;

    if (baseBoardType == "")
    {
        // Base Board Type is not defined. Get it from HardwareInfo.
        baseBoardType = env->ds.hardwareInfo->getGeneral_BaseBoardType().value.toString();
    }

    if (baseBoardType == "OCS")
    {
        arg = "OCS";
    }
    else if (baseBoardType == "Battery")
    {
        arg = "BATTERY";
    }
    else if (baseBoardType == "NoBattery")
    {
        arg = "NO_BATTERY";
    }
    else
    {
        status.state = DS_ACTION_STATE_BAD_REQUEST;
        status.err = QString().asprintf("Bad Base Board Type (%s)", baseBoardType.CSTR());
        actionStarted(status);
        return status;
    }

    return startAction("SETBASETYPE", arg, actGuid);
}

DataServiceActionStatus DS_McuAction::actSetHwTypes(QString actGuid)
{
    QString argStr = "";

    DS_McuDef::HwRevCompatibilityGroups groups = env->ds.mcuData->getHwRevCompatibilityGroups();
    for (int groupIdx = 0; groupIdx < groups.length(); groupIdx++)
    {
        const DS_McuDef::HwRevCompatibilityGroup &group = groups[groupIdx];
        QString hwTypeStr = group.hwDefault;

        QString hwRev = "";

        // Get Rev number for current group
        if (group.hwType == "BASE")
        {
            hwRev = env->ds.hardwareInfo->getBoards_Revision_PA1304_Base().value.toString();
        }

        int hwRevNumber = (hwRev.length() >= 2) ? (hwRev[0].toLatin1() << 8) + hwRev[1].toLatin1() : 0;

        for (int itemIdx = 0; itemIdx < group.compatibilityList.length(); itemIdx++)
        {
            // Find right compatible hw type if exist
            const DS_McuDef::HwRevCompatibility &item = group.compatibilityList[itemIdx];
            int hwRevNumberTarget = (item.revOffset.length() >= 2) ? (item.revOffset[0].toLatin1() << 8) + item.revOffset[1].toLatin1() : 0;

            if (hwRevNumber >= hwRevNumberTarget)
            {
                // Compatible hw type found. Keep scan to last.
                hwTypeStr = group.hwType + QString().asprintf("%d", item.hwNumber);
            }
        }
        argStr += hwTypeStr;

        if (groupIdx < groups.length() - 1)
        {
            argStr += ",";
        }
    }
    return startAction("SETHWTYPES", argStr, actGuid);
}

DataServiceActionStatus DS_McuAction::actDigest(QString actGuid)
{
    return startAction("DIGEST", "", actGuid);
}

DataServiceActionStatus DS_McuAction::actArm(const DS_McuDef::InjectionProtocol &injProtocol, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "Arm", QString().asprintf("%s", Util::qVarientToJsonData(ImrParser::ToImr_McuInjectionProtocol(injProtocol)).CSTR()));

    QString sudsFluidType = "SALINE";
    QList<QString> sudsFluids = env->ds.deviceData->getFluidSourceSuds().currentFluidKinds;
    if ( (sudsFluids.length() > 0) &&
         (sudsFluids.last() == "Contrast") )
    {
        sudsFluidType = "CONTRAST";
    }

    // Get Arm injection settings
    int adaptiveFlowReduction = injProtocol.maximumFlowRateReduction;
    int pressureLimitSensitivity = injProtocol.pressureLimitSensitivity;

    QString body = QString().asprintf("%d,%d,%s,%d,%d", injProtocol.pressureLimit, (int)injProtocol.phases.length(), sudsFluidType.CSTR(), adaptiveFlowReduction, pressureLimitSensitivity);

    for (int i = 0; i < injProtocol.phases.length(); i++)
    {
        QString typeStr = ImrParser::ToImr_InjectionPhaseType(injProtocol.phases[i].type);

        if ( (typeStr == _L("NONE")) ||
             (typeStr == _L("UNKNOWN")) )
        {
             status.state = DS_ACTION_STATE_BAD_REQUEST;
             status.err = "BAD REQUEST";
             actionStarted(status);
             return status;
        }

        body += QString().asprintf(",%s,%d,%.1f,%.1f,%d",
                                  typeStr.CSTR(),
                                  injProtocol.phases[i].mix,
                                  injProtocol.phases[i].vol,
                                  injProtocol.phases[i].flow,
                                  injProtocol.phases[i].type == DS_McuDef::INJECTION_PHASE_TYPE_PAUSE ? injProtocol.phases[i].duration : 0);
    }

    // Action Started Callback
    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);

        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            // Action Completed Callback
            QString dataChangedGuid = Util::newGuid();
            QMetaObject::Connection conn = connect(env->ds.mcuData, &DS_McuData::signalDataChanged_InjectorStatus, this, [=](const DS_McuDef::InjectorStatus &injectorStatus) {
                DataServiceActionStatus curStatus = status;

                if (injectorStatus.state == DS_McuDef::INJECTOR_STATE_READY_START)
                {
                    adjustFlowRateBusy = false;
                    curStatus.state = DS_ACTION_STATE_COMPLETED;
                    env->ds.mcuData->setInjectionProtocol(injProtocol);
                }
                else
                {
                    curStatus.state = DS_ACTION_STATE_INTERNAL_ERR;
                    curStatus.err = "Arm Failed";
                }
                actionCompleted(curStatus);
                env->actionMgr->deleteActCompleted(dataChangedGuid);
            });
            env->actionMgr->createActCompleted(dataChangedGuid, conn, QString(__PRETTY_FUNCTION__) + ": actArm: InjectorStatus");
        }
    });

    // Action Start
    //Timeout increased for pre-pressurisation delay
    return startAction("ARM", body, guid, MCU_ARM_PROCESSING_TIMEOUT_MS);
}

DataServiceActionStatus DS_McuAction::actDisarm(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "DISARM");

    // Action Started Callback
    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);

        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            // Action Completed Callback
            QString dataChangedGuid = Util::newGuid();
            QMetaObject::Connection conn = connect(env->ds.mcuData, &DS_McuData::signalDataChanged_InjectorStatus, this, [=](const DS_McuDef::InjectorStatus &injectorStatus) {
                DataServiceActionStatus curStatus = status;
                if (injectorStatus.state == DS_McuDef::INJECTOR_STATE_IDLE)
                {
                    curStatus.state = DS_ACTION_STATE_COMPLETED;
                }
                else
                {
                    curStatus.state = DS_ACTION_STATE_INTERNAL_ERR;
                    curStatus.err = "Disarm Failed";
                }

                actionCompleted(curStatus);
                env->actionMgr->deleteActCompleted(dataChangedGuid);
            });
            env->actionMgr->createActCompleted(dataChangedGuid, conn, QString(__PRETTY_FUNCTION__) + ": actDisarm: InjectorStatus");
        }
    });

    // Action Start
    return startAction("DISARM", "", guid);
}

DataServiceActionStatus DS_McuAction::actInjectStart(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "INJECTSTART");

    // Action Started Callback
    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);

        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            // Action Completed Callback
            QString dataChangedGuid = Util::newGuid();
            QMetaObject::Connection conn = connect(env->ds.mcuData, &DS_McuData::signalDataChanged_InjectorStatus, this, [=](const DS_McuDef::InjectorStatus &injectorStatus) {
                DataServiceActionStatus curStatus = status;
                bool actionCompletedOk = false;
                // If the injection was paused at the very end of the phase, it is possible
                // that the state is COMPLETING after resuming.
                if ( (injectorStatus.state == DS_McuDef::INJECTOR_STATE_DELIVERING) ||
                     (injectorStatus.state == DS_McuDef::INJECTOR_STATE_PHASE_PAUSED) ||
                     (injectorStatus.state == DS_McuDef::INJECTOR_STATE_COMPLETING) )
                {
                    actionCompletedOk = true;
                }

                if (actionCompletedOk)
                {
                    curStatus.state = DS_ACTION_STATE_COMPLETED;
                }
                else
                {
                    curStatus.state = DS_ACTION_STATE_INTERNAL_ERR;
                    curStatus.err = "Inject Start Failed";
                }

                actionCompleted(curStatus);
                env->actionMgr->deleteActCompleted(dataChangedGuid);
            });
            env->actionMgr->createActCompleted(dataChangedGuid, conn, QString(__PRETTY_FUNCTION__) + ": actInjectStart: InjectorStatus");
        }
    });

    return startAction("INJECTSTART", "", guid);
}

DataServiceActionStatus DS_McuAction::actInjectStop(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "INJECTSTOP");

    // Action Started Callback
    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);

        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            // Action Completed Callback
            QString dataChangedGuid = Util::newGuid();
            QMetaObject::Connection conn = connect(env->ds.mcuData, &DS_McuData::signalDataChanged_InjectorStatus, this, [=](const DS_McuDef::InjectorStatus &injectorStatus) {
                DataServiceActionStatus curStatus = status;
                bool actionCompletedOk = false;
                if ( (injectorStatus.state == DS_McuDef::INJECTOR_STATE_COMPLETING) ||
                     (injectorStatus.state == DS_McuDef::INJECTOR_STATE_COMPLETED) )
                {
                    actionCompletedOk = true;
                }

                if (actionCompletedOk)
                {
                    curStatus.state = DS_ACTION_STATE_COMPLETED;
                }
                else
                {
                    curStatus.state = DS_ACTION_STATE_INTERNAL_ERR;
                    curStatus.err = "Inject Stop Failed";
                }

                actionCompleted(curStatus);
                env->actionMgr->deleteActCompleted(dataChangedGuid);
            });
            env->actionMgr->createActCompleted(dataChangedGuid, conn, QString(__PRETTY_FUNCTION__) + ": actInjectStop: InjectorStatus");
        }
    });

    return startAction("INJECTSTOP", "", guid);
}

DataServiceActionStatus DS_McuAction::actInjectHold(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "INJECTHOLD");

    // Action Started Callback
    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);

        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            // Action Completed Callback
            QString dataChangedGuid = Util::newGuid();
            QMetaObject::Connection conn = connect(env->ds.mcuData, &DS_McuData::signalDataChanged_InjectorStatus, this, [=](const DS_McuDef::InjectorStatus &injectorStatus) {
                DataServiceActionStatus curStatus = status;
                bool actionCompletedOk = false;
                if (injectorStatus.state == DS_McuDef::INJECTOR_STATE_HOLDING)
                {
                    actionCompletedOk = true;
                }

                if (actionCompletedOk)
                {
                    curStatus.state = DS_ACTION_STATE_COMPLETED;
                }
                else
                {
                    LOG_WARNING("actInjectHold: Inject Hold Failed. InjectorStatus is changed to %s\n", ImrParser::ToImr_InjectorState(injectorStatus.state).CSTR());
                    curStatus.state = DS_ACTION_STATE_INTERNAL_ERR;
                    curStatus.err = "Inject Hold Failed";
                }

                actionCompleted(curStatus);
                env->actionMgr->deleteActCompleted(dataChangedGuid);
            });
            env->actionMgr->createActCompleted(dataChangedGuid, conn, QString(__PRETTY_FUNCTION__) + ": actInjectHold: InjectorStatus");
        }
    });

    return startAction("INJECTHOLD", "", guid);
}

DataServiceActionStatus DS_McuAction::actInjectJump(int hcuJumpFromIdx, int hcuJumpToIdx, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "InjectJump", QString().asprintf("%d;%d;", hcuJumpFromIdx, hcuJumpToIdx));

    DS_McuDef::InjectionProtocol mcuInjectProtocol = env->ds.mcuData->getInjectionProtocol();
    int mcuJumpFromIdx = DS_McuDef::getMcuInjectPhaseIdxFromHcuPhaseIdx(hcuJumpFromIdx, mcuInjectProtocol.twoContrastInjectPhaseIndex);
    int mcuJumpToIdx = DS_McuDef::getMcuInjectPhaseIdxFromHcuPhaseIdx(hcuJumpToIdx, mcuInjectProtocol.twoContrastInjectPhaseIndex);

    LOG_INFO("actInjectJump(): McuJumpFromIdx=%d, McuJumpToIdx=%d\n", mcuJumpFromIdx, mcuJumpToIdx);

    return startAction("INJECTJUMP", QString().asprintf("%d", mcuJumpToIdx), actGuid);
}

DataServiceActionStatus DS_McuAction::actPower(DS_McuDef::PowerControlType ctrl, QString actGuid)
{
    QString body = ImrParser::ToImr_PowerControlType(ctrl);
    DataServiceActionStatus status = actionInit(actGuid, "Power", QString().asprintf("%s", body.CSTR()));

    switch (ctrl)
    {
    case DS_McuDef::POWER_CONTROL_TYPE_OFF:
    case DS_McuDef::POWER_CONTROL_TYPE_REBOOT:
        break;
    default:
        status.state = DS_ACTION_STATE_BAD_REQUEST;
        status.err = "BAD REQUEST";
        actionStarted(status);
        return status;
    }

    // Action Started Callback
    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        if (curStatus.err.contains("T_POWERFAILED_InvalidState"))
        {
            curStatus.state = DS_ACTION_STATE_BUSY;
        }

        actionStarted(curStatus, &status);

        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            curStatus.state = DS_ACTION_STATE_COMPLETED;
            actionCompleted(curStatus, &status);
        }
    });

    return startAction("POWER", body, guid);
}

DataServiceActionStatus DS_McuAction::actAdjflow(double delta, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "Adjflow", QString().asprintf("%.1f", delta));

    if (adjustFlowRateBusy)
    {
        status.state = DS_ACTION_STATE_BUSY;
        status.err = "Adjust Flow In Progress";
        actionStarted(status);
        return status;
    }

    adjustFlowRateBusy = true;

    QList<double> startFlows = env->ds.mcuData->getSyringeFlows();
    double startFlowTotal = 0;
    for (int syringeIdx = 0; syringeIdx < startFlows.length(); syringeIdx++)
    {
        startFlowTotal += startFlows[syringeIdx];
    }

    QString statePathMonitorGuid = Util::newGuid();

    // Action Started Callback
    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);

        env->actionMgr->deleteActCompleted(statePathMonitorGuid);

        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            // Delta processed
            DataServiceActionStatus completedStatus = status;
            completedStatus.state = DS_ACTION_STATE_COMPLETED;
            actionCompleted(completedStatus);
        }

        adjustFlowRateBusy = false;
    });

    // Monitor statePath. If injection completed before action completed, abort action.
    QMetaObject::Connection conn = connect(env->ds.systemData, &DS_SystemData::signalDataChanged_StatePath, this, [=](DS_SystemDef::StatePath statePath) {
        if (statePath != DS_SystemDef::STATE_PATH_EXECUTING)
        {
            LOG_WARNING("actAdjflow(): Unexpected StatePath=%s\n", ImrParser::ToImr_StatePath(statePath).CSTR());
            env->actionMgr->deleteActCompleted(statePathMonitorGuid);

            DataServiceActionStatus curStatus = status;
            curStatus.state = DS_ACTION_STATE_INVALID_STATE;
            curStatus.err = "T_ADJFLOWFAILED_InvalidState";
            adjustFlowRateBusy = false;
            actionCompleted(curStatus);
        }
    });
    env->actionMgr->createActCompleted(statePathMonitorGuid, conn, QString(__PRETTY_FUNCTION__) + ": actAdjflow: StatePath");

    // Action Start
    QString body = QString().asprintf("%.1f", delta);
    return startAction("ADJFLOW", body, guid);
}

DataServiceActionStatus DS_McuAction::actStopcock(SyringeIdx location, DS_McuDef::StopcockPos targetPos, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "Stopcock", QString().asprintf("%s;%s",
                                                                                       ImrParser::ToImr_FluidSourceSyringeIdx(location).CSTR(),
                                                                                       ImrParser::ToImr_StopcockPos(targetPos).CSTR()));
    DS_McuDef::StopcockPosAll listPos = env->ds.mcuData->getStopcockPosAll();

    bool actionRequired = false;

    if ( (targetPos == DS_McuDef::STOPCOCK_POS_FILL) ||
         (targetPos == DS_McuDef::STOPCOCK_POS_INJECT) ||
         (targetPos == DS_McuDef::STOPCOCK_POS_CLOSED) )
    {
        if (targetPos != listPos[location])
        {
            actionRequired = true;
        }
    }

    if (!actionRequired)
    {
        LOG_INFO("ACT_STOPCOCK: Index=%d, targetPos=%s Already at target position\n", location, ImrParser::ToImr_StopcockPos(targetPos).CSTR());

        // Stopcocks already in favoured position. Ignore request.
        status.state = DS_ACTION_STATE_STARTED;
        actionStarted(status, NULL, false);

        status.state = DS_ACTION_STATE_COMPLETED;
        actionCompleted(status, NULL, false);
        return status;
    }

    QString body = "";
    for (int scIdx = 0; scIdx < SYRINGE_IDX_MAX; scIdx++)
    {
        if (body != "")
        {
            body += ",";
        }

        if (location == scIdx)
        {
            body += ImrParser::ToImr_StopcockPos(targetPos);
        }
        else
        {
            body += ImrParser::ToImr_StopcockPos(DS_McuDef::STOPCOCK_POS_NONE);
        }
    }

    // Action Started Callback
    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);

        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            // Action Completed Callback
            QString dataChangedGuid = Util::newGuid();
            QMetaObject::Connection conn = connect(env->ds.mcuData, &DS_McuData::signalDataChanged_StopcockPosAll, this, [=](const DS_McuDef::StopcockPosAll &posAll) {
                if (targetPos == posAll[location])
                {
                    DataServiceActionStatus curStatus = status;
                    env->actionMgr->deleteActCompleted(dataChangedGuid);
                    curStatus.state = DS_ACTION_STATE_COMPLETED;
                    actionCompleted(curStatus);
                }
            });

            QTimer::singleShot(STOPCOCK_ACTION_TIMEOUT_MS, [=] {
                if (env->actionMgr->isActExist(dataChangedGuid))
                {
                    DataServiceActionStatus curStatus = status;
                    env->actionMgr->deleteActCompleted(dataChangedGuid);
                    curStatus.err = "TIMEOUT";
                    curStatus.state = DS_ACTION_STATE_TIMEOUT;
                    actionCompleted(curStatus);
                }
            });

            env->actionMgr->createActCompleted(dataChangedGuid, conn, QString(__PRETTY_FUNCTION__) + ": actStopcock: StopcockPosAll");
        }
    });

    // Action Start
    return startAction("STOPCOCK", body, guid);
}

DataServiceActionStatus DS_McuAction::actCalInletAirDetector(SyringeIdx syringeIdx, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "CalInletAirDetector", QString().asprintf("%s", ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdx).CSTR()));

    QString iadCalibrationMethod = env->ds.cfgLocal->get_ServiceCalibration_IADCalibrationMethod();

    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);
        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            actionCompleted(curStatus, &status);
        }
        env->ds.alertAction->activate("SRUHardwareCalibrated", QString().asprintf("InletAirDetector;%s;%s;%s", ImrParser::ToImr_FluidSourceBottleIdx(syringeIdx).CSTR(), iadCalibrationMethod.CSTR(), (curStatus.err == "") ? "OK" : curStatus.err.CSTR()));
        if (curStatus.err == "")
        {
            env->ds.hardwareInfo->updateCalibrationInfo(QString().asprintf("InletAirDetector%s", ImrParser::ToImr_FluidSourceBottleIdx(syringeIdx).CSTR()));
        }
    });

    QString body = QString().asprintf("%d", syringeIdx) + " " + iadCalibrationMethod;
    return startAction("CALBUB", body, guid, CALBUB_TIMEOUT_MS);
}

DataServiceActionStatus DS_McuAction::actCalMotor(SyringeIdx syringeIdx, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "CalMotor", QString().asprintf("%s", ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdx).CSTR()));

    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);
        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            actionCompleted(curStatus, &status);
        }
        env->ds.alertAction->activate("SRUHardwareCalibrated", QString().asprintf("Motor;%s;NA;%s", ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdx).CSTR(), (curStatus.err == "") ? "OK" : curStatus.err.CSTR()));
        if (curStatus.err == "")
        {
            env->ds.hardwareInfo->updateCalibrationInfo(QString().asprintf("Motor%s", ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdx).CSTR()));
        }
    });

    // 2 minute timeout...
    QString body = QString().asprintf("%d", syringeIdx);
    return startAction("CALMOTOR", body, guid, CALMOTOR_TIMEOUT_MS);
}

DataServiceActionStatus DS_McuAction::actCalSudsSensor(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "CalSudsSensor");

    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);
        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            actionCompleted(curStatus, &status);
        }
        env->ds.alertAction->activate("SRUHardwareCalibrated", QString().asprintf("SUDS;NA;NA;%s", (curStatus.err == "") ? "OK" : curStatus.err.CSTR()));
        if (curStatus.err == "")
        {
            env->ds.hardwareInfo->updateCalibrationInfo("SUDS");
        }
    });

    return startAction("CALSUDS", "", guid, CALSUDS_TIMEOUT_MS);
}

DataServiceActionStatus DS_McuAction::actCalPlunger(SyringeIdx syringeIdx, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "CalPlunger", QString().asprintf("%s", ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdx).CSTR()));

    QString body = QString().asprintf("%d", syringeIdx);

    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);
        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            actionCompleted(curStatus, &status);
        }
        env->ds.alertAction->activate("SRUHardwareCalibrated", QString().asprintf("Plunger;%s;NA;%s", ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdx).CSTR(), (curStatus.err == "") ? "OK" : curStatus.err.CSTR()));
        if (curStatus.err == "")
        {
            env->ds.hardwareInfo->updateCalibrationInfo(QString().asprintf("Plunger%s", ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdx).CSTR()));
        }
    });

    return startAction("CALPLUNGER", body, guid, CALPLUNGER_TIMEOUT_MS);
}

DataServiceActionStatus DS_McuAction::actCalSetPressureMeter(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "CalSetPressureMeter");

    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);
        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            actionCompleted(curStatus, &status);
        }
        env->ds.alertAction->activate("SRUHardwareCalibrated", QString().asprintf("PressureMeter;NA;NA;%s", (curStatus.err == "") ? "OK" : curStatus.err.CSTR()));
        if (curStatus.err == "")
        {
            env->ds.hardwareInfo->updateCalibrationInfo("PressureMeter");
        }
    });

    return startAction("CALMETER", "", guid);
}


DataServiceActionStatus DS_McuAction::actCalPressureStart(SyringeIdx syringeIdx, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "CalPressureStart", QString().asprintf("%s", ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdx).CSTR()));

    mcuPressureCalibrationHandler->calibrationStart(syringeIdx);
    status.state = DS_ACTION_STATE_STARTED;

    return status;
}

DataServiceActionStatus DS_McuAction::actCalPressureStop(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "CalPressureStop");

    mcuPressureCalibrationHandler->calibrationStop();

    status.state = DS_ACTION_STATE_COMPLETED;

    return status;
}

DataServiceActionStatus DS_McuAction::actCalGetPressureData(SyringeIdx syringeIdx, bool isProductionType, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "CalGetPressureData", QString().asprintf("%s;%s", ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdx).CSTR(), isProductionType ? "Production" : "Normal"));

    // Action Started Callback
    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);
        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            actionCompleted(curStatus, &status);
        }
    });

    QString body = QString().asprintf("%d,%s", syringeIdx, isProductionType ? "production" : "normal");
    return startAction("GETPCALDATA", body, guid);
}

DataServiceActionStatus DS_McuAction::actCalGetPressureCoeff(SyringeIdx syringeIdx, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "GETPCALCOEFF", QString().asprintf("%s", ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdx).CSTR()));

    // Action Started Callback
    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);
        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            actionCompleted(curStatus, &status);
        }
    });

    QString body = QString().asprintf("%d", syringeIdx);
    return startAction("GETPCALCOEFF", body, guid);
}

DataServiceActionStatus DS_McuAction::actGetPlungerFrictionData(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "GETPLUNGERFRICTION", "all");

    // Action Started Callback
    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);
        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            actionCompleted(curStatus, &status);
        }
    });

    return startAction("GETPLUNGERFRICTION", "all", guid);
}

DataServiceActionStatus DS_McuAction::actCalSetPressureData(SyringeIdx syringeIdx, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "CalSetPressureData", QString().asprintf("%s", ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdx).CSTR()));

    // Action Started Callback
    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);
        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            actionCompleted(curStatus, &status);
        }
        env->ds.alertAction->activate("SRUHardwareCalibrated", QString().asprintf("PressureCalibrationData;%s;NA;%s", ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdx).CSTR(), (curStatus.err == "") ? "OK" : curStatus.err.CSTR()));
        if (curStatus.err == "")
        {
            env->ds.hardwareInfo->updateCalibrationInfo(QString().asprintf("PressureCalibrationData%s", ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdx).CSTR()));
        }
    });

    QString body = QString().asprintf("%d,%s", syringeIdx, mcuPressureCalibrationHandler->getCoefficients().CSTR());
    return startAction("SETPCALCOEFF", body, guid);
}

DataServiceActionStatus DS_McuAction::actCalDigest(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "CalDigest");

    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);
        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            actionCompleted(curStatus, &status);
        }
    });

    return startAction("CALDIGEST", "", guid);
}

DataServiceActionStatus DS_McuAction::actEngage(SyringeIdx syringeIdx, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "Engage", QString().asprintf("%s", ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdx).CSTR()));

    // Action Started Callback
    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        env->ds.alertAction->deactivate("PlungerEngagementFault", QString().asprintf("%s", ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdx).CSTR()));

        actionStarted(curStatus, &status);
        waitSyringeActionComplete(syringeIdx, "ENGAGE", curStatus);
    });

    // Action Start
    QString body = QString().asprintf("%d", syringeIdx);
    return startAction("ENGAGE", body, guid);
}

DataServiceActionStatus DS_McuAction::actEngageAll(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "EngageAll");

    // Confirm Action Completed
    bool engagedAll = true;
    DS_McuDef::PlungerStates listStates = env->ds.mcuData->getPlungerStates();
    for (int i = 0; i < SYRINGE_IDX_MAX; i++)
    {
        if (listStates[i] != DS_McuDef::PLUNGER_STATE_ENGAGED)
        {
            engagedAll = false;
            break;
        }
    }

    if (engagedAll)
    {
        status.state = DS_ACTION_STATE_STARTED;
        actionStarted(status);

        status.state = DS_ACTION_STATE_COMPLETED;
        actionCompleted(status);
        return status;
    }

    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        env->ds.alertAction->deactivate("PlungerEngagementFault");

        actionStarted(curStatus, &status);
        waitAllSyringeActionComplete("ENGAGE", curStatus);
    });

    return startAction("ENGAGE", "all", guid);
}

DataServiceActionStatus DS_McuAction::actDisengage(SyringeIdx syringeIdx, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "Disengage", QString().asprintf("%s", ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdx).CSTR()));

    // Action Started Callback
    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        env->ds.alertAction->deactivate("PlungerEngagementFault", QString().asprintf("%s", ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdx).CSTR()));

        actionStarted(curStatus, &status);
        waitSyringeActionComplete(syringeIdx, "DISENGAGE", curStatus);
    });

    // Action Start
    QString body = QString().asprintf("%d", syringeIdx);
    return startAction("DISENGAGE", body, guid);
}

DataServiceActionStatus DS_McuAction::actDisengageAll(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "DisengageAll");

    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        env->ds.alertAction->deactivate("PlungerEngagementFault");

        actionStarted(curStatus, &status);
        waitAllSyringeActionComplete("DISENGAGE", curStatus);
    });

    // Action Start
    return startAction("DISENGAGE", "all", guid);
}

DataServiceActionStatus DS_McuAction::actPurge(SyringeIdx syringeIdx, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "Purge", QString().asprintf("%s", ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdx).CSTR()));

    QList<double> listVol = env->ds.mcuData->getSyringeVols();
    if (listVol[syringeIdx] == 0)
    {
        status.state = DS_ACTION_STATE_STARTED;
        actionStarted(status);

        status.state = DS_ACTION_STATE_COMPLETED;
        actionCompleted(status);
        return status;
    }

    // Action Started Callback
    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);
        waitSyringeActionComplete(syringeIdx, "PURGE", curStatus);
    });

    // Action Start
    QString body = QString().asprintf("%d", syringeIdx);
    return startAction("PURGE", body, guid);
}

DataServiceActionStatus DS_McuAction::actPullPlunger(SyringeIdx syringeIdx, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "PullPlunger");

    // Action Started Callback
    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);
        waitSyringeActionComplete(syringeIdx, "PULL", curStatus);
    });

    // Action Start
    QString body = QString().asprintf("%d", syringeIdx);
    return startAction("PULL", body, guid);
}

DataServiceActionStatus DS_McuAction::actPullPlungers(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "PullPlungers");

    // Action Started Callback
    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);
        waitAllSyringeActionComplete("PULL", curStatus);
    });

    // Action Start
    return startAction("PULL", "all", guid);
}

DataServiceActionStatus DS_McuAction::actPurgeAll(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "PurgeAll");

    bool purgedAll = true;
    QList<double> listVol = env->ds.mcuData->getSyringeVols();
    for (int i = 0; i < SYRINGE_IDX_MAX; i++)
    {
        if (listVol[i] != 0)
        {
            purgedAll = false;
            break;
        }
    }

    if (purgedAll)
    {
        status.state = DS_ACTION_STATE_STARTED;
        actionStarted(status);

        status.state = DS_ACTION_STATE_COMPLETED;
        actionCompleted(status);
        return status;
    }

    // Action Started Callback
    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);
        waitAllSyringeActionComplete("PURGE", curStatus);
    });

    // Action Start
    return startAction("PURGE", "all", guid);
}

DataServiceActionStatus DS_McuAction::actStopAll(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "STOP");

    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);

        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            if (env->ds.mcuData->getSyringesBusy())
            {
                QString dataChangedGuid = Util::newGuid();
                QMetaObject::Connection conn = connect(env->ds.mcuData, &DS_McuData::signalDataChanged_SyringeStates, this, [=] {
                    if (!env->ds.mcuData->getSyringesBusy())
                    {
                        // Syringes are all stopped
                        env->actionMgr->deleteActCompleted(dataChangedGuid);
                        DataServiceActionStatus curStatus = status;
                        curStatus.err = "";
                        curStatus.state = DS_ACTION_STATE_COMPLETED;
                        actionCompleted(curStatus);
                    }
                });
                env->actionMgr->createActCompleted(dataChangedGuid, conn, QString(__PRETTY_FUNCTION__) + ": actStopAll: SyringeStates");
            }
            else
            {
                // Syringes are all stopped
                actionCompleted(curStatus);
            }
        }
    });

    return startAction("STOP", "", guid);
}

DataServiceActionStatus DS_McuAction::actCalSyringeAirCheck(SyringeIdx syringeIdx, QString actGuid)
{
    QString body = QString().asprintf("%d,%.1f,%d", syringeIdx,
                                      env->ds.capabilities->get_FluidControl_CalSyringeAirCheckFlow(),
                                      env->ds.capabilities->get_FluidControl_CalSyringeAirCheckTargetPressure());

    DataServiceActionStatus status = actionInit(actGuid, "CALSYRINGEAIRCHECK", body.CSTR());

    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);
        waitSyringeActionComplete(syringeIdx, "CALSYRINGEAIRCHECK", curStatus);
    });

    return startAction("CALSYRINGEAIRCHECK", body, guid);
}

DataServiceActionStatus DS_McuAction::actSyringeAirCheck(SyringeIdx syringeIdx, QString actGuid)
{
    QString body = QString().asprintf("%d,%.1f,%d", syringeIdx,
                                      env->ds.capabilities->get_AirCheck_SyringeAirCheckFlow(),
                                      env->ds.capabilities->get_AirCheck_SyringeAirCheckTargetPressure());

    DataServiceActionStatus status = actionInit(actGuid, "SYRINGEAIRCHECK", body.CSTR());

    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);
        waitSyringeActionComplete(syringeIdx, "SYRINGEAIRCHECK", curStatus);
    });

    // Action Start
    return startAction("SYRINGEAIRCHECK", body, guid);
}

DataServiceActionStatus DS_McuAction::actGetSyringeAirCheckData(SyringeIdx syringeIdx, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "GETSYRINGEAIRCHECKDATA", QString().asprintf("%s", ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdx).CSTR()));

    QString body = QString().asprintf("%d", syringeIdx);

    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);
        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            actionCompleted(curStatus, &status);
        }
    });

    return startAction("GETSYRINGEAIRCHECKDATA", body, guid);
}

DataServiceActionStatus DS_McuAction::actGetSyringeAirCheckCoeff(SyringeIdx syringeIdx, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "GETSYRINGEAIRCHECKCOEFF", QString().asprintf("%s", ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdx).CSTR()));

    QString body = QString().asprintf("%d", syringeIdx);

    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);
        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            actionCompleted(curStatus, &status);
        }
    });

    return startAction("GETSYRINGEAIRCHECKCOEFF", body, guid);
}

DataServiceActionStatus DS_McuAction::actGetSyringeAirVol(SyringeIdx syringeIdx, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "GETSYRINGEAIRVOL", QString().asprintf("%s", ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdx).CSTR()));

    QString body = QString().asprintf("%d", syringeIdx);

    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);
        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            actionCompleted(curStatus, &status);
        }
    });

    return startAction("GETSYRINGEAIRVOL", body, guid);
}

DataServiceActionStatus DS_McuAction::actSyringeRecoveryVacuum(SyringeIdx syringeIdx, QString actGuid)
{
    QString body = QString().asprintf("%d", syringeIdx);

    DataServiceActionStatus status = actionInit(actGuid, "RECOVERYVACUUM", body.CSTR());

    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);
        waitSyringeActionComplete(syringeIdx, "RECOVERYVACUUM", curStatus);
    });

    // Action Start
    return startAction("RECOVERYVACUUM", body, guid);
}

DataServiceActionStatus DS_McuAction::actCalSlack(SyringeIdx syringeIdx, QString actGuid)
{
    QString body = QString().asprintf("%d", syringeIdx);

    DataServiceActionStatus status = actionInit(actGuid, "CalSlack", body.CSTR());

    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);
        waitSyringeActionComplete(syringeIdx, "CALSLACK", curStatus);
    });

    // Action Start
    return startAction("CALSLACK", body, guid);
}

DataServiceActionStatus DS_McuAction::actSetSlack(SyringeIdx syringeIdx, double volume, QString actGuid)
{
    QString body = QString().asprintf("%d,%.1f", syringeIdx, volume);
    return startAction("SETSLACK", body.CSTR(), actGuid);
}

DataServiceActionStatus DS_McuAction::actPiston(const DS_McuDef::ActPistonParams &params, QString actGuid)
{
    QString body = QString().asprintf("%d,%.1f,%.1f", params.idx, params.vol, params.flow);

    DataServiceActionStatus status = actionInit(actGuid, "Piston", QString().asprintf("%s", body.CSTR()));

    // Action Started Callback
    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);
        waitSyringeActionComplete(params.idx, "PISTON", curStatus);
    });

    // Action Start
    return startAction("PISTON", body, guid);
}

DataServiceActionStatus DS_McuAction::actPistonAll(const DS_McuDef::ActPistonParams &params, QString actGuid)
{
    QString body = QString().asprintf("%s,%.1f,%.1f", params.arg.CSTR(), params.vol, params.flow);

    DataServiceActionStatus status = actionInit(actGuid, "PistonAll", QString().asprintf("%s", body.CSTR()));

    // Action Started Callback
    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);
        waitAllSyringeActionComplete("PISTON", curStatus);
    });

    // Action Start
    return startAction("PISTON", body, guid);
}

DataServiceActionStatus DS_McuAction::actLeds(const DS_McuDef::ActLedParamsList &paramsList, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "Leds");

    QString body;
    for (int i = 0; i < paramsList.length(); i++)
    {
        if (i > 0)
        {
            body += ",";
        }

        QString paramStr = ImrParser::ToImr_McuActLedParams(paramsList[i]);

        switch (paramsList[i].type)
        {
        case DS_McuDef::LED_CONTROL_TYPE_NO_CHANGE:
        case DS_McuDef::LED_CONTROL_TYPE_OFF:
        case DS_McuDef::LED_CONTROL_TYPE_SET:
            body += paramStr;
            break;
        default:
            status.arg = body;
            status.state = DS_ACTION_STATE_BAD_REQUEST;
            status.err = "BAD REQUEST";
            actionStarted(status);
            return status;
        }
    }

    status.arg = body;

    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            DS_McuDef::LedControlStatus ledControlStatus = env->ds.mcuData->getLedControlStatus();
            for (int i = 0; i < paramsList.length(); i++)
            {
                if ( (paramsList[i].type == DS_McuDef::LED_CONTROL_TYPE_OFF) ||
                     (paramsList[i].type == DS_McuDef::LED_CONTROL_TYPE_SET) )
                {
                    ledControlStatus.paramsList[i] = paramsList[i];
                }
            }
            env->ds.mcuData->setLedControlStatus(ledControlStatus);
        }
        actionStarted(curStatus, &status, false);

        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            // LED is set successfully, emit completed signal
            curStatus.state = DS_ACTION_STATE_COMPLETED;
            actionCompleted(curStatus, &status, false);
        }
    });

    return startAction("LEDS", body, guid);
}

DataServiceActionStatus DS_McuAction::actFill(const DS_McuDef::ActFillParams &params, QString actGuid)
{
    QString body = QString().asprintf("%d,%.1f,%.1f,%s", params.idx, params.vol, params.flow, ImrParser::ToImr_FillType(params.type).CSTR());

    DataServiceActionStatus status = actionInit(actGuid, "Fill", QString().asprintf("%s", body.CSTR()));

    // Action Started Callback
    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);
        waitSyringeActionComplete(params.idx, "FILL", curStatus);
    });

    // Action Start
    return startAction("FILL", body, guid);
}

DataServiceActionStatus DS_McuAction::actPrime(const DS_McuDef::ActPrimeParams &params, QString actGuid)
{
    QString body = QString().asprintf("%d,%.1f,%.1f,%.1f", params.idx, params.vol1, params.vol2, params.flow);

    DataServiceActionStatus status = actionInit(actGuid, "Prime", QString().asprintf("%s", body.CSTR()), QString().asprintf("%s", params.operationName.CSTR()));

    // Action Started Callback
    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);
        waitSyringeActionComplete(params.idx, "PRIME", curStatus);
    });

    // Action Start
    return startAction("PRIME", body, guid);
}

DataServiceActionStatus DS_McuAction::actAlarmClear(QString actGuid)
{
    return startAction("CLEARALARMS", "", actGuid);
}

DataServiceActionStatus DS_McuAction::actResetMuds(QString actGuid)
{
    return startAction("RESETMUDS", "", actGuid);
}

DataServiceActionStatus DS_McuAction::actHeatMaintainerOn(double temp, QString actGuid)
{
    QString body = QString().asprintf("%.1f", temp);
    return startAction("HEATER", body, actGuid);
}

DataServiceActionStatus DS_McuAction::actHwDigest(QString actGuid)
{
    return startAction("HWDIGEST", "", actGuid);
}

DataServiceActionStatus DS_McuAction::actInjectDigest(QString actGuid)
{
    return startAction("INJECTDIGEST", "", actGuid);
}

DataServiceActionStatus DS_McuAction::actBmsDigest(QString actGuid)
{
    return startAction("BMSDIGEST", "", actGuid);
}

DataServiceActionStatus DS_McuAction::actBmsCommand(int index, QString data, QString actGuid)
{
    QString body = QString().asprintf("%d,%s", index, data.CSTR());
    return startAction("BMSCOMMAND", body, actGuid);
}

DataServiceActionStatus DS_McuAction::actBmsEnableFET(int index, bool enabled, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "BmsEnableFET", QString().asprintf("%d;%s", index, enabled ? "Enable" : "Disable"), "", true);
    DS_McuDef::BMSDigests digests = env->ds.mcuData->getBMSDigests();

    if (index >= digests.length())
    {
        status.state = DS_ACTION_STATE_BAD_REQUEST;
        status.err = QString().asprintf("Bad Battery Index (%d)", index);
        actionStarted(status);
        return status;
    }

    const DS_McuDef::BMSDigest &digest = digests[index];
    bool fetEnToggleRequired = (enabled != digest.manufacturingStatus.allFetAction);

    if (!fetEnToggleRequired)
    {
        LOG_DEBUG("actBmsEnableFET(): toggle not required\n");
        status.state = DS_ACTION_STATE_STARTED;
        actionStarted(status);

        status.state = DS_ACTION_STATE_COMPLETED;
        actionCompleted(status);
        return status;
    }

    LOG_DEBUG("actBmsEnableFET(): toggle required\n");

    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            env->actionMgr->onActionCompleted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
                actionCompleted(curStatus, &status);
            });
        }
        actionStarted(curStatus, &status);
    });

    return actBmsCommand(index, "0022", guid);
}

DataServiceActionStatus DS_McuAction::actBmsBootCtl(int batteryIdx, bool enabled, QString actGuid)
{
    QString body = QString().asprintf("%d,%s", batteryIdx, enabled ? "on" : "off");
    return startAction("BMSBOOTCTL", body, actGuid);
}

DataServiceActionStatus DS_McuAction::actChgFetCtl(int batteryIdx, bool enabled, QString actGuid)
{
    QString body = QString().asprintf("%d,%s", batteryIdx, enabled ? "on" : "off");
    return startAction("CHGFETCTL", body, actGuid);
}

DataServiceActionStatus DS_McuAction::actVOutDchgCtl(bool enabled, QString actGuid)
{
    QString body = QString().asprintf("%s", enabled ? "on" : "off");
    DataServiceActionStatus status = actionInit(actGuid, "VOutDchgCtl", body, "", true);

    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);
        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            actionCompleted(curStatus, &status, false);
        }
    });

    return startAction("VOUTDCHGCTL", body, actGuid);
}

DataServiceActionStatus DS_McuAction::actInitPowerSources(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "InitPowerSources");

    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);
        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            actionCompleted(curStatus, &status, false);
        }
    });

    return startAction("INITPOWERSOURCES", "", actGuid);
}

DataServiceActionStatus DS_McuAction::actChargeBattery(int batteryIdx, bool enabled, QString actGuid)
{
    QString body = QString().asprintf("%d,%s", batteryIdx, enabled ? "on" : "off");
    DataServiceActionStatus status = actionInit(actGuid, "ChargeBattery", body, "", true);

    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);
        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            actionCompleted(curStatus, &status, false);
        }
    });

    return startAction("CHARGEBATTERY", body, actGuid);
}

DataServiceActionStatus DS_McuAction::actBatteryToUnit(int batteryIdx, bool enabled, QString actGuid)
{
    QString body = QString().asprintf("%d,%s", batteryIdx, enabled ? "on" : "off");
    DataServiceActionStatus status = actionInit(actGuid, "BatteryToUnit", body, "", true);

    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);
        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            actionCompleted(curStatus, &status, false);
        }
    });

    return startAction("BATTERYTOUNIT", body, actGuid);
}

DataServiceActionStatus DS_McuAction::actMainToUnit(bool enabled, QString actGuid)
{
    QString body = QString().asprintf("%s", enabled ? "on" : "off");
    DataServiceActionStatus status = actionInit(actGuid, "MainToUnit", body, "", true);

    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);
        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            actionCompleted(curStatus, &status, false);
        }
    });

    return startAction("MAINTOUNIT", body, actGuid);
}

DataServiceActionStatus DS_McuAction::actLockDoor(bool lock, QString actGuid)
{
    QString body = QString().asprintf("%d", lock ? 1 : 0);
    return startAction("DOORLOCK", body, actGuid);
}

DataServiceActionStatus DS_McuAction::actSetMotorModuleSerialNumber(SyringeIdx syringeIdx, QString serialNumber, QString actGuid)
{
    QString body = QString().asprintf("%d,%s", syringeIdx, serialNumber.CSTR());
    return startAction("SETMMSERIAL", body, actGuid);
}

DataServiceActionStatus DS_McuAction::actResetMcuHw(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "ResetMcuHw");
    status.state = DS_ACTION_STATE_STARTED;
    status.reply = "OK";

    emit mcuLink->signalLinkDisconnect();

    if (env->ds.capabilities->get_Developer_McuSimulationEnabled())
    {
        // No need to reset MCU Simulator
    }
    else
    {
        McuHardware::resetDevice(HW_DEVICE_TYPE_MCU);
    }

    return status;
}

DataServiceActionStatus DS_McuAction::actResetScbHw(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "ResetScbHw");
    status.state = DS_ACTION_STATE_STARTED;
    status.reply = "OK";

    emit mcuLink->signalLinkDisconnect();

    if (env->ds.capabilities->get_Developer_McuSimulationEnabled())
    {
    }
    else
    {
        McuHardware::resetDevice(HW_DEVICE_TYPE_SCB);
    }

    return status;
}

DataServiceActionStatus DS_McuAction::actSetBaseFanTemperature(int temperature, QString actGuid)
{
    QString body = QString().asprintf("%d", temperature);
    return startAction("SETBASEFANTEMPERATURE", body, actGuid);
}

DataServiceActionStatus DS_McuAction::actFindPlunger(SyringeIdx syringeIdx, QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "FindPlunger", QString().asprintf("%s", ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdx).CSTR()));

    if (env->ds.mcuData->getPlungerStates()[syringeIdx] == DS_McuDef::PLUNGER_STATE_ENGAGED)
    {
        //Plunger is already engaged, therefore already found
        status.state = DS_ACTION_STATE_STARTED;
        actionStarted(status);

        status.state = DS_ACTION_STATE_COMPLETED;
        actionCompleted(status);
        return status;
    }

    // Action Started Callback
    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);
        waitSyringeActionComplete(syringeIdx, "FINDPLUNGER", curStatus);
    });

    QString body = QString().asprintf("%d", syringeIdx);
    return startAction("FINDPLUNGER", body, guid);
}

DataServiceActionStatus DS_McuAction::actFindPlungerAll(QString actGuid)
{
    DataServiceActionStatus status = actionInit(actGuid, "FindPlungerAll");

    if (env->ds.mcuData->getPlungersEngaged())
    {
        //Plungers are already engaged, therefore already found
        status.state = DS_ACTION_STATE_STARTED;
        actionStarted(status);
        actionCompleted(status);
        return status;
    }

    // Action Started Callback
    QString guid = Util::newGuid();
    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        actionStarted(curStatus, &status);
        waitAllSyringeActionComplete("FINDPLUNGER", curStatus);
    });
    return startAction("FINDPLUNGER", "all", guid);
}
