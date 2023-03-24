#include <QUrl>
#include "Apps/AppManager.h"
#include "ImrServerWrapper.h"
#include "httpserver/httplistener.h"
#include "ImrServer0/ImrServer0.h"
#include "ImrServer/ImrServer.h"
#include "DataServices/Cru/DS_CruData.h"
#include "DataServices/CfgLocal/DS_CfgLocal.h"
#include "DataServices/Capabilities/DS_Capabilities.h"
#include "DataServices/ImrServer/DS_ImrServerData.h"
#include "ImrServerBase.h"
#include "Common/Util.h"

ImrServerWrapper::ImrServerWrapper(QObject *parent, EnvGlobal *env_) :
    HttpRequestHandler(parent),
    env(env_)
{
    /**
     * Register the main app for runnig a thread task
     */
    crossThreadJob = new CrossThreadJob();
    crossThreadJob->subscribeExecutingThread((QObject *)env->appManager);

    envLocal = new EnvLocal("DS_ImrServer-Wrapper", "IMR_SERVER_WRAPPER", LOG_LRG_SIZE_BYTES);
    connect(env->appManager, &AppManager::signalAppInitialised, this, [=]() {
        slotAppInitialised();
    });

    //bool dosAttackMonitorEnabled = env->ds.capabilities->getNetwork_DosAttackMonitorEnabled().value.toBool();
    connect(env->ds.capabilities, &DS_Capabilities::signalConfigChanged_Network_DosAttackMonitorEnabled, this, [=](const Config::Item &val, const Config::Item &prevVal) {
         dosAttackMonitorEnabled = val.value.toBool();
    });

    //int bodyDumpLimit = env->ds.capabilities->getLogging_ImrServerDumpBodySizeLimit().value.toInt();
    connect(env->ds.capabilities, &DS_Capabilities::signalConfigChanged_Logging_ImrServerDumpBodySizeLimit, this, [=](const Config::Item &val, const Config::Item &prevVal) {
         bodyDumpLimit = val.value.toInt();
    });

    //bool logHcuDigestEnabled = env->ds.capabilities->getLogging_ImrServerHcuDigest().value.toBool();
    connect(env->ds.capabilities, &DS_Capabilities::signalConfigChanged_Logging_ImrServerHcuDigest, this, [=](const Config::Item &val, const Config::Item &prevVal) {
         logHcuDigestEnabled = val.value.toBool();
    });

    servers.append(new ImrServer0(this, IMR_SRU_URL_PREFIX "/v0/", env, envLocal));
    servers.append(new ImrServer(this, IMR_SRU_URL_PREFIX "/" IMR_VERSION "/", env, envLocal));
}

ImrServerWrapper::~ImrServerWrapper()
{
    qDeleteAll(servers.begin(), servers.end());
    delete listener;
    delete settings;
    delete envLocal;
    delete crossThreadJob;
}

void ImrServerWrapper::slotAppInitialised()
{
    // Start the ImrServer after app started
    QTimer::singleShot(1000, this, [=] {
        // Init settings
        settings = new QSettings(QString(PATH_RESOURCES_DOC) + "/ImxRestServer.settings", QSettings::IniFormat, this);
        settings->beginGroup("listener");

        int portNumber = env->ds.cfgLocal->get_Hidden_HcuPort();
        settings->setValue("port", portNumber);
        listener = new HttpListener(settings, this, parent());
        LOG_INFO("Server is listening from '0.0.0.0:%d%s'..\n", portNumber, IMR_SRU_URL_PREFIX);
    });    
}


/**
 * @brief ImrServerWrapper::getRequestParametersString
 * @param request
 * @return the parameter request matching CRU implementation SortedQueryString()
 * e.g.       from=11&maxdigests=1&minify=true&omitalerts=true&requestid=76%3b143511362%3b0
 */
QString ImrServerWrapper::getRequestParametersString(const HttpRequest &request) const
{
    QMultiMap<QByteArray, QByteArray> paramMap = request.getParameterMap();
    if (paramMap.isEmpty())
        return QString();

    auto i = paramMap.begin();
    QStringList strList;
    while (i != paramMap.end())
    {
        QByteArray key = i.key();
        QList<QByteArray> values = paramMap.values(key);
        /*
         * value encoding:
         * C# HttpUtility.UrlEncode()
         * Qt QUrl::toPercentEncoding
         */
        auto joint_values = QUrl::toPercentEncoding(values.join(';'));
        strList << QString().asprintf("%s=%s", key.CSTR(), joint_values.CSTR());
        i++;
    }
    strList.sort(); //Sorts the list of strings in ascending order

    return strList.join("&");
}


