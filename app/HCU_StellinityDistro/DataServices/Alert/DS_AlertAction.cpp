#include "Apps/AppManager.h"
#include "DS_AlertAction.h"
#include "Common/ImrParser.h"
#include "Common/Translator.h"
#include "DataServices/Alert/DS_AlertData.h"
#include "DataServices/Capabilities/DS_Capabilities.h"
#include "DataServices/System/DS_SystemData.h"

DS_AlertAction::DS_AlertAction(QObject *parent, EnvGlobal *env_) :
    QObject(parent),
    env(env_)
{
    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });

    envLocal = new EnvLocal("DS_Alert-Action", "ALERT_ACTION", LOG_LRG_SIZE_BYTES * 2);
    monitor = new AlertMonitor(this, env);

    loadAlertDescriptions();

    // Prepare alertBufferOverflowOccurred
    alertBufferOverflowOccurred = prepareAlert(_L("AlertBufferOverflowOccurred"));
    alertBufferOverflowOccurred.insert(_L("Reporter"), "HCU");
    alertBufferOverflowOccurred.insert(_L("Data"), "");
    alertBufferOverflowOccurred.insert(_L("Status"), "Inactive");
    if (alertBufferOverflowOccurred[_L("CodeName")].toString() == _L(""))
    {
        LOG_ERROR("Failed to prepare AlertBufferOverflowOccurred alert\n");
    }
    restoreLastSavedAlerts();
}

DS_AlertAction::~DS_AlertAction()
{
    delete monitor;
    delete envLocal;
}

void DS_AlertAction::slotAppInitialised()
{
    connect(env->ds.systemData, &DS_SystemData::signalDataChanged_StatePath, this, [=](DS_SystemDef::StatePath curStatePath, DS_SystemDef::StatePath prevStatePath) {
        if ( ( (prevStatePath == DS_SystemDef::STATE_PATH_IDLE) && (curStatePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING) ) ||
             ( (prevStatePath == DS_SystemDef::STATE_PATH_BUSY_SERVICING) && (curStatePath == DS_SystemDef::STATE_PATH_IDLE) ) )
        {
            LOG_INFO("\n\n-----------------\nStatePath=%s\n\n", ImrParser::ToImr_StatePath(curStatePath).CSTR());
        }
    });

    if (env->humanUseAllowed)
    {
        if (env->ds.alertAction->isActivated("NotForHumanUseSRU"))
        {
            deactivate("NotForHumanUseSRU");
        }
    }
    else
    {
        if (!env->ds.alertAction->isActivated("NotForHumanUseSRU"))
        {
            activate("NotForHumanUseSRU");
        }
    }
}

void DS_AlertAction::insertAlertItem(QVariantList &target, int index, QVariantMap newItem)
{
    if (index > 0)
    {
        QVariantMap prevItem = target[index - 1].toMap();
        prevItem[_L("NextAlertGuid")] = newItem[_L("GUID")];
        target[index - 1] = prevItem;
    }

    if (index >= target.length())
    {
        newItem[_L("NextAlertGuid")] = EMPTY_GUID;
        target.append(newItem);
    }
    else
    {
        QVariantMap nextItem = target[index].toMap();
        newItem[_L("NextAlertGuid")] = nextItem[_L("GUID")];
        target.insert(index, newItem);
    }
}

void DS_AlertAction::insertAlertItem(QVariantList &target, int targetStartIdx, int targetEndIdx, QVariantMap newAlert)
{
    if (target.length() == 0)
    {
        // Empty target. Append to last.
        insertAlertItem(target, 0, newAlert);
        return;
    }

    qint64 newAlertActiveAt = Util::utcDateTimeStrToQDateTime(newAlert[_L("ActiveAt")].toString()).toMSecsSinceEpoch();
    QVariantMap firstAlert = target[targetStartIdx].toMap();
    qint64 firstAlertActiveAt = Util::utcDateTimeStrToQDateTime(firstAlert[_L("ActiveAt")].toString()).toMSecsSinceEpoch();

    if (newAlertActiveAt < firstAlertActiveAt)
    {
        // Alert is the oldest (add it to the start of given area)
        insertAlertItem(target, targetStartIdx, newAlert);
        return;
    }

    QVariantMap lastAlert = target[targetEndIdx].toMap();
    qint64 lastAlertActiveAt = Util::utcDateTimeStrToQDateTime(lastAlert[_L("ActiveAt")].toString()).toMSecsSinceEpoch();

    if (newAlertActiveAt >= lastAlertActiveAt)
    {
        // Alert is the newest (add it to outside given area)
        insertAlertItem(target, targetEndIdx + 1, newAlert);
        return;
    }

    if (targetEndIdx - targetStartIdx <= 1)
    {
        // Add alert between First and Last
        insertAlertItem(target, targetEndIdx, newAlert);
        return;
    }

    // Insert alert to the target list using Binary Search Algorithm
    int insertPt = (targetStartIdx + targetEndIdx) / 2;
    QVariantMap insertPtAlert = target[insertPt].toMap();
    qint64 insertPtAlertActiveAt = Util::utcDateTimeStrToQDateTime(insertPtAlert[_L("ActiveAt")].toString()).toMSecsSinceEpoch();

    if (newAlertActiveAt > insertPtAlertActiveAt)
    {
        // Keep look for insert point (after middle point)
        insertAlertItem(target, insertPt, targetEndIdx, newAlert);
    }
    else
    {
        // Keep look for insert point (before middle point)
        insertAlertItem(target, targetStartIdx, insertPt, newAlert);
    }
}

