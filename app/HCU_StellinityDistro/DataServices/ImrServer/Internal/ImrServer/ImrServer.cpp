#include <QEventLoop>
#include <QHostAddress>
#include <QMutexLocker>
#include "Common/Util.h"
#include "Common/ImrParser.h"
#include "ImrServer.h"
//#include "Apps/AppManager.h"

ImrServer::ImrServer(QObject *parent, QString baseUrl_, EnvGlobal *env_, EnvLocal *envLocal_):
    ImrServerBase(parent, baseUrl_, env_, envLocal_)
{
    digestProvider = new DigestProvider(this, env);
}

ImrServer::~ImrServer()
{
    delete digestProvider;
}

void ImrServer::ImrService(Params &params)
{
    try
    {
        QString apiName = params.request.apiName;

        if (apiName == _L("digests"))
        {
            rhDigests(params);
        }
        else if (apiName == _L("digestsdelta"))
        {
            rhDigestsDelta(params);
        }
        else if (apiName == _L("profile"))
        {
            rhProfile(params);
        }
        else if (apiName == _L("datagroup"))
        {
            rhDataGroup(params);
        }
        else if (apiName == _L("alert"))
        {
            rhAlert(params);
        }
        else if (apiName == _L("configs"))
        {
            rhConfigs(params);
        }
        else if (apiName == _L("fluidoptions"))
        {
            rhFluidOptions(params);
        }
        else if (apiName == _L("actiondigest"))
        {
            rhActionDigest(params);
        }
        else if (apiName == _L("commands/invalidate"))
        {
            rhCommandInvalidate(params);
        }
        else if (apiName == _L("commands/program"))
        {
            rhCommandProgram(params);
        }
        else if (apiName == _L("commands/truncate"))
        {
            rhCommandTruncate(params);
        }
        else if (apiName == _L("commands/start"))
        {
            rhCommandStart(params);
        }
        else if (apiName == _L("commands/stop"))
        {
            rhCommandStop(params);
        }
        else if (apiName == _L("commands/hold"))
        {
            rhCommandHold(params);
        }
        else if (apiName == _L("commands/jump"))
        {
            rhCommandJump(params);
        }
        else if (apiName == _L("commands/adjust"))
        {
            rhCommandAdjust(params);
        }
        else if (apiName == _L("commands/aircheck"))
        {
            rhCommandAircheck(params);
        }
        else if (apiName == _L("commands/arm"))
        {
            rhCommandArm(params);
        }
        else if (apiName == _L("commands/disarm"))
        {
            rhCommandDisarm(params);
        }
        else if (apiName == _L("commands/alerts/recorded"))
        {
            rhCommandAlertsRecorded(params);
        }
        else if (apiName == _L("commands/alerts/raise"))
        {
            rhCommandAlertsRaise(params);
        }
        else if (apiName == _L("commands/alerts/update"))
        {
            rhCommandAlertsUpdate(params);
        }
        else if (apiName == _L("commands/scannerinterlocks"))
        {
            rhCommandScannerInterlocks(params);
        }
        else if (apiName == _L("notify/exam/started"))
        {
            rhCommandStartExam(params);
        }
        else if (apiName == _L("notify/exam/ended"))
        {
            rhCommandEndExam(params);
        }
        else if (apiName == _L("commands/kvo/start"))
        {
            rhCommandKvoStart(params);
        }
        else if (apiName == _L("commands/kvo/stop"))
        {
            rhCommandKvoStop(params);
        }
        else if (apiName == _L("commands/patency/start"))
        {
            rhCommandPatencyStart(params);
        }
        else if (apiName == _L("commands/patency/stop"))
        {
            rhCommandPatencyStop(params);
        }
        else if (apiName == _L("calibrations"))
        {
            rhCalibrations(params);
        }
        else if (apiName == _L("permissions"))
        {
            rhPermissions(params);
        }
        else
        {
            // Unknown API received
            sendFault(params, HTTP_STATUS_NOT_IMPLEMENTED, "API not implemented: " + params.request.apiName);
        }
    }
    catch (std::exception &e)
    {
        sendFault(params, HTTP_STATUS_BAD_REQUEST, e.what());
    }

    catch (...)
    {
        // Unexpected error found while processing request..
        // Will send response with error message.
        sendFault(params, HTTP_STATUS_BAD_REQUEST, "Unknown Exception Raised");
    }
}

QString ImrServer::getActStatusMapDump()
{
    QString ret;
    mutexActStatusMap.lock();
    ret += QString().asprintf("IMR Actions[%d]: [\n", (int)actStatusMap.size());
    QMap<QString, DataServiceActionStatus>::const_iterator i = actStatusMap.begin();
    while (i != actStatusMap.end())
    {
        QString guid = i.key();
        DataServiceActionStatus status = i.value();
        ret += QString().asprintf("\tAction[%s]: %s\n", guid.CSTR(), ImrParser::ToImr_DataServiceActionStatusStr(status).CSTR());
        i++;
    }
    mutexActStatusMap.unlock();
    ret += QString().asprintf("]\n");

    return ret;
}

