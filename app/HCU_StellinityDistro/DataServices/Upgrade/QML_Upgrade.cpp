#include "QML_Upgrade.h"
#include "Common/ImrParser.h"
#include "DataServices/Upgrade/DS_UpgradeData.h"
#include "DataServices/Upgrade/DS_UpgradeAction.h"

QML_Upgrade::QML_Upgrade(QObject *parent, EnvGlobal *env_) :
    QObject(parent),
    env(env_)
{
    envLocal = new EnvLocal("QML_Upgrade", "QML_UPGRADE");
    qmlSrc = env->qml.object->findChild<QObject*>("dsUpgrade");
    env->qml.engine->rootContext()->setContextProperty("dsUpgradeCpp", this);

    if (qmlSrc == NULL)
    {
        LOG_ERROR("Failed to assign qmlSrc.\n");
    }

    // Register Data Callbacks
    connect(env->ds.upgradeData, &DS_UpgradeData::signalDataChanged_UpgradeDigest, this, [=](const DS_UpgradeDef::UpgradeDigest &upgradeDigest){
        qmlSrc->setProperty("upgradeDigest", ImrParser::ToImr_UpgradeDigest(upgradeDigest));
    });
}

QML_Upgrade::~QML_Upgrade()
{
    delete envLocal;
}

void QML_Upgrade::slotUpdateSelectedPackageInfo(QString pathPackage)
{
    pathPackage.replace("file://", "");
    env->ds.upgradeAction->actUpdateSelectedPackageInfo(pathPackage);
}

void QML_Upgrade::slotUpgrade()
{
    env->ds.upgradeAction->actUpgrade();
}
