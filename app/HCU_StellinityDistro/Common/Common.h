#ifndef COMMON_H
#define COMMON_H

#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QElapsedTimer>
#include "Log.h"
#include "Environment.h"

#define LOG_DEBUG(format, args...)      do { if (envLocal->log != NULL) { envLocal->log->writeDebug(QString().asprintf(format, ## args)); } \
                                             if (env->allLog != NULL) { env->allLog->writeDebug(QString().asprintf(format, ## args), (envLocal->log == NULL) ? "" : (envLocal->logPrefix + ": ")); } \
                                        }while(0)

#define LOG_INFO(format, args...)       do { if (envLocal->log != NULL) { envLocal->log->writeInfo(QString().asprintf(format, ## args)); } \
                                             if (env->allLog != NULL) { env->allLog->writeInfo(QString().asprintf(format, ## args), (envLocal->log == NULL) ? "" : (envLocal->logPrefix + ": ")); } \
                                         }while(0)

#define LOG_WARNING(format, args...)    do { if (envLocal->log != NULL) { envLocal->log->writeWarning(QString().asprintf(format, ## args)); } \
                                             if (env->errLog != NULL) { env->errLog->writeWarning(QString().asprintf(format, ## args), (envLocal->log == NULL) ? "" : (envLocal->logPrefix + ": ")); } \
                                             if (env->allLog != NULL) { env->allLog->writeWarning(QString().asprintf(format, ## args), (envLocal->log == NULL) ? "" : (envLocal->logPrefix + ": ")); } \
                                        }while(0)

#define LOG_ERROR(format, args...)      do { if (envLocal->log != NULL) { envLocal->log->writeError(QString().asprintf(format, ## args)); } \
                                             if (env->errLog != NULL) { env->errLog->writeError(QString().asprintf(format, ## args), (envLocal->log == NULL) ? "" : (envLocal->logPrefix + ": ")); } \
                                             if (env->allLog != NULL) { env->allLog->writeError(QString().asprintf(format, ## args), (envLocal->log == NULL) ? "" : (envLocal->logPrefix + ": ")); } \
                                        }while(0)

#define ARRAY_LEN(arr)                  (int)(sizeof(arr) / sizeof(arr[0]))
#define ARRAY_CMP(arr1, arr2)           (bool)((ARRAY_LEN(arr1) == ARRAY_LEN(arr2))&& (std::equal(std::begin(arr1), std::end(arr1), std::begin(arr2))))

#define _L(str)                         QLatin1String(str)
#define CSTR()                          toStdString().c_str()

//========================================================
// Data Structure - Action Status

enum DataServiceActionStateType
{
    DS_ACTION_STATE_UNKNOWN = 0,
    DS_ACTION_STATE_INIT,
    DS_ACTION_STATE_START_WAITING,
    DS_ACTION_STATE_STARTED, // action is started, caller needs to get the action status for completed status itself
    DS_ACTION_STATE_COMPLETED,
    DS_ACTION_STATE_INVALID_STATE,
    DS_ACTION_STATE_BUSY,
    DS_ACTION_STATE_BAD_REQUEST,
    DS_ACTION_STATE_TIMEOUT,
    DS_ACTION_STATE_INTERNAL_ERR,
    DS_ACTION_STATE_ALLSTOP_BTN_ABORT,
    DS_ACTION_STATE_USER_ABORT,
    DS_ACTION_STATE_NOT_IMPLEMENTED
};

struct DataServiceActionStatus
{
    DataServiceActionStateType state;
    QString guid;
    QString request;
    QString arg;
    QString arg2;
    QString reply;
    QString err;

    DataServiceActionStatus()
    {
        state = DS_ACTION_STATE_INIT;
        guid = "";
        request = "";
        arg = "";
        arg2 = "";
        reply = "";
        err = "";
    }

    DataServiceActionStatus(QString guid_, QString request_, QString arg_ = "", QString arg2_ = "")
    {
        state = DS_ACTION_STATE_INIT;
        guid = guid_;
        request = request_;
        arg = arg_;
        arg2 = arg2_;
        reply = "";
        err = "";
    }
};

Q_DECLARE_METATYPE(DataServiceActionStatus);


//========================================================
// Data Structure - Local Environment Descriptor

class EnvLocal
{
public:
    EnvLocal(QString moduleName_, QString logPrefix_, qint64 logSizeLimit = LOG_MIN_SIZE_BYTES) :
        moduleName(moduleName_),
        logPrefix(logPrefix_)
    {
        log = new Log(PATH_LOG_DIR + moduleName + ".log", logSizeLimit);
        log->open();

        log->writeInfo("\n\n\n");
        log->writeInfo("===========================================================\n");
        log->writeInfo(QString().asprintf("HCU_Stellinity Distro - %s Started..\n", moduleName.CSTR()));
    }

    ~EnvLocal()
    {
        delete log;
    }

    Log *log;
    QString moduleName;
    QString logPrefix;
};


//========================================================
// Data Structure - Global Environment Descriptor

class DS_WorkflowData;
class DS_WorkflowAction;

class DS_DeviceData;
class DS_DeviceAction;

class DS_ExamData;
class DS_ExamAction;

class DS_McuData;
class DS_McuAction;

class DS_CruData;
class DS_CruAction;

class DS_MwlData;
class DS_MwlAction;

class DS_ImrServerData;
class DS_ImrServerAction;

class DS_HardwareInfo;

class DS_CfgLocal;
class DS_CfgGlobal;
class DS_Capabilities;

class DS_SystemData;
class DS_SystemAction;

class DS_AlertData;
class DS_AlertAction;

class DS_UpgradeData;
class DS_UpgradeAction;

class DS_TestData;
class DS_TestAction;

class ImrParser;
class ActionManager;
class AppManager;
class Translator;

class EnvGlobal : public QObject
{
    Q_OBJECT
public:

    enum State
    {
        STATE_INIT = 0,
        STATE_INITIALISING_CFG_SERVICES,
        STATE_INITIALISING_DATA_SERVICES,
        STATE_INTRO_MEDIA_COMPLETE_WAITING,
        STATE_INITIALISING_QML_APP,
        STATE_INITIALISING_QML_SERVICES,
        STATE_INIT_COMPLETING,
        STATE_STARTING,
        STATE_RUNNING,
        STATE_EXITING,
    };

    explicit EnvGlobal(QObject *parent = 0) : QObject(parent)
    {
        state = STATE_INIT;
        humanUseAllowed = false;

        systemUpElapsedTimer.start();
        systemUpEpochMs = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
    }

    ~EnvGlobal()
    {
    }

    struct DataServices
    {
        DS_WorkflowData *workflowData;
        DS_WorkflowAction *workflowAction;
        DS_DeviceData *deviceData;
        DS_DeviceAction *deviceAction;
        DS_ExamData *examData;
        DS_ExamAction *examAction;
        DS_McuData *mcuData;
        DS_McuAction *mcuAction;
        DS_CruData *cruData;
        DS_CruAction *cruAction;
        DS_MwlData *mwlData;
        DS_MwlAction *mwlAction;
        DS_ImrServerData *imrServerData;
        DS_ImrServerAction *imrServerAction;
        DS_HardwareInfo *hardwareInfo;
        DS_CfgLocal *cfgLocal;
        DS_CfgGlobal *cfgGlobal;
        DS_Capabilities *capabilities;
        DS_SystemData *systemData;
        DS_SystemAction *systemAction;
        DS_AlertData *alertData;
        DS_AlertAction *alertAction;
        DS_UpgradeData *upgradeData;
        DS_UpgradeAction *upgradeAction;

        DS_TestData *testData;
        DS_TestAction *testAction;
    };

    struct QmlParams
    {
        QQmlEngine *engine;
        QQmlComponent *component;
        QObject *object;
        QQmlContext *context;
    };

    AppManager *appManager;
    DataServices ds;
    /**
     * @brief mutexLastImrCruRequest
     * This is to protect concurrent access of ds.imrServerData->getLastImrCruRequests()
     * between IMR server threads (QtWebApp thread pool)
     *
     * Since ds.imrServerData is not designed for thread safe and it is not feasible to
     * re-write it for threads-safe.
     * So let protected the identified known issues only.
     *
     * Everytime getLastImrCruRequests is called it must be protected this mutex mutexLastImrCruRequest
     * for example
     *     {//open the scope to release the mutex as soon as it out of scope
     *          QMutexLocker(mutexLastImrCruRequest)
     *          auto cru_requests = ds.imrServerData->getLastImrCruRequests()
     *          ...(modify cru_requests)...
     *     }
     */
    QMutex       mutexLastImrCruRequest;

    QmlParams qml;
    ActionManager *actionMgr;
    Log *errLog;
    Log *allLog;
    Translator *translator;
    State state;
    bool humanUseAllowed;

    QElapsedTimer systemUpElapsedTimer;
    qint64 systemUpEpochMs;

signals:
    void signalExitDistro();
    void signalTimeShifted(qint64 timeShiftedMs);
};


#endif // COMMON_H
