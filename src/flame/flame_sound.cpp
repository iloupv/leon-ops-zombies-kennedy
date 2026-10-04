#include "flame_sound.h"
#include "flame_class_stream.h"
#include <cgame_mp/cg_main_mp.h>
#include <sound/snd_utils.h>
#include <cgame/cg_sound.h>

void __cdecl Flame_Class_Source_Sound_Init(flameSource_t *source)
{
    source->wasFiring = 0;
    source->lastSoundTime = 0;
    source->flameSoundPoint[0] = 0.0f;
    source->flameSoundPoint[1] = 0.0f;
    source->flameSoundPoint[2] = 0.0f;
}

void __cdecl Flame_Class_Source_Sound(
                int localClientNum,
                bool isFiring,
                bool isFirstPerson,
                int time,
                flameTable *table,
                flameSource_t *source)
{
    float *v6; // [esp+14h] [ebp-3Ch]
    flameRenderList_s *renderList; // [esp+18h] [ebp-38h]
    float listener[3]; // [esp+24h] [ebp-2Ch] BYREF
    float points[2][3]; // [esp+30h] [ebp-20h] BYREF
    const playerState_s *playerState; // [esp+48h] [ebp-8h]
    int flags; // [esp+4Ch] [ebp-4h]

    if ( !table && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\flame\\flame_sound.cpp", 24, 0, "%s", "table") )
        __debugbreak();
    if ( !source && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\flame\\flame_sound.cpp", 25, 0, "%s", "source") )
        __debugbreak();
    if ( !source->lastSoundTime )
    {
        source->lastSoundTime = time;
        source->flameSoundPoint[0] = source->origin[0];
        source->flameSoundPoint[1] = source->origin[1];
        source->flameSoundPoint[2] = source->origin[2];
    }
    if ( source->currentStream && source->currentStream->chunkList )
    {
        playerState = CG_GetPredictedPlayerState(localClientNum);
        listener[0] = playerState->origin[0];
        listener[1] = playerState->origin[1];
        listener[2] = playerState->origin[2];
        renderList = source->currentStream->renderList;
        *(_QWORD *)&points[0][0] = *(_QWORD *)&renderList->fuelVerts[0][0];
        points[0][2] = renderList->fuelVerts[0][2];
        v6 = source->currentStream->renderList->fuelVerts[(int)table->flameVar_streamFuelNumSegments - 1];
        points[1][0] = *v6;
        points[1][1] = v6[1];
        points[1][2] = v6[2];
        SND_GetNearestPointOnStrip(listener, points, 2u, source->flameSoundPoint);
        if ( ((LODWORD(source->flameSoundPoint[0]) & 0x7F800000) == 0x7F800000
             || (LODWORD(source->flameSoundPoint[1]) & 0x7F800000) == 0x7F800000
             || (LODWORD(source->flameSoundPoint[2]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\flame\\flame_sound.cpp",
                        42,
                        0,
                        "%s",
                        "!IS_NAN((source->flameSoundPoint)[0]) && !IS_NAN((source->flameSoundPoint)[1]) && !IS_NAN((source->flameSoundPoint)[2])") )
        {
            __debugbreak();
        }
    }
    else
    {
        source->flameSoundPoint[0] = source->origin[0];
        source->flameSoundPoint[1] = source->origin[1];
        source->flameSoundPoint[2] = source->origin[2];
    }
    flags = 0;
    if ( !source->wasFiring && isFiring )
        flags |= 1u;
    if ( source->wasFiring && !isFiring )
        flags |= 2u;
    if ( isFiring )
        flags |= 4u;
    source->wasFiring = isFiring;
    source->lastSoundTime = time;
    CG_FlameSourceUpdate(
        localClientNum,
        source->entityNum,
        source->origin,
        source->flameSoundPoint,
        (snd_flame_source_flags)flags,
        (char *)table->flameOffLoopSound,
        (char *)table->flameIgniteSound,
        (char *)table->flameOnLoopSound,
        (char *)table->flameCooldownSound);
}

