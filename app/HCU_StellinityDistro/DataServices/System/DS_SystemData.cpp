#include "Apps/AppManager.h"
#include "DS_SystemData.h"

DS_SystemData::DS_SystemData(QObject *parent, EnvGlobal *env_) :
    QObject(parent),
    env(env_)
{
    connect(env->appManager, &AppManager::signalAppStarted, this, [=]() {
        slotAppStarted();
    });

    m_DataLocked = false;
    m_DigestLocked = false;
    m_DiskMainFreeSpaceMB = 0;
    m_DiskUserFreeSpaceMB = 0;

    // Read version & buildDate
    QFile fileVer(PATH_VERSION_INFO);
    if (fileVer.open(QFile::ReadOnly | QFile::Text))
    {
        QTextStream streamIn(&fileVer);
        while (!streamIn.atEnd())
        {
            QString lineTxt = streamIn.readLine();
            lineTxt = lineTxt.replace("\r", "").replace("\n", "");

            if (lineTxt.indexOf("<VERSION>") >= 0)
            {
                setVersion(lineTxt.replace("<VERSION>", "").trimmed());
            }
            else if (lineTxt.indexOf("<BUILD_DATE>") >= 0)
            {
                setBuildDate(lineTxt.replace("<BUILD_DATE>", "").trimmed());
            }
            else if (lineTxt.indexOf("<BUILD_TYPE>") >= 0)
            {
                setBuildType(lineTxt.replace("<BUILD_TYPE>", "").trimmed());
                setenv("BUILD_TYPE", getBuildType().CSTR(), 1);
            }
            else if (lineTxt.indexOf("<MCU_VERSION_PREFIX>") >= 0)
            {
                setCompatibleMcuVersionPrefix(lineTxt.replace("<MCU_VERSION_PREFIX>", "").trimmed());
            }
            else if (lineTxt.indexOf("<MCU_COMMAND_VERSION>") >= 0)
            {
                setCompatibleMcuCommandVersion(lineTxt.replace("<MCU_COMMAND_VERSION>", "").trimmed());
            }
        }
        fileVer.close();
        setUptime(QDateTime::currentDateTimeUtc());
    }

    //Init Values
    m_UsbStickInserted = false;
    m_StatePath = DS_SystemDef::STATE_PATH_OFF_UNREACHABLE;

    SET_LAST_DATA(DataLocked)
    SET_LAST_DATA(DigestLocked)
    SET_LAST_DATA(Version)
    SET_LAST_DATA(BuildDate)
    SET_LAST_DATA(BuildType)
    SET_LAST_DATA(StatePath)
    SET_LAST_DATA(DiskMainFreeSpaceMB)
    SET_LAST_DATA(DiskUserFreeSpaceMB)
    SET_LAST_DATA(Uptime)
    SET_LAST_DATA(UsbStickInserted)
    SET_LAST_DATA(HcuTemperatureParams)
    SET_LAST_DATA(CompatibleMcuVersionPrefix)
    SET_LAST_DATA(CompatibleMcuCommandVersion)
    SET_LAST_DATA(NetworkSettingParams)
    SET_LAST_DATA(IwconfigParams)
}

DS_SystemData::~DS_SystemData()
{
}

void DS_SystemData::slotAppStarted()
{
    EMIT_DATA_CHANGED_SIGNAL(DataLocked)
    EMIT_DATA_CHANGED_SIGNAL(DigestLocked)
    EMIT_DATA_CHANGED_SIGNAL(Version)
    EMIT_DATA_CHANGED_SIGNAL(BuildDate)
    EMIT_DATA_CHANGED_SIGNAL(BuildType)
    //EMIT_DATA_CHANGED_SIGNAL(StatePath)
    EMIT_DATA_CHANGED_SIGNAL(Uptime)
    //EMIT_DATA_CHANGED_SIGNAL(UsbStickInserted)
    EMIT_DATA_CHANGED_SIGNAL(CompatibleMcuVersionPrefix)
    EMIT_DATA_CHANGED_SIGNAL(CompatibleMcuCommandVersion)
    EMIT_DATA_CHANGED_SIGNAL(NetworkSettingParams)
    EMIT_DATA_CHANGED_SIGNAL(IwconfigParams)
}