/**
 * @brief ImrServerWrapper::validateHmacRequest
 * @param request
 * @param params
 * @return true if HMAC header contains valid data, return false on error and response error has been set in the response.
 */
bool ImrServerWrapper::validateHmacRequest(const HttpRequest &request, HttpResponse &response, ImrServerBase::Params &params)
{
    QString method = request.getMethod();
    QString url = request.getPath();
    QString body = request.getBody();

    //with the current QtWebApp version the raw path has been converted into params!
    //no longer a raw path - does not have the query!
    QString rawPath = request.getRawPath(); //same as url!

    //QtWebApp has splitted the queries into map so we need to restore the same way the CRU does
    //split into arrays and sort aphabetically.
    QString query = getRequestParametersString(request);

    auto header_map = request.getHeaderMap();
    auto hmac_hash = header_map.constFind(HTTP_HMAC_HASH_HEADER);
    auto hmac_at = header_map.constFind(HTTP_HMAC_UE_HEADER);
    qint64 now_ue = QDateTime::currentSecsSinceEpoch();

    if
    (
        hmac_hash == header_map.constEnd()
        ||
        hmac_at == header_map.constEnd()
    )
    {
        QStringList list;
        auto i = header_map.constBegin();
        while (i != header_map.constEnd()) {
            list << i.key() << ": " << i.value() << ", ";
            ++i;
        }

        LOG_ERROR("REQ[%s] missing SY2-HMAC or SY2-HMAC-UE headers Headers=%s\n", url.CSTR(), list.join("").CSTR());        
        response.setStatus(HMAC_HTTP_STATUS_RESPONSE_ON_ERROR);
        return false;
    }
    QString hmac_at_str = hmac_at.value();
    QString hmac_hash_str = hmac_hash.value();

    //ignore time stamp for /profile, /digests, or /versions url
    bool ignored_time_diff = url.endsWith("/profile") || url.endsWith("/digests") || url.endsWith("/digest") || url.endsWith("/versions");
    QString serial = this->env->ds.hardwareInfo->get_General_SerialNumber();
    HMAC hmac(serial);
    auto computedHashString = hmac.getRequestHash(method, url, query, body, hmac_at_str);

    if (!hmac.validateHMAC(hmac_hash_str, hmac_at_str, computedHashString, now_ue, ignored_time_diff))
    {
        LOG_ERROR
        (
            "REQ[%s] HMAC mismatched\nquery=%s\n body=%s\n rx.t=%s\n",
            rawPath.CSTR(),
            query.CSTR(),
            body.CSTR(),
            hmac_at_str.CSTR()
         );
        LOG_ERROR("HMAC rx.h=%s\n", hmac_hash_str.CSTR());
        LOG_ERROR("HMAC calc=%s\n",  computedHashString.CSTR());
        response.setStatus(HMAC_HTTP_STATUS_RESPONSE_ON_ERROR);
        return false;
    }

    return true;
}

//static
ParamsResponse ImrServerWrapper::doServiceOnMain(void *context, const ParamsRequest &request)
{
    ImrServerBase *serverPtr = (ImrServerBase *)context;
    ImrServerBase::Params params(request);
    serverPtr->ImrService(params);
    //qDebug() << QThread::currentThreadId() << "work start" << request.apiName << context;

    ParamsResponse res = params.response;
    return res;
}