void DS_AlertAction::removeAlertItem(QVariantList &target, int index)
{
    if (index > 0)
    {
        // Update previous item
        QVariantMap prevItem = target[index - 1].toMap();

        if (index >= target.length() - 1)
        {
            prevItem[_L("NextAlertGuid")] = EMPTY_GUID;
        }
        else
        {
            QVariantMap nextItem = target[index + 1].toMap();
            prevItem[_L("NextAlertGuid")] = nextItem[_L("GUID")];
        }
        target[index - 1] = prevItem;
    }
    target.removeAt(index);
}

void DS_AlertAction::updateAllAlerts()
{
    QVariantList activeAlerts = env->ds.alertData->getActiveAlerts();
    QVariantList inactiveAlerts = env->ds.alertData->getInactiveAlerts();
    QVariantList allAlerts;

    int bufferLimit = env->ds.capabilities->get_Alert_AlertBufferLimit();
    bool bufferOverflowOccurred = (alertBufferOverflowOccurred[_L("Status")].toString() == _L("Active"));
    int alertsToBeTrimmed = activeAlerts.length() + inactiveAlerts.length() - bufferLimit;

    // Handle alertBufferOverflowOccurred
    if (alertsToBeTrimmed > 0)
    {
        if (!bufferOverflowOccurred)
        {
            // Activate alertBufferOverflowOccurred
            alertBufferOverflowOccurred[_L("GUID")] = Util::newGuid();
            alertBufferOverflowOccurred[_L("Status")] = "Active";
            alertBufferOverflowOccurred[_L("ActiveAt")] = Util::qDateTimeToUtcDateTimeStr(QDateTime::currentDateTimeUtc());
            alertBufferOverflowOccurred[_L("InactiveAt")] = QVariant();
            activeAlerts.insert(0, alertBufferOverflowOccurred);
            env->ds.alertData->setActiveAlerts(activeAlerts);
            LOG_WARNING("AlertBufferOverflow occurred (limit=%d, alertsToBeTrimmed=%d)\n", bufferLimit, alertsToBeTrimmed);
        }

        // Trim inactive alerts
        for (int i = 0; i < alertsToBeTrimmed; i++)
        {
            removeAlertItem(inactiveAlerts, 0);
        }
        env->ds.alertData->setInactiveAlerts(inactiveAlerts);
    }
    else
    {
        if (bufferOverflowOccurred)
        {
            // Deactivate alertBufferOverflowOccurred
            alertBufferOverflowOccurred[_L("Status")] = "Inactive";
            alertBufferOverflowOccurred[_L("InactiveAt")] = Util::qDateTimeToUtcDateTimeStr(QDateTime::currentDateTimeUtc());
            insertAlertItem(inactiveAlerts, 0, alertBufferOverflowOccurred);
            activeAlerts.removeFirst();
            env->ds.alertData->setInactiveAlerts(inactiveAlerts);
            env->ds.alertData->setActiveAlerts(activeAlerts);
            LOG_INFO("AlertBufferOverflow deactivated\n");
        }
    }

    // Merge inactiveAlerts and activeAlerts
    //LOG_DEBUG("Merging activeAlerts[%d] and inactiveAlerts[%d]\n", activeAlerts.length(), inactiveAlerts.length());
    //LOG_DEBUG("activeAlerts=%s, alertBufferOverflowOccurred=%s\n", Util::qVarientToJsonData(activeAlerts).CSTR(), Util::qVarientToJsonData(alertBufferOverflowOccurred).CSTR());
    allAlerts = inactiveAlerts;

    for (int activeAlertIdx = activeAlerts.length() - 1; activeAlertIdx >= 0; activeAlertIdx--)
    {
        QVariantMap activeAlert = activeAlerts[activeAlertIdx].toMap();

        //LOG_DEBUG("ActiveAlert[%d]: Codename=%s, guid=%s\n", activeAlertIdx, activeAlert[_L("CodeName")].toString().CSTR(), activeAlert[_L("GUID")].toString().CSTR());

        if (activeAlert[_L("GUID")] == alertBufferOverflowOccurred[_L("GUID")])
        {
            //LOG_DEBUG("Insert BufferOveflow alert to first\n");
            insertAlertItem(allAlerts, 0, activeAlert);
            continue;
        }
        insertAlertItem(allAlerts, 0, allAlerts.length() - 1, activeAlert);
    }

    env->ds.alertData->setAllAlerts(allAlerts);
}

