#include "flame_class_smoke.h"
#include "flame_class_stream.h"

#include <cstring>
#include "flame_system.h"
#include <cgame_mp/cg_local_mp.h>
#include <universal/com_math_anglevectors.h>
#include <cgame/cg_drawtools.h>


flameSmoke_t flameSmoke[250];
flameSmoke_t *flameSmokeFree;
flameSmoke_t *flameSmokeUsed;
int g_SmokeCount;

void __cdecl Flame_Class_Smoke_Init()
{
    memset(flameSmoke, 0, 0x7148u);
    Flame_List_Init((flameGeneric_s*)flameSmoke, 116, 250);
    flameSmokeFree = flameSmoke;
    flameSmokeUsed = 0;
    ++g_SmokeCount;
}

flameSmoke_t *__cdecl Flame_Class_Smoke_Alloc()
{
    flameSmoke_t *freeSmoke; // [esp+0h] [ebp-4h]

    freeSmoke = flameSmokeFree;
    if ( !flameSmokeFree )
        return 0;
    Flame_List_Move_Global(&flameSmokeFree->gen, (flameGeneric_s **)&flameSmokeFree, (flameGeneric_s **)&flameSmokeUsed);
    ++g_SmokeCount;
    return freeSmoke;
}

void __cdecl Flame_Class_Smoke_Free(flameSmoke_t *smoke)
{
    Flame_List_Move_Global(&smoke->gen, (flameGeneric_s **)&flameSmokeUsed, (flameGeneric_s **)&flameSmokeFree);
    Flame_Item_Init(&smoke->gen, 0x74u);
    --g_SmokeCount;
}

void __cdecl Flame_Class_Smoke_Age(bool is_server, int time)
{
    flameSmoke_t *next; // [esp+8h] [ebp-8h]
    flameSmoke_t *trav; // [esp+Ch] [ebp-4h]

    if ( !is_server )
    {
        for ( trav = flameSmokeUsed; trav; trav = next )
        {
            next = (flameSmoke_t *)trav->gen.listGlobal.next;
            if ( trav->gen.age.lastUpdateTime >= trav->gen.age.endTime )
            {
                Flame_Class_Smoke_Free(trav);
            }
            else
            {
                if ( ((*((unsigned int *)&trav->gen + 23) >> 4) & 0xFFFFFFFu) < flame_freeze_id )
                {
                    trav->gen.phys.velocity[0] = 0.0f;
                    trav->gen.phys.velocity[1] = 0.0f;
                }
                Flame_Phys_Update_Item_Smoke(&trav->gen, time);
                if ( trav->gen.age.lastUpdateTime > trav->gen.age.endTime )
                    trav->gen.age.lastUpdateTime = trav->gen.age.endTime;
            }
        }
        Flame_Phys_Update_Items(0);
    }
}