template <typename LambdaFn>
void ImrServer::performAction(Params &params, LambdaFn actionStartCb)
{
    DataServiceActionStatus status;
    QString actGuid = Util::newGuid();
    status.guid = actGuid;

    // Event loop while action is in progress
    QEventLoop eventLoop;
    connect(this, &ImrServer::signalActionCompleted, &eventLoop, &QEventLoop::quit);

    // Insert current status
    mutexActStatusMap.lock();
    actStatusMap.insert(actGuid, status);
    mutexActStatusMap.unlock();

    // Handle action started/completed
    env->actionMgr->onActionStarted(actGuid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
        mutexActStatusMap.lock();
        if (!actStatusMap.contains(actGuid))
        {
            mutexActStatusMap.unlock();
            LOG_WARNING("IMRSERVER" IMR_VERSION ": %s: Cannot find status(%s) from actStatusMap\n", Util::curThreadId().CSTR(), actGuid.CSTR());
            LOG_DEBUG("getActStatusMapDump() = %s\n", getActStatusMapDump().CSTR());
            return;
        }

        actStatusMap[actGuid] = curStatus;
        mutexActStatusMap.unlock();

        if (curStatus.state == DS_ACTION_STATE_STARTED)
        {
            LOG_INFO("IMRSERVER" IMR_VERSION ": %s: ACTION_STATUS: %s\n", Util::curThreadId().CSTR(), ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());

            env->actionMgr->onActionCompleted(actGuid, __PRETTY_FUNCTION__, [=](DataServiceActionStatus curStatus) {
                mutexActStatusMap.lock();
                if (!actStatusMap.contains(actGuid))
                {
                    mutexActStatusMap.unlock();
                    LOG_WARNING("IMRSERVER" IMR_VERSION ": %s: Cannot find status(%s) from actStatusMap\n", Util::curThreadId().CSTR(), actGuid.CSTR());
                    LOG_DEBUG("getActStatusMapDump() = %s\n", getActStatusMapDump().CSTR());
                    return;
                }

                actStatusMap[actGuid] = curStatus;
                mutexActStatusMap.unlock();

                LOG_INFO("IMRSERVER" IMR_VERSION ": %s: ACTION_STATUS: %s\n", Util::curThreadId().CSTR(), ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());
                emit signalActionCompleted();
            });
        }
        else
        {
            LOG_ERROR("IMRSERVER" IMR_VERSION ": %s: ACTION_STATUS: %s\n", Util::curThreadId().CSTR(), ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());

            // Status is expected not to have DS_ACTION_STATE_COMPLETED
            emit signalActionCompleted();
        }
    });

    // Handle timeout while action is in progress
    QTimer::singleShot(CRU_HCU_ACTION_PROCESS_TIMEOUT_MS, this, [=] {
        if (env->state == EnvGlobal::STATE_EXITING)
        {
            return;
        }

        mutexActStatusMap.lock();
        if (!actStatusMap.contains(actGuid))
        {
            mutexActStatusMap.unlock();
            //LOG_WARNING("IMRSERVER" IMR_VERSION ": %s: TIMEOUT: Cannot find status(%s) from actStatusMap\n", Util::curThreadId().CSTR(), actGuid.CSTR());
            return;
        }

        DataServiceActionStatus curStatus = actStatusMap[actGuid];
        mutexActStatusMap.unlock();

        if ( (curStatus.state == DS_ACTION_STATE_START_WAITING) ||
             (curStatus.state == DS_ACTION_STATE_STARTED) )
        {
            curStatus.state = DS_ACTION_STATE_TIMEOUT;
            curStatus.err = "ACTION TIMEOUT";

            env->actionMgr->deleteActAll(actGuid);
            mutexActStatusMap.lock();
            actStatusMap[actGuid] = curStatus;
            mutexActStatusMap.unlock();

            LOG_ERROR("IMRSERVER" IMR_VERSION ": %s: TIMEOUT: ACTION_STATUS: %s\n", Util::curThreadId().CSTR(), ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());
            emit signalActionCompleted();
        }
        else
        {
            LOG_WARNING("IMRSERVER" IMR_VERSION ": %s: TIMEOUT: ACTION_STATUS: %s: Unexpected Timeout Detected\n", Util::curThreadId().CSTR(), ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());
        }
    });

    // Start action
    DataServiceActionStatus innnerStatus = actionStartCb(actGuid);

    // Monitor action status and complete the http response
    if ( (innnerStatus.state == DS_ACTION_STATE_START_WAITING) ||
         (innnerStatus.state == DS_ACTION_STATE_STARTED) )
    {
        // Action started successfully.

        DataServiceActionStatus curStatus;

        while (1)
        {
            // Wait from event loop until action completed..
            eventLoop.exec();

            mutexActStatusMap.lock();
            if (!actStatusMap.contains(actGuid))
            {
                mutexActStatusMap.unlock();
                LOG_WARNING("IMRSERVER" IMR_VERSION ": %s: Cannot find status(%s) from actStatusMap\n", Util::curThreadId().CSTR(), actGuid.CSTR());
                LOG_DEBUG("getActStatusMapDump() = %s\n", getActStatusMapDump().CSTR());
                return;
            }

            curStatus = actStatusMap[actGuid];
            mutexActStatusMap.unlock();

            if ( (curStatus.state == DS_ACTION_STATE_START_WAITING) ||
                 (curStatus.state == DS_ACTION_STATE_STARTED) )
            {
                LOG_WARNING("IMRSERVER" IMR_VERSION ": %s: Unexpected ActionCompleted signal received while action(%s) is still in progress. Wait until action complete...\n", Util::curThreadId().CSTR(), ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());
                LOG_DEBUG("getActStatusMapDump() = %s\n", getActStatusMapDump().CSTR());
            }
            else
            {
                break;
            }
        }

        if (curStatus.state == DS_ACTION_STATE_COMPLETED)
        {
            sendOK(params);
        }
        else
        {
            LOG_ERROR("IMRSERVER" IMR_VERSION ": %s: ACTION_STATUS: %s\n", Util::curThreadId().CSTR(), ImrParser::ToImr_DataServiceActionStatusStr(curStatus).CSTR());
            sendFault(params, HTTP_STATUS_CONFLICT, curStatus.err);
        }

        disconnect(this, &ImrServer::signalActionCompleted, &eventLoop, &QEventLoop::quit);

        mutexActStatusMap.lock();
        actStatusMap.remove(actGuid);
        mutexActStatusMap.unlock();
    }
    else if (innnerStatus.state == DS_ACTION_STATE_COMPLETED)
    {
        // Action completed straight away
        sendOK(params);

        disconnect(this, &ImrServer::signalActionCompleted, &eventLoop, &QEventLoop::quit);

        mutexActStatusMap.lock();
        actStatusMap.remove(actGuid);
        mutexActStatusMap.unlock();
    }
    else
    {
        // Action start failed
        LOG_ERROR("IMRSERVER" IMR_VERSION ": %s: ACTION_STATUS: %s\n", Util::curThreadId().CSTR(), ImrParser::ToImr_DataServiceActionStatusStr(innnerStatus).CSTR());
        env->actionMgr->deleteActStarted(actGuid);
        sendFault(params, HTTP_STATUS_CONFLICT, innnerStatus.err);

        disconnect(this, &ImrServer::signalActionCompleted, &eventLoop, &QEventLoop::quit);
        mutexActStatusMap.lock();
        actStatusMap.remove(actGuid);
        mutexActStatusMap.unlock();
    }
}

