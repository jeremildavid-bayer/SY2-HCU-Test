#include "HCU_McuSimulator.h"
#include <QFile>
#include <QShortcut>
#include <QThread>
#include "Syringe.h"
#include "StopCock.h"

HCU_McuSimulator::HCU_McuSimulator(QObject *parent) :
    QObject(parent)
{
    qmlEngine = new QQmlApplicationEngine;
    qmlContext = new QQmlContext(qmlEngine->rootContext());

    ImxqConfig *config = new ImxqConfig(this, PATH_CONFIG_LOCAL);
    int titleBarH = config->getInt("TITLEBAR_H_PIXELS");
    qmlContext->setContextProperty("mainY", QVariant(titleBarH));
    qmlContext->setContextProperty("mainWidth", QVariant(config->getInt("SCREEN_W_PIXELS")));
    qmlContext->setContextProperty("mainHeight", QVariant(config->getInt("SCREEN_H_PIXELS") - titleBarH));
    qmlContext->setContextProperty("currentItemIdx", QVariant(0));
    delete config;

    qmlComponent = new QQmlComponent(qmlEngine, QUrl("qrc:/qrc/main.qml"));
    qmlObject = qmlComponent->create(qmlContext);

    connect(qmlObject, SIGNAL(qmlSignalClose()), this, SLOT(slotExit()));
    connect(qmlObject, SIGNAL(qmlSignalToggleMuds()), this,SLOT(slotToggleMuds()));
    connect(qmlObject, SIGNAL(qmlSignalToggleSuds()), this,SLOT(slotToggleSuds()));
    connect(qmlObject, SIGNAL(qmlSignalToggleWasteBin()), this,SLOT(slotToggleWasteBin()));
    connect(qmlObject, SIGNAL(qmlSignalBubblePatientLine()), this,SLOT(slotToggleBubblePatientLine()));
    connect(qmlObject, SIGNAL(qmlSignalBubbleSaline()), this,SLOT(slotToggleBubbleSaline()));
    connect(qmlObject, SIGNAL(qmlSignalBubbleContrast1()), this,SLOT(slotToggleBubbleContrast1()));
    connect(qmlObject, SIGNAL(qmlSignalBubbleContrast2()), this,SLOT(slotToggleBubbleContrast2()));
    connect(qmlObject, SIGNAL(qmlSignalToggleDoor()), this,SLOT(slotToggleDoor()));
    connect(qmlObject, SIGNAL(qmlSignalHideMenu()), this, SLOT(slotMainIconPressed()));
    connect(qmlObject, SIGNAL(qmlSignalStopButtonPressed()), this, SLOT(slotStopButtonPressed()));
    connect(qmlObject, SIGNAL(qmlSignalStopButtonReleased()), this, SLOT(slotStopButtonReleased()));
    connect(qmlObject, SIGNAL(qmlSignalResetMuds()), this, SLOT(slotResetMuds()));

    envInit();
    hardwareInit();
    mcuStateMachine = new McuStateMachine(this, salineControlSet, contrast1ControlSet, contrast2ControlSet, env);
    formMainIcon = new FormMainIcon(this, env);
    connect(formMainIcon, SIGNAL(signalIconPresed()), SLOT(slotMainIconPressed()));

    connect(&tmrUpdateEventMessages, SIGNAL(timeout()), this, SLOT(slotTmrUpdateEventMessagesTimeout()));
    tmrUpdateEventMessages.start(400);
}

HCU_McuSimulator::~HCU_McuSimulator()
{
    tmrInjectionState.stop();
    tmrMcuState.stop();
    tmrUpdateEventMessages.stop();

    delete mcuStateMachine;
    envDeinit();
    hardwareDeinit();
    delete formMainIcon;
    delete qmlContext;
    delete qmlComponent;
    delete qmlEngine;
}

