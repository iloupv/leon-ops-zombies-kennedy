#include "flame_class_fire.h"
#include "flame_class_stream.h"

#include <cstring>
#include "flame_physics.h"
#include <cgame_mp/cg_local_mp.h>
#include <universal/com_math_anglevectors.h>
#include <cgame/cg_drawtools.h>

flameFire_t flameFire[2400];
flameFire_t *flameFireFree;
flameFire_t *flameFireUsed;
int g_FireCount;
int g_FireCountWaterMark;

void __cdecl Flame_Class_Fire_Init()
{
    memset(flameFire, 0, 0x3A980u);
    Flame_List_Init((flameGeneric_s*)flameFire, 100, 2400);
    flameFireFree = flameFire;
    flameFireUsed = 0;
    ++g_FireCount;
}

flameFire_t *__cdecl Flame_Class_Fire_Alloc()
{
    flameFire_t *freeFire; // [esp+0h] [ebp-4h]

    freeFire = flameFireFree;
    if ( !flameFireFree )
        return 0;
    Flame_List_Move_Global(&flameFireFree->gen, (flameGeneric_s **)&flameFireFree, (flameGeneric_s **)&flameFireUsed);
    if ( ++g_FireCount > g_FireCountWaterMark )
        g_FireCountWaterMark = g_FireCount;
    return freeFire;
}

void __cdecl Flame_Class_Fire_Free(bool is_server, flameFire_t *fire)
{
    if ( fire->flameRend )
        Flame_List_Sub_Local(&fire->gen, &fire->flameRend->fireList);
    Flame_List_Move_Global(&fire->gen, (flameGeneric_s **)&flameFireUsed, (flameGeneric_s **)&flameFireFree);
    --g_FireCount;
    Flame_Item_Init(&fire->gen, 0x64u);
}

void __cdecl Flame_Class_Fire_Age(bool is_server, int time)
{
    flameFire_t *next; // [esp+8h] [ebp-8h]
    flameFire_t *trav; // [esp+Ch] [ebp-4h]

    if ( !is_server )
    {
        for ( trav = flameFireUsed; trav; trav = next )
        {
            next = (flameFire_t *)trav->gen.listGlobal.next;
            if ( trav->gen.age.lastUpdateTime < trav->gen.age.endTime )
            {
                if ( ((*((unsigned int *)&trav->gen + 23) >> 4) & 0xFFFFFFFu) < flame_freeze_id )
                {
                    trav->gen.phys.velocity[0] = 0.0f;
                    trav->gen.phys.velocity[1] = 0.0f;
                }
                Flame_Phys_Update_Item_Fire(&trav->gen, time);
            }
            if ( trav->gen.age.lastUpdateTime >= trav->gen.age.endTime )
                Flame_Class_Fire_Free(0, trav);
        }
        Flame_Phys_Update_Items(0);
    }
}

