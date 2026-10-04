#include "flame_class_chunk.h"
#include "flame_class_stream.h"
#include "flame_class_fire.h"
#include "flame_class_drips.h"
#include "flame_class_smoke.h"
#include <universal/com_math_anglevectors.h>
#include <cgame_mp/cg_local_mp.h>
#include <cgame/cg_drawtools.h>

flameChunk_s flameChunks[800];
flameChunk_s *flameChunksFree;
flameChunk_s *flameChunksUsed;

flameChunk_s sv_flameChunks[800];
flameChunk_s *sv_flameChunksFree;
flameChunk_s *sv_flameChunksUsed;

int g_ChunkCount;
int g_ChunkCountWaterMark;

void __cdecl Flame_Class_Chunk_Init()
{
    memset(flameChunks, 0, 0x15E00u);
    Flame_List_Init((flameGeneric_s*)flameChunks, 112, 800);
    flameChunksFree = flameChunks;
    flameChunksUsed = 0;

    memset(sv_flameChunks, 0, 0x15E00u);
    Flame_List_Init((flameGeneric_s *)sv_flameChunks, 112, 800);
    sv_flameChunksFree = sv_flameChunks;
    sv_flameChunksUsed = 0;

    ++g_ChunkCount;
}

flameChunk_s *__cdecl Flame_Class_Chunk_Alloc(bool is_server)
{
    flameChunk_s *freeChunk; // [esp+0h] [ebp-4h]

    if ( is_server )
        freeChunk = sv_flameChunksFree;
    else
        freeChunk = flameChunksFree;
    if ( !freeChunk )
        return 0;
    if ( is_server )
    {
        Flame_List_Move_Global(
            &freeChunk->gen,
            (flameGeneric_s **)&sv_flameChunksFree,
            (flameGeneric_s **)&sv_flameChunksUsed);
    }
    else
    {
        Flame_List_Move_Global(&freeChunk->gen, (flameGeneric_s **)&flameChunksFree, (flameGeneric_s **)&flameChunksUsed);
        if ( ++g_ChunkCount > g_ChunkCountWaterMark )
            g_ChunkCountWaterMark = g_ChunkCount;
    }
    return freeChunk;
}

void __cdecl Flame_Class_Chunk_Free(bool is_server, flameChunk_s *chunk)
{
    if ( chunk->gen.stream )
        Flame_List_Sub_Local(&chunk->gen, (flameGeneric_s **)&chunk->gen.stream->chunkList);
    if ( is_server )
    {
        Flame_List_Move_Global(&chunk->gen, (flameGeneric_s **)&sv_flameChunksUsed, (flameGeneric_s **)&sv_flameChunksFree);
    }
    else
    {
        Flame_List_Move_Global(&chunk->gen, (flameGeneric_s **)&flameChunksUsed, (flameGeneric_s **)&flameChunksFree);
        --g_ChunkCount;
    }
    Flame_Item_Init(&chunk->gen, 0x70u);
}

