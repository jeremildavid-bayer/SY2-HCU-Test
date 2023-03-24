#include "crossthreadjob.h"
#include <QDebug>
#include <QThread>
#include <QEventLoop>
#include <QMutexLocker>

/**
 * This function should be called once on startup where the main thread is specifed
 * @brief CrossThreadJob::subscribeExecutingThread
 * @param executedThread_: the thread object than perform a task (e.g. the main application task).
 * @return
 */
bool CrossThreadJob::subscribeExecutingThread(QObject *executedThread_)
{
    //executedThread = executedThread_; //need?
    if (executedThread != nullptr)
    {//the subscribed thread has been accept. Cant do it anymore
        qDebug() << "ERROR: thread has been subscribed";
        return false;
    }
    executedThread = executedThread_;

    connect(this,  &CrossThreadJob::signalPerformAction, executedThread_,  [=](int peformedJobId, ExecuteFunctionPointer functionPointer, void *functionContext, QMutex *mutextPtr, const ParamsRequest &request){
        ParamsResponse response;

        //call the callback now
        response = functionPointer(functionContext, request);

        responseMapMutex.lock();
        responseMap.insert(peformedJobId, response);
        responseMapMutex.unlock();

        // Use signal in threadpool causing deadlock. This needs further investigation if complete-signal is used for notification.
        //    emit signalActionComplete(peformedJobId, response); --> not working!
        // So the mutex is use for synchronisation with the response map instead.
        if (mutextPtr != nullptr)
            mutextPtr->unlock();

        //qDebug() << QThread::currentThreadId() << "slotPerformAction emit" << peformedJobId << response.responseBody;
    }, Qt::QueuedConnection);
    return true;
}

/**
 * This function transfer the runnting task to perform action
 * on the thread that has been volunteer to run the task (subscribeExecutingThread)
 * then wait until the action has been performed.
 *
 * The purpose is to ensure an single-thread app to to interact with other threads
 * through executing the code in the single-thread context hence thread-safe.
 *
 * Particularly QtWebapp request is comming in threads and want to interact
 * with the main app.
 *
 *     job->performAction(this, [](void *context) {
 *        qDebug() << QThread::currentThreadId() << "lambda function";
 *   });
 *
 * @brief CrossThreadJob::performAction
 * @param context
 * @param actionCb
 */
ParamsResponse CrossThreadJob::performAction(ParamsRequest &request, void *context, ExecuteFunctionPointer actionCb)
{
    ParamsResponse my_response;
    int jobId = this->newJobId();
    if (executedThread == nullptr)
    {//there is no thread to run the task, it would be a deadlock to continue
        qDebug() << "ERROR: no-thread has been subscribed";
        return my_response;
    }
    QMutex action_mutex;

    action_mutex.lock(); //first aqquire the lock

    // This is in a thread
    //qDebug() << QThread::currentThreadId() << "performAction start" << request.apiName;

    //signal the thread that subscribed to perform the action, use action_mutex as a context
    emit this->signalPerformAction(jobId, actionCb, context, &action_mutex, request);

    action_mutex.lock(); //self-locking until the main thread release
    action_mutex.unlock();//to avoid warning.


    responseMapMutex.lock();
    auto it = responseMap.find(jobId);
    if (it != responseMap.end())
    {
        //copy the response from the other main -- safe??
        my_response = it.value();
        responseMapMutex.unlock();
        emit this->signalCompleteAction(jobId); //let the the main thread release the memory
    }
    else
    {
        responseMapMutex.unlock();
        qDebug() << QThread::currentThreadId() <<  "ERROR: no actionperformAction" << jobId;
    }

    //qDebug() << QThread::currentThreadId() << "performAction end" << jobId << my_response.responseBody;
    return my_response;
}


