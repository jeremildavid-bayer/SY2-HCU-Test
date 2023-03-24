#ifndef MCU_SIM_SYRINGE_GROUP_H
#define MCU_SIM_SYRINGE_GROUP_H

class McuSimSyringe;
class McuSimStopcock;

struct McuSimSyringeGroup
{
    McuSimSyringe *syringe;
    McuSimStopcock *stopCock;
    bool airDetected;
};

#endif // MCU_SIM_SYRINGE_GROUP_H
