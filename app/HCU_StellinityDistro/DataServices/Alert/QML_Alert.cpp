#include "QML_Alert.h"
#include "DataServices/Alert/DS_AlertData.h"
#include "DataServices/Alert/DS_AlertAction.h"

QML_Alert::QML_Alert(QObject *parent, EnvGlobal *env_) :
    QObject(parent),
    env(env_)
{
    envLocal = new EnvLocal("QML_Alert", "QML_ALERT");
    qmlSrc = env->qml.object->findChild<QObject*>("dsAlert");
    env->qml.engine->rootContext()->setContextProperty("dsAlertCpp", this);

    if (qmlSrc == NULL)
    {
        LOG_ERROR("Failed to assign qmlSrc.\n");
    }

    // Register Data Callbacks
    connect(env->ds.alertData, &DS_AlertData::signalDataChanged_ActiveSystemAlerts, this, [=](const QVariantList &activeSystemAlerts) {
        qmlSrc->setProperty("activeSystemAlerts", activeSystemAlerts);
    });

    connect(env->ds.alertData, &DS_AlertData::signalDataChanged_ActiveAlerts, this, [=](const QVariantList &activeAlerts) {
        qmlSrc->setProperty("activeAlerts", activeAlerts);
    });

    connect(env->ds.alertData, &DS_AlertData::signalDataChanged_InactiveAlerts, this, [=](const QVariantList &inactiveAlerts) {
        qmlSrc->setProperty("inactiveAlerts", inactiveAlerts);
    });

    connect(env->ds.alertData, &DS_AlertData::signalDataChanged_ActiveFluidSourceAlerts, this, [=](const QVariantMap &activeFluidSourceAlerts, const QVariantMap &prevActiveFluidSourceAlerts) {
        if (activeFluidSourceAlerts["ML"] != prevActiveFluidSourceAlerts["ML"])
        {
            qmlSrc->setProperty("activeAlertsMuds", activeFluidSourceAlerts["ML"]);
        }
        if (activeFluidSourceAlerts["PL"] != prevActiveFluidSourceAlerts["PL"])
        {
            qmlSrc->setProperty("activeAlertsSuds", activeFluidSourceAlerts["PL"]);
        }
        if (activeFluidSourceAlerts["WC"] != prevActiveFluidSourceAlerts["WC"])
        {
            qmlSrc->setProperty("activeAlertsWasteContainer", activeFluidSourceAlerts["WC"]);
        }
        if (activeFluidSourceAlerts["BS0"] != prevActiveFluidSourceAlerts["BS0"])
        {
            qmlSrc->setProperty("activeAlertsBottle1", activeFluidSourceAlerts["BS0"]);
        }
        if (activeFluidSourceAlerts["BC1"] != prevActiveFluidSourceAlerts["BC1"])
        {
            qmlSrc->setProperty("activeAlertsBottle2", activeFluidSourceAlerts["BC1"]);
        }
        if (activeFluidSourceAlerts["BC2"] != prevActiveFluidSourceAlerts["BC2"])
        {
            qmlSrc->setProperty("activeAlertsBottle3", activeFluidSourceAlerts["BC2"]);
        }
        if (activeFluidSourceAlerts["RS0"] != prevActiveFluidSourceAlerts["RS0"])
        {
            qmlSrc->setProperty("activeAlertsSyringe1", activeFluidSourceAlerts["RS0"]);
        }
        if (activeFluidSourceAlerts["RC1"] != prevActiveFluidSourceAlerts["RC1"])
        {
            qmlSrc->setProperty("activeAlertsSyringe2", activeFluidSourceAlerts["RC1"]);
        }
        if (activeFluidSourceAlerts["RC2"] != prevActiveFluidSourceAlerts["RC2"])
        {
            qmlSrc->setProperty("activeAlertsSyringe3", activeFluidSourceAlerts["RC2"]);
        }
    });
}

QML_Alert::~QML_Alert()
{
    delete envLocal;
}

void QML_Alert::slotActivateAlert(QString codeName, QString data)
{
    env->ds.alertAction->activate(codeName, data);
}

void QML_Alert::slotDeactivateAlert(QString codeName, QString data)
{
    env->ds.alertAction->deactivate(codeName, data);
}

void QML_Alert::slotDeactivateAlertWithReason(QString codeName, QString newData, QString oldData, bool ignoreData)
{
    env->ds.alertAction->deactivateWithReason(codeName, newData, oldData, ignoreData);
}

QVariantMap QML_Alert::slotGetActiveAlertFromCodeName(QString codeName)
{
    QVariantMap alert = env->ds.alertAction->getActiveAlert(codeName, "", true);
    if (alert["GUID"] == EMPTY_GUID)
    {
        LOG_ERROR("slotGetActiveAlertFromCodeName(): Failed to get active alert for codename(%s)\n", codeName.CSTR());
    }
    return alert;
}

QVariantMap QML_Alert::slotGetAlertFromCodeName(QString codeName)
{
    QVariantMap alert = env->ds.alertAction->getAlert(codeName, "", true);
    if (alert["GUID"] == EMPTY_GUID)
    {
        LOG_ERROR("slotGetAlertFromCodeName(): Failed to get alert for codename(%s)\n", codeName.CSTR());
    }
    return alert;
}

QVariantList QML_Alert::slotGetMergedAlerts(QVariantList alerts)
{
    return env->ds.alertAction->getMergedAlerts(alerts);
}
