#include "flame_class_drips.h"
#include "flame_class_stream.h"
#include <cstring>
#include <cgame_mp/cg_local_mp.h>
#include <universal/com_math_anglevectors.h>
#include <cgame/cg_drawtools.h>

flameDrips_t flameDrips[250];
flameDrips_t *flameDripsFree;
flameDrips_t *flameDripsUsed;

flameDrips_t sv_flameDrips[250];
flameDrips_t *sv_flameDripsFree;
flameDrips_t *sv_flameDripsUsed;

int g_DripsCount;
int g_DripsCountWaterMark;

void __cdecl Flame_Class_Drips_Init()
{
    memset(flameDrips, 0, 0x61A8u);
    Flame_List_Init((flameGeneric_s *)flameDrips, 100, 250);
    flameDripsFree = flameDrips;
    flameDripsUsed = 0;

    memset(sv_flameDrips, 0, 0x61A8u);
    Flame_List_Init((flameGeneric_s*)sv_flameDrips, 100, 250);
    sv_flameDripsFree = sv_flameDrips;
    sv_flameDripsUsed = 0;

    ++g_DripsCount;
}

flameDrips_t *__cdecl Flame_Class_Drips_Alloc(bool is_server)
{
    flameDrips_t *freeDrips; // [esp+0h] [ebp-4h]

    if ( is_server )
        freeDrips = sv_flameDripsFree;
    else
        freeDrips = flameDripsFree;
    if ( !freeDrips )
        return 0;
    if ( is_server )
    {
        Flame_List_Move_Global(
            &freeDrips->gen,
            (flameGeneric_s **)&sv_flameDripsFree,
            (flameGeneric_s **)&sv_flameDripsUsed);
    }
    else
    {
        Flame_List_Move_Global(&freeDrips->gen, (flameGeneric_s **)&flameDripsFree, (flameGeneric_s **)&flameDripsUsed);
        if ( ++g_DripsCount > g_DripsCountWaterMark )
            g_DripsCountWaterMark = g_DripsCount;
    }
    return freeDrips;
}

void __cdecl Flame_Class_Drips_Free(bool is_server, flameDrips_t *fire)
{
    if ( fire->flameRend )
        Flame_List_Sub_Local(&fire->gen, &fire->flameRend->dripsList);
    if ( is_server )
    {
        Flame_List_Move_Global(&fire->gen, (flameGeneric_s **)&sv_flameDripsUsed, (flameGeneric_s **)&sv_flameDripsFree);
    }
    else
    {
        Flame_List_Move_Global(&fire->gen, (flameGeneric_s **)&flameDripsUsed, (flameGeneric_s **)&flameDripsFree);
        --g_DripsCount;
    }
    Flame_Item_Init(&fire->gen, 0x64u);
}

void __cdecl Flame_Class_Drips_Free_If_Stream(bool is_server, flameStream_s *stream)
{
    flameDrips_t *next; // [esp+0h] [ebp-8h]
    flameDrips_t *trav; // [esp+4h] [ebp-4h]

    if ( is_server )
        trav = sv_flameDripsUsed;
    else
        trav = flameDripsUsed;
    while ( trav )
    {
        next = (flameDrips_t *)trav->gen.listGlobal.next;
        if ( trav->gen.stream == stream )
            Flame_Class_Drips_Free(is_server, trav);
        trav = next;
    }
}

void __cdecl Flame_Class_Drips_Age(bool is_server, int time)
{
    phys_static_array<flameGeneric_s *,1000> *v2; // [esp+0h] [ebp-10h]
    flameDrips_t *next; // [esp+8h] [ebp-8h]
    flameDrips_t *trav; // [esp+Ch] [ebp-4h]

    if ( is_server )
        trav = sv_flameDripsUsed;
    else
        trav = flameDripsUsed;
    while ( trav )
    {
        next = (flameDrips_t *)trav->gen.listGlobal.next;
        if ( trav->gen.age.lastUpdateTime < trav->gen.age.endTime )
        {
            if ( is_server )
                v2 = &sv_flames;
            else
                v2 = &cl_flames;
            Flame_Phys_Update_Item_Drip(&trav->gen, time, v2);
        }
        if ( trav->gen.age.lastUpdateTime >= trav->gen.age.endTime )
            Flame_Class_Drips_Free(is_server, trav);
        trav = next;
    }
    Flame_Phys_Update_Items(is_server);
}