void DS_AlertAction::loadAlertDescriptions()
{
    QFile fileBuf(QString(PATH_RESOURCES_DEFAULT_CONFIG) + "/AlertDescriptions.json");
    if (fileBuf.open(QFile::ReadOnly | QFile::Text))
    {
        QJsonParseError parseErr;
        QString jsonStr = fileBuf.readAll();
        fileBuf.close();

        QJsonDocument document = QJsonDocument::fromJson(jsonStr.toUtf8(), &parseErr);

        if (parseErr.error == QJsonParseError::NoError)
        {
            alertDescriptions = document.toVariant().toMap();
        }
        else
        {
            LOG_ERROR("Failed to load Alert Table file. ParseErr=%s\n", parseErr.errorString().CSTR());
        }
    }
    else
    {
        LOG_ERROR("Failed to open %s. Err=%s\n", fileBuf.fileName().CSTR(), fileBuf.errorString().CSTR());
    }
}

bool DS_AlertAction::isActivated(QVariantList activeAlerts, QString codeName, QString data, bool ignoreData)
{
    for (int alertIdx = 0; alertIdx < activeAlerts.length(); alertIdx++)
    {
        QVariantMap alert = activeAlerts[alertIdx].toMap();
        if (alert[_L("CodeName")].toString() == codeName)
        {
            if ( (ignoreData) ||
                 (alert[_L("Data")].toString() == data) )
            {
                return true;
            }
        }
    }
    return false;
}

bool DS_AlertAction::isActivated(QString codeName, QString data, bool ignoreData)
{
    QVariantList activeAlerts = env->ds.alertData->getActiveAlerts();
    return isActivated(activeAlerts, codeName, data, ignoreData);
}

bool DS_AlertAction::isActivated(QVariantMap alert)
{
    return isActivated(alert[_L("CodeName")].toString(), alert[_L("Data")].toString());
}

bool DS_AlertAction::isActivatedWithSyringeIdx(QString codeName, SyringeIdx syringeIdx)
{
    QString syringeIdxStr = ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdx);
    QVariantList activeAlerts = env->ds.alertData->getActiveAlerts();
    for (int alertIdx = 0; alertIdx < activeAlerts.length(); alertIdx++)
    {
        QString alertCodeName = activeAlerts[alertIdx].toMap()[_L("CodeName")].toString();
        QString alertData = activeAlerts[alertIdx].toMap()[_L("Data")].toString();
        if ( (alertCodeName == codeName) &&
             (alertData.contains(syringeIdxStr)) )
        {
            return true;
        }
    }
    return false;
}

bool DS_AlertAction::isLastOccurredWithSyringeIdx(QString codeName, SyringeIdx syringeIdx)
{
    QString syringeIdxStr = ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdx);

    // Occurred alerts are in inactive list
    QVariantList inActiveAlerts = env->ds.alertData->getInactiveAlerts();

    for (int alertIdx = inActiveAlerts.length() - 1; alertIdx >= 0; alertIdx--)
    {
        QString alertCodeName = inActiveAlerts[alertIdx].toMap()[_L("CodeName")].toString();
        QString alertData = inActiveAlerts[alertIdx].toMap()[_L("Data")].toString();
        if ( (alertCodeName == codeName) &&
             (alertData.contains(syringeIdxStr)) )
        {
            return true;
        }
    }

    return false;
}

void DS_AlertAction::saveLastAlerts()
{
    QFile fileBuf(PATH_LAST_ALERTS);
    if (fileBuf.open(QFile::WriteOnly | QFile::Text))
    {
        QVariantList allAlerts = env->ds.alertData->getAllAlerts();
        fileBuf.write(Util::qVarientToJsonData(allAlerts, env->humanUseAllowed));
        fileBuf.close();
    }

    QFile fileBuf2(PATH_LAST_ACTIVE_ALERTS);
    if (fileBuf2.open(QFile::WriteOnly | QFile::Text))
    {
        QVariantList activeAlerts = env->ds.alertData->getActiveAlerts();
        fileBuf2.write(Util::qVarientToJsonData(activeAlerts, env->humanUseAllowed));
        fileBuf2.close();
    }
}

