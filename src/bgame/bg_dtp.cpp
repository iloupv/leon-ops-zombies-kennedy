#include "bg_dtp.h"
#include "bg_pmove.h"
#include "bg_public.h"
#include "bg_local.h"

#include <string.h>
#include <game/g_debug.h>
#include "bg_misc.h"
#include "bg_weapons.h"
#include "bg_jump.h"
#include <cgame_mp/cg_predict_mp.h>

const dvar_s *dtp;
const dvar_s *dtp_debug;
const dvar_s *dtp_max_slide_duration;
const dvar_s *dtp_max_slide_addition;
const dvar_s *dtp_startup_delay;
const dvar_s *dtp_post_move_pause;
const dvar_s *dtp_fall_damage_min_height;
const dvar_s *dtp_fall_damage_max_height;
const dvar_s *dtp_exhaustion_window;
const dvar_s *dtp_min_speed;
const dvar_s *door_breach_weapondrop;
const dvar_s *dtp_new_trajectory;
const dvar_s *dtp_new_trajectory_multiplier;
const dvar_s *dtp_max_apex_duration;

int Dtp_SurfaceRemapTable[31] =
{
  0,
  3,
  2,
  4,
  4,
  2,
  5,
  2,
  7,
  5,
  7,
  6,
  4,
  1,
  8,
  2,
  2,
  2,
  5,
  4,
  8,
  3,
  2,
  2,
  2,
  2,
  5,
  5,
  1,
  5,
  7
};



void __cdecl Dtp_RegisterDvars()
{
    dtp = _Dvar_RegisterBool("dtp", 1, 0x4080u, "Turn on/off the dive to prone functionality");
    dtp_debug = _Dvar_RegisterBool("dtp_debug", 0, 0x4080u, "Turn on/off dive to prone debug draw path");
    dtp_max_slide_duration = _Dvar_RegisterFloat(
                                                         "dtp_max_slide_duration",
                                                         300.0,
                                                         0.0,
                                                         10000.0,
                                                         0x4080u,
                                                         "Determines the max time the slide portion of the dtp move will take");
    dtp_max_slide_addition = _Dvar_RegisterFloat(
                                                         "dtp_max_slide_addition",
                                                         0.0,
                                                         0.0,
                                                         10000.0,
                                                         0x4080u,
                                                         "Additional slide time should the player land on a mantle-able surface");
    dtp_startup_delay = _Dvar_RegisterFloat(
                                                "dtp_startup_delay",
                                                250.0,
                                                100.0,
                                                10000.0,
                                                0x4080u,
                                                "Determines how long the player has to sprint before the dtp move can trigger");
    dtp_post_move_pause = _Dvar_RegisterFloat(
                                                    "dtp_post_move_pause",
                                                    100.0,
                                                    0.0,
                                                    10000.0,
                                                    0x4080u,
                                                    "Duration of the post move pause");
    dtp_fall_damage_min_height = _Dvar_RegisterFloat(
                                                                 "dtp_fall_damage_min_height",
                                                                 65.0,
                                                                 0.0,
                                                                 1000.0,
                                                                 0x4080u,
                                                                 "If the player dive to prone from less than this dvar value, he will not receive any kill damage");
    dtp_fall_damage_max_height = _Dvar_RegisterFloat(
                                                                 "dtp_fall_damage_max_height",
                                                                 200.0,
                                                                 0.0,
                                                                 1000.0,
                                                                 0x4080u,
                                                                 "If the player dives to prone from more than this dvar value, he will receive kill damage");
    dtp_exhaustion_window = _Dvar_RegisterFloat(
                                                        "dtp_exhaustion_window",
                                                        1500.0,
                                                        100.0,
                                                        10000.0,
                                                        0x4080u,
                                                        "Determines how long the player has to sprint before the dtp move can trigger");
    dtp_min_speed = _Dvar_RegisterFloat(
                                        "dtp_min_speed",
                                        3.1600001,
                                        0.0,
                                        1000.0,
                                        0x4080u,
                                        "Minimum player speed required to start a DTP or sustain a DTP");
    door_breach_weapondrop = _Dvar_RegisterBool(
                                                         "door_breach_weapondrop",
                                                         1,
                                                         0x4080u,
                                                         "Turn on/off the waepon drop/rise for during door breach");
    dtp_new_trajectory = _Dvar_RegisterBool("dtp_new_trajectory", 1, 0x4080u, "Use new dive to prone trajectory");
    dtp_new_trajectory_multiplier = _Dvar_RegisterFloat(
                                                                        "dtp_new_trajectory_multiplier",
                                                                        2.0,
                                                                        0.0,
                                                                        10.0,
                                                                        0x4080u,
                                                                        "New dive to prone trajectory modifier");
    dtp_max_apex_duration = _Dvar_RegisterInt(
                                                        "dtp_max_apex_duration",
                                                        400,
                                                        0,
                                                        10000,
                                                        0x4080u,
                                                        "Determines the max time in milliseconds the dtp will spend at the apex of the jump");
}