flameDrips_t *__cdecl Flame_Class_Drips_Spawn(bool is_server, flameChunk_s *fromChunk, int curTime, float minLifeFrac)
{
    float v5; // [esp+8h] [ebp-48h]
    float v6; // [esp+Ch] [ebp-44h]
    float v7; // [esp+14h] [ebp-3Ch]
    flameDrips_t *fire; // [esp+28h] [ebp-28h]
    flameRender_s *flameRend; // [esp+30h] [ebp-20h]
    flameTable *flameVars; // [esp+38h] [ebp-18h]
    float speed; // [esp+3Ch] [ebp-14h]
    float lifetime; // [esp+40h] [ebp-10h]
    float sideVec; // [esp+44h] [ebp-Ch]
    float sideVec_4; // [esp+48h] [ebp-8h]

    flameRend = 0;
    if ( !fromChunk
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\flame\\flame_class_drips.cpp", 221, 0, "%s", "fromChunk") )
    {
        __debugbreak();
    }
    if ( !fromChunk || !fromChunk->gen.stream )
        return 0;
    fire = Flame_Class_Drips_Alloc(is_server);
    if ( !fire )
        return 0;
    flameVars = fromChunk->gen.stream->flameVars;
    if ( !is_server )
        flameRend = fromChunk->gen.stream->renderList->flameRend;
    *((unsigned int *)&fire->gen + 23) = *((unsigned int *)&fire->gen + 23) & 0xFFFFFFF8 | 4;
    fire->gen.stream = fromChunk->gen.stream;
    if ( (float)((float)(fromChunk->gen.age.lastUpdateTime - fromChunk->gen.age.startTime)
                         / (float)(fromChunk->gen.age.endTime - fromChunk->gen.age.startTime)) <= minLifeFrac )
        v6 = minLifeFrac;
    else
        v6 = (float)(fromChunk->gen.age.lastUpdateTime - fromChunk->gen.age.startTime)
             / (float)(fromChunk->gen.age.endTime - fromChunk->gen.age.startTime);
    lifetime = Flame_Random(is_server) * flameVars->flameVar_dripsLifeRand + flameVars->flameVar_dripsLife;
    fire->gen.age.startTime = curTime - (int)(float)((float)(v6 * lifetime) * 1000.0);
    fire->gen.age.endTime = fire->gen.age.startTime + (int)(float)(1000.0 * lifetime);
    fire->gen.age.lastUpdateTime = curTime;
    fire->gen.size.current = fromChunk->gen.size.current * flameVars->flameVar_dripsStartSizeScale;
    fire->gen.size.rate = 1.0f;
    fire->gen.phys.origin[0] = fromChunk->gen.phys.origin[0];
    fire->gen.phys.origin[1] = fromChunk->gen.phys.origin[1];
    fire->gen.phys.origin[2] = fromChunk->gen.phys.origin[2];
    v5 = 1.0 - flameVars->flameVar_dripsSpeedScaleRand;
    speed = (Flame_Random(is_server) * flameVars->flameVar_dripsSpeedScaleRand + v5) * flameVars->flameVar_dripsSpeedScale;
    fire->gen.phys.velocity[0] = speed * fromChunk->gen.phys.velocity[0];
    fire->gen.phys.velocity[1] = speed * fromChunk->gen.phys.velocity[1];
    fire->gen.phys.velocity[2] = speed * fromChunk->gen.phys.velocity[2];
    sideVec = fire->gen.phys.velocity[1];
    sideVec_4 = fire->gen.phys.velocity[0];
    v7 = Flame_CRandom(is_server) * flameVars->flameVar_dripsVelocityAddSideways;
    fire->gen.phys.velocity[0] = (float)(v7 * sideVec) + fire->gen.phys.velocity[0];
    fire->gen.phys.velocity[1] = (float)(v7 * sideVec_4) + fire->gen.phys.velocity[1];
    fire->gen.phys.velocity[2] = (float)(v7 * 0.0) + fire->gen.phys.velocity[2];
    fire->gen.phys.velocity[2] = Flame_Random(is_server) * flameVars->flameVar_dripsVelocityAddZRand
                                                         + flameVars->flameVar_dripsVelocityAddZ
                                                         + fire->gen.phys.velocity[2];
    fire->gen.phys.invStartSpeed = Flame_CalcInvStartSpeed(
                                                                     fromChunk->gen.phys.invStartSpeed,
                                                                     flameVars->flameVar_dripsSpeedScale);
    fire->gen.phys.gravity = flameVars->flameVar_dripsGravity;
    fire->gen.phys.rotation = fromChunk->gen.phys.rotation;
    fire->gen.phys.rotVel = Flame_CRandom(is_server) * flameVars->flameVar_dripsMaxRotVel;
    fire->flameRend = flameRend;
    if ( !is_server )
        Flame_List_Add_Local(&fire->gen, &fire->flameRend->dripsList);
    return fire;
}

void __cdecl Flame_Class_Drips_Render_Item(int localClientNum, flameDrips_t *fire)
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
                    "C:\\projects_pc\\cod\\codsrc\\src\\flame\\flame_class_drips.cpp",
                    305,
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
            CG_DebugLine(start, end, colorMagenta, 1, 1);
            start[0] = end[0];
            start[1] = end[1];
            start[2] = end[2];
        }
    }
}

void __cdecl Flame_Class_Drips_Render_All(int localClientNum)
{
    flameDrips_t *fire; // [esp+0h] [ebp-4h]

    if ( flame_debug_render->current.integer > 0 )
    {
        for ( fire = flameDripsUsed; fire; fire = (flameDrips_t *)fire->gen.listGlobal.next )
        {
            if ( flame_debug_render->current.integer > 0 )
                Flame_Class_Drips_Render_Item(localClientNum, fire);
        }
    }
}

void __cdecl Flame_Class_Drips_Render_Local_List(int localClientNum, flameDrips_t *start)
{
    flameDrips_t *fire; // [esp+0h] [ebp-3F8h]
    cg_s *clientGlobals; // [esp+4h] [ebp-3F4h]
    flameGeneric_s *renderList[250]; // [esp+8h] [ebp-3F0h] BYREF
    int count; // [esp+3F4h] [ebp-4h]
    int savedregs; // [esp+3F8h] [ebp+0h] BYREF

    clientGlobals = CG_GetLocalClientGlobals(localClientNum);
    if ( !clientGlobals
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\flame\\flame_class_drips.cpp",
                    392,
                    0,
                    "%s\n\t%s",
                    "clientGlobals",
                    "unknown client index") )
    {
        __debugbreak();
    }
    if ( (!start || start->gen.stream) && (!start || Flame_GetLocalClientSourceRange()) )
    {
        fire = start;
        count = 0;
        while ( count < 250 && fire )
        {
            renderList[count++] = &fire->gen;
            fire = (flameDrips_t *)fire->gen.listLocal.next;
        }
        if ( count > 1 )
            Flame_Render_Sprites(
                clientGlobals,
                start->gen.stream->flameVars->drips,
                renderList,
                count);
    }
}

