#include "bg_animconditions.h"

#include "bg_weapons.h"
#include "bg_animation.h"
#include "bg_weapons_def.h"
#include "bg_local.h"
#include "bg_public.h"
#include "bg_weapons_load_obj.h"
#include "bg_misc.h"
#include <cgame_mp/cg_predict_mp.h>

#include <cmath>
#include "bg_pmove.h"
#include "bg_perks.h"

void __cdecl BG_LocalEvalPlayerAnimType(pmove_t *pm)
{
    int typeIndex; // [esp+0h] [ebp-10h]
    const WeaponDef *weaponDef; // [esp+4h] [ebp-Ch]
    unsigned int weaponIndex; // [esp+8h] [ebp-8h]
    playerState_s *ps; // [esp+Ch] [ebp-4h]

    ps = pm->ps;
    weaponIndex = BG_GetPlayerStateWeaponIndex(pm);
    weaponDef = BG_GetWeaponDef(weaponIndex);
    if ( !weaponDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_animconditions.cpp", 65, 0, "%s", "weaponDef") )
    {
        __debugbreak();
    }
    if ( ps->pm_type != 7 || (typeIndex = BG_GetPlayerAnimTypeIndex(0), typeIndex == -1) )
        BG_SetConditionBit(ps->clientNum, 0, weaponDef->playerAnimType);
    else
        BG_SetConditionBit(ps->clientNum, 0, typeIndex);
}

void __cdecl BG_EvalPlayerAnimType(const entityState_s *es, const clientInfo_t *)
{
    const WeaponDef *weaponDef; // [esp+0h] [ebp-4h]

    weaponDef = BG_GetWeaponDef(es->weapon);
    if ( !weaponDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_animconditions.cpp", 87, 0, "%s", "weaponDef") )
    {
        __debugbreak();
    }
    BG_SetConditionBit(es->clientNum, 0, weaponDef->playerAnimType);
}

void __cdecl BG_LocalEvalNextPlayerAnimType(pmove_t *pm)
{
    const WeaponDef *weaponDef; // [esp+0h] [ebp-8h]

    weaponDef = BG_GetWeaponDef(pm->cmd.weapon);
    if ( !weaponDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_animconditions.cpp", 96, 0, "%s", "weaponDef") )
    {
        __debugbreak();
    }
    BG_SetConditionBit(pm->ps->clientNum, 2, weaponDef->playerAnimType);
}

void __cdecl BG_LocalEvalWeaponClass(pmove_t *pm)
{
    const WeaponDef *weaponDef; // [esp+0h] [ebp-Ch]
    unsigned int weaponIndex; // [esp+4h] [ebp-8h]
    playerState_s *ps; // [esp+8h] [ebp-4h]

    ps = pm->ps;
    weaponIndex = BG_GetPlayerStateWeaponIndex(pm);
    weaponDef = BG_GetWeaponDef(weaponIndex);
    BG_SetConditionBit(ps->clientNum, 1, weaponDef->weapClass);
}

void __cdecl BG_EvalWeaponClass(const entityState_s *es, const clientInfo_t *)
{
    const WeaponDef *weaponDef; // [esp+0h] [ebp-4h]

    weaponDef = BG_GetWeaponDef(es->weapon);
    if ( !weaponDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_animconditions.cpp", 117, 0, "%s", "weaponDef") )
    {
        __debugbreak();
    }
    BG_SetConditionBit(es->clientNum, 1, weaponDef->weapClass);
}

void __cdecl BG_LocalEvalNextWeaponClass(pmove_t *pm)
{
    const WeaponDef *weaponDef; // [esp+0h] [ebp-8h]

    weaponDef = BG_GetWeaponDef(pm->cmd.weapon);
    BG_SetConditionBit(pm->ps->clientNum, 3, weaponDef->weapClass);
}

void __cdecl BG_LocalEvalMounted(pmove_t *pm)
{
    playerState_s *ps; // [esp+0h] [ebp-4h]

    ps = pm->ps;
    if ( (pm->ps->eFlags & 0x300) != 0 )
        BG_SetConditionValue(ps->clientNum, 4u, 1u);
    else
        BG_SetConditionValue(ps->clientNum, 4u, 0);
}

void __cdecl BG_EvalMounted(const entityState_s *es, const clientInfo_t *)
{
    if ( (es->lerp.eFlags & 0x300) != 0
        || (es->lerp.eFlags & 0x4000) != 0 && es->lerp.u.player.vehicleSeat && es->lerp.u.player.vehicleSeat <= 4u )
    {
        BG_SetConditionValue(es->clientNum, 4u, 1u);
    }
    else
    {
        BG_SetConditionValue(es->clientNum, 4u, 0);
    }
}

void __cdecl BG_LocalEvalMoveStatus(pmove_t *pm)
{
    if ( (pm->ps->pm_flags & 0x100) != 0
        && player_knockbackMoveThreshhold->current.value > (float)((float)(pm->ps->velocity[0] * pm->ps->velocity[0])
                                                                                                                         + (float)(pm->ps->velocity[1] * pm->ps->velocity[1])) )
    {
        goto LABEL_7;
    }
    if ( (float)((float)(pm->ps->velocity[0] * pm->ps->velocity[0]) + (float)(pm->ps->velocity[1] * pm->ps->velocity[1])) > (float)(player_runThreshhold->current.value * player_runThreshhold->current.value) )
    {
        BG_SetConditionBit(pm->ps->clientNum, 5, 2);
        return;
    }
    if ( (float)((float)(pm->ps->velocity[0] * pm->ps->velocity[0]) + (float)(pm->ps->velocity[1] * pm->ps->velocity[1])) <= (float)(player_moveThreshhold->current.value * player_moveThreshhold->current.value) )
LABEL_7:
        BG_SetConditionBit(pm->ps->clientNum, 5, 0);
    else
        BG_SetConditionBit(pm->ps->clientNum, 5, 1);
}

void __cdecl BG_EvalMoveStatus(const entityState_s *es, const clientInfo_t *)
{
    unsigned int legsAnim; // [esp+0h] [ebp-4h]

    legsAnim = es->animState.state & 0xFFFFFBFF;
   
    if ((bgs->animData->animScriptData.animations[legsAnim].flags & 0x4000000) != 0)
    {
        BG_SetConditionValue(es->clientNum, 5u, 2u);
    }
    else if ((bgs->animData->animScriptData.animations[legsAnim].flags & 0x2000000) != 0)
    {
        BG_SetConditionValue(es->clientNum, 5u, 1u);
    }
    else
    {
        BG_SetConditionValue(es->clientNum, 5u, 0);
    }
}

void __cdecl BG_LocalEvalDirection(pmove_t *pm)
{
    float t; // [esp+18h] [ebp-24h]
    float temp[3]; // [esp+1Ch] [ebp-20h] BYREF
    float moveVec[2]; // [esp+28h] [ebp-14h] BYREF
    float yaw; // [esp+30h] [ebp-Ch]
    scriptAnimDirections_t directionState; // [esp+34h] [ebp-8h]
    float deltaYaw; // [esp+38h] [ebp-4h]

    moveVec[0] = (float)pm->cmd.rightmove;
    moveVec[1] = (float)pm->cmd.forwardmove;
    Vec2Normalize(moveVec);
    if ( player_topDownCamMode->current.integer >= 2 )
    {
        t = moveVec[0];
        moveVec[0] = moveVec[1];
        moveVec[1] = -(t);
        yaw = vectoyaw(moveVec);
        deltaYaw = AngleDelta(yaw, pm->ps->viewangles[1]);
        YawVectors(deltaYaw, temp, 0);
        moveVec[0] = -(temp[1]);
        moveVec[1] = temp[0];
    }
    if ( moveVec[1] <= player_strafeAnimCosAngle->current.value )
    {
        //if ( COERCE_FLOAT(player_strafeAnimCosAngle->current.integer ^ _mask__NegFloat_) <= moveVec[1] )
        if ( -player_strafeAnimCosAngle->current.value <= moveVec[1] )
        {
            if ( moveVec[0] <= 0.0 )
                directionState = ANIM_DIRECTION_LEFT;
            else
                directionState = ANIM_DIRECTION_RIGHT;
        }
        else
        {
            directionState = ANIM_DIRECTION_BACKWARD;
        }
    }
    else
    {
        directionState = ANIM_DIRECTION_FORWARD;
    }
    BG_SetConditionValue(pm->ps->clientNum, 6u, directionState);
}

void __cdecl BG_EvalDirection(const entityState_s *es, const clientInfo_t *)
{
    unsigned int legsAnim; // [esp+0h] [ebp-4h]

    legsAnim = es->animState.state & 0xFFFFFBFF;
    if ((bgs->animData->animScriptData.animations[legsAnim].flags
        & 0x10) != 0)
    {
        BG_SetConditionValue(es->clientNum, 6u, 3u);
    }
    else if ((bgs->animData->animScriptData.animations[legsAnim].flags
        & 0x20) != 0)
    {
        BG_SetConditionValue(es->clientNum, 6u, 4u);
    }
    else if ((bgs->animData->animScriptData.animations[legsAnim].flags
        & 0x40) != 0)
    {
        BG_SetConditionValue(es->clientNum, 6u, 2u);
    }
    else if ((bgs->animData->animScriptData.animations[legsAnim].flags
        & 0x80) != 0)
    {
        BG_SetConditionValue(es->clientNum, 6u, 1u);
    }
    else
    {
        BG_SetConditionValue(es->clientNum, 6u, 0);
    }
}

void __cdecl BG_LocalEvalDmgDirection(pmove_t *pm)
{
    playerState_s *ps; // [esp+0h] [ebp-4h]

    ps = pm->ps;
    if ( pm->ps->dmgDirection == 3 )
    {
        BG_SetConditionValue(ps->clientNum, 7u, 3u);
    }
    else
    {
        switch ( ps->dmgDirection )
        {
            case 4:
                BG_SetConditionValue(ps->clientNum, 7u, 4u);
                break;
            case 2:
                BG_SetConditionValue(ps->clientNum, 7u, 2u);
                break;
            case 1:
                BG_SetConditionValue(ps->clientNum, 7u, 1u);
                break;
            default:
                BG_SetConditionValue(ps->clientNum, 7u, 0);
                break;
        }
    }
}

void __cdecl BG_EvalDmgDirection(const entityState_s *es, const clientInfo_t *)
{
    unsigned int legsAnim; // [esp+0h] [ebp-4h]

    legsAnim = es->animState.state & 0xFFFFFBFF;
    if ((bgs->animData->animScriptData.animations[legsAnim].flags
        & 0x80000) != 0)
    {
        BG_SetConditionValue(es->clientNum, 7u, 3u);
    }
    else if ((bgs->animData->animScriptData.animations[legsAnim].flags
        & 0x100000) != 0)
    {
        BG_SetConditionValue(es->clientNum, 7u, 4u);
    }
    else if ((bgs->animData->animScriptData.animations[legsAnim].flags
        & 0x200000) != 0)
    {
        BG_SetConditionValue(es->clientNum, 7u, 2u);
    }
    else if ((bgs->animData->animScriptData.animations[legsAnim].flags
        & 0x400000) != 0)
    {
        BG_SetConditionValue(es->clientNum, 7u, 1u);
    }
    else
    {
        BG_SetConditionValue(es->clientNum, 7u, 0);
    }
}

void __cdecl BG_LocalEvalDmgType(pmove_t *pm)
{
    playerState_s *ps; // [esp+0h] [ebp-4h]

    ps = pm->ps;
    if ( pm->ps->dmgType == 1 )
    {
        BG_SetConditionValue(ps->clientNum, 8u, 4u);
    }
    else
    {
        switch ( ps->dmgType )
        {
            case 4:
                BG_SetConditionValue(ps->clientNum, 8u, 1u);
                break;
            case 3:
                BG_SetConditionValue(ps->clientNum, 8u, 3u);
                break;
            case 2:
                BG_SetConditionValue(ps->clientNum, 8u, 2u);
                break;
            case 5:
                BG_SetConditionValue(ps->clientNum, 8u, 5u);
                break;
            default:
                BG_SetConditionValue(ps->clientNum, 8u, 0);
                break;
        }
    }
}

void __cdecl BG_LocalEvalStance(pmove_t *pm)
{
    int stance; // [esp+0h] [ebp-4h]

    stance = PM_GetEffectiveStance(pm->ps);
    if ( stance == 1 || (pm->ps->pm_flags & 0x400000) != 0 )
    {
        BG_SetConditionValue(pm->ps->clientNum, 9u, 2u);
    }
    else if ( stance == 2 )
    {
        BG_SetConditionValue(pm->ps->clientNum, 9u, 1u);
    }
    else
    {
        BG_SetConditionValue(pm->ps->clientNum, 9u, 0);
    }
}

void __cdecl BG_EvalStance(const entityState_s *es, const clientInfo_t *)
{
    unsigned int legsAnim; // [esp+0h] [ebp-4h]

    legsAnim = es->animState.state & 0xFFFFFBFF;
    if ((bgs->animData->animScriptData.animations[legsAnim].flags
        & 0x1000000) != 0)
    {
        BG_SetConditionValue(es->clientNum, 9u, 2u);
    }
    else if ((bgs->animData->animScriptData.animations[legsAnim].flags
        & 0x800000) != 0)
    {
        BG_SetConditionValue(es->clientNum, 9u, 1u);
    }
    else
    {
        BG_SetConditionValue(es->clientNum, 9u, 0);
    }
}

void __cdecl BG_LocalEvalUnderhand(pmove_t *pm)
{
    BG_SetConditionValue(pm->ps->clientNum, 0xAu, pm->ps->viewangles[0] > 0.0);
}

void __cdecl BG_EvalUnderhand(const entityState_s *es, const clientInfo_t *ci)
{
    BG_SetConditionValue(es->clientNum, 0xAu, ci->playerAngles[0] > 0.0);
}

void __cdecl BG_LocalEvalFiring(pmove_t *pm)
{
    //if ( bitarray<51>::testBit(&pm->cmd.button_bits, 0) )
    if ( pm->cmd.button_bits.testBit(0) )
        BG_SetConditionValue(pm->ps->clientNum, 0xBu, 1u);
    else
        BG_SetConditionValue(pm->ps->clientNum, 0xBu, 0);
}

void __cdecl BG_EvalFiring(const entityState_s *es, const clientInfo_t *)
{
    if ( (es->lerp.eFlags & 0x40) != 0 )
        BG_SetConditionValue(es->clientNum, 0xBu, 1u);
    else
        BG_SetConditionValue(es->clientNum, 0xBu, 0);
}

void __cdecl BG_LocalEvalWeaponPosition(pmove_t *pm)
{
    if ( (pm->ps->pm_flags & 0x10) != 0 )
        BG_SetConditionValue(pm->ps->clientNum, 0xCu, 1u);
    else
        BG_SetConditionValue(pm->ps->clientNum, 0xCu, 0);
}

void __cdecl BG_EvalWeaponPosition(const entityState_s *es, const clientInfo_t *)
{
    if ( (es->lerp.eFlags & 0x80000) != 0 )
        BG_SetConditionValue(es->clientNum, 0xCu, 1u);
    else
        BG_SetConditionValue(es->clientNum, 0xCu, 0);
}

void __cdecl BG_LocalEvalSlope(pmove_t *pm)
{
    scriptAnimSlopeStates_t slopeState; // [esp+0h] [ebp-4h]

    if ( player_useSlopeAnims->current.enabled )
    {
        if ( pm->averagePitch <= player_slopeAnimAngle->current.value )
        {
            if ( (-player_slopeAnimAngle->current.value) <= pm->averagePitch )
                slopeState = ANIM_SLOPE_NONE;
            else
                slopeState = ANIM_SLOPE_DOWN;
        }
        else
        {
            slopeState = ANIM_SLOPE_UP;
        }
    }
    else
    {
        slopeState = ANIM_SLOPE_NONE;
    }
    BG_SetConditionValue(pm->ps->clientNum, 0xDu, slopeState);
}

void __cdecl BG_EvalSlope(const entityState_s *es, const clientInfo_t *)
{
    unsigned int legsAnim; // [esp+0h] [ebp-4h]

    legsAnim = es->animState.state & 0xFFFFFBFF;
    if ((bgs->animData->animScriptData.animations[legsAnim].flags & 0x20000) != 0)
    {
        BG_SetConditionValue(es->clientNum, 0xDu, 1u);
    }
    else if ((bgs->animData->animScriptData.animations[legsAnim].flags & 0x40000) != 0)
    {
        BG_SetConditionValue(es->clientNum, 0xDu, 2u);
    }
    else
    {
        BG_SetConditionValue(es->clientNum, 0xDu, 0);
    }
}

void __cdecl BG_LocalEvalPerk(pmove_t *pm)
{
    playerState_s *ps; // [esp+14h] [ebp-4h]

    ps = pm->ps;
    if ( pm->ps->pm_type == 6 || ps->pm_type == 7 )
    {
        BG_SetConditionBit(ps->clientNum, 14, 1);
    }
    else if ( (ps->weapFlags & 0x40000) != 0
                 && (ps->perks[0] & 0x4000000) != 0
                 && ((ps->eFlags & 0x4000) == 0 || ps->vehicleType == 6) )
    {
        if ( BG_FindWeaponIndexForName(perk_grenadeDeath->current.string) )
            BG_SetConditionBit(ps->clientNum, 14, 2);
    }
    else
    {
        BG_SetConditionBit(ps->clientNum, 14, 0);
    }
}

void __cdecl BG_LocalEvalAttachment(pmove_t *pm)
{
    const WeaponDef *weaponDef; // [esp+0h] [ebp-Ch]
    unsigned int weaponIndex; // [esp+4h] [ebp-8h]
    playerState_s *ps; // [esp+8h] [ebp-4h]

    ps = pm->ps;
    weaponIndex = BG_GetPlayerStateWeaponIndex(pm);
    weaponDef = BG_GetWeaponDef(weaponIndex);
    if ( !weaponDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_animconditions.cpp", 463, 0, "%s", "weaponDef") )
    {
        __debugbreak();
    }
    if ( weaponDef->bHasBayonet )
        BG_SetConditionValue(ps->clientNum, 0xFu, 1u);
    else
        BG_SetConditionValue(ps->clientNum, 0xFu, 0);
}

void __cdecl BG_LocalEvalVehicleName(pmove_t *pm)
{
    int animSetIndex; // [esp+0h] [ebp-Ch]
    unsigned __int16 animSetStr; // [esp+4h] [ebp-8h]
    playerState_s *ps; // [esp+8h] [ebp-4h]

    ps = pm->ps;
    if ( (pm->ps->eFlags & 0x4000) == 0
        || (ps->eFlags & 0x10000) != 0
        || (ps->eFlags2 & 0x10000000) != 0
        || (animSetStr = pmoveHandlers[pm->handler].getVehicleTypeString(ps->clientNum, ps->viewlocked_entNum)) == 0)
    {
        BG_SetConditionValue(ps->clientNum, 0x10u, 0xFFu);
    }
    else
    {
        animSetIndex = BG_GetVehicleAnimSetIndex(animSetStr);
        if ( animSetIndex >= 0 )
            BG_SetConditionValue(ps->clientNum, 0x10u, animSetIndex);
    }
}

void __cdecl BG_LocalEvalVehicleSeatTo(pmove_t *pm)
{
    playerState_s *ps; // [esp+0h] [ebp-4h]

    ps = pm->ps;
    if ( (pm->ps->eFlags & 0x4000) != 0 && (ps->eFlags & 0x10000) == 0 && (ps->eFlags2 & 0x10000000) == 0 )
        BG_SetConditionBit(ps->clientNum, 17, ps->vehiclePos);
}

void __cdecl BG_LocalEvalEvent(pmove_t *pm)
{
    playerState_s *ps; // [esp+0h] [ebp-4h]

    ps = pm->ps;
    if ( !pm->ps
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_animconditions.cpp", 558, 0, "%s", "ps") )
    {
        __debugbreak();
    }
    if ( (ps->pm_flags & 0x100000) != 0 )
    {
        if ( !ps->scriptedAnimTime )
            BG_SetConditionValue(ps->clientNum, 0x15u, ps->scriptedAnim);
        if ( pm->cmd.serverTime < ps->scriptedAnimTime )
            BG_SetConditionValue(ps->clientNum, 0x15u, ps->scriptedAnim);
    }
}