int __cdecl Dtp_CalcSurfaceType(pmove_t *pm, pml_t *pml)
{
    return Dtp_SurfaceRemapTable[PM_GroundSurfaceType(pm->ps, pml)];
}

bool __cdecl Dtp_IsSliding(const playerState_s *ps)
{
    return dtp->current.enabled && (ps->pm_flags & 0x400000) != 0 && ps->jumpTime < 0;
}

void __cdecl Dtp_AdjustDamage(const playerState_s *ps, float fallHeight, int *damage)
{
    int v3; // [esp+0h] [ebp-Ch]
    int v4; // [esp+8h] [ebp-4h]

    if ( dtp->current.enabled && (ps->pm_flags & 0x400000) != 0 )
    {
        if ( dtp_fall_damage_min_height->current.value <= 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_dtp.cpp",
                        74,
                        0,
                        "%s",
                        "dtp_fall_damage_min_height->current.value > 0") )
        {
            __debugbreak();
        }
        if ( dtp_fall_damage_max_height->current.value <= dtp_fall_damage_min_height->current.value
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_dtp.cpp",
                        75,
                        0,
                        "%s",
                        "dtp_fall_damage_min_height->current.value < dtp_fall_damage_max_height->current.value") )
        {
            __debugbreak();
        }
        if ( dtp_fall_damage_min_height->current.value < fallHeight )
        {
            if ( fallHeight < dtp_fall_damage_max_height->current.value )
            {
                *damage = (int)(float)((float)((float)(fallHeight - dtp_fall_damage_min_height->current.value)
                                                                         / (float)(dtp_fall_damage_max_height->current.value
                                                                                         - dtp_fall_damage_min_height->current.value))
                                                         * 100.0);
                if ( *damage < 100 )
                    v4 = *damage;
                else
                    v4 = 100;
                if ( v4 > 0 )
                    v3 = v4;
                else
                    v3 = 0;
                *damage = v3;
            }
            else
            {
                *damage = 100;
            }
        }
        else
        {
            *damage = 0;
        }
    }
}

void __cdecl Dtp_Interrupt(pmove_t *pm)
{
    float *velocity; // [esp+0h] [ebp-4h]

    pm->ps->pm_flags &= ~0x400000u;
    pm->ps->jumpTime = 0;
    velocity = pm->ps->velocity;
    *velocity = 0.0f;
    velocity[1] = 0.0f;
    velocity[2] = 0.0f;
    pm->ps->lastDtpEnd = pm->cmd.serverTime - (int)dtp_post_move_pause->current.value - 1;
    if ( pm->ps->lastDtpEnd < 0 )
        pm->ps->lastDtpEnd = 0;
    pm->ps->sprintState.sprintButtonUpRequired = 0;
}

void __cdecl Dtp_CheckForEnd(pmove_t *pm)
{
    float *velocity; // edx

    if ( dtp->current.enabled && (pm->ps->pm_flags & 0x400000) != 0 && pm->ps->jumpTime < 0 )
    {
        velocity = pm->ps->velocity;
        if ( (float)(dtp_min_speed->current.value * dtp_min_speed->current.value) > (float)((float)((float)(*velocity * *velocity)
                                                                                                                                                                                            + (float)(velocity[1] * velocity[1]))
                                                                                                                                                                            + (float)(velocity[2] * velocity[2])) )
            Dtp_End(pm);
    }
}