flameFire_t *__cdecl Flame_Class_Fire_Spawn(bool is_server, flameChunk_s *fromChunk, int curTime, float minLifeFrac)
{
    float v5; // [esp+8h] [ebp-78h]
    float v6; // [esp+Ch] [ebp-74h]
    float v7; // [esp+14h] [ebp-6Ch]
    flameFire_t *fire; // [esp+3Ch] [ebp-44h]
    float fwdVec[3]; // [esp+48h] [ebp-38h] BYREF
    flameRender_s *flameRend; // [esp+54h] [ebp-2Ch]
    float lifeFrac; // [esp+58h] [ebp-28h]
    flameTable *flameVars; // [esp+5Ch] [ebp-24h]
    float speed; // [esp+60h] [ebp-20h]
    float lifetime; // [esp+64h] [ebp-1Ch]
    float sideVec[3]; // [esp+68h] [ebp-18h] BYREF
    float upVec[3]; // [esp+74h] [ebp-Ch] BYREF

    flameRend = 0;
    upVec[0] = 0.0f;
    upVec[1] = 0.0f;
    upVec[2] = 1.0f;
    if ( !fromChunk
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\flame\\flame_class_fire.cpp", 250, 0, "%s", "fromChunk") )
    {
        __debugbreak();
    }
    if ( is_server )
        return 0;
    if ( !fromChunk || !fromChunk->gen.stream )
        return 0;
    fire = Flame_Class_Fire_Alloc();
    if ( !fire )
        return 0;
    flameVars = fromChunk->gen.stream->flameVars;
    flameRend = fromChunk->gen.stream->renderList->flameRend;
    *((unsigned int *)&fire->gen + 23) = *((unsigned int *)&fire->gen + 23) & 0xFFFFFFF8 | 2;
    fire->gen.stream = fromChunk->gen.stream;
    *((unsigned int *)&fire->gen + 23) = (16 * (*((unsigned int *)&fromChunk->gen + 23) >> 4)) | *((unsigned int *)&fire->gen + 23) & 0xF;
    if ( fromChunk->gen.age.endTime == fromChunk->gen.age.startTime
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\flame\\flame_class_fire.cpp",
                    284,
                    0,
                    "%s",
                    "fromChunk->gen.age.endTime!=fromChunk->gen.age.startTime") )
    {
        __debugbreak();
    }
    if ( (float)((float)(fromChunk->gen.age.lastUpdateTime - fromChunk->gen.age.startTime)
                         / (float)(fromChunk->gen.age.endTime - fromChunk->gen.age.startTime)) <= minLifeFrac )
        v6 = minLifeFrac;
    else
        v6 = (float)(fromChunk->gen.age.lastUpdateTime - fromChunk->gen.age.startTime)
             / (float)(fromChunk->gen.age.endTime - fromChunk->gen.age.startTime);
    lifeFrac = v6;
    lifetime = Flame_Random(0) * flameVars->flameVar_fireLifeRand + flameVars->flameVar_fireLife;
    fire->gen.age.startTime = curTime - (int)(float)((float)(v6 * lifetime) * 1000.0);
    fire->gen.age.endTime = fire->gen.age.startTime + (int)(float)(1000.0 * lifetime);
    fire->gen.age.lastUpdateTime = curTime;
    fire->gen.size.current = fromChunk->gen.size.current * flameVars->flameVar_fireStartSizeScale;
    fire->gen.size.rate = (float)((float)((float)(fromChunk->gen.size.current * flameVars->flameVar_fireEndSizeScale)
                                                                            + flameVars->flameVar_fireEndSizeAdd)
                                                            - fire->gen.size.current)
                                            / (float)((float)(1.0 - lifeFrac) * flameVars->flameVar_fireLife);
    if ( fire->gen.size.rate > 70.0 )
        fire->gen.size.rate = 70.0f;
    if ( (LODWORD(fire->gen.size.rate) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\flame\\flame_class_fire.cpp",
                    299,
                    0,
                    "%s",
                    "!IS_NAN(gen->size.rate)") )
    {
        __debugbreak();
    }
    fire->gen.phys.origin[0] = fromChunk->gen.phys.origin[0];
    fire->gen.phys.origin[1] = fromChunk->gen.phys.origin[1];
    fire->gen.phys.origin[2] = fromChunk->gen.phys.origin[2];
    v5 = 1.0 - flameVars->flameVar_fireSpeedScaleRand;
    speed = (Flame_Random(0) * flameVars->flameVar_fireSpeedScaleRand + v5) * flameVars->flameVar_fireSpeedScale;
    fire->gen.phys.velocity[0] = speed * fromChunk->gen.phys.velocity[0];
    fire->gen.phys.velocity[1] = speed * fromChunk->gen.phys.velocity[1];
    fire->gen.phys.velocity[2] = speed * fromChunk->gen.phys.velocity[2];
    fwdVec[0] = fire->gen.phys.velocity[0];
    fwdVec[1] = fire->gen.phys.velocity[1];
    fwdVec[2] = 0.0f;
    speed = Vec3Normalize(fwdVec);
    Vec3Cross(fwdVec, upVec, sideVec);
    v7 = Flame_CRandom(0) * speed * flameVars->flameVar_fireVelocityAddSideways;
    fire->gen.phys.velocity[0] = (float)(v7 * sideVec[0]) + fire->gen.phys.velocity[0];
    fire->gen.phys.velocity[1] = (float)(v7 * sideVec[1]) + fire->gen.phys.velocity[1];
    fire->gen.phys.velocity[2] = (float)(v7 * sideVec[2]) + fire->gen.phys.velocity[2];
    fire->gen.phys.velocity[2] = Flame_Random(0) * flameVars->flameVar_fireVelocityAddZRand
                                                         + flameVars->flameVar_fireVelocityAddZ
                                                         + fire->gen.phys.velocity[2];
    fire->gen.phys.invStartSpeed = Flame_CalcInvStartSpeed(
                                                                     fromChunk->gen.phys.invStartSpeed,
                                                                     flameVars->flameVar_fireSpeedScale);
    fire->gen.phys.gravity = flameVars->flameVar_fireGravity;
    fire->gen.phys.rotation = fromChunk->gen.phys.rotation;
    fire->gen.phys.rotVel = Flame_CRandom(0) * flameVars->flameVar_fireMaxRotVel;
    fire->flameRend = flameRend;
    Flame_List_Add_Local(&fire->gen, &fire->flameRend->fireList);
    return fire;
}

