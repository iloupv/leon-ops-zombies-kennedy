#include "bg_jump.h"
#include "bg_local.h"
#include "bg_pmove.h"
#include "bg_misc.h"

#include <cgame_mp/cg_predict_mp.h>
#include <qcommon/common.h>

const dvar_t *jump_height;
const dvar_t *jump_stepSize;
const dvar_t *jump_slowdownEnable;
const dvar_t *jump_ladderPushVel;
const dvar_t *jump_spreadAdd;

// zombies: SP disables jump slowdown by default (SP 0x00579F20).
// Map selection can follow registration; migrate defaults, preserving custom values.
void Jump_UpdateSlowdownDefault()
{
    const bool enabled = !(zombiemode && zombiemode->current.enabled);
    dvar_s *dvar = const_cast<dvar_s *>(Dvar_FindVar("jump_slowdownEnable"));
    if (dvar && dvar->type == DVAR_TYPE_BOOL && dvar->reset.enabled != enabled)
    {
        const bool atDefault = dvar->current.enabled == dvar->reset.enabled
            && dvar->latched.enabled == dvar->reset.enabled;
        DvarValue value = {};
        value.enabled = enabled;
        Dvar_UpdateResetValue(dvar, value);
        if (atDefault)
            Dvar_SetBool(dvar, enabled);
    }
    jump_slowdownEnable = _Dvar_RegisterBool("jump_slowdownEnable", enabled, 0x180u, "Slow player movement after jumping");
}

void __cdecl Jump_RegisterDvars()
{
    jump_height = _Dvar_RegisterFloat("jump_height", 39.0, 0.0, 1000.0, 0x180u, "The maximum height of a player's jump");
    jump_stepSize = _Dvar_RegisterFloat(
                                        "jump_stepSize",
                                        18.0,
                                        0.0,
                                        64.0,
                                        0x180u,
                                        "The maximum step up to the top of a jump arc");
    Jump_UpdateSlowdownDefault();
    jump_ladderPushVel = _Dvar_RegisterFloat(
                                                 "jump_ladderPushVel",
                                                 128.0,
                                                 0.0,
                                                 1024.0,
                                                 0x180u,
                                                 "The velocity of a jump off of a ladder");
    jump_spreadAdd = _Dvar_RegisterFloat(
                                         "jump_spreadAdd",
                                         64.0,
                                         0.0,
                                         512.0,
                                         0x180u,
                                         "The amount of spread scale to add as a side effect of jumping");
}

void __cdecl Jump_ClearState(playerState_s *ps)
{
    ps->pm_flags &= ~0x4000u;
    ps->jumpOriginZ = 0.0f;
}

char __cdecl Jump_GetStepHeight(playerState_s *ps, const float *origin, float *stepSize)
{
    if ( (ps->pm_flags & 0x4000) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_jump.cpp",
                    72,
                    0,
                    "%s",
                    "ps->pm_flags & PMF_JUMPING") )
    {
        __debugbreak();
    }
    if ( !origin && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_jump.cpp", 73, 0, "%s", "origin") )
        __debugbreak();
    if ( !stepSize && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_jump.cpp", 74, 0, "%s", "stepSize") )
        __debugbreak();
    if ( (float)(ps->jumpOriginZ + jump_height->current.value) <= origin[2] )
        return 0;
    *stepSize = jump_stepSize->current.value;
    if ( (float)(origin[2] + *stepSize) > (float)(ps->jumpOriginZ + jump_height->current.value) )
        *stepSize = (float)(ps->jumpOriginZ + jump_height->current.value) - origin[2];
    return 1;
}

bool __cdecl Jump_IsPlayerAboveMax(playerState_s *ps)
{
    if ( (ps->pm_flags & 0x4000) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_jump.cpp",
                    92,
                    0,
                    "%s",
                    "ps->pm_flags & PMF_JUMPING") )
    {
        __debugbreak();
    }
    return ps->origin[2] >= (float)(ps->jumpOriginZ + jump_height->current.value);
}

void __cdecl Jump_ActivateSlowdown(playerState_s *ps)
{
    if ( !ps->pm_time )
    {
        ps->pm_flags |= 0x4000u;
        ps->pm_time = 1800;
    }
}

void __cdecl Jump_ApplySlowdown(playerState_s *ps)
{
    float scale; // [esp+8h] [ebp-4h]

    if ( (ps->pm_flags & 0x4000) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_jump.cpp",
                    110,
                    0,
                    "%s",
                    "ps->pm_flags & PMF_JUMPING") )
    {
        __debugbreak();
    }
    scale = 1.0f;
    if ( ps->pm_time <= 1800 )
    {
        if ( !ps->pm_time )
        {
            if ( (float)(ps->jumpOriginZ + 18.0) <= ps->origin[2] )
            {
                ps->pm_time = 1200;
                scale = 0.5f;
            }
            else
            {
                ps->pm_time = 1800;
                scale = 0.65f;
            }
        }
    }
    else
    {
        Jump_ClearState(ps);
        scale = 0.65f;
    }
    if ( jump_slowdownEnable->current.enabled )
    {
        ps->velocity[0] = scale * ps->velocity[0];
        ps->velocity[1] = scale * ps->velocity[1];
        ps->velocity[2] = scale * ps->velocity[2];
    }
}