void __cdecl Dtp_End(pmove_t *pm)
{
    float *velocity; // [esp+0h] [ebp-4h]

    pm->ps->pm_flags &= ~0x400000u;
    pm->ps->jumpTime = 0;
    velocity = pm->ps->velocity;
    *velocity = 0.0f;
    velocity[1] = 0.0f;
    velocity[2] = 0.0f;
    pm->ps->lastDtpEnd = pm->cmd.serverTime;
    pm->ps->sprintState.sprintButtonUpRequired = 0;
}

double __cdecl Dtp_ReduceFriction(pmove_t *pm, const playerState_s *ps)
{
    float v3; // [esp+4h] [ebp-6Ch]
    float v4; // [esp+8h] [ebp-68h]
    float v5; // [esp+10h] [ebp-60h]
    float v6; // [esp+14h] [ebp-5Ch]
    int delta; // [esp+18h] [ebp-58h]
    float start[3]; // [esp+1Ch] [ebp-54h] BYREF
    trace_t trace; // [esp+28h] [ebp-48h] BYREF
    float point[3]; // [esp+64h] [ebp-Ch] BYREF

    if ( (ps->pm_flags & 0x400000) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_dtp.cpp",
                    159,
                    0,
                    "%s",
                    "ps->pm_flags & PMF_DIVE_TO_PRONE") )
    {
        __debugbreak();
    }
    if ( ps->jumpTime >= 0 )
        return 1.0;
    delta = pm->ps->jumpTime + pm->cmd.serverTime;
    v5 = ps->origin[1];
    v6 = ps->origin[2] + 1.0;
    start[0] = ps->origin[0];
    start[1] = v5;
    start[2] = v6;
    v3 = ps->origin[1];
    v4 = ps->origin[2] - 1.0;
    point[0] = ps->origin[0];
    point[1] = v3;
    point[2] = v4;
    PM_trace(pm, &trace, start, pm->mins, pm->maxs, point, ps->clientNum, 0x1000000);
    if ( trace.fraction < 1.0
        && (float)(dtp_max_slide_duration->current.value + dtp_max_slide_addition->current.value) > (float)delta )
    {
        return 0.0;
    }
    if ( dtp_max_slide_duration->current.value > (float)delta )
        return (double)delta / dtp_max_slide_duration->current.value;
    Dtp_End(pm);
    return 1.0;
}

void __cdecl Dtp_AirMove(playerState_s *ps, const pmove_t *pm)
{
    float v2; // [esp+0h] [ebp-8h]

    if ( (ps->pm_flags & 0x400000) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_dtp.cpp",
                    194,
                    0,
                    "%s",
                    "ps->pm_flags & PMF_DIVE_TO_PRONE") )
    {
        __debugbreak();
    }
    if ( dtp_debug->current.enabled && !pm->handler )
    {
        if ( dtp_new_trajectory->current.enabled )
            G_DebugStar(ps->origin, colorRed, 1500);
        else
            G_DebugStar(ps->origin, colorYellow, 1500);
    }
    if ( dtp_new_trajectory->current.enabled && ps->origin[2] >= (float)(ps->jumpOriginZ + jump_height->current.value) )
    {
        if ( pm->cmd.serverTime - ps->jumpTime >= dtp_max_apex_duration->current.integer )
        {
            if ( (float)(0.0 - ps->velocity[2]) < 0.0 )
                v2 = 0.0f;
            else
                v2 = ps->velocity[2];
            ps->velocity[2] = v2;
        }
        else
        {
            ps->velocity[2] = 0.0f;
        }
    }
}