void HCU_McuSimulator::hardwareInit()
{
    salineControlSet = new ControlGroup();
    contrast1ControlSet = new ControlGroup();
    contrast2ControlSet = new ControlGroup();

    salineControlSet->syringe = new Syringe(this, env);
    salineControlSet->syringe->setSyringeName("SALINE");
    contrast1ControlSet->syringevb = new Syringe(this, env);
    contrast1ControlSet->syringe->setSyringeName("CONTRAST1");
    contrast2ControlSet->syringe = new Syringe(this, env);
    contrast2ControlSet->syringe->setSyringeName("CONTRAST2");

    salineControlSet->stopCock = new StopCock();
    contrast1ControlSet->stopCock = new StopCock();
    contrast2ControlSet->stopCock = new StopCock();

    connect(salineControlSet->syringe, SIGNAL(signalSyringeActionDone(ImxProt_EvtSyringeActionDone)), SLOT(slotSalineSyringeActionComplete(ImxProt_EvtSyringeActionDone)));
    connect(contrast1ControlSet->syringe, SIGNAL(signalSyringeActionDone(ImxProt_EvtSyringeActionDone)), SLOT(slotContrast1SyringeActionComplete(ImxProt_EvtSyringeActionDone)));
    connect(contrast2ControlSet->syringe, SIGNAL(signalSyringeActionDone(ImxProt_EvtSyringeActionDone)), SLOT(slotContrast2SyringeActionComplete(ImxProt_EvtSyringeActionDone)));

    setDefaultValues();
}

void HCU_McuSimulator::setDefaultValues()
{
    //--------------------------------------------------------------
    // DEFAULTS
    //--------------------------------------------------------------
    hwMuds.present = false;
    hwMuds.fullyEngaged = false;
    hwMudsDoorState = DOOR_OPEN;
    hwWasteBinState = HW_DATA_WASTE_BIN_OK;
    hwPatientAirDetected = false;
    hwBatteryStatus = BATTERY_MAIN;
    hwSuds = true;
    hwStopBtn = HW_BTN_RELEASED;
    salineControlSet->airDetected = false;
    contrast1ControlSet->airDetected = false;
    contrast2ControlSet->airDetected = false;
}

void HCU_McuSimulator::hardwareDeinit()
{
    delete salineControlSet->syringe;
    delete contrast1ControlSet->syringe;
    delete contrast2ControlSet->syringe;

    delete salineControlSet->stopCock;
    delete contrast1ControlSet->stopCock;
    delete contrast2ControlSet->stopCock;

    delete salineControlSet;
    delete contrast1ControlSet;
    delete contrast2ControlSet;
}

void HCU_McuSimulator::envInit()
{
    env = &environment;

    env->configLocal = new ImxqConfig(this, PATH_CONFIG_LOCAL);

    env->log = new ImxqLog(PATH_LOG);
    env->log->setLogLevel(LOG_LEVEL_DEBUG);
    env->log->open();
    LOG_INFO("==================================================\n");
    LOG_INFO("MAIN: Starting HCU_McuSimulator\n");

    env->comm = new ImxqCommTcpClient(this, IMX_PROT_NODE_HCU_MCU_SIMULATOR);
    env->comm->setTcpCfg("127.0.0.1", IMX_PROT_TCP_PORT_HCU_MCU_SIMULATOR);

    //connect(env->comm, SIGNAL(signalDebugMsg(QString)), SLOT(slotCommDebugMsg(QString)));

    connect(env->comm, SIGNAL(signalConnected()), SLOT(slotCommConnected()));
    connect(env->comm, SIGNAL(signalDisconnected()), SLOT(slotCommDisconnected()));
    connect(env->comm, SIGNAL(signalErrorMsg(QString)), SLOT(slotCommErrorMsg(QString)));
    connect(env->comm, SIGNAL(signalRxMsg(void*,uint8_t)), SLOT(slotCommRxMsg(void*,uint8_t)));
    env->comm->openComm(4000, true);

    env->alarmHighFlag = 0;
    env->alarmLowFlag = 0;
}

