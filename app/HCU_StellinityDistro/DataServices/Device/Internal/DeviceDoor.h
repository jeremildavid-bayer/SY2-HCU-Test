#ifndef DEVICE_DOOR_H
#define DEVICE_DOOR_H

#include <QObject>
#include "Common/Common.h"
#include "Common/ActionBaseExt.h"
#include "DataServices/Mcu/DS_McuDef.h"

class DeviceDoor : public ActionBaseExt
{
    Q_OBJECT
public:
    explicit DeviceDoor(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~DeviceDoor();

    
private:
    enum State
    {
        STATE_UNKNOWN = 0,

        STATE_INACTIVE,
        STATE_INIT
    };

    QString curMoodLightColorData;
    QTimer tmrBackgroundMoodLightPlay;
    DS_McuDef::ActLedParamsList backgroundMoodLightSequenceItems;
    int backgroundMoodLightSequenceIdx;

    bool isSetStateReady();
    void processState();
    void setMoodLight();

    void processBackgroundMoodLight();
    void backgroundMoodLightStart();
    void backgroundMoodLightStop();
    void copySudsLeds();

private slots:
    void slotAppInitialised();
    void slotBackgroundMoodLightPlay();
};

#endif // DEVICE_DOOR_H