void __cdecl Flame_Phys_Spawn_Items(bool is_server, int curTime, flameChunk_s *trav)
{
    int max; // [esp+8h] [ebp-10h]
    int min; // [esp+10h] [ebp-8h]
    float lifeFrac; // [esp+14h] [ebp-4h]

    lifeFrac = (float)(trav->gen.age.lastUpdateTime - trav->gen.age.startTime)
                     / (float)(trav->gen.age.endTime - trav->gen.age.startTime);
    if ( trav->spawnFireInterval && curTime >= trav->spawnFireInterval + trav->lastSpawnFire )
    {
        min = (int)(float)(1000.0 * trav->gen.stream->flameVars->flameVar_streamChunkFireMinLifeFracStart);
        max = (int)(float)(1000.0 * trav->gen.stream->flameVars->flameVar_streamChunkFireMinLifeFracEnd);
        if ( trav->gen.age.lastUpdateTime - trav->gen.age.startTime >= min )
        {
            if ( trav->gen.age.lastUpdateTime - trav->gen.age.startTime <= max )
                Flame_Class_Fire_Spawn(
                    is_server,
                    trav,
                    curTime,
                    (float)(1.0 - (float)((trav->gen.age.lastUpdateTime - trav->gen.age.startTime - min) / (max - min)))
                * trav->gen.stream->flameVars->flameVar_streamChunkFireMinLifeFrac);
            else
                Flame_Class_Fire_Spawn(is_server, trav, curTime, 0.0);
        }
        else
        {
            Flame_Class_Fire_Spawn(is_server, trav, curTime, trav->gen.stream->flameVars->flameVar_streamChunkFireMinLifeFrac);
        }
        trav->lastSpawnFire += trav->spawnFireInterval;
        trav->spawnFireInterval = (int)(Flame_Random(is_server)
                                                                    * ((trav->gen.stream->flameVars->flameVar_streamChunkSpawnFireIntervalEnd
                                                                        - trav->gen.stream->flameVars->flameVar_streamChunkSpawnFireIntervalStart)
                                                                     * lifeFrac
                                                                     + trav->gen.stream->flameVars->flameVar_streamChunkSpawnFireIntervalStart)
                                                                    * 1000.0);
    }
    if ( trav->spawnDripsInterval && trav->gen.age.lastUpdateTime - trav->gen.age.startTime > trav->spawnDripsInterval )
    {
        Flame_Class_Drips_Spawn(is_server, trav, curTime, 0.0);
        trav->spawnDripsInterval = 0;
    }
    if ( !is_server
        && trav->spawnSmokeInterval
        && trav->gen.age.lastUpdateTime - trav->gen.age.startTime > trav->spawnSmokeInterval )
    {
        Flame_Class_Smoke_Spawn(trav, curTime);
        trav->spawnSmokeInterval = 0;
    }
}

void __cdecl Flame_Class_Chunk_Age(bool is_server, int time)
{
    phys_static_array<flameGeneric_s *,1000> *v2; // [esp+8h] [ebp-54h]
    phys_static_array<flameGeneric_s *,1000> *v3; // [esp+Ch] [ebp-50h]
    flamePhysics_t physBackup; // [esp+1Ch] [ebp-40h] BYREF
    flameChunk_s *nextTrav; // [esp+50h] [ebp-Ch]
    int curTime; // [esp+54h] [ebp-8h]
    flameChunk_s *trav; // [esp+58h] [ebp-4h]

    if ( is_server )
        trav = sv_flameChunksUsed;
    else
        trav = flameChunksUsed;
    while ( trav )
    {
        nextTrav = (flameChunk_s *)trav->gen.listGlobal.next;
        curTime = time;
        if ( trav->gen.age.lastUpdateTime < trav->gen.age.endTime
            && trav->spawnFireInterval
            && curTime > trav->spawnFireInterval + trav->lastSpawnFire )
        {
            curTime = trav->spawnFireInterval + trav->lastSpawnFire;
        }
        if ( ((*((unsigned int *)&trav->gen + 23) >> 3) & 1) != 0
            || trav->gen.age.lastUpdateTime - trav->gen.age.startTime >= (int)(float)(1000.0
                                                                                                                                                            * trav->gen.stream->flameVars->flameVar_streamChunkSpawnFireMaxLifeFrac) )
        {
            Flame_Class_Chunk_Free(is_server, trav);
        }
        else
        {
            while ( curTime <= time )
            {
                if ( trav->gen.age.lastUpdateTime - trav->gen.age.startTime < (int)(float)(1000.0
                                                                                                                                                                 * trav->gen.stream->flameVars->flameVar_streamChunkSpawnFireMaxLifeFrac) )
                {
                    if ( curTime == time )
                    {
                        if ( is_server )
                            v2 = &sv_flames;
                        else
                            v2 = &cl_flames;
                        Flame_Phys_Update_Item_Chunk(&trav->gen, curTime, v2);
                        Flame_Phys_Spawn_Items(is_server, curTime, trav);
                    }
                    else
                    {
                        memcpy(&physBackup, trav, sizeof(physBackup));
                        if ( is_server )
                            v3 = &sv_flames;
                        else
                            v3 = &cl_flames;
                        Flame_Phys_Update_Item_Chunk(&trav->gen, curTime, v3);
                        Flame_Phys_Spawn_Items(is_server, curTime, trav);
                        memcpy(trav, &physBackup, 0x34u);
                    }
                }
                if ( curTime == time )
                    break;
                curTime = time;
            }
        }
        trav = nextTrav;
    }
    Flame_Phys_Update_Items(is_server);
}