void ImrServerWrapper::service(HttpRequest &request, HttpResponse &response)
{
    if (env->state == EnvGlobal::STATE_EXITING)
    {
        return;
    }

    if (servers.length() == 0)
    {
        LOG_ERROR("No Servers Available\n");
        return;
    }

    QString path = request.getPath().toLower();

    if (path.contains("/favicon.ico"))
    {
        // Ignore Web browser traffic
        return;
    }

    QString err;
    ImrServerBase::Params params(request, &response);
    params.request.reqId = Util::newGuid();

    if (!validateHmacRequest(request, response, params))
    { //error has been set in the response.
        return;
    }

    updateCruLinkStatus(request);

    // Check Dos Attack
    err = dosAttackCheck(request);
    if (err != "")
    {
        servers[0]->sendFault(params, HTTP_STATUS_CONFLICT, err, Util::curThreadId());
        return;
    }

    // Start Elasped Timer
    QElapsedTimer reqProcessedElapsedTimer;
    reqProcessedElapsedTimer.start();

    // Prepare Service Provider
    requestPreProcess(request, response, params);

    // Handle Request
    bool supportedPath = false;
    for (int serverIdx = 0; serverIdx < servers.length(); serverIdx++)
    {
        QString baseUrl = servers[serverIdx]->getBaseUrl();
        if (path.startsWith(baseUrl))
        {
            supportedPath = true;
            // don't transfer server[0] or digests to the main thread
            // because they are thread-safe.
            bool to_main_thread = serverIdx == 0 ? false : (!path.endsWith("digests"));
            //bool to_main_thread = serverIdx == 0 ? false : true;

            params.request.apiName = path.replace(baseUrl, "");
            if (to_main_thread)
            {
                params.response = crossThreadJob->performAction(params.request, servers[serverIdx], &doServiceOnMain);
            }
            else
            {
                servers[serverIdx]->ImrService(params);
            }

            // now flush send the response has been generated.
            servers[serverIdx]->updateHeaderAndSendDelayRespose(params, response);

            break;
        }
    }

    if (!supportedPath)
    {
        err = "API not implemented";
        servers[0]->sendFault(params, HTTP_STATUS_NOT_IMPLEMENTED, "API not implemented", Util::curThreadId());
        servers[0]->updateHeaderAndSendDelayRespose(params, response);
        return;
    }

    qint64 spentTimeMs = reqProcessedElapsedTimer.elapsed();
    if (spentTimeMs > IMR_HCU_ACTION_PROCESS_TIME_WARNING_MS)
    {
        LOG_ERROR("%s: Request Processed TOO SLOW: Processed=%lldms, Limit=%dms\n", Util::curThreadId().CSTR(), spentTimeMs, CRU_HCU_ACTION_PROCESS_TIMEOUT_MS);
    }

    dosAttackPostProcess(request);

    requestPostProcess(request, response, params);
}

void ImrServerWrapper::updateCruLinkStatus(const HttpRequest &request)
{
    // Update CRU Link Status
    DS_CruDef::CruLinkStatus linkStatus = env->ds.cruData->getCruLinkStatus();
    if (linkStatus.state == DS_CruDef::CRU_LINK_STATE_INACTIVE)
    {
        // Confirm the peer is CRU
        if (env->ds.cruAction->isRequestFromCru(request.getPeerAddress().toIPv4Address()))
        {
            //LOG_DEBUG("Received from CRU(%s)\n", peerAddressStr.CSTR());
            LOG_INFO("%s: Received first request while link is inactive. HCU shall start the hand shake soon.\n", Util::curThreadId().CSTR());
            linkStatus.state = DS_CruDef::CRU_LINK_STATE_RECOVERING;
            linkStatus.quality = DS_CruDef::CRU_LINK_QUALITY_POOR;
            linkStatus.signalLevel = "";
            env->ds.cruData->setCruLinkStatus(linkStatus);
        }
    }
}

void ImrServerWrapper::requestPreProcess(const HttpRequest &request, HttpResponse &response, const ImrServerBase::Params &params)
{
    // Prepare RX Info Prams
    // TODO: specify body length
    QString requestDump = servers[0]->getRequestDump(request, bodyDumpLimit);
    QString requestPath = request.getPath().toLower();

    bool digestRequest = requestPath.contains(_L("digests"));

    static int prevSeqNumber = -1;

    if ( !digestRequest || logHcuDigestEnabled )
    {
        LOG_DEBUG("%s: REQ[%s]: RX: %s\n", Util::curThreadId().CSTR(), params.request.reqId.CSTR(), requestDump.CSTR());
        if (digestRequest)
        {
            QMultiMap<QByteArray, QByteArray> paramMap = request.getParameterMap();
            if (paramMap.contains("requestId"))
            {
                QString requestId = paramMap.value("requestId");
                QStringList requestIdValues = requestId.split(";");
                int seqNumber = requestIdValues[0].toInt();
                qint64 reqAtTime = QTime::fromString(requestIdValues[1], "HHmmsszzz").msecsSinceStartOfDay();
                qint64 currTime = QTime::currentTime().msecsSinceStartOfDay();
                qint64 timeDiff = currTime - reqAtTime;

                if (timeDiff > (CRU_TIME_SYNC_TOLERANCE_SEC * 1000))
                {
                    // TODO: timeDiff is not accurate as HCU and CRU clock is not perfectly synchronised. The warning should be displayed based on average delay.
                    LOG_ERROR("%s: REQ[%s]: RX: Delayed request received: Delay=%lldms\n", Util::curThreadId().CSTR(), params.request.reqId.CSTR(), timeDiff);
                }

                if ((prevSeqNumber != -1) && (seqNumber != 0) && (prevSeqNumber + 1) != seqNumber)
                {
                    LOG_ERROR("%s: REQ[%s]: RX: Sequence Number mismatch: SeqNumber = %d -> %d\n", Util::curThreadId().CSTR(), params.request.reqId.CSTR(), prevSeqNumber, seqNumber);
                }
                prevSeqNumber = seqNumber;
            }
        }
    }

    response.setHeader("Content-Type", "application/json; charset=UTF-8");
    response.setHeader("Access-Control-Allow-Origin", "*");
    response.setHeader("Access-Control-Allow-Headers", "Origin, X-requested-with, Content-Type, Accept");

    // TODO: Test dummy removed. See if required.
    //response.setCookie(HttpCookie("wsTest","CreateDummyPerson", 600));
}