void ImrServer::rhProfile(Params &params)
{//request.getMethod().toLower()
    if (params.request.requestMethodLowerCase == _L("get"))
    {//todo thread-safe?
        // Get software version
        QVariantMap profileMap;
        profileMap.insert("TypeName", "Stellinity2");
        profileMap.insert("FirmwareVersion", env->ds.systemData->getVersion());
        profileMap.insert("BuildDate", env->ds.systemData->getBuildDate());
        profileMap.insert("SerialNumber", env->ds.hardwareInfo->get_General_SerialNumber());
        profileMap.insert("MaximumDigestNumber", (quint32)-1);
        profileMap.insert("LastDigestNumber", digestProvider->getLastDigestId());
        profileMap.insert("DigestWrapCount", digestProvider->getDigestWrappedCount());
        profileMap.insert("ServiceStartTime", Util::qDateTimeToUtcDateTimeStr(env->ds.systemData->getUptime()));
        profileMap.insert("CurrentUtcNow", Util::qDateTimeToUtcDateTimeStr(QDateTime::currentDateTimeUtc()));

        QString connectionType = env->ds.cfgLocal->get_Settings_SruLink_ConnectionType();
        profileMap.insert("LinkType", connectionType);
        profileMap.insert("BuildType", env->ds.systemData->getBuildType());

        bool minify = (QString(params.getParameter("minify")).toLower() != _L("false"));
        sendResponseWithJsonData(params, profileMap, minify);
    }
    else
    {
        sendFault(params, HTTP_STATUS_BAD_REQUEST, "Unsupported method type");
    }
}

void ImrServer::rhDataGroup(Params &params)
{
    if (params.request.requestMethodLowerCase == _L("get"))
    {//todo thread-safe?
        QString types = params.getParameter("types");
        QString omitDataArg = params.getParameter("omitData");
        bool omitData = omitDataArg.toLower() == _L("true");
        QVariantMap dataGroup = env->ds.imrServerData->getDataGroup();
        QVariantMap dataGroupRet;

        if (types == "")
        {
            // Types not defined. Send all data items.
            QMap<QString, QVariant>::const_iterator i = dataGroup.begin();
            while (i != dataGroup.end())
            {
                QString curType = i.key();
                QVariantMap dataItem;

                dataItem.insert("UpdatedAt", dataGroup[curType].toMap()["UpdatedAt"]);
                if (!omitData)
                {
                    dataItem.insert("Data", dataGroup[curType].toMap()["Data"]);
                }
                dataGroupRet.insert(curType, dataItem);
                i++;
            }
        }
        else
        {
            // Parse Types (, separated)
            QStringList typeList = types.split(',');
            for (int typeIdx = 0; typeIdx < typeList.length(); typeIdx++)
            {
                QString type = typeList[typeIdx].trimmed();
                if (dataGroup.contains(type))
                {
                    QVariantMap dataItem;
                    dataItem.insert("UpdatedAt", dataGroup[type].toMap()["UpdatedAt"]);
                    if (!omitData)
                    {
                        dataItem.insert("Data", dataGroup[type].toMap()["Data"]);
                    }
                    dataGroupRet.insert(type, dataItem);
                }
            }
        }

        bool minify = (QString(params.getParameter("minify")).toLower() != _L("false"));
        sendResponseWithJsonData(params, dataGroupRet, minify);
    }
    else
    {
        sendFault(params, HTTP_STATUS_BAD_REQUEST, "Unsupported method type");
    }
}

void ImrServer::rhDigests(Params &params)
{
    if (params.request.requestMethodLowerCase == _L("get"))
    {//todo thread-safe?
        // Get FromId
        QString arg = params.getParameter("from");
        int fromId;

        if (arg.length() == 0)
        {
            fromId = digestProvider->getLastDigestId();
        }
        else
        {
            fromId = QString(arg).toInt();
        }

        // Get Alerts filter
        bool omitAlerts = false;
        arg = params.getParameter("omitAlerts");

        if (QString(arg).toLower() == _L("true"))
        {
            omitAlerts = true;
        }

        // Get optional maxDigests
        arg = params.getParameter("maxDigests");
        // default size limit (no limit) is -1
        int maxDigests = (arg.length() == 0) ? -1 : QString(arg).toInt();

        // Get digests
        QVariantList digests = digestProvider->getDigests(fromId, maxDigests, false, omitAlerts);
        bool minify = (QString(params.getParameter("minify")).toLower() != _L("false"));
        sendResponseWithJsonData(params, digests, minify);
    }
    else
    {
        sendFault(params, HTTP_STATUS_BAD_REQUEST, "Unsupported method type");
    }
}

void ImrServer::rhDigestsDelta(Params &params)
{
    if (params.request.requestMethodLowerCase == _L("get"))
    {//todo thread-safe?
        // Get FromId
        QString arg = params.getParameter("from");
        int fromId;

        if (arg.length() == 0)
        {
            fromId = digestProvider->getLastDigestId();
        }
        else
        {
            fromId = QString(arg).toInt();
        }

        // Get Alerts filter
        bool omitAlerts = false;
        arg = params.getParameter("omitAlerts");

        if (QString(arg).toLower() == _L("true"))
        {
            omitAlerts = true;
        }

        // Since deltas are much smaller in comparison to full digests, no need to apply a limit
        // default size limit (no limit) is -1
        int maxDigests = -1;

        // Get optional requestId
        QString requestId = params.getParameter("requestId");

        // Get all digests including one previous digest
        QVariantList digests = digestProvider->getDigests(fromId - 1, maxDigests, false, omitAlerts);
        QVariantList digestsDelta;

        for (int digestIdx = 1; digestIdx < digests.length(); digestIdx++)
        {
            QVariantMap digestDelta;
            digestProvider->getDigestDeltaMap(digestDelta, digests[digestIdx - 1].toMap(), digests[digestIdx].toMap());
            digestsDelta.append(digestDelta);
        }

        bool minify = (QString(params.getParameter("minify")).toLower() != _L("false"));
        sendResponseWithJsonData(params, digestsDelta, minify);
    }
    else
    {
        sendFault(params, HTTP_STATUS_BAD_REQUEST, "Unsupported method type");
    }
}