double __cdecl Jump_ReduceFriction(playerState_s *ps)
{
    if ( (ps->pm_flags & 0x4000) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_jump.cpp",
                    155,
                    0,
                    "%s",
                    "ps->pm_flags & PMF_JUMPING") )
    {
        __debugbreak();
    }
    if ( ps->pm_time > 1800 )
    {
        Jump_ClearState(ps);
        return 1.0f;
    }
    else
    {
        return (float)Jump_GetSlowdownFriction(ps);
    }
}

double __cdecl Jump_GetSlowdownFriction(playerState_s *ps)
{
    if ( (ps->pm_flags & 0x4000) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_jump.cpp",
                    139,
                    0,
                    "%s",
                    "ps->pm_flags & PMF_JUMPING") )
    {
        __debugbreak();
    }
    if ( ps->pm_time > 1800
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_jump.cpp",
                    140,
                    0,
                    "%s",
                    "ps->pm_time <= JUMP_LAND_SLOWDOWN_TIME") )
    {
        __debugbreak();
    }
    if ( !jump_slowdownEnable->current.enabled )
        return 1.0;
    if ( ps->pm_time < 1700 )
        return (double)ps->pm_time * 1.5 * 0.00058823527 + 1.0;
    return 2.5;
}

void __cdecl Jump_ClampVelocity(playerState_s *ps, const float *origin)
{
    float maxJumpVel; // [esp+4h] [ebp-8h]
    float heightDiff; // [esp+8h] [ebp-4h]

    if ( (ps->pm_flags & 0x4000) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_jump.cpp",
                    175,
                    0,
                    "%s",
                    "ps->pm_flags & PMF_JUMPING") )
    {
        __debugbreak();
    }
    if ( !origin && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_jump.cpp", 176, 0, "%s", "origin") )
        __debugbreak();
    if ( (float)(ps->origin[2] - origin[2]) > 0.0 )
    {
        heightDiff = (float)(ps->jumpOriginZ + jump_height->current.value) - ps->origin[2];
        if ( heightDiff >= 0.1 )
        {
            maxJumpVel = sqrtf((float)(heightDiff * 2.0) * (float)ps->gravity);
            if ( ps->velocity[2] > maxJumpVel )
                ps->velocity[2] = maxJumpVel;
        }
        else
        {
            ps->velocity[2] = 0.0f;
        }
    }
}

char __cdecl Jump_Check(pmove_t *pm, pml_t *pml)
{
    playerState_s *ps; // [esp+4h] [ebp-4h]

    if ( !pm && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_jump.cpp", 312, 0, "%s", "pm") )
        __debugbreak();
    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_jump.cpp", 315, 0, "%s", "ps") )
        __debugbreak();
    if ( (ps->pm_flags & 0x80000) != 0 )
        return 0;
    if ( pm->cmd.serverTime - ps->jumpTime < 500 )
        return 0;
    if ( (ps->pm_flags & 0x400) != 0 )
        return 0;
    if ( (ps->pm_flags & 4) != 0 )
        return 0;
    if ( ps->pm_type >= 9 )
        return 0;
    if ( PM_GetEffectiveStance(ps) && ps->groundEntityNum != 1023 )
        return 0;
    //if ( !bitarray<51>::testBit(&pm->cmd.button_bits, 0xAu) )
    if ( !pm->cmd.button_bits.testBit(10) )
        return 0;
    //if ( bitarray<51>::testBit(&pm->oldcmd.button_bits, 0xAu) )
    if ( pm->oldcmd.button_bits.testBit(10) )
    {
        //bitarray<51>::resetBit(&pm->cmd.button_bits, 0xAu);
        pm->cmd.button_bits.resetBit(10);
        return 0;
    }
    else if ( (pml->groundTrace.sflags & 2) != 0 )
    {
        return 0;
    }
    else
    {
        Jump_Start(pm, pml, jump_height->current.value);
        Jump_AddSurfaceEvent(ps, pml);
        if ( (ps->pm_flags & 8) != 0 )
            Jump_PushOffLadder(ps, pml);
        BG_AnimScriptEvent(pm, ANIM_ET_JUMP, 0, 1);
        return 1;
    }
}

void __cdecl Jump_Start(pmove_t *pm, pml_t *pml, float height)
{
    float factor; // [esp+0h] [ebp-Ch]
    float velocitySqrd; // [esp+4h] [ebp-8h]
    playerState_s *ps; // [esp+8h] [ebp-4h]

    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_jump.cpp", 217, 0, "%s", "ps") )
        __debugbreak();
    velocitySqrd = (float)(height * 2.0) * (float)ps->gravity;
    if ( (ps->pm_flags & 0x4000) != 0 && ps->pm_time <= 1800 )
    {
        factor = Jump_GetLandFactor(ps);
        if ( factor == 0.0
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_jump.cpp", 226, 0, "%s", "factor") )
        {
            __debugbreak();
        }
        velocitySqrd = velocitySqrd / factor;
    }
    pml->groundPlane = 0;
    pml->almostGroundPlane = 0;
    pml->walking = 0;
    ps->groundEntityNum = 1023;
    ps->jumpTime = pm->cmd.serverTime;
    ps->jumpOriginZ = ps->origin[2];
    ps->velocity[2] = sqrtf(velocitySqrd);
    ps->pm_flags &= 0xFFFFFE7F;
    ps->pm_flags |= 0x4000u;
    ps->pm_time = 0;
    ps->sprintState.sprintButtonUpRequired = 0;
    ps->aimSpreadScale = ps->aimSpreadScale + jump_spreadAdd->current.value;
    if ( ps->aimSpreadScale > 255.0 )
        ps->aimSpreadScale = 255.0f;
}

double __cdecl Jump_GetLandFactor(playerState_s *ps)
{
    if ( (ps->pm_flags & 0x4000) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_jump.cpp",
                    198,
                    0,
                    "%s",
                    "ps->pm_flags & PMF_JUMPING") )
    {
        __debugbreak();
    }
    if ( ps->pm_time > 1800
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_jump.cpp",
                    199,
                    0,
                    "%s",
                    "ps->pm_time <= JUMP_LAND_SLOWDOWN_TIME") )
    {
        __debugbreak();
    }
    if ( !jump_slowdownEnable->current.enabled )
        return 1.0;
    if ( ps->pm_time < 1700 )
        return (double)ps->pm_time * 1.5 * 0.00058823527 + 1.0;
    return 2.5;
}

