#ifndef MCU_SIM_CONTROL_GROUP_H
#define MCU_SIM_CONTROL_GROUP_H

#include "DataServices/Mcu/DS_McuDef.h"

class McuSimSyringe;
class McuSimStopcock;

struct SimHwState
{
    McuSimSyringe *syringe;
    McuSimStopcock *stopCock;
    bool airDetected;
};

#endif // MCU_SIM_CONTROL_GROUP_H