void ImrServer::rhAlert(Params &params)
{
    if (params.request.requestMethodLowerCase == _L("get"))
    {//todo not thread-safe
        QString guid = params.getParameter("guid");
        QVariant alert;

        if ( (guid.length() == 0) ||
             (guid == EMPTY_GUID) )
        {
            // return oldest alert
            QVariantList allAlerts = env->ds.alertData->getAllAlerts();

            if (allAlerts.length() > 0)
            {
                // return oldest alert
                alert = allAlerts.first();
            }
        }
        else
        {
            QVariantMap alertWithGuid = env->ds.alertAction->getFromGuid(guid);
            if (alertWithGuid[_L("GUID")].toString() != EMPTY_GUID)
            {
                // return alert with requested guid
                alert = alertWithGuid;
            }
        }

        bool minify = (QString(params.getParameter("minify")).toLower() != _L("false"));
        sendResponseWithJsonData(params, alert, minify);
    }
}

void ImrServer::rhConfigs(Params &params)
{
    if (params.request.requestMethodLowerCase == _L("get"))
    {//todo thread-safe?
        QString err = "";
        QVariantMap mapOut = env->ds.cfgGlobal->getConfigs(&err);

        // Complete request
        if (err == "")
        {
            bool minify = (QString(params.getParameter("minify")).toLower() != _L("false"));
            sendResponseWithJsonData(params, mapOut, minify);
        }
        else
        {
            sendFault(params, HTTP_STATUS_CONFLICT, err);
        }
    }
    else if (params.request.requestMethodLowerCase == _L("put"))
    {//todo thread-safe?
        QString jsonStr = params.request.requestBody;

        QJsonParseError parseErr;
        QJsonDocument document = QJsonDocument::fromJson(jsonStr.toUtf8(), &parseErr);
        QVariantMap mapCfg = document.toVariant().toMap();
        QString err = "";

        if (parseErr.error == QJsonParseError::NoError)
        {
            // Verify configs
            env->ds.cfgGlobal->verifyConfigs(mapCfg, &err);
            if (err == "")
            {
                env->ds.cfgGlobal->setConfigs(mapCfg, false, &err);
            }
        }
        else
        {
            err = QString().asprintf("ParseErr=%s", parseErr.errorString().CSTR());
        }

        if (err == "")
        {
            sendOK(params);
        }
        else
        {
            sendFault(params, HTTP_STATUS_BAD_REQUEST, err);
        }
    }
    else
    {
        sendFault(params, HTTP_STATUS_BAD_REQUEST, "Unsupported method type");
    }
}

void ImrServer::rhFluidOptions(Params &params)
{
    if (params.request.requestMethodLowerCase == _L("get"))
    {
        //todo thread-safe?
        QString err;
        DS_DeviceDef::FluidOptions fluidOptions = env->ds.deviceData->getFluidOptions();
        QVariantMap fluidOptionsImr = ImrParser::ToImr_FluidOptions(fluidOptions, &err);

        // Complete request
        if (err == "")
        {
            bool minify = (QString(params.getParameter("minify")).toLower() != _L("false"));
            sendResponseWithJsonData(params, fluidOptionsImr, minify);
        }
        else
        {
            sendFault(params, HTTP_STATUS_CONFLICT, err);
        }
    }
    else if (params.request.requestMethodLowerCase == _L("put"))
    {
        QString jsonStr = params.request.requestBody;

        QJsonParseError parseErr;
        QJsonDocument document = QJsonDocument::fromJson(jsonStr.toUtf8(), &parseErr);
        QString err;

        if (parseErr.error == QJsonParseError::NoError)
        {//todo thread-safe?
            QVariantMap fluidOptionsMap = document.toVariant().toMap();
            LOG_DEBUG("IMRSERVER" IMR_VERSION ": %s: PUT FLUID OPTIONS: NewOptions=%s\n", Util::curThreadId().CSTR(), Util::qVarientToJsonData(fluidOptionsMap).CSTR());
            DS_DeviceDef::FluidOptions fluidOptions = ImrParser::ToCpp_FluidOptions(fluidOptionsMap, &err);
            env->ds.deviceData->setFluidOptions(fluidOptions);
        }
        else
        {
            err = QString().asprintf("ParseErr=%s", parseErr.errorString().CSTR());
        }

        if (err == "")
        {
            sendOK(params);
        }
        else
        {
            sendFault(params, HTTP_STATUS_BAD_REQUEST, err);
        }
    }
    else
    {
        sendFault(params, HTTP_STATUS_BAD_REQUEST, "Unsupported method type");
    }
}

void ImrServer::rhActionDigest(Params &params)
{
    if (params.request.requestMethodLowerCase == _L("get"))
    {//todo thread-safe?
        params.response.responseBody = env->actionMgr->dump();
        params.response.responseBody += "\n\n";
        params.response.responseBody += getActStatusMapDump();
        params.response.responseBody += "\n\n";

        {//open the local scope to release the mutex as soon as it is done
            QMutexLocker(&env->mutexLastImrCruRequest);
            DS_ImrServerDef::ImrRequests lastImrCruRequests = env->ds.imrServerData->getLastImrCruRequests();

            params.response.responseBody += QString().asprintf("IMR CRU Past Requests[%d]: [\n", (int)lastImrCruRequests.size());
            auto it = lastImrCruRequests.cbegin();
            while (it != lastImrCruRequests.cend())
            {
                QString key = it.key();
                params.response.responseBody += QString().asprintf("\tRequest[%s]: %lldms ago\n", key.CSTR(), lastImrCruRequests[key].elapsed());
                it++;
            }
        }
        params.response.responseBody += QString().asprintf("]\n");
        params.response.responseBody += "\n\n";
        sendResponseBody(params);
    }
    else
    {
        sendFault(params, HTTP_STATUS_BAD_REQUEST, "Unsupported method type");
    }
}

