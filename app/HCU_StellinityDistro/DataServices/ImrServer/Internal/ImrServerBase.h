#ifndef IMR_SERVER_BASE_H
#define IMR_SERVER_BASE_H

#include "Common/Common.h"
#include "Common/Util.h"
#include "httpserver/httprequesthandler.h"
#include "DataServices/Mcu/DS_McuData.h"
#include "DataServices/Cru/DS_CruAction.h"
#include "DataServices/Cru/DS_CruData.h"
#include "DataServices/ImrServer/DS_ImrServerData.h"
#include "DataServices/Mwl/DS_MwlData.h"
#include "DataServices/Device/DS_DeviceData.h"
#include "DataServices/Exam/DS_ExamData.h"
#include "DataServices/Exam/DS_ExamAction.h"
#include "DataServices/Alert/DS_AlertAction.h"
#include "DataServices/Alert/DS_AlertData.h"
#include "DataServices/Workflow/DS_WorkflowData.h"
#include "DataServices/System/DS_SystemData.h"
#include "DataServices/Upgrade/DS_UpgradeData.h"
#include "DataServices/Test/DS_TestData.h"
#include "DataServices/HardwareInfo/DS_HardwareInfo.h"
#include "DataServices/CfgGlobal/DS_CfgGlobal.h"
#include "DataServices/CfgLocal/DS_CfgLocal.h"
#include "DataServices/Capabilities/DS_Capabilities.h"
#include "Common/HMAC.h"
#include "Common/crossthreadjob.h"

#define EXCEPTION_STR_HEAD              "<Fault xmlns=\"http://schemas.microsoft.com/ws/2005/05/envelope/none\"><Code><Value>Sender</Value></Code><Reason><Text xml:lang=\"en-US\"> "
#define EXCEPTION_STR_TAIL              " </Text></Reason></Fault>"

#define HTTP_STATUS_ENTITY_TOO_LARGE    413
#define HTTP_STATUS_BAD_REQUEST         400
#define HTTP_STATUS_CONFLICT            409
#define HTTP_STATUS_NOT_IMPLEMENTED     501

using namespace stefanfrings;

class ImrServerBase : public QObject
{
    Q_OBJECT

public:
    struct Params
    {
        ParamsRequest request;
        ParamsResponse response;
        /** Parameters of the request */
#if 0
        QMultiMap<QByteArray,QByteArray> parameters;

        QString requestBody;
        QString requestPathLowerCase;
        QString requestMethodLowerCase;
        QString reqId;
        QString apiName;
        QString responseBody;
        bool responseMinified;
        bool responseCompressed;
        int responseCompressedLen;
        int responseStatusCode;
        QByteArray responseStatusDescription;
#endif
        Params(const ParamsRequest &request_)
        {
            request = request_;
            response.responseBody = "";
            response.responseMinified = false;
            response.responseCompressed = false;
            response.responseStatusCode = 200; //http OK;
            response.responseCompressedLen = 0;
        }

        Params(const HttpRequest &request_, HttpResponse *_httpResponse)
        {
            request.reqId = "";
            request.apiName = "";
            response.responseBody = "";
            response.responseMinified = false;
            response.responseCompressed = false;
            response.responseStatusCode = 200; //http OK;
            response.responseCompressedLen = 0;

            /**
             * Since http request and repsonse cause memory corruption when using across thread.
             * Let try to get everything we need on the thread before passing to the main thread for processing.
             */

            request.requestPathLowerCase = request_.getPath().toLower();
            request.requestMethodLowerCase = request_.getMethod().toLower();

            auto paramMap = request_.getParameterMap();
            auto i = paramMap.begin();
            //deep copy of the parameters
            while (i != paramMap.end())
            {
                request.parameters.insert(i.key(), i.value());
                i++;
            }
            request.requestBody = request_.getBody();
        }

        QByteArray getParameter(const QByteArray& name) const
        {
            return request.parameters.value(name);
        }

        void setStatus(const int statusCode, const QString &description)
        {
            response.responseStatusCode = statusCode;
            response.responseStatusDescription = description.toLocal8Bit();
        }
    };

    explicit ImrServerBase(QObject *parent = 0, QString baseUrl_ = "", EnvGlobal *env_ = NULL, EnvLocal *envLocal_ = NULL):
        QObject(parent),
        baseUrl(baseUrl_),
        env(env_),
        envLocal(envLocal_)
    {
        CrossThreadJob::registerDataTypesForThread();
        ActionManager::registerDataTypesForThread();
        DS_WorkflowData::registerDataTypesForThread();
        DS_DeviceData::registerDataTypesForThread();
        DS_ExamData::registerDataTypesForThread();
        DS_McuData::registerDataTypesForThread();
        DS_CruData::registerDataTypesForThread();
        DS_MwlData::registerDataTypesForThread();
        DS_ImrServerData::registerDataTypesForThread();
        DS_AlertData::registerDataTypesForThread();
        DS_UpgradeData::registerDataTypesForThread();
        DS_TestData::registerDataTypesForThread();
        DS_SystemData::registerDataTypesForThread();
        DS_CfgLocal::registerDataTypesForThread();
        DS_CfgGlobal::registerDataTypesForThread();
        DS_Capabilities::registerDataTypesForThread();
        DS_HardwareInfo::registerDataTypesForThread();
    }