void DS_AlertAction::restoreLastSavedAlerts()
{
    if (env->ds.systemData->getBuildType() == _L(BUILD_TYPE_PROD))
    {
        return;
    }

    QFile fileBuf(PATH_LAST_ALERTS);
    if (!fileBuf.open(QFile::ReadOnly | QFile::Text))
    {
        LOG_ERROR("Failed to open last alerts (path=%s)\n", PATH_LAST_ALERTS);
        return;
    }

    QJsonParseError parseErr;
    QString jsonStr = fileBuf.readAll();
    fileBuf.close();

    QJsonDocument document = QJsonDocument::fromJson(jsonStr.toUtf8(), &parseErr);

    if (parseErr.error != QJsonParseError::NoError)
    {
        LOG_ERROR("Failed to parse last alerts. ParseErr=%s\n", parseErr.errorString().CSTR());
        return;
    }

    QVariantList lastAlerts = document.toVariant().toList();

    LOG_INFO("Restoring last saved alerts[%d]\n", (int)lastAlerts.length());
    QVariantList activeAlerts, inactiveAlerts;

    for (int alertIdx = 0; alertIdx < lastAlerts.length(); alertIdx++)
    {
        //LOG_DEBUG("Restoring alert[%d]..\n", alertIdx);

        QVariantMap alert = lastAlerts[alertIdx].toMap();

        // update fields.
        populateAlertFields(alert);

        bool isActive = true;
        QString status = alert[_L("Status")].toString();
        QString severity = alert[_L("Severity")].toString();
        QString clearOnStartup = alert[_L("ClearOnStartup")].toString();

        if ( ((severity == _L("Fatal")) && (status == _L("Active")))
             || (clearOnStartup == _L("true")))
        {
            // Fatal alert shall be activated again if occurred
            // alerts marked clear on startup will be de-activated
            isActive = false;
        }
        else
        {
            isActive = (status == _L("Active"));
        }

        if (isActive)
        {
            activeAlerts.append(alert);
        }
        else
        {
            if (status == _L("Active"))
            {
                // Deactivate active alert
                alert[_L("InactiveAt")] = Util::qDateTimeToUtcDateTimeStr(QDateTime::currentDateTimeUtc());
                alert[_L("Status")] = "Inactive";
            }
            insertAlertItem(inactiveAlerts, 0, inactiveAlerts.length() - 1, alert);
        }
    }

    LOG_INFO("Active Alerts restored:\n%s\n", Util::qVarientToJsonData(activeAlerts, false).CSTR());
    env->ds.alertData->setActiveAlerts(activeAlerts);

    LOG_INFO("Inactive Alerts restored:\n%s\n", Util::qVarientToJsonData(inactiveAlerts, false).CSTR());
    env->ds.alertData->setInactiveAlerts(inactiveAlerts);

    updateAllAlerts();
}

QVariantMap DS_AlertAction::prepareAlert(QString codeName, QString data)
{
    QVariantMap alert;
    alert.insert("GUID", EMPTY_GUID);
    alert.insert("NextAlertGuid", EMPTY_GUID);
    alert.insert("CodeName", "");
    alert.insert("Data", "");
    alert.insert("ActiveAt", QVariant());
    alert.insert("InactiveAt", QVariant());
    alert.insert("Status", "Inactive");
    alert.insert("Reporter", "HCU");
    alert.insert("Severity", "Unknown");
    alert.insert("Lifecycle", "Unknown");
    alert.insert("SystemAlert", "Unknown");
    alert.insert("ClearOnStartup", "UnKnown");

    QVariantMap alertDescription = alertDescriptions[codeName].toMap();
    if (alertDescription.count() != 0)
    {
        alert = alertDescription;
        alert[_L("CodeName")] = codeName;
        alert[_L("GUID")] = Util::newGuid();
        alert[_L("NextAlertGuid")] = EMPTY_GUID;
        alert[_L("Data")] = data;
        alert[_L("ActiveAt")] = Util::qDateTimeToUtcDateTimeStr(QDateTime::currentDateTimeUtc());
        alert[_L("InactiveAt")] = QVariant();
        alert[_L("Status")] = "Active";
    }

    return alert;
}

QString DS_AlertAction::getActiveAlertGuid(QString codeName, QString data, bool ignoreData)
{
    // Find active alert guid from latest
    QVariantList activeAlerts = env->ds.alertData->getActiveAlerts();
    int alertIdxMax = activeAlerts.length() - 1;
    for (int alertIdx = alertIdxMax; alertIdx >= 0; alertIdx--)
    {
        QVariantMap alert = activeAlerts[alertIdx].toMap();
        if (alert[_L("CodeName")].toString() == codeName)
        {
            if ( (ignoreData) ||
                 (alert[_L("Data")].toString() == data) )
            {
                return alert[_L("GUID")].toString();
            }
        }
    }
    return EMPTY_GUID;
}

QString DS_AlertAction::getAlertGuid(QString codeName, QString data, bool ignoreData)
{
    // Find alert guid from latest
    QVariantList allAlerts = env->ds.alertData->getAllAlerts();
    int alertIdxMax = allAlerts.length() - 1;
    for (int alertIdx = alertIdxMax; alertIdx >= 0; alertIdx--)
    {
        QVariantMap alert = allAlerts[alertIdx].toMap();
        if (alert[_L("CodeName")].toString() == codeName)
        {
            if ( (ignoreData) ||
                 (alert[_L("Data")].toString() == data) )
            {
                return alert[_L("GUID")].toString();
            }
        }
    }
    return EMPTY_GUID;
}

QVariantMap DS_AlertAction::getActiveAlert(QString codeName, QString data, bool ignoreData)
{
    return getFromGuid(getActiveAlertGuid(codeName, data, ignoreData));
}

QVariantMap DS_AlertAction::getAlert(QString codeName, QString data, bool ignoreData)
{
    return getFromGuid(getAlertGuid(codeName, data, ignoreData));
}

