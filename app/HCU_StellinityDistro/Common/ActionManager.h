#ifndef ACTION_MANAGER_H
#define ACTION_MANAGER_H

#include <QMutex>
#include <QTimer>
#include <QDateTime>
#include "Common/Common.h"
#include "Common/Environment.h"
#include "DataServices/System/DS_SystemData.h"

class ActionManager : public QObject
{
    Q_OBJECT
public:
    ActionManager(QObject *parent, EnvGlobal *env_) :
        QObject(parent),
        env(env_)
    {
        envLocal = new EnvLocal("ActionManager", "ACT_MGR");
    }

    ~ActionManager()
    {
        delete envLocal;
    }

    static void registerDataTypesForThread()
    {
        qRegisterMetaType<DataServiceActionStatus>("DataServiceActionStatus");
    }


    template <typename LambdaFn>
    void onActionStarted(QString guid, QString actionName, LambdaFn cb)
    {
        if (guid == "")
        {
            return;
        }

        QMetaObject::Connection conn = connect(this, &ActionManager::signalActionStarted, [=](DataServiceActionStatus status) {
            if (status.guid == guid)
            {
                cb(status);
                handleActionStartedState(guid, status.state);
            }
        });
        createActStarted(guid, conn, actionName);
    }

    template <typename LambdaFn>
    void onActionCompleted(QString guid, QString actionName, LambdaFn cb)
    {
        if (guid == "")
        {
            return;
        }

        if (!isActExist(guid))
        {
            // actionStartedConnection is not created. Creating one now.
            QMetaObject::Connection conn = connect(this, &ActionManager::signalActionStarted, [=](DataServiceActionStatus status) {
                if (status.guid == guid)
                {
                    handleActionStartedState(guid, status.state);
                }
            });
            createActStarted(guid, conn, actionName);
        }

        QMetaObject::Connection conn = connect(this, &ActionManager::signalActionCompleted, [=](DataServiceActionStatus status) {
            if (status.guid == guid)
            {
                deleteActCompleted(guid);
                cb(status);
            }
        });
        createActCompleted(guid, conn, actionName);
    }

    void createActStarted(QString guid, QMetaObject::Connection connection, QString name = "")
    {
        mutexMapInfo.lock();
        if (!mapInfo.contains(guid))
        {
            ConnectionInfo info;
            info.guid = guid;
            info.name = name;
            info.createdEpochMs = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
            info.connActCompletedExist = false;
            mapInfo.insert(guid, info);
        }
        mapInfo[guid].connActStarted = connection;
        mapInfo[guid].connActStartedExist = true;
        mutexMapInfo.unlock();
    }

    void createActCompleted(QString guid, QMetaObject::Connection connection, QString actionName = "")
    {
        mutexMapInfo.lock();
        if (!mapInfo.contains(guid))
        {
            ConnectionInfo info;
            info.guid = guid;
            info.name = actionName;
            info.createdEpochMs = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
            info.connActStartedExist = false;
            mapInfo.insert(guid, info);
        }

        mapInfo[guid].connActCompleted = connection;
        mapInfo[guid].connActCompletedExist = true;
        mutexMapInfo.unlock();
    }

    void deleteActStarted(QString guid)
    {
        mutexMapInfo.lock();
        if (mapInfo.contains(guid))
        {
            QObject::disconnect(mapInfo[guid].connActStarted);
            mapInfo[guid].connActStartedExist = false;

            if (mapInfo[guid].connActCompletedExist)
            {
                // Connection for completed is still exist. don't delete connection yet.
            }
            else
            {
                mapInfo.remove(guid);
            }
        }
        mutexMapInfo.unlock();
    }

    void deleteActCompleted(QString guid)
    {
        mutexMapInfo.lock();
        if (mapInfo.contains(guid))
        {
            QObject::disconnect(mapInfo[guid].connActCompleted);
            mapInfo[guid].connActCompletedExist = false;

            if (mapInfo[guid].connActStartedExist)
            {
                QObject::disconnect(mapInfo[guid].connActStarted);
                mapInfo[guid].connActStartedExist = false;
            }

            mapInfo.remove(guid);
        }
        mutexMapInfo.unlock();
    }

    void deleteActAll(QString guid)
    {
        deleteActStarted(guid);
        deleteActCompleted(guid);
    }

    bool isActExist(QString guid)
    {
        mutexMapInfo.lock();
        bool isActExist = mapInfo.contains(guid);
        mutexMapInfo.unlock();

        return isActExist;
    }

    QString dump()
    {
        QString ret = "";

        mutexMapInfo.lock();
        int idx = 0;

        ret += QString().asprintf("Actions[%d]: [\n", mapInfo.size());
        foreach (ConnectionInfo info, mapInfo)
        {
            QString callbacksExist = "[ ";
            if (info.connActStartedExist)
            {
                callbacksExist += "Started ";
            }

            if (info.connActStartedExist)
            {
                callbacksExist += "Completed ";
            }
            callbacksExist += "]";

            ret += QString().asprintf("\tAction[%d]: %s: CreatedAt=%s, Name=%s, Callbacks=%s\n",
                                     idx++,
                                     info.guid.CSTR(),
                                     QDateTime::fromMSecsSinceEpoch(info.createdEpochMs).toString("MMdd-hh:mm:ss.zzz").CSTR(),
                                     info.name.CSTR(),
                                     callbacksExist.CSTR());
        }
        ret += "]\n";
        mutexMapInfo.unlock();

        return ret;
    }

private:
    struct ConnectionInfo
    {
        QString guid;
        QString name;
        qint64 createdEpochMs;
        bool connActStartedExist;
        bool connActCompletedExist;
        QMetaObject::Connection connActStarted;
        QMetaObject::Connection connActCompleted;
    };

    QMap<QString, ConnectionInfo> mapInfo;
    QMutex mutexMapInfo;
    EnvLocal *envLocal;
    EnvGlobal *env;

    void handleActionStartedState(QString guid, DataServiceActionStateType state)
    {
        if (state != DS_ACTION_STATE_START_WAITING)
        {
            deleteActStarted(guid);
        }

        if ( (state != DS_ACTION_STATE_START_WAITING) &&
             (state != DS_ACTION_STATE_STARTED) )
        {
            // action didn't start, delete completed cb
            deleteActCompleted(guid);
        }
    }

signals:
    void signalActionStarted(DataServiceActionStatus status);
    void signalActionCompleted(DataServiceActionStatus status);

};

#endif // ACTION_MANAGER_H