void __cdecl Jump_PushOffLadder(playerState_s *ps, pml_t *pml)
{
    float value; // [esp+4h] [ebp-50h]
    float v3; // [esp+34h] [ebp-20h]
    float flatForward[3]; // [esp+38h] [ebp-1Ch] BYREF
    float pushOffDir[3]; // [esp+44h] [ebp-10h] BYREF
    float dot; // [esp+50h] [ebp-4h]

    if ( (ps->pm_flags & 8) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_jump.cpp",
                    261,
                    0,
                    "%s",
                    "ps->pm_flags & PMF_LADDER") )
    {
        __debugbreak();
    }
    ps->velocity[2] = ps->velocity[2] * 0.75;
    v3 = pml->forward[1];
    flatForward[0] = pml->forward[0];
    flatForward[1] = v3;
    flatForward[2] = 0.0f;
    Vec3Normalize(flatForward);
    dot = (float)((float)(ps->vLadderVec[0] * pml->forward[0]) + (float)(ps->vLadderVec[1] * pml->forward[1]))
            + (float)(ps->vLadderVec[2] * pml->forward[2]);
    if ( dot >= 0.0 )
    {
        pushOffDir[0] = flatForward[0];
        pushOffDir[1] = flatForward[1];
        pushOffDir[2] = flatForward[2];
    }
    else
    {
        dot = (float)((float)(flatForward[0] * ps->vLadderVec[0]) + (float)(flatForward[1] * ps->vLadderVec[1]))
                + (float)(flatForward[2] * ps->vLadderVec[2]);
        pushOffDir[0] = (float)((float)(-2.0 * dot) * ps->vLadderVec[0]) + flatForward[0];
        pushOffDir[1] = (float)((float)(-2.0 * dot) * ps->vLadderVec[1]) + flatForward[1];
        pushOffDir[2] = (float)((float)(-2.0 * dot) * ps->vLadderVec[2]) + flatForward[2];
        Vec3Normalize(pushOffDir);
    }
    value = jump_ladderPushVel->current.value;
    ps->velocity[0] = value * pushOffDir[0];
    ps->velocity[1] = value * pushOffDir[1];
    ps->pm_flags &= ~8u;
}

void __cdecl Jump_AddSurfaceEvent(playerState_s *ps, pml_t *pml)
{
    int surfType; // [esp+0h] [ebp-4h]

    if ( (ps->pm_flags & 8) != 0 )
    {
        BG_AddPredictableEventToPlayerstate(0x6Eu, 0x15u, ps);
    }
    else
    {
        surfType = PM_GroundSurfaceType(ps, pml);
        if ( surfType )
            BG_AddPredictableEventToPlayerstate(0x6Eu, surfType, ps);
    }
}

//void __thiscall bitarray<51>::resetBit(bitarray<51> *this, unsigned int pos)
//{
//    if ( pos >= 0x33
//        && !Assert_MyHandler(
//                    "c:\\projects_pc\\cod\\codsrc\\src\\universal\\../qcommon/bitarray.h",
//                    127,
//                    0,
//                    "%s",
//                    "pos < BIT_COUNT") )
//    {
//        __debugbreak();
//    }
//    this->array[pos >> 5] &= ~(0x80000000 >> (pos & 0x1F));
//}
//
//