void ImrServerWrapper::requestPostProcess(const HttpRequest &request, HttpResponse &response, const ImrServerBase::Params &params)
{
    QString requestPath = request.getPath().toLower();

    if ( (!requestPath.contains(_L("digests"))) || (logHcuDigestEnabled) )
    {
        // Minimise the responseBody
        QString shortResponseBody = params.response.responseBody;
        if (params.response.responseBody.length() > bodyDumpLimit)
        {
            shortResponseBody = params.response.responseBody.left(bodyDumpLimit) + "...";
        }

        LOG_DEBUG("%s: REQ[%s]: TX: len=%d(zipLen=%d): %s\n",
                  Util::curThreadId().CSTR(),
                  params.request.reqId.CSTR(),
                  (int)params.response.responseBody.length(),
                  params.response.responseCompressed ? params.response.responseCompressedLen : (int)params.response.responseBody.length(),
                  shortResponseBody.CSTR());
    }
}

QString ImrServerWrapper::dosAttackCheck(const HttpRequest &request)
{
    // Prevent Dos Attack via IMR from CRU: Prevent a new message that has same API and method to last one if received too soon

    if (!dosAttackMonitorEnabled)
    {
        return "";
    }

    quint32 clientAddr = request.getPeerAddress().toIPv4Address();
    bool isRequestFromCru = env->ds.cruAction->isRequestFromCru(clientAddr);

    if (!isRequestFromCru)
    {
        // No Dos Attack: Request NOT from CRU
        return "";
    }

    QMutexLocker(&env->mutexLastImrCruRequest);
    DS_ImrServerDef::ImrRequests lastImrCruRequests = env->ds.imrServerData->getLastImrCruRequests();
    QString imrKey = request.getPath() + request.getMethod();
    imrKey = imrKey.toLower();

    if (!lastImrCruRequests.contains(imrKey))
    {
        // No Dos Attack: New Request
        return "";
    }

    quint64 timeMsSinceLast = lastImrCruRequests[imrKey].elapsed();
    if (timeMsSinceLast < IMR_LAST_REQUEST_LIMIT_MS)
    {
        QString err = QString().asprintf("Dos Attack Detected: TimeSinceLastRx=%lldms", timeMsSinceLast);
        LOG_ERROR("%s", err.CSTR());
        env->ds.alertAction->activate("HCUInternalSoftwareError", err);
        lastImrCruRequests.remove(imrKey);
        return err;
    }
    return "";
}

void ImrServerWrapper::dosAttackPostProcess(const HttpRequest &request)
{
    if (!dosAttackMonitorEnabled)
    {
        return;
    }

    quint32 clientAddr = request.getPeerAddress().toIPv4Address();
    bool isRequestFromCru = env->ds.cruAction->isRequestFromCru(clientAddr);

    if (!isRequestFromCru)
    {
        return;
    }

    // Clean up old requests from CRU
    QMutexLocker(&env->mutexLastImrCruRequest);
    DS_ImrServerDef::ImrRequests lastImrCruRequests = env->ds.imrServerData->getLastImrCruRequests();
    auto it = lastImrCruRequests.begin();
    while (it != lastImrCruRequests.end())
    {
        quint64 timeMsSinceLast = it.value().elapsed();
        if (timeMsSinceLast >= IMR_LAST_REQUEST_LIMIT_MS)
        {
            it = lastImrCruRequests.erase(it);
        }
        else
        {
            it++;
        }
    }

    // Remember current request
    QElapsedTimer reqProcessedElapsedTimer;
    QString imrKey = request.getPath() + request.getMethod();
    lastImrCruRequests.insert(imrKey, reqProcessedElapsedTimer);
    lastImrCruRequests[imrKey].restart();
    env->ds.imrServerData->setLastImrCruRequests(lastImrCruRequests);
}
