#ifndef CRU_LINK_H
#define CRU_LINK_H

#include <QObject>
#include <QTimer>
#include "Common/Common.h"
#include "DataServices/System/DS_SystemDef.h"

class CruLink : public QObject
{
    Q_OBJECT
public:
    explicit CruLink(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~CruLink();

private:
    enum State
    {
        STATE_IDLE = 0,

        STATE_CONNECTING,
        STATE_INCOMING_REQUEST_WAITING,

        STATE_REQ_PROFILE_STARTED,
        STATE_REQ_PROFILE_PROGRESS,
        STATE_REQ_PROFILE_DONE,

        STATE_HEART_BEATING
    };

    EnvGlobal *env;
    EnvLocal *envLocal;
    State state;
    QTimer tmrUpdateInjectionPlanData;
    int rxRetryCount;

    void reset();
    void setState(State state_, int waitTimeMs = PROCESS_STATE_TRANSITION_DELAY_MS);
    void processState();

private slots:
    void slotStart();
    void slotUpdateInjectionPlanData();
};

#endif // CRU_LINK_H
