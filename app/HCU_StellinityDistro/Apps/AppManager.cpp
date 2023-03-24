#include <QCoreApplication>
#include <QZXing.h>
#include "AppManager.h"
#include "Common/ImrParser.h"
#include "Common/ActionManager.h"

#include "DataServices/Workflow/DS_WorkflowData.h"
#include "DataServices/Workflow/DS_WorkflowAction.h"
#include "DataServices/Device/DS_DeviceData.h"
#include "DataServices/Device/DS_DeviceAction.h"
#include "DataServices/Exam/DS_ExamData.h"
#include "DataServices/Exam/DS_ExamAction.h"
#include "DataServices/Mcu/DS_McuData.h"
#include "DataServices/Mcu/DS_McuAction.h"
#include "DataServices/Cru/DS_CruData.h"
#include "DataServices/Cru/DS_CruAction.h"
#include "DataServices/Mwl/DS_MwlData.h"
#include "DataServices/Mwl/DS_MwlAction.h"
#include "DataServices/ImrServer/DS_ImrServerData.h"
#include "DataServices/ImrServer/DS_ImrServerAction.h"
#include "DataServices/HardwareInfo/DS_HardwareInfo.h"
#include "DataServices/CfgLocal/DS_CfgLocal.h"
#include "DataServices/CfgGlobal/DS_CfgGlobal.h"
#include "DataServices/Capabilities/DS_Capabilities.h"
#include "DataServices/System/DS_SystemData.h"
#include "DataServices/System/DS_SystemAction.h"
#include "DataServices/Alert/DS_AlertAction.h"
#include "DataServices/Alert/DS_AlertData.h"
#include "DataServices/Upgrade/DS_UpgradeAction.h"
#include "DataServices/Upgrade/DS_UpgradeData.h"
#include "DataServices/Test/DS_TestAction.h"
#include "DataServices/Test/DS_TestData.h"
#include "Common/Translator.h"

AppManager::AppManager(QObject *parent) : QObject(parent)
{
    envLocal = new EnvLocal("Main", "MAIN");
    env = new EnvGlobal(this);

    env->errLog = NULL;
    env->allLog = NULL;

    env->appManager = this;

    // Read version & buildDate
    QFile fileVer(PATH_VERSION_INFO);
    if (fileVer.open(QFile::ReadOnly | QFile::Text))
    {
        LOG_INFO("%s:\n%s\n", PATH_VERSION_INFO, fileVer.readAll().CSTR());
        fileVer.close();
    }
    else
    {
        QString err = QString().asprintf("File Read Error (%s)\n", PATH_VERSION_INFO);
        LOG_ERROR("%s", err.CSTR());
        env->ds.alertAction->activate("HCUInternalSoftwareError", err);

    }

    // Initiate Error Log: this contains all warning/error type message from all logs
    env->errLog = new Log(PATH_LOG_DIR + QString("Error.log"), LOG_MID_SIZE_BYTES);
    env->errLog->open();
    env->errLog->writeInfo("\n\n\n");
    env->errLog->writeInfo("===========================================================\n");
    env->errLog->writeInfo("HCU_Stellinity Distro - ErrLog Started..\n");

    env->allLog = new Log(PATH_LOG_DIR + QString("All.log"),LOG_LRG_SIZE_BYTES);
    env->allLog->open();
    env->allLog->writeInfo("\n\n\n");
    env->allLog->writeInfo("===========================================================\n");
    env->allLog->writeInfo("HCU_Stellinity Distro - AllLog Started..\n");

    // Initiate Config Services
    env->state = EnvGlobal::STATE_INITIALISING_CFG_SERVICES;
    env->ds.alertData = new DS_AlertData(this, env);
    env->ds.systemData = new DS_SystemData(this, env);
    env->ds.hardwareInfo = new DS_HardwareInfo(this, env);
    env->ds.capabilities = new DS_Capabilities(this, env);
    env->ds.cfgLocal = new DS_CfgLocal(this, env);
    env->ds.cfgGlobal = new DS_CfgGlobal(this, env);

    // Set Human Use State
    QString buildType = env->ds.systemData->getBuildType();
    bool tradeShowModeEnabled = env->ds.cfgGlobal->get_Service_TradeshowModeEnabled();
    env->humanUseAllowed = (buildType == BUILD_TYPE_REL) && (!tradeShowModeEnabled);

    // Start playing intro video
    procStartVideo = NULL;
    if ( (env->ds.systemData->getBuildType() != BUILD_TYPE_PROD) &&
         (env->ds.capabilities->get_General_StartVideoEnabled()) )
    {
        procStartVideo = new QProcess();
        connect(procStartVideo, SIGNAL(finished(int)), this, SLOT(slotShowUi()));
        int volumeLevel = env->ds.cfgLocal->get_Settings_Sound_NormalAudioLevel();
        int volumeLevelPercent = 0;
        if (volumeLevel != DS_CfgLocalDef::SOUND_VOLUME_LEVEL_MUTE)
        {
            // Adjust volume level (not linear)
            int volumeLevelMin = 40;
            volumeLevelPercent = (volumeLevel - 1) * (((100 - volumeLevelMin) / (DS_CfgLocalDef::SOUND_VOLUME_LEVEL_MAX - DS_CfgLocalDef::SOUND_VOLUME_LEVEL_MIN)));
            volumeLevelPercent += volumeLevelMin;
        }
        procStartVideo->start("mplayer", QStringList() << PATH_INTRO_VIDEO << "-fs" << "-framedrop" << "-volume" << QString().asprintf("%d", volumeLevelPercent));
    }

    env->actionMgr = new ActionManager(this, env);

    // Initiate Data Services
    env->state = EnvGlobal::STATE_INITIALISING_DATA_SERVICES;
    env->ds.alertAction = new DS_AlertAction(this, env);
    env->ds.workflowData = new DS_WorkflowData(this, env);
    env->ds.workflowAction = new DS_WorkflowAction(this, env);
    env->ds.deviceData = new DS_DeviceData(this, env);
    env->ds.deviceAction = new DS_DeviceAction(this, env);
    env->ds.examData = new DS_ExamData(this, env);
    env->ds.examAction = new DS_ExamAction(this, env);
    env->ds.mcuData = new DS_McuData(this, env);
    env->ds.mcuAction = new DS_McuAction(this, env);
    env->ds.systemAction = new DS_SystemAction(this, env);
    env->ds.cruData = new DS_CruData(this, env);
    env->ds.cruAction = new DS_CruAction(this, env);
    env->ds.mwlData = new DS_MwlData(this, env);
    env->ds.mwlAction = new DS_MwlAction(this, env);
    env->ds.imrServerData = new DS_ImrServerData(this, env);
    env->ds.imrServerAction = new DS_ImrServerAction(this, env);
    env->ds.testData = new DS_TestData(this, env);
    env->ds.testAction = new DS_TestAction(this, env);
    env->ds.upgradeData = new DS_UpgradeData(this, env);
    env->ds.upgradeAction = new DS_UpgradeAction(this, env);

    env->state = EnvGlobal::STATE_INTRO_MEDIA_COMPLETE_WAITING;

    /*
     * Must call initQmlApp() to continue the intialise sequence
     */
}

