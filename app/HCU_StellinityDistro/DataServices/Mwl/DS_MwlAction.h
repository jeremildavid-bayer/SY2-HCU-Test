#ifndef DS_MWL_ACTION_H
#define DS_MWL_ACTION_H

#include "Common/Common.h"
#include "Common/Util.h"
#include "Common/ActionBase.h"
#include "DS_MwlDef.h"
#include "Internal/MwlMonitor.h"

class DS_MwlAction : public ActionBase
{
    Q_OBJECT
public:
    explicit DS_MwlAction(QObject *parent = 0, EnvGlobal *env = NULL);
    ~DS_MwlAction();

    DataServiceActionStatus actSelectWorklistEntry(QString studyInstanceUid, QString actGuid = "");
    DataServiceActionStatus actQueryWorklist(QString actGuid = "");

private:
    MwlMonitor *monitor;

private slots:
    void slotAppInitialised();

signals:

};

#endif // DS_MWL_ACTION_H