flameSmoke_t *__cdecl Flame_Class_Smoke_Spawn(flameChunk_s *fromChunk, int curTime)
{
    float flameVar_smokeSpeedScale; // [esp+Ch] [ebp-1Ch]
    int duration; // [esp+14h] [ebp-14h]
    flameTable *flameVars; // [esp+20h] [ebp-8h]
    flameSmoke_t *smoke; // [esp+24h] [ebp-4h]

    if ( !fromChunk || !fromChunk->gen.stream )
        return 0;
    smoke = Flame_Class_Smoke_Alloc();
    if ( !smoke )
        return 0;
    flameVars = fromChunk->gen.stream->flameVars;
    duration = (int)((Flame_Random(0) * flameVars->flameVar_smokeLifeRand + flameVars->flameVar_smokeLife) * 1000.0);
    *((unsigned int *)&smoke->gen + 23) = *((unsigned int *)&smoke->gen + 23) & 0xFFFFFFF8 | 3;
    smoke->gen.stream = fromChunk->gen.stream;
    *((unsigned int *)&smoke->gen + 23) = (16 * (*((unsigned int *)&fromChunk->gen + 23) >> 4)) | *((unsigned int *)&smoke->gen + 23) & 0xF;
    smoke->gen.age.startTime = curTime;
    smoke->gen.age.endTime = duration + curTime;
    smoke->gen.age.lastUpdateTime = curTime;
    smoke->gen.size.current = fromChunk->gen.size.current + flameVars->flameVar_smokeStartSizeAdd;
    smoke->gen.size.rate = (float)((float)(fromChunk->gen.size.current + flameVars->flameVar_smokeEndSizeAdd)
                                                             - smoke->gen.size.current)
                                             / flameVars->flameVar_smokeLife;
    smoke->gen.phys.origin[0] = fromChunk->gen.phys.origin[0];
    smoke->gen.phys.origin[1] = fromChunk->gen.phys.origin[1];
    smoke->gen.phys.origin[2] = fromChunk->gen.phys.origin[2];
    smoke->gen.phys.origin[0] = Flame_CRandom(0) * flameVars->flameVar_smokeOriginOffset + smoke->gen.phys.origin[0];
    smoke->gen.phys.origin[1] = Flame_CRandom(0) * flameVars->flameVar_smokeOriginOffset + smoke->gen.phys.origin[1];
    smoke->gen.phys.origin[2] = (float)((float)(fromChunk->gen.size.current * flameVars->flameVar_smokeOriginSizeOfsZScale)
                                                                        + flameVars->flameVar_smokeOriginOfsZ)
                                                        + smoke->gen.phys.origin[2];
    flameVar_smokeSpeedScale = flameVars->flameVar_smokeSpeedScale;
    smoke->gen.phys.velocity[0] = flameVar_smokeSpeedScale * fromChunk->gen.phys.velocity[0];
    smoke->gen.phys.velocity[1] = flameVar_smokeSpeedScale * fromChunk->gen.phys.velocity[1];
    smoke->gen.phys.velocity[2] = flameVar_smokeSpeedScale * fromChunk->gen.phys.velocity[2];
    smoke->gen.phys.velocity[2] = smoke->gen.phys.velocity[2] + flameVars->flameVar_smokeVelocityAddZ;
    smoke->gen.phys.invStartSpeed = Flame_CalcInvStartSpeed(
                                                                        fromChunk->gen.phys.invStartSpeed,
                                                                        flameVars->flameVar_smokeSpeedScale);
    smoke->gen.phys.gravity = flameVars->flameVar_smokeGravity;
    smoke->gen.phys.rotation = Flame_CRandom(0) * flameVars->flameVar_smokeMaxRotation;
    smoke->gen.phys.rotVel = Flame_CRandom(0) * flameVars->flameVar_smokeMaxRotVel;
    smoke->smokeFadein = flameVars->flameVar_smokeFadein;
    smoke->smokeFadeout = flameVars->flameVar_smokeFadeout;
    smoke->smokeMaxAlpha = flameVars->flameVar_smokeMaxAlpha;
    smoke->smokeBrightness = flameVars->flameVar_smokeBrightness;
    smoke->material = flameVars->smoke;
    return smoke;
}