/**
 * Must be called right after the contructor return.
 * This must be done this way because Qt does not want a constructor to emit signal
 *
 * @brief AppManager::initQmlApp
 * @return true on success, false otherwise and the app should stop
 */
bool AppManager::initQmlApp()
{
    // Init MainWindow
    env->state = EnvGlobal::STATE_INITIALISING_QML_APP;


    env->qml.engine = new QQmlEngine();
    env->qml.object = nullptr;

    // Init QZXing
    QZXing::registerQMLTypes();
    QZXing::registerQMLImageProvider(*env->qml.engine);

    env->qml.engine->rootContext()->setContextProperty("dsApp", this);
    env->qml.context = new QQmlContext(env->qml.engine->rootContext());

    //need env->qml.engine->rootContext()
    env->translator = new Translator(this, env);

    env->qml.component = new QQmlComponent(env->qml.engine, QUrl("qrc:/Qml/Apps/Qml/AppManager.qml"));

    if (!env->qml.component->isLoading())
    {
        slotContinueLoading();
    }
    else
    {
        qDebug() << "delay loading";
        QObject::connect(env->qml.component, &QQmlComponent::statusChanged, this, &AppManager::slotContinueLoading);
    }
    return true;
}

void AppManager::slotContinueLoading()
{
    if (!env->qml.component->isReady())
    {
        if (procStartVideo != NULL)
        {
            disconnect(procStartVideo, SIGNAL(finished(int)), this, SLOT(slotShowUi()));
        }

        //need?? env->state = EnvGlobal::STATE_EXITING;
        QString err = QString().asprintf("Failed to create QML Component. Error=%s\n", env->qml.component->errorString().CSTR());
        qCritical() << err;
        env->ds.alertAction->activate("HCUInternalSoftwareError", err);
        slotExit();
        return;
    }

    env->qml.object = env->qml.component->create(env->qml.context);


    // Create QML Data Service modules
    env->state = EnvGlobal::STATE_INITIALISING_QML_SERVICES;
    qmlMcu = new QML_Mcu(this, env);
    qmlMcuSim = new QML_McuSim(this, env);
    qmlCru = new QML_Cru(this, env);
    qmlMwl = new QML_Mwl(this, env);
    qmlExam = new QML_Exam(this, env);
    qmlDevice = new QML_Device(this, env);
    qmlSystem = new QML_System(this, env);
    qmlTest = new QML_Test(this, env);
    qmlAlert = new QML_Alert(this, env);
    qmlImrServer = new QML_ImrServer(this, env);
    qmlUpgrade = new QML_Upgrade(this, env);
    qmlHardwareInfo = new QML_HardwareInfo(this, env);
    qmlCfgLocal = new QML_CfgLocal(this, env);
    qmlCfgGlobal = new QML_CfgGlobal(this, env);
    qmlCfgCapabilities = new QML_Capabilities(this, env);
    qmlWorkflow = new QML_Workflow(this, env);
    qmlDevTools = new QML_DevTools(this, env);

    env->state = EnvGlobal::STATE_INIT_COMPLETING;
    emit signalAppInitialised();

    // HCUApplicationStarted should be the first active alerts during start up
    QString hcuVersion = env->ds.systemData->getVersion();
    QString hcuBuildType = env->ds.systemData->getBuildType();
    QString alertData = QString().asprintf("%s %s", hcuVersion.CSTR(), hcuBuildType.CSTR());
    env->ds.alertAction->activate("HCUApplicationStarted", alertData);

    env->state = EnvGlobal::STATE_STARTING;
    emit signalAppStarted();

    // Start UI
    if (procStartVideo == NULL)
    {
        slotShowUi();
    }
}