    ~ImrServerBase()
    {
    }

    virtual void ImrService(Params &params) = 0;

    QString getBaseUrl() { return baseUrl; }

    QString getRequestDump(const HttpRequest &request, int bodyDumpLimit = 80)
    {
        QString body = request.getBody();
        QString shortBody = body;
        if (body.length() > bodyDumpLimit)
        {
            shortBody = body.left(bodyDumpLimit) + "...";
        }
        shortBody.replace("\\r", "").replace("\\n", "");

        QString method = request.getMethod();
        QString params;
        QMultiMap<QByteArray, QByteArray> paramMap = request.getParameterMap();
        QMultiMap<QByteArray, QByteArray>::const_iterator i = paramMap.cbegin();
        while (i != paramMap.cend())
        {
            QByteArray key = i.key();
            QList<QByteArray> values = paramMap.values(key);
            if (params.length() > 0)
            {
                params += ",";
            }
            params += QString().asprintf("%s:%s", key.CSTR(), values.join(",").CSTR());
            i++;
        }

        QString peerAddress = request.getPeerAddress().toString();
        peerAddress.replace("::ffff:", "");

        QString ret = QString().asprintf("%s: SRC=%s%s PRMS{%s} BDY[%d]={%s}", method.CSTR(), peerAddress.CSTR(), request.getPath().CSTR(), params.CSTR(), (int)body.length(), shortBody.CSTR());

        return ret;
    }

    void sendOK(Params &params)
    {
        params.response.responseBody = "OK";
        sendResponseBody(params);
    }

    void sendResponseBody(Params &params)
    {//this is for uncompressed case only
        params.response.responseCompressed = false;
    }

    void updateHeaderAndSendDelayRespose(Params &params, HttpResponse &httpResponse)
    {
        if (!params.response.responseCompressed)
        {
            updateHmacResposeHeader(params, httpResponse);
            httpResponse.write(params.response.responseBody.toLocal8Bit());
        }
        else
        {
            // Compress json response as it can be quite large
            QByteArray bodyCompressed = compressHttpResponse(httpResponse, params.response.responseBody.CSTR());
            params.response.responseCompressedLen = bodyCompressed.length();

            updateHmacResposeHeader(params, httpResponse);
            httpResponse.write(bodyCompressed);
        }
    }

    void sendFault(Params &params, int httpState, QString description, QString threadId = Util::curThreadId())
    {
        description = QString().asprintf("HTTP STATUS %d: ", httpState) + description;
        LOG_WARNING("%s: REQ[%s]: TX: %s\n", threadId.CSTR(), params.request.reqId.CSTR(), description.CSTR());

        //don't call httpresponse->setStatus() directly due to thread-safety
        params.setStatus(httpState, description.toLocal8Bit());

        params.response.responseBody = QString(EXCEPTION_STR_HEAD) + description + QString(EXCEPTION_STR_TAIL);
        sendResponseBody(params);
    }

    void sendResponseWithJsonData(Params &params, const QVariant &data, bool minify = true)
    {
        params.response.responseMinified = minify;
        params.response.responseBody = Util::qVarientToJsonData(data, minify);
        params.response.responseCompressed = true; //will request for compression when sending
    }

protected:
    void updateHmacResposeHeader(Params &params, HttpResponse &httpResponse)
    {//caculated HMMAC using un-compressed response body.
        HMAC hmac(env->ds.hardwareInfo->get_General_SerialNumber());

        if (params.response.responseStatusDescription.length() > 0)
             httpResponse.setStatus(params.response.responseStatusCode, params.response.responseStatusDescription);

        auto hmac_ue = QString::number(QDateTime::currentSecsSinceEpoch());
        auto status = QString::number(httpResponse.getStatusCode());
        auto hmac_value = hmac.getResponseHash(status, params.response.responseBody, hmac_ue);

        httpResponse.setHeader(QByteArray(HTTP_HMAC_UE_HEADER_UPPER), hmac_ue.toLocal8Bit());
        httpResponse.setHeader(QByteArray(HTTP_HMAC_HASH_HEADER_UPPER), hmac_value.toLocal8Bit());
    }

    QByteArray compressHttpResponse(HttpResponse &response, const QByteArray& data)
    {
        // Compression is not needed for small bytes of data (and in fact may cause unnecessary additional overhead)
        if (data.length() < 1000)
            return data;

        // qCompress uses zlib which uses deflate compression
        response.setHeader("Content-Encoding", "deflate");

        QByteArray compressedData = qCompress(data);

        // Remove 4-byte length put on by qCompress and 2-byte zlib header, which returns "deflate" data only
        compressedData.remove(0, 6);

        // Comment out debug logging...
        // Example log: Full=64307 bytes, Compressed=3771 bytes, Saved=60536 bytes 94.14%
        //int savedBytes = data.length() - compressedData.length();
        //double savedPercentage = (double)savedBytes / data.length() * 100;
        //if (savedBytes > 0)
        //{
        //    LOG_DEBUG("compressHttpResponse savings: Full=%d bytes, Compressed=%d bytes, Saved=%d bytes %.2f%\n", data.length(), compressedData.length(), savedBytes, savedPercentage);
        //}

        return compressedData;
    }


    QString baseUrl;
    EnvGlobal *env;
    EnvLocal *envLocal;
};

#endif // IMR_SERVER_BASE_H