void HCU_McuSimulator::envDeinit()
{
    delete env->log;
    delete env->configLocal;
    delete env->comm;
}

void HCU_McuSimulator::slotTmrUpdateEventMessagesTimeout()
{
    updateBatteryStatus();
    updateHwData();
    updateInjectProgress();
    updateSyringeState();
}

void HCU_McuSimulator::getSyringeStateAck(ImxProt_AckGetInjectorSyringeState *syringeState)
{
    getSyringeState((ImxProt_EvtInjectorSyringeState*)syringeState);
}

void HCU_McuSimulator::getSyringeState(ImxProt_EvtInjectorSyringeState *syringeState)
{
    Syringe::SyringeInfo syrInfo = salineControlSet->syringe->getInfo();
    syringeState->vol[SYRINGE_IDX_SALINE] = syrInfo.volume;
    syringeState->flow[SYRINGE_IDX_SALINE] = syrInfo.flow;
    syringeState->plungerStatus[SYRINGE_IDX_SALINE] = syrInfo.engaged ? PLUNGER_STATUS_PUSHING : PLUNGER_STATUS_DISENGAGED;

    syrInfo = contrast1ControlSet->syringe->getInfo();
    syringeState->vol[SYRINGE_IDX_CONTRAST1] = syrInfo.volume;
    syringeState->flow[SYRINGE_IDX_CONTRAST1] = syrInfo.flow;
    syringeState->plungerStatus[SYRINGE_IDX_CONTRAST1] = syrInfo.engaged ? PLUNGER_STATUS_PUSHING : PLUNGER_STATUS_DISENGAGED;

    syrInfo = contrast2ControlSet->syringe->getInfo();
    syringeState->vol[SYRINGE_IDX_CONTRAST2] = syrInfo.volume;
    syringeState->flow[SYRINGE_IDX_CONTRAST2] = syrInfo.flow;
    syringeState->plungerStatus[SYRINGE_IDX_CONTRAST2] = syrInfo.engaged ? PLUNGER_STATUS_PUSHING : PLUNGER_STATUS_DISENGAGED;

    syringeState->pressure = 0; //TODO - maybe
}

void HCU_McuSimulator::updateSyringeState()
{
    getSyringeState(&currentSyringeState);

    if (memcmp(&currentSyringeState, &lastSyringeState, sizeof(currentSyringeState)) != 0)
    {
        env->comm->txMsg(IMX_PROT_INJECTOR_SYRINGE_STATE, IMX_PROT_TYPE_EVENT, IMX_PROT_NODE_HCU_CONTROLLER, &currentSyringeState, sizeof(currentSyringeState));
    }

    lastSyringeState = currentSyringeState;
}

void HCU_McuSimulator::updateInjectProgress()
{
    currentInjectProgress.adaptiveFlowActivated = false;
    currentInjectProgress.curPhaseIdx = mcuStateMachine->getCurPhase();
    currentInjectProgress.state = mcuStateMachine->getInjectProgressState();
    currentInjectProgress.completeState = (uint8_t)mcuStateMachine->getInjectCompleteState();
    currentInjectProgress.completeAlarmId = 0;
    currentInjectProgress.maxPressureMonitored = 0;
    currentInjectProgress.protIdx = mcuStateMachine->getActiveProtIdx();
    mcuStateMachine->getElapsedTimes(currentInjectProgress.elapsedFromStartMs);
    mcuStateMachine->getInjectedVol(&currentInjectProgress.injectedVol);

    if (mcuStateMachine->getState() == McuStateMachine::MCU_STATE_INJECTING)
    {
        if (memcmp(&currentInjectProgress, &lastInjectProgress, sizeof(currentInjectProgress)) != 0)
        {
            env->comm->txMsg(IMX_PROT_INJECT_PROGRESS, IMX_PROT_TYPE_EVENT, IMX_PROT_NODE_HCU_CONTROLLER, &currentInjectProgress, sizeof(currentInjectProgress));
        }

        if (currentInjectProgress.state == INJECT_PROGRESS_DONE)
        {
            //Allow the first DONE msg to be sent then change the state
            mcuStateMachine->setState(McuStateMachine::MCU_STATE_FILL);
        }
    }

    lastInjectProgress = currentInjectProgress;
}

