#include "QML_HardwareInfo.h"
#include "Common/Util.h"
#include "DataServices/HardwareInfo/DS_HardwareInfo.h"

QML_HardwareInfo::QML_HardwareInfo(QObject *parent, EnvGlobal *env_) :
    QObject(parent),
    env(env_)
{
    envLocal = new EnvLocal("QML_HardwareInfo", "QML_HARDWARE_INFO");
    qmlSrc = env->qml.object->findChild<QObject*>("dsHardwareInfo");
    env->qml.engine->rootContext()->setContextProperty("dsHardwareInfoCpp", this);

    if (qmlSrc == NULL)
    {
        LOG_ERROR("Failed to assign qmlSrc.\n");
    }

    // Register Data Callbacks
    connect(env->ds.hardwareInfo, &DS_HardwareInfo::signalConfigChanged_Hidden_CalibrationInfo, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("calibrationInfo", cfg.value);
    });

    connect(env->ds.hardwareInfo, &DS_HardwareInfo::signalConfigChanged_General_SerialNumber, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("serialNumber", cfg.value);
    });

    connect(env->ds.hardwareInfo, &DS_HardwareInfo::signalConfigChanged_General_ModelNumber, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("modelNumber", cfg.value);
    });

    connect(env->ds.hardwareInfo, &DS_HardwareInfo::signalConfigChanged_General_BaseBoardType, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("baseBoardType", cfg.value);
    });

    connect(env->ds.hardwareInfo, &DS_HardwareInfo::signalConfigChanged, this, [=](){
        QString err;
        QVariantMap map = env->ds.hardwareInfo->getConfigs(&err);

        if (err == "")
        {
            QVariantList list = Util::configMapToSortedList(map, &err);
            qmlSrc->setProperty("configTable", list);
        }

        if (err != "")
        {
            LOG_ERROR("Failed to get hardware info list (err=%s)\n", err.CSTR());
        }
    });
}

QML_HardwareInfo::~QML_HardwareInfo()
{
    delete envLocal;
}

void QML_HardwareInfo::slotConfigChanged(QVariant configItem)
{
    QString err;
    QVariantMap configItemMap = configItem.toMap();
    QVariantMap map = env->ds.hardwareInfo->getConfigs(&err);

    if (err == "")
    {
        map.insert(configItemMap["KeyName"].toString(), configItemMap);
        env->ds.hardwareInfo->setConfigs(map, true, &err);
    }

    if (err != "")
    {
        LOG_ERROR("Failed to set hardware info (err=%s)\n", err.CSTR());
    }
}
