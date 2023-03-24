#include "QML_Test.h"
#include "DataServices/Test/DS_TestData.h"
#include "DataServices/Test/DS_TestAction.h"
#include "Common/ImrParser.h"

QML_Test::QML_Test(QObject *parent, EnvGlobal *env_) :
    QObject(parent),
    env(env_)
{
    envLocal = new EnvLocal("QML_Test", "QML_TEST");
    qmlSrc = env->qml.object->findChild<QObject*>("dsTest");
    env->qml.engine->rootContext()->setContextProperty("dsTestCpp", this);

    if (qmlSrc == NULL)
    {
        LOG_ERROR("Failed to assign qmlSrc.\n");
    }

    // Register Data Callbacks
    connect(env->ds.testData, &DS_TestData::signalDataChanged_TestStatus, this, [=](const DS_TestDef::TestStatus &status) {
        qmlSrc->setProperty("testStatus", ImrParser::ToImr_TestStatus(status));
    });
}

QML_Test::~QML_Test()
{
    delete envLocal;
}

void QML_Test::slotTestStart(QString testName, QVariant testParams)
{
    env->ds.testAction->actStart(ImrParser::ToCpp_TestType(testName), testParams.toList());
}

void QML_Test::slotTestStop()
{
    env->ds.testAction->actStop();
}