void __cdecl Flame_Class_Smoke_Render_Item(int localClientNum, flameSmoke_t *smoke)
{
    float v2; // [esp+Ch] [ebp-8Ch]
    float v3; // [esp+10h] [ebp-88h]
    float v4; // [esp+1Ch] [ebp-7Ch]
    float v5; // [esp+20h] [ebp-78h]
    float alpha; // [esp+24h] [ebp-74h]
    float fwd[3]; // [esp+28h] [ebp-70h] BYREF
    float points[4][2]; // [esp+34h] [ebp-64h]
    float right[3]; // [esp+54h] [ebp-44h] BYREF
    float start[3]; // [esp+60h] [ebp-38h] BYREF
    float end[3]; // [esp+6Ch] [ebp-2Ch] BYREF
    float angles[3]; // [esp+78h] [ebp-20h] BYREF
    float down[3]; // [esp+84h] [ebp-14h] BYREF
    int i; // [esp+90h] [ebp-8h]
    cg_s *clientGlobals; // [esp+94h] [ebp-4h]

    clientGlobals = CG_GetLocalClientGlobals(localClientNum);
    if ( !clientGlobals
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\flame\\flame_class_smoke.cpp",
                    260,
                    0,
                    "%s\n\t%s",
                    "clientGlobals",
                    "unknown client index") )
    {
        __debugbreak();
    }
    if ( flame_debug_render->current.integer <= 0 )
    {
        alpha = 1.0f;
        if ( (float)((float)smoke->gen.age.startTime
                             + (float)((float)(smoke->gen.age.endTime - smoke->gen.age.startTime) * smoke->smokeFadein)) <= (float)smoke->gen.age.lastUpdateTime )
        {
            if ( (float)smoke->gen.age.lastUpdateTime > (float)((float)smoke->gen.age.startTime
                                                                                                                + (float)((float)(smoke->gen.age.endTime
                                                                                                                                                - smoke->gen.age.startTime)
                                                                                                                                * smoke->smokeFadeout)) )
                alpha = 1.0
                            - (float)((float)((float)((float)(smoke->gen.age.lastUpdateTime - smoke->gen.age.startTime)
                                                                            / (float)(smoke->gen.age.endTime - smoke->gen.age.startTime))
                                                            - smoke->smokeFadeout)
                                            / (float)(1.0 - smoke->smokeFadeout));
        }
        else
        {
            alpha = (float)((float)(smoke->gen.age.lastUpdateTime - smoke->gen.age.startTime)
                                        / (float)(smoke->gen.age.endTime - smoke->gen.age.startTime))
                        / smoke->smokeFadein;
        }
        Flame_Render_Sprite(
            clientGlobals,
            smoke->material,
            &smoke->gen,
            alpha * smoke->smokeMaxAlpha,
            -1.0,
            (int)(float)(255.0 * smoke->smokeBrightness));
    }
    else
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
        angles[2] = smoke->gen.phys.rotation;
        AngleVectors(angles, fwd, right, down);
        v5 = smoke->gen.size.current * 0.5;
        right[0] = v5 * right[0];
        right[1] = v5 * right[1];
        right[2] = v5 * right[2];
        v4 = smoke->gen.size.current * -0.5;
        down[0] = v4 * down[0];
        down[1] = v4 * down[1];
        down[2] = v4 * down[2];
        start[0] = (float)(points[3][0] * right[0]) + smoke->gen.phys.origin[0];
        start[1] = (float)(points[3][0] * right[1]) + smoke->gen.phys.origin[1];
        start[2] = (float)(points[3][0] * right[2]) + smoke->gen.phys.origin[2];
        start[0] = (float)(points[3][1] * down[0]) + start[0];
        start[1] = (float)(points[3][1] * down[1]) + start[1];
        start[2] = (float)(points[3][1] * down[2]) + start[2];
        for ( i = 0; i < 4; ++i )
        {
            v3 = points[i][0];
            end[0] = (float)(v3 * right[0]) + smoke->gen.phys.origin[0];
            end[1] = (float)(v3 * right[1]) + smoke->gen.phys.origin[1];
            end[2] = (float)(v3 * right[2]) + smoke->gen.phys.origin[2];
            v2 = points[i][1];
            end[0] = (float)(v2 * down[0]) + end[0];
            end[1] = (float)(v2 * down[1]) + end[1];
            end[2] = (float)(v2 * down[2]) + end[2];
            CG_DebugLine(start, end, colorMdGrey, 1, 1);
            start[0] = end[0];
            start[1] = end[1];
            start[2] = end[2];
        }
    }
}

void __cdecl Flame_Class_Smoke_Render_All(int localClientNum)
{
    flameSmoke_t *smoke; // [esp+0h] [ebp-4h]

    for ( smoke = flameSmokeUsed; smoke && smoke->gen.listGlobal.next; smoke = (flameSmoke_t *)smoke->gen.listGlobal.next )
        ;
    while ( smoke )
    {
        Flame_Class_Smoke_Render_Item(localClientNum, smoke);
        smoke = (flameSmoke_t *)smoke->gen.listGlobal.prev;
    }
}

