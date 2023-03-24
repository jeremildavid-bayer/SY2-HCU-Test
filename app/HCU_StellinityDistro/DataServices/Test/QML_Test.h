#ifndef QML_TEST_H
#define QML_TEST_H

#include "Common/Common.h"
#include "DataServices/Test/DS_TestDef.h"

class QML_Test : public QObject
{
    Q_OBJECT
public:
    explicit QML_Test(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~QML_Test();

    Q_INVOKABLE void slotTestStart(QString testName, QVariant testParams);
    Q_INVOKABLE void slotTestStop();

private:
    EnvGlobal *env;
    EnvLocal *envLocal;
    QObject *qmlSrc;
};

#endif // QML_TEST_H
