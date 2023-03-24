#include "QML_ImrServer.h"
#include "DataServices/ImrServer/DS_ImrServerData.h"
#include "DataServices/ImrServer/DS_ImrServerAction.h"
#include "Common/ImrParser.h"

QML_ImrServer::QML_ImrServer(QObject *parent, EnvGlobal *env_) :
    QObject(parent),
    env(env_)
{
    envLocal = new EnvLocal("QML_ImrServer", "QML_IMR_SERVER");
    qmlSrc = env->qml.object->findChild<QObject*>("dsImrServer");
    env->qml.engine->rootContext()->setContextProperty("dsImrServerCpp", this);

    if (qmlSrc == NULL)
    {
        LOG_ERROR("Failed to assign qmlSrc.\n");
    }

    // Register Data Callbacks
    connect(env->ds.imrServerData, &DS_ImrServerData::signalDataChanged_ClientCount, this, [=](const int &clientCount){
        qmlSrc->setProperty("clientCount", clientCount);
    });
}

QML_ImrServer::~QML_ImrServer()
{
    delete envLocal;
}