void HCU_McuSimulator::updateHwData()
{
    currentHwData.id = HW_DATA_DIGEST;
    currentHwData.dataLen = sizeof(HwDataDigest);
    HwDataDigest *digest = (HwDataDigest*)currentHwData.value;
    getHwDataDigest(digest);

    if (memcmp(&currentHwData, &lastHwData, sizeof(currentHwData)) != 0)
    {
        env->comm->txMsg(IMX_PROT_HW_DATA_VALUE, IMX_PROT_TYPE_EVENT, IMX_PROT_NODE_HCU_CONTROLLER, &currentHwData, sizeof(currentHwData));
    }

    memcpy(&lastHwData, &currentHwData, sizeof(lastHwData));
}

void HCU_McuSimulator::getHwDataDigest(HwDataDigest *digest)
{
    digest->bottleBubbleDetected[SYRINGE_IDX_SALINE] = salineControlSet->airDetected;
    digest->bottleBubbleDetected[SYRINGE_IDX_CONTRAST1] = contrast1ControlSet->airDetected;
    digest->bottleBubbleDetected[SYRINGE_IDX_CONTRAST2] = contrast2ControlSet->airDetected;

    digest->mudsDoorState = hwMudsDoorState;
    digest->mudsState = hwMuds.fullyEngaged ? HW_DATA_MUDS_OK : HW_DATA_MUDS_NOT_AVAILABLE;
    digest->plungerLock[SYRINGE_IDX_SALINE] = HW_PLUNGER_LOCK_OPEN;
    digest->plungerLock[SYRINGE_IDX_CONTRAST1] = HW_PLUNGER_LOCK_OPEN;
    digest->plungerLock[SYRINGE_IDX_CONTRAST2] = HW_PLUNGER_LOCK_OPEN;
    digest->primeBtnState = HW_BTN_RELEASED; //TODO? is it needed?
    digest->stopBtnState = hwStopBtn;
    digest->stopcockEngaged[SYRINGE_IDX_SALINE] = salineControlSet->stopCock->isEngaged();
    digest->stopcockEngaged[SYRINGE_IDX_CONTRAST1] = contrast1ControlSet->stopCock->isEngaged();
    digest->stopcockEngaged[SYRINGE_IDX_CONTRAST2] = contrast2ControlSet->stopCock->isEngaged();
    digest->sudsBubbleDetected = hwPatientAirDetected;
    digest->sudsState = hwSuds ? HW_DATA_SUDS_OK : HW_DATA_SUDS_NOT_AVAILABLE;
    digest->wasteBinState = hwWasteBinState;
    digest->stopcockPos[SYRINGE_IDX_SALINE] = salineControlSet->stopCock->getPosition();
    digest->stopcockPos[SYRINGE_IDX_CONTRAST1] = contrast1ControlSet->stopCock->getPosition();
    digest->stopcockPos[SYRINGE_IDX_CONTRAST2] = contrast2ControlSet->stopCock->getPosition();
}

void HCU_McuSimulator::updateBatteryStatus()
{
    currentBatteryStatus.batteryStatus = hwBatteryStatus;

    if (memcmp(&currentBatteryStatus, &lastBatteryStatus, sizeof(currentBatteryStatus)) != 0)
    {
        env->comm->txMsg(IMX_PROT_BATTERY_STATUS, IMX_PROT_TYPE_EVENT, IMX_PROT_NODE_HCU_CONTROLLER, &currentBatteryStatus, sizeof(currentBatteryStatus));
    }

    lastBatteryStatus = currentBatteryStatus;
}