flameChunk_s *__cdecl Flame_Class_Chunk_Spawn(
                bool is_server,
                flameChunkSpawnVars_t *spawnVars,
                flameGeneric_s **chunkList,
                float *velocityAdd,
                flameTable *fTable)
{
    int v6; // esi
    float speed; // [esp+18h] [ebp-14h]
    flameChunk_s *chunk; // [esp+28h] [ebp-4h]

    chunk = Flame_Class_Chunk_Alloc(is_server);
    if ( !chunk )
        return 0;
    *((unsigned int *)&chunk->gen + 23) = *((unsigned int *)&chunk->gen + 23) & 0xFFFFFFF8 | 1;
    *((unsigned int *)&chunk->gen + 23) = (16 * flame_spawn_id++) | *((unsigned int *)&chunk->gen + 23) & 0xF;
    *((unsigned int *)&chunk->gen + 23) &= ~8u;
    chunk->gen.age.startTime = spawnVars->time;
    chunk->gen.age.endTime = spawnVars->duration + spawnVars->time;
    chunk->gen.age.lastUpdateTime = spawnVars->time;
    chunk->gen.size.current = spawnVars->sizeStart;
    chunk->gen.size.rate = spawnVars->sizeRate;
    chunk->gen.phys.origin[0] = spawnVars->origin[0];
    chunk->gen.phys.origin[1] = spawnVars->origin[1];
    chunk->gen.phys.origin[2] = spawnVars->origin[2];
    AngleVectors(spawnVars->angle, chunk->gen.phys.velocity, 0, 0);
    speed = spawnVars->speed;
    chunk->gen.phys.velocity[0] = speed * chunk->gen.phys.velocity[0];
    chunk->gen.phys.velocity[1] = speed * chunk->gen.phys.velocity[1];
    chunk->gen.phys.velocity[2] = speed * chunk->gen.phys.velocity[2];
    chunk->gen.phys.velocity[0] = chunk->gen.phys.velocity[0] + *velocityAdd;
    chunk->gen.phys.velocity[1] = chunk->gen.phys.velocity[1] + velocityAdd[1];
    chunk->gen.phys.velocity[2] = chunk->gen.phys.velocity[2] + velocityAdd[2];
    chunk->gen.phys.invStartSpeed = Flame_CalcInvStartSpeed(1.0, spawnVars->speed);
    chunk->gen.phys.gravity = spawnVars->gravityStart;
    chunk->gen.phys.rotation = Flame_CRandom(is_server) * fTable->flameVar_streamChunkRotationRange;
    chunk->gen.phys.rotVel = 0.0f;
    chunk->spawnFireInterval = (int)(float)(1000.0 * fTable->flameVar_streamChunkSpawnFireIntervalStart);
    v6 = (int)(float)(1000.0 * fTable->flameVar_streamChunkSpawnFireMinLifeFrac);
    chunk->lastSpawnFire = spawnVars->time - (int)(Flame_Random(is_server) * (double)chunk->spawnFireInterval) + v6;
    chunk->spawnDripsInterval = 0;
    chunk->spawnSmokeInterval = 0;
    if ( chunkList )
        Flame_List_Add_Local(&chunk->gen, chunkList);
    return chunk;
}

