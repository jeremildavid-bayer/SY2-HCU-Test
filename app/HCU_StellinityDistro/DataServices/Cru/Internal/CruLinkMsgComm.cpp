#include <QNetworkReply>
#include "CruLinkMsgComm.h"
#include "DataServices/Cru/DS_CruAction.h"
#include "DataServices/Cru/DS_CruData.h"
#include "DataServices/Alert/DS_AlertAction.h"
#include "DataServices/HardwareInfo/DS_HardwareInfo.h"

CruLinkMsgComm::CruLinkMsgComm(QObject *parent, EnvGlobal *env_, EnvLocal *envLocal_, QString guid_, DataServiceActionStatus actStatus_, int timeoutMs_) :
    QObject(parent),
    env(env_),
    envLocal(envLocal_),
    guid(guid_),
    actStatus(actStatus_),
    timeoutMs(timeoutMs_)
{
    nam = new QNetworkAccessManager(NULL);

    connect(nam, SIGNAL(finished(QNetworkReply*)), SLOT(slotFinished(QNetworkReply*)));
    connect(&tmrRxTimeout, SIGNAL(timeout()), SLOT(slotRxTimeout()));
}

CruLinkMsgComm::~CruLinkMsgComm()
{
    tmrRxTimeout.stop();
    nam->deleteLater();
}

void CruLinkMsgComm::start()
{
    QNetworkRequest req;
    HMAC hmac(env->ds.hardwareInfo->get_General_SerialNumber());
    auto hmac_ue = QString::number(QDateTime::currentSecsSinceEpoch());
    req.setUrl(QUrl(actStatus.request));
    req.setRawHeader("Content-Type", "application/json");
	
    tmrRxTimeout.start(timeoutMs);

    QString reqMethod = actStatus.arg;
    QString body = actStatus.arg2;
    QString query;

    if (req.url().hasQuery())
    {//sort the query string like CRU
        QString unsorted_query = req.url().query(QUrl::FullyEncoded);
        query = hmac.sortedQueryString(unsorted_query);
    }

    auto urlPath = req.url().path();
    QString hmac_value = hmac.getRequestHash(reqMethod, urlPath, query, body, hmac_ue);

    //debug disable HMAC to CRU
    req.setRawHeader(HTTP_HMAC_UE_HEADER_UPPER, hmac_ue.toLocal8Bit());
    req.setRawHeader(HTTP_HMAC_HASH_HEADER_UPPER, hmac_value.toLocal8Bit());

#if 0 //turn off after debugging
    LOG_DEBUG
    (
        "CRU_REQ %s | %s | %s | %s\n",
        reqMethod.CSTR(),
        actStatus.request.CSTR(),
        urlPath.CSTR(),
        body.CSTR()
    );
    LOG_DEBUG
    (
        "CRU_HMAC | %s | %s\n",
        hmac_ue.CSTR(),
        hmac_value.CSTR()
    );
#endif

    if (reqMethod == _L("put"))
    {
        nam->put(req, body.toLocal8Bit());
    }
    else if (reqMethod == _L("post"))
    {
        nam->post(req, body.toLocal8Bit());
    }
    else
    {
        nam->get(req);
    }

}


