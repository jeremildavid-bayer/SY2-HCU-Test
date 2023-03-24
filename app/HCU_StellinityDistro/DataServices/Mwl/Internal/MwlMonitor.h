#ifndef MWL_MONITOR_H
#define MWL_MONITOR_H

#include <QObject>
#include <QTimer>
#include "Common/Common.h"

class MwlMonitor : public QObject
{
    Q_OBJECT
public:
    explicit MwlMonitor(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~MwlMonitor();

    void init();

private:
    EnvGlobal *env;
    EnvLocal *envLocal;
    bool isActive;

    void handleSuiteNameChanged();
    void handleCruLinkStateOrExamStateChanged();

private slots:
    void slotAppInitialised();


};

#endif // MWL_MONITOR_H