void __cdecl Flame_Class_Chunk_Render_Item(int localClientNum, flameChunk_s *chunk)
{
    float v2; // [esp+0h] [ebp-88h]
    float v3; // [esp+4h] [ebp-84h]
    float v4; // [esp+10h] [ebp-78h]
    float v5; // [esp+14h] [ebp-74h]
    float fwd[3]; // [esp+18h] [ebp-70h] BYREF
    float points[4][2]; // [esp+24h] [ebp-64h]
    float right[3]; // [esp+44h] [ebp-44h] BYREF
    float start[3]; // [esp+50h] [ebp-38h] BYREF
    float end[3]; // [esp+5Ch] [ebp-2Ch] BYREF
    float angles[3]; // [esp+68h] [ebp-20h] BYREF
    float down[3]; // [esp+74h] [ebp-14h] BYREF
    int i; // [esp+80h] [ebp-8h]
    cg_s *clientGlobals; // [esp+84h] [ebp-4h]

    clientGlobals = CG_GetLocalClientGlobals(localClientNum);
    if ( !clientGlobals
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\flame\\flame_class_chunk.cpp",
                    353,
                    0,
                    "%s\n\t%s",
                    "clientGlobals",
                    "unknown client index") )
    {
        __debugbreak();
    }
    if ( flame_debug_render->current.integer > 0 )
    {
        points[0][0] = -1.0f;
        points[0][1] = -1.0f;

        points[1][0] = -1.0f;
        points[1][1] = 1.0f;

        points[2][0] = 1.0f;
        points[2][1] = 1.0f;

        points[3][0] = 1.0f;
        points[3][1] = -1.0f;

        AxisToAngles(clientGlobals->refdef.viewaxis, angles);
        angles[2] = chunk->gen.phys.rotation;
        AngleVectors(angles, fwd, right, down);
        v5 = chunk->gen.size.current * 0.5;
        right[0] = v5 * right[0];
        right[1] = v5 * right[1];
        right[2] = v5 * right[2];
        v4 = chunk->gen.size.current * -0.5;
        down[0] = v4 * down[0];
        down[1] = v4 * down[1];
        down[2] = v4 * down[2];
        start[0] = (float)(points[3][0] * right[0]) + chunk->gen.phys.origin[0];
        start[1] = (float)(points[3][0] * right[1]) + chunk->gen.phys.origin[1];
        start[2] = (float)(points[3][0] * right[2]) + chunk->gen.phys.origin[2];
        start[0] = (float)(points[3][1] * down[0]) + start[0];
        start[1] = (float)(points[3][1] * down[1]) + start[1];
        start[2] = (float)(points[3][1] * down[2]) + start[2];
        for ( i = 0; i < 4; ++i )
        {
            v3 = points[i][0];
            end[0] = (float)(v3 * right[0]) + chunk->gen.phys.origin[0];
            end[1] = (float)(v3 * right[1]) + chunk->gen.phys.origin[1];
            end[2] = (float)(v3 * right[2]) + chunk->gen.phys.origin[2];
            v2 = points[i][1];
            end[0] = (float)(v2 * down[0]) + end[0];
            end[1] = (float)(v2 * down[1]) + end[1];
            end[2] = (float)(v2 * down[2]) + end[2];
            CG_DebugLine(start, end, colorRed, 1, 1);
            start[0] = end[0];
            start[1] = end[1];
            start[2] = end[2];
        }
    }
}

void __cdecl Flame_Class_Chunk_Render_All(int localClientNum)
{
    flameChunk_s *chunk; // [esp+0h] [ebp-4h]

    if ( flame_debug_render->current.integer > 0 )
    {
        for ( chunk = flameChunksUsed; chunk; chunk = (flameChunk_s *)chunk->gen.listGlobal.next )
            Flame_Class_Chunk_Render_Item(localClientNum, chunk);
    }
}