void DS_AlertAction::activate(QVariantMap alert)
{
    if (isActivated(alert))
    {
        LOG_INFO("Failed to activate alert: Is already activated (guid=%s, codeName=%s, data=%s)\n",
                 alert[_L("GUID")].toString().CSTR(),
                 alert[_L("CodeName")].toString().CSTR(),
                 alert[_L("Data")].toString().CSTR());
        return;
    }

    if (alert[_L("Lifecycle")].toString() == _L("Occurred"))
    {
        if (alert[_L("InactiveAt")].isNull())
        {
            alert[_L("InactiveAt")] = Util::qDateTimeToUtcDateTimeStr(QDateTime::currentDateTimeUtc());
        }

        // Lifecycle is Occurred type. Set Inactive state right after activated.
        alert[_L("Status")] = "Inactive";
    }

    addAlert(alert);
}

void DS_AlertAction::activate(QString codeName, QString data, QString reporter)
{
    QVariantMap alert = prepareAlert(codeName);

    if (alert[_L("CodeName")].toString() == _L(""))
    {
        LOG_ERROR("Failed to prepare alert with CodeName(%s)\n", codeName.CSTR());
        return;
    }

    alert[_L("Reporter")] = reporter;
    alert[_L("Data")] = data;
    alert[_L("Status")] = "Active";
    alert[_L("ActiveAt")] = Util::qDateTimeToUtcDateTimeStr(QDateTime::currentDateTimeUtc());

    activate(alert);
}

void DS_AlertAction::addAlert(QVariantMap alert)
{
    const QVariantList prevActiveAlerts = env->ds.alertData->getActiveAlerts();
    QVariantList newActiveAlerts = env->ds.alertData->getActiveAlerts();
    const QVariantList prevInactiveAlerts = env->ds.alertData->getInactiveAlerts();
    QVariantList newInactiveAlerts = env->ds.alertData->getInactiveAlerts();

    // lock it so all alert related signals can be sent after everything is updated
    env->ds.alertData->setDataLocked(true);

    QString alertReporter = alert[_L("Reporter")].toString();

    if ( (alertReporter != _L("MCU")) &&
         (alertReporter != _L("CRU")) )
    {
        alert[_L("Reporter")] = "HCU";
    }

    // if alert comes from CRU, it doesn't call prepare alert.
    // HCU maintained alert fields from alertDescriptions will be populated here
    populateAlertFields(alert);

    QString alertStatus = alert[_L("Status")].toString();
    if (alertStatus == _L("Active"))
    {
       newActiveAlerts.append(alert);
       env->ds.alertData->setActiveAlerts(newActiveAlerts);
    }
    else
    {
        if (alert[_L("Lifecycle")].toString() == _L("Occurred"))
        {
            // Alert is 'occurred' type (activatedAt time is already latest). There is no need to insert with sorting, insert to last.
            insertAlertItem(newInactiveAlerts, newInactiveAlerts.length(), alert);
        }
        else
        {
            // Insert alert with sorting
            insertAlertItem(newInactiveAlerts, 0, newInactiveAlerts.length() - 1, alert);
        }
        env->ds.alertData->setInactiveAlerts(newInactiveAlerts);
    }

    QString alertData = alert[_L("Data")].toString();

    QString buildType = env->ds.systemData->getBuildType();
    bool anonymizeOn = (buildType == BUILD_TYPE_VNV || buildType == BUILD_TYPE_REL);

    bool alertHasPrivateData = alert[_L("CodeName")].toString() == "ExamFinalized";
    if (anonymizeOn && alertHasPrivateData)
    {
        // Anonymize alert data
        alertData = "";
    }

    LOG_INFO("Alert[%d] Added (reporter=%s, guid=%s, codeName=%s, status=%s, activeAt=%s, inactiveAt=%s, data=%s)\n",
             (int)env->ds.alertData->getAllAlerts().length(),
             alert[_L("Reporter")].toString().CSTR(),
             alert[_L("GUID")].toString().CSTR(),
             alert[_L("CodeName")].toString().CSTR(),
             alertStatus.CSTR(),
             alert[_L("ActiveAt")].toString().CSTR(),
             alert[_L("InactiveAt")].toString().CSTR(),
             alertData.CSTR());

    // unlock now so updateAllAlerts works
    env->ds.alertData->setDataLocked(false);

    updateAllAlerts();

    if (newActiveAlerts != prevActiveAlerts)
    {
        emit env->ds.alertData->signalDataChanged_ActiveAlerts(newActiveAlerts, prevActiveAlerts);
    }

    if (newInactiveAlerts != prevInactiveAlerts)
    {
        emit env->ds.alertData->signalDataChanged_InactiveAlerts(newInactiveAlerts, prevInactiveAlerts);
    }
}

void DS_AlertAction::setActiveAlertData(QString guid, QString data)
{
    if (guid == EMPTY_GUID)
    {
        return;
    }

    QVariantList activeAlerts = env->ds.alertData->getActiveAlerts();
    for (int alertIdx = 0; alertIdx < activeAlerts.length(); alertIdx++)
    {
        QVariantMap alert = activeAlerts[alertIdx].toMap();
        QString alertGuid = alert[_L("GUID")].toString();
        if (alertGuid == guid)
        {
            alert[_L("Data")] = data;
            activeAlerts[alertIdx] = alert;
            env->ds.alertData->setActiveAlerts(activeAlerts);
            updateAllAlerts();
            return;
        }
    }
    LOG_WARNING("Failed to set data for alert(guid=%s, data=%s), GUID not found\n", guid.CSTR(), data.CSTR());
}

