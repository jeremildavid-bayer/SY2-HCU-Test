#ifndef DEVICE_LEDS_H
#define DEVICE_LEDS_H

#include <QTimer>
#include "Common/Common.h"
#include "Common/ActionBase.h"
#include "DataServices/Device/DS_DeviceDef.h"

class DeviceLeds : public ActionBase
{
    Q_OBJECT
public:
    explicit DeviceLeds(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~DeviceLeds();

    DataServiceActionStatus actLeds(LedIndex index, DS_McuDef::ActLedParams param, QString actGuid = "");
    DataServiceActionStatus actCopyLeds(LedIndex from, LedIndex to, QString actGuid = "");

private:
    QTimer tmrFlash;
    bool flashFlag;
    DS_McuDef::ActLedParamsList curLedParamsList;

private slots:
    void slotFlash();
    void slotAppInitialised();

};

#endif // DEVICE_LEDS_H