bool __cdecl Dtp_CanMove(pmove_t *pm)
{
    if ( dtp->current.enabled
        && (pm->ps->pm_flags & 0x400000) != 0
        && pm->ps->waterlevel >= 1
        && (!dtp_new_trajectory->current.enabled || pm->ps->velocity[2] <= 0.0) )
    {
        Dtp_Interrupt(pm);
    }
    return !dtp->current.enabled
            || (pm->ps->pm_flags & 0x400000) != 0
            || !pm->ps->lastDtpEnd
            || dtp_post_move_pause->current.value <= (float)(pm->cmd.serverTime - pm->ps->lastDtpEnd);
}

int __cdecl Dtp_Update(pmove_t *pm, pml_t *pml)
{
    int eventParm; // [esp+4h] [ebp-14h]
    int timeSinceLastDtp; // [esp+Ch] [ebp-Ch]
    unsigned int dtp_stype; // [esp+10h] [ebp-8h]
    playerState_s *ps; // [esp+14h] [ebp-4h]

    if ( !dtp->current.enabled )
        return 0;
    if ( !pm && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_dtp.cpp", 291, 0, "%s", "pm") )
        __debugbreak();
    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_dtp.cpp", 294, 0, "%s", "ps") )
        __debugbreak();
    if ( (ps->pm_flags & 0x400000) == 0 )
    {
        if ( Dtp_IsDtp(pm, ps) )
        {
            timeSinceLastDtp = pm->cmd.serverTime - ps->lastDtpEnd;
            if ( ((float)timeSinceLastDtp > dtp_exhaustion_window->current.value || timeSinceLastDtp < 0)
                && (float)(pm->cmd.serverTime - ps->sprintState.lastSprintStart) > dtp_startup_delay->current.value )
            {
                Dtp_Start(pm, pml);
                eventParm = Dtp_CalcSurfaceType(pm, pml);
                BG_AddPredictableEventToPlayerstate(0xBAu, eventParm, ps);
                return 1;
            }
        }
        return 0;
    }
    if ( ps->jumpTime > 0 )
    {
        dtp_stype = Dtp_CalcSurfaceType(pm, pml);
        BG_AddPredictableEventToPlayerstate(0xBBu, dtp_stype, ps);
        BG_AnimScriptEvent(pm, ANIM_ET_DTP_LAND, 1, 1);
        ps->jumpTime = -pm->cmd.serverTime;
    }
    return 1;
}

void __cdecl Dtp_Start(pmove_t *pm, pml_t *pml)
{
    float velocitySqrd; // [esp+0h] [ebp-8h]
    playerState_s *ps; // [esp+4h] [ebp-4h]

    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_dtp.cpp", 98, 0, "%s", "ps") )
        __debugbreak();
    velocitySqrd = (float)(jump_height->current.value * 2.0) * (float)ps->gravity;
    pml->groundPlane = 0;
    pml->almostGroundPlane = 0;
    pml->walking = 0;
    ps->groundEntityNum = 1023;
    ps->jumpTime = pm->cmd.serverTime;
    ps->velocity[2] = sqrtf(velocitySqrd);
    if ( dtp_new_trajectory->current.enabled )
        ps->velocity[2] = ps->velocity[2] * dtp_new_trajectory_multiplier->current.value;
    ps->pm_flags |= 0x400000u;
    BG_AnimScriptEvent(pm, ANIM_ET_DTP_TAKEOFF, 0, 1);
    ps->pm_flags &= 0xFFDFBE7F;
    ps->pm_time = 0;
    ps->jumpOriginZ = ps->origin[2];
}

bool __cdecl Dtp_IsDtp(const pmove_t *pm, const playerState_s *ps)
{
    if ( !dtp->current.enabled )
        return 0;
    if ( BG_WeaponBlocksProne(ps->weapon) )
        return 0;
    if ( ps->sprintState.lastSprintEnd != pm->cmd.serverTime )
        return 0;
    //if ( bitarray<51>::testBit(&pm->cmd.button_bits, 0x2Cu) )
    if ( pm->cmd.button_bits.testBit(44) )
        return (float)(dtp_min_speed->current.value * dtp_min_speed->current.value) < (float)((float)(ps->velocity[0]
                                                                                                                                                                                                * ps->velocity[0])
                                                                                                                                                                                + (float)(ps->velocity[1]
                                                                                                                                                                                                * ps->velocity[1]));
    return 0;
}

