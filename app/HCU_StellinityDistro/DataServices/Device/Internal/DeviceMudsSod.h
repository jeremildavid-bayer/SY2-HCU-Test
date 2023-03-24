#ifndef DEVICE_MUDS_SOD_H
#define DEVICE_MUDS_SOD_H

#include "Common/ActionBaseExt.h"
#include "DataServices/Device/DS_DeviceDef.h"

class DeviceMudsSod : public ActionBaseExt
{
    Q_OBJECT
public:
    explicit DeviceMudsSod(QObject *parent = 0, EnvGlobal *env_ = NULL, EnvLocal *envLocal_ = NULL);
    ~DeviceMudsSod();

    DataServiceActionStatus actSodStart(QString actGuid = "");
    DataServiceActionStatus actSodAbort(QString actGuid = "");

private:
    SyringeIdx syringeIdxSodStart;

    QString guidSyringesIdleWaiting;
    QString guidSalineVolWaiting;
    QString guidSudsWaiting;
    QString guidOutletDoorWaiting;
    QString guidWasteContainerWaiting;

    QList<SyringeIdx> sodPerformedSyringes;

    int getState();
    QString getStateStr(int state);
    void setStateSynch(int newState);
    bool isSetStateReady();
    void processState();

private slots:
    void slotAppInitialised();
};

#endif // DEVICE_MUDS_SOD_H