void DS_AlertAction::update(QString codeName, QString data, QString status)
{
    QVariantList activeAlerts = env->ds.alertData->getActiveAlerts();
    for (int alertIdx = activeAlerts.length() - 1; alertIdx >= 0; alertIdx--)
    {
        QVariantMap alert = activeAlerts[alertIdx].toMap();
        QString alertCodeName = alert[_L("CodeName")].toString();
        QString alertData = alert[_L("Data")].toString();
        if (alertCodeName == codeName)
        {
            if ( (data == "") &&
                 (alertData != "") )
            {
                if (status == "Inactive")
                {
                    LOG_INFO("Alert Deactivated with no input data. (index=%d, reporter=%s, guid=%s, codeName=%s, status=%s, activeAt=%s, data=%s, newStatus=%s)\n",
                                alertIdx,
                                alert[_L("Reporter")].toString().CSTR(),
                                alert[_L("GUID")].toString().CSTR(),
                                alertCodeName.CSTR(),
                                alert[_L("Status")].toString().CSTR(),
                                alert[_L("ActiveAt")].toString().CSTR(),
                                alertData.CSTR(),
                                status.CSTR());
                }
                else
                {
                    LOG_WARNING("Alert Updated with no input data. (index=%d, reporter=%s, guid=%s, codeName=%s, status=%s, activeAt=%s, data=%s, newStatus=%s)\n",
                                alertIdx,
                                alert[_L("Reporter")].toString().CSTR(),
                                alert[_L("GUID")].toString().CSTR(),
                                alertCodeName.CSTR(),
                                alert[_L("Status")].toString().CSTR(),
                                alert[_L("ActiveAt")].toString().CSTR(),
                                alertData.CSTR(),
                                status.CSTR());
                }

                update(alertIdx, status);

                // Keep go through the loop as there may be more alert with same code
            }
            else if (alertData == data)
            {
                if (status == "Inactive")
                {
                    LOG_INFO("Alert Deactivated. (index=%d, reporter=%s, guid=%s, codeName=%s, status=%s, activeAt=%s, data=%s, newStatus=%s)\n",
                                alertIdx,
                                alert[_L("Reporter")].toString().CSTR(),
                                alert[_L("GUID")].toString().CSTR(),
                                alertCodeName.CSTR(),
                                alert[_L("Status")].toString().CSTR(),
                                alert[_L("ActiveAt")].toString().CSTR(),
                                alertData.CSTR(),
                                status.CSTR());
                }
                else
                {
                    LOG_WARNING("Alert Updated. (index=%d, reporter=%s, guid=%s, codeName=%s, status=%s, activeAt=%s, data=%s, newStatus=%s)\n",
                                alertIdx,
                                alert[_L("Reporter")].toString().CSTR(),
                                alert[_L("GUID")].toString().CSTR(),
                                alertCodeName.CSTR(),
                                alert[_L("Status")].toString().CSTR(),
                                alert[_L("ActiveAt")].toString().CSTR(),
                                alertData.CSTR(),
                                status.CSTR());
                }
                update(alertIdx, status);
                return;
            }
        }
    }

    //LOG_ERROR("Failed to update alert: Cannot find alert (codeName=%s, data=%s, newStatus=%s)\n", codeName.CSTR(), data.CSTR(), status.CSTR());
}

void DS_AlertAction::update(QString guid, QString status)
{
    QVariantList activeAlerts = env->ds.alertData->getActiveAlerts();
    for (int alertIdx = activeAlerts.length() - 1; alertIdx >= 0; alertIdx--)
    {
        QString alertGuid = activeAlerts[alertIdx].toMap()[_L("GUID")].toString();
        if (alertGuid == guid)
        {
            update(alertIdx, status);
            return;
        }
    }

    //LOG_ERROR("Failed to update alert: Cannot find alert (GUID=%s, newStatus=%s)\n", guid.CSTR(), status.CSTR());
}

void DS_AlertAction::update(int activeAlertIdx, QString status)
{
    QVariantList activeAlerts = env->ds.alertData->getActiveAlerts();
    QVariantMap alert = activeAlerts[activeAlertIdx].toMap();

    if ( (status == _L("Inactive")) &&
         (alert[_L("Status")].toString() == _L("Active")) )
    {
        alert[_L("InactiveAt")] = Util::qDateTimeToUtcDateTimeStr(QDateTime::currentDateTimeUtc());
        alert[_L("Status")] = status;

        // Move alert to inactive alerts
        QVariantList inactiveAlerts = env->ds.alertData->getInactiveAlerts();

        insertAlertItem(inactiveAlerts, 0, inactiveAlerts.length() - 1, alert);

        activeAlerts.removeAt(activeAlertIdx);
        env->ds.alertData->setInactiveAlerts(inactiveAlerts);
        env->ds.alertData->setActiveAlerts(activeAlerts);
        updateAllAlerts();
    }
    else
    {
        LOG_ERROR("Update alert failed. Bad status: reporter=%s, guid=%s, codeName=%s, activeAt=%s, currentStatus=%s, newStatus=%s\n",
                  alert[_L("Reporter")].toString().CSTR(),
                  alert[_L("GUID")].toString().CSTR(),
                  alert[_L("CodeName")].toString().CSTR(),
                  alert[_L("ActiveAt")].toString().CSTR(),
                  alert[_L("Status")].toString().CSTR(),
                  status.CSTR());
    }
}

