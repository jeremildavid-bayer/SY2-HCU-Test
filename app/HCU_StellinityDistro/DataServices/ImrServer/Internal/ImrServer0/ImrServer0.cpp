#include <QThread>
#include "ImrServer0.h"

ImrServer0::ImrServer0(QObject *parent, QString baseUrl, EnvGlobal *env_, EnvLocal *envLocal_):
    ImrServerBase(parent, baseUrl, env_, envLocal_)
{
    // IMR_VERSION string is "v7" need to remove "v"
    currentImrVersion = QString(IMR_VERSION).remove(0,1).toInt();
}

ImrServer0::~ImrServer0()
{
}

void ImrServer0::ImrService(Params &params)
{
    try
    {
        QString apiName = params.request.apiName;

        if (apiName.indexOf('?') > 0)
        {
            apiName = apiName.left(apiName.indexOf('?'));
        }

        // Handle apiName
        if (apiName == _L("api/description"))
        {
            rhApiDescription(params);
        }
        else if (apiName == _L("api/versions"))
        {
            rhApiVersions(params);
        }
        else if (apiName == _L("test/message"))
        {
            rhTestMessage(params);
        }
        else
        {
            // Unknown API received
            sendFault(params, HTTP_STATUS_NOT_IMPLEMENTED, "API not implemented: " + apiName);
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

void ImrServer0::rhApiDescription(Params &params)
{
    if (params.request.requestMethodLowerCase == _L("get"))
    {
        LOG_INFO("V0: GET API/DESCRIPTION\n");

        QFile fileBuf(QString(PATH_RESOURCES_DOC) + "/DescriptionV0.txt");
        if (fileBuf.open(QFile::ReadOnly | QFile::Text))
        {
            QByteArray jsonStr;
            params.response.responseBody = fileBuf.readAll();
            fileBuf.close();
            sendResponseBody(params);
        }
        else
        {
            sendFault(params, HTTP_STATUS_BAD_REQUEST, "Description file not found");
        }
    }
    else
    {
        sendFault(params, HTTP_STATUS_BAD_REQUEST, "Unsupported method type");
    }
}

void ImrServer0::rhApiVersions(Params &params)
{
    if (params.request.requestMethodLowerCase == _L("get"))
    {
        LOG_INFO("V0: GET API/VERSIONS\n");

        // Prepare version list: supported api versions in current application
        QVariantList apiVersions;
        apiVersions << 0 << currentImrVersion;
        params.response.responseBody = Util::qVarientToJsonData(apiVersions);
        sendResponseBody(params);
    }
    else
    {
        sendFault(params, HTTP_STATUS_BAD_REQUEST, "Unsupported method type");
    }
}

void ImrServer0::rhTestMessage(Params &params)
{
    if (params.request.requestMethodLowerCase == _L("get"))
    {
        LOG_INFO("V0: GET TEST_MSG\n");

        params.response.responseBody = testMsg.serialize();
        sendResponseBody(params);
    }
    else if (params.request.requestMethodLowerCase == _L("put"))
    {
        QString reqDataStr = params.request.requestBody;

        LOG_INFO("V0: PUT TEST_MSG: %s\n", reqDataStr.CSTR());

        // parse the request data json string into a testmessage object
        QJsonParseError err;
        testMsg.parse(reqDataStr, err);

        LOG_INFO("V0: PUT TEST_MSG: Requested at %s\n", testMsg.params["RequestAt"].toString().CSTR());

        if (err.error == QJsonParseError::NoError)
        {
            if (testMsg.params.contains("ProcessingMillis"))
            {
                // sleep for this long to simulate a processing delay.
                int sleepMs = testMsg.params["ProcessingMillis"].toInt();
                LOG_INFO("V0: PUT_TEST_MSG: Requested Wait: Sleeping for %lums..\n", (unsigned long)sleepMs);
                QThread::msleep((unsigned long)sleepMs);
            }
            testMsg.params["ResponseAt"] = QDateTime::currentDateTimeUtc();

            // serialize the test message object into a json string
            params.response.responseBody = testMsg.serialize();
            sendResponseBody(params);

            LOG_INFO("V0: PUT TEST_MSG: Output=%s\n", params.response.responseBody.toStdString().c_str());
        }
        else
        {
            LOG_ERROR("V0: Parsing err=%s\n", err.errorString().CSTR());
        }
    }
    else
    {
        sendFault(params, HTTP_STATUS_BAD_REQUEST, "Unsupported method type");
    }
}