void ImrServer::rhCalibrations(Params &params)
{
    if (params.request.requestMethodLowerCase == _L("get"))
    {
        bool minify = (QString(params.getParameter("minify")).toLower() != _L("false"));
        Config::Item cfg = env->ds.hardwareInfo->getHidden_CalibrationInfo();
        sendResponseWithJsonData(params, cfg.value, minify);
    }
    else
    {
        sendFault(params, HTTP_STATUS_BAD_REQUEST, "Unsupported method type");
    }
}

void ImrServer::rhPermissions(Params &params)
{
    if (params.request.requestMethodLowerCase == _L("get"))
    {
        /*
            Create permissionMap.
            The permissionMap structure is Map<api_name, required_permissions>
            Permission types are: { InjectorConfig, InjectionProgram, InjectionControl }
        */
        QVariantMap permissionMap;
        permissionMap.insert("configs", QVariantList() << "InjectorConfig");
        permissionMap.insert("commands/program", QVariantList() << "InjectionProgram");
        permissionMap.insert("commands/truncate", QVariantList() << "InjectionProgram");
        permissionMap.insert("commands/start", QVariantList() << "InjectionControl");
        permissionMap.insert("commands/stop", QVariantList() << "InjectionControl");
        permissionMap.insert("commands/hold", QVariantList() << "InjectionControl");
        permissionMap.insert("commands/jump", QVariantList() << "InjectionControl");
        permissionMap.insert("commands/adjust", QVariantList() << "InjectionControl");
        permissionMap.insert("commands/aircheck", QVariantList() << "InjectionControl");
        permissionMap.insert("commands/arm", QVariantList() << "InjectionControl");
        permissionMap.insert("commands/disarm", QVariantList() << "InjectionControl");
        permissionMap.insert("notify/exam/started", QVariantList() << "InjectionProgram");
        permissionMap.insert("notify/exam/ended", QVariantList() << "InjectionProgram");

        bool minify = (QString(params.getParameter("minify")).toLower() != _L("false"));
        sendResponseWithJsonData(params, permissionMap, minify);
    }
    else
    {
        sendFault(params, HTTP_STATUS_BAD_REQUEST, "Unsupported method type");
    }
}

void ImrServer::rhCommandInvalidate(Params &params)
{
    if (params.request.requestMethodLowerCase == _L("post"))
    {
        QString url = params.getParameter("url");
        if (url == "")
        {
            sendFault(params, HTTP_STATUS_BAD_REQUEST, "Argument {url} is missing");
        }
        else
        {
            QString method = params.getParameter("method");
            if (method == "")
            {
                method = "get";
            }

            LOG_DEBUG("IMRSERVER" IMR_VERSION ": %s: COMMANDS/INVALIDATE with args (url=%s, method=%s)\n", Util::curThreadId().CSTR(), url.CSTR(), method.CSTR());

            // Run the action before the current thread destroyed
            emit env->ds.cruAction->signalInvalidateAction(url, method);

            sendOK(params);
        }
    }
    else
    {
        sendFault(params, HTTP_STATUS_BAD_REQUEST, "Unsupported method type");
    }
}

void ImrServer::rhCommandProgram(Params &params)
{
    if (params.request.requestMethodLowerCase == _L("post"))
    {
        QString reqBody = params.request.requestBody;
        LOG_INFO("IMRSERVER" IMR_VERSION ": %s: COMMANDS/PROGRAM: Body=%s\n", Util::curThreadId().CSTR(), reqBody.CSTR());

        QString err;
        QJsonParseError parseErr;
        QJsonDocument document = QJsonDocument::fromJson(reqBody.toUtf8(), &parseErr);

        if (parseErr.error == QJsonParseError::NoError)
        {
            QVariantMap newPlanImr = document.toVariant().toMap();
            DS_ExamDef::InjectionPlan newPlan = ImrParser::ToCpp_InjectionPlan(newPlanImr, &err, true);

            if (err == "")
            {//todo thread-safe?
                DataServiceActionStatus status = env->ds.examAction->actSetContrastFluidLocationByPlan(newPlan);
                if (status.state == DS_ACTION_STATE_COMPLETED)
                {
                    // Modify current plan
                    status = env->ds.examAction->actInjectionProgram(newPlan);
                }

                if (status.state == DS_ACTION_STATE_COMPLETED)
                {
                    // Set new contrast fluid location from latest injection program
                    DS_DeviceDef::FluidInfo selectedContrast = env->ds.examData->getSelectedContrast();
                    selectedContrast.location = newPlan.getCurContrastLocation(env->ds.systemData->getStatePath(), env->ds.examData->getExecutedSteps());
                    env->ds.examData->setSelectedContrast(selectedContrast);

                    // Reload contrast. Ignore returned status
                    env->ds.examAction->actReloadSelectedContrast();
                }

                if (status.state != DS_ACTION_STATE_COMPLETED)
                {
                    LOG_ERROR("IMRSERVER" IMR_VERSION ": %s: COMMANDS/PROGRAM: Bad Status=%s\n", Util::curThreadId().CSTR(), ImrParser::ToImr_DataServiceActionStatusStr(status).CSTR());
                }

                err = status.err;
            }
        }
        else
        {
            LOG_ERROR("IMRSERVER" IMR_VERSION ": %s: Parsing error (%s)\n", Util::curThreadId().CSTR(), err.CSTR());
            err = QString().asprintf("ParseErr=%s", parseErr.errorString().CSTR());
        }

        if (err == "")
        {
            sendOK(params);
        }
        else
        {
            sendFault(params, HTTP_STATUS_BAD_REQUEST, err);
        }
    }
    else
    {
        sendFault(params, HTTP_STATUS_BAD_REQUEST, "Unsupported method type");
    }
}