void DS_AlertAction::deactivate(QString codeName, QString data)
{
    update(codeName, data, "Inactive");
}

void DS_AlertAction::deactivateWithReason(QString codeName, QString newData, QString oldData, bool ignoreData)
{
    QString guid = env->ds.alertAction->getActiveAlertGuid(codeName, oldData, ignoreData);
    if (guid != EMPTY_GUID)
    {
        setActiveAlertData(guid, newData);
        deactivate(codeName, newData);
    }
}

void DS_AlertAction::deactivateFromSyringeIdx(QString codeName, SyringeIdx syringeIdx)
{
    QVariantList activeAlerts = env->ds.alertData->getActiveAlerts();
    QString syringeIdxStr = ImrParser::ToImr_FluidSourceSyringeIdx(syringeIdx);

    for (int alertIdx = 0; alertIdx < activeAlerts.length(); alertIdx++)
    {
        QString alertCodeName = activeAlerts[alertIdx].toMap()[_L("CodeName")].toString();
        QString alertData = activeAlerts[alertIdx].toMap()[_L("Data")].toString();
        if ( (alertCodeName == codeName) &&
             (alertData.contains(syringeIdxStr)) )
        {
            env->ds.alertAction->deactivate(alertCodeName, alertData);
        }
    }
}

void DS_AlertAction::remove(QString guid)
{
    QVariantList inactiveAlerts = env->ds.alertData->getInactiveAlerts();

    for (int alertIdx = 0; alertIdx < inactiveAlerts.length(); alertIdx++)
    {
        QVariantMap alert = inactiveAlerts[alertIdx].toMap();
        QString alertGuid = alert[_L("GUID")].toString();
        if (alertGuid == guid)
        {
            LOG_INFO("Alert removed (index=%d, reporter=%s, guid=%s, codeName=%s, status=%s, activeAt=%s, inactiveAt=%s). AlertCount=%d\n",
                     alertIdx,
                     alert[_L("Reporter")].toString().CSTR(),
                     alertGuid.CSTR(),
                     alert[_L("CodeName")].toString().CSTR(),
                     alert[_L("Status")].toString().CSTR(),
                     alert[_L("ActiveAt")].toString().CSTR(),
                     alert[_L("InactiveAt")].toString().CSTR(),
                     (int)inactiveAlerts.length() - 1);

            removeAlertItem(inactiveAlerts, alertIdx);
            env->ds.alertData->setInactiveAlerts(inactiveAlerts);
            updateAllAlerts();
            return;
        }
    }

    LOG_ERROR("Failed to remove alert - GUID not found or still active (guid=%s)\n", guid.CSTR());
}

void DS_AlertAction::remove(QVariantList guidList)
{
    QVariantList inactiveAlerts = env->ds.alertData->getInactiveAlerts();

    for (int guidIdx = 0; guidIdx < guidList.length(); guidIdx++)
    {
        QString guid = guidList[guidIdx].toString();

        for (int alertIdx = 0; alertIdx < inactiveAlerts.length(); alertIdx++)
        {
            QVariantMap alert = inactiveAlerts[alertIdx].toMap();
            QString alertGuid = alert[_L("GUID")].toString();
            if (alertGuid == guid)
            {
                removeAlertItem(inactiveAlerts, alertIdx);
                break;
            }
        }
    }

    //LOG_INFO("Alert removed. List[%d]\n", guidList.length());

    env->ds.alertData->setInactiveAlerts(inactiveAlerts);
    updateAllAlerts();
}

void DS_AlertAction::removeAll()
{
    env->ds.alertData->setInactiveAlerts(QVariantList());
    updateAllAlerts();
}

QVariantMap DS_AlertAction::getFromGuid(QString guid)
{
    QVariantMap ret;
    ret[_L("GUID")] = EMPTY_GUID;
    ret[_L("Data")] = "ALERT_NOT_FOUND";

    if (guid == EMPTY_GUID)
    {
        return ret;
    }

    QVariantList allAlerts = env->ds.alertData->getAllAlerts();

    for (int alertIdx = 0; alertIdx < allAlerts.length(); alertIdx++)
    {
        if (allAlerts[alertIdx].toMap()[_L("GUID")].toString() == guid)
        {
            // Alert found
            return allAlerts[alertIdx].toMap();
        }
    }

    return ret;
}