AppManager::~AppManager()
{
    delete env->ds.workflowData;
    delete env->ds.workflowAction;
    delete env->ds.deviceData;
    delete env->ds.deviceAction;
    delete env->ds.examData;
    delete env->ds.examAction;
    delete env->ds.mcuData;
    delete env->ds.mcuAction;
    delete env->ds.cruData;
    delete env->ds.cruAction;
    delete env->ds.mwlData;
    delete env->ds.mwlAction;
    delete env->ds.imrServerData;
    delete env->ds.imrServerAction;
    delete env->ds.testData;
    delete env->ds.testAction;
    delete env->ds.hardwareInfo;
    delete env->ds.capabilities;
    delete env->ds.cfgLocal;
    delete env->ds.cfgGlobal;
    delete env->ds.systemData;
    delete env->ds.systemAction;
    delete env->ds.alertData;
    delete env->ds.alertAction;
    delete env->ds.upgradeData;
    delete env->ds.upgradeAction;

    delete env->qml.context;
    delete env->qml.component;
    delete env->actionMgr;
    delete env->errLog;
    delete env->allLog;
    delete env->translator;

    delete qmlMcu;
    delete qmlMcuSim;
    delete qmlCru;
    delete qmlMwl;
    delete qmlExam;
    delete qmlWorkflow;
    delete qmlDevice;
    delete qmlSystem;
    delete qmlTest;
    delete qmlAlert;
    delete qmlImrServer;
    delete qmlUpgrade;
    delete qmlHardwareInfo;
    delete qmlCfgLocal;
    delete qmlCfgGlobal;
    delete qmlCfgCapabilities;
    delete qmlDevTools;

    delete env->qml.engine;
    delete env;
    delete envLocal;
}

void AppManager::slotShowUi()
{
    LOG_INFO("Reenabling touchscreen.\n");
    (void)system(QString().asprintf("%s enable", PATH_SET_TOUCHSCREEN).CSTR());

    // Give little moment to initialise the app. (i.e. S2SRUSW-3749: Screen flicker may be prevented)
    QTimer::singleShot(1000, this, [=] {

        // bind MCU FTDI USB connection
        (void)system(PATH_RECONNECT_MCU_FTDI);

        //if slotContinueLoading() is not ready yet let wait -
        // todo - is there a better way?
        while (env->state != EnvGlobal::STATE_STARTING)
        {
            LOG_WARNING("Waiting for the enviroment to be ready. State = %d\n", env->state);
            sleep(1);
        }

        env->state = EnvGlobal::STATE_RUNNING;

        env->ds.systemAction->actSetStatePath(DS_SystemDef::STATE_PATH_STARTUP_UNKNOWN);
        QMetaObject::invokeMethod(env->qml.object, "slotAppStarted");
        if (procStartVideo != NULL)
        {
            procStartVideo->deleteLater();
        }
    });
}

void AppManager::slotExit()
{
    env->ds.systemAction->actSafeExit();

    QTimer::singleShot(POWER_OFF_WAIT_TIME_MS, this, [=] {
        QCoreApplication::quit();
    });
}

void AppManager::slotScreenShot()
{
    LOG_INFO("ScreenShot requested\n");
    env->ds.systemAction->actScreenShot();
}

void AppManager::slotMultipleScreenShots()
{
    LOG_INFO("Multiple Screen Shots requested\n");
    env->ds.systemAction->actMultipleScreenShots();
}