void HCU_McuSimulator::slotSalineSyringeActionComplete(ImxProt_EvtSyringeActionDone state)
{
    updateSyringeState();
    state.syringeIdx = SYRINGE_IDX_SALINE;
    if (mcuStateMachine->getState() == McuStateMachine::MCU_STATE_FILL)
    {
        env->comm->txMsg(IMX_PROT_SYRINGE_ACTION_DONE, IMX_PROT_TYPE_EVENT, IMX_PROT_NODE_HCU_CONTROLLER, &state, sizeof(state));
        LOG_DEBUG("MAIN: Action done msg - syringeIdx(%d), actionType(%d), result(%d)\n", state.syringeIdx, state.type, state.result);
    }
}

void HCU_McuSimulator::slotContrast1SyringeActionComplete(ImxProt_EvtSyringeActionDone state)
{
    updateSyringeState();
    state.syringeIdx = SYRINGE_IDX_CONTRAST1;
    if (mcuStateMachine->getState() == McuStateMachine::MCU_STATE_FILL)
    {
        env->comm->txMsg(IMX_PROT_SYRINGE_ACTION_DONE, IMX_PROT_TYPE_EVENT, IMX_PROT_NODE_HCU_CONTROLLER, &state, sizeof(state));
        LOG_DEBUG("MAIN: Action done msg - syringeIdx(%d), actionType(%d), result(%d)\n", state.syringeIdx, state.type, state.result);
    }
}

void HCU_McuSimulator::slotContrast2SyringeActionComplete(ImxProt_EvtSyringeActionDone state)
{
    updateSyringeState();
    state.syringeIdx = SYRINGE_IDX_CONTRAST2;
    if (mcuStateMachine->getState() == McuStateMachine::MCU_STATE_FILL)
    {
        env->comm->txMsg(IMX_PROT_SYRINGE_ACTION_DONE, IMX_PROT_TYPE_EVENT, IMX_PROT_NODE_HCU_CONTROLLER, &state, sizeof(state));
        LOG_DEBUG("MAIN: Action done msg - syringeIdx(%d), actionType(%d), result(%d)\n", state.syringeIdx, state.type, state.result);
    }
}

void HCU_McuSimulator::showMainUi(bool show)
{
    QMetaObject::invokeMethod(qmlObject, "slotQmlSetVisible", Q_ARG(QVariant,   show));
}

void HCU_McuSimulator::slotMainIconPressed()
{
    static bool show = true;
    LOG_DEBUG("MAIN: MainIcon Pressed (show = %s)\n", show ? "true" : "false");
    showMainUi(show);
    show = !show;
}

void HCU_McuSimulator::slotExit()
{
    LOG_DEBUG("MAIN: Exit Requested..\n");
    QApplication::exit();
}

void HCU_McuSimulator::slotToggleMuds()
{
    hwMuds.fullyEngaged = !hwMuds.fullyEngaged;
}

void HCU_McuSimulator::slotToggleSuds()
{
    hwSuds = !hwSuds;
    if (!hwSuds)
    {
        mcuStateMachine->triggerSudsRemoved();
    }
}

void HCU_McuSimulator::slotToggleDoor()
{
    HwMudsDoorState door = hwMudsDoorState;
    switch (door)
    {
//    case DOOR_CLOSED_LOCKED:
//        door = DOOR_CLOSED_UNLOCKED;
//        LOG_DEBUG("MAIN: slotToggleDoor() -> DOOR_CLOSED_LOCKED\n");
//        break;
    case DOOR_CLOSED_UNLOCKED:
        door = DOOR_OPEN;
        LOG_DEBUG("MAIN: slotToggleDoor() -> DOOR_OPEN\n");
        break;
    case DOOR_OPEN:
        door = DOOR_CLOSED_UNLOCKED;
        LOG_DEBUG("MAIN: slotToggleDoor() -> DOOR_CLOSED_UNLOCKED\n");
        break;
    default:
        door = DOOR_OPEN;
        LOG_DEBUG("MAIN: slotToggleDoor() -> DOOR_OPEN\n");
        break;
    }
    hwMudsDoorState = door;
}

