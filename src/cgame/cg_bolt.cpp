#include "cg_bolt.h"
#include <bgame/bg_local.h>
#include "cg_spawn.h"
#include <bgame/bg_misc.h>
#include <bgame/bg_weapons_def.h>
#include <cgame_mp/cg_local_mp.h>
#include <xanim/xmodel_utils.h>
#include <universal/com_math_anglevectors.h>

centity_s *g_BoltEnt[1][10];
unsigned int g_BoltIndex[1];

void __cdecl CG_InitBolt(unsigned int localClientNum)
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < 10; ++i )
    {
        g_BoltEnt[localClientNum][i] = CG_Spawn(localClientNum);
        if ( g_BoltEnt[localClientNum][i] )
        {
            g_BoltEnt[localClientNum][i]->nextState.eType = ET_SCRIPTMOVER;
            g_BoltEnt[localClientNum][i]->nextState.lerp.u.player.meleeWeapon = 1023;
            g_BoltEnt[localClientNum][i]->currentState.eFlags |= 0x20u;
            g_BoltEnt[localClientNum][i]->nextState.lerp.eFlags |= 0x20u;
        }
    }
    g_BoltIndex[localClientNum] = 0;
}

unsigned int __cdecl CG_AddBolt(
                int localClientNum,
                int targetEntityNum,
                unsigned int weapon,
                const float *position,
                const float *normal,
                unsigned __int8 tagIndex,
                int physUserBody)
{
    const char *Name; // eax
    int ModelIndex; // eax
    float *v10; // [esp+4h] [ebp-58h]
    float *angles; // [esp+8h] [ebp-54h]
    float *trBase; // [esp+Ch] [ebp-50h]
    float *origin; // [esp+10h] [ebp-4Ch]
    float wallPos[3]; // [esp+1Ch] [ebp-40h] BYREF
    float axis[3][3]; // [esp+28h] [ebp-34h] BYREF
    cg_s *cgameGlob; // [esp+4Ch] [ebp-10h]
    centity_s *cent; // [esp+50h] [ebp-Ch]
    centity_s *other; // [esp+54h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+58h] [ebp-4h]
    int savedregs; // [esp+5Ch] [ebp+0h] BYREF

    cent = g_BoltEnt[localClientNum][g_BoltIndex[localClientNum] % 0xA];
    weapDef = BG_GetWeaponDef(weapon);
    if ( !weapDef && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_bolt.cpp", 101, 0, "%s", "weapDef") )
        __debugbreak();
    if ( !cent )
        return -1;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->predictedPlayerState.clientNum == targetEntityNum )
        return -1;
    other = CG_GetEntity(localClientNum, targetEntityNum);
    cent->currentState.eFlags &= ~0x20u;
    cent->nextState.lerp.eFlags &= ~0x20u;
    Name = XModelGetName(weapDef->projectileModel);
    ModelIndex = CG_GetModelIndex(Name, localClientNum);
    AssignToSmallerType<short>(&cent->nextState.index.brushmodel, ModelIndex);
    AssignToSmallerType<short>(&cent->nextState.lerp.u.vehicle.gunnerAngles[2].yaw, targetEntityNum);
    AssignToSmallerType<int>(&cent->nextState.lerp.u.loopFx.period, tagIndex);
    cent->nextState.lerp.eFlags2 |= 0x80000u;
    origin = cent->pose.origin;
    cent->pose.origin[0] = *position;
    origin[1] = position[1];
    origin[2] = position[2];
    trBase = cent->nextState.lerp.pos.trBase;
    cent->nextState.lerp.pos.trBase[0] = *position;
    trBase[1] = position[1];
    trBase[2] = position[2];
    angles = cent->pose.angles;
    cent->pose.angles[0] = *normal;
    angles[1] = normal[1];
    angles[2] = normal[2];
    v10 = cent->nextState.lerp.apos.trBase;
    cent->nextState.lerp.apos.trBase[0] = *normal;
    v10[1] = normal[1];
    v10[2] = normal[2];
    if ( physUserBody )
    {
        AnglesToAxis(normal, axis);
        wallPos[0] = *position - axis[0][0];
        wallPos[1] = position[1] - axis[0][1];
        wallPos[2] = position[2] - axis[0][2];
        Phys_SetUserBody(physUserBody, wallPos);
    }
    return ++g_BoltIndex[localClientNum];
}

void __cdecl CG_UpdateBolt(int localClientNum)
{
    int i; // [esp+Ch] [ebp-8h]
    centity_s *bolt; // [esp+10h] [ebp-4h]

    for ( i = 0; i < 10; ++i )
    {
        bolt = g_BoltEnt[localClientNum][i];
        if ( bolt
            && bolt->nextState.lerp.u.player.meleeWeapon != 1022
            && bolt->linkInfo
            && ((*((unsigned int *)CG_GetEntity(localClientNum, bolt->linkInfo->linkEnt) + 201) >> 1) & 1) == 0 )
        {
            bolt->nextState.lerp.u.player.meleeWeapon = 1023;
            bolt->nextState.lerp.eFlags |= 0x20u;
        }
    }
}