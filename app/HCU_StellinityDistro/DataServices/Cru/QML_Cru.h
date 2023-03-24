#ifndef QML_CRU_H
#define QML_CRU_H

#include "Common/Common.h"
#include "DataServices/Cru/DS_CruDef.h"

class QML_Cru : public QObject
{
    Q_OBJECT
public:
    explicit QML_Cru(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~QML_Cru();

    Q_INVOKABLE void slotGetTestMessage();
    Q_INVOKABLE void slotPutTestMessage(int messageByteLen, int processingTimeMs);
    Q_INVOKABLE void slotPostUpdateInjectionParameter(QVariantMap param);
    Q_INVOKABLE void slotPostUpdateExamField(QVariantMap param);
    Q_INVOKABLE void slotPostUpdateExamFieldParameter(QVariantMap param);
    Q_INVOKABLE void slotPostUpdateLinkedAccession(QVariantMap entry, bool isLinked);
    Q_INVOKABLE void slotApplyLimits();

private:
    EnvGlobal *env;
    EnvLocal *envLocal;
    QObject *qmlSrc;

    void setAdvanceMode();

};

#endif // QML_CRU_H
