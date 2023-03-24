#ifndef CROSSTHREADJOB_H
#define CROSSTHREADJOB_H

#include <QObject>
#include <QMap>
#include <QMutex>

typedef struct ParamsRequest ParamsRequest;
typedef struct ParamsResponse ParamsResponse;

typedef ParamsResponse (*ExecuteFunctionPointer)(void *functionContext, const ParamsRequest &request);
Q_DECLARE_METATYPE(ExecuteFunctionPointer);
struct ParamsRequest
{
    QMultiMap<QByteArray,QByteArray> parameters;

    QString requestBody;
    QString requestPathLowerCase;
    QString requestMethodLowerCase;
    QString reqId;
    QString apiName;
};
Q_DECLARE_METATYPE(ParamsRequest);

struct ParamsResponse
{
    QString responseBody;
    bool responseMinified;
    bool responseCompressed;    
    int responseStatusCode;
    int responseCompressedLen;
    QByteArray responseStatusDescription;
};

Q_DECLARE_METATYPE(ParamsResponse);

class CrossThreadJob : public QObject
{
    Q_OBJECT
public:

    explicit CrossThreadJob(QObject *parent = 0) : QObject(parent), executedThread(nullptr){}
    ParamsResponse performAction(ParamsRequest &request,void *context, ExecuteFunctionPointer actionStartCb);
    bool subscribeExecutingThread(QObject *executedThread_);

    static void registerDataTypesForThread()
    {
        qRegisterMetaType<ParamsRequest>();
        qRegisterMetaType<ParamsResponse>();
        qRegisterMetaType<ExecuteFunctionPointer>();
    }

    int newJobId()
    {
        int ret;
        jobIdMutex.lock();
        ret = jobId;
        jobId++;
        jobIdMutex.unlock();
        return ret;
    }

signals:
    void signalPerformAction(int jobId, ExecuteFunctionPointer functionPointer, void *functionContext, QMutex*mutextPtr, const ParamsRequest &request);

    //not used too hard so use mutex instead
    //void signalActionComplete(int jobId, const ParamsResponse &response);

    //expected to be handle by the main thread to clean up the response
    void signalCompleteAction(int jobId);

private:
    QObject *executedThread;

    QMutex responseMapMutex;
    QMap<int, ParamsResponse> responseMap;

    QMutex jobIdMutex;
    int jobId = 0;
};


#endif // CROSSTHREADJOB_H
