#ifndef DS_CFG_LOCAL_DEF_H
#define DS_CFG_LOCAL_DEF_H

#include <QString>
#include <QVariantList>

class DS_CfgLocalDef
{
public:
    enum SoundVolumeLevel
    {
        SOUND_VOLUME_LEVEL_MUTE = 0,
        SOUND_VOLUME_LEVEL_MIN = 1,
        SOUND_VOLUME_LEVEL_DEFAULT = 3,
        SOUND_VOLUME_LEVEL_MAX = 5
    };

    enum ScreenBrightnessLevel
    {
        SCREEN_BRIGHTNESS_LEVEL_MIN = 1,
        SCREEN_BRIGHTNESS_LEVEL_DEFAULT = 4,
        SCREEN_BRIGHTNESS_LEVEL_MAX = 5
    };
};

#endif // DS_CFG_LOCAL_DEF_H