void ImrServer::rhCommandTruncate(Params &params)
{
    if (params.request.requestMethodLowerCase == _L("post"))
    {
        performAction(params, [=](QString actGuid) {
            return env->ds.examAction->actAdjustInjectionVolumes(actGuid);
        });
    }
    else
    {
        sendFault(params, HTTP_STATUS_BAD_REQUEST, "Unsupported method type");
    }
}

void ImrServer::rhCommandStart(Params &params)
{
    if (params.request.requestMethodLowerCase == _L("post"))
    {
        LOG_INFO("IMRSERVER" IMR_VERSION ": %s: COMMANDS/START\n", Util::curThreadId().CSTR());

        performAction(params, [=](QString actGuid) {
            DS_ExamDef::InjectionRequestProcessStatus reqProcessStatus;
            reqProcessStatus.requestedByHcu = false;
            reqProcessStatus.state = "T_INJECTSTARTING";
            env->ds.examData->setInjectionRequestProcessStatus(reqProcessStatus);
            return env->ds.examAction->actInjectionStart(actGuid);
        });
    }
    else
    {
        sendFault(params, HTTP_STATUS_BAD_REQUEST, "Unsupported method type");
    }
}

void ImrServer::rhCommandStop(Params &params)
{

    if (params.request.requestMethodLowerCase == _L("post"))
    {
        performAction(params, [=](QString actGuid) {
            return env->ds.examAction->actInjectionStop(actGuid);
        });
    }
    else
    {
        sendFault(params, HTTP_STATUS_BAD_REQUEST, "Unsupported method type");
    }
}

void ImrServer::rhCommandHold(Params &params)
{

    if (params.request.requestMethodLowerCase == _L("post"))
    {
        performAction(params, [=](QString actGuid) {
            return env->ds.examAction->actInjectionHold(actGuid);
        });
    }
    else
    {
        sendFault(params, HTTP_STATUS_BAD_REQUEST, "Unsupported method type");
    }
}

void ImrServer::rhCommandJump(Params &params)
{

    if (params.request.requestMethodLowerCase == _L("post"))
    {
        DS_SystemDef::StatePath statePath = env->ds.systemData->getStatePath();

        QString arg = params.getParameter("phase");
        if (arg.length() == 0)
        {
            sendFault(params, HTTP_STATUS_BAD_REQUEST, "Argument {phase} is missing");
        }
        else if (statePath == DS_SystemDef::STATE_PATH_BUSY_HOLDING)
        {
            sendFault(params, HTTP_STATUS_BAD_REQUEST, "T_INJECTJUMPFAILED_InvalidState");
        }
        else if (arg.toLower() == "next")
        {//todo thread-safe?
            DS_ExamDef::ExecutedSteps executedSteps = env->ds.examData->getExecutedSteps();
            if (executedSteps.length() > 0)
            {
                int jumpToIdx = executedSteps.last().phaseIndex + 1;
                performAction(params, [=](QString actGuid) {
                    return env->ds.examAction->actInjectionJump(jumpToIdx, actGuid);
                });
            }
        }
        else
        {
            int jumpToIdx = QString(arg).toInt();
            performAction(params, [=](QString actGuid) {
                return env->ds.examAction->actInjectionJump(jumpToIdx, actGuid);
            });
        }
    }
    else
    {
        sendFault(params, HTTP_STATUS_BAD_REQUEST, "Unsupported method type");
    }
}

void ImrServer::rhCommandAdjust(Params &params)
{

    if (params.request.requestMethodLowerCase == _L("post"))
    {
        QString arg = params.getParameter("delta");
        if (arg.length() == 0)
        {
            sendFault(params, HTTP_STATUS_BAD_REQUEST, "Argument {delta} is missing");
        }
        else
        {
            double delta = arg.toDouble();
            performAction(params, [=](QString actGuid) {
                return env->ds.examAction->actInjectionAdjustFlowRate(delta, actGuid);
            });
        }
    }
    else
    {
        sendFault(params, HTTP_STATUS_BAD_REQUEST, "Unsupported method type");
    }
}

void ImrServer::rhCommandAircheck(Params &params)
{

    if (params.request.requestMethodLowerCase == _L("post"))
    {
        QString arg = params.getParameter("required");
        if (arg.length() == 0)
        {
            sendFault(params, HTTP_STATUS_BAD_REQUEST, "Argument {required} is missing");
        }
        else
        {
            bool isCheckNeeded = (arg.toLower() == _L("true"));
            bool requestOk = true;

            if (!isCheckNeeded)
            {//todo thread-safe?
                DS_DeviceDef::FluidSource suds = env->ds.deviceData->getFluidSourceSuds();
                if (!suds.isInstalled())
                {
                    requestOk = false;
                    sendFault(params, HTTP_STATUS_CONFLICT, "T_AIRCHECKFAILED_PatientLineNotConnected");
                }
                else if (suds.needsReplaced)
                {
                    requestOk = false;
                    sendFault(params, HTTP_STATUS_CONFLICT, "T_AIRCHECKFAILED_PatientLineNeedsReplaced");
                }
                else if (!suds.isReady)
                {
                    requestOk = false;
                    sendFault(params, HTTP_STATUS_CONFLICT, "T_AIRCHECKFAILED_PatientLineNotReady");
                }
            }

            if (requestOk)
            {
                env->ds.examData->setIsAirCheckNeeded(isCheckNeeded);
                sendOK(params);
            }
        }
    }
    else
    {
        sendFault(params, HTTP_STATUS_BAD_REQUEST, "Unsupported method type");
    }
}

