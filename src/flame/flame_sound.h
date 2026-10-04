#pragma once
#include "flame_system.h"

enum snd_flame_source_flags : __int32
{                                       // XREF: ?CG_FlameSourceUpdate@@YAXHHQBM0W4snd_flame_source_flags@@PBD222@Z/r
    SND_FLAME_SOURCE_NONE      = 0x0,
    SND_FLAME_SOURCE_TURN_ON   = 0x1,
    SND_FLAME_SOURCE_TURN_OFF  = 0x2,
    SND_FLAME_SOURCE_IS_FIRING = 0x4,
};

void __cdecl Flame_Class_Source_Sound_Init(flameSource_t *source);
void __cdecl Flame_Class_Source_Sound(
                int localClientNum,
                bool isFiring,
                bool isFirstPerson,
                int time,
                flameTable *table,
                flameSource_t *source);