QVariantList DS_AlertAction::getMergedAlerts(const QVariantList &alerts)
{
    QVariantList mergedAlerts = alerts;

    // Merge duplicate alerts - combine data values
    // NOTE: The merged alert will have "ActiveAt" which is the 'latest' time of an alert activated.
    for (int alertIdx1 = 0; alertIdx1 < mergedAlerts.length(); alertIdx1++)
    {
        QVariantMap alert1 = mergedAlerts[alertIdx1].toMap();
        QString codeName1 = alert1[_L("CodeName")].toString();

        for (int alertIdx2 = alertIdx1 + 1; alertIdx2 < mergedAlerts.length(); alertIdx2++)
        {
            QVariantMap alert2 = mergedAlerts[alertIdx2].toMap();
            QString codeName2 = alert2[_L("CodeName")].toString();

            if (codeName1 == codeName2)
            {
                // Duplicate alerts found
                QStringList data1 = alert1[_L("Data")].toString().split(";");
                QStringList data2 = alert2[_L("Data")].toString().split(";");
                QString dataAll = "";

                // Merge data
                // E.g. Data1="RC1" + Data2="RC2" = DataMerged="RC1, RC2"
                //      Data1="RC1;AAA" + Data2="RC2;BBB" = DataMerged="RC1, RC2;AAA, BBB"
                //      Data1="RC1;AAA" + Data2="RC2" = DataMerged="RC1, RC2;AAA"

                int dataIdxMax = qMax(data1.length(), data2.length());

                for (int dataIdx = 0; dataIdx < dataIdxMax; dataIdx++)
                {
                    QString dataMerged = "";

                    if ( (dataIdx < data1.length()) &&
                         (dataIdx < data2.length()) &&
                         (data1[dataIdx] != data2[dataIdx]) )
                    {
                        dataMerged = env->translator->translate(data1[dataIdx]) + ", " + env->translator->translate(data2[dataIdx]);
                    }
                    else if (dataIdx < data1.length())
                    {
                        dataMerged = env->translator->translate(data1[dataIdx]);
                    }
                    else if (dataIdx < data2.length())
                    {
                        dataMerged = env->translator->translate(data2[dataIdx]);
                    }

                    if (dataIdx > 0)
                    {
                        dataAll += ";";
                    }
                    dataAll += dataMerged;
                }

                // combine data values
                alert1[_L("Data")] = dataAll;
                alert1[_L("ActiveAt")] = alert2[_L("ActiveAt")];
                mergedAlerts.removeAt(alertIdx2);
                alertIdx2--;
            }
        }
        mergedAlerts[alertIdx1] = alert1;
    }

    // Resort the array for <old> to <new>
    // Use List->Map->List to resort
    QMap<qint64, QVariant> alertMapBuf;
    for (int alertIdx = 0; alertIdx < mergedAlerts.length(); alertIdx++)
    {
        QVariantMap alert = mergedAlerts[alertIdx].toMap();
        qint64 activeAtEpochMs = Util::utcDateTimeStrToQDateTime(alert[_L("ActiveAt")].toString()).toMSecsSinceEpoch();
        alertMapBuf.insert(activeAtEpochMs, alert);
    }

    mergedAlerts.clear();
    QMap<qint64, QVariant>::const_iterator i = alertMapBuf.begin();
    while (i != alertMapBuf.end())
    {
        mergedAlerts.append(i.value());
        i++;
    }

    return mergedAlerts;
}


QVariantList DS_AlertAction::getAlertsFromTimeOffset(QDateTime from)
{
    QVariantList alerts;
    QVariantList allAlerts = env->ds.alertData->getAllAlerts();
    qint64 curTimeEpochMs = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
    qint64 fromEpochMs = from.toMSecsSinceEpoch();

    for (int alertIdx = 0; alertIdx < allAlerts.length(); alertIdx++)
    {
        QVariantMap alert = allAlerts[alertIdx].toMap();
        qint64 alertEpochMs;

        if (alert[_L("Status")].toString() == _L("Active"))
        {
            alertEpochMs = Util::utcDateTimeStrToQDateTime(alert[_L("ActiveAt")].toString()).toMSecsSinceEpoch();
        }
        else
        {
            alertEpochMs = Util::utcDateTimeStrToQDateTime(alert[_L("InactiveAt")].toString()).toMSecsSinceEpoch();
        }

        if ( (fromEpochMs >= alertEpochMs) ||
             (curTimeEpochMs < alertEpochMs) )
        {
            // Add alert if:
            // (1) alert is happened since timeoffset OR
            // (2) is happened in the future (just in case the time synch stuffed up)
            alerts.append(alert);
        }
    }
    return alerts;
}

// Ensure alert has fields from alert descriptions
void DS_AlertAction::populateAlertFields(QVariantMap &alert)
{
    QString codeName = alert[_L("CodeName")].toString();
    alert[_L("SystemAlert")] = alertDescriptions[codeName].toMap()[_L("SystemAlert")].toString();
    alert[_L("ClearOnStartup")] = alertDescriptions[codeName].toMap()[_L("ClearOnStartup")].toString();

    // Severity and Lifecycle shouldn't change.
    // There is also possibility where CRU's fields and alertDescriptions' fields are different
    // We are NOT bluntly force updating all fields from alertDescriptions for now
    //alert[_L("Severity")] = alertDescriptions[codeName].toMap()[_L("Severity")].toString();
    //alert[_L("Lifecycle")] = alertDescriptions[codeName].toMap()[_L("Lifecycle")].toString();
}