void ImrServer::rhCommandArm(Params &params)
{

    if (params.request.requestMethodLowerCase == _L("post"))
    {
        const DS_ExamDef::InjectionStep *curStep = env->ds.examData->getExecutingStep();
        if (curStep == NULL)
        {
            sendFault(params, HTTP_STATUS_CONFLICT, "Bad Executing Step");
            return;
        }
        bool isPreloaded = curStep->isPreloaded;

        performAction(params, [=](QString actGuid) {
            DS_ExamDef::InjectionRequestProcessStatus reqProcessStatus;
            reqProcessStatus.requestedByHcu = false;
            reqProcessStatus.state = "T_ARMING";
            env->ds.examData->setInjectionRequestProcessStatus(reqProcessStatus);

            return env->ds.examAction->actArm(isPreloaded ? DS_ExamDef::ARM_TYPE_PRELOAD_SECOND : DS_ExamDef::ARM_TYPE_NORMAL, actGuid);
        });
    }
    else
    {
        sendFault(params, HTTP_STATUS_BAD_REQUEST, "Unsupported method type");
    }
}

void ImrServer::rhCommandDisarm(Params &params)
{

    if (params.request.requestMethodLowerCase == _L("post"))
    {
        performAction(params, [=](QString actGuid) {
            DS_ExamDef::InjectionRequestProcessStatus reqProcessStatus;
            reqProcessStatus.requestedByHcu = false;
            reqProcessStatus.state = "T_DISARMING";
            env->ds.examData->setInjectionRequestProcessStatus(reqProcessStatus);
            return env->ds.examAction->actDisarm(actGuid);
        });
    }
    else
    {
        sendFault(params, HTTP_STATUS_BAD_REQUEST, "Unsupported method type");
    }
}

void ImrServer::rhCommandKvoStart(Params &params)
{

    if (params.request.requestMethodLowerCase == _L("post"))
    {
        sendFault(params, HTTP_STATUS_NOT_IMPLEMENTED, "API not implemented");
    }
    else
    {
        sendFault(params, HTTP_STATUS_BAD_REQUEST, "Unsupported method type");
    }
}

void ImrServer::rhCommandKvoStop(Params &params)
{

    if (params.request.requestMethodLowerCase == _L("post"))
    {
        sendFault(params, HTTP_STATUS_NOT_IMPLEMENTED, "API not implemented");
    }
    else
    {
        sendFault(params, HTTP_STATUS_BAD_REQUEST, "Unsupported method type");
    }
}

void ImrServer::rhCommandPatencyStart(Params &params)
{

    if (params.request.requestMethodLowerCase == _L("post"))
    {
        sendFault(params, HTTP_STATUS_NOT_IMPLEMENTED, "API not implemented");
    }
    else
    {
        sendFault(params, HTTP_STATUS_BAD_REQUEST, "Unsupported method type");
    }
}

void ImrServer::rhCommandPatencyStop(Params &params)
{

    if (params.request.requestMethodLowerCase == _L("post"))
    {
        sendFault(params, HTTP_STATUS_NOT_IMPLEMENTED, "API not implemented");
    }
    else
    {
        sendFault(params, HTTP_STATUS_BAD_REQUEST, "Unsupported method type");
    }
}

void ImrServer::rhCommandAlertsRecorded(Params &params)
{

    if (params.request.requestMethodLowerCase == _L("post"))
    {
        QJsonParseError parseErr;
        QString reqBody = params.request.requestBody;
        QJsonDocument document = QJsonDocument::fromJson(reqBody.toUtf8(), &parseErr);
        QString err = "";

        if (parseErr.error == QJsonParseError::NoError)
        {
            QVariantList list = document.toVariant().toList();

            if (list.length() == 0)
            {
                err = "List<GUID> length is 0";
            }
            else
            {
                //todo not thread-safe
                env->ds.alertAction->remove(list);
            }
        }
        else
        {
            err = QString().asprintf("ParseErr=%s", parseErr.errorString().CSTR());
        }

        if (err == "")
        {
            sendOK(params);
        }
        else
        {
            LOG_ERROR("IMRSERVER" IMR_VERSION ": %s: Parsing error (%s)\n", Util::curThreadId().CSTR(), err.CSTR());
            sendFault(params, HTTP_STATUS_BAD_REQUEST, err);
        }
    }
    else
    {
        sendFault(params, HTTP_STATUS_BAD_REQUEST, "Unsupported method type");
    }
}

void ImrServer::rhCommandAlertsRaise(Params &params)
{

    if (params.request.requestMethodLowerCase == _L("post"))
    {
        QJsonParseError parseErr;
        QString err = "";
        QString reqBody = params.request.requestBody;
        QJsonDocument document = QJsonDocument::fromJson(reqBody.toUtf8(), &parseErr);

        if (parseErr.error == QJsonParseError::NoError)
        {
            QVariantMap descriptor = document.toVariant().toMap();
            descriptor.insert("Reporter", "CRU");

            QString alertId = descriptor["GUID"].toString();

            //todo thread-safe?
            if ( (env->ds.alertAction->getFromGuid(alertId)["GUID"].toString() != EMPTY_GUID) ||
                 (env->ds.alertAction->isActivated(descriptor)) )
            {
                err = QString().asprintf("Alert Already Activated");
            }
            else
            {
                env->ds.alertAction->activate(descriptor);
            }
        }
        else
        {
            err = QString().asprintf("ParseErr=%s", parseErr.errorString().CSTR());
        }

        if (err == "")
        {
            sendOK(params);
        }
        else
        {
            LOG_ERROR("IMRSERVER" IMR_VERSION ": %s: Parsing error (%s)\n", Util::curThreadId().CSTR(), err.CSTR());
            sendFault(params, HTTP_STATUS_BAD_REQUEST, err);
        }
    }
    else
    {
        sendFault(params, HTTP_STATUS_BAD_REQUEST, "Unsupported method type");
    }
}

void ImrServer::rhCommandAlertsUpdate(Params &params)
{

    if (params.request.requestMethodLowerCase == _L("post"))
    {
        if (params.getParameter("guid").length() == 0)
        {
            sendFault(params, HTTP_STATUS_BAD_REQUEST, "Argument {guid} is missing");
        }
        else if (params.getParameter("status").length() == 0)
        {
            sendFault(params, HTTP_STATUS_BAD_REQUEST, "Argument {status} is missing");
        }
        else
        {
            QString guid = params.getParameter("guid");
            QString status = params.getParameter("status");

            LOG_DEBUG("IMRSERVER" IMR_VERSION ": %s: POST ALERTS_UPDATE: guid=%s, status=%s\n", Util::curThreadId().CSTR(), guid.CSTR(), status.CSTR());
            //todo thread-safe?
            env->ds.alertAction->update(guid, status);
            sendOK(params);
        }
    }
}

