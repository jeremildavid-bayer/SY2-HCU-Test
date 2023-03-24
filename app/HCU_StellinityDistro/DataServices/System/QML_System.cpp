#include "QML_System.h"
#include "DataServices/System/DS_SystemData.h"
#include "DataServices/System/DS_SystemAction.h"
#include "DataServices/Alert/DS_AlertData.h"
#include "Common/ImrParser.h"

QML_System::QML_System(QObject *parent, EnvGlobal *env_) :
    QObject(parent),
    env(env_)
{
    envLocal = new EnvLocal("QML_System", "QML_SYSTEM");
    qmlSrc = env->qml.object->findChild<QObject*>("dsSystem");
    env->qml.engine->rootContext()->setContextProperty("dsSystemCpp", this);

    if (qmlSrc == NULL)
    {
        LOG_ERROR("Failed to assign qmlSrc.\n");
    }

    // Register Data Callbacks
    connect(env->ds.systemData, &DS_SystemData::signalDataChanged_StatePath, this, [=](DS_SystemDef::StatePath curStatePath, DS_SystemDef::StatePath lastStatePath) {
        qmlSrc->setProperty("lastStatePath", QVariant(ImrParser::ToImr_StatePath(lastStatePath)));
        qmlSrc->setProperty("statePath", QVariant(ImrParser::ToImr_StatePath(curStatePath)));
        qmlSrc->setProperty("lastStatePath", QVariant(ImrParser::ToImr_StatePath(curStatePath)));
    });

    connect(env->ds.systemData, &DS_SystemData::signalDataChanged_UsbStickInserted, this, [=](bool usbStickInserted) {
        qmlSrc->setProperty("usbStickInserted", usbStickInserted);
    });

    connect(env->ds.systemData, &DS_SystemData::signalDataChanged_Version, this, [=](QString version) {
        qmlSrc->setProperty("hcuVersion", version);
    });

    connect(env->ds.systemData, &DS_SystemData::signalDataChanged_BuildType, this, [=](QString buildType) {
        qmlSrc->setProperty("hcuBuildType", buildType);
    });

    connect(env->ds.systemData, &DS_SystemData::signalDataChanged_NetworkSettingParams, this, [=](const DS_SystemDef::NetworkSettingParams &networkSettingParams) {
        qmlSrc->setProperty("networkSettingParams", ImrParser::ToImr_NetworkSettingParams(networkSettingParams));
    });

    connect(env->ds.systemData, &DS_SystemData::signalDataChanged_HcuTemperatureParams, this, [=](const DS_SystemDef::HcuTemperatureParams &hcuTemperatureParams) {
        qmlSrc->setProperty("cpuTemperatureCelcius", hcuTemperatureParams.cpuTemperatureCelcius);
        qmlSrc->setProperty("hcuFanSpeed", hcuTemperatureParams.fanSpeed);
    });

    qmlSrc->setProperty("webAppHostPort", WEB_APPLICATION_HOST_PORT);

}

QML_System::~QML_System()
{
    delete envLocal;
}

void QML_System::slotStartupScreenExit()
{
    DS_SystemDef::StatePath statePath = env->ds.systemData->getStatePath();
    if (statePath == DS_SystemDef::STATE_PATH_STARTUP_UNKNOWN)
    {
        LOG_INFO("slotStartupScreenExit(): Startup Screen Exit OK\n");
        env->ds.systemAction->actSetStatePath(DS_SystemDef::STATE_PATH_IDLE);
    }
    else
    {
        LOG_ERROR("slotStartupScreenExit(): Bad StatePath(%s) while start up screen is exiting\n", ImrParser::ToImr_StatePath(statePath).CSTR());
    }
}

void QML_System::slotServiceModeActive(bool serviceModeActive)
{
    DS_SystemDef::StatePath statePath = env->ds.systemData->getStatePath();
    DS_SystemDef::StatePath newStatePath;

    if (serviceModeActive)
    {
        newStatePath = DS_SystemDef::STATE_PATH_BUSY_SERVICING;

        if (statePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING)
        {
            // Already in service mode
        }
        else if (statePath == DS_SystemDef::STATE_PATH_STARTUP_UNKNOWN)
        {
            env->ds.systemAction->actSetStatePath(DS_SystemDef::STATE_PATH_IDLE);
            env->ds.systemAction->actSetStatePath(newStatePath);
        }
        else if ( (statePath == DS_SystemDef::STATE_PATH_IDLE) ||
                  (statePath == DS_SystemDef::STATE_PATH_ERROR) )
        {
            env->ds.systemAction->actSetStatePath(newStatePath);
        }
        else
        {
            LOG_ERROR("SERVICE_MODE: Cannot set StatePath from %s->%s\n", ImrParser::ToImr_StatePath(statePath).CSTR(), ImrParser::ToImr_StatePath(newStatePath).CSTR());
        }
    }
    else
    {
        newStatePath = DS_SystemDef::STATE_PATH_IDLE;

        if (statePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING)
        {
            env->ds.systemAction->actSetStatePath(newStatePath);
        }
        else
        {
            LOG_ERROR("SERVICE_MODE: Should not set StatePath from %s->%s\n", ImrParser::ToImr_StatePath(statePath).CSTR(), ImrParser::ToImr_StatePath(newStatePath).CSTR());
        }
    }
}

void QML_System::slotShutdown(bool isShutdownType)
{
    QString guid = Util::newGuid();

    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            // Shutdown ok
        }
        else if (curStatus.err == _L("DISCONNECTED"))
        {
            // Mcu disconnected
        }
        else
        {

            LOG_WARNING("slotShutdown(): ACTION_STATUS: %s\n", ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());
            QMetaObject::invokeMethod(qmlSrc, "slotShutdownRestartIgnored");
        }
    });

    env->ds.systemAction->actShutdown(isShutdownType ? DS_McuDef::POWER_CONTROL_TYPE_OFF : DS_McuDef::POWER_CONTROL_TYPE_REBOOT, guid);
}

void QML_System::slotFactoryReset()
{
    QString guid = Util::newGuid();

    env->actionMgr->onActionStarted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            env->actionMgr->onActionCompleted(guid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
                if (curStatus.state == DS_ACTION_STATE_COMPLETED)
                {
                    env->ds.systemAction->actShutdown(DS_McuDef::POWER_CONTROL_TYPE_REBOOT);
                }
            });
        }
    });

    env->ds.systemAction->actFactoryReset(guid);
}

void QML_System::slotSaveUserData()
{
    env->ds.systemAction->actSaveUserData();
}

void QML_System::slotScreenWakeup()
{
    env->ds.systemAction->actScreenWakeup();
}

void QML_System::slotSetScreenSleepTime(int sleepTimeMinutes)
{
    env->ds.systemAction->actSetScreenSleepTime(sleepTimeMinutes);
}

void QML_System::slotSetScreenSleepTimeToDefault()
{
    env->ds.systemAction->actSetScreenSleepTimeToDefault();
}


