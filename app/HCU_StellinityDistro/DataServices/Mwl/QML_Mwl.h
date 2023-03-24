#ifndef QML_MWL_H
#define QML_MWL_H

#include "Common/Common.h"
#include "DataServices/Mwl/DS_MwlDef.h"

class QML_Mwl : public QObject
{
    Q_OBJECT
public:
    explicit QML_Mwl(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~QML_Mwl();

    Q_INVOKABLE void slotPatientsReload();
    Q_INVOKABLE void slotSelectWorklistEntry(QString studyUid);
    Q_INVOKABLE void slotDeselectWorklistEntry();

private:
    EnvGlobal *env;
    EnvLocal *envLocal;
    QObject *qmlSrc;

};

#endif // QML_MWL_H