void ImrServer::rhCommandScannerInterlocks(Params &params)
{

    if (params.request.requestMethodLowerCase == _L("post"))
    {
        QString err = "";
        QJsonParseError parseErr;
        QString reqBody = params.request.requestBody;
        QJsonDocument document = QJsonDocument::fromJson(reqBody.toUtf8(), &parseErr);

        if (parseErr.error == QJsonParseError::NoError)
        {
            QVariantMap map = document.toVariant().toMap();

            //todo thread-safe?
            QString err;
            DS_ExamDef::ScannerInterlocks interlocks = ImrParser::ToCpp_ScannerInterlocks(map, &err);

            if (err == "")
            {
                env->ds.examData->setScannerInterlocks(interlocks);
            }
            else
            {
                err = QString().asprintf("ParseErr=%s", err.CSTR());
            }
        }
        else
        {
            err = QString().asprintf("ParseErr=%s", parseErr.errorString().CSTR());
        }

        if (err == "")
        {
            sendOK(params);
        }
        else
        {
            LOG_ERROR("IMRSERVER" IMR_VERSION ": %s: Parsing error (%s)\n", Util::curThreadId().CSTR(), err.CSTR());
            sendFault(params, HTTP_STATUS_BAD_REQUEST, err);
        }
    }
    else
    {
        sendFault(params, HTTP_STATUS_BAD_REQUEST, "Unsupported method type");
    }
}

void ImrServer::rhCommandStartExam(Params &params)
{

    if (params.request.requestMethodLowerCase == _L("post"))
    {
        LOG_INFO("IMRSERVER" IMR_VERSION ": %s: COMMANDS/ENDSTART:\n", Util::curThreadId().CSTR());

        if (params.getParameter("examGuid").length() == 0)
        {
            sendFault(params, HTTP_STATUS_BAD_REQUEST, "Argument {examGuid} is missing");
        }
        else
        {
            //todo thread-safe?
            QString newExamGuid = params.getParameter("examGuid");
            QString prevExamGuid = env->ds.examData->getExamGuid();

            LOG_INFO("IMRSERVER" IMR_VERSION ": %s: COMMANDS/ENDSTART: Setting exam guid from %s -> %s\n", Util::curThreadId().CSTR(),  prevExamGuid.CSTR(), newExamGuid.CSTR());

            if (prevExamGuid == EMPTY_GUID)
            {
                // Exam is triggered from CRU
                LOG_INFO("IMRSERVER" IMR_VERSION ": %s: COMMANDS/ENDSTART: Exam is started from CRU\n", Util::curThreadId().CSTR());

                // Exam is triggered from CRU.
                DS_ExamDef::ExamAdvanceInfo examAdvanceInfo = env->ds.examData->getExamAdvanceInfo();
                examAdvanceInfo.guid = newExamGuid;
                env->ds.examData->setExamAdvanceInfo(examAdvanceInfo);

                env->ds.examAction->actExamStart(true);
                sendOK(params);
            }
            else if (prevExamGuid != newExamGuid)
            {
                sendFault(params, HTTP_STATUS_BAD_REQUEST, "Exam in Progress: " + prevExamGuid);
            }
            else
            {
                sendOK(params);
            }
        }
    }
    else
    {
        sendFault(params, HTTP_STATUS_BAD_REQUEST, "Unsupported method type");
    }
}

void ImrServer::rhCommandEndExam(Params &params)
{
    if (params.request.requestMethodLowerCase == _L("post"))
    {
        LOG_INFO("IMRSERVER" IMR_VERSION ": %s: COMMANDS/ENDEXAM:\n", Util::curThreadId().CSTR());

        DS_SystemDef::StatePath curState = env->ds.systemData->getStatePath();
        if ( (curState == DS_SystemDef::STATE_PATH_READY_ARMED) ||
             (curState == DS_SystemDef::STATE_PATH_BUSY_HOLDING) ||
             (curState == DS_SystemDef::STATE_PATH_BUSY_FINISHING) ||
             (curState == DS_SystemDef::STATE_PATH_EXECUTING) )
        {
            sendFault(params, HTTP_STATUS_BAD_REQUEST, "Invalid State: " + ImrParser::ToImr_StatePath(curState));
            return;
        }

        if (params.getParameter("examGuid").length() == 0)
        {
            sendFault(params, HTTP_STATUS_BAD_REQUEST, "Argument {examGuid} is missing");
        }
        else
        {//todo thread-safe?
            QString endExamGuid = params.getParameter("examGuid");
            QString curExamGuid = env->ds.examData->getExamGuid();

            LOG_INFO("IMRSERVER" IMR_VERSION ": %s: COMMANDS/ENDEXAM: examGuid=%s\n", Util::curThreadId().CSTR(), endExamGuid.CSTR());

            if (endExamGuid == curExamGuid)
            {
                // Exam is ended from CRU, so perform the required action
                LOG_INFO("IMRSERVER" IMR_VERSION ": %s: COMMANDS/ENDEXAM: Exam (guid=%s) is ended from CRU\n", Util::curThreadId().CSTR(), endExamGuid.CSTR());
                env->ds.examAction->actExamEnd(true);
            }
            else
            {
                // It is possible for the SRU to end the current exam and then start a new one
                // before the CRU notifies of the ended past one. Due to this race condition,
                // we don't return a fault, and instead log a warning.
                LOG_INFO("IMRSERVER" IMR_VERSION ": %s: COMMANDS/ENDEXAM: Exam in Progress (guid=%s) does not match provided exam (guid=%s). Doing nothing.\n", Util::curThreadId().CSTR(), curExamGuid.CSTR(), endExamGuid.CSTR());
            }
            sendOK(params);
        }
    }
    else
    {
        sendFault(params, HTTP_STATUS_BAD_REQUEST, "Unsupported method type");
    }
}