void __cdecl Flame_Class_Fire_Render_Item(int localClientNum, flameFire_t *fire)
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
                    "C:\\projects_pc\\cod\\codsrc\\src\\flame\\flame_class_fire.cpp",
                    340,
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
        angles[2] = fire->gen.phys.rotation;
        AngleVectors(angles, fwd, right, down);
        v5 = fire->gen.size.current * 0.5;
        right[0] = v5 * right[0];
        right[1] = v5 * right[1];
        right[2] = v5 * right[2];
        v4 = fire->gen.size.current * -0.5;
        down[0] = v4 * down[0];
        down[1] = v4 * down[1];
        down[2] = v4 * down[2];
        start[0] = (float)(points[3][0] * right[0]) + fire->gen.phys.origin[0];
        start[1] = (float)(points[3][0] * right[1]) + fire->gen.phys.origin[1];
        start[2] = (float)(points[3][0] * right[2]) + fire->gen.phys.origin[2];
        start[0] = (float)(points[3][1] * down[0]) + start[0];
        start[1] = (float)(points[3][1] * down[1]) + start[1];
        start[2] = (float)(points[3][1] * down[2]) + start[2];
        for ( i = 0; i < 4; ++i )
        {
            v3 = points[i][0];
            end[0] = (float)(v3 * right[0]) + fire->gen.phys.origin[0];
            end[1] = (float)(v3 * right[1]) + fire->gen.phys.origin[1];
            end[2] = (float)(v3 * right[2]) + fire->gen.phys.origin[2];
            v2 = points[i][1];
            end[0] = (float)(v2 * down[0]) + end[0];
            end[1] = (float)(v2 * down[1]) + end[1];
            end[2] = (float)(v2 * down[2]) + end[2];
            CG_DebugLine(start, end, colorYellow, 1, 1);
            start[0] = end[0];
            start[1] = end[1];
            start[2] = end[2];
        }
    }
}

void __cdecl Flame_Class_Fire_Render_All(int localClientNum)
{
    flameFire_t *fire; // [esp+0h] [ebp-4h]

    if ( flame_debug_render->current.integer > 0 )
    {
        for ( fire = flameFireUsed; fire; fire = (flameFire_t *)fire->gen.listGlobal.next )
        {
            if ( flame_debug_render->current.integer > 0 )
                Flame_Class_Fire_Render_Item(localClientNum, fire);
        }
    }
}

void __cdecl Flame_Class_Fire_Render_Local_List(int localClientNum, flameFire_t *start)
{
    flameFire_t *next; // [esp+0h] [ebp-2590h]
    cg_s *clientGlobals; // [esp+4h] [ebp-258Ch]
    flameGeneric_s *flameGenericList[2401]; // [esp+8h] [ebp-2588h] BYREF
    int numItems; // [esp+258Ch] [ebp-4h]
    int savedregs; // [esp+2590h] [ebp+0h] BYREF

    clientGlobals = CG_GetLocalClientGlobals(localClientNum);
    if ( !clientGlobals
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\flame\\flame_class_fire.cpp",
                    427,
                    0,
                    "%s\n\t%s",
                    "clientGlobals",
                    "unknown client index") )
    {
        __debugbreak();
    }
    if ( (!start || start->gen.stream) && (!start || Flame_GetLocalClientSourceRange()) )
    {
        next = start;
        numItems = 0;
        while ( numItems < 2400 && next )
        {
            flameGenericList[numItems++] = &next->gen;
            next = (flameFire_t *)next->gen.listLocal.next;
        }
        if ( numItems > 1 )
            Flame_Render_Sprites(
                clientGlobals,
                start->gen.stream->flameVars->fire,
                flameGenericList,
                numItems);
    }
}