void HCU_McuSimulator::slotToggleWasteBin()
{
    HwDataWasteBinState bin = hwWasteBinState;
    switch(bin)
    {
    case HW_DATA_WASTE_BIN_OK:
        bin = HW_DATA_WASTE_BIN_FULL;
        break;
    case HW_DATA_WASTE_BIN_FULL:
        bin = HW_DATA_WASTE_BIN_NOT_AVAILABLE;
        break;
    case HW_DATA_WASTE_BIN_NOT_AVAILABLE:
        bin = HW_DATA_WASTE_BIN_OK;
        break;
    default:
        bin = HW_DATA_WASTE_BIN_OK;
        break;
    }
    hwWasteBinState = bin;
}

void HCU_McuSimulator::slotToggleBubblePatientLine()
{
    hwPatientAirDetected = !hwPatientAirDetected;
    mcuStateMachine->triggerAirDetected();
}

void HCU_McuSimulator::slotToggleBubbleSaline()
{
    salineControlSet->airDetected = !salineControlSet->airDetected;
    mcuStateMachine->triggerSalineAirDetector();
}

void HCU_McuSimulator::slotToggleBubbleContrast1()
{
    contrast1ControlSet->airDetected = !contrast1ControlSet->airDetected;
    mcuStateMachine->triggerContrast1AirDetector();
}

void HCU_McuSimulator::slotToggleBubbleContrast2()
{
    contrast2ControlSet->airDetected = !contrast2ControlSet->airDetected;
    mcuStateMachine->triggerContrast2AirDetector();
}

void HCU_McuSimulator::slotStopButtonPressed()
{
    mcuStateMachine->stopButtonClicked();
    hwStopBtn = HW_BTN_PRESSED;

    ImxProt_EvtHwDataValue hwData;
    hwData.id = HW_DATA_BTN_STOP;
    hwData.dataLen = 1;
    hwData.value[0] = HW_BTN_PRESSED;
    env->comm->txMsg(IMX_PROT_HW_DATA_VALUE, IMX_PROT_TYPE_EVENT, IMX_PROT_NODE_HCU_CONTROLLER, &hwData, sizeof(hwData));
}

void HCU_McuSimulator::slotStopButtonReleased()
{
    hwStopBtn = HW_BTN_RELEASED;

    ImxProt_EvtHwDataValue hwData;
    hwData.id = HW_DATA_BTN_STOP;
    hwData.dataLen = 1;
    hwData.value[0] = HW_BTN_RELEASED;
    env->comm->txMsg(IMX_PROT_HW_DATA_VALUE, IMX_PROT_TYPE_EVENT, IMX_PROT_NODE_HCU_CONTROLLER, &hwData, sizeof(hwData));
}

void HCU_McuSimulator::slotResetMuds()
{
    if (mcuStateMachine->resetMuds())
    {
        setDefaultValues();
        QMetaObject::invokeMethod(qmlObject, "slotQmlResetButtonStates");
    }
    else
    {
        //error beep?
    }
}

QString HCU_McuSimulator::getLedColorStrFromIdx(uint8_t colorIdx)
{
    QString color = "grey";
    switch (colorIdx)
    {
        case HW_DATA_LED_COLOUR_NO_CHANGE:
            break;
        case HW_DATA_LED_COLOUR_OFF:
            color = "grey";
            break;
        case HW_DATA_LED_COLOUR_RED:
            color = "red";
            break;
        case HW_DATA_LED_COLOUR_GREEN:
            color = "#32c800";
            break;
        case HW_DATA_LED_COLOUR_BLUE:
            color = "#00c8ff";
            break;
        case HW_DATA_LED_COLOUR_WHITE:
            color = "white";
            break;
        case HW_DATA_LED_COLOUR_ORANGE:
            color = "orange";
            break;
        case HW_DATA_LED_COLOUR_PURPLE:
            color = "#c864ff";
            break;
        default:
            break;
    }
    return color;
}