void CruLinkMsgComm::slotFinished(QNetworkReply *reply)
{
    tmrRxTimeout.stop();
    actStatus.reply = reply->readAll();
    //Get the current time.
    qint64 now_ue = QDateTime::currentSecsSinceEpoch(); // toSecsSinceEpoch is already in UTC

    //validate the HMAC from the reply
    bool has_hmac_ue = reply->hasRawHeader(HTTP_HMAC_UE_HEADER_UPPER);
    bool has_hmac_hash = reply->hasRawHeader(HTTP_HMAC_HASH_HEADER_UPPER);
    QVariant http_status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    // Since we never set the http "Content-Encoding" to accept deflate or gzip
    // the reply data at this point has been decompressed automatically for us.

    bool hmac_error = false;
    if
    (
        has_hmac_ue
        &&
        has_hmac_hash
        &&
        reply->error() == QNetworkReply::NoError
        &&
        http_status_code.isValid()
        &&
        http_status_code.toInt() != HMAC_HTTP_STATUS_RESPONSE_ON_ERROR
    )
    {//Only look at the response if there is no error

        HMAC hmac(env->ds.hardwareInfo->get_General_SerialNumber());
        QString rx_hmac_at = reply->rawHeader(HTTP_HMAC_UE_HEADER_UPPER);
        QString rx_hmac_hash = reply->rawHeader(HTTP_HMAC_HASH_HEADER_UPPER);
        QString body = actStatus.reply;
        auto status = QString::number(http_status_code.toInt());
        auto computed_hmac_hash = hmac.getResponseHash(status, body, rx_hmac_at);
        auto url = reply->url().toString();
        bool ignored_time_diff = url.endsWith("/profile") || url.endsWith("/digests") || url.endsWith("/digest") || url.endsWith("/versions");
        hmac_error = !hmac.validateHMAC(rx_hmac_hash, rx_hmac_at, computed_hmac_hash, now_ue, ignored_time_diff);
        if (hmac_error)
        {
            LOG_ERROR
            (
                "CRU_HMAC mismatched %s\nbody=%s\n  at=%s\n  rx=%s\n cal=%s\n",
                actStatus.request.CSTR(),
                body.CSTR(),
                rx_hmac_at.CSTR(),
                rx_hmac_hash.CSTR(),
                computed_hmac_hash.CSTR()
            );
        }
    }
    else
    {
        hmac_error = true;
    }

    if (reply->error() == QNetworkReply::NoError && (hmac_error == false))
    {
        actStatus.state = DS_ACTION_STATE_COMPLETED;
        actStatus.err = "";
        LOG_DEBUG("MSG_COMM: ACTION_STATUS: %s\n", ImrParser::ToImr_DataServiceActionStatusStr(actStatus).CSTR());
    }
    else
    {

        actStatus.state = DS_ACTION_STATE_INTERNAL_ERR;
        if (reply->error() != QNetworkReply::NoError)
        {
            actStatus.err = reply->errorString();
        }
        else
        {
            //todo why bother - if network attack and we raise internal error - does it make sense?
            // can we ignore it - does it become an type of DOS attack?
            actStatus.err = "HMAC Validation Error";
        }

        //Verify that the request is one that the user initiated (aka not one that occurs in the background).
        if ( !(actStatus.request.contains(IMR_CRU_URL_CONFIGS) ||
               actStatus.request.contains(IMR_CRU_URL_START_EXAM) ||
               actStatus.request.contains(IMR_CRU_URL_END_EXAM) ||
               actStatus.request.contains(IMR_CRU_URL_APPLY_LIMITS) ||
               actStatus.request.contains(IMR_CRU_URL_QUERY_WORKLIST) ||
               actStatus.request.contains(IMR_CRU_URL_SELECT_WORKLIST_ENTRY) ||
               actStatus.request.contains(IMR_CRU_URL_UPDATE_EXAM_FIELD) ||
               actStatus.request.contains(IMR_CRU_URL_UPDATE_EXAM_FIELD_PARAMETER) ||
               actStatus.request.contains(IMR_CRU_URL_UPDATE_INJECTION_PARAMTER) ||
               actStatus.request.contains(IMR_CRU_URL_UPDATE_LINKED_ACCESSION)) )
        {
            // Don't show popup errors for IMR requests which are not the ones specifed above.
            LOG_WARNING("MSG_COMM: ACTION_STATUS: %s\n", ImrParser::ToImr_DataServiceActionStatusStr(actStatus).CSTR());
        }
        else if ( (actStatus.err.contains("Connection refused")) || (actStatus.err.contains("Network unreachable")) )
        {
            // Don't show popup errors for responses due to comm being down.
            LOG_WARNING("MSG_COMM: ACTION_STATUS: %s\n", ImrParser::ToImr_DataServiceActionStatusStr(actStatus).CSTR());
        }
        else
        {
            LOG_ERROR("MSG_COMM: ACTION_STATUS: %s\n", ImrParser::ToImr_DataServiceActionStatusStr(actStatus).CSTR());

            // Note for future enhancement - the reply from the CRU is larger than desired; it contains the html
            // header with unnecessary style for the body. Investigation is occuring on the CRU side to see if
            // this code can change to "actStatus.err", or for the html data to be smaller.
            // Since this case is rare (CRU returning Bad Request), the implementation will be kept this way for now.
            const QString &errCheckString = actStatus.reply;
            QString err = "";

            int index = errCheckString.indexOf("T_EXAMOPERATIONFAILED");
            if (index != -1)
            {
                err = "T_EXAMOPERATIONFAILED";

                // check for further detail
                if (errCheckString.indexOf("_ExamDoesNotExist", index) != -1)
                {
                    err.append("_ExamDoesNotExist");
                }
            }
            else
            {
                err = actStatus.err;
            }

            env->ds.alertAction->activate("HCUSoftwareError", err);
        }
    }

    emit env->ds.cruAction->signalRxMsg(guid, actStatus);
}

void CruLinkMsgComm::slotRxTimeout()
{
    tmrRxTimeout.stop();

    actStatus.state = DS_ACTION_STATE_TIMEOUT;
    actStatus.err = "CRU TIMEOUT";

    LOG_ERROR("MSG_COMM: ACTION_STATUS: %s, timeout=%dms\n", ImrParser::ToImr_DataServiceActionStatusStr(actStatus).CSTR(), timeoutMs);

    emit env->ds.cruAction->signalRxMsg(guid, actStatus);
}
