#ifndef APP_MANAGER_H
#define APP_MANAGER_H

#include <QObject>
#include <QProcess>

#include "Common/Common.h"
#include "DataServices/Mcu/QML_Mcu.h"
#include "DataServices/Mcu/QML_McuSim.h"
#include "DataServices/Cru/QML_Cru.h"
#include "DataServices/Mwl/QML_Mwl.h"
#include "DataServices/Exam/QML_Exam.h"
#include "DataServices/Workflow/QML_Workflow.h"
#include "DataServices/Device/QML_Device.h"
#include "DataServices/System/QML_System.h"
#include "DataServices/Test/QML_Test.h"
#include "DataServices/Alert/QML_Alert.h"
#include "DataServices/Upgrade/QML_Upgrade.h"
#include "DataServices/HardwareInfo/QML_HardwareInfo.h"
#include "DataServices/CfgLocal/QML_CfgLocal.h"
#include "DataServices/CfgGlobal/QML_CfgGlobal.h"
#include "DataServices/Capabilities/QML_Capabilities.h"
#include "DataServices/ImrServer/QML_ImrServer.h"
#include "DataServices/DevTools/QML_DevTools.h"

class AppManager : public QObject
{
    Q_OBJECT
public:
    explicit AppManager(QObject *parent = 0);
    ~AppManager();
    bool initQmlApp();

    Q_INVOKABLE void slotExit();
    Q_INVOKABLE void slotScreenShot();
    Q_INVOKABLE void slotMultipleScreenShots();

private:
    EnvGlobal *env; // Environment Dataset for all apps
    EnvLocal *envLocal; // Environment Dataset for current app

    QProcess *procStartVideo;

    QML_Mcu *qmlMcu;
    QML_McuSim *qmlMcuSim;
    QML_Cru *qmlCru;
    QML_Mwl *qmlMwl;
    QML_Exam *qmlExam;
    QML_Workflow *qmlWorkflow;
    QML_Device *qmlDevice;
    QML_System *qmlSystem;
    QML_Test *qmlTest;
    QML_Alert *qmlAlert;
    QML_ImrServer *qmlImrServer;
    QML_Upgrade *qmlUpgrade;
    QML_HardwareInfo *qmlHardwareInfo;
    QML_CfgLocal *qmlCfgLocal;
    QML_CfgGlobal *qmlCfgGlobal;
    QML_Capabilities *qmlCfgCapabilities;
    QML_DevTools *qmlDevTools;

signals:
    void signalAppInitialised();
    void signalAppStarted();

private slots:
    void slotContinueLoading();
    void slotShowUi();
};

#endif // APP_MANAGER_H
