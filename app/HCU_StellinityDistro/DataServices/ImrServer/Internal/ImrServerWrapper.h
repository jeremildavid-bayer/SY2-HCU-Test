#ifndef IMR_SERVER_WRAPPER_H
#define IMR_SERVER_WRAPPER_H

#include "httpserver/httprequesthandler.h"
#include "httpserver/httplistener.h"
#include "ImrServerBase.h"
#include "Common/Common.h"
#include "Common/crossthreadjob.h"


using namespace stefanfrings;

// The request mapper dispatches incoming HTTP requests to controller classes depending on the requested path.
class ImrServerWrapper : public HttpRequestHandler
{
    Q_OBJECT
public:
    ImrServerWrapper(QObject *parent = 0, EnvGlobal *env = NULL);
    ~ImrServerWrapper();
    void service(HttpRequest &request, HttpResponse &response);

private:
    EnvGlobal *env;
    EnvLocal *envLocal;
    HttpListener *listener;
    QSettings *settings;
    QList<ImrServerBase*> servers;
    CrossThreadJob *crossThreadJob;

    //cache capabilities
    bool dosAttackMonitorEnabled;
    bool logHcuDigestEnabled;
    int bodyDumpLimit;

    void updateCruLinkStatus(const HttpRequest &request);
    QString dosAttackCheck(const HttpRequest &request);
    void dosAttackPostProcess(const HttpRequest &request);
    void requestPreProcess(const HttpRequest &request, HttpResponse &response, const ImrServerBase::Params &params);
    void requestPostProcess(const HttpRequest &request, HttpResponse &response, const ImrServerBase::Params &params);
    bool validateHmacRequest(const HttpRequest &request, HttpResponse &response, ImrServerBase::Params &params);
    QString getRequestParametersString(const HttpRequest &request) const;

    static ParamsResponse doServiceOnMain(void *context, const ParamsRequest &request);
private slots:
    void slotAppInitialised();
};

#endif // IMR_SERVER_WRAPPER_H
